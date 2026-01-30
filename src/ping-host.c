/*
 * ping-host.c
 *
 * Defines the PingHost object which represents
 * a host to ping.
 */

#include "ping-host.h"

#include "glib-object.h"
#include "glib.h"

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef enum {
    PROP_NAME = 1,
    PROP_HOST_NAME,
    PROP_ADDRESS,
    N_PROPERTIES
} PingHostProperty;

struct _PingHost {
    GObject parent_instance;

    gchar* name;
    gchar* hostname;
    gchar* addr;

    struct sockaddr_in addr_in;
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL,};

G_DEFINE_TYPE (PingHost, ping_host, G_TYPE_OBJECT)

void ping_update_hostname(PingHost* host) {
    int s;

    struct in_addr _in_addr;
    if (inet_pton(AF_INET, host->addr, &_in_addr) == 0) {
        printf("%s isn't a valid IP address\n", host->addr);
        return;
    }

    memset(&host->addr_in, 0, sizeof( host->addr_in ));
    host->addr_in.sin_family = AF_INET;
    host->addr_in.sin_addr = _in_addr;

    struct sockaddr* addr = (struct sockaddr*)&host->addr_in;
    socklen_t addrlen = sizeof( host->addr_in );
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    s = getnameinfo(addr, addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
    if (s != 0) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        return;
    }

    GValue string = G_VALUE_INIT;
    g_value_init(&string, G_TYPE_STRING);
    g_value_set_string(&string, hbuf);

    g_object_set_property(G_OBJECT(host), "hostname", &string);
    g_value_unset(&string);
}

void ping_update_address(PingHost* host) {
    int s;
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;      /* Allow IPv4 only */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(host->hostname, NULL, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return;
    }

    host->addr_in = *(struct sockaddr_in *)result[0].ai_addr;

    GValue string = G_VALUE_INIT;
    g_value_init(&string, G_TYPE_STRING);
    g_value_set_string(&string, inet_ntoa(host->addr_in.sin_addr));

    g_object_set_property(G_OBJECT(host), "address", &string);
    g_value_unset(&string);
}

static void ping_host_set_property(GObject* obj, guint prop_id, const GValue *value, GParamSpec *spec) {
    PingHost* self = PING_HOST(obj);

    switch ((PingHostProperty) prop_id) {
        case PROP_NAME:
            g_free(self->name);
            self->name = g_value_dup_string(value);
            break;

        case PROP_HOST_NAME:
            g_free(self->hostname);
            self->hostname = g_value_dup_string(value);
            break;

        case PROP_ADDRESS:
            g_free(self->addr);
            self->addr = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
            break;
    }
}

static void ping_host_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *spec) {
    PingHost *self = PING_HOST(obj);

    switch ((PingHostProperty) prop_id) {
        case PROP_NAME:
            g_value_set_string(value, self->name);
            break;

        case PROP_HOST_NAME:
            g_value_set_string(value, self->hostname);
            break;

        case PROP_ADDRESS:
            g_value_set_string(value, self->addr);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
            break;
    }
}

static void ping_host_class_init(PingHostClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);

    object_class->set_property = ping_host_set_property;
    object_class->get_property = ping_host_get_property;

    obj_properties[PROP_NAME] = g_param_spec_string(
        "name", "Name", "name of the device", NULL, G_PARAM_READWRITE
    );

    obj_properties[PROP_HOST_NAME] = g_param_spec_string(
        "hostname", "HostName", "host name of the device", NULL, G_PARAM_READWRITE
    );

    obj_properties[PROP_ADDRESS] = g_param_spec_string(
        "address", "Address", "address of the device", NULL, G_PARAM_READWRITE
    );

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void ping_host_init(PingHost *self) {
    self->name = g_strdup("");
    self->addr = g_strdup("");
}

