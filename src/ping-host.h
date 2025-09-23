
#ifndef PING_VIEWER_PING_HOST_H
#define PING_VIEWER_PING_HOST_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PING_TYPE_HOST ping_host_get_type()
G_DECLARE_FINAL_TYPE (PingHost, ping_host, PING, HOST, GObject)

PingHost *ping_host_new(void);

G_END_DECLS

char     *ping_host_get_hostname(PingHost *host);
char     *ping_host_get_address(PingHost *host);

#endif // PING_VIEWER_PING_HOST_H
