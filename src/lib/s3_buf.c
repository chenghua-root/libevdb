#include "lib/s3_buf.h"

#include <stdlib.h>
#include <assert.h>
#include "lib/s3_error.h"
#include "lib/s3_malloc.h"

S3Buf *s3_buf_construct() {
    S3Buf *b = s3_malloc_(S3_MOD_BUF, sizeof(S3Buf));
    if (b != NULL) {
        *b = (S3Buf)s3_buf_null;
        s3_list_head_init(&b->node);
    }
    return b;
}

void s3_buf_destruct(S3Buf *b) {
    if (b != NULL) {
        s3_buf_destroy(b);
        s3_free_(S3_MOD_BUF, b);
    }
}

int s3_buf_init(S3Buf *b, int64_t len) {
    b->data = s3_malloc_(S3_MOD_BUF_DATA, len);
    assert(b->data != NULL);
    b->left = b->right = b->data;
    b->len = len;

    return S3_OK;
}

void s3_buf_destroy(S3Buf *b) {
    if (b != NULL) {
        s3_free_(S3_MOD_BUF_DATA, b->data);
        *b = (S3Buf)s3_buf_null;
    }
}

int s3_buf_init_with_data(S3Buf *b, char *data, int64_t data_len) {
    assert(data != NULL);
    b->data = data;
    b->left = b->data;
    b->right = b->data + data_len;
    b->len = data_len;

    return S3_OK;
}
