/*
*
*/

#include "ping-host.h"
#include "glib-object.h"
#include "glib.h"

typedef enum {
    PROP_HOST_NAME = 1,
    PROP_ADDRESS,
    N_PROPERTIES
} PingHostProperty;

struct _PingHost {
    GObject parent_instance;

    gchar *hostname;
    gchar *addr;
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL,};

G_DEFINE_TYPE (PingHost, ping_host, G_TYPE_OBJECT)

static void ping_host_set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *spec) {
    PingHost *self = PING_HOST(obj);

    switch ((PingHostProperty) prop_id) {
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

    obj_properties[PROP_HOST_NAME] = g_param_spec_string(
        "hostname", "HostName", "host name of the device", NULL, G_PARAM_READWRITE
    );

    obj_properties[PROP_ADDRESS] = g_param_spec_string(
        "address", "Address", "address of the device", NULL, G_PARAM_READWRITE
    );

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void ping_host_init(PingHost *self) {
}

