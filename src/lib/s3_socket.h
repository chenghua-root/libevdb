#ifndef S3_LIB_SOCKET_H_
#define S3_LIB_SOCKET_H_

#define PORT 9000
#define IP "127.0.0.1"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

int s3_socket_create_listenfd();
int s3_socket_read(int fd, char *buf, int size);

#endif
