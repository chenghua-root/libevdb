#include <assert.h>
#include "third/logc/log.h"
#include "lib/s3_crc64.h"
#include "lib/s3_packet.h"
#include "lib/s3_net_code.h"

void *s3_net_decode(S3Message *m) {
    S3Buf *in_buf = m->in_buf;
    if (s3_buf_unconsumed_size(in_buf) < S3_PACKET_HEADER_LEN) {
        m->next_read_len = S3_PACKET_HEADER_LEN - s3_buf_unconsumed_size(in_buf);
        m->read_status = S3_MSG_READ_STATUS_AGAIN;
        return NULL;
    }

    S3Packet *p = s3_packet_construct();
    assert(p != NULL);
    S3PacketHeader *header = &p->header;
    *header = *(S3PacketHeader*)(in_buf->left);
    in_buf->left += S3_PACKET_HEADER_LEN;

    uint64_t header_crc = header->header_crc;
    header->header_crc = 0;
    uint64_t cmpt_header_crc = s3_crc64(header, S3_PACKET_HEADER_LEN);
    assert(header_crc == cmpt_header_crc);
    header->header_crc = header_crc;

    if (s3_buf_unconsumed_size(in_buf) < header->data_len + sizeof(uint64_t)) {
        m->next_read_len = header->data_len + sizeof(uint64_t) - s3_buf_unconsumed_size(in_buf);
        m->read_status = S3_MSG_READ_STATUS_AGAIN;
        in_buf->left -= S3_PACKET_HEADER_LEN;
        s3_packet_desstruct(p);
        return NULL;
    }
    p->data = in_buf->left;
    in_buf->left += p->header.data_len;

    uint64_t data_crc = *(uint64_t*)(in_buf->left);
    in_buf->left += sizeof(uint64_t);
    uint64_t cmpt_data_crc = s3_crc64(p->data, header->data_len);
    assert(data_crc == cmpt_data_crc);

    m->next_read_len = 0;
    m->read_status = S3_MSG_READ_STATUS_INIT;

    log_info("decode packet, pcode=%ld session_id=%ld data_len=%ld header_crc=%ld data_crc=%ld",
             header->pcode, header->session_id, header->data_len, header_crc, data_crc);

    return p;
}
