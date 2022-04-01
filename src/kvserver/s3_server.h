#include "lib/s3_io.h"
#include "third/logc/log.h"

typedef struct S3Global S3Global;
struct S3Global {
    S3IO            *s3io;
    S3IOHandler     io_handler;
};

#define s3_global_null { \
    .s3io = NULL,        \
}

int s3_init_log();
int s3_init_net();
int s3_start_net();

void s3_destroy();

extern S3Global s3_g;
