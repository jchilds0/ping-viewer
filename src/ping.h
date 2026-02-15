#ifndef PING_VIEWER_PING_H
#define PING_VIEWER_PING_H

#include "glib.h"
#include "gio/gio.h"
#include <netinet/in.h>
#include <stdbool.h>

typedef struct ping_s {
    bool succeeded;
    int64_t ttl;
    gchar* msg;
    gchar* reply_addr;
} ping_t;

int ping_family_to_protocol(GSocketFamily family);
GSocket *ping_socket(char* addr, int domain, int proto, GError** error);
int ping_send(GSocket* sock, GSocketAddress* sockaddr, int seq_no, GError** error);
void ping_recv(GSocket* sock, gint timeout, GSocketAddress** rcv_addr, int* seq_no, int* ttl, GError** error);
void ping_free(gpointer data);

#endif  // PING_VIEWER_PING_H
