#include "s3_handle_request.h"

#include "third/logc/log.h"
#include "lib/s3_packet.h"
#include "lib/s3_error.h"
#include "lib/s3_rpc.h"
#include "lib/s3_threads_queue.h"
#include "s3_global.h"
#include "s3_handle_compute.h"

static void s3_handle_do_request_cb(void *arg);
static int s3_task_dispatch(uint16_t pcode, S3ThreadTask *task);

/***************************run by io thread***********************************/
int s3_handle_request(S3Request *r) {
    S3Packet *p = r->in_packet;
    int pcode = p->header.pcode;

    S3ThreadTask task = {
        .arg_ = r,
        .work_func_ = s3_handle_do_request_cb,
        .free_func_ = NULL,
    };

    int ret = s3_task_dispatch(pcode, &task);
    assert(ret == S3_OK);

    return ret;
}

static int s3_task_dispatch(uint16_t pcode, S3ThreadTask *task) {
    int ret = S3_OK;
    switch (pcode) {
        case S3_PACKET_CODE_ADD:
        case S3_PACKET_CODE_SUB:
        case S3_PACKET_CODE_MUL:
        case S3_PACKET_CODE_DIV:
            ret = s3_threads_queue_push(s3_g.cmpt_workers, task);
            break;
        default:
            log_error("invalid pcode=%ld", pcode);
            return S3_ERR_INVALID_RPC;
    }
    return ret;
}

/***************************run by worker thread*********************************/
static void s3_handle_do_request_cb(void *arg) {
    int ret = S3_OK;
    S3Request *r = (S3Request *)arg;
    S3Packet *p = r->in_packet;
    int pcode = p->header.pcode;

    switch (pcode) {
        case S3_PACKET_CODE_ADD:
            ret = s3_handle_compute_add(r);
            break;
        case S3_PACKET_CODE_SUB:
            ret = s3_handle_compute_sub(r);
            break;
        case S3_PACKET_CODE_MUL:
            ret = s3_handle_compute_mul(r);
            break;
        case S3_PACKET_CODE_DIV:
            ret = s3_handle_compute_div(r);
            break;
        default:
            log_error("invalid pcode=%ld", pcode);
    }
}

