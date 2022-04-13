#ifndef S3_LIB_MESSAGE_H_
#define S3_LIB_MESSAGE_H_

#include <stdint.h>
#include "lib/s3_list.h"
#include "lib/s3_buf.h"

/*
 * one S3Message has one or multi S3Packet.
 * every S3Packet would be wraped in one S3Request.
 */

typedef enum S3MsgReadStatus S3MsgReadStatus;
enum S3MsgReadStatus {
    S3_MSG_READ_STATUS_INIT     = 0,
    S3_MSG_READ_STATUS_AGAIN    = 1,  // packet not integrity
    S3_MSG_READ_STATUS_DONE     = 2,  // message has no space
};

#define S3_MSG_BUF_LEN     (8*1024*1024)
#define S3_MSG_BUF_MIN_LEN 1024

typedef struct S3Message S3Message;
struct S3Message {
  S3Buf                  *in_buf;

  S3MsgReadStatus        read_status;
  uint64_t               next_read_len;
  S3ListHead             message_list_node;

  S3List                 request_list;
  uint32_t               request_cnt;
  uint32_t               request_done_cnt;

  void                   *conn;

};

#define s3_message_null {                   \
    .in_buf = NULL,                         \
    .read_status = S3_MSG_READ_STATUS_INIT, \
    .conn = NULL,                           \
}

S3Message *s3_message_construct();
void s3_message_destruct(S3Message *m);
int s3_message_init(S3Message *m);
void s3_message_destroy(S3Message *m);

S3Message *s3_message_create();
S3Message *s3_message_create_with_old(S3Message *old_msg);

void s3_message_try_destroy(S3Message *m);

#endif
