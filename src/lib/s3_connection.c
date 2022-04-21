#include "lib/s3_connection.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "third/logc/log.h"
#include "lib/s3_error.h"
#include "lib/s3_malloc.h"
#include "lib/s3_message.h"
#include "lib/s3_net_code.h"
#include "lib/s3_packet.h"
#include "lib/s3_request.h"
#include "lib/s3_socket.h"

S3Connection *s3_connection_construct() {
    S3Connection *c = s3_malloc_(S3_MOD_CONNECTION, sizeof(S3Connection));
    if (c != NULL) {
        *c = (S3Connection)s3_connection_null;
    }
    return c;
}

void s3_connection_destruct(S3Connection *c) {
    if (c != NULL) {
        s3_connection_destroy(c);
        s3_free_(S3_MOD_CONNECTION, c);
    }
}

int s3_connection_init(S3Connection *c, struct ev_loop *loop, int fd) {
    s3_socket_set_non_blocking(fd);

    c->loop          = loop;
    c->fd            = fd;
    s3_list_head_init(&c->conn_list_node);
    s3_list_head_init(&c->write_list_node);
    s3_list_init(&c->message_list);
    s3_list_init(&c->output_buf_list);

    return S3_OK;
}

void s3_connection_destroy(S3Connection *c) {
    if (c != NULL) {

#ifdef LIBEVDB_UNIT_TEST
        log_info("unit test\n");
        if (c->loop != NULL) {
            ev_io_stop(c->loop, &c->read_watcher);
            ev_io_stop(c->loop, &c->write_watcher);
        }
        if (c->fd >= 0) {
            close(c->fd);
        }
#else
        ev_io_stop(c->loop, &c->read_watcher);
        ev_io_stop(c->loop, &c->write_watcher);

        close(c->fd);
#endif

        log_info("destroy connection, fd=%d, recv total request cnt=%ld",
                 c->fd, c->request_total_cnt);
        c->loop = NULL;
        c->fd = -1;
        c->closed = 1;

        S3Buf *b, *dummy_buf;
        s3_list_for_each_entry_safe(b, dummy_buf, &c->output_buf_list, node) {
            s3_list_del(&b->node);
            s3_buf_destruct(b);
        }

        S3Message *m, *dummy;
        s3_list_for_each_entry_safe(m, dummy, &c->message_list, message_list_node) {
            s3_list_del(&m->message_list_node);
            s3_message_destruct(m);
        }

    }
}

static int s3_connnection_request_done(S3Request *r);
static int s3_connection_write_socket(S3Connection *c);
static int s3_connection_write_socket_again(S3Connection *c);
static int s3_connection_process_request(S3Connection *c, S3List *request_list) {
    S3List req_list;
    s3_list_movelist(request_list, &req_list);

    int ret = 0;
    S3Request *r, *rn;
    s3_list_for_each_entry_safe(r, rn, &req_list, request_list_node) {
        s3_list_del(&r->request_list_node);
        ret = (c->handler->process)(r);
        assert(ret == 0);
    }
    return S3_OK;
}

static int s3_connection_process_message(S3Connection *c, S3Message *m) {
    while (s3_buf_unconsumed_size(m->recv_buf) > 0) {
        S3Packet *p = s3_net_decode(m);
        if (p == NULL) {
            break;
        }
        S3Request *r = s3_request_construct();
        assert(r != NULL);
        r->in_packet = p;
        r->message = m;
        m->request_cnt++;

        s3_list_add_tail(&r->request_list_node, &m->request_list);
        c->request_doing_cnt++;
        c->request_total_cnt++;
    }
    s3_connection_process_request(c, &m->request_list);
    return S3_OK;
}

static S3Message *s3_connection_get_recv_msg(S3Connection *c) {
    S3Message *m = s3_list_get_last(&c->message_list, S3Message, message_list_node);

    if (m == NULL) {
        m = s3_message_create();
        assert(m != NULL);
        m->conn = c;
        c->message_total_cnt++;
        s3_list_add_tail(&m->message_list_node, &c->message_list);
    } else if ((s3_buf_free_size(m->recv_buf) < S3_MSG_BUF_MIN_LEN) ||
               (m->read_status == S3_MSG_READ_STATUS_AGAIN && m->next_read_len > s3_buf_free_size(m->recv_buf))) {

        log_info("message free not enough, next_read_len=%ld, free size=%ld",
                m->next_read_len, s3_buf_free_size(m->recv_buf));
        S3Message *old_msg = m;

        m = s3_message_create_with_old(old_msg);
        assert(m != NULL);
        m->conn = c;
        c->message_total_cnt++;
        s3_list_add_tail(&m->message_list_node, &c->message_list);

        s3_message_try_destruct(old_msg);
    }

    return m;
}

