#include "lib/s3_io.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "third/logc/log.h"
#include "lib/s3_connection.h"
#include "lib/s3_error.h"
#include "lib/s3_malloc.h"
#include "lib/s3_message.h"
#include "lib/s3_packet.h"
#include "lib/s3_pthread.h"
#include "lib/s3_socket.h"

#define S3_IOTH_WITH_REQUEST(r) ((S3IOThread*)((S3Connection*)((S3Message*)r->message)->conn)->ioth)

static void s3_io_thread_destroy(S3IOThread *ioth);

/**********************************s3 io***********************************/
S3IO *s3_io_construct() {
    S3IO *s3io = s3_malloc_(S3_MOD_S3IO, sizeof(S3IO));
    if (s3io != NULL) {
        *s3io = (S3IO)s3_io_null;
    }
    return s3io;
}

void s3_io_desconstruct(S3IO *s3io) {
    if (s3io != NULL) {
        s3_io_destroy(s3io);
        s3_free_(S3_MOD_S3IO, s3io);
    }
}

int s3_io_init(S3IO *s3io, int io_thread_cnt) {
    assert(io_thread_cnt <= MAX_IO_THREAD_CNT);
    s3io->io_thread_cnt = io_thread_cnt;
    return 0;
}

void s3_io_destroy(S3IO *s3io) {
    s3_io_listen_destroy(s3io->listen);
    for (int i = 0; i < s3io->io_thread_cnt; ++i) {
        s3_io_thread_destroy(s3io->ioths + i);
    }
}


/*********************************io routine**********************************/

static void s3_pipe_recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    S3IOThread *ioth = (S3IOThread*)w->data;

    int socket_fd;
    int ret = read(w->fd, &socket_fd, sizeof(int));
    assert(ret == sizeof(int));
    log_info("read socket fd=%d, dispatch io thread id=%d", socket_fd, ioth->id);

    S3Connection *conn = s3_connection_construct();
    s3_connection_init(conn, ioth->loop, socket_fd);
    conn->handler = ioth->handler;
    conn->ioth = ioth;
    s3_list_add_tail(&conn->conn_list_node, &ioth->conn_list);
    ioth->conn_cnt++;

    conn->read_watcher.data = conn;
    ev_io_init(&conn->read_watcher, s3_connection_recv_socket_cb, socket_fd, EV_READ);

    conn->write_watcher.data = conn;
    ev_io_init(&conn->write_watcher, s3_connection_write_socket_cb, socket_fd, EV_WRITE);

    ev_io_start(loop, &conn->read_watcher);
}

static void s3_io_thread_wakeup_cb(struct ev_loop *loop, ev_io *w, int revents) {
    S3IOThread *ioth = (S3IOThread*)w->data;
    S3ListHead request_list;

    log_debug("ioth wakeup, id=%d", ioth->id);

    pthread_spin_lock(&ioth->pthread_lock);
    s3_list_movelist(&ioth->request_list, &request_list);
    pthread_spin_unlock(&ioth->pthread_lock);

    s3_connnection_resp_request(&request_list);

    log_debug("request list left cnt=%d", s3_list_length(&request_list));
    //sleep(1);

    /*
     * NOTICE: 剩下的request_list放回ioth->request_list
     */
    if (!s3_list_empty(&request_list)) {
        pthread_spin_lock(&ioth->pthread_lock);
        s3_list_join(&request_list, &ioth->request_list); // FIXME: 是否前插到ioth->request_list
        pthread_spin_unlock(&ioth->pthread_lock);

        // TODO: 验证thread_watcher 同时收到多个async send，只会唤醒一次
        ev_async_send(ioth->loop, &ioth->thread_watcher);
    }
}

static void s3_io_thread_stop_cb(struct ev_loop *loop, ev_io *w, int revents) {
    ev_break(loop, EVBREAK_ONE);
}

static int s3_io_thread_init(S3IOThread *ioth, int id, S3IOHandler *handler) {
    ioth->id = id;
    ioth->handler = handler;
    pthread_spin_init(&ioth->pthread_lock, PTHREAD_PROCESS_PRIVATE);
    s3_list_init(&ioth->conn_list);
    s3_list_init(&ioth->request_list);

    int ret = pipe(ioth->pipefd);
    assert(ret == 0);

    ioth->loop = ev_loop_new(EVBACKEND_EPOLL);
    assert(ioth->loop != NULL);

    ioth->pipe_read_watcher.data = ioth;
    ev_io_init(&ioth->pipe_read_watcher, s3_pipe_recv_socket_cb, ioth->pipefd[0], EV_READ);
    ev_io_start(ioth->loop, &ioth->pipe_read_watcher);

    ioth->thread_watcher.data = ioth;
    ev_async_init(&ioth->thread_watcher, s3_io_thread_wakeup_cb);
    ev_async_start(ioth->loop, &ioth->thread_watcher);

    ev_async_init(&ioth->stop_watcher, s3_io_thread_stop_cb);
    ev_async_start(ioth->loop, &ioth->stop_watcher);

    return 0;
}

static void s3_io_thread_destroy(S3IOThread *ioth) {
    S3Connection *conn = NULL, *dummy;
    s3_list_for_each_entry_safe(conn, dummy, &ioth->conn_list, conn_list_node) {
        s3_list_del(&conn->conn_list_node);
        s3_connection_destruct(conn);
    }

    // make sure work thread has exited
    S3Request *r, *dummy_request;
    s3_list_for_each_entry_safe(r, dummy_request, &ioth->request_list, request_list_node) {
        s3_request_destruct(r);
    }

    ev_io_stop(ioth->loop, &ioth->pipe_read_watcher);
    close(ioth->pipefd[0]);
    close(ioth->pipefd[1]);

    ev_async_stop(ioth->loop, &ioth->thread_watcher);

    ev_async_send(ioth->loop, &ioth->stop_watcher);
    int ret = s3_pthread_join(ioth->tid, NULL);
    log_info("-------------------------io thread exit. id=%d, ret=%d", ioth->id, ret);
    ev_async_stop(ioth->loop, &ioth->stop_watcher);

    ev_loop_destroy(ioth->loop);

    pthread_spin_destroy(&ioth->pthread_lock);
}

