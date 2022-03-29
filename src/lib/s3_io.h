typedef struct S3IO             S3IO;
struct S3IO {
  int                           io_thread_cnt;
  /*
  S3MemPool                     *pool;
  S3ListHead                    sio_list_node;
  S3Atomic                      lock;

  struct S3Listen               *listen;
  int                           io_thread_count;
  struct S3BaseThreadPool       *io_thread_pool;
  void                          *user_data;

  uint32_t                      stoped : 1;
  uint32_t                      started : 1;
  uint32_t                      tcp_cork : 1;
  uint32_t                      tcp_nodelay : 1;
  uint32_t                      tcp_defer_accept : 1;
  uint32_t                      affinity_enable : 1;
  uint32_t                      no_redispatch : 1;
  uint32_t                      do_signal : 1;
  uint32_t                      block_thread_signal : 1;
  uint32_t                      support_ipv6 : 1;
  uint32_t                      use_accept4 : 1;
  uint32_t                      no_delayack : 1;
  uint32_t                      no_force_destroy: 1;

  int32_t                       send_qlen;
  int32_t                       force_destroy_second;
  sigset_t                      block_thread_sigset;
  int                           listen_backlog;
  uint32_t                      accept_count;

  ev_tstamp                     start_time;
  struct S3Summary              *sio_summary;
  */
};
#define s3_s3io_null {  \
    .io_thread_cnt = 0, \
}

S3IO *s3_s3io_construct();
void s3_s3io_desconstruct(S3IO *s3io);
int s3_s3io_init(S3IO *s3io, int io_thread_cnt);
void s3_s3io_destroy(S3IO *s3io);

S3IO *s3_s3io_create();

/*
 * 调用方可以定义或者选择网络IO处理动作(如接收，发送，编码，解码，处理等)，
 * 通过handler传递给网络层
 */
typedef struct S3IOHandler      S3IOHandler;
struct S3IOHandler {
  /*
  void                         *(*decode)(struct S3Message *m);
  int                           (*encode)(struct S3Request *r, void *packet);
  S3IOProcessHandler            *process;
  int                           (*batch_process)(struct S3Message *m);
  S3IOCleanupHandler            *cleanup;
  uint64_t                       (*get_packet_id)(struct S3Connection *c, void *packet);
  int                           (*on_connect)(struct S3Connection *c);
  int                           (*on_disconnect)(struct S3Connection *c);
  int                           (*new_packet)(struct S3Connection *c);
  int                           (*on_idle)(struct S3Connection *c);
  int                           (*set_data)(struct S3Request *r, const char *data, int len);
  void                          (*send_buf_done)(struct S3Request *r);
  void                          (*sending_data)(struct S3Connection *c);
  int                           (*send_data_done)(struct S3Connection *c);
  int                           (*on_redispatch)(struct S3Connection *c);
  int                           (*on_close)(struct S3Connection *c);
  void                          *user_data, *user_data2;
  */
};
