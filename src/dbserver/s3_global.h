#ifndef S3_GLOBAL_H_
#define S3_GLOBAL_H_

#include "lib/s3_io.h"
#include "lib/s3_io_handler.h"
#include "lib/s3_threads_queue.h"

typedef struct S3Global S3Global;
struct S3Global {
    S3IO                *s3io;
    S3IOHandler         io_handler;

    S3ThreadsQueue      *cmpt_workers;

    int                 stop_flag;
};
#define s3_global_null {  \
    .s3io = NULL,         \
    .cmpt_workers = NULL, \
    .stop_flag = 0,       \
}

extern S3Global s3_g;

#endif