void s3_connection_recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    S3Connection *c = (S3Connection*)w->data;
    if (c->request_doing_cnt >= S3_CONN_DOING_MAX_REQ_CNT) {
        log_error("connection doing req count exceed, doing cnt=%ld", c->request_doing_cnt);
        (c->handler->close)(c);
        return;
    }

    S3Message *m = s3_connection_get_recv_msg(c);
    assert(m != NULL);

    int n = s3_socket_read(w->fd, m->recv_buf->right, s3_buf_free_size(m->recv_buf));
    if (n < 0) {
        if (n == S3_ERR_NET_AGAIN) {
            return;
        }
        // connection reset by peer
        log_info("connection read fail. fd=%d, ret=%d, errno=%d", c->fd, n, errno);
        (c->handler->close)(c);
        return;
    } else if (n == 0) {
        log_info("peer connection close. fd=%d", c->fd);
        (c->handler->close)(c);
        return;
    }
    m->recv_buf->right += n;
    log_debug("recv data, len=%d", n);

    int ret = s3_connection_process_message(c, m);
    assert(ret == 0);
}

void s3_connection_write_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    S3Connection *c = (S3Connection*)w->data;

    int ret = S3_OK;
    ret = s3_connection_write_socket(c);
    if (ret == S3_OK) {
        ev_io_stop(loop, w/*conn->write_watcher*/);
        return;
    }

    if (ret == S3_ERR_NET_AGAIN) { // not stop write watcher
        return;
    }

    if (ret == S3_ERR_NET_ABORT) {
        log_info("conn write error, try destruct conn.");
        (c->handler->close)(c);
        return;
    }

    assert(false);
}

static int s3_connnection_request_done(S3Request *r) {
    S3Message *m = (S3Message*)r->message;
    S3Connection *c = (S3Connection*)m->conn;

    m->request_done_cnt++;
    c->request_doing_cnt--;
}

void s3_connnection_resp_request(S3List *request_list) {
    int ret = S3_OK;
    int64_t total_size = 0;
    S3List write_conn_list = S3_LIST_INIT(write_conn_list);

    S3Message *m;
    S3Connection *c, *dummy_conn;
    S3Request *r, *dummy_request;
    s3_list_for_each_entry_safe(r, dummy_request, request_list, request_list_node) {
        if (total_size > S3_IO_MAX_SIZE) {
            break;
        }
        s3_list_del(&r->request_list_node);

        m = (S3Message*)r->message;
        c = (S3Connection*)m->conn;

        S3List out_list = S3_LIST_HEAD_INIT(out_list);
        ret = s3_packet_out_buf_list(r->out_packet, &out_list); // request可以被释放了
        s3_list_join(&out_list, &c->output_buf_list);

        if (s3_list_empty(&c->write_list_node)) {
            s3_list_add_tail(&c->write_list_node, &write_conn_list);
        }

        s3_connnection_request_done(r);
        s3_request_destruct(r);

        s3_message_try_destruct(m);
    }

    s3_list_for_each_entry_safe(c, dummy_conn, &write_conn_list, write_list_node) {
        s3_list_del(&c->write_list_node);
        ret = s3_connection_write_socket(c);
        if (c->closed) {
            (c->handler->close)(c);
            continue; // would not send resp when socket is half close
        }
        if (ret == S3_ERR_NET_ABORT) {
            log_info("conn write error, try destruct conn.");
            (c->handler->close)(c);
            continue;
        }
    }
}

/*
 * return:
 *     S3_OK
 *     or S3_ERR_NET_AGAIN
 *     or S3_ERR_NET_ABORT
 */
static int s3_connection_write_socket(S3Connection *c) {
    if (s3_list_empty(&c->output_buf_list)) {
        return S3_OK;
    }

    int ret = S3_OK;
    ret = s3_socket_write(c->fd, &c->output_buf_list);
    log_debug("socket write, ret=%d", ret);
    if (ret < 0 && ret != S3_ERR_NET_AGAIN) {
        log_error("socket write fail, ret=%d", ret);
        return S3_ERR_NET_ABORT;
    }

    /*
     * 1. output_buf_list not write complete;
     */
    return  s3_connection_write_socket_again(c);
}

static int s3_connection_write_socket_again(S3Connection *c) {
    if (s3_list_empty(&c->output_buf_list)) {
        return S3_OK;
    }

    ev_io_start(c->loop, &c->write_watcher);
    return S3_ERR_NET_AGAIN;
}

void s3_connection_try_destruct(S3Connection *c) {
    ev_io_stop(c->loop, &c->read_watcher);
    ev_io_stop(c->loop, &c->write_watcher);

    /*
     * 还有未完成的request: worker thread正在处理的request
     */
    if (c->request_doing_cnt > 0) {
        return;
    }

    log_info("try destruct conn succ.");

    s3_list_del(&c->conn_list_node);
    s3_connection_destruct(c);
}
