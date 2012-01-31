#ifndef UDP_LIB_POSIX_IMPL_H
#define UDP_LIB_POSIX_IMPL_H

#include <sys/socket.h>
#include <netinet/in.h>

#define POSIX_UDP_DATA_INIT() { \
   .sock                = -1, \
   .socketAddress       = {0}, \
   .socketAddressLength = 0, \
   .clientAddr          = {0}, \
   .clientAddr_len      = 0, \
   .rcvAddrPtr          = NULL, \
   .rcvAddrLenPtr       = NULL, \
}

typedef struct {
    int sock;
    struct sockaddr_in socketAddress;
    socklen_t socketAddressLength;

    struct sockaddr_in clientAddr;
    socklen_t clientAddr_len;

    struct sockaddr_in *rcvAddrPtr;
    socklen_t *rcvAddrLenPtr;
}
PosixUdpData;

#endif //UDP_LIB_POSIX_IMPL_H
