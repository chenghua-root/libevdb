#ifndef S3_LIB_PTHREAD_H_
#define S3_LIB_PTHREAD_H_

#include <errno.h>
#include <string.h>
#include <pthread.h>

typedef void* (*S3PthreadRoutine)(void *);
static inline int s3_pthread_create_(pthread_t *thread, S3PthreadRoutine routine, void *arg, int detachstate) {
  int ret = 0;
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  ret = pthread_attr_setdetachstate(&attr, detachstate);
  assert(ret == 0);
  ret = pthread_create(thread, &attr, routine, arg);
  assert(ret == 0);
  pthread_attr_destroy(&attr);

  return ret;
}

#define s3_pthread_create_joinable(thread, routine, arg) s3_pthread_create_((thread), (routine), (arg), PTHREAD_CREATE_JOINABLE)
#define s3_pthread_create_detached(thread, routine, arg) s3_pthread_create_((thread), (routine), (arg), PTHREAD_CREATE_DETACHED)
#define s3_pthread_join(thread, retval)                  pthread_join((thread), (retval))
#define s3_pthread_detach(thread)                        pthread_detach((thread))
#define s3_pthread_exit(retval)                          pthread_exit((retval))
#define s3_pthread_cancel(thread)                        pthread_cancel((thread))

#endif
