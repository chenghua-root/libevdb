#ifndef S3_LIB_REQUEST_H_
#define S3_LIB_REQUEST_H_

#include "lib/s3_list.h"
#include "lib/s3_packet.h"

typedef struct S3Request S3Request;
struct S3Request {
    S3Packet   *in_packet;
    S3Packet   *out_packet;
    S3List     *out_buf_list;

    S3ListHead request_list_node;
    void       *message;

    // TODO: recv_time;
};
#define s3_request_null { \
    .in_packet = NULL,    \
    .out_packet = NULL,   \
    .request_list_node = s3_list_head_null, \
    .message = NULL,      \
}

S3Request *s3_request_construct();
void s3_request_destruct(S3Request *r);
int s3_request_init(S3Request *r);
void s3_request_destroy(S3Request *r);

#endif
