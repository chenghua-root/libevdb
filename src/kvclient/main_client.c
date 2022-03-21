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

#include "lib/s3_packet_header.pb-c.h"

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

static void *routine(void *args)
{
    int fd;
    char buf[256];
    int ret;

    fd = create_clientfd("127.0.0.1", 9000);

    int i = 0;
    for (; ; ++i) {
        S3PacketHeader header = S3_PACKET_HEADER__INIT;
        header.pcode = 1;
        header.session_id = i;
        header.data_len = 0;

        size_t header_len = s3_packet_header__get_packed_size(&header);
        size_t pack_len = s3_packet_header__pack(&header, buf); // 打包
        if (header_len != pack_len) {
            printf("---------------------header_len=%d, pack_len=%d\n", header_len, pack_len);
            exit(1);
        }

        ret = write(fd, buf, pack_len);
        if (ret <= 0) {
            perror("write error\n");
        }
        printf("write ret=%d\n", ret);

        memset(buf, '\0', sizeof(buf));
        ret = read(fd, buf, sizeof(buf));
        fprintf(stdout, "pthreadid:%ld, len=%d, %s\n", pthread_self(), ret, buf);
        sleep(5);
    }
}

int main(void)
{
    pthread_t pids[4];

    for (int i = 0; i < sizeof(pids)/sizeof(pthread_t); ++i) {
        pthread_create(pids + i, NULL, routine, 0);
    }

    for (int i = 0; i < sizeof(pids)/sizeof(pthread_t); ++i) {
        pthread_join(pids[i], 0);
    }

    return 0;
}
