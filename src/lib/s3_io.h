#ifndef S3_LIB_IO_H_
#define S3_LIB_IO_H_

#include <ev.h>
#include <stdint.h>
#include "lib/s3_list.h"
#include "lib/s3_message.h"

#define MAX_IO_THREAD_CNT 8

typedef void *(S3IOThreadStartRoutine) (void *);

typedef struct S3IOThread S3IOThread;
struct S3IOThread {
    pthread_t              tid;
    int                    id;
    S3List                 conn_list;
    struct ev_loop         *loop;
    int                    pipefd[2];
    ev_io                  pipe_read_watcher; // listen线程把新连接fd通过pipe发送到此watcher
    uint64_t               conn_cnt;
};

void s3_io_thread_destroy();

typedef struct S3Listen S3Listen;
struct S3Listen {
    int                 listenfd;
    struct ev_loop      *listen_loop; // DV_DEFAULT, should not destroy
    ev_io               lwatcher;
};

void s3_io_listen_destroy();

typedef struct S3IO     S3IO;
struct S3IO {
    int                 io_thread_cnt;
    S3IOThread          ioths[MAX_IO_THREAD_CNT];
    S3Listen            listen;
    uint64_t            conn_cnt;
};
#define s3_io_null {    \
    .io_thread_cnt = 0, \
    .ioths = {0},       \
}

S3IO *s3_io_construct();
void s3_io_desconstruct(S3IO *s3io);
int s3_io_init(S3IO *s3io, int io_thread_cnt);
void s3_io_destroy(S3IO *s3io);

S3IO *s3_io_create();
void s3_io_start_run(S3IO *s3io);

/*
 * 调用方可以定义或者选择网络IO处理动作(如接收，发送，编码，解码，处理等)，
 * 通过handler传递给网络层
 */
typedef struct S3IOHandler      S3IOHandler;
struct S3IOHandler {
  void                         *(*decode)(S3Message *m);
};

#endif
