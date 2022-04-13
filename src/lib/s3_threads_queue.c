#include "lib/s3_threads_queue.h"

#include <memory.h>
#include "third/logc/log.h"
#include "lib/s3_atomic.h"
#include "lib/s3_error.h"
#include "lib/s3_define.h"
#include "lib/s3_malloc.h"
#include "lib/s3_pthread.h"

#define S3ThreadsQueueErrorTaskCnt 1000
#define S3ThreadsQueueErrorLogInterval 5 * 1000000L
#define S3ThreadsQueueStatePrintInterval 5 * 1000000L

static void s3_threads_queue_print_task_info_(S3ThreadsQueue *tq);

S3ThreadsQueue *s3_threads_queue_construct() {
  S3ThreadsQueue *q = NULL;
  if (NULL == (q = s3_malloc_(S3_MOD_THREADS_QUEUE, sizeof(S3ThreadsQueue)))) {
    log_error("malloc S3ThreadsQueue failed!");
  } else {
    *q = (S3ThreadsQueue)s3_threads_queue_null;
  }
  return q;
}

void s3_threads_queue_destruct(S3ThreadsQueue *tq) {
  if (tq != NULL) {
    s3_threads_queue_destroy(tq);
    s3_free_(S3_MOD_THREADS_QUEUE, tq);
  }
}

S3ListHead *s3_thread_queues_pop_node_(S3ThreadsQueue *tq) {
  pthread_spin_lock(&tq->q_lock_);
  S3ListHead *node = s3_queue_remove_head(&tq->que_);
  if (node != NULL) {
    --tq->waiting_task_cnt_;
  }
  pthread_spin_unlock(&tq->q_lock_);
  return node;
}

typedef struct S3ThreadsQueueWorkerArg S3ThreadsQueueWorkerArg;
struct S3ThreadsQueueWorkerArg {
  S3ThreadsQueue *tq;
  int64_t idx;
};

void *s3_threads_queue_work_func_(void *arg) {
  S3ThreadsQueueWorkerArg *tq_arg = (S3ThreadsQueueWorkerArg*)arg;
  S3Cond *cond = &tq_arg->tq->cnd_[tq_arg->idx];
  S3ThreadQueueState *state = &tq_arg->tq->state_;
  S3ThreadsQueue *tq= tq_arg->tq;
  s3_free_(S3_MOD_QUEUE_WORKER_ARG, arg);

  S3ListHead *node = NULL;
  S3ThreadTask *task = NULL;

  if (tq->init_thread.cb_func != NULL) {
    tq->init_thread.cb_func(tq->init_thread.cb_arg);
  }

  log_info("s3_threads_queue_work_func_ starting..");
  while (1) {
    s3_cond_wait(cond);

    while (1) {

      if (s3_atomic_load(state) != START) {
        log_info("S3ThreadsQueue state changed, work thread quit, state=%d", s3_atomic_load(state));
        goto exit;
      }

      node = s3_thread_queues_pop_node_(tq);
      if (NULL == node) {
        break;
      }

      task = s3_list_entry(node, S3ThreadTask, node_);
      task->work_func_(task->arg_);
      if (task->free_func_ != NULL) {
        task->free_func_(task->arg_);
      }
      s3_free_(S3_MOD_THREADS_TASK, task);
    }
  }

exit:
  log_info("s3_threads_queue_work_func_ exiting..");
  return NULL;
}

void s3_threads_queue_free_(void *ptr) {
  s3_free_(S3_MOD_THREADS_TASK, ptr);
}

static int s3_threads_queue_do_init_(S3ThreadsQueue *tq, S3Callback *init_thread, int64_t thread_cnt, S3ThreadsQueueWakeupMode mode) {
  log_debug("->s3_threads_queue_do_init_ tq=%p, cnt=%ld", tq, thread_cnt);
  pthread_spin_init(&tq->q_lock_, PTHREAD_PROCESS_PRIVATE);
  int ret = s3_queue_init(&tq->que_, s3_threads_queue_free_);
  assert(ret == S3_OK);

  tq->cnd_ = s3_calloc_(S3_MOD_THREADS_QUEUE, thread_cnt, sizeof(S3Cond));
  ret = tq->cnd_ != NULL ? S3_OK : S3_ERR_OUT_OF_MEM;
  assert(ret == S3_OK);

  tq->pid_ = s3_calloc_(S3_MOD_THREADS_QUEUE, thread_cnt, sizeof(pthread_t));
  ret = tq->pid_ != NULL ? S3_OK : S3_ERR_OUT_OF_MEM;
  assert(ret == S3_OK);

  if (init_thread != NULL) {
    tq->init_thread = *init_thread;
  }

  for (int64_t i = 0; i < thread_cnt; i++) {
    ret = s3_cond_init(&tq->cnd_[i]);
    assert(ret == S3_OK);

    S3ThreadsQueueWorkerArg *arg = s3_calloc_(S3_MOD_QUEUE_WORKER_ARG, 1, sizeof(S3ThreadsQueueWorkerArg));
    ret = arg != NULL ? S3_OK : S3_ERR_OUT_OF_MEM;
    assert(ret == S3_OK);
    arg->idx = i;
    arg->tq = tq;
    ret = s3_pthread_create_joinable(&tq->pid_[i], s3_threads_queue_work_func_, (void *)(arg));
    assert(ret == S3_OK);
  }

  if (ret != S3_OK) {
    exit(1);
  }
  tq->threads_cnt_ = thread_cnt;
  tq->wakeup_mode_ = mode;
  tq->state_ =  START;
  return ret;
}

