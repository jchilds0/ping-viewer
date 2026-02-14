/*
 * ping.c 
 *
 * util functions for pinging hosts
 *
 */

#include "ping.h"

#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <sys/socket.h>

#include "gio/gio.h"
#include "ping-viewer.h"

#define BUFSIZE       2048

GSocket *ping_socket(char* addr, int domain, int proto, GError** error) {
    int sock = socket(domain, SOCK_DGRAM, proto);
    if (socket < 0) {
        return NULL;
    }

    setsockopt(sock, proto, IP_RECVTTL, &(int){1}, sizeof( int ));

    GSocket* g_sock = g_socket_new_from_fd(sock, error);
    if (*error != NULL) {
        close(sock);
        return NULL;
    }

    g_socket_set_blocking(g_sock, true);
    return g_sock;
}

int ping_send(GSocket* sock, GSocketAddress* sockaddr, int seq_no, GError** error) {
    struct icmphdr icmp_hdr;
    char data[BUFSIZE];
    gsize size = sizeof( icmp_hdr );

    memset(&icmp_hdr, 0, sizeof icmp_hdr);
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.un.echo.id = 1;

    icmp_hdr.un.echo.sequence = seq_no;
    memcpy(data, &icmp_hdr, sizeof icmp_hdr);

    return g_socket_send_to(sock, sockaddr, data, size, NULL, error);
}

void ping_recv(GSocket* sock, gint timeout, GSocketAddress** rcv_addr, int* seq_no, int* ttl, GError** error) {
    g_socket_set_timeout(sock, timeout);

    GSocketControlMessage** messages;
    gint num_messages = 0;
    gint flags = 0;

    g_socket_receive_message(sock, rcv_addr, NULL, 0, &messages, &num_messages, &flags, NULL, error);
    if (error != NULL) {
        return;
    }

    for (size_t i = 0; i < num_messages; i++) {
        GSocketControlMessage* cmsg = messages[i];

        if (g_socket_control_message_get_level(cmsg) != IPPROTO_IP) {
            goto CONTINUE;
        }

        if (g_socket_control_message_get_level(cmsg) != IP_TTL) {
            goto CONTINUE;
        }

        if (g_socket_control_message_get_size(cmsg) != sizeof( int )) {
            ping_log("invalid control message size");
            *ttl = 0;
            goto CONTINUE;
        }

        g_socket_control_message_serialize(cmsg, ttl);

    CONTINUE:
        g_object_unref(cmsg);
    }

    g_free(messages);
    return;
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
