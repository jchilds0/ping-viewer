/*
 * list.c
 *
 * Widget containing a list of hosts to ping.
 */

#include "list.h"

#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "host.h"

#define PING_INTERVAL         (10 * 1000)

static void empty_cb(GtkWidget* widget, gpointer data) {}

static void hostname_activate_cb(GtkWidget *widget, gpointer data) {
    GtkListItem* list_item = data;
    PingHost* host = PING_HOST(gtk_list_item_get_item(list_item));

    ping_host_cancel_current_ping(host);
    ping_host_reset_stats(host);

    ping_host_update_address(host);

    /* run a ping task immediately */
    ping_host_ping_task(host);
}

static void address_activate_cb(GtkWidget *widget, gpointer data) {
    GtkListItem* list_item = data;
    PingHost* host = PING_HOST(gtk_list_item_get_item(list_item));

    ping_host_cancel_current_ping(host);
    ping_host_reset_stats(host);

    ping_host_update_hostname(host);

    /* run a ping task immediately */
    ping_host_ping_task(host);
}

LIST_INPUT_CB(name, PROPERTY_NAME, empty_cb);
LIST_INPUT_CB(hostname, PROPERTY_HOST_NAME, hostname_activate_cb);
LIST_INPUT_CB(address, PROPERTY_ADDRESS, address_activate_cb);
LIST_TEXT_CB(reply_address, PROPERTY_REPLY_ADDRESS);
LIST_TEXT_CB(succeed_count, PROPERTY_SUCCEEDED_COUNT);
LIST_TEXT_CB(failed_count, PROPERTY_FAILED_COUNT);
LIST_TEXT_CB(consecutive_failed_count, PROPERTY_CONSECUTIVE_FAILED_COUNT);
LIST_TEXT_CB(max_consecutive_failed_count, PROPERTY_MAX_CONSECUTIVE_FAILED_COUNT);
LIST_TEXT_CB(max_consecutive_failed_time, PROPERTY_MAX_CONSECUTIVE_FAILED_TIME);
LIST_TEXT_CB(total_ping_count, PROPERTY_TOTAL_PING_COUNT);
LIST_TEXT_CB(percentage_failed, PROPERTY_PERCENTAGE_FAILED);
LIST_TEXT_CB(last_ping_status, PROPERTY_LAST_PING_STATUS);
LIST_TEXT_CB(last_ping_time, PROPERTY_LAST_PING_TIME);
LIST_TEXT_CB(last_ping_ttl, PROPERTY_LAST_PING_TTL);
LIST_TEXT_CB(avg_ping_time, PROPERTY_AVERAGE_PING_TIME);
LIST_TEXT_CB(last_succeeded_on, PROPERTY_LAST_SUCCEEDED_ON);
LIST_TEXT_CB(last_failed_on, PROPERTY_LAST_FAILED_ON);
LIST_TEXT_CB(min_ping_time, PROPERTY_MINIMUM_PING_TIME);
LIST_TEXT_CB(max_ping_time, PROPERTY_MAXIMUM_PING_TIME);

void ping_list_add_host(GtkWidget *widget, gpointer data) {
    g_return_if_fail(data != NULL);

    GtkWidget* column_view = data;
    GtkSelectionModel* model = gtk_column_view_get_model(GTK_COLUMN_VIEW(column_view));
    GListModel* list_model = gtk_single_selection_get_model(GTK_SINGLE_SELECTION(model));

    PingHost* host = g_object_new(PING_TYPE_HOST, NULL);
    g_list_store_append(G_LIST_STORE(list_model), host);
}

void ping_list_remove_host(GtkWidget *widget, gpointer data) {
    g_return_if_fail(data != NULL);

    GtkWidget* column_view = data;
    GtkSelectionModel* model = gtk_column_view_get_model(GTK_COLUMN_VIEW(column_view));
    GListModel* list_model = gtk_single_selection_get_model(GTK_SINGLE_SELECTION(model));

    guint selected = gtk_single_selection_get_selected(GTK_SINGLE_SELECTION(model));
    g_list_store_remove(G_LIST_STORE(list_model), selected);
}

static gboolean ping_list_ping_hosts(gpointer data) {
    g_return_val_if_fail(data != NULL, G_SOURCE_REMOVE);

    GtkWidget* column_view = data;
    GtkSelectionModel* model = gtk_column_view_get_model(GTK_COLUMN_VIEW(column_view));
    GListModel* list_model = gtk_single_selection_get_model(GTK_SINGLE_SELECTION(model));

    for (size_t i = 0; i < g_list_model_get_n_items(list_model); i++) {
        gpointer item = g_list_model_get_item(list_model, i);
        PingHost* host = PING_HOST(item);

        if (!ping_host_is_valid(host)) {
            continue;
        }

        ping_host_ping_task(host);
    }

    return G_SOURCE_CONTINUE;
}

GtkWidget *ping_create_host_list() {
    GListStore* list = g_list_store_new(PING_TYPE_HOST);

    GtkSingleSelection* model = gtk_single_selection_new(G_LIST_MODEL(list));
    GtkWidget* column_view = gtk_column_view_new(GTK_SELECTION_MODEL(model));

    gtk_widget_set_name(column_view, "list");
    gtk_column_view_set_reorderable(GTK_COLUMN_VIEW(column_view), true);
    gtk_column_view_set_show_row_separators(GTK_COLUMN_VIEW(column_view), true);

    LIST_ADD_COLUMN(column_view, "Name", name);
    LIST_ADD_COLUMN(column_view, "Host Name", hostname);
    LIST_ADD_COLUMN(column_view, "Address", address);
    LIST_ADD_COLUMN(column_view, "Reply Address", reply_address);
    LIST_ADD_COLUMN(column_view, "Succeeded Count", succeed_count);
    LIST_ADD_COLUMN(column_view, "Failed Count", failed_count);
    LIST_ADD_COLUMN(column_view, "Consecutive Failed Count", consecutive_failed_count);
    LIST_ADD_COLUMN(column_view, "Max Consecutive Failed Count", max_consecutive_failed_count);
    LIST_ADD_COLUMN(column_view, "Max Consecutive Failed Time", max_consecutive_failed_time);
    LIST_ADD_COLUMN(column_view, "Total Ping Count", total_ping_count);
    LIST_ADD_COLUMN(column_view, "Percentage Failed", percentage_failed);
    LIST_ADD_COLUMN(column_view, "Last Ping Status", last_ping_status);
    LIST_ADD_COLUMN(column_view, "Last Ping Time", last_ping_time);
    LIST_ADD_COLUMN(column_view, "Last Ping TTL", last_ping_ttl);
    LIST_ADD_COLUMN(column_view, "Average Ping Time", avg_ping_time);
    LIST_ADD_COLUMN(column_view, "Last Succeeded On", last_succeeded_on);
    LIST_ADD_COLUMN(column_view, "Last Failed On", last_failed_on);
    LIST_ADD_COLUMN(column_view, "Minimum Ping Time", min_ping_time);
    LIST_ADD_COLUMN(column_view, "Maximum Ping Time", max_ping_time);

    g_timeout_add(PING_INTERVAL, ping_list_ping_hosts, column_view);
    return column_view;
}
