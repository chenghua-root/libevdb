#include "s3_server.h"

FILE *g_log_fp = NULL;
int s3_init_log() {
    log_set_level(0);
    //log_set_quiet(true);
    FILE *g_log_fp;
    g_log_fp = fopen("./libkv.log", "ab");
    if(g_log_fp == NULL)
    {
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


int s3_init() {
    return 0;
}

void s3_destroy() {
    s3_destroy_log();
}
