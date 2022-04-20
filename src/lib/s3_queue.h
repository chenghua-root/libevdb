#ifndef S3_LIB_QUEUE_H_
#define S3_LIB_QUEUE_H_

#include "lib/s3_list.h"

typedef void (*S3QueueDestroyHandler)(void *);

typedef struct S3ListHead S3QueueEntry;

typedef struct S3Queue S3Queue;
struct S3Queue {
  S3QueueEntry head_;
  S3QueueDestroyHandler destroy_handler;
  int8_t inited_;
};
#define s3_queue_null {       \
    .head_ = {NULL, NULL},    \
    .destroy_handler = NULL,  \
    .inited_=0,               \
}

#define s3_queue_for_each(_q, elt)                \
    for (S3QueueEntry *elt = (_q)->head_.next,    \
                      *_n = elt->next;            \
         elt != &(_q)->head_;                     \
         elt = _n, _n = elt->next )


S3Queue *s3_queue_construct();
void s3_queue_destruct(S3Queue *queue);
int s3_queue_init(S3Queue *queue, S3QueueDestroyHandler dh);
void s3_queue_destroy(S3Queue *queue);

S3QueueEntry *s3_queue_get_head(S3Queue *queue);
S3QueueEntry *s3_queue_get_tail(S3Queue *queue);
S3QueueEntry *s3_queue_remove_head(S3Queue *queue);
int s3_queue_append(S3Queue *queue, S3QueueEntry *node);
int s3_queue_prepend(S3Queue *queue, S3QueueEntry *node);
int s3_queue_is_empty(S3Queue *queue);
void s3_queue_remove_entry(S3Queue *queue, S3QueueEntry *entry);
int64_t s3_queue_get_length(S3Queue *queue);

#endif
