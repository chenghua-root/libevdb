#include "s3_handle_request.h"

#include "third/logc/log.h"
#include "lib/s3_packet.h"
#include "lib/s3_packet.pb-c.h"
#include "lib/s3_error.h"
#include "lib/s3_rpc.h"

static int s3_handle_compute_add(S3Request *r);

int s3_handle_process_request(S3Request *r) {
    int ret = S3_OK;
    S3Packet *p = r->in_packet;
    int pcode = p->header.pcode;

    switch (pcode) {
        case S3_PACKET_CODE_ADD:
            ret = s3_handle_compute_add(r);
            break;
        case S3_PACKET_CODE_SUB:
            // TODO
            break;
        case S3_PACKET_CODE_MUL:
            // TODO
            break;
        case S3_PACKET_CODE_DIV:
            // TODO
            break;
        default:
            log_error("invalid pcode=%ld", pcode);
            return S3_ERR_INVALID_RPC;
    }
    return ret;
}

static int s3_handle_compute_add(S3Request *r) {
    S3Packet *p = r->in_packet;
    S3PacketHeaderV2 *header = &p->header;

    S3AddReq *add_req = s3_add_req__unpack(NULL, header->data_len, p->data);
    int sum = add_req->a + add_req->b;
    log_info("-----------------------------------------------------\
            do add, a=%ld, b=%ld, sum=%d", add_req->a, add_req->b, sum);
    s3_add_req__free_unpacked(add_req, NULL);

    return S3_OK;
}

