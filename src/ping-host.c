/*
 * ping-host.c
 *
 * Defines the PingHost object which represents
 * a host to ping.
 */

#include "ping-host.h"

#include "glib-object.h"
#include "glib.h"

#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "ping.h"

#define PING_FREQUENCY      10
#define PING_TIMEOUT        1000

typedef enum {
    PROP_NAME = 1,
    PROP_HOST_NAME,
    PROP_ADDRESS,
    PROP_REPLY_ADDRESS,
    PROP_SUCCEEDED_COUNT,
    PROP_FAILED_COUNT,
    PROP_CONSECUTIVE_FAILED_COUNT,
    PROP_MAX_CONSECUTIVE_FAILED_COUNT,
    PROP_MAX_CONSECUTIVE_FAILED_TIME,
    PROP_TOTAL_PING_COUNT,
    PROP_PERCENTAGE_FAILED,
    PROP_LAST_PING_STATUS,
    PROP_LAST_PING_TIME,
    PROP_LAST_PING_TTL,
    PROP_AVERAGE_PING_TIME,
    PROP_LAST_SUCCEEDED_ON,
    PROP_LAST_FAILED_ON,
    PROP_MINIMUM_PING_TIME,
    PROP_MAXIMUM_PING_TIME,
    N_PROPERTIES
} PingHostProperty;

struct _PingHost {
    GObject parent_instance;

    gchar* name;
    gchar* hostname;
    gchar* addr;
    gchar* replay_addr;
    int64_t succeeded_count;
    int64_t failed_count;
    int64_t cons_failed_count;
    int64_t max_cons_failed_count;
    gchar* max_cons_failed_time;
    int64_t total_ping_count;
    double percentage_failed;
    gchar* last_ping_status;
    gchar* last_ping_time;
    int64_t last_ping_ttl;
    gchar* avg_ping_time;
    gchar* last_succeeded_on;
    gchar* last_failed_on;
    gchar* min_ping_time;
    gchar* max_ping_time;

    gint exit;
    GArray* pings;
    GThread* thread;
    GRWLock lock;

    struct sockaddr_in addr_in;
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL,};

G_DEFINE_TYPE (PingHost, ping_host, G_TYPE_OBJECT)

static void ping_host_dispose(GObject* self);
static void ping_host_set_property(GObject* obj, guint prop_id, const GValue *value, GParamSpec *spec);
static void ping_host_get_property(GObject* obj, guint prop_id, GValue *value, GParamSpec *spec);

static gpointer ping_host_loop(gpointer data);

