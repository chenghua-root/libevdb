#include "lib/s3_connection.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "lib/s3_packet_header.pb-c.h"

#define PORT 9000
#define IP "127.0.0.1"

S3Connection *s3_connection_construct() {
    S3Connection *conn = malloc(sizeof(S3Connection));
    if (conn != NULL) {
        *conn = (S3Connection)s3_connection_null;
    }
    return conn;
}

void s3_connection_desconstruct(S3Connection *conn) {
    if (conn != NULL) {
        s3_connection_destroy(conn);
        free(conn);
    }
}

int s3_connection_init(S3Connection *conn, struct ev_loop *loop, int fd) {
    conn->loop          = loop;
    conn->fd            = fd;
    s3_list_head_init(&conn->list_node);
    conn->read_watcher.data = conn;
    conn->write_watcher.data = conn;
}

void s3_connection_destroy(S3Connection *conn) {
    if (conn != NULL) {
        close(conn->fd);
        ev_io_stop(conn->loop, &conn->read_watcher);
        ev_io_stop(conn->loop, &conn->write_watcher);
    }
}

#define MAX_BUF_LEN 1024
void s3_connection_recv_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    char buf[MAX_BUF_LEN] = {0};
    int ret = 0;
    S3Connection *conn = (S3Connection*)w->data;

    do {
        ret = recv(w->fd, buf, MAX_BUF_LEN - 1, 0);
        if (ret > 0) {
            /*
             * FIXME:
             *   解包，必须提供长度参数。但同一个protobuf结构，成员值不同时，其长度不一.
             *   替换为其它数据交换格式
             */
            S3PacketHeader *header = s3_packet_header__unpack(NULL, ret, buf);
            printf("recv len=%d message: header.pcode=%d, .session_id=%ld, .data_len=%ld\n",
                    ret,
                    header->pcode,
                    header->session_id,
                    header->data_len);
            s3_packet_header__free_unpacked(header, NULL); // 释放空间

            ev_io_start(loop, &conn->write_watcher);
            return;
        }

        if (ret == 0) {
            printf("remote socket closed\n");
            break;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
        }

        break;
    } while(1);

    close(w->fd);
    ev_io_stop(loop, w);
    s3_connection_desconstruct(conn);
}

void s3_connection_write_socket_cb(struct ev_loop *loop, ev_io *w, int revents) {
    char buf[MAX_BUF_LEN] = {0};

    snprintf(buf, MAX_BUF_LEN - 1, "this is test message from libev\n");
    send(w->fd, buf, strlen(buf), 0);
    ev_io_stop(loop, w);
}
