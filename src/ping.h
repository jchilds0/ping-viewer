#ifndef PING_VIEWER_PING_H
#define PING_VIEWER_PING_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct ping_s {
    bool succeeded;
    int64_t ttl;
} ping_t;

int ping_addr(const char *addr, int n, struct sockaddr_in *sock_addr);
int ping_send(int sock, struct sockaddr *addr, int addr_len, int seq_no);
int ping_recv(int sock, struct timeval timeout, struct sockaddr *rcv_addr, socklen_t *rcv_addr_len, int *seq_no);

#endif  // PING_VIEWER_PING_H
