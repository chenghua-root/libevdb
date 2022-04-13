#include "s3_handle_rpc.h"

#include "lib/s3_error.h"
#include "lib/s3_packet.pb-c.h"
#include "lib/s3_io.h"
#include "lib/s3_rpc.h"

int s3_handle_rpc_send_response(S3Request *r, int32_t pcode, uint64_t session_id,
                         int64_t time_out, S3RpcSerializePacket *spacket) {
    size_t data_len = spacket->size_func(spacket->packet);
    void *data = malloc(data_len);
    size_t pack_len = spacket->serialize_func(spacket->packet, data);
    assert(data_len == pack_len);

    S3Packet *p = s3_packet_construct();
    int ret = s3_packet_init(p, pcode, session_id, time_out, data_len, data);
    r->out_packet = p;

    s3_io_thread_add_resp_request(r);

    return ret;
}

static inline size_t s3_compute_add_resp_packed_size(const void *ptr) {
    return s3_add_resp__get_packed_size(ptr);
}
static inline size_t s3_compute_add_resp_pack(const void *ptr, uint8_t *out) {
    return s3_add_resp__pack(ptr, out);
}

int s3_handle_rpc_compute_add_resp(S3Request *r, int result, int64_t val) {
    S3AddResp resp = S3_ADD_RESP__INIT;
    resp.result = result;
    resp.val = val;

    S3RpcSerializePacket spacket = {
        .packet = &resp,
        .size_func = s3_compute_add_resp_packed_size,
        .serialize_func = s3_compute_add_resp_pack,
        .deserialize_func = NULL,
    };

    uint64_t session_id = r->in_packet->header.session_id;
    int ret = s3_handle_rpc_send_response(r, S3_PACKET_CODE_ADD_RESP, session_id, 1000/*FIXME*/, &spacket);

    return ret;
}
