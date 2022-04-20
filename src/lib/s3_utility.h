#ifndef S3_LIB_UTILITY_H_
#define S3_LIB_UTILITY_H_

#include <stdint.h>

int s3_databuff_printf(char *buf, const int64_t buf_len, int64_t* pos, const char* fmt, ...);

int s3_cpu_affinity();

#endif
