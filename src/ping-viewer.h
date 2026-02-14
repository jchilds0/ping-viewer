#ifndef PING_VIEWER_PING_VIEWER_H
#define PING_VIEWER_PING_VIEWER_H

#include "gtk/gtk.h"

#define PING_VIEWER_DIR           "ping-viewer"
#define PING_VIEWER_CSS_FILE      (PING_VIEWER_DIR "/style.css")
#define PING_VIEWER_CONF_FILE     (PING_VIEWER_DIR "/ping-viewer.conf")
#define PING_TIME_FORMAT          "%04d-%02d-%02dT%02d:%02d:%02d"

void ping_log(const char* buf, ...);

#endif // PING_VIEWER_PING_VIEWER_H
