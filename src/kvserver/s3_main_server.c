#include <unistd.h>
#include "lib/s3_connection.h"
#include "lib/s3_connection_bak.h"
#include "s3_server.h"

int main(int argc, char *argv[]) {
    s3_init_log();

    //log_info("start run ev loop...");

    //struct ev_loop *loop = EV_DEFAULT;
    //s3_connection_bak_create_listen_and_io_loop(loop);
    //s3_connection_bak_loop_run(loop);

    s3_init_net();
    s3_start_net();
    while(1) {
        sleep(1);
    }
    s3_destroy();

    return 0;
}
