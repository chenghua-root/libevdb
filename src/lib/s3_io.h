#ifndef S3_LIB_IO_H_
#define S3_LIB_IO_H_

#include <ev.h>
#include <stdint.h>
#include "lib/s3_list.h"
#include "lib/s3_request.h"
#include "lib/s3_connection.h"
#include "lib/s3_io_handler.h"

#define MAX_IO_THREAD_CNT 8

typedef struct S3Listen S3Listen;
struct S3Listen {
    pthread_t           tid;

    int                 listenfd;
    struct ev_loop      *listen_loop; // DV_DEFAULT, should not destroy
    ev_io               lwatcher;
    ev_async            stop_watcher;
};
void s3_io_listen_destroy();


typedef struct S3IOThread S3IOThread;
struct S3IOThread {
    pthread_t             tid;

    int                   id;
    S3IOHandler           *handler;
    pthread_spinlock_t    pthread_lock;
    S3List                conn_list;
    uint64_t              conn_cnt;
    S3List                request_list;

    struct ev_loop        *loop;
    int                   pipefd[2];
    ev_io                 pipe_read_watcher; // listen线程把新连接fd通过pipe发送到此watcher
    ev_async              thread_watcher;
    ev_async              stop_watcher;
};

typedef struct S3IO     S3IO;
struct S3IO {
    S3Listen            listen;
    uint64_t            recved_conn_cnt;

    int                 io_thread_cnt;
    S3IOThread          ioths[MAX_IO_THREAD_CNT];
};
#define s3_io_null {      \
    .io_thread_cnt = 0,   \
    .recved_conn_cnt = 0, \
    .ioths = {0},         \
}

S3IO *s3_io_construct();
void s3_io_desconstruct(S3IO *s3io);
int s3_io_init(S3IO *s3io, int io_thread_cnt);
void s3_io_destroy(S3IO *s3io);

S3IO *s3_io_create(int io_thread_cnt, S3IOHandler *handler);
void s3_io_start_run(S3IO *s3io);

void s3_io_thread_add_resp_request(S3Request *r);

void s3_io_connection_close(void *c);

/*
 * NOTICE: not safe
 */
void s3_io_print_stat(S3IO *s3io);

#endif
