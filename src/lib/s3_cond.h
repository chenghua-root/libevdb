#ifndef S3_LIB_COND_H_
#define S3_LIB_COND_H_

#include <pthread.h>
#include <stdbool.h>
#include "lib/s3_atomic.h"

#define S3_COND_SLEEPING  1
#define S3_COND_WAKING_UP 0

typedef struct S3Cond S3Cond;
struct S3Cond {
  pthread_cond_t cd_;
  pthread_mutex_t mtx_;
  int64_t state_;
} CACHE_ALIGNED;

S3Cond *s3_cond_construct();
void s3_cond_destruct(S3Cond *cond);
int s3_cond_init(S3Cond *cond);
void s3_cond_destroy(S3Cond *cond);

bool s3_cond_wakeup(S3Cond *cond);
int s3_cond_wait(S3Cond *cond);
int s3_cond_timedwait(S3Cond *cond, int64_t time_us);

#endif
