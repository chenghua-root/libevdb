#ifndef S3_LIB_MESSAGE_H_
#define S3_LIB_MESSAGE_H_

#include "lib/s3_list.h"

/*
 * one S3Message has one or multi S3Packet.
 * everyone S3Packet would be wraped in one S3Request.
 *
 * input_buf has total length.
 * recved data(packets) store in input_buf from start to used.
 * data would be decode to packets, the buf start to consumed has be decoded.
 *
 * input_buf:
 *     |--------------|-------------------|-------------------|
 *     |              |                   |                 total
 *     |              |                  used
 *     |           consumed
 *    start
 */
typedef struct S3Message        S3Message;
struct S3Message {
  void                          *input_buf;
  int                           input_total;
  int                           input_used;
  int                           input_consumed;

  S3ListHead                    request_list;
};

#endif
