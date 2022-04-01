#ifndef S3_LIB_CONNECTION_BAK_H_
#define S3_LIB_CONNECTION_BAK_H_

#include <ev.h>
#include <stdint.h>
#include "lib/s3_list.h"

typedef struct S3ConnectionBak S3ConnectionBak;
struct S3ConnectionBak {
  S3ListHead      list_node;
  int             id;
  int             loop_idx; // one io thread has one loop
  struct ev_loop  *loop;

  int             fd;
  ev_io           read_watcher;
  ev_io           write_watcher;
};

#define s3_connection_bak_null { \
    .id = 0,                 \
    .loop = NULL,            \
    .fd = 0,                 \
    .read_watcher = {0},     \
    .write_watcher = {0},    \
}

S3ConnectionBak *s3_connection_bak_construct();
void s3_connection_bak_desconstruct(S3ConnectionBak *conn);
int s3_connection_bak_init(S3ConnectionBak *conn, struct ev_loop *loop, int fd);
void s3_connection_bak_destroy(S3ConnectionBak *conn);

int s3_connection_bak_create_listen_and_io_loop(struct ev_loop *loop);
void s3_connection_bak_loop_run();

#endif

