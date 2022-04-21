#include "s3_handle_compute.h"

#include <unistd.h>
#include "third/logc/log.h"
#include "lib/s3_error.h"
#include "lib/s3_packet.pb-c.h"
#include "s3_handle_rpc.h"

int s3_handle_compute_add(S3Request *r) {
    S3Packet *p = r->in_packet;
    S3PacketHeader *header = &p->header;

    S3AddReq *req = s3_add_req__unpack(NULL, header->data_len, p->data_buf->data);
    int64_t val = req->a + req->b;
    //log_info("add request, a=%ld, b=%ld, val=%d", req->a, req->b, val);
    s3_add_req__free_unpacked(req, NULL);

    s3_handle_rpc_compute_add_resp(r, S3_OK, val);

    return S3_OK;
}

int s3_handle_compute_sub(S3Request *r) {
    S3Packet *p = r->in_packet;
    S3PacketHeader *header = &p->header;

    S3SubReq *req = s3_sub_req__unpack(NULL, header->data_len, p->data_buf->data);
    int64_t val = req->a - req->b;
    s3_sub_req__free_unpacked(req, NULL);

    s3_handle_rpc_compute_sub_resp(r, S3_OK, val);

    return S3_OK;
}

int s3_handle_compute_mul(S3Request *r) {
    S3Packet *p = r->in_packet;
    S3PacketHeader *header = &p->header;

    S3MulReq *req = s3_mul_req__unpack(NULL, header->data_len, p->data_buf->data);
    int64_t val = req->a * req->b;
    s3_mul_req__free_unpacked(req, NULL);

    s3_handle_rpc_compute_mul_resp(r, S3_OK, val);

    return S3_OK;
}

int s3_handle_compute_div(S3Request *r) {
    S3Packet *p = r->in_packet;
    S3PacketHeader *header = &p->header;

    S3DivReq *req = s3_div_req__unpack(NULL, header->data_len, p->data_buf->data);
    int64_t val = req->dividend / req->dividend;
    s3_div_req__free_unpacked(req, NULL);

    s3_handle_rpc_compute_div_resp(r, S3_OK, val);

    return S3_OK;
}
