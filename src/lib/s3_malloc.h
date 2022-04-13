#ifndef S3_LIB_MALLOC_H_
#define S3_LIB_MALLOC_H_

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "lib/s3_atomic.h"
#include "lib/s3_define.h"
#include "lib/s3_utility.h"
#include "third/logc/log.h"
#define MEM_PRINTB_TEMPLATE "\n%-30s %10luB  %10lu  %10lu."
#define MEM_PRINTK_TEMPLATE "\n%-30s %10.1fK  %10lu  %10lu."
#define MEM_PRINTM_TEMPLATE "\n%-30s %10.1fM  %10lu  %10lu."
#define MEM_PRINT_HEADER_TEMPLATE "\n%-30s %10s  %10s  %10s."
#define MOD_PRINT_BUF_SIZE 20480
#define SIZE_MB (1024 * 1024)
#define SIZE_KB (1024)

#define _as_str(q, ...)   #q,
#define _as_const(q, ...)  q,
#define _mods(_render)                                        \
  _render(S3_MOD_BUF)                                    /*1*/\
  _render(S3_MOD_BUF_DATA)                                    \
  _render(S3_MOD_COND)                                        \
  _render(S3_MOD_QUEUE)                                       \
  _render(S3_MOD_THREADS_QUEUE)                               \
  _render(S3_MOD_QUEUE_WORKER_ARG)                            \
  _render(S3_MOD_THREADS_TASK)                                \
  _render(S3_MOD_MAX)

enum S3ModID {
  S3_MOD_NONE = 0,
  _mods(_as_const)
};

static const char *s3_mod_string_[] = {
  _mods(_as_str)
};
static const char *s3_mod_string_none = "S3_MOD_NONE";
static const char *s3_mod_string_unknown = "S3_MOD_KNOWN";
static const int64_t s3_print_mem_usage_interval = 600L * 1000000L;

static inline const char *s3_mod_str(int rc) {

  if (rc == 0) {
    return s3_mod_string_none;
  }

  rc = rc - S3_MOD_NONE;

  if (rc < 0 || rc >= sizeof(s3_mod_string_)) {
    return s3_mod_string_unknown;
  }

  return s3_mod_string_[ rc - 1];
}

#undef _mods
#undef _as_str
#undef _as_const
typedef struct S3MallocHeader S3MallocHeader;
struct S3MallocHeader{
  /* data */
  int mod_id;
  int preserved;
  uint64_t size;
} CACHE_ALIGNED;

typedef struct S3ModItem S3ModItem;
struct S3ModItem {
  /* data */
  uint64_t mem_alloc;
  uint64_t mem_alloc_times;
  uint64_t mem_free_times;
};

extern S3ModItem *g_mod_stats;
extern int64_t g_mod_last_print_time;
extern char g_mod_print_buf[];

static inline void s3_print_mem_usage() {
  int64_t pos = 0;
  const int64_t buf_len = MOD_PRINT_BUF_SIZE;
  char* buf = g_mod_print_buf;
  struct timeval tv;
  struct tm tm;
  gettimeofday(&tv, NULL);
  localtime_r((const time_t *)&tv.tv_sec, &tm);
  s3_databuff_printf(buf, buf_len, &pos, "\n[%04d-%02d-%02d %02d:%02d:%02d.%06ld]======= print mem usage. =======",
      tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
      tm.tm_hour, tm.tm_min, tm.tm_sec, (long)tv.tv_usec);
  s3_databuff_printf(buf, buf_len, &pos, MEM_PRINT_HEADER_TEMPLATE,
      "mod name", "mem_hold", "alloc_times","free_times");
  for (int i = 0; i < (int)S3_MOD_MAX; i++) {
    const S3ModItem *item = g_mod_stats + i;
    if (item->mem_alloc >= SIZE_MB) {
      s3_databuff_printf(buf, buf_len, &pos, MEM_PRINTM_TEMPLATE,
          s3_mod_str(i), item->mem_alloc/(double)SIZE_MB, item->mem_alloc_times, item->mem_free_times);
    } else if (item->mem_alloc >= SIZE_KB) {
      s3_databuff_printf(buf, buf_len, &pos, MEM_PRINTK_TEMPLATE,
          s3_mod_str(i), item->mem_alloc/(double)SIZE_KB, item->mem_alloc_times, item->mem_free_times);
    } else {
      s3_databuff_printf(buf, buf_len, &pos, MEM_PRINTB_TEMPLATE,
          s3_mod_str(i), item->mem_alloc, item->mem_alloc_times, item->mem_free_times);
    }
    if (item->mem_alloc_times != item->mem_free_times) {
      s3_databuff_printf(buf, buf_len, &pos, "possible leak.");
    }
  }
  s3_databuff_printf(buf, buf_len, &pos, "\n");
  log_info(buf, &pos);
}

void* s3_malloc_(int mod_id, uint64_t size);
void* s3_calloc_(int mod_id, int cnt, uint64_t size);
void* s3_realloc_(int mode_id, void *ptr, uint64_t size);
void s3_free_(int mod_id, void *ptr);

#endif
