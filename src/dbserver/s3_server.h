#ifndef S3_SERVER_H_
#define S3_SERVER_H_


int s3_init_log(int level);
int s3_init_net();
int s3_start_net();
int s3_start_worker_threads();

void s3_global_destroy();

int s3_regist_signal();

#endif
