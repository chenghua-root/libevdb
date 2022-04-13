#include "lib/s3_malloc.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "third/logc/log.h"

S3ModItem *g_mod_stats = NULL;
char g_mod_print_buf[MOD_PRINT_BUF_SIZE] = {'\0'};

static  __attribute__((constructor)) void init_mod_stats() {
  if (g_mod_stats == NULL) {
    int64_t len = sizeof(S3ModItem) * ((int)S3_MOD_MAX);
    g_mod_stats = (S3ModItem *)malloc(len);
    if (g_mod_stats == NULL) {
      log_error("failed to create mod stats. quit.");
      assert(0);
    } else {
      memset(g_mod_stats, 0, len);
      memset(g_mod_print_buf, 0, MOD_PRINT_BUF_SIZE); //no need
    }
  }
}

static  __attribute__((destructor)) void free_mod_stats() {
  if (g_mod_stats != NULL) {
    free(g_mod_stats);
    g_mod_stats = NULL;
  }
}

#define check_mem(_size, _mod_id) \
  s3_atomic_add(&g_mod_stats[_mod_id].mem_alloc, (_size)); \
  s3_atomic_inc(&g_mod_stats[_mod_id].mem_alloc_times);

void* s3_malloc_(int mod_id, uint64_t size) {
  void* ret = malloc(size + sizeof(S3MallocHeader));
  if s3_likely(ret) {
    *(S3MallocHeader*)ret = (S3MallocHeader){.mod_id = mod_id, .preserved = 0, .size = size};
    check_mem(size, mod_id);
    return ret + sizeof(S3MallocHeader);
  } else {
    s3_print_mem_usage();
    return ret;
  }
}

void* s3_calloc_(int mod_id, int cnt, uint64_t size) {
  void* ret = calloc(1, cnt * size + sizeof(S3MallocHeader));
  if s3_likely(ret) {
    *(S3MallocHeader*)ret = (S3MallocHeader){.mod_id = mod_id, .preserved = 0, .size = cnt * size};
    check_mem(size * cnt, mod_id);
    return ret + sizeof(S3MallocHeader);
  } else {
    s3_print_mem_usage();
    return ret;
  }
}

void s3_free_(int mod_id, void *ptr) {
  if (ptr != NULL) {
    S3MallocHeader *header = (S3MallocHeader*)(ptr - sizeof(S3MallocHeader));
    if (mod_id == header->mod_id) {
      s3_atomic_inc(&g_mod_stats[header->mod_id].mem_free_times);
      s3_atomic_sub(&g_mod_stats[header->mod_id].mem_alloc, header->size);
      free((void*)header);
    } else {
      log_warn("mod_id not match, header:[%d], supply:[%d]", header->mod_id, mod_id);
    }
  }
}
