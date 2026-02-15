/*
 * ping-host.c
 *
 * Defines the PingHost object which represents
 * a host to ping.
 */

#include "host.h"

#include "ping.h"
#include "ping-viewer.h"

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
    GCancellable* cancel;

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

static void ping_host_init(PingHost* self) {
    self->name = g_strdup("");
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
        self->last_ping_ttl = g_value_get_int64(value);
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

void ping_host_update_address(PingHost* host) {
    g_autoptr(GError) error = NULL;
    GResolver* resolver = g_resolver_get_default();
    GList* names = NULL;

    names = g_resolver_lookup_by_name(resolver, host->hostname, NULL, &error);
    if (error != NULL) {
        ping_log("[%s] resolver error: %s", host->hostname, error->message);
        return;
    }

    GList* addr = g_list_first(names);
    if (addr == NULL) {
        ping_log("[%s] no ip addresses found", host->hostname);
        return;
    }

    GInetAddress* inet_addr = g_list_first(names)->data;

    ping_host_set_string(host, "address", g_inet_address_to_string(inet_addr));
    g_resolver_free_addresses(names);
}

void ping_host_update_hostname(PingHost* host) {
    g_autoptr(GError) error = NULL;
    GResolver* resolver = g_resolver_get_default();
    gchar* hostname = NULL;

    g_autoptr(GInetAddress) inet_addr = g_inet_address_new_from_string(host->hostname);

    hostname = g_resolver_lookup_by_address(resolver, inet_addr, NULL, &error);
    if (error != NULL) {
        ping_log("[%s] resolver error: %s", host->hostname, error->message);
        return;
    }

    ping_host_set_string(host, "hostname", hostname);
}

bool ping_host_is_valid(PingHost* host) {
    return host->addr != NULL;
}

bool ping_host_last_ping(PingHost* host) {
    return host->internal.last_ping_succeeded;
}

void ping_host_reset_stats(PingHost* host) {
    host->internal.last_ping_succeeded = false;
    host->internal.ping_time_sum = 0;

    ping_host_set_string(host, PROPERTY_REPLY_ADDRESS, "");
    ping_host_set_string(host, PROPERTY_LAST_PING_STATUS, "");
    ping_host_set_string(host, PROPERTY_LAST_PING_TIME, "");
    ping_host_set_string(host, PROPERTY_LAST_SUCCEEDED_ON, "");
    ping_host_set_string(host, PROPERTY_LAST_FAILED_ON, "");

    ping_host_set_integer(host, PROPERTY_SUCCEEDED_COUNT, 0);
    ping_host_set_integer(host, PROPERTY_FAILED_COUNT, 0);
    ping_host_set_integer(host, PROPERTY_CONSECUTIVE_FAILED_COUNT, 0);
    ping_host_set_integer(host, PROPERTY_MAX_CONSECUTIVE_FAILED_COUNT, 0);
    ping_host_set_integer(host, PROPERTY_TOTAL_PING_COUNT, 0);
    ping_host_set_integer(host, PROPERTY_PERCENTAGE_FAILED, 0);
    ping_host_set_integer(host, PROPERTY_LAST_PING_TTL, 0);
    ping_host_set_integer(host, PROPERTY_AVERAGE_PING_TIME, 0);
    ping_host_set_integer(host, PROPERTY_MINIMUM_PING_TIME, 0);
    ping_host_set_integer(host, PROPERTY_MAXIMUM_PING_TIME, 0);
}

static void ping_host_thread(GTask* task, gpointer source_object, 
                             gpointer task_data, GCancellable* cancellable) {
    PingHost* host = PING_HOST(source_object);
    ping_t* ping = g_malloc0(sizeof( ping_t ));
    g_autoptr(GError) error = NULL;
    gint timeout = 3;
    int seq_no = 0, received_ttl = 0;

    ping_log("[%s] icmp echo ping", host->addr);

    g_autoptr(GSocketAddress) sockaddr = g_inet_socket_address_new_from_string(host->addr, 0);
    if (sockaddr == NULL) {
        ping->msg = g_strdup("invalid address");
        ping->succeeded = false;

        ping_log("[%s] %s", host->name, ping->msg);
        g_task_return_pointer(task, ping, ping_free);
        return;
    }

    GInetAddress* inet_addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(sockaddr));
    GSocketFamily family = g_inet_address_get_family(inet_addr);

    int proto = ping_family_to_protocol(family);
    if (proto < 0) {
        ping->msg = g_strdup("invalid socket family");
        ping->succeeded = false;

        ping_log("[%s] %s", host->addr, ping->msg);
        g_task_return_pointer(task, ping, ping_free);
        return;
    }

    g_autoptr(GSocket) sock = ping_socket(host->addr, family, proto, &error);
    if (sock == NULL) {
        if (error != NULL) {
            ping->msg = g_strdup(error->message);
        } else {
            ping->msg = g_strdup("error creating socket");
        }
        ping->succeeded = false;

        ping_log("[%s] %s", host->addr, ping->msg);
        g_task_return_pointer(task, ping, ping_free);
        return;
    }

    ping_send(sock, sockaddr, seq_no, &error);
    if (error != NULL) {
        ping->msg = g_strdup(error->message);
        ping->succeeded = false;

        ping_log("[%s] send error: %s", host->addr, ping->msg);
        g_task_return_pointer(task, ping, ping_free);
        return;
    }

    GSocketAddress* rcv_addr;
    ping_recv(sock, timeout, &rcv_addr, &seq_no, &received_ttl, &error);
    if (error != NULL) {
        ping->msg = g_strdup(error->message);
        ping->succeeded = false;

        ping_log("[%s] recv error: %s", host->addr, ping->msg);
        g_task_return_pointer(task, ping, ping_free);
        return;
    }

    g_autoptr(GInetAddress) rcv_inet_addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(rcv_addr));
    gchar* reply_addr = g_inet_address_to_string(inet_addr);

    if (g_task_set_return_on_cancel(task, FALSE)) {
        ping->msg = g_strdup("Succeeded");
        ping->reply_addr = reply_addr;
        ping->ttl = received_ttl;
        ping->succeeded = true;

        ping_log("[%s] received reply from %s", host->addr, ping->reply_addr);
        g_task_return_pointer(task, ping, ping_free);
    }
}

