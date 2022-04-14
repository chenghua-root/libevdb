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

#define err_message(msg) \
    do {perror(msg); exit(EXIT_FAILURE);} while(0)

static int create_clientfd(char const *addr, uint16_t u16port)
{
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

static void *recv_routine(void *args) {
    int fd = *(int*)args;
    S3List message_list = S3_LIST_HEAD_INIT(message_list);
    while (1) {
        S3Message *m = s3_list_get_last(&message_list, S3Message, message_list_node);
        S3Message *old_msg = NULL;
        if (m == NULL) {
            m = s3_message_create();
            assert(m != NULL);
            s3_list_add_tail(&m->message_list_node, &message_list);
        } else if (m->read_status == S3_MSG_READ_STATUS_AGAIN &&
                   m->next_read_len > s3_buf_free_size(m->in_buf)) {
            old_msg = m;
            old_msg->read_status = S3_MSG_READ_STATUS_DONE;
            m = s3_message_create_with_old(old_msg);
            assert(m != NULL);
            s3_list_add_tail(&m->message_list_node, &message_list);
            perror("unexpect one");
        } else if (s3_buf_free_size(m->in_buf) < S3_MSG_BUF_MIN_LEN) {
            old_msg = m;
            old_msg->read_status = S3_MSG_READ_STATUS_DONE;
            m = s3_message_create();
            assert(m != NULL);
            s3_list_add_tail(&m->message_list_node, &message_list);
            perror("unexpect two");
        }
        int n = s3_socket_read(fd, m->in_buf->right, s3_buf_free_size(m->in_buf));
        m->in_buf->right += n;
        printf("recv len=%d\n", n);

        while(1) {
            S3Packet *p = s3_net_decode(m);
            if (p == NULL) {
                break;
            }
            switch(p->header.pcode) {
                case S3_PACKET_CODE_ADD_RESP:
                    {
                    S3AddResp *add_resp = s3_add_resp__unpack(NULL, p->header.data_len, p->data);
                    log_info("------------------------------------add resp. session_id=%ld result=%d val=%ld\n",
                             p->header.session_id, add_resp->result, add_resp->val);
                    s3_add_resp__free_unpacked(add_resp, NULL);
                    }
                    break;
            }
        }
    }
}

static void *routine(void *args) {
    int fd;
    char buf[256];
    char recv_buf[256];
    int ret;

    fd = create_clientfd("127.0.0.1", 9000);

    pthread_t tid;
    pthread_create(&tid, NULL, recv_routine, &fd);

    int i = 0;
    for (; ; ++i) {
        S3AddReq req = S3_ADD_REQ__INIT;
        req.a = i;
        req.b = i+1;
        size_t data_len = s3_add_req__get_packed_size(&req);
        size_t pack_len = s3_add_req__pack(&req, buf);
        assert(data_len == pack_len);
        uint64_t data_crc = s3_crc64(buf, pack_len);

        S3PacketHeader header = s3_packet_header_null;
        header.pcode = S3_PACKET_CODE_ADD;
        header.session_id = i;
        header.data_len = pack_len;
        uint64_t header_crc = s3_crc64(&header, sizeof(S3PacketHeader));
        header.header_crc = header_crc;

        ret = write(fd, &header, sizeof(S3PacketHeader));
        if (ret <= 0) {
            perror("write header error\n");
        }
        printf("write head size=%d\n", ret);
        //sleep(1);

        ret = write(fd, buf, 1);
        //sleep(1);
        ret = write(fd, buf+1, pack_len-1);
        if (ret <= 0) {
            perror("write data error\n");
        }
        printf("write body size=%d\n", ret+1);

        ret = write(fd, &data_crc, sizeof(data_crc));
        if (ret <= 0) {
            perror("write data crc error\n");
        }

        printf("write add req, a=%ld, b=%ld\n", req.a, req.b);

        /*
        int n = recv(fd, recv_buf, 256, 0);
        printf("recv len=%d\n", n);
        */

        if (i%10 == 0) {
            sleep(10);
        }
    }
}

int main(void)
{
    pthread_t pids[2];

    for (int i = 0; i < sizeof(pids)/sizeof(pthread_t); ++i) {
        pthread_create(pids + i, NULL, routine, 0);
    }

    for (int i = 0; i < sizeof(pids)/sizeof(pthread_t); ++i) {
        pthread_join(pids[i], 0);
    }

    return 0;
}
