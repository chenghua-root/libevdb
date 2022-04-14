#include "s3_handle_compute.h"

#include "third/logc/log.h"
#include "lib/s3_error.h"
#include "lib/s3_packet.pb-c.h"
#include "s3_handle_rpc.h"

int s3_handle_compute_add(S3Request *r) {
    S3Packet *p = r->in_packet;
    S3PacketHeader *header = &p->header;

    S3AddReq *add_req = s3_add_req__unpack(NULL, header->data_len, p->data);
    int sum = add_req->a + add_req->b;
    log_info("-A----------------------------------------------------\
            do add, a=%ld, b=%ld, sum=%d", add_req->a, add_req->b, sum);
    s3_add_req__free_unpacked(add_req, NULL);

    s3_handle_rpc_compute_add_resp(r, S3_OK, sum);

    return S3_OK;
}
