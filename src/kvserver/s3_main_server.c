#include <unistd.h>
#include "lib/s3_error.h"
#include "lib/s3_malloc.h"
#include "s3_global.h"
#include "s3_server.h"

int main(int argc, char *argv[]) {
    int ret = S3_OK;
    ret = s3_init_log(LOG_INFO);
    if (ret != S3_OK) {
        printf("init log fail"); exit(1);
    }

    ret = s3_init_net(2/*io thread cnt*/);
    if (ret != S3_OK) {
        log_fatal("init net fail"); exit(1);
    }

    ret = s3_start_worker_threads(2/*worker thread cnt*/);
    if (ret != S3_OK) {
        log_fatal("init work threads fail"); exit(1);
    }

    ret = s3_start_net();
    if (ret != S3_OK) {
        log_fatal("start net fail"); exit(1);
    }

    ret = s3_regist_signal();
    if (ret != S3_OK) {
        log_fatal("regist signal fail"); exit(1);
    }

    s3_print_mem_usage();

    while(s3_g.stop_flag == 0) {
        sleep(1);
    }

    s3_global_destroy();

    s3_print_mem_usage();

    return 0;
}
