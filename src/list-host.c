#include <gtk/gtk.h>

#include "gio/gio.h"
#include "glib-object.h"
#include "ping-host.h"
#include "list.h"

LIST_CB(hostname, "hostname")
LIST_CB(address, "address")

static PingHost *create_host(char *name, char *addr) {
    PingHost *host = g_object_new(PING_TYPE_HOST, NULL);

    GValue val_name = G_VALUE_INIT;
    g_value_init(&val_name, G_TYPE_STRING);
    g_value_take_string(&val_name, name);
    g_object_set_property(G_OBJECT(host), "hostname", &val_name);

    GValue val_addr = G_VALUE_INIT;
    g_value_init(&val_addr, G_TYPE_STRING);
    g_value_take_string(&val_addr, addr);
    g_object_set_property(G_OBJECT(host), "address", &val_addr);

    return host;
}

GtkWidget *ping_create_host_list() {
    GListStore *list = g_list_store_new(PING_TYPE_HOST);

    PingHost *cf = create_host("cloudflare", "1.1.1.1");
    PingHost *google = create_host("google", "8.8.8.8");

    g_list_store_append(list, cf);
    g_list_store_append(list, google);

    GtkSingleSelection *model = gtk_single_selection_new(G_LIST_MODEL(list));
    GtkWidget *column_view = gtk_column_view_new(GTK_SELECTION_MODEL(model));

    LIST_ADD_COLUMN(column_view, "Host Name", hostname)
    LIST_ADD_COLUMN(column_view, "Address", address)

    return column_view;
}
