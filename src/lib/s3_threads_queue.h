#ifndef S3_LIB_THREADS_QUEUE_H_
#define S3_LIB_THREADS_QUEUE_H_

#include "lib/s3_cond.h"
#include "lib/s3_define.h"
#include "lib/s3_queue.h"

typedef void (*FreeFunc)(void *);
typedef void (*WorkFunc)(void *);
typedef void *(*ThreadFunc)(void *);
typedef void (*EmptyCbFunc)(void *);

typedef enum S3TaskLevel S3TaskLevel;
enum S3TaskLevel {
  S3_TASK_LEVEL_HIGH = 0,
  S3_TASK_LEVEL_LOW = 1,
  S3_TASK_LEVEL_LOWER = 2,
  S3_TASK_LEVEL_CNT,
};

typedef enum S3ThreadsQueueWakeupMode S3ThreadsQueueWakeupMode;
enum S3ThreadsQueueWakeupMode {
  S3_THREADS_QUEUE_WAKEUP_START_,
  S3_THREADS_QUEUE_WAKEUP_RR,
  S3_THREADS_QUEUE_WAKEUP_ALL,
  S3_THREADS_QUEUE_WAKEUP_END_,
};

typedef enum S3ThreadQueueState S3ThreadQueueState;
enum S3ThreadQueueState {
  UNINIT = 0x00,
  START,
  STOP,
};

typedef struct S3ThreadTask S3ThreadTask;
struct S3ThreadTask {
  S3ListHead node_;
  void *arg_;
  WorkFunc work_func_; //调用者指定的工作函数,不可为空
  FreeFunc free_func_; //调用者指定的释放arg的内存的释放函数，可以为空
  S3TaskLevel level_;
};

typedef struct  S3ThreadsQueue S3ThreadsQueue;
struct S3ThreadsQueue {
  S3Cond *cnd_ CACHE_ALIGNED;
  pthread_t *pid_;
  S3Queue que_;
  S3ThreadQueueState state_ CACHE_ALIGNED;
  int64_t threads_cnt_;
  int64_t next_pid_;
  int64_t waiting_task_cnt_;
  S3ThreadsQueueWakeupMode wakeup_mode_;
  S3Callback init_thread;
  pthread_spinlock_t q_lock_;
};

#define s3_threads_queue_null { \
  .cnd_ = NULL,\
  .pid_ = NULL,\
  .que_ = s3_queue_null,\
  .state_  = UNINIT, \
  .threads_cnt_ = 0, \
  .next_pid_ = 0, \
  .waiting_task_cnt_ = 0, \
  .wakeup_mode_ = S3_THREADS_QUEUE_WAKEUP_RR, \
  .init_thread = {.cb_func=NULL, .cb_arg=NULL}, \
}

S3ThreadsQueue *s3_threads_queue_construct();
void s3_threads_queue_destruct(S3ThreadsQueue *tq);

/*
 * 初始化并启动工作线程
 * 非线程安全
 */
int s3_threads_queue_init(S3ThreadsQueue *tq,
                          S3Callback *init_thread,
                          int64_t threads_cnt,
                          S3ThreadsQueueWakeupMode mode);

void s3_threads_queue_destroy(S3ThreadsQueue *tq);

/*
 * task 在内部实现中会被拷贝，因此task内存由调用者负责申请和释放，也可以使用栈变量
 * task内部的arg成员变量的内存由调用者负责申请，而S3ThreadsQueue执行完毕work_func_后负责释放
 * 调用者需要保证调用s3_threads_queue_destroy()后，不会再调用s3_threads_queue_push()
 */
int s3_threads_queue_push(S3ThreadsQueue *tq, S3ThreadTask *task);

#endif
