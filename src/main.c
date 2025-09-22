#include <arpa/inet.h>
#include <gtk/gtk.h>
#include "ping.h"

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *label;

    window = gtk_application_window_new (app);
    label = gtk_label_new("Hello GNOME!");
    gtk_container_add (GTK_CONTAINER (window), label);
    gtk_window_set_title(GTK_WINDOW (window), "Welcome to GNOME");
    gtk_window_set_default_size(GTK_WINDOW (window), 400, 200);
    gtk_widget_show_all(window);
} 

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    struct in_addr dst;
    const char addr[] = "1.1.1.1";

    if (inet_aton(addr, &dst) == 0) {
        perror("inet_aton");
        printf("%s isn't a valid IP address\n", addr);
        return 1;
    }

    ping_it(&dst);

#if GLIB_CHECK_VERSION(2, 74, 0)
    app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
#else
    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
#endif

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
