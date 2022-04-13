#include "lib/s3_message.h"

#include <assert.h>
#include <string.h>
#include "third/logc/log.h"
#include "lib/s3_request.h"

S3Message *s3_message_construct() {
    S3Message *m = malloc(sizeof(S3Message));
    if (m != NULL) {
        *m = (S3Message)s3_message_null;
    }
    return m;
}

void s3_message_destruct(S3Message *m) {
    if (m != NULL) {
        s3_message_destroy(m);
        free(m);
    }
}

int s3_message_init(S3Message *m) {
    S3Buf *in_buf = s3_buf_construct();
    assert(in_buf != NULL);
    s3_buf_init(in_buf, S3_MSG_BUF_LEN);
    m->in_buf = in_buf;

    s3_list_head_init(&m->message_list_node);
    s3_list_init(&m->request_list);
}

void s3_message_destroy(S3Message *m) {
    s3_list_del(&m->message_list_node);

    S3Request *r = NULL;
    S3Request *dummy = NULL;
    s3_list_for_each_entry_safe(r, dummy, &m->request_list, request_list_node) {
        s3_request_destruct(r);
    }

    log_info("message has req cnt=%d, done cnt=%d",
             m->request_cnt, m->request_done_cnt);
}

void s3_message_try_destroy(S3Message *m) {
    if (m->request_done_cnt < m->request_cnt) { // TODO, atomic
        return;
    }
    if (m->read_status != S3_MSG_READ_STATUS_DONE) {
        return;
    }
    assert(m->next_read_len == 0);
    assert(s3_buf_unconsumed_size(m->in_buf) == 0);

    s3_message_destroy(m);
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

    memcpy(m->in_buf->data, old_msg->in_buf->left, s3_buf_unconsumed_size(old_msg->in_buf));
    m->in_buf->right += s3_buf_unconsumed_size(old_msg->in_buf);
    old_msg->in_buf->right = old_msg->in_buf->left;

    return m;
}

