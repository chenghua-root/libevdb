#include "lib/s3_timestamp.h"
#include "lib/s3_define.h"
#include "lib/s3_atomic.h"

static int64_t g_s3_timestamp = -1;
int64_t s3_get_timestamp() {
  return s3_get_curr_time();
}

int64_t s3_get_inc_timestamp() {
  int64_t old_ts = 0;
  int64_t new_ts = s3_get_curr_time();
  do {
    old_ts = s3_atomic_load(&g_s3_timestamp);
    new_ts = old_ts < new_ts ? new_ts : old_ts + 1;
  } while (!s3_atomic_cmp_set(&g_s3_timestamp, old_ts, new_ts));
  return new_ts;
}
