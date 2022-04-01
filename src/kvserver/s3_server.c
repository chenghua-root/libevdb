#include "s3_server.h"

#include <ev.h>
#include "s3_net_callback.h"

S3Global s3_g = s3_global_null;
static FILE *g_log_fp = NULL;

/*****************************s3_init_log***************************/
int s3_init_log() {
    log_set_level(0);
    log_set_quiet(false);

    FILE *g_log_fp;
    g_log_fp = fopen("./libkv.log", "ab");
    if(g_log_fp == NULL) {
        return -1;
    }
    log_add_fp(g_log_fp, 0);

    log_info("init log succ...");

    return 0;
}

static void s3_destroy_log() {
    if (g_log_fp != NULL) {
        fclose(g_log_fp);
        g_log_fp = NULL;
    }
}

/*****************************s3_init_net***************************/
int s3_init_net() {
    s3_g.s3io = s3_io_create();

    S3IOHandler *io_handler = &s3_g.io_handler;
    io_handler->decode = s3_net_decode;

    log_info("init net succ...");

    return 0;
}

int s3_start_net() {
    s3_io_start_run(s3_g.s3io);
}

static void s3_destroy_net() {
}


/*******************************s3_destroy*****************************/
void s3_destroy() {
    s3_destroy_log();
    s3_destroy_net();
}