static void ping_host_class_init(PingHostClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);

    object_class->dispose      = ping_host_dispose;

    object_class->set_property = ping_host_set_property;
    object_class->get_property = ping_host_get_property;

    obj_properties[PROP_NAME] = g_param_spec_string(
        PROPERTY_NAME, "Name", "name of the device", NULL, G_PARAM_READWRITE
    );

    obj_properties[PROP_HOST_NAME] = g_param_spec_string(
        PROPERTY_HOST_NAME, "HostName", "host name of the device", NULL, G_PARAM_READWRITE
    );

    obj_properties[PROP_ADDRESS] = g_param_spec_string(
        PROPERTY_ADDRESS, "Address", "address of the device", NULL, G_PARAM_READWRITE
    );

    obj_properties[PROP_REPLY_ADDRESS] = g_param_spec_string(
        PROPERTY_REPLY_ADDRESS, "ReplyAddress", "reply address from last ping", NULL, G_PARAM_READWRITE
    );

    obj_properties[PROP_SUCCEEDED_COUNT] = g_param_spec_int64(
        PROPERTY_SUCCEEDED_COUNT, "SucceededCount", "number of succeeded pings", 0, INT64_MAX, 0, G_PARAM_READWRITE
    );

    obj_properties[PROP_FAILED_COUNT] = g_param_spec_int64(
        PROPERTY_FAILED_COUNT, "FailedCount", "number of failed pings", 0, INT64_MAX, 0, G_PARAM_READWRITE
    );

    obj_properties[PROP_CONSECUTIVE_FAILED_COUNT] = g_param_spec_int64(
        PROPERTY_CONSECUTIVE_FAILED_COUNT, "ConsecutiveFailedCount", "number of consecutive failed pings", 
        0, INT64_MAX, 0, G_PARAM_READWRITE
    );

    obj_properties[PROP_MAX_CONSECUTIVE_FAILED_COUNT] = g_param_spec_int64(
        PROPERTY_MAX_CONSECUTIVE_FAILED_COUNT, "MaxConsecutiveFailedCount", "max number of consecutive failed pings", 
        0, INT64_MAX, 0, G_PARAM_READWRITE
    );

    obj_properties[PROP_MAX_CONSECUTIVE_FAILED_TIME] = g_param_spec_string(
        PROPERTY_MAX_CONSECUTIVE_FAILED_TIME, "MaxConsecutiveFailedTime", "max time since last succeeded ping", 
        "", G_PARAM_READWRITE
    );

    obj_properties[PROP_TOTAL_PING_COUNT] = g_param_spec_int64(
        PROPERTY_TOTAL_PING_COUNT, "TotalPingCount", "total ping count", 0, INT64_MAX, 0, G_PARAM_READWRITE
    );

    obj_properties[PROP_PERCENTAGE_FAILED] = g_param_spec_double(
        PROPERTY_PERCENTAGE_FAILED, "PercentageFailed", "percentage of pings failed", 0, 100, 0, G_PARAM_READWRITE
    );

    obj_properties[PROP_LAST_PING_STATUS] = g_param_spec_string(
        PROPERTY_LAST_PING_STATUS, "LastPingStatus", "last ping status", "", G_PARAM_READWRITE
    );

    obj_properties[PROP_LAST_PING_TIME] = g_param_spec_string(
        PROPERTY_LAST_PING_TIME, "LastPingTime", "last ping time", "", G_PARAM_READWRITE
    );

    obj_properties[PROP_LAST_PING_TTL] = g_param_spec_int64(
        PROPERTY_LAST_PING_TTL, "LastPingTTL", "last ping ttl", 0, INT64_MAX, 0, G_PARAM_READWRITE
    );

    obj_properties[PROP_AVERAGE_PING_TIME] = g_param_spec_string(
        PROPERTY_AVERAGE_PING_TIME, "AveragePingTime", "average ping time", "", G_PARAM_READWRITE
    );

    obj_properties[PROP_LAST_SUCCEEDED_ON] = g_param_spec_string(
        PROPERTY_LAST_SUCCEEDED_ON, "LastSucceededOn", "last succeeded on", "", G_PARAM_READWRITE
    );

    obj_properties[PROP_LAST_FAILED_ON] = g_param_spec_string(
        PROPERTY_LAST_FAILED_ON, "LastFailedOn", "last failed on", "", G_PARAM_READWRITE
    );

    obj_properties[PROP_MINIMUM_PING_TIME] = g_param_spec_string(
        PROPERTY_MINIMUM_PING_TIME, "MinimumPingTime", "minimum ping time", "", G_PARAM_READWRITE
    );

    obj_properties[PROP_MAXIMUM_PING_TIME] = g_param_spec_string(
        PROPERTY_MAXIMUM_PING_TIME, "MaximumPingTime", "maximum ping time", "", G_PARAM_READWRITE
    );

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void ping_host_init(PingHost* self) {
    g_rw_lock_init(&self->lock);

    self->thread = g_thread_new("ping-host", ping_host_loop, self);
    self->pings = g_array_new(false, true, sizeof( ping_t ));
}

static void ping_host_dispose(GObject* self) {
    PingHost* host = PING_HOST(self);
    g_atomic_int_set(&host->exit, 1);
    g_thread_join(host->thread);
    g_rw_lock_clear(&host->lock);

    G_OBJECT_CLASS (ping_host_parent_class)->dispose(self);
}
static void ping_host_set_property(GObject* obj, guint prop_id, const GValue *value, GParamSpec *spec) {
    PingHost* self = PING_HOST(obj);

    g_rw_lock_writer_lock(&self->lock);

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

    case PROP_REPLY_ADDRESS:
        g_free(self->replay_addr);
        self->replay_addr = g_value_dup_string(value);
        break;

    case PROP_SUCCEEDED_COUNT:
        self->succeeded_count = g_value_get_int64(value);
        break;

    case PROP_FAILED_COUNT:
        self->failed_count = g_value_get_int64(value);
        break;

    case PROP_CONSECUTIVE_FAILED_COUNT:
        self->cons_failed_count = g_value_get_int64(value);
        break;

    case PROP_MAX_CONSECUTIVE_FAILED_COUNT:
        self->max_cons_failed_count = g_value_get_int64(value);
        break;

    case PROP_MAX_CONSECUTIVE_FAILED_TIME:
        g_free(self->max_cons_failed_time);
        self->max_cons_failed_time = g_value_dup_string(value);
        break;

    case PROP_TOTAL_PING_COUNT:
        self->total_ping_count = g_value_get_int64(value);
        break;

    case PROP_PERCENTAGE_FAILED:
        self->percentage_failed = g_value_get_double(value);
        break;

    case PROP_LAST_PING_STATUS:
        g_free(self->last_ping_status);
        self->last_ping_status = g_value_dup_string(value);
        break;

    case PROP_LAST_PING_TIME:
        g_free(self->last_ping_time);
        self->last_ping_time = g_value_dup_string(value);
        break;

    case PROP_LAST_PING_TTL:
        self->failed_count = g_value_get_int64(value);
        break;

    case PROP_AVERAGE_PING_TIME:
        g_free(self->avg_ping_time);
        self->avg_ping_time = g_value_dup_string(value);
        break;

    case PROP_LAST_SUCCEEDED_ON:
        g_free(self->last_succeeded_on);
        self->last_succeeded_on = g_value_dup_string(value);
        break;

    case PROP_LAST_FAILED_ON:
        g_free(self->last_failed_on);
        self->last_failed_on = g_value_dup_string(value);
        break;

    case PROP_MINIMUM_PING_TIME:
        g_free(self->min_ping_time);
        self->min_ping_time = g_value_dup_string(value);
        break;

    case PROP_MAXIMUM_PING_TIME:
        g_free(self->max_ping_time);
        self->max_ping_time = g_value_dup_string(value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
        break;
    }

    g_rw_lock_writer_unlock(&self->lock);
}

