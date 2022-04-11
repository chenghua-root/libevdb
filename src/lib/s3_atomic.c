#include "lib/s3_atomic.h"

void s3_spin_lock_safe(S3SpinLock *lock) {
  int i, n;
  int32_t tid = (int32_t)s3_get_tid();
  S3SpinLock lock_val;

  if (tid == lock->tid && 0 < lock->atomic32) {
    // lock reentry
    S3SpinLock old_val = (S3SpinLock) {{{.tid = tid, .atomic32=lock->atomic32}}};
    lock_val = (S3SpinLock) {{{.tid = tid, .atomic32=lock->atomic32 + 1}}};
    if (s3_atomic_cmp_set(&lock->atomic, old_val.atomic, lock_val.atomic)) {
      return;
    }
  }
  lock_val = (S3SpinLock){{{.tid = tid, .atomic32 = 1}}};
  for (; ;) {
    if (lock->atomic == 0 && s3_atomic_cmp_set(&lock->atomic, 0, lock_val.atomic)) {
      return;
    }

    for (n = 1; n < 1024; n <<= 1) {

      for (i = 0; i < n; i++) {
        __asm__(".byte 0xf3, 0x90");
      }

      if (lock->atomic == 0 && s3_atomic_cmp_set(&lock->atomic, 0, lock_val.atomic)) {
        return;
      }
    }

    sched_yield();
  }
}

void s3_spin_lock(S3Atomic *lock) {
  int i, n;
  for (; ;) {
    if (*lock == 0 && s3_atomic_cmp_set(lock, 0, 1)) {
      return;
    }

    for (n = 1; n < 1024; n <<= 1) {

      for (i = 0; i < n; i++) {
        __asm__(".byte 0xf3, 0x90");
      }

      if (*lock == 0 && s3_atomic_cmp_set(lock, 0, 1)) {
        return;
      }
    }

    sched_yield();
  }
}
