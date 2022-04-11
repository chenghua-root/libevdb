#include "s3_server.h"

#include <errno.h>
#include <ev.h>
#include <pthread.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include "lib/s3_error.h"
#include "s3_handle_request.h"

S3Global s3_g = s3_global_null;

/*****************************s3_init_log***************************/
static FILE *g_log_fp = NULL;
static pthread_mutex_t MUTEX_LOG;
static void s3_log_lock(bool lock, void *udata) {
    pthread_mutex_t *LOCK = (pthread_mutex_t*)(udata);
    if (lock) {
        pthread_mutex_lock(LOCK);
    } else {
        pthread_mutex_unlock(LOCK);
    }
}

int s3_init_log() {
    log_set_level(0);
    log_set_quiet(false); // false: 同步输出到屏幕; true: 只输出到文件

    pthread_mutex_init(&MUTEX_LOG, NULL);
    log_set_lock(s3_log_lock, &MUTEX_LOG);

    FILE *g_log_fp;
    g_log_fp = fopen("./libkv.log", "ab");
    if(g_log_fp == NULL) {
        return S3_FAIL;
    }
    log_add_fp(g_log_fp, 0);

    log_trace("init log succ...");
    log_debug("init log succ...");
    log_info ("init log succ...");
    log_warn ("init log succ...");
    log_error("init log succ...");
    log_fatal("init log succ...");

    return S3_OK;
}

static void s3_destroy_log() {
    if (g_log_fp != NULL) {
        fclose(g_log_fp);
        g_log_fp = NULL;
    }
    pthread_mutex_destroy(&MUTEX_LOG);
}

/*****************************s3_init_net***************************/
int s3_init_net() {
    S3IOHandler *io_handler = &s3_g.io_handler;
    io_handler->process = s3_handle_process_request;

    s3_g.s3io = s3_io_create(2, io_handler);

    log_info("init net succ...");

    return 0;
}

int s3_start_net() {
    s3_io_start_run(s3_g.s3io);
}

static void s3_destroy_net() {
    s3_io_destroy(s3_g.s3io);
}

/****************************s3_global_destroy**************************/
void s3_global_destroy() {
    s3_destroy_net();

    s3_destroy_log();
}

/*******************************s3_signal*****************************/
static void s3_signal_handler_quit(int sig) {
    log_info("receive SIGTERM signal, signal num=%d", sig);
    s3_g.stop_flag = 1;
}

static void s3_signal_handler_interrupt(int sig) {
    log_info("receive SIGINT signal, signal num=%d", sig);
    s3_g.stop_flag = 1;
}

static void s3_signal_handler_print_stat(int sig) {
    log_info("receive self define(print stat) signal, signal num=%d", sig);
    log_info("total connection count=%ld", s3_g.s3io->conn_cnt);
}

struct {
    int sig;
    __sighandler_t handler;
} s3_signal_table[] = {
    {SIGTERM, s3_signal_handler_quit},
    {SIGINT,  s3_signal_handler_interrupt},
    {41,      s3_signal_handler_print_stat}, // 自定义信号, kill -41 ${pid}
};

int s3_regist_signal() {
    for (int i = 0; i < sizeof(s3_signal_table) / sizeof(s3_signal_table[0]); ++i) {
        typeof(s3_signal_table[0]) *sh = & s3_signal_table[i];
        if (SIG_ERR == signal(sh->sig, sh->handler)) {
            log_error("regist signal fail. signal num = %d, errno = %s", sh->sig, strerror(errno));
            return S3_FAIL;
        }
    }
    return S3_OK;
}
