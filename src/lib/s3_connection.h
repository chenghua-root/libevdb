#ifndef S3_LIB_CONNECTION_H_
#define S3_LIB_CONNECTION_H_

#include <ev.h>
#include <stdint.h>

typedef struct S3Connection S3Connection;
struct S3Connection {
  struct ev_loop *loop;
  int            idx;

  int             fd;
  ev_io           read_watcher;
  ev_io           write_watcher;
};

#define s3_connection_null { \
    .loop = NULL,            \
    .idx = 0,                \
    .fd = 0,                 \
    .read_watcher = {0},     \
    .write_watcher = {0},    \
}

S3Connection *s3_connection_construct();
void s3_connection_desconstruct(S3Connection *conn);
int s3_connection_init(S3Connection *conn, struct ev_loop *loop, int fd);
void s3_connection_destroy(S3Connection *conn);

int s3_connection_create_listen_and_io_loop(struct ev_loop *loop);
void s3_connection_loop_run();

#endif
