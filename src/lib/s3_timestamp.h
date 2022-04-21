#ifndef S3_LIB_TIMESTAMP_H_
#define S3_LIB_TIMESTAMP_H_

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

int64_t s3_get_timestamp();
int64_t s3_get_inc_timestamp();

static inline int64_t s3_get_curr_time() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return ((int64_t)(t.tv_sec) * (int64_t)(1000000) + (int64_t)(t.tv_usec));
}

#endif
