#ifndef S3_LIB_UTILITY_H_
#define S3_LIB_UTILITY_H_

#include <stdint.h>
#include "lib/s3_error.h"

#define S3_TO_CSTRING_BUFFER_NUM 128
#define S3_TO_CSTRING_BUFFER_SIZE (1024*8)

#define S3_STRUCT_TO_CSTRING(type, prefix)                                     \
  int prefix##_to_string(const type *ptr,                                      \
    char *buf,                                                                 \
    const int64_t len,                                                         \
    int64_t *pos);                                                             \
                                                                               \
  static inline const char *prefix##_to_cstring(const type *ptr) {             \
    char *buf = s3_utility_get_tostring_buf();                                 \
    int64_t pos = 0;                                                           \
    int ret = prefix##_to_string(ptr, buf, S3_TO_CSTRING_BUFFER_SIZE-1, &pos); \
    if (ret != S3_OK) {                                                        \
      pos = 0;                                                                 \
    }                                                                          \
    buf[pos] = '\0';                                                           \
    return buf;                                                                \
  }

char *s3_utility_get_tostring_buf();


int s3_buf_printf(char *buf, const int64_t buf_len, int64_t *pos, const char *fmt, ...);


int s3_cpu_affinity();

#endif
