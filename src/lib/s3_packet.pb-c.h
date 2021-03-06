/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: s3_packet.proto */

#ifndef PROTOBUF_C_s3_5fpacket_2eproto__INCLUDED
#define PROTOBUF_C_s3_5fpacket_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1004000 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct S3AddReq S3AddReq;
typedef struct S3AddResp S3AddResp;
typedef struct S3SubReq S3SubReq;
typedef struct S3SubResp S3SubResp;
typedef struct S3MulReq S3MulReq;
typedef struct S3MulResp S3MulResp;
typedef struct S3DivReq S3DivReq;
typedef struct S3DivResp S3DivResp;


/* --- enums --- */


/* --- messages --- */

struct  S3AddReq
{
  ProtobufCMessage base;
  int64_t a;
  int64_t b;
};
#define S3_ADD_REQ__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&s3_add_req__descriptor) \
    , 0, 0 }


struct  S3AddResp
{
  ProtobufCMessage base;
  int32_t result;
  int64_t val;
};
#define S3_ADD_RESP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&s3_add_resp__descriptor) \
    , 0, 0 }


struct  S3SubReq
{
  ProtobufCMessage base;
  int64_t a;
  int64_t b;
};
#define S3_SUB_REQ__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&s3_sub_req__descriptor) \
    , 0, 0 }


struct  S3SubResp
{
  ProtobufCMessage base;
  int32_t result;
  int64_t val;
};
#define S3_SUB_RESP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&s3_sub_resp__descriptor) \
    , 0, 0 }


struct  S3MulReq
{
  ProtobufCMessage base;
  int64_t a;
  int64_t b;
};
#define S3_MUL_REQ__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&s3_mul_req__descriptor) \
    , 0, 0 }


struct  S3MulResp
{
  ProtobufCMessage base;
  int32_t result;
  int64_t val;
};
#define S3_MUL_RESP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&s3_mul_resp__descriptor) \
    , 0, 0 }


struct  S3DivReq
{
  ProtobufCMessage base;
  int64_t divisor;
  int64_t dividend;
};
#define S3_DIV_REQ__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&s3_div_req__descriptor) \
    , 0, 0 }


struct  S3DivResp
{
  ProtobufCMessage base;
  int32_t result;
  int64_t val;
};
#define S3_DIV_RESP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&s3_div_resp__descriptor) \
    , 0, 0 }


/* S3AddReq methods */
void   s3_add_req__init
                     (S3AddReq         *message);
size_t s3_add_req__get_packed_size
                     (const S3AddReq   *message);
size_t s3_add_req__pack
                     (const S3AddReq   *message,
                      uint8_t             *out);
size_t s3_add_req__pack_to_buffer
                     (const S3AddReq   *message,
                      ProtobufCBuffer     *buffer);
S3AddReq *
       s3_add_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   s3_add_req__free_unpacked
                     (S3AddReq *message,
                      ProtobufCAllocator *allocator);
/* S3AddResp methods */
void   s3_add_resp__init
                     (S3AddResp         *message);
size_t s3_add_resp__get_packed_size
                     (const S3AddResp   *message);
size_t s3_add_resp__pack
                     (const S3AddResp   *message,
                      uint8_t             *out);
size_t s3_add_resp__pack_to_buffer
                     (const S3AddResp   *message,
                      ProtobufCBuffer     *buffer);
S3AddResp *
       s3_add_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   s3_add_resp__free_unpacked
                     (S3AddResp *message,
                      ProtobufCAllocator *allocator);
/* S3SubReq methods */
void   s3_sub_req__init
                     (S3SubReq         *message);
size_t s3_sub_req__get_packed_size
                     (const S3SubReq   *message);
size_t s3_sub_req__pack
                     (const S3SubReq   *message,
                      uint8_t             *out);
size_t s3_sub_req__pack_to_buffer
                     (const S3SubReq   *message,
                      ProtobufCBuffer     *buffer);
S3SubReq *
       s3_sub_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   s3_sub_req__free_unpacked
                     (S3SubReq *message,
                      ProtobufCAllocator *allocator);
/* S3SubResp methods */
void   s3_sub_resp__init
                     (S3SubResp         *message);
