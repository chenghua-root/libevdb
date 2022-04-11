#include <unistd.h>
#include "lib/s3_connection_bak.h"
#include "lib/s3_error.h"
#include "s3_server.h"

int main(int argc, char *argv[]) {
    int ret = S3_OK;
    ret = s3_init_log();
    if (ret != S3_OK) {
        printf("init log fail"); exit(1);
    }

    //log_info("start run ev loop...");
    //struct ev_loop *loop = EV_DEFAULT;
    //s3_connection_bak_create_listen_and_io_loop(loop);
    //s3_connection_bak_loop_run(loop);

    ret = s3_init_net();
    if (ret != S3_OK) {
        log_fatal("init net fail"); exit(1);
    }

    ret = s3_start_net();
    if (ret != S3_OK) {
        log_fatal("start net fail"); exit(1);
    }

    ret = s3_regist_signal();
    if (ret != S3_OK) {
        log_fatal("regist signal fail"); exit(1);
    }

    while(s3_g.stop_flag == 0) {
        sleep(1);
    }

    s3_global_destroy();

    return 0;
}