static void *s3_io_thread_start_rontine(void *arg) {
    S3IOThread *ioth = (S3IOThread*)arg;

    int ret = ev_run(ioth->loop, 0);
    log_info("io thread ev run over, id=%d, ret=%d", ioth->id, ret);

    return NULL;
}

S3IO *s3_io_create(int io_thread_cnt, S3IOHandler *handler) {
    int ret = 0;
    S3IO *s3io = NULL;

    s3io = s3_io_construct();
    assert(s3io != NULL);
    ret = s3_io_init(s3io, io_thread_cnt);
    assert(ret == 0);

    for (int i = 0; i < s3io->io_thread_cnt; ++i) {
        S3IOThread *ioth = &s3io->ioths[i];
        s3_io_thread_init(ioth, i, handler);
        s3_pthread_create_joinable(&ioth->tid, s3_io_thread_start_rontine, (void*)ioth);
    }
    usleep(100);

    return s3io;
}

void s3_io_thread_add_resp_request(S3Request *r) {
    S3IOThread *ioth = S3_IOTH_WITH_REQUEST(r);

    pthread_spin_lock(&ioth->pthread_lock);
    s3_list_add_tail(&r->request_list_node, &ioth->request_list);
    pthread_spin_unlock(&ioth->pthread_lock);

    ev_async_send(ioth->loop, &ioth->thread_watcher);
}

/*********************************listen**********************************/

static void s3_accept_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    int fd;
    int listenfd = w->fd;
    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(struct sockaddr);
    do {
        fd = accept(listenfd, (struct sockaddr *)&sin, &addrlen);
        if (fd >= 0) {
            break;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
        }
        log_error("accept fail. ret=%d", fd);
        assert(false);
    } while (1);

    S3IO *s3io = (S3IO*)w->data;
    S3IOThread *ioth = &s3io->ioths[s3io->recved_conn_cnt++ % s3io->io_thread_cnt];
    int ret = write(ioth->pipefd[1], &fd, sizeof(fd));
    assert(ret == sizeof(fd));
}

static void s3_listen_thread_stop_cb(struct ev_loop *loop, ev_io *w, int revents) {
    ev_break(loop, EVBREAK_ONE);
}

static int s3_listen_init(S3Listen *l, S3IO *s3io) {
    l->listen_loop = EV_DEFAULT;

    l->listenfd = s3_socket_create_listenfd();
    assert(l->listenfd >= 0);

    l->lwatcher.data = s3io;
    ev_io_init(&l->lwatcher, s3_accept_socket_cb, l->listenfd, EV_READ);
    ev_io_start(l->listen_loop, &l->lwatcher);

    ev_async_init(&l->stop_watcher, s3_listen_thread_stop_cb);
    ev_async_start(l->listen_loop, &l->stop_watcher);

    return S3_OK;
}

void s3_io_listen_destroy(S3Listen *l) {
    ev_io_stop(l->listen_loop, &l->lwatcher);
    close(l->listenfd);

    ev_async_send(l->listen_loop, &l->stop_watcher);
    int ret = s3_pthread_join(l->tid, NULL);
    log_info("listen thread exit. ret=%d", ret);
    ev_async_stop(l->listen_loop, &l->stop_watcher);

    ev_loop_destroy(l->listen_loop);
    return;
}

static void *s3_listen_thread_start_rontine(void *arg) {
    S3Listen *l = (S3Listen*)arg;

    int ret = ev_run(l->listen_loop, 0);
    log_info("listen thread ev run over, ret=%d", ret);

    return NULL;
}

void s3_io_start_run(S3IO *s3io) {
    s3_listen_init(&s3io->listen, s3io);

    s3_pthread_create_joinable(&s3io->listen.tid, s3_listen_thread_start_rontine, (void*)&s3io->listen);
    log_info("create listen routine, tid=%lu", s3io->listen.tid);
}

void s3_io_connection_close(void *conn) {
    S3Connection *c = conn;
    if (c->closed == 0) {
        S3IOThread *ioth = c->ioth;
        ioth->conn_cnt--;
        c->closed = 1;
    }

    s3_connection_try_destruct(c);
}

void s3_io_print_stat(S3IO *s3io) {
    uint64_t conn_total_cnt = 0;
    uint64_t msg_total_cnt = 0;
    uint64_t req_total_cnt = 0;
    uint64_t req_doing_cnt = 0;
    for (int i = 0; i < s3io->io_thread_cnt; ++i) {
        S3IOThread *ioth = s3io->ioths + i;
        S3Connection *c;
        s3_list_for_each_entry(c, &ioth->conn_list, conn_list_node) {
            conn_total_cnt++;
            msg_total_cnt += c->message_total_cnt;
            req_total_cnt += c->request_total_cnt;
            req_doing_cnt += c->request_doing_cnt;
        }
    }
    log_info("active connection cnt=%ld", conn_total_cnt);
    log_info("total message cnt=%ld", msg_total_cnt);
    log_info("total request cnt=%ld", req_total_cnt);
    log_info("doing request cnt=%ld", req_doing_cnt);
}
