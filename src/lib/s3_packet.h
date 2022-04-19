#ifndef S3_LIB_PACKET_H_
#define S3_LIB_PACKET_H_

#include <stdint.h>
#include "lib/s3_buf.h"
#include "lib/s3_list.h"

typedef struct S3PacketHeader S3PacketHeader;
struct S3PacketHeader {
    int8_t   pcode;
    uint64_t session_id;
    uint64_t start_time; // usecond
    uint64_t time_out;   // +usecond
    uint64_t data_len;
    uint64_t preserved1;
    uint64_t header_crc; // compute header crc with set header_crc = 0
};
#define s3_packet_header_null {    \
    .pcode = 0,                    \
    .session_id = 0,               \
    .start_time = 0,               \
    .time_out = 0,                 \
    .data_len = 0,                 \
    .preserved1 = 0,               \
    .header_crc = 0,               \
}
static int S3_PACKET_HEADER_LEN = sizeof(struct S3PacketHeader);

typedef struct S3Packet S3Packet;
struct S3Packet {
    S3PacketHeader header;
    S3Buf          *data_buf;
    uint64_t       data_crc;
};
#define s3_packet_null {               \
    .header   = s3_packet_header_null, \
    .data_buf = NULL,                  \
    .data_crc = 0,                     \
}

S3Packet *s3_packet_construct();
void s3_packet_desstruct(S3Packet *p);
int s3_packet_init(S3Packet *p, int8_t pcode, uint64_t session_id,
                   int64_t time_out, void *data, int64_t data_len,
                   uint8_t data_owned);
void s3_packet_destroy(S3Packet *p);

int s3_packet_init_data(S3Packet *p, void *data, int64_t data_len, uint64_t data_crc, uint8_t data_owned);
int s3_packet_out_buf_list(S3Packet *p, S3List *out_list);

#endif
