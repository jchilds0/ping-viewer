#include <arpa/inet.h>

#include "gtk/gtk.h"
#include "ping-viewer.h"
#include "ping.h"

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;

    window = gtk_application_window_new (app);
    gtk_window_set_title(GTK_WINDOW(window), "Ping Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 300);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(box));

    GtkWidget *list_host = ping_create_host_list();
    gtk_box_append(GTK_BOX(box), list_host);

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

int ping_loop(void) {
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
    return 0;
}