size_t s3_sub_resp__get_packed_size
                     (const S3SubResp   *message);
size_t s3_sub_resp__pack
                     (const S3SubResp   *message,
                      uint8_t             *out);
size_t s3_sub_resp__pack_to_buffer
                     (const S3SubResp   *message,
                      ProtobufCBuffer     *buffer);
S3SubResp *
       s3_sub_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   s3_sub_resp__free_unpacked
                     (S3SubResp *message,
                      ProtobufCAllocator *allocator);
/* S3MulReq methods */
void   s3_mul_req__init
                     (S3MulReq         *message);
size_t s3_mul_req__get_packed_size
                     (const S3MulReq   *message);
size_t s3_mul_req__pack
                     (const S3MulReq   *message,
                      uint8_t             *out);
size_t s3_mul_req__pack_to_buffer
                     (const S3MulReq   *message,
                      ProtobufCBuffer     *buffer);
S3MulReq *
       s3_mul_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   s3_mul_req__free_unpacked
                     (S3MulReq *message,
                      ProtobufCAllocator *allocator);
/* S3MulResp methods */
void   s3_mul_resp__init
                     (S3MulResp         *message);
size_t s3_mul_resp__get_packed_size
                     (const S3MulResp   *message);
size_t s3_mul_resp__pack
                     (const S3MulResp   *message,
                      uint8_t             *out);
size_t s3_mul_resp__pack_to_buffer
                     (const S3MulResp   *message,
                      ProtobufCBuffer     *buffer);
S3MulResp *
       s3_mul_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   s3_mul_resp__free_unpacked
                     (S3MulResp *message,
                      ProtobufCAllocator *allocator);
/* S3DivReq methods */
void   s3_div_req__init
                     (S3DivReq         *message);
size_t s3_div_req__get_packed_size
                     (const S3DivReq   *message);
size_t s3_div_req__pack
                     (const S3DivReq   *message,
                      uint8_t             *out);
size_t s3_div_req__pack_to_buffer
                     (const S3DivReq   *message,
                      ProtobufCBuffer     *buffer);
S3DivReq *
       s3_div_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   s3_div_req__free_unpacked
                     (S3DivReq *message,
                      ProtobufCAllocator *allocator);
/* S3DivResp methods */
void   s3_div_resp__init
                     (S3DivResp         *message);
size_t s3_div_resp__get_packed_size
                     (const S3DivResp   *message);
size_t s3_div_resp__pack
                     (const S3DivResp   *message,
                      uint8_t             *out);
size_t s3_div_resp__pack_to_buffer
                     (const S3DivResp   *message,
                      ProtobufCBuffer     *buffer);
S3DivResp *
       s3_div_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   s3_div_resp__free_unpacked
                     (S3DivResp *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*S3AddReq_Closure)
                 (const S3AddReq *message,
                  void *closure_data);
typedef void (*S3AddResp_Closure)
                 (const S3AddResp *message,
                  void *closure_data);
typedef void (*S3SubReq_Closure)
                 (const S3SubReq *message,
                  void *closure_data);
typedef void (*S3SubResp_Closure)
                 (const S3SubResp *message,
                  void *closure_data);
typedef void (*S3MulReq_Closure)
                 (const S3MulReq *message,
                  void *closure_data);
typedef void (*S3MulResp_Closure)
                 (const S3MulResp *message,
                  void *closure_data);
typedef void (*S3DivReq_Closure)
                 (const S3DivReq *message,
                  void *closure_data);
typedef void (*S3DivResp_Closure)
                 (const S3DivResp *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor s3_add_req__descriptor;
extern const ProtobufCMessageDescriptor s3_add_resp__descriptor;
extern const ProtobufCMessageDescriptor s3_sub_req__descriptor;
extern const ProtobufCMessageDescriptor s3_sub_resp__descriptor;
extern const ProtobufCMessageDescriptor s3_mul_req__descriptor;
extern const ProtobufCMessageDescriptor s3_mul_resp__descriptor;
extern const ProtobufCMessageDescriptor s3_div_req__descriptor;
extern const ProtobufCMessageDescriptor s3_div_resp__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_s3_5fpacket_2eproto__INCLUDED */
