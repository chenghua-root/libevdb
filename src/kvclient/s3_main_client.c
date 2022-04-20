#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "third/logc/log.h"
#include "lib/s3_crc64.h"
#include "lib/s3_packet.h"
#include "lib/s3_packet.pb-c.h"
#include "lib/s3_rpc.h"

#include "lib/s3_message.h"
#include "lib/s3_buf.h"
#include "lib/s3_socket.h"
#include "lib/s3_net_code.h"
#include "lib/s3_atomic.h"

#define err_message(msg) \
    do {perror(msg); exit(EXIT_FAILURE);} while(0)

static int create_clientfd(char const *addr, uint16_t u16port) {
    int fd;
    struct sockaddr_in server;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) err_message("socket err\n");

    server.sin_family = AF_INET;
    server.sin_port = htons(u16port);
    inet_pton(AF_INET, addr, &server.sin_addr);

    if (connect(fd, (struct sockaddr *)&server, sizeof(server)) < 0) perror("connect err\n");

    return fd;
}

typedef struct RecvRoutineArg RecvRoutineArg;
struct RecvRoutineArg {
    int fd;
    uint64_t doing_req_cnt;
};

static void *recv_routine(void *args) {
    RecvRoutineArg *arg = args;

    int fd = arg->fd;
    S3List message_list = S3_LIST_INIT(message_list);
    while (1) {
        S3Message *m = s3_list_get_last(&message_list, S3Message, message_list_node);

        if (m == NULL) {
            m = s3_message_create();
            assert(m != NULL);
            s3_list_add_tail(&m->message_list_node, &message_list);
        } else if ((s3_buf_free_size(m->recv_buf) < S3_MSG_BUF_MIN_LEN) ||
                   (m->read_status == S3_MSG_READ_STATUS_AGAIN && m->next_read_len > s3_buf_free_size(m->recv_buf))) {

            log_info("message free not enough, next_read_len=%ld, free size=%ld",
                    m->next_read_len, s3_buf_free_size(m->recv_buf));
            S3Message *old_msg = m;

            m = s3_message_create_with_old(old_msg);
            assert(m != NULL);
            s3_list_add_tail(&m->message_list_node, &message_list);

            s3_message_try_destruct(old_msg);
        }

        int n = s3_socket_read(fd, m->recv_buf->right, s3_buf_free_size(m->recv_buf));
        if (n == 0) {
            log_info("socket closed");
            close(fd);
            return NULL;
        } else if (n < 0) {
            log_info("recv len error, ret=%d, errno=%d\n", n, errno);
            return NULL;
        }
        m->recv_buf->right += n;
        //log_info("recv len=%d\n", n);

        while(1) {
            S3Packet *p = s3_net_decode(m);
            if (p == NULL) {
                break;
            }
            switch(p->header.pcode) {
                case S3_PACKET_CODE_ADD_RESP:
                {
                    S3AddResp *add_resp = s3_add_resp__unpack(NULL, p->header.data_len, p->data_buf->data);
                    //log_info("add resp. session_id=%ld result=%d val=%ld\n", \
                              p->header.session_id, add_resp->result, add_resp->val);
                    s3_add_resp__free_unpacked(add_resp, NULL);
                    break;
                }
                case S3_PACKET_CODE_MUL_RESP:
                {
                    S3MulResp *resp = s3_mul_resp__unpack(NULL, p->header.data_len, p->data_buf->data);
                    log_info("mul resp. session_id=%ld result=%d val=%ld\n", \
                              p->header.session_id, resp->result, resp->val);
                    s3_mul_resp__free_unpacked(resp, NULL);
                    break;
                }
            }
            s3_packet_desstruct(p);
            s3_atomic_dec(&arg->doing_req_cnt);
        }
    }
}

static void *routine(void *args) {
    int fd;
    char buf[256];
    int ret;

    fd = create_clientfd("127.0.0.1", 9000);
    RecvRoutineArg arg = {.fd = fd, .doing_req_cnt = 0};

    pthread_t tid;
    pthread_create(&tid, NULL, recv_routine, &arg);

    int i = 0;
    for (; ; ++i) {
        S3AddReq req = S3_ADD_REQ__INIT;
        //S3MulReq req = S3_MUL_REQ__INIT;
        req.a = i;
        req.b = i+1;

        size_t data_len = s3_add_req__get_packed_size(&req);
        size_t pack_len = s3_add_req__pack(&req, buf);
        //size_t data_len = s3_mul_req__get_packed_size(&req);
        //size_t pack_len = s3_mul_req__pack(&req, buf);

        assert(data_len == pack_len);
        uint64_t data_crc = s3_crc64(buf, pack_len);

        S3PacketHeader header = s3_packet_header_null;
        header.pcode = S3_PACKET_CODE_ADD;
        //header.pcode = S3_PACKET_CODE_MUL;
        header.session_id = i;
        header.data_len = pack_len;
        uint64_t header_crc = s3_crc64(&header, sizeof(S3PacketHeader));
        header.header_crc = header_crc;

        ret = write(fd, &header, sizeof(S3PacketHeader));
        if (ret <= 0) {
            perror("write header error\n");
        }
        //printf("write head size=%d\n", ret);

        ret = write(fd, buf, 1);
        ret = write(fd, buf+1, pack_len-1);
        if (ret <= 0) {
            perror("write data error\n");
        }
        //printf("write body size=%d\n", ret+1);

        ret = write(fd, &data_crc, sizeof(data_crc));
        if (ret <= 0) {
            perror("write data crc error\n");
            return NULL;
        }
        //printf("write req, a=%ld, b=%ld\n", req.a, req.b);

        uint64_t cnt = s3_atomic_inc(&arg.doing_req_cnt);
        //sleep(2);
        if (cnt >= 20000) {
            printf("doing_req_cnt=%lu\n", cnt);
            usleep(1000000);
        }
    }
}

int main() {
    pthread_t pids[4];

    for (int i = 0; i < sizeof(pids) / sizeof(pthread_t); ++i) {
        pthread_create(pids + i, NULL, routine, 0);
    }

    for (int i = 0; i < sizeof(pids) / sizeof(pthread_t); ++i) {
        pthread_join(pids[i], 0);
    }

    return 0;
}
