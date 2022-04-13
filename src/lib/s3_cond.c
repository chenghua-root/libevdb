#include "lib/s3_cond.h"

#include <errno.h>
#include "lib/s3_define.h"
#include "lib/s3_error.h"
#include "lib/s3_malloc.h"

#define S3_COND_PAUSE_CNT 1000

S3Cond *s3_cond_construct() {
    S3Cond *cond = s3_malloc_(S3_MOD_COND, sizeof(S3Cond));
    return cond;
}

void s3_cond_destruct(S3Cond *cond) {
    if (cond != NULL) {
        s3_cond_destroy(cond);
        s3_free_(S3_MOD_COND, cond);
    }
}

int s3_cond_init(S3Cond *cond) {
    s3_must_be(cond != NULL, S3_ERR_INVALID_ARG);

    pthread_mutex_init(&cond->mtx_, NULL);
    pthread_cond_init(&cond->cd_, NULL);
    s3_atomic_store(&cond->state_, S3_COND_SLEEPING);

    return S3_OK;
}

void s3_cond_destroy(S3Cond *cond) {
    if (cond != NULL) {
        pthread_mutex_destroy(&cond->mtx_);
        pthread_cond_destroy(&cond->cd_);
    }
}

bool s3_cond_wakeup(S3Cond *cond) {
  int64_t state = s3_atomic_cmp_swap(&cond->state_,
                                     S3_COND_SLEEPING,
                                     S3_COND_WAKING_UP);
  if (state == S3_COND_SLEEPING) {
      pthread_mutex_lock(&cond->mtx_);
      pthread_cond_signal(&cond->cd_);
      pthread_mutex_unlock(&cond->mtx_);
      return true;
  }

  return false;
}

int s3_cond_wait(S3Cond *cond) {
  return s3_cond_timedwait(cond, 0x0fffffffffffffffLL);
}

int s3_cond_timedwait(S3Cond *cond, int64_t abs_wakeup_us) {

  for(int i = 0; i < S3_COND_PAUSE_CNT; i++) {
     if (S3_COND_WAKING_UP == s3_atomic_cmp_swap(&cond->state_,
                                                 S3_COND_WAKING_UP,
                                                 S3_COND_SLEEPING)) {
      return S3_OK;
    }
    __asm__(".byte 0xf3, 0x90");
  }

  struct timespec ts;
  int ret = S3_OK;

  ts.tv_sec = abs_wakeup_us / (1000 * 1000);
  ts.tv_nsec = (abs_wakeup_us % (1000 * 1000))  * 1000;

  pthread_mutex_lock(&cond->mtx_);
  while (S3_COND_WAKING_UP != s3_atomic_cmp_swap(&cond->state_,
                                                 S3_COND_WAKING_UP,
                                                 S3_COND_SLEEPING)) {
    if (pthread_cond_timedwait(&cond->cd_, &cond->mtx_, &ts) == ETIMEDOUT) {
      ret = S3_ERR_TIMEOUT;
      break;
    }
  }
  pthread_mutex_unlock(&cond->mtx_);

  return ret;
}
