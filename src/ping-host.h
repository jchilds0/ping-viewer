
#ifndef PING_VIEWER_PING_HOST_H
#define PING_VIEWER_PING_HOST_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PING_TYPE_HOST ping_host_get_type()
G_DECLARE_FINAL_TYPE (PingHost, ping_host, PING, HOST, GObject)

PingHost *ping_host_new(void);
void ping_update_hostname(PingHost* host);
void ping_update_address(PingHost* host);

G_END_DECLS

#endif // PING_VIEWER_PING_HOST_H
