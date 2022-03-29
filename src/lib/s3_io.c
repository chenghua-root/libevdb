#include <assert.h>
#include <stdlib.h>

#include "lib/s3_io.h"

S3IO *s3_s3io_construct() {
    S3IO *s3io = malloc(sizeof(S3IO));
    if (s3io != NULL) {
        *s3io = (S3IO)s3_s3io_null;
    }
    return s3io;
}

void s3_s3io_desconstruct(S3IO *s3io) {
    if (s3io != NULL) {
        s3_s3io_destroy(s3io);
        free(s3io);
    }
}

int s3_s3io_init(S3IO *s3io, int io_thread_cnt) {
    s3io->io_thread_cnt = io_thread_cnt;
    return 0;
}

void s3_s3io_destroy(S3IO *s3io) {
    s3io->io_thread_cnt = 0;
}

S3IO *s3_s3io_create() {
    int ret = 0;
    S3IO *s3io = NULL;

    s3io = s3_s3io_construct();
    assert(s3io != NULL);

    ret = s3_s3io_init(s3io, 2/*io thread cnt*/);
    assert(ret == 0);

    return s3io;
}
