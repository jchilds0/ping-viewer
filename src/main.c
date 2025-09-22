#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <sys/socket.h>
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

    struct sockaddr_in sock_addr;
    const char addr[] = "1.1.1.1";

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        return sock;
    }

    if (ping_addr(addr, sizeof( addr ), &sock_addr) < 0) {
        return 1;
    }

    struct timeval timeout = {3, 0};
    if (ping_send(sock, (struct sockaddr *)&sock_addr, sizeof( sock_addr ), 0) < 0) {
        return 1;
    }

    struct sockaddr_in rcv_addr = {0};
    socklen_t rcv_addr_len = sizeof rcv_addr;
    int seq_no = 0;

    if (ping_recv(sock, timeout, (struct sockaddr *)&rcv_addr, &rcv_addr_len, &seq_no) < 0) {
        return 1;
    }

    char rcv_buf[2048];
    memset(rcv_buf, 0, sizeof rcv_buf);
    inet_ntop(AF_INET, &(rcv_addr.sin_addr), rcv_buf, rcv_addr_len);
    printf("ICMP echo reply from %s, seq no %d\n", rcv_buf, seq_no);

#if GLIB_CHECK_VERSION(2, 74, 0)
    app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
#else
    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
#endif

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    // status = g_application_run(G_APPLICATION(app), argc, argv);
    // g_object_unref(app);

    return status;
}
