#ifndef PING_VIEWER_PING_VIEWER_H
#define PING_VIEWER_PING_VIEWER_H

#include "gtk/gtk.h"

#define PING_VIEWER_CSS_FILE      "ping-viewer/style.css"
#define PING_TIME_FORMAT          "%04d-%02d-%02dT%02d:%02d:%02d"

void ping_log(const char* buf, ...);

#endif // PING_VIEWER_PING_VIEWER_H
