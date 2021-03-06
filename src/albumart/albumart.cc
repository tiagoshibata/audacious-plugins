/*
 * albumart.c
 * Copyright 2012-2013 John Lindgren
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions, and the following disclaimer in the documentation
 *    provided with the distribution.
 *
 * This software is provided "as is" and without any warranty, express or
 * implied. In no event shall the authors be liable for any damages arising from
 * the use of this software.
 */

#include <libaudcore/drct.h>
#include <libaudcore/i18n.h>
#include <libaudcore/plugin.h>
#include <libaudcore/hook.h>
#include <libaudgui/libaudgui.h>
#include <libaudgui/libaudgui-gtk.h>

class AlbumArtPlugin : public GeneralPlugin
{
public:
    static constexpr PluginInfo info = {
        N_("Album Art"),
        PACKAGE
    };

    constexpr AlbumArtPlugin () : GeneralPlugin (info, false) {}

    void * get_gtk_widget ();
};

EXPORT AlbumArtPlugin aud_plugin_instance;

static void album_update (void * unused, GtkWidget * widget)
{
    if (! aud_drct_get_playing ())
        return;

    GdkPixbuf * pixbuf = audgui_pixbuf_request_current ();

    if (! pixbuf)
        pixbuf = audgui_pixbuf_fallback ();

    audgui_scaled_image_set (widget, pixbuf);

    if (pixbuf)
        g_object_unref (pixbuf);
}

static void album_clear (void * unused, GtkWidget * widget)
{
    audgui_scaled_image_set (widget, nullptr);
}

static void album_cleanup (GtkWidget * widget)
{
    hook_dissociate ("playback begin", (HookFunction) album_update, widget);
    hook_dissociate ("current art ready", (HookFunction) album_update, widget);
    hook_dissociate ("playback stop", (HookFunction) album_clear, widget);

    audgui_cleanup ();
}

void * AlbumArtPlugin::get_gtk_widget ()
{
    audgui_init ();

    GtkWidget * widget = audgui_scaled_image_new (nullptr);
    gtk_widget_set_size_request (widget, 96, 96);

    g_signal_connect (widget, "destroy", (GCallback) album_cleanup, nullptr);

    hook_associate ("playback begin", (HookFunction) album_update, widget);
    hook_associate ("current art ready", (HookFunction) album_update, widget);
    hook_associate ("playback stop", (HookFunction) album_clear, widget);

    album_update (nullptr, widget);

    return widget;
}
