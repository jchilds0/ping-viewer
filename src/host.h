
#ifndef PING_VIEWER_PING_HOST_H
#define PING_VIEWER_PING_HOST_H

#include "gio/gio.h"
#include <glib-object.h>
#include <stdbool.h>

#define PROPERTY_NAME                            "name"
#define PROPERTY_HOST_NAME                       "hostname"
#define PROPERTY_ADDRESS                         "address"
#define PROPERTY_REPLY_ADDRESS                   "reply-address"
#define PROPERTY_SUCCEEDED_COUNT                 "succeeded-count"
#define PROPERTY_FAILED_COUNT                    "failed-count"
#define PROPERTY_CONSECUTIVE_FAILED_COUNT        "consecutive-failed-count"
#define PROPERTY_MAX_CONSECUTIVE_FAILED_COUNT    "max-consecutive-failed-count"
#define PROPERTY_MAX_CONSECUTIVE_FAILED_TIME     "max-consecutive-failed-time"
#define PROPERTY_TOTAL_PING_COUNT                "total-ping-count"
#define PROPERTY_PERCENTAGE_FAILED               "percentage-failed"
#define PROPERTY_LAST_PING_STATUS                "last-ping-status"
#define PROPERTY_LAST_PING_TIME                  "last-ping-time"
#define PROPERTY_LAST_PING_TTL                   "last-ping-ttl"
#define PROPERTY_AVERAGE_PING_TIME               "average-ping-time"
#define PROPERTY_LAST_SUCCEEDED_ON               "last-succeeded-on"
#define PROPERTY_LAST_FAILED_ON                  "last-failed-on"
#define PROPERTY_MINIMUM_PING_TIME               "minimum-ping-time"
#define PROPERTY_MAXIMUM_PING_TIME               "maximum-ping-time"

G_BEGIN_DECLS

#define PING_TYPE_HOST ping_host_get_type()
G_DECLARE_FINAL_TYPE (PingHost, ping_host, PING, HOST, GObject)

PingHost *ping_host_new(void);

void ping_host_set_string(PingHost* host, const char* prop_name, const char* value);
int64_t ping_host_get_integer(PingHost* host, const char* prop_name);
void ping_host_set_integer(PingHost* host, const char* prop_name, int64_t value);
void ping_host_reset_stats(PingHost* host);
void ping_host_update_hostname(PingHost* host);
void ping_host_update_address(PingHost* host);
void ping_host_update_cb(GObject* source_object, GAsyncResult* res, gpointer data);
bool ping_host_is_valid(PingHost* host);
void ping_host_thread(GTask* task, gpointer source_object, gpointer task_data, GCancellable* cancellable);

G_END_DECLS

#endif // PING_VIEWER_PING_HOST_H
