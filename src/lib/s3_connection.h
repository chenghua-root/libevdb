#ifndef S3_LIB_CONNECTION_H_
#define S3_LIB_CONNECTION_H_

#include <ev.h>
#include <stdint.h>
#include "lib/s3_list.h"
#include "lib/s3_request.h"
#include "lib/s3_io_handler.h"

#define S3_CONN_DOING_MAX_REQ_CNT 65536
#define S3_IO_MAX_SIZE (8*1024*1024)

typedef struct S3Connection S3Connection;
struct S3Connection {
  struct ev_loop  *loop;
  S3ListHead      conn_list_node;
  S3ListHead      write_list_node;
  S3List          message_list;
  S3List          output_buf_list;
  uint64_t        request_doing_cnt;
  uint64_t        request_total_cnt;
  uint64_t        message_total_cnt;

  S3IOHandler     *handler;

  int             fd;
  ev_io           read_watcher;
  ev_io           write_watcher;

  void            *ioth;
  int8_t          closed;
};

#define s3_connection_null { \
    .loop = NULL,            \
    .fd = -1,                \
    .request_doing_cnt = 0,  \
    .request_total_cnt = 0,  \
    .message_total_cnt = 0,  \
    .read_watcher = {0},     \
    .write_watcher = {0},    \
    .ioth = NULL,            \
    .closed = 0,             \
}

S3Connection *s3_connection_construct();
void s3_connection_destruct(S3Connection *conn);
int s3_connection_init(S3Connection *conn, struct ev_loop *loop, int fd);
void s3_connection_destroy(S3Connection *conn);

void s3_connection_recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents);
void s3_connection_write_socket_cb(struct ev_loop *loop, ev_io *w, int revents);

void s3_connnection_resp_request(S3List *request_list);
void s3_connection_try_destruct(S3Connection *c);

#endif
