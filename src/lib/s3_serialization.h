#ifndef S3_LIB_SERIALIZATION_H_
#define S3_LIB_SERIALIZATION_H_

#include <stdint.h>
#include "lib/s3_packet.pb-c.h"

typedef size_t (*S3FuncGetSerializedSize)(const void *ptr);
typedef size_t (*S3FuncSerialize)(const void *ptr, uint8_t *out);
typedef size_t (*S3FuncDeserialize)(const void *ptr, uint8_t *out);

#endif
