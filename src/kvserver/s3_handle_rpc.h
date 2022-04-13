#ifndef S3_RPC_H_
#define S3_RPC_H_

#include "lib/s3_request.h"
#include "lib/s3_serialization.h"

typedef struct S3RpcSerializePacket S3RpcSerializePacket;
struct S3RpcSerializePacket {
  void *packet;
  S3FuncGetSerializedSize size_func;
  S3FuncSerialize serialize_func;
  S3FuncDeserialize deserialize_func;
};

int s3_handle_rpc_send_response(S3Request *r, int32_t pcode,
                                uint64_t session_id, int64_t time_out,
                                S3RpcSerializePacket *spacket);

int s3_handle_rpc_compute_add_resp(S3Request *r, int result, int64_t val);

#endif

