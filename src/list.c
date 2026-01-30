/*
 * list.c
 *
 * Widget containing a list of hosts to ping.
 */

#include <gtk/gtk.h>

#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"

#include "gtk/gtkshortcut.h"
#include "ping-host.h"
#include "list.h"

static void empty_cb(GtkWidget* widget, gpointer data) {}

static void hostname_activate_cb(GtkWidget *widget, gpointer data) {
    GtkListItem* list_item = data;
    PingHost* host = PING_HOST(gtk_list_item_get_item(list_item));

    ping_update_address(host);
}

static void address_activate_cb(GtkWidget *widget, gpointer data) {
    GtkListItem* list_item = data;
    PingHost* host = PING_HOST(gtk_list_item_get_item(list_item));

    ping_update_hostname(host);
}

LIST_INPUT_CB(name, "name", empty_cb);
LIST_INPUT_CB(hostname, "hostname", hostname_activate_cb);
LIST_INPUT_CB(address, "address", address_activate_cb);

void ping_list_add_host(GtkWidget *widget, gpointer data) {
    g_return_if_fail(data != NULL);

    GtkWidget* column_view = data;
    GtkSelectionModel* model = gtk_column_view_get_model(GTK_COLUMN_VIEW(column_view));
    GListModel* list_model = gtk_single_selection_get_model(GTK_SINGLE_SELECTION(model));

    PingHost* host = g_object_new(PING_TYPE_HOST, NULL);
    g_list_store_append(G_LIST_STORE(list_model), host);

    return;
}

GtkWidget *ping_create_host_list() {
    GListStore* list = g_list_store_new(PING_TYPE_HOST);

    GtkSingleSelection* model = gtk_single_selection_new(G_LIST_MODEL(list));
    GtkWidget* column_view = gtk_column_view_new(GTK_SELECTION_MODEL(model));

    LIST_ADD_COLUMN(column_view, "Name", name);
    LIST_ADD_COLUMN(column_view, "Host Name", hostname);
    LIST_ADD_COLUMN(column_view, "Address", address);

    return column_view;
}
