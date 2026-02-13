/*
 * ping.c 
 *
 * util functions for pinging hosts
 *
 */

#include "ping.h"

#include <stdio.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define BUFSIZE       2048

int ping_addr(const char *addr, int n, struct sockaddr_in *sock_addr) {
    struct in_addr _in_addr;
    if (inet_pton(AF_INET, addr, &_in_addr) == 0) {
        printf("%s isn't a valid IP address\n", addr);
        return -1;
    }

    memset(sock_addr, 0, n);
    sock_addr->sin_family = AF_INET;
    sock_addr->sin_addr = _in_addr;
    return 0;
}

int ping_send(int sock, struct sockaddr* addr, int addr_len, int seq_no) {
    struct icmphdr icmp_hdr;
    unsigned char data[BUFSIZE];

    memset(&icmp_hdr, 0, sizeof icmp_hdr);
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.un.echo.id = 1;

    icmp_hdr.un.echo.sequence = seq_no;
    memcpy(data, &icmp_hdr, sizeof icmp_hdr);

    return sendto(sock, data, sizeof icmp_hdr, 0, addr, addr_len);
}

int ping_recv(int sock, struct timeval timeout, struct sockaddr* rcv_addr, int* seq_no, int* ttl) {
    int rc;
    fd_set read_set = {0};

    const size_t largestPacketExpected = 1500;
    uint8_t buffer[largestPacketExpected];
    struct iovec iov[1] = { { buffer, sizeof(buffer) } };
    struct sockaddr_storage srcAddress;
    uint8_t ctrlDataBuffer[CMSG_SPACE(sizeof(uint8_t))];

    struct msghdr hdr = {
        .msg_name = &srcAddress,
        .msg_namelen = sizeof(srcAddress),
        .msg_iov = iov,
        .msg_iovlen = 1,
        .msg_control = ctrlDataBuffer,
        .msg_controllen = sizeof(ctrlDataBuffer)
    };

    FD_SET(sock, &read_set);

    // wait for a reply with a timeout
    rc = select(sock + 1, &read_set, NULL, NULL, &timeout);
    if (rc <= 0) {
        return -1;
    }

    rc = recvmsg(sock, &hdr, 0);
    if (rc <= 0) {
        return -1;
    }

    *rcv_addr = *(struct sockaddr *)&srcAddress;

    struct cmsghdr *cmsg;
    for (cmsg = CMSG_FIRSTHDR(&hdr); cmsg != NULL; cmsg = CMSG_NXTHDR(&hdr, cmsg)) {
        if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL) {
            memcpy(ttl, CMSG_DATA(cmsg), sizeof(*ttl));
            break;
        }
    }

    if (cmsg == NULL) {
        printf("IP_RECVTTL not enabled\n");
        return -1;
    }

    return 0;
}

void ping_free(gpointer data) {
    if (data == NULL) {
        return;
    }

    ping_t* ping = data;
    g_free(ping->msg);
    g_free(ping->reply_addr);
    g_free(ping);
}
