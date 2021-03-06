#ifndef S3_LIB_RPC_H_
#define S3_LIB_RPC_H_

enum {
    S3_PACKET_CODE_START    = 100,

    S3_PACKET_CODE_ADD      = 101,
    S3_PACKET_CODE_ADD_RESP = 102,

    S3_PACKET_CODE_SUB      = 103,
    S3_PACKET_CODE_SUB_RESP = 104,

    S3_PACKET_CODE_MUL      = 105,
    S3_PACKET_CODE_MUL_RESP = 106,

    S3_PACKET_CODE_DIV      = 107,
    S3_PACKET_CODE_DIV_RESP = 108,
};

#endif
