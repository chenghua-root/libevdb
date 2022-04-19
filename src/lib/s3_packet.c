#include "lib/s3_packet.h"

#include <stddef.h>
#include <stdlib.h>
#include "lib/s3_buf.h"
#include "lib/s3_crc64.h"
#include "lib/s3_error.h"
#include "lib/s3_malloc.h"

S3Packet *s3_packet_construct() {
    S3Packet *p = s3_malloc_(S3_MOD_PACKET, sizeof(S3Packet));
    if (p != NULL) {
        *p = (S3Packet)s3_packet_null;
    }
    return p;
}

void s3_packet_desstruct(S3Packet *p) {
    if (p != NULL) {
        s3_packet_destroy(p);
        s3_free_(S3_MOD_PACKET, p);
    }
}

int s3_packet_init(S3Packet *p, int8_t pcode, uint64_t session_id,
                   int64_t time_out, void *data, int64_t data_len,
                   uint8_t data_owned) {
    p->header.pcode = pcode;
    p->header.session_id = session_id;
    p->header.start_time = 0; // TODO
    p->header.time_out = time_out;
    p->header.data_len = data_len;

    uint64_t header_crc = s3_crc64(&p->header, S3_PACKET_HEADER_LEN);
    p->header.header_crc = header_crc;

    p->data_buf = s3_buf_construct();
    assert(p->data_buf != NULL);
    s3_buf_init_with_data(p->data_buf, data, data_len, data_owned);
    p->data_crc = s3_crc64(data, data_len);

    return S3_OK;
}

int s3_packet_init_data(S3Packet *p, void *data, int64_t data_len,
                        uint64_t data_crc, uint8_t data_owned) {
    s3_must_be(p != NULL, S3_ERR_INVALID_ARG);
    s3_must_be(p->data_buf == NULL, S3_ERR_INVALID_ARG);

    p->data_buf = s3_buf_construct();
    assert(p->data_buf != NULL);
    s3_buf_init_with_data(p->data_buf, data, data_len, data_owned);
    p->data_crc = data_crc;

    return S3_OK;
}

void s3_packet_destroy(S3Packet *p) {
    if (p->data_buf != NULL) {
        s3_buf_destruct(p->data_buf);
    }
    *p = (S3Packet)s3_packet_null;
}

int s3_packet_out_buf_list(S3Packet *p, S3List *out_list) {
    S3Buf *head_buf = s3_buf_construct();
    s3_buf_init(head_buf, S3_PACKET_HEADER_LEN);
    memcpy(head_buf->data, &p->header, S3_PACKET_HEADER_LEN);
    head_buf->right += S3_PACKET_HEADER_LEN;

    S3Buf *data_crc_buf = s3_buf_construct();
    s3_buf_init(data_crc_buf, sizeof(p->data_crc));
    memcpy(data_crc_buf->data, &p->data_crc, sizeof(p->data_crc));
    data_crc_buf->right += sizeof(p->data_crc);

    s3_list_add_tail(&head_buf->node, out_list);
    s3_list_add_tail(&p->data_buf->node, out_list);
    p->data_buf = NULL;
    s3_list_add_tail(&data_crc_buf->node, out_list);

    return S3_OK;
}
