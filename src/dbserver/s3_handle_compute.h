#ifndef S3_HANDLE_COMPUTE_H_
#define S3_HANDLE_COMPUTE_H_

#include "lib/s3_request.h"

int s3_handle_compute_add(S3Request *r);

int s3_handle_compute_sub(S3Request *r);

int s3_handle_compute_mul(S3Request *r);

int s3_handle_compute_div(S3Request *r);

#endif
