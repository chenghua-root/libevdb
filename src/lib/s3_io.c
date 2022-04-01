#include "lib/s3_io.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h> // pipe()
#include "lib/s3_connection.h"
#include "lib/s3_pthread.h"
#include "lib/s3_socket.h"
#include "third/logc/log.h"

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
    s3io->io_thread_cnt = 0;
}

static void s3_ioth_recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    S3IOThread *ioth = (S3IOThread*)w->data;

    int socket_fd;
    int ret = read(w->fd, &socket_fd, sizeof(int));
    assert(ret == sizeof(int));
    log_info("read socket fd=%d, dispatch io thread id=%d\n", socket_fd, ioth->id);

    S3Connection *conn = s3_connection_construct();
    s3_connection_init(conn, ioth->loop, socket_fd);
    s3_list_add(&ioth->conn_list, &conn->list_node);
    ioth->conn_cnt++;

    ev_io_init(&conn->read_watcher, s3_connection_recv_socket_cb, socket_fd, EV_READ);
    ev_io_init(&conn->write_watcher, s3_connection_write_socket_cb, socket_fd, EV_WRITE);

    ev_io_start(loop, &conn->read_watcher);
}

static int s3_io_thread_init(S3IOThread *ioth, int id) {
    ioth->id = id;
    s3_list_init(&ioth->conn_list);

    int ret = pipe(ioth->pipefd);
    assert(ret == 0);

    ioth->loop = ev_loop_new(EVBACKEND_EPOLL);
    assert(ioth->loop != NULL);

    ioth->pipe_read_watcher.data = ioth;
    ev_io_init(&ioth->pipe_read_watcher, s3_ioth_recv_socket_cb, ioth->pipefd[0], EV_READ);
    ev_io_start(ioth->loop, &ioth->pipe_read_watcher);

    return 0;
}

static void *s3_io_thread_start_rontine(void *arg) {
    S3IOThread *ioth = (S3IOThread*)arg;

    int ret = ev_run(ioth->loop, 0);
    log_info("io thread ev run over, thread id=%d, ret=%d", ioth->id, ret);

    return NULL;
}

S3IO *s3_io_create() {
    int ret = 0;
    S3IO *s3io = NULL;

    s3io = s3_io_construct();
    assert(s3io != NULL);
    ret = s3_io_init(s3io, 2); // io thread cnt
    assert(ret == 0);

    for (int i = 0; i < s3io->io_thread_cnt; ++i) {
        S3IOThread *ioth = &s3io->ioths[i];
        s3_io_thread_init(ioth, i);
        s3_pthread_create_joinable(&ioth->tid, s3_io_thread_start_rontine, (void*)ioth);
    }
    usleep(200);

    return s3io;
}

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
        log_error("accept fail. ret=%d\n", fd);
        assert(0);
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
