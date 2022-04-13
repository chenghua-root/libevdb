#include "lib/s3_utility.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include "lib/s3_error.h"

int s3_databuff_printf(char *buf, const int64_t buf_len, int64_t* pos, const char* fmt, ...) {
  assert(buf && pos && 0 <= *pos && *pos < buf_len);
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(buf + *pos, buf_len - *pos, fmt, args);
  va_end(args);
  if (len < buf_len - *pos) {
    *pos += len;
  } else {
    *pos = buf_len - 1; /*length do not contain end \0*/
  }
  return S3_OK;
}
