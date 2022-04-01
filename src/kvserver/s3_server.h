#include "lib/s3_io.h"
#include "third/logc/log.h"

typedef struct S3Global S3Global;
struct S3Global {
    S3IO            *s3io;
    S3IOHandler     io_handler;

    int             stop_flag;
};

extern S3Global s3_g;

#define s3_global_null { \
    .s3io = NULL,        \
    .stop_flag = 0,      \
}

int s3_init_log();
int s3_init_net();
int s3_start_net();

void s3_global_destroy();

int s3_regist_signal();
