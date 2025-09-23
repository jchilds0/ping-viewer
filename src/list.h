

#ifndef PING_VIEWER_LIST_H
#define PING_VIEWER_LIST_H

#include "glib-object.h"

#define LIST_SET_LISTITEM(column_name) \
    static void setup_listitem_##column_name##_cb(GtkListItemFactory *factory, GtkListItem *list_item) { \
        GtkWidget *label = gtk_label_new(""); \
        gtk_list_item_set_child(list_item, label); \
    }

#define LIST_BIND_LISTITEM(column_name, prop_name) \
    static void bind_listitem_##column_name##_cb(GtkListItemFactory *factory, GtkListItem *list_item) { \
        GtkWidget *label = gtk_list_item_get_child(list_item); \
        PingHost *host = gtk_list_item_get_item(list_item); \
        g_object_bind_property(G_OBJECT(host), prop_name, G_OBJECT(label), "label", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE); \
    }

#define LIST_CB(column_name, prop_name) LIST_SET_LISTITEM(column_name) \
    LIST_BIND_LISTITEM(column_name, prop_name)

#define LIST_ADD_COLUMN(col_view, column, column_name) { \
        GtkListItemFactory *factory = gtk_signal_list_item_factory_new(); \
        g_signal_connect(factory, "setup", G_CALLBACK(setup_listitem_##column_name##_cb), NULL); \
        g_signal_connect(factory, "bind", G_CALLBACK(bind_listitem_##column_name##_cb), NULL); \
        GtkColumnViewColumn *col_view_c = gtk_column_view_column_new(column, factory); \
        gtk_column_view_append_column(GTK_COLUMN_VIEW(col_view), col_view_c); \
    }

#endif // !PING_VIEWER_LIST_H
