#include <arpa/inet.h>

#include "glib-object.h"
#include "gtk/gtk.h"
#include "ping.h"
#include "list.h"

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;

    window = gtk_application_window_new (app);
    gtk_window_set_title(GTK_WINDOW(window), "Ping Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 300);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(box));

    GtkWidget *taskbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *list_host = ping_create_host_list();

    /* taskbar */
    gtk_box_append(GTK_BOX(box), taskbar);

    GtkWidget *button_add_host = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(button_add_host), "add host");
    g_signal_connect(button_add_host, "clicked", G_CALLBACK(ping_list_add_host), list_host);
    gtk_box_append(GTK_BOX(taskbar), button_add_host);

    /* host list */
    GtkWidget* scroll_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), list_host);
    gtk_widget_set_vexpand(scroll_window, true);
    gtk_widget_set_hexpand(scroll_window, true);

    gtk_box_append(GTK_BOX(box), scroll_window);

    gtk_window_present(GTK_WINDOW(window));
} 

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

#if GLIB_CHECK_VERSION(2, 74, 0)
    app = gtk_application_new("com.ping.viewer", G_APPLICATION_DEFAULT_FLAGS);
#else
    app = gtk_application_new("com.ping.viewer", G_APPLICATION_FLAGS_NONE);
#endif

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
