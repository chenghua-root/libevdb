#include "lib/s3_message.h"

#include <assert.h>
#include <string.h>
#include "third/logc/log.h"
#include "lib/s3_malloc.h"
#include "lib/s3_request.h"

S3Message *s3_message_construct() {
    S3Message *m = s3_malloc_(S3_MOD_MESSAGE, sizeof(S3Message));
    if (m != NULL) {
        *m = (S3Message)s3_message_null;
    }
    return m;
}

void s3_message_destruct(S3Message *m) {
    if (m != NULL) {
        s3_message_destroy(m);
        s3_free_(S3_MOD_MESSAGE, m);
    }
}

int s3_message_init(S3Message *m) {
    s3_list_head_init(&m->message_list_node);
    s3_list_init(&m->request_list);

    S3Buf *recv_buf = s3_buf_construct();
    assert(recv_buf != NULL);
    s3_buf_init(recv_buf, S3_MSG_BUF_LEN);
    m->recv_buf = recv_buf;
}

void s3_message_destroy(S3Message *m) {
    log_debug("destroy message, %s", s3_message_to_cstring(m));

    s3_list_del(&m->message_list_node);

    S3Request *r = NULL;
    S3Request *dummy = NULL;
    s3_list_for_each_entry_safe(r, dummy, &m->request_list, request_list_node) {
        s3_list_del(&r->request_list_node);
        s3_request_destruct(r);
    }
    s3_buf_destruct(m->recv_buf);
    m->recv_buf = NULL;
}

int s3_message_to_string(const S3Message *m, char *buf, const int64_t len, int64_t *pos) {
  return s3_buf_printf(buf, len, pos,
             "[read_status=%d, next_read_len=%lu, request_cnt=%u, request_done_cnt=%u]",
             m->read_status,
             m->next_read_len,
             m->request_cnt,
             m->request_done_cnt);
}

void s3_message_try_destruct(S3Message *m) {
    if (m->request_done_cnt < m->request_cnt) {
        return;
    }
    if (m->read_status != S3_MSG_READ_STATUS_DONE) {
        return;
    }
    assert(m->next_read_len == 0);
    assert(s3_buf_unconsumed_size(m->recv_buf) == 0);

    s3_list_del(&m->message_list_node);
    s3_message_destruct(m);
}

S3Message *s3_message_create() {
    S3Message *m = s3_message_construct();
    assert(m != NULL);
    s3_message_init(m);
    return m;
}

S3Message *s3_message_create_with_old(S3Message *old_msg) {
    S3Message *m = s3_message_construct();
    assert(m != NULL);
    s3_message_init(m);

    memcpy(m->recv_buf->data, old_msg->recv_buf->left, s3_buf_unconsumed_size(old_msg->recv_buf));
    m->recv_buf->right += s3_buf_unconsumed_size(old_msg->recv_buf);
    old_msg->recv_buf->right = old_msg->recv_buf->left;
    old_msg->next_read_len = 0;
    old_msg->read_status = S3_MSG_READ_STATUS_DONE;

    return m;
}