int s3_threads_queue_init(S3ThreadsQueue *tq, S3Callback *init_thread, int64_t threads_cnt, S3ThreadsQueueWakeupMode mode) {
  s3_must_be(tq, S3_ERR_INVALID_ARG);
  s3_must_be(threads_cnt > 0, S3_ERR_INVALID_ARG);
  s3_must_be(S3_THREADS_QUEUE_WAKEUP_START_ < mode && mode < S3_THREADS_QUEUE_WAKEUP_END_, S3_ERR_INVALID_ARG);

  if (UNINIT != s3_atomic_load(&tq->state_)) {
    log_error("s3_threads_queue_init tiwce, queue=%p", tq);
    return S3_ERR_INIT_TWICE;
  }

  return s3_threads_queue_do_init_(tq, init_thread, threads_cnt, mode);
}

void s3_threads_queue_destroy(S3ThreadsQueue *tq) {
  s3_must_be(tq);

  if (START != s3_atomic_cmp_swap(&tq->state_, START, STOP)) {
    log_warn("S3ThreadsQueue state is unexpected, state = %d", tq->state_);
    return;
  }

  for (int i = 0; i < tq->threads_cnt_; i++) {
    s3_cond_wakeup(&tq->cnd_[i]);
    s3_pthread_join(tq->pid_[i], NULL);
  }

  S3ListHead *node = NULL;
  do {
    if (NULL == node) {
    } else {
      S3ThreadTask *task = NULL;
      task = s3_list_entry(node, S3ThreadTask, node_);
      if (task->free_func_ != NULL) {
        task->free_func_(task->arg_);
      }
      s3_free_(S3_MOD_THREADS_TASK, task);
    }
    pthread_spin_lock(&tq->q_lock_);
    node = s3_queue_remove_head(&tq->que_);
    pthread_spin_unlock(&tq->q_lock_);
  } while (node != NULL);
  s3_queue_destroy(&tq->que_);

  s3_free_(S3_MOD_THREADS_QUEUE, tq->cnd_);
  s3_free_(S3_MOD_THREADS_QUEUE, tq->pid_);
  pthread_spin_destroy(&tq->q_lock_);
  *tq = (S3ThreadsQueue)s3_threads_queue_null;
}

static S3ThreadTask *s3_alloc_threads_task_(S3ThreadTask *task) {

  S3ThreadTask *t = NULL;
  int ret = S3_OK;

  if (NULL == (t = s3_malloc_(S3_MOD_THREADS_TASK, sizeof(S3ThreadTask)))) {
    log_error("alloc ThreadTask failed, ret = %d", ret);
  } else {
    memset(t, 0, sizeof(S3ThreadTask));
    t->arg_ = task->arg_;
    t->free_func_ = task->free_func_;
    t->work_func_ = task->work_func_;
    init_s3_list_head(&t->node_);
  }
  return t;
}

void s3_threads_queue_wake_up_rr(S3ThreadsQueue *tq) {
  int64_t guard = s3_atomic_inc(&tq->next_pid_);
  for (int64_t i = 0, j = guard; s3_atomic_load(&tq->waiting_task_cnt_) > 0 && i < tq->threads_cnt_; i++, j++) {
   if (s3_cond_wakeup(&tq->cnd_[j % tq->threads_cnt_])) {
      return;
    }
  }
}

void s3_threads_queue_wake_up_all(S3ThreadsQueue *tq) {
  for (int64_t i = 0; s3_atomic_load(&tq->waiting_task_cnt_) > 0 && i < tq->threads_cnt_; i++) {
    s3_cond_wakeup(&tq->cnd_[i]);
  }
}

static inline void s3_threads_queue_wake_up_(S3ThreadsQueue *tq) {
  switch (tq->wakeup_mode_) {
    case S3_THREADS_QUEUE_WAKEUP_RR:
      s3_threads_queue_wake_up_rr(tq);
      break;
    case S3_THREADS_QUEUE_WAKEUP_ALL:
      s3_threads_queue_wake_up_all(tq);
      break;
    default:
      log_error("Invalid threads queue dispatch mode: %d", tq->wakeup_mode_);
  }
}

int s3_threads_queue_push(S3ThreadsQueue *tq, S3ThreadTask *t) {
  s3_must_be(tq && t && t->work_func_, S3_ERR_INVALID_ARG);
  s3_must_be(START == s3_atomic_load(&tq->state_), S3_ERR_INVALID_ARG);

  S3ThreadTask *task = s3_alloc_threads_task_(t);
  s3_must_be(task != NULL, S3_ERR_OUT_OF_MEM);

  pthread_spin_lock(&tq->q_lock_);
  int ret = s3_queue_append(&tq->que_, &task->node_);
  if s3_likely(S3_OK == ret) {
    ++tq->waiting_task_cnt_;
  }
  pthread_spin_unlock(&tq->q_lock_);
  assert(ret == S3_OK);

  s3_threads_queue_wake_up_(tq);

  if (ret != S3_OK) {
    s3_free_(S3_MOD_THREADS_TASK, task);
  }
  return ret;
}

// vim:ts=2:sw=2:et