static gchar* current_time() {
    g_autoptr(GDateTime) now = g_date_time_new_now_local();

    return g_strdup_printf(PING_TIME_FORMAT, 
        g_date_time_get_year(now),
        g_date_time_get_month(now),
        g_date_time_get_day_of_month(now),
        g_date_time_get_hour(now),
        g_date_time_get_minute(now),
        g_date_time_get_second(now)
    );
}

static void ping_host_update_cb(GObject* source_object, GAsyncResult* res, gpointer data) {
    PingHost* host = PING_HOST(source_object);
    GError* error = NULL;
    ping_t* ping;
    gchar* now_str;

    ping_log("[%s] update ping stats", host->addr);

    ping = g_task_propagate_pointer(G_TASK(res), &error);
    if (ping == NULL) {
        if (error->domain == G_IO_ERROR && error->code == G_IO_ERROR_CANCELLED) {
            ping_log("[%s] ping cancelled", host->name);
        } else {
            ping_log("[%s] task error: %s", host->name, error->message);
        }

        g_error_free(error);
        return;
    }

    now_str = current_time();

    ping_host_set_string(host, PROPERTY_REPLY_ADDRESS, ping->reply_addr);
    ping_host_set_string(host, PROPERTY_LAST_PING_STATUS, ping->msg);
    ping_host_set_string(host, PROPERTY_LAST_PING_TIME, now_str);

    ping_host_set_integer(host, PROPERTY_SUCCEEDED_COUNT, host->succeeded_count + ping->succeeded);
    ping_host_set_integer(host, PROPERTY_FAILED_COUNT, host->failed_count + !ping->succeeded);
    ping_host_set_integer(host, PROPERTY_TOTAL_PING_COUNT, host->total_ping_count + 1);
    ping_host_set_integer(host, PROPERTY_LAST_PING_TTL, ping->ttl);

    GValue value_f = G_VALUE_INIT;
    g_value_init(&value_f, G_TYPE_DOUBLE);
    g_value_set_double(&value_f, (double)(host->failed_count * 100) / host->total_ping_count);

    g_object_set_property(G_OBJECT(host), PROPERTY_PERCENTAGE_FAILED, &value_f);
    g_value_unset(&value_f);

    if (ping->succeeded) {
        ping_host_set_string(host, PROPERTY_LAST_SUCCEEDED_ON, now_str);
        ping_host_set_integer(host, PROPERTY_CONSECUTIVE_FAILED_COUNT, 0);
    } else {
        ping_host_set_string(host, PROPERTY_LAST_FAILED_ON, now_str);
    }

    if (!ping->succeeded) {
        ping_host_set_integer(host, PROPERTY_CONSECUTIVE_FAILED_COUNT, host->cons_failed_count + 1);

        if (host->cons_failed_count > host->max_cons_failed_count) {
            ping_host_set_integer(host, PROPERTY_MAX_CONSECUTIVE_FAILED_COUNT, host->cons_failed_count);
            ping_host_set_string(host, PROPERTY_MAX_CONSECUTIVE_FAILED_TIME, now_str);
        }
    }

    host->internal.last_ping_succeeded = ping->succeeded;

    ping_free(ping);
    g_free(now_str);
}

void ping_host_ping_task(PingHost* host) {
    g_return_if_fail(host != NULL);

    if (host->cancel != NULL) {
        g_object_unref(host->cancel);
    }

    host->cancel = g_cancellable_new();
    g_autoptr(GTask) task = g_task_new(host, host->cancel, ping_host_update_cb, host);

    ping_log("[%s] starting ping task", host->addr);
    g_task_set_return_on_cancel(task, TRUE);
    g_task_run_in_thread(task, ping_host_thread);
}

void ping_host_cancel_current_ping(PingHost* host) {
    g_return_if_fail(host != NULL);

    if (host->cancel == NULL) {
        return;
    }

    ping_log("[%s] ping cancelled", host->addr);
    g_cancellable_cancel(host->cancel);
}
