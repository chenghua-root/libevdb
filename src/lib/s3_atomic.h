#ifndef  S3_LIB_ATOMIC_
#define  S3_LIB_ATOMIC_

#include <stdint.h>
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "s3_define.h"


#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#define __COMPILER_BARRIER() asm volatile("" ::: "memory")
#define s3_mem_barrier() __sync_synchronize()
#define s3_pause() asm("pause\n")

#if GCC_VERSION > 40704
#define s3_atomic_load(x) __atomic_load_n((x), __ATOMIC_SEQ_CST)
#define s3_atomic_store(x, v) __atomic_store_n((x), (v), __ATOMIC_SEQ_CST)
#else
#define s3_atomic_load(x) ({__COMPILER_BARRIER(); *(x);})
#define s3_atomic_store(x, v) ({__COMPILER_BARRIER(); *(x) = v; __sync_synchronize(); })
#endif

#define s3_atomic_add(val, addv) __sync_fetch_and_add((val), (addv))
#define s3_atomic_add_return(val, addv) __sync_add_and_fetch((val), (addv))
#define s3_atomic_sub_return(val, subv) __sync_sub_and_fetch((val), (subv))
#define s3_atomic_sub(val, subv) __sync_fetch_and_sub((val), (subv))
#define s3_atomic_inc(val) __sync_add_and_fetch((val), 1)
#define s3_atomic_dec(val) __sync_sub_and_fetch((val), 1)
#define s3_atomic_return_set(val, newv) __sync_lock_test_and_set((val), (newv))
#define s3_atomic_cmp_swap(val, cmpv, newv) __sync_val_compare_and_swap((val), (cmpv), (newv))
#define s3_atomic_cmp_set(val, cmpv, newv) __sync_bool_compare_and_swap((val), (cmpv), (newv))

#define CACHE_ALIGN_SIZE 64
#define CACHE_ALIGNED __attribute__((aligned(CACHE_ALIGN_SIZE)))

typedef volatile int64_t S3Atomic;
typedef volatile int32_t S3Atomic32;

#define s3_spin_lock_null     {{.atomic = 0}}
#define s3_trylock(lock)      (*(lock) == 0 && s3_atomic_cmp_set(lock, 0, 1))
#define s3_unlock(lock)       {__asm__ ("" ::: "memory"); *(lock) = 0;}
#define s3_spin_unlock        s3_unlock

#define s3_spin_unlock_safe   s3_unlock_safe

typedef struct {
 union {
  struct {
    int32_t tid;
    int32_t atomic32;
  };
  volatile int64_t atomic;
 };
} CACHE_ALIGNED S3SpinLock;


/*
 * thread id is unique in a single process, man gettid(2) for more details.
 */
static __inline__ int64_t s3_get_tid() {
  static __thread int64_t tid = -1;
  if s3_unlikely(tid == -1) {
    tid = (int64_t)(syscall(__NR_gettid));
  }
  return tid;
}

static __inline__ int s3_try_lock_safe(S3SpinLock *lock) {
  S3SpinLock lock_val = {{{.tid = (int32_t)s3_get_tid(), .atomic32 = 1}}};
  return lock->atomic == 0 && s3_atomic_cmp_set(&lock->atomic, 0, lock_val.atomic);
}

static __inline__ int s3_unlock_safe(S3SpinLock *lock) {
  S3SpinLock old_val;
  S3SpinLock lock_val;
  int32_t tid = (int32_t)s3_get_tid();

  if (tid == lock->tid && 1 < lock->atomic32) {
    old_val = (S3SpinLock) {{{.tid = tid, .atomic32 = lock->atomic32}}};
    lock_val = (S3SpinLock) {{{.tid = tid, .atomic32 = lock->atomic32 - 1}}};
  } else {
    old_val = (S3SpinLock){{{.tid = tid, .atomic32 = 1}}};
    lock_val = (S3SpinLock){{.atomic = 0}};
  }
  return s3_atomic_cmp_set(&lock->atomic, old_val.atomic, lock_val.atomic);
}

extern void s3_spin_lock_safe(S3SpinLock *lock);
extern void s3_spin_lock(S3Atomic *lock);

#endif
