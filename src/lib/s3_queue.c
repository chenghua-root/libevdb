#include "lib/s3_queue.h"

#include "lib/s3_define.h"
#include "lib/s3_error.h"
#include "lib/s3_malloc.h"
#include "third/logc/log.h"

struct S3Queue *s3_queue_construct() {
  S3Queue *ret = s3_malloc_(S3_MOD_QUEUE, sizeof(S3Queue));
  if (NULL == ret) {
  } else {
    init_s3_list_head(&ret->head_);
    ret->destroy_handler = NULL;
    ret->inited_ = 0;
  }
  return ret;
}

void s3_queue_destruct(S3Queue *queue) {
  s3_queue_destroy(queue);
  s3_free_(S3_MOD_QUEUE, queue);
}

int s3_queue_init(S3Queue *queue, S3QueueDestroyHandler dh) {
  int ret = S3_OK;
  if (queue == NULL) {
    ret = S3_ERR_INVALID_ARG;
  } else if (queue->inited_ == 1) {
    log_warn("s3_queue_init twice, %p", queue);
    return S3_ERR_INIT_TWICE;
  } else {
    init_s3_list_head(&queue->head_);
    queue->destroy_handler = dh;
    queue->inited_ = 1;
    log_debug("s3_queue_init succ, %p", queue);
  }
  return ret;
}

void s3_queue_destroy(S3Queue *queue) {
  if(queue != NULL && queue->inited_ == 1) {
    S3QueueEntry *tmp = NULL;
    S3QueueEntry *list = NULL;
    s3_list_for_each_safe(tmp, list, &queue->head_) {
      s3_list_del_init(tmp);
      if (queue->destroy_handler != NULL) {
        queue->destroy_handler(tmp);
      }
    }
    queue->inited_ = 0;
  }
}

int s3_queue_append(S3Queue *queue, S3QueueEntry *node) {
  int ret = S3_OK;
  if (node == NULL) {
    ret =  S3_ERR_INVALID_ARG;
  } else {
    s3_list_add_tail(node, &queue->head_);
  }
  return ret;
}

int s3_queue_prepend(S3Queue *queue, S3QueueEntry *node) {
  int ret = S3_OK;
  if (node == NULL) {
    ret =  S3_ERR_INVALID_ARG;
  } else {
    s3_list_add(node, &queue->head_);
  }
  return ret;
}

S3QueueEntry *s3_queue_get_head(S3Queue *queue) {
  S3QueueEntry *ret = NULL;
  if (!s3_list_empty(&queue->head_)) {
    ret = queue->head_.next;
  }
  return ret;
}

S3QueueEntry *s3_queue_get_tail(S3Queue *queue) {
  S3QueueEntry *ret = NULL;
  if (!s3_list_empty(&queue->head_)) {
    ret = queue->head_.prev;
  }
  return ret;
}

S3QueueEntry *s3_queue_remove_head(S3Queue *queue) {
  S3QueueEntry *ret = s3_queue_get_head(queue);
  if (ret != NULL) {
    s3_list_del_init(ret);
  }
  return ret;
}

int s3_queue_is_empty(S3Queue *queue) {
  return s3_list_empty(&queue->head_) ? 1 : 0;
}

void s3_queue_remove_entry(S3Queue *queue, S3QueueEntry *entry) {
  if (entry != NULL) {
    s3_list_del_init(entry);
  }
}

int64_t s3_queue_get_length(S3Queue *queue) {
  int64_t ret = 0;
  if (NULL != queue) {
    s3_queue_for_each(queue, elt) {
      ++ret;
    }
  }
  return ret;
}
