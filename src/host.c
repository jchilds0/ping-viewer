/*
 * ping-host.c
 *
 * Defines the PingHost object which represents
 * a host to ping.
 */

#include "host.h"

#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "ping.h"
#include "src/ping-viewer.h"

#define BUFSIZE             2048

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

    struct {
        bool last_ping_succeeded;
        int64_t ping_time_sum;
    } internal;

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
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL,};

G_DEFINE_TYPE (PingHost, ping_host, G_TYPE_OBJECT)

static void ping_host_set_property(GObject* obj, guint prop_id, const GValue *value, GParamSpec *spec);
static void ping_host_get_property(GObject* obj, guint prop_id, GValue *value, GParamSpec *spec);

static void ping_host_class_init(PingHostClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);

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

static void ping_host_init(PingHost* self) {}

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
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    struct sockaddr_in addr_in = {0};
    struct in_addr _in_addr;
    int s;

    if (inet_pton(AF_INET, host->addr, &_in_addr) == 0) {
        printf("%s isn't a valid IP address\n", host->addr);
        return;
    }

    addr_in.sin_family = AF_INET;
    addr_in.sin_addr = _in_addr;

    s = getnameinfo(
        (struct sockaddr *)&addr_in, sizeof( addr_in ), 
        hbuf, sizeof(hbuf), 
        sbuf, sizeof(sbuf), 
        NI_NUMERICHOST | NI_NUMERICSERV
    );
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

    struct sockaddr_in *sin_addr = (struct sockaddr_in *)result[0].ai_addr;
    ping_host_set_string(host, "address", inet_ntoa(sin_addr->sin_addr));
}

void ping_host_thread(GTask* task, gpointer source_object, 
                      gpointer task_data, GCancellable* cancellable) {
    PingHost* host = PING_HOST(source_object);
    ping_t* ping = g_malloc0(sizeof( ping_t ));
    int sock, status, seq_no = 0;

    struct timeval timeout = {3, 0};
    struct sockaddr_in addr_in = {0}, rcv_addr = {0};
    socklen_t rcv_addr_len = sizeof rcv_addr;
    char rcv_buf[2048];

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (socket < 0) {
        ping->msg = g_strdup("invalid internal socket");
        ping->succeeded = false;
        g_task_return_pointer(task, ping, ping_free);
        return;
    }

    if (host->addr == NULL) {
        ping->succeeded = false;
        g_task_return_pointer(task, ping, ping_free);
        return;
    }

    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = inet_addr(host->addr);

    status = ping_send(sock, (struct sockaddr *)&addr_in, sizeof( addr_in ), seq_no);
    if (status <= 0) {
        ping->msg = g_strdup(strerror(errno));
        ping->succeeded = false;
        g_task_return_pointer(task, ping, ping_free);
        return;
    }

    status = ping_recv(sock, timeout, (struct sockaddr *)&rcv_addr, &rcv_addr_len, &seq_no);
    if (status < 0) {
        ping->msg = g_strdup(strerror(errno));
        ping->succeeded = false;
        g_task_return_pointer(task, ping, ping_free);
        return;
    }

    memset(rcv_buf, 0, sizeof rcv_buf);
    inet_ntop(AF_INET, &(rcv_addr.sin_addr), rcv_buf, rcv_addr_len);

    ping->msg = g_strdup("Succeeded");
    ping->replay_addr = strdup(rcv_buf);
    ping->succeeded = true;
    g_task_return_pointer(task, ping, ping_free);
}

static gchar* current_time() {
    GDateTime* now = g_date_time_new_now_local();

    return g_strdup_printf("%04d-%02d-%02dT%02d:%02d:%02d", 
        g_date_time_get_year(now),
        g_date_time_get_month(now),
        g_date_time_get_day_of_month(now),
        g_date_time_get_hour(now),
        g_date_time_get_minute(now),
        g_date_time_get_second(now)
    );
}

void ping_host_update_cb(GObject* source_object, GAsyncResult* res, gpointer data) {
    PingHost* host = PING_HOST(source_object);
    ping_t* ping;
    gchar* now_str;

    ping = g_task_propagate_pointer(G_TASK(res), NULL);
    if (ping == NULL) {
        ping_log("missing ping result");
        return;
    }

    now_str = current_time();

    ping_host_set_string(host, PROPERTY_REPLY_ADDRESS, ping->replay_addr);
    ping_host_set_string(host, PROPERTY_LAST_PING_STATUS, ping->msg);
    ping_host_set_string(host, PROPERTY_LAST_PING_TIME, now_str);

    ping_host_set_integer(host, PROPERTY_SUCCEEDED_COUNT, host->succeeded_count + ping->succeeded);
    ping_host_set_integer(host, PROPERTY_FAILED_COUNT, host->failed_count + !ping->succeeded);
    ping_host_set_integer(host, PROPERTY_TOTAL_PING_COUNT, host->total_ping_count + 1);

    GValue value_f = G_VALUE_INIT;
    g_value_init(&value_f, G_TYPE_DOUBLE);
    g_value_set_double(&value_f, (double)(host->failed_count * 100) / host->total_ping_count);

    g_object_set_property(G_OBJECT(host), PROPERTY_PERCENTAGE_FAILED, &value_f);
    g_value_unset(&value_f);

    if (host->internal.last_ping_succeeded) {
        ping_host_set_integer(host, PROPERTY_CONSECUTIVE_FAILED_COUNT, 0);
    }

    if (ping->succeeded) {
        ping_host_set_string(host, PROPERTY_LAST_SUCCEEDED_ON, now_str);
    } else {
        ping_host_set_string(host, PROPERTY_LAST_FAILED_ON, now_str);
    }

    if (!host->internal.last_ping_succeeded && !ping->succeeded) {
        ping_host_set_integer(host, PROPERTY_CONSECUTIVE_FAILED_COUNT, host->cons_failed_count + 1);

        if (host->cons_failed_count > host->max_cons_failed_count) {
            ping_host_set_integer(host, PROPERTY_MAX_CONSECUTIVE_FAILED_COUNT, host->cons_failed_count);
            ping_host_set_string(host, PROPERTY_MAX_CONSECUTIVE_FAILED_TIME, now_str);
        }
    }

    host->internal.last_ping_succeeded = ping->succeeded;

    g_free(now_str);
}
