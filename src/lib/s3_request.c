#include "lib/s3_request.h"
#include "lib/s3_error.h"

S3Request *s3_request_construct() {
    S3Request *r = malloc(sizeof(S3Request));
    if (r != NULL) {
        *r = (S3Request)s3_request_null;
    }
    return r;
}

void s3_request_destruct(S3Request *r) {
    if (r != NULL) {
        s3_request_destroy(r);
        free(r);
    }
}

int s3_request_init(S3Request *r) {
    s3_list_head_init(&r->request_list_node);
    return S3_OK;
}

void s3_request_destroy(S3Request *r) {
    s3_list_del(&r->request_list_node);
    s3_packet_desstruct(r->in_packet);
    s3_packet_desstruct(r->out_packet);
}
