#include "lib/s3_io.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/s3_pthread.h"
#include "lib/s3_socket.h"
#include "lib/s3_packet.h"
#include "lib/s3_connection.h"
#include "lib/s3_message.h"
#include "third/logc/log.h"

#define S3_IOTH_WITH_REQUEST(r) ((S3IOThread*)((S3Connection*)((S3Message*)r->message)->conn)->ioth)

/**********************************s3 io***********************************/
S3IO *s3_io_construct() {
    S3IO *s3io = malloc(sizeof(S3IO));
    if (s3io != NULL) {
        *s3io = (S3IO)s3_io_null;
    }
    return s3io;
}

void s3_io_desconstruct(S3IO *s3io) {
    if (s3io != NULL) {
        s3_io_destroy(s3io);
        free(s3io);
    }
}

int s3_io_init(S3IO *s3io, int io_thread_cnt) {
    assert(io_thread_cnt <= MAX_IO_THREAD_CNT);
    s3io->io_thread_cnt = io_thread_cnt;
    return 0;
}

void s3_io_destroy(S3IO *s3io) {
    s3_io_listen_destroy(s3io->listen);
    s3_io_thread_destroy(s3io->ioths);
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

    ev_io_init(&conn->read_watcher, s3_connection_recv_socket_cb_v2, socket_fd, EV_READ);
    ev_io_init(&conn->write_watcher, s3_connection_write_socket_cb, socket_fd, EV_WRITE);

    ev_io_start(loop, &conn->read_watcher);
}

static void s3_io_thread_wakeup_cb(struct ev_loop *loop, ev_io *w, int revents) {
    S3IOThread *ioth = (S3IOThread*)w->data;
    S3ListHead request_list;

    log_debug("ioth wakeup, tid=%d", ioth->tid);

    pthread_spin_lock(&ioth->pthread_lock);
    s3_list_movelist(&ioth->request_list, &request_list);
    pthread_spin_unlock(&ioth->pthread_lock);

    s3_connnection_send_resp(&request_list);
    // TODO 剩下的request_list放回ioth->request_list
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

    return 0;
}

void s3_io_thread_destroy(S3IOThread *ioth) {
    S3Connection *conn = NULL;
    s3_list_for_each_entry(conn, &ioth->conn_list, conn_list_node) {
        s3_connection_destruct(conn);
    }

    ev_io_stop(ioth->loop, &ioth->pipe_read_watcher);
    close(ioth->pipefd[0]);
    close(ioth->pipefd[1]);

    ev_loop_destroy(ioth->loop);
}

static void *s3_io_thread_start_rontine(void *arg) {
    S3IOThread *ioth = (S3IOThread*)arg;

    int ret = ev_run(ioth->loop, 0);
    log_info("io thread ev run over, thread id=%d, ret=%d", ioth->id, ret);

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
    S3IOThread *ioth = &s3io->ioths[s3io->conn_cnt++%s3io->io_thread_cnt];
    int ret = write(ioth->pipefd[1], &fd, sizeof(fd));
    assert(ret == sizeof(fd));
}

static int s3_listen_init(S3Listen *l, S3IO *s3io) {
    l->listen_loop = EV_DEFAULT;

    l->listenfd = s3_socket_create_listenfd();
    assert(l->listenfd >= 0);

    l->lwatcher.data = s3io;
    ev_io_init(&l->lwatcher, s3_accept_socket_cb, l->listenfd, EV_READ);
    ev_io_start(l->listen_loop, &l->lwatcher);
}

void s3_io_listen_destroy(S3Listen *l) {
    ev_io_stop(l->listen_loop, &l->lwatcher);
    close(l->listenfd);
}

static void *s3_listen_thread_start_rontine(void *arg) {
    S3Listen *l = (S3Listen*)arg;

    ev_run(l->listen_loop, 0);

    return NULL;
}

void s3_io_start_run(S3IO *s3io) {
    s3_listen_init(&s3io->listen, s3io);

    pthread_t tid;
    s3_pthread_create_joinable(&tid, s3_listen_thread_start_rontine, (void*)&s3io->listen);
    usleep(100);
    log_info("create listen routine, tid=%ld", (long)tid);
}
