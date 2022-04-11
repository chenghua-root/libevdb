#ifndef S3_LIB_PACKET_H_
#define S3_LIB_PACKET_H_

#include <stdint.h>

typedef struct S3PacketHeaderV2 S3PacketHeaderV2;
struct S3PacketHeaderV2 {
    int8_t   pcode;
    uint64_t session_id;
    int64_t  start_time; // usecond
    int64_t  time_out;   // +usecond
    uint64_t data_len;
    uint64_t preserved1;
    uint64_t header_crc; // compute header crc with set header_crc = 0
};
#define s3_packet_header_v2_null { \
    .pcode = 0,                    \
    .session_id = 0,               \
    .start_time = 0,               \
    .time_out = 0,                 \
    .data_len = 0,                 \
    .preserved1 = 0,               \
    .header_crc = 0,               \
}
static int S3_PACKET_HEADER_LEN = sizeof(struct S3PacketHeaderV2);

typedef struct S3Packet S3Packet;
struct S3Packet {
    S3PacketHeaderV2 header;
    void             *data;
    uint64_t         data_crc;
};
#define s3_packet_null {                  \
    .header   = s3_packet_header_v2_null, \
    .data     = NULL,                     \
    .data_crc = 0,                        \
}

S3Packet *s3_packet_construct();
void s3_packet_desstruct(S3Packet *p);
int s3_packet_init(S3Packet *p, void *data, uint64_t data_crc);
void s3_packet_destroy(S3Packet *p);

#endif
