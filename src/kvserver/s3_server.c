#include "s3_server.h"

S3Global s3_g = s3_global_null;
FILE *g_log_fp = NULL;

int s3_init_log() {
    log_set_level(0);
    //log_set_quiet(true);

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

int s3_init_net() {
    s3_g.s3io = s3_s3io_create();

    //S3IOHandler *io_handler = &s3_g.io_handler;

    return 0;
}


int s3_init() {
    return 0;
}

void s3_destroy() {
    s3_destroy_log();
}