static void ping_host_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *spec) {
    PingHost *self = PING_HOST(obj);

    g_rw_lock_reader_lock(&self->lock);

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

    case PROP_REPLY_ADDRESS:
        g_value_set_string(value, self->replay_addr);
        break;

    case PROP_SUCCEEDED_COUNT:
        g_value_set_int64(value, self->succeeded_count);
        break;

    case PROP_FAILED_COUNT:
        g_value_set_int64(value, self->failed_count);
        break;

    case PROP_CONSECUTIVE_FAILED_COUNT:
        g_value_set_int64(value, self->cons_failed_count);
        break;

    case PROP_MAX_CONSECUTIVE_FAILED_COUNT:
        g_value_set_int64(value, self->max_cons_failed_count);
        break;

    case PROP_MAX_CONSECUTIVE_FAILED_TIME:
        g_value_set_string(value, self->max_cons_failed_time);
        break;

    case PROP_TOTAL_PING_COUNT:
        g_value_set_int64(value, self->total_ping_count);
        break;

    case PROP_PERCENTAGE_FAILED:
        g_value_set_double(value, self->percentage_failed);
        break;

    case PROP_LAST_PING_STATUS:
        g_value_set_string(value, self->last_ping_status);
        break;

    case PROP_LAST_PING_TIME:
        g_value_set_string(value, self->last_ping_time);
        break;

    case PROP_LAST_PING_TTL:
        g_value_set_int64(value, self->last_ping_ttl);
        break;

    case PROP_AVERAGE_PING_TIME:
        g_value_set_string(value, self->avg_ping_time);
        break;

    case PROP_LAST_SUCCEEDED_ON:
        g_value_set_string(value, self->last_succeeded_on);
        break;

    case PROP_LAST_FAILED_ON:
        g_value_set_string(value, self->last_failed_on);
        break;

    case PROP_MINIMUM_PING_TIME:
        g_value_set_string(value, self->min_ping_time);
        break;

    case PROP_MAXIMUM_PING_TIME:
        g_value_set_string(value, self->max_ping_time);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
        break;
    }

    g_rw_lock_reader_unlock(&self->lock);
}

void ping_host_set_integer(PingHost* host, const char* prop_name, int64_t value) {
    GValue integer = G_VALUE_INIT;
    g_value_init(&integer, G_TYPE_INT64);
    g_value_set_int64(&integer, value);

    g_object_set_property(G_OBJECT(host), prop_name, &integer);
    g_value_unset(&integer);
}

int64_t ping_host_get_integer(PingHost* host, const char* prop_name) {
    GValue integer = G_VALUE_INIT;
    g_value_init(&integer, G_TYPE_INT);

    g_object_get_property(G_OBJECT(host), prop_name, &integer);
    int64_t value = g_value_get_int(&integer);
    g_value_unset(&integer);

    return value;
}

void ping_host_set_string(PingHost* host, const gchar* prop_name, const gchar* value) {
    GValue string = G_VALUE_INIT;
    g_value_init(&string, G_TYPE_STRING);
    g_value_set_string(&string, value);

    g_object_set_property(G_OBJECT(host), prop_name, &string);
    g_value_unset(&string);
}

void ping_host_update_hostname(PingHost* host) {
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

    ping_host_set_string(host, "hostname", hbuf);
}

void ping_host_update_address(PingHost* host) {
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
    ping_host_set_string(host, "address", inet_ntoa(host->addr_in.sin_addr));
}

static gpointer ping_host_loop(gpointer data) {
    g_return_val_if_fail(data != NULL, NULL);
    PingHost* host = data;

    while (!g_atomic_int_get(&host->exit)) {
        int64_t ping_count = ping_host_get_integer(host, PROPERTY_TOTAL_PING_COUNT);
        ping_host_set_integer(host, PROPERTY_TOTAL_PING_COUNT, ping_count + 1);

        g_usleep(PING_FREQUENCY * G_USEC_PER_SEC);
    }

    return NULL;
}
