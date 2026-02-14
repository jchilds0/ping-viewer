/*
* conf.h
*/

#ifndef PING_VIEWER_PING_CONF_H
#define PING_VIEWER_PING_CONF_H

#include <gtk/gtk.h>
#include <stdbool.h>

#define COMMENT_PREFIX  "\"#"

bool load_config(GtkWidget* column_view, const char* path);

#endif // PING_VIEWER_PING_CONF_H
