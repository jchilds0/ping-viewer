/*
 * conf.c
 */

#include "conf.h"

#include <glib/gstdio.h>

#include "glib.h"
#include "ping-viewer.h"
#include "src/host.h"
#include "src/list.h"

bool load_config(GtkWidget* column_view, const char* path) {
    /* open file */
    g_autoptr(GFile) f = g_file_new_for_path(path);
    if (f == NULL) {
        ping_log("[config] failed to open config file '%s'", path);
        return false;
    }

    g_autoptr(GFileInputStream) stream = g_file_read(f, NULL, NULL);
    if (stream == NULL) {
        ping_log("[config] failed to open config file '%s'", path);
        return false;
    }

    g_autoptr(GDataInputStream) datastream = g_data_input_stream_new(G_INPUT_STREAM(stream));
    if (datastream == NULL) {
        ping_log("[config] failed to open config file '%s'", path);
        return false;
    }

    /* read lines */
    char* line               = NULL;
    gsize line_length        = 0;
    unsigned int line_number = 1;
    while ((line = g_data_input_stream_read_line(datastream, &line_length, NULL, NULL)) != NULL) {
        /* skip empty lines and comments */
        if (line_length == 0 || strchr(COMMENT_PREFIX, line[0]) != NULL) {
            goto CONTINUE;
        }

        gchar** args = g_strsplit(line, " ", 3);
        int length = g_strv_length(args);
        
        if (length != 3) {
            ping_log("[config] line %d: incorrect number of args, expected 3, got %d", line_number, length);
            g_strfreev(args);
            goto CONTINUE;
        }

        if (strcmp(args[0], "Host")) {
            ping_log("[config] line %d: incorrect identifier, expected 'Host', got %s", line_number, args[0]);
            g_strfreev(args);
            goto CONTINUE;
        }

        PingHost* host = ping_list_add_host(NULL, column_view);
        ping_host_set_string(host, PROPERTY_NAME, args[1]);
        ping_host_set_string(host, PROPERTY_HOST_NAME, args[2]);

        ping_host_update_address(host);
        ping_host_ping_task(host);

        g_strfreev(args);

    CONTINUE:
        line_number++;
        g_free(line);
    }

    return true;
}
