/*
 * ping-viewer.c 
 *
 */

#include "ping-viewer.h"

#include "glib.h"
#include "list.h"
#include "src/conf.h"
#include <stdlib.h>

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
    gtk_widget_set_name(taskbar, "taskbar");

    GtkWidget* button_add_host = gtk_button_new_from_icon_name("list-add");
    gtk_widget_set_name(button_add_host, "add-ping-host");
    g_signal_connect(button_add_host, "clicked", G_CALLBACK(ping_list_add_host), list_host);
    gtk_box_append(GTK_BOX(taskbar), button_add_host);

    GtkWidget *button_remove_host = gtk_button_new_from_icon_name("list-remove");
    gtk_widget_set_name(button_remove_host, "remove-ping-host");
    g_signal_connect(button_remove_host, "clicked", G_CALLBACK(ping_list_remove_host), list_host);
    gtk_box_append(GTK_BOX(taskbar), button_remove_host);

    /* host list */
    GtkWidget* scroll_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), list_host);
    gtk_widget_set_vexpand(scroll_window, true);
    gtk_widget_set_hexpand(scroll_window, true);

    gtk_box_append(GTK_BOX(box), scroll_window);

    /* load css */
    gchar* css_path = g_strdup_printf("%s/%s", g_get_user_config_dir(), PING_VIEWER_CSS_FILE);
    GFile* file = g_file_new_for_path(css_path);
    if (g_file_query_exists(file, NULL)) {
        GtkCssProvider* css_provider = gtk_css_provider_new();
        GdkDisplay* display = gdk_display_get_default();

        gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        gtk_css_provider_load_from_file(css_provider, file);

        g_object_unref(css_provider);
        g_free(css_path);
    }

    /* load config */
    gchar* conf_path = g_strdup_printf("%s/%s", g_get_user_config_dir(), PING_VIEWER_CONF_FILE);
    if (!load_config(list_host, conf_path)) {
        exit(1);
    }

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

void ping_log(const char* buf, ...) {
    va_list argptr = {0};
    va_start(argptr, buf);

    g_autofree GDateTime* now = g_date_time_new_now_local();
    g_autofree gchar* msg = g_strdup_vprintf(buf, argptr);

    printf("[" PING_TIME_FORMAT "] %s\n", 
        g_date_time_get_year(now),
        g_date_time_get_month(now),
        g_date_time_get_day_of_month(now),
        g_date_time_get_hour(now),
        g_date_time_get_minute(now),
        g_date_time_get_second(now),
        msg
    );
}
