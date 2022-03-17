#ifndef S3_LIB_CONNECTION_H_
#define S3_LIB_CONNECTION_H_

#include <ev.h>
#include <stdint.h>

typedef struct S3Connection S3Connection;
struct S3Connection {
  uint64_t        magic;
  struct ev_loop  *loop;

  int             fd;
  ev_io           read_watcher;
  ev_io           write_watcher;
};

#endif
