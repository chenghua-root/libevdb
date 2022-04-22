#include "lib/s3_util.h"

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include "third/logc/log.h"
#include "lib/s3_error.h"
#include "lib/s3_atomic.h"

char *s3_utility_get_tostring_buf() {
  static __thread char buffers[S3_TO_CSTRING_BUFFER_NUM][S3_TO_CSTRING_BUFFER_SIZE];
  volatile static __thread uint64_t i = 0;
  char *buffer = buffers[i++ % S3_TO_CSTRING_BUFFER_NUM];
  buffer[0] = '\0';
  return buffer;
}

int s3_buf_printf(char *buf, const int64_t buf_len, int64_t *pos, const char *fmt, ...) {
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

int s3_cpu_affinity() {
    static S3Atomic cpuid = -1;
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    cpu_set_t mask;
    int idx = (s3_atomic_add_return(&cpuid, 1) & 0x7fffffff) % cpu_num;
    CPU_ZERO(&mask);
    CPU_SET(idx, &mask);

    int ret = sched_setaffinity(0, sizeof(mask), &mask);
    if (ret != 0) {
        log_error("sched_setaffinity error: %d (%s), cpuid=%ld",
                  errno, strerror(errno), cpuid);
        return ret;
    }
    return S3_OK;
}
