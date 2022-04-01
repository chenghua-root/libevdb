#ifndef S3_LIB_CONNECTION_H_
#define S3_LIB_CONNECTION_H_

#include <ev.h>
#include <stdint.h>
#include "lib/s3_list.h"

typedef struct S3Connection S3Connection;
struct S3Connection {
  S3ListHead      list_node;
  struct ev_loop  *loop;

  int             fd;
  ev_io           read_watcher;
  ev_io           write_watcher;
};

#define s3_connection_null { \
    .loop = NULL,            \
    .fd = 0,                 \
    .read_watcher = {0},     \
    .write_watcher = {0},    \
}

S3Connection *s3_connection_construct();
void s3_connection_destruct(S3Connection *conn);
int s3_connection_init(S3Connection *conn, struct ev_loop *loop, int fd);
void s3_connection_destroy(S3Connection *conn);


void s3_connection_recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents);
void s3_connection_write_socket_cb(struct ev_loop *loop, ev_io *w, int revents);

#endif
