#ifndef S3_LIB_BUF_H_
#define S3_LIB_BUF_H_

#include <stdint.h>
#include "lib/s3_list.h"

/* buf.data    buf.left      buf.right    buf.data+buf.len
 *  |               |             |            |
 *  +---------------+-------------+------------+
 *  |   consumed    |  unconsumed |     free   |
 *
 *  [buf.data, buf.right] has filled data,
 *  [buf.data, buf.left]  has consumed,
 *  [buf.left, buf.right] has not consumed.
 *
 *  consumed + unconsumed + free == buf.len
 */

typedef struct S3Buf S3Buf;
typedef void (*S3BufCleanupFunc)(S3Buf *, void *);
struct S3Buf {
    int64_t len;
    int8_t  data_owned;  // = 1 data是自己申请的，= 0 data是别人的
    char    *data;
    char    *left;
    char    *right;

    S3ListHead       node;
    S3BufCleanupFunc cleanup_func;
    void             *cleanup_args;
    uint8_t          inited_;
};

#define s3_buf_null {          \
    .len = 0,                  \
    .data_owned = 0,           \
    .data = NULL,              \
    .left = NULL,              \
    .right = NULL,             \
    .node = s3_list_head_null, \
    .cleanup_func = NULL,      \
    .cleanup_args = NULL,      \
    .inited_ = 0,              \
}

S3Buf *s3_buf_construct();
void s3_buf_destruct(S3Buf *b);
int s3_buf_init(S3Buf *b, int64_t len);
void s3_buf_destroy(S3Buf *b);

int s3_buf_init_with_data(S3Buf *b, char *data, int64_t data_len, uint8_t data_owned);

static inline int64_t s3_buf_consumed_size(const S3Buf *b) {
  return b->left - b->data;
}
static inline int64_t s3_buf_unconsumed_size(const S3Buf *b) {
  return b->right - b->left;
}
static inline int64_t s3_buf_free_size(const S3Buf *b) {
  return b->data + b->len - b->right;
}


#endif

