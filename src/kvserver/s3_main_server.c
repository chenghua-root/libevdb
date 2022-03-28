#include "lib/s3_connection.h"
#include "s3_server.h"


int main(int argc, char *argv[]) {
    s3_init_log();

    log_info("start run ev loop...");
    struct ev_loop *loop = EV_DEFAULT;
    s3_connection_create_listen_and_io_loop(loop);
    s3_connection_loop_run(loop);

    s3_destroy();

    return 0;
}
