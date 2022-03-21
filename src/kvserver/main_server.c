#include "lib/s3_connection.h"

int main(int argc, char *argv[]) {
    struct ev_loop *loop = EV_DEFAULT;

    s3_connection_create_listen_and_io_loop(loop);
    s3_connection_loop_run(loop);

    return 0;
}
