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

int ping_send(int sock, struct sockaddr *addr, int addr_len, int seq_no) {
    struct icmphdr icmp_hdr;
    unsigned char data[BUFSIZE];

    memset(&icmp_hdr, 0, sizeof icmp_hdr);
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.un.echo.id = 1;

    icmp_hdr.un.echo.sequence = seq_no;
    memcpy(data, &icmp_hdr, sizeof icmp_hdr);

    int rc = sendto(sock, data, sizeof icmp_hdr, 0, addr, addr_len);
    if (rc <= 0) {
        perror("ping send");
        return -1;
    }

    return 0;
}

int ping_recv(int sock, struct timeval timeout, struct sockaddr *rcv_addr, socklen_t *rcv_addr_len, int *seq_no) {
    unsigned char data[BUFSIZE];

    fd_set read_set;
    memset(&read_set, 0, sizeof read_set);
    FD_SET(sock, &read_set);

    // wait for a reply with a timeout
    int rc = select(sock + 1, &read_set, NULL, NULL, &timeout);
    if (rc == 0) {
        puts("Got no reply\n");
        return -1;
    } else if (rc < 0) {
        perror("Select");
        return -1;
    }

    struct icmphdr rcv_hdr;
    rc = recvfrom(sock, data, sizeof data, 0, rcv_addr, rcv_addr_len);
    if (rc <= 0) {
        perror("recvfrom");
        return -1;
    } else if (rc < sizeof rcv_hdr) {
        printf("Error, got short ICMP packet, %d bytes\n", rc);
        return -1;
    }

    memcpy(&rcv_hdr, data, sizeof rcv_hdr);
    if (rcv_hdr.type == ICMP_ECHOREPLY) {
        *seq_no = rcv_hdr.un.echo.sequence;
        printf("ICMP reply, id=0x%x, sequence =  0x%x\n", rcv_hdr.un.echo.id, rcv_hdr.un.echo.sequence);
        return 0;
    } else {
        printf("Got ICMP packet with type 0x%x ?!?\n", rcv_hdr.type);
        return -1;
    }
}

int ping_loop(void) {
    struct sockaddr_in sock_addr;
    const char addr[] = "1.1.1.1";

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        return sock;
    }

    if (ping_addr(addr, sizeof( addr ), &sock_addr) < 0) {
        return 1;
    }

    struct timeval timeout = {3, 0};
    if (ping_send(sock, (struct sockaddr *)&sock_addr, sizeof( sock_addr ), 0) < 0) {
        return 1;
    }

    struct sockaddr_in rcv_addr = {0};
    socklen_t rcv_addr_len = sizeof rcv_addr;
    int seq_no = 0;

    if (ping_recv(sock, timeout, (struct sockaddr *)&rcv_addr, &rcv_addr_len, &seq_no) < 0) {
        return 1;
    }

    char rcv_buf[2048];
    memset(rcv_buf, 0, sizeof rcv_buf);
    inet_ntop(AF_INET, &(rcv_addr.sin_addr), rcv_buf, rcv_addr_len);
    printf("ICMP echo reply from %s, seq no %d\n", rcv_buf, seq_no);
    return 0;
}

