#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "lib/s3_connection.h"
#include "lib/s3_packet_header.pb-c.h"

#define PORT 9000
#define IP "127.0.0.1"

#define EV_IO_LOOP_NUM 4
struct ev_loop *io_loops[EV_IO_LOOP_NUM] = {NULL};
int64_t accept_cnt = 0;

void ev_loop_multi_create();
void* ev_loop_do_create(void *arg);
int  create_socket();
void accept_socket_cb(struct ev_loop *loop, ev_io *w, int revents);
void recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents);
void write_socket_cb(struct ev_loop *loop, ev_io *w, int revents);

int main(int argc, char *argv[]) {
    ev_loop_multi_create();

    int s = create_socket();
    if (s < 0) {
        return -1;
    }

    struct ev_loop *loop = EV_DEFAULT;
    ev_io ev_listen_watcher;

    ev_io_init(&ev_listen_watcher, accept_socket_cb, s, EV_READ);
    ev_io_start(loop, &ev_listen_watcher);

    ev_run(loop, 0);

    return 0;
}

void ev_loop_multi_create() {
    pthread_t threads[EV_IO_LOOP_NUM];
    int ret = 0;
    for (int i = 0; i < EV_IO_LOOP_NUM; ++i) {
        uint64_t j = i;
        ret = pthread_create(&threads[i], NULL, ev_loop_do_create, (void*)j);
        if (ret != 0) {
            printf("pthread create fail, ret=%d, errno=%d", ret, errno);
            break;
        }
    }
    usleep(500);
}

static void repeate_hook(EV_P_ ev_timer *w, int revents) {
    (void) w;
    (void) revents;
    (void) loop;
    printf("----------------------repeate revents=%d\n", revents); // what is revents?
}

void* ev_loop_do_create(void *arg) {
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
    ev_timer_init(&wtimer, repeate_hook, 3., 20.);
    ev_timer_again(loop, &wtimer);

    /*
     * FIXME:
     *  before start run, at least regist one watcher.
     *  if regist a timer watcher, would WAIT the timer watcher callback,
     *  and then the io watcher would be work.
     *
     */
    int ret = ev_run(loop, 0);
    printf("io ev loop run over, ret=%d, errno=%d\n", ret, errno);
    ev_timer_stop(loop, &wtimer);
    ev_loop_destroy(loop);
}

int create_socket() {
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

void accept_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
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
    printf("accept_cnt=%d, io_loop=%ld\n", accept_cnt, (uint64_t*)io_loop);

    ev_io *accept_watcher = malloc(sizeof(ev_io));
    memset(accept_watcher, 0x00, sizeof(ev_io));

    ev_io_init(accept_watcher, recv_socket_cb, fd, EV_READ);
    ev_io_start(io_loop, accept_watcher);
}

#define MAX_BUF_LEN 1024
void recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
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
            ev_io_init(w, write_socket_cb, w->fd, EV_WRITE);
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

void write_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    char buf[MAX_BUF_LEN] = {0};

    snprintf(buf, MAX_BUF_LEN - 1, "this is test message from libev\n");

    send(w->fd, buf, strlen(buf), 0);

    ev_io_stop(loop, w);
    ev_io_init(w, recv_socket_cb, w->fd, EV_READ);
    ev_io_start(loop, w);
}
