/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: s3_packet.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "s3_packet.pb-c.h"
void   s3_add_req__init
                     (S3AddReq         *message)
{
  static const S3AddReq init_value = S3_ADD_REQ__INIT;
  *message = init_value;
}
size_t s3_add_req__get_packed_size
                     (const S3AddReq *message)
{
  assert(message->base.descriptor == &s3_add_req__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t s3_add_req__pack
                     (const S3AddReq *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &s3_add_req__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t s3_add_req__pack_to_buffer
                     (const S3AddReq *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &s3_add_req__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
S3AddReq *
       s3_add_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (S3AddReq *)
     protobuf_c_message_unpack (&s3_add_req__descriptor,
                                allocator, len, data);
}
void   s3_add_req__free_unpacked
                     (S3AddReq *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &s3_add_req__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   s3_add_resp__init
                     (S3AddResp         *message)
{
  static const S3AddResp init_value = S3_ADD_RESP__INIT;
  *message = init_value;
}
size_t s3_add_resp__get_packed_size
                     (const S3AddResp *message)
{
  assert(message->base.descriptor == &s3_add_resp__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t s3_add_resp__pack
                     (const S3AddResp *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &s3_add_resp__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t s3_add_resp__pack_to_buffer
                     (const S3AddResp *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &s3_add_resp__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
S3AddResp *
       s3_add_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (S3AddResp *)
     protobuf_c_message_unpack (&s3_add_resp__descriptor,
                                allocator, len, data);
}
void   s3_add_resp__free_unpacked
                     (S3AddResp *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &s3_add_resp__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   s3_sub_req__init
                     (S3SubReq         *message)
{
  static const S3SubReq init_value = S3_SUB_REQ__INIT;
  *message = init_value;
}
size_t s3_sub_req__get_packed_size
                     (const S3SubReq *message)
{
  assert(message->base.descriptor == &s3_sub_req__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t s3_sub_req__pack
                     (const S3SubReq *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &s3_sub_req__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t s3_sub_req__pack_to_buffer
                     (const S3SubReq *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &s3_sub_req__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
S3SubReq *
       s3_sub_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (S3SubReq *)
     protobuf_c_message_unpack (&s3_sub_req__descriptor,
                                allocator, len, data);
}
void   s3_sub_req__free_unpacked
                     (S3SubReq *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &s3_sub_req__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   s3_sub_resp__init
                     (S3SubResp         *message)
{
  static const S3SubResp init_value = S3_SUB_RESP__INIT;
  *message = init_value;
}
size_t s3_sub_resp__get_packed_size
                     (const S3SubResp *message)
{
  assert(message->base.descriptor == &s3_sub_resp__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t s3_sub_resp__pack
                     (const S3SubResp *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &s3_sub_resp__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t s3_sub_resp__pack_to_buffer
                     (const S3SubResp *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &s3_sub_resp__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
S3SubResp *
       s3_sub_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (S3SubResp *)
     protobuf_c_message_unpack (&s3_sub_resp__descriptor,
                                allocator, len, data);
}
void   s3_sub_resp__free_unpacked
                     (S3SubResp *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &s3_sub_resp__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   s3_mul_req__init
                     (S3MulReq         *message)
{
  static const S3MulReq init_value = S3_MUL_REQ__INIT;
  *message = init_value;
}
size_t s3_mul_req__get_packed_size
                     (const S3MulReq *message)
{
  assert(message->base.descriptor == &s3_mul_req__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t s3_mul_req__pack
                     (const S3MulReq *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &s3_mul_req__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t s3_mul_req__pack_to_buffer
                     (const S3MulReq *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &s3_mul_req__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
S3MulReq *
       s3_mul_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (S3MulReq *)
     protobuf_c_message_unpack (&s3_mul_req__descriptor,
                                allocator, len, data);
}
void   s3_mul_req__free_unpacked
                     (S3MulReq *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &s3_mul_req__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   s3_mul_resp__init
                     (S3MulResp         *message)
{
  static const S3MulResp init_value = S3_MUL_RESP__INIT;
  *message = init_value;
}
size_t s3_mul_resp__get_packed_size
                     (const S3MulResp *message)
{
  assert(message->base.descriptor == &s3_mul_resp__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t s3_mul_resp__pack
                     (const S3MulResp *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &s3_mul_resp__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t s3_mul_resp__pack_to_buffer
                     (const S3MulResp *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &s3_mul_resp__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
S3MulResp *
       s3_mul_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (S3MulResp *)
     protobuf_c_message_unpack (&s3_mul_resp__descriptor,
                                allocator, len, data);
}
void   s3_mul_resp__free_unpacked
                     (S3MulResp *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &s3_mul_resp__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   s3_div_req__init
                     (S3DivReq         *message)
{
  static const S3DivReq init_value = S3_DIV_REQ__INIT;
  *message = init_value;
}
size_t s3_div_req__get_packed_size
                     (const S3DivReq *message)
{
  assert(message->base.descriptor == &s3_div_req__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t s3_div_req__pack
                     (const S3DivReq *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &s3_div_req__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t s3_div_req__pack_to_buffer
                     (const S3DivReq *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &s3_div_req__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
S3DivReq *
       s3_div_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (S3DivReq *)
     protobuf_c_message_unpack (&s3_div_req__descriptor,
                                allocator, len, data);
}
void   s3_div_req__free_unpacked
                     (S3DivReq *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &s3_div_req__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   s3_div_resp__init
                     (S3DivResp         *message)
{
  static const S3DivResp init_value = S3_DIV_RESP__INIT;
  *message = init_value;
}
size_t s3_div_resp__get_packed_size
                     (const S3DivResp *message)
{
  assert(message->base.descriptor == &s3_div_resp__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t s3_div_resp__pack
                     (const S3DivResp *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &s3_div_resp__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t s3_div_resp__pack_to_buffer
                     (const S3DivResp *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &s3_div_resp__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
S3DivResp *
       s3_div_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (S3DivResp *)
     protobuf_c_message_unpack (&s3_div_resp__descriptor,
                                allocator, len, data);
}
void   s3_div_resp__free_unpacked
                     (S3DivResp *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &s3_div_resp__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor s3_add_req__field_descriptors[2] =
{
  {
    "A",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3AddReq, a),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "B",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3AddReq, b),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned s3_add_req__field_indices_by_name[] = {
  0,   /* field[0] = A */
  1,   /* field[1] = B */
};
static const ProtobufCIntRange s3_add_req__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor s3_add_req__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "S3AddReq",
  "S3AddReq",
  "S3AddReq",
  "",
  sizeof(S3AddReq),
  2,
  s3_add_req__field_descriptors,
  s3_add_req__field_indices_by_name,
  1,  s3_add_req__number_ranges,
  (ProtobufCMessageInit) s3_add_req__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor s3_add_resp__field_descriptors[1] =
{
  {
    "ret",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3AddResp, ret),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned s3_add_resp__field_indices_by_name[] = {
  0,   /* field[0] = ret */
};
static const ProtobufCIntRange s3_add_resp__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor s3_add_resp__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "S3AddResp",
  "S3AddResp",
  "S3AddResp",
  "",
  sizeof(S3AddResp),
  1,
  s3_add_resp__field_descriptors,
  s3_add_resp__field_indices_by_name,
  1,  s3_add_resp__number_ranges,
  (ProtobufCMessageInit) s3_add_resp__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor s3_sub_req__field_descriptors[2] =
{
  {
    "A",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3SubReq, a),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "B",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3SubReq, b),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned s3_sub_req__field_indices_by_name[] = {
  0,   /* field[0] = A */
  1,   /* field[1] = B */
};
static const ProtobufCIntRange s3_sub_req__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor s3_sub_req__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "S3SubReq",
  "S3SubReq",
  "S3SubReq",
  "",
  sizeof(S3SubReq),
  2,
  s3_sub_req__field_descriptors,
  s3_sub_req__field_indices_by_name,
  1,  s3_sub_req__number_ranges,
  (ProtobufCMessageInit) s3_sub_req__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor s3_sub_resp__field_descriptors[1] =
{
  {
    "ret",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3SubResp, ret),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned s3_sub_resp__field_indices_by_name[] = {
  0,   /* field[0] = ret */
};
static const ProtobufCIntRange s3_sub_resp__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor s3_sub_resp__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "S3SubResp",
  "S3SubResp",
  "S3SubResp",
  "",
  sizeof(S3SubResp),
  1,
  s3_sub_resp__field_descriptors,
  s3_sub_resp__field_indices_by_name,
  1,  s3_sub_resp__number_ranges,
  (ProtobufCMessageInit) s3_sub_resp__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor s3_mul_req__field_descriptors[2] =
{
  {
    "A",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3MulReq, a),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "B",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3MulReq, b),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned s3_mul_req__field_indices_by_name[] = {
  0,   /* field[0] = A */
  1,   /* field[1] = B */
};
static const ProtobufCIntRange s3_mul_req__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor s3_mul_req__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "S3MulReq",
  "S3MulReq",
  "S3MulReq",
  "",
  sizeof(S3MulReq),
  2,
  s3_mul_req__field_descriptors,
  s3_mul_req__field_indices_by_name,
  1,  s3_mul_req__number_ranges,
  (ProtobufCMessageInit) s3_mul_req__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor s3_mul_resp__field_descriptors[1] =
{
  {
    "ret",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3MulResp, ret),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned s3_mul_resp__field_indices_by_name[] = {
  0,   /* field[0] = ret */
};
static const ProtobufCIntRange s3_mul_resp__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor s3_mul_resp__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "S3MulResp",
  "S3MulResp",
  "S3MulResp",
  "",
  sizeof(S3MulResp),
  1,
  s3_mul_resp__field_descriptors,
  s3_mul_resp__field_indices_by_name,
  1,  s3_mul_resp__number_ranges,
  (ProtobufCMessageInit) s3_mul_resp__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor s3_div_req__field_descriptors[2] =
{
  {
    "Divisor",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3DivReq, divisor),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "Dividend",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3DivReq, dividend),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned s3_div_req__field_indices_by_name[] = {
  1,   /* field[1] = Dividend */
  0,   /* field[0] = Divisor */
};
static const ProtobufCIntRange s3_div_req__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor s3_div_req__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "S3DivReq",
  "S3DivReq",
  "S3DivReq",
  "",
  sizeof(S3DivReq),
  2,
  s3_div_req__field_descriptors,
  s3_div_req__field_indices_by_name,
  1,  s3_div_req__number_ranges,
  (ProtobufCMessageInit) s3_div_req__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor s3_div_resp__field_descriptors[1] =
{
  {
    "ret",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(S3DivResp, ret),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned s3_div_resp__field_indices_by_name[] = {
  0,   /* field[0] = ret */
};
static const ProtobufCIntRange s3_div_resp__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor s3_div_resp__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "S3DivResp",
  "S3DivResp",
  "S3DivResp",
  "",
  sizeof(S3DivResp),
  1,
  s3_div_resp__field_descriptors,
  s3_div_resp__field_indices_by_name,
  1,  s3_div_resp__number_ranges,
  (ProtobufCMessageInit) s3_div_resp__init,
  NULL,NULL,NULL    /* reserved[123] */
};