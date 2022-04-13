#include "lib/s3_packet.h"

#include <stddef.h>
#include <stdlib.h>
#include "lib/s3_buf.h"
#include "lib/s3_crc64.h"
#include "lib/s3_error.h"

S3Packet *s3_packet_construct() {
    S3Packet *p = malloc(sizeof(S3Packet));
    if (p != NULL) {
        *p = (S3Packet)s3_packet_null;
    }
    return p;
}

void s3_packet_desstruct(S3Packet *p) {
    if (p != NULL) {
        free(p);
    }
}

int s3_packet_init(S3Packet *p, int8_t pcode, uint64_t session_id,
                   int64_t time_out, int64_t data_len, void *data) {
    p->header.pcode = pcode;
    p->header.session_id = session_id;
    p->header.start_time = 0; // TODO
    p->header.time_out = time_out;
    p->header.data_len = data_len;

    uint64_t header_crc = s3_crc64(&p->header, S3_PACKET_HEADER_LEN);
    p->header.header_crc = header_crc;

    p->data = data;
    p->data_crc = s3_crc64(data, data_len);

    return S3_OK;
}

void s3_packet_destroy(S3Packet *p) {
    /*
     * notice: packet->data used message.in_buf, do not free here
     */
    *p = (S3Packet)s3_packet_null;
}

int s3_packet_out_buf_list(S3Packet *p, S3List *out_list) {
    S3Buf *head_buf = s3_buf_construct();
    s3_buf_init(head_buf, S3_PACKET_HEADER_LEN);
    memcpy(head_buf->data, &p->header, S3_PACKET_HEADER_LEN);
    head_buf->right += S3_PACKET_HEADER_LEN;

    S3Buf *data_buf = s3_buf_construct();
    s3_buf_init_with_data(data_buf, p->data, p->header.data_len);
    p->data = NULL;

    S3Buf *data_crc_buf = s3_buf_construct();
    s3_buf_init(data_crc_buf, sizeof(p->data_crc));
    memcpy(head_buf->data, &p->header, sizeof(p->data_crc));
    head_buf->right += sizeof(p->data_crc);

    s3_list_add_tail(&head_buf->node, out_list);
    s3_list_add_tail(&data_buf->node, out_list);
    s3_list_add_tail(&data_crc_buf->node, out_list);

    return S3_OK;
}
