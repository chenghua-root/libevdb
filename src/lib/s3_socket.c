#include "lib/s3_socket.h"

#include <assert.h>
#include <errno.h>
#include <strings.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include "third/logc/log.h"
#include "lib/s3_buf.h"
#include "lib/s3_define.h"
#include "lib/s3_error.h"

int s3_socket_set_non_blocking(int fd) {
    int flags = 1;
    return ioctl(fd, FIONBIO, &flags);
}

int s3_socket_create_listenfd() {
    struct sockaddr_in addr;
    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);

    if(s == -1){
        perror("create socket error \n");
        return -1;
    }

    int so_reuseaddr = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
    bzero(&addr, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);

    if(bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1){
        perror("bind socket error \n");
        return -1;
    }

    if(listen(s, 32) == -1){
        perror("listen socket error\n");
        return -1;
    }
    log_info("bind %s, listen %d\n", IP, PORT);

    return s;
}

int s3_socket_read(int fd, char *buf, int size) {
  int n = 0;
  do {
    n = recv(fd, buf, size, 0);
  } while (n == -1 && errno == EINTR);

  if (n < 0) {
    n = ((errno == EAGAIN) ? S3_ERR_NET_AGAIN : S3_ERR_NET_ABORT);
  }

  return n;
}

static int s3_socket_writev(int fd, struct iovec *iovs, int cnt) {
    int n = 0;
    do {
        if (cnt == 1) {
            n = send(fd, iovs[0].iov_base, iovs[0].iov_len, 0);
        } else {
            n = writev(fd, iovs, cnt);
        }
    } while (n == -1 && errno == EINTR);

    return n;
}

#define S3_IOV_MAX      256
#define S3_IOV_MAX_SIZE (256*1024)
int s3_socket_write(int fd, S3List *buf_list) {
    s3_must_be(s3_list_inited(buf_list), S3_ERR_INVALID_ARG);

    int ret = S3_OK;
    struct iovec iovs[S3_IOV_MAX];
    int send_size = 0;
    int size = 0;
    int cnt = 0;

    S3Buf *b = NULL, *dummy = NULL;
    s3_list_for_each_entry(b, buf_list, node) {
        size = s3_buf_unconsumed_size(b);
        iovs[cnt].iov_base = b->left;
        iovs[cnt].iov_len = size;
        ++cnt;
        send_size += size;

        if (cnt >= S3_IOV_MAX || send_size >= S3_IOV_MAX_SIZE) {
            break;
        }
    }

    if (cnt == 0) {
        return 0;
    }

    ret = s3_socket_writev(fd, iovs, cnt);
    if (ret < 0) {
        log_error("socket write error. ret=%d, errno=%d", ret, errno);
        ret = ((errno == EAGAIN) ? S3_ERR_NET_AGAIN : S3_ERR_NET_ABORT);
        return ret;
    }

    int sended = ret;
    int check_cnt = 0;
    s3_list_for_each_entry_safe(b, dummy, buf_list, node) {
        size = s3_buf_unconsumed_size(b);

        if (sended < size) {
            b->left += sended;
            sended = 0;
            break;
        }

        s3_list_del(&b->node);
        s3_buf_destruct(b);
        sended -= size;
        ++check_cnt;
    }
    assert(cnt == check_cnt);
    return ret;
}
