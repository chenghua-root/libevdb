#include <stddef.h>
#include <stdlib.h>
#include "lib/s3_packet.h"

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

int s3_packet_init(S3Packet *p, void *data, uint64_t data_crc) {
    p->data = data;
    p->data_crc = data_crc;
}

void s3_packet_destroy(S3Packet *p) {
    /*
     * notice: packet->data used message.in_buf, do not free here
     */
    *p = (S3Packet)s3_packet_null;
}
