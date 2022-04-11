#include "lib/s3_connection.h"

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
#include "lib/s3_message.h"
#include "lib/s3_net_code.h"
#include "lib/s3_packet.h"
#include "lib/s3_packet_header.pb-c.h"
#include "lib/s3_request.h"
#include "lib/s3_socket.h"

S3Connection *s3_connection_construct() {
    S3Connection *conn = malloc(sizeof(S3Connection));
    if (conn != NULL) {
        *conn = (S3Connection)s3_connection_null;
    }
    return conn;
}

void s3_connection_destruct(S3Connection *conn) {
    if (conn != NULL) {
        s3_connection_destroy(conn);
        free(conn);
    }
}

int s3_connection_init(S3Connection *conn, struct ev_loop *loop, int fd) {
    conn->loop          = loop;
    conn->fd            = fd;
    s3_list_head_init(&conn->conn_list_node);
    s3_list_init(&conn->message_list);
    conn->read_watcher.data = conn;
    conn->write_watcher.data = conn;
}

void s3_connection_destroy(S3Connection *conn) {
    if (conn != NULL) {
        close(conn->fd);
        ev_io_stop(conn->loop, &conn->read_watcher);
        ev_io_stop(conn->loop, &conn->write_watcher);
    }
}

static int s3_connection_write_socket(S3Connection *c) {
    // TODO
    return 0;
}

static int s3_connection_process_request(S3Connection *c, S3List *request_list) {
    S3List req_list;
    s3_list_movelist(request_list, &req_list);

    int ret = 0;
    S3Request *r, *rn;
    s3_list_for_each_entry_safe(r, rn, &req_list, request_list_node) {
        log_info("process a request");
        s3_list_del(&r->request_list_node);
        ret = (c->handler->process)(r);
        assert(ret == 0);
    }
    ret = s3_connection_write_socket(c);
}

static int s3_connection_process_message(S3Connection *c, S3Message *m) {
    while (s3_buf_unconsumed_size(m->in_buf) > 0) {
        S3Packet *packet = s3_net_decode(m);
        if (packet == NULL) {
            break;
        }
        S3Request *r = s3_request_construct();
        assert(r != NULL);
        r->in_packet = packet;
        s3_list_add_tail(&r->request_list_node, &m->request_list);
        c->request_doing_cnt++;  // TODO atomic
        c->request_total_cnt++;
        log_info("decode a request, session_id=%ld", packet->header.session_id);
    }
    s3_connection_process_request(c, &m->request_list);
}

void s3_connection_recv_socket_cb_v2(struct ev_loop *loop, ev_io *w, int revents) {
    S3Connection *c = (S3Connection*)w->data;
    if (c->request_doing_cnt >= S3_CONN_DOING_MAX_REQ_CNT) {
        log_error("connection doing req count exceed, doing cnt=%ld", c->request_doing_cnt);
        // TODO: destroy connection
        return;
    }

    S3Message *m = s3_list_get_last(&c->message_list, S3Message, message_list_node);
    S3Message *old_msg = NULL;
    if (m == NULL) {
        m = s3_message_create();
        assert(m != NULL);
        s3_list_add_tail(&m->message_list_node, &c->message_list);
    } else if (m->read_status == S3_MSG_READ_STATUS_AGAIN &&
               m->next_read_len > s3_buf_free_size(m->in_buf)) {
        old_msg = m;
        old_msg->read_status = S3_MSG_READ_STATUS_DONE;
        m = s3_message_create_with_old(old_msg);
        assert(m != NULL);
        s3_list_add_tail(&m->message_list_node, &c->message_list);
        log_error("unexpect one");
    } else if (s3_buf_free_size(m->in_buf) < S3_MSG_BUF_MIN_LEN) {
        old_msg = m;
        old_msg->read_status = S3_MSG_READ_STATUS_DONE;
        m = s3_message_create();
        assert(m != NULL);
        s3_list_add_tail(&m->message_list_node, &c->message_list);
        log_error("unexpect two");
    }

    if (old_msg != NULL) {
        s3_message_try_destroy(old_msg);
    }

    int n = s3_socket_read(w->fd, m->in_buf->right, s3_buf_free_size(m->in_buf));
    if (n < 0) {
        if (n == S3_ERR_NET_AGAIN) {
            return;
        }
        assert(false);
    } else if (n == 0) {
        close(w->fd);
    }
    m->in_buf->right += n;
    log_info("recv data, len=%d", n);

    int ret = s3_connection_process_message(c, m);
    assert(ret == 0);
}

#define MAX_BUF_LEN 1024
void s3_connection_recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    char buf[MAX_BUF_LEN] = {0};
    int ret = 0;
    S3Connection *conn = (S3Connection*)w->data;

    do {
        ret = recv(w->fd, buf, MAX_BUF_LEN - 1, 0);
        if (ret > 0) {
            // FIXME:
            //    解包，必须提供长度参数。但同一个protobuf结构，成员值不同时，其长度不一.
            //    替换为其它数据交换格式
            S3PacketHeader *header = s3_packet_header__unpack(NULL, ret, buf);
            printf("recv len=%d message: header.pcode=%d, .session_id=%ld, .data_len=%ld\n",
                    ret,
                    header->pcode,
                    header->session_id,
                    header->data_len);
            s3_packet_header__free_unpacked(header, NULL); // 释放空间

            ev_io_start(loop, &conn->write_watcher);
            return;
        }

        if (ret == 0) {
            printf("remote socket closed\n");
            break;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
        }

        break;
    } while(1);

    close(w->fd);
    ev_io_stop(loop, w);
    s3_connection_destruct(conn);
}

void s3_connection_write_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    char buf[MAX_BUF_LEN] = {0};

    snprintf(buf, MAX_BUF_LEN - 1, "this is test message from libev\n");
    send(w->fd, buf, strlen(buf), 0);
    ev_io_stop(loop, w);
}
