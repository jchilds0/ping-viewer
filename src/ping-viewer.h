#ifndef PING_VIEWER_PING_VIEWER_H
#define PING_VIEWER_PING_VIEWER_H

#include "gtk/gtk.h"

#define PING_VIEWER_CSS_FILE      "ping-viewer/style.css"

#define ping_log(...)             \
    printf("%s:", __func__); \
    printf(__VA_ARGS__); \
    printf("\n");

#endif // PING_VIEWER_PING_VIEWER_H
