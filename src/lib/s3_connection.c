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

#include "lib/s3_packet_header.pb-c.h"

#define PORT 9000
#define IP "127.0.0.1"

#define EV_IO_LOOP_NUM 4
pthread_t io_threads[EV_IO_LOOP_NUM];
struct ev_loop *io_loops[EV_IO_LOOP_NUM] = {NULL};
int64_t accept_cnt = 0;

S3Connection *s3_connection_construct() {
    S3Connection *conn = malloc(sizeof(S3Connection));
    if (conn != NULL) {
        *conn = (S3Connection)s3_connection_null;
    }
    return conn;
}

void s3_connection_desconstruct(S3Connection *conn) {
    if (conn != NULL) {
        s3_connection_destroy(conn);
        free(conn);
    }
}

int s3_connection_init(S3Connection *conn, struct ev_loop *loop, int fd,
                       ev_io read_watcher, ev_io write_watcher) {
    conn->loop          = loop;
    conn->fd            = fd;
    conn->read_watcher  = read_watcher;
    conn->write_watcher = write_watcher;
}

void s3_connection_destroy(S3Connection *conn) {
    if (conn != NULL) {
        close(conn->fd);
        ev_io_stop(conn->loop, &conn->read_watcher);
        ev_io_stop(conn->loop, &conn->write_watcher);
    }
}


static void s3_connection_create_multi_io_loop();
static void *s3_connection_do_create_io_loop(void *arg);
static void s3_connection_ev_loop_timer_hook(EV_P_ ev_timer *w, int revents);

static int s3_connection_create_socket();
static void s3_connection_accept_socket_cb(struct ev_loop *loop, ev_io *w, int revents);
static void s3_connection_recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents);
static void s3_connection_write_socket_cb(struct ev_loop *loop, ev_io *w, int revents);

int s3_connection_create_listen_and_io_loop(struct ev_loop *loop) {
    int listenfd = 0;
    ev_io lwatcher;

    s3_connection_create_multi_io_loop();

    listenfd = s3_connection_create_socket();
    if (listenfd < 0) {
        perror("create listenfd fail\n");
        return -1;
    }

    ev_io_init(&lwatcher, s3_connection_accept_socket_cb, listenfd, EV_READ);
    ev_io_start(loop, &lwatcher);

    ev_run(loop, 0);

    return 0;
}

void s3_connection_loop_run(struct ev_loop *loop) {
    ev_run(loop, 0);
}

static void s3_connection_create_multi_io_loop() {
    int ret = 0;
    for (int i = 0; i < EV_IO_LOOP_NUM; ++i) {
        uint64_t j = i;
        ret = pthread_create(&io_threads[i], NULL, s3_connection_do_create_io_loop, (void*)j);
        if (ret != 0) { // TODO: create fail
            printf("pthread create fail, ret=%d, errno=%d", ret, errno);
            break;
        }
    }
    usleep(500);
}

static void *s3_connection_do_create_io_loop(void *arg) {
    int64_t idx = (int64_t) arg;
    if (idx < 0 || idx >= EV_IO_LOOP_NUM) {
        printf("loop idx invalid, idx=%d\n", idx);
        return NULL;
    }

    struct ev_loop *loop = ev_loop_new(EVBACKEND_EPOLL);
    if (loop == NULL) {
        perror("new ev loop fail.");
        return NULL;
    }
    io_loops[idx] = loop;
    printf("create io ev loop, idx=%d, loop=%ld\n", idx, (uint64_t*)loop);

    ev_timer wtimer;
    ev_timer_init(&wtimer, s3_connection_ev_loop_timer_hook, 0., 10.);
    ev_timer_again(loop, &wtimer);

    /*
     * FIXME:
     *  before start run, at least regist one watcher.
     *  if regist a timer watcher, would WAIT the timer watcher callback,
     *  and then the io watcher would be work.
     *
     */
    int ret = ev_run(loop, 0);
    printf("io loop run over, loop idx=%d, ret=%d\n", idx, ret);
    ev_timer_stop(loop, &wtimer);
    ev_loop_destroy(loop);
}

static void s3_connection_ev_loop_timer_hook(EV_P_ ev_timer *w, int revents) {
    (void) w;
    (void) revents;
    (void) loop;
    printf("----------------------ev timer. revents=%d\n", revents); // what is revents?
}


static int s3_connection_create_socket() {
    struct sockaddr_in addr;
    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);

    if(s == -1){
        perror("create socket error \n");
        return -1;
    }

    int so_reuseaddr = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
    bzero(&addr, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);

    if(bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1){
        perror("bind socket error \n");
        return -1;
    }


    if(listen(s, 32) == -1){
        perror("listen socket error\n");
        return -1;
    }

    printf("bind %s, listen %d\n", IP, PORT);

    return s;
}

static void s3_connection_accept_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    int fd;
    int s = w->fd;
    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(struct sockaddr);
    do {
        fd = accept(s, (struct sockaddr *)&sin, &addrlen);
        if(fd > 0) {
            break;
        }

        if(errno == EAGAIN || errno == EWOULDBLOCK){
            continue;
        }
    } while(1);

    struct ev_loop *io_loop = io_loops[accept_cnt++ % EV_IO_LOOP_NUM]; // FIXME: accept_cnt atomic
    printf("accept_cnt=%ld, io_loop=%ld\n", accept_cnt, (uint64_t*)io_loop);

    ev_io *accept_watcher = malloc(sizeof(ev_io));
    memset(accept_watcher, 0x00, sizeof(ev_io));

    ev_io_init(accept_watcher, s3_connection_recv_socket_cb, fd, EV_READ);
    ev_io_start(io_loop, accept_watcher);
}

#define MAX_BUF_LEN 1024
static void s3_connection_recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    char buf[MAX_BUF_LEN] = {0};
    int ret = 0;

    do {
        ret = recv(w->fd, buf, MAX_BUF_LEN - 1, 0);

        if (ret > 0) {
            /*
             * FIXME:
             *   解包，必须提供长度参数。但同一个protobuf结构，成员值不同时，其长度不一.
             */
            S3PacketHeader *header = s3_packet_header__unpack(NULL, ret, buf);

            printf("recv len=%d message: header.pcode=%d, .session_id=%ld, .data_len=%ld\n",
                    ret,
                    header->pcode,
                    header->session_id,
                    header->data_len);

            s3_packet_header__free_unpacked(header, NULL); // 释放空间

            ev_io_stop(loop, w);
            ev_io_init(w, s3_connection_write_socket_cb, w->fd, EV_WRITE);
            ev_io_start(loop, w);

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
    free(w);
}

static void s3_connection_write_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    char buf[MAX_BUF_LEN] = {0};

    snprintf(buf, MAX_BUF_LEN - 1, "this is test message from libev\n");

    send(w->fd, buf, strlen(buf), 0);

    ev_io_stop(loop, w);
    ev_io_init(w, s3_connection_recv_socket_cb, w->fd, EV_READ);
    ev_io_start(loop, w);
}
