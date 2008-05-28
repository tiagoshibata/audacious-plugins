#include <config.h>

#include <glib.h>
#include <sys/types.h>
#include <audacious/plugin.h>
#include <audacious/ui_plugin_menu.h>
#include <audacious/i18n.h>
#include <gtk/gtk.h>
#include <audacious/util.h>
#include "gui.h"
typedef struct {
    guint class;
    gchar* address;
    gchar* name;
}DeviceData;



void refresh_call(void);
void connect_call(void);
GList * audio_devices;
