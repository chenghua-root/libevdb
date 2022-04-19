#ifndef S3_LIB_IO_HANDLER_H_
#define S3_LIB_IO_HANDLER_H_

/*
 * 调用方可以定义或者选择网络IO处理动作(如接收，发送，编码，解码，处理等)，
 * 通过handler传递给网络层,
 * 目前只定义了process
 */

typedef struct S3IOHandler S3IOHandler;
struct S3IOHandler {
  int             (*process)(struct S3Request *r);
  void            (*close)  (void *conn);
};

#endif
