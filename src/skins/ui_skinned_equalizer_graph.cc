/*
 * Audacious - a cross-platform multimedia player
 * Copyright (c) 2007 Tomasz Moń
 * Copyright (c) 2011 John Lindgren
 *
 * Based on:
 * BMP - Cross-platform multimedia player
 * Copyright (C) 2003-2004  BMP development team.
 * XMMS:
 * Copyright (C) 1998-2003  XMMS development team.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; under version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses>.
 */

#include <libaudcore/equalizer.h>
#include <libaudcore/runtime.h>

#include "draw-compat.h"
#include "skins_cfg.h"
#include "ui_skin.h"
#include "ui_skinned_equalizer_graph.h"

static void init_spline (const double * x, const double * y, int n, double * y2)
{
    int i, k;
    double p, qn, sig, un, *u;

    u = (double *) g_malloc(n * sizeof(double));

    y2[0] = u[0] = 0.0;

    for (i = 1; i < n - 1; i++) {
        sig = ((double) x[i] - x[i - 1]) / ((double) x[i + 1] - x[i - 1]);
        p = sig * y2[i - 1] + 2.0;
        y2[i] = (sig - 1.0) / p;
        u[i] =
            (((double) y[i + 1] - y[i]) / (x[i + 1] - x[i])) -
            (((double) y[i] - y[i - 1]) / (x[i] - x[i - 1]));
        u[i] = (6.0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
    }
    qn = un = 0.0;

    y2[n - 1] = (un - qn * u[n - 2]) / (qn * y2[n - 2] + 1.0);
    for (k = n - 2; k >= 0; k--)
        y2[k] = y2[k] * y2[k + 1] + u[k];
    g_free(u);
}

double eval_spline (const double * xa, const double * ya, const double * y2a,
 int n, double x)
{
    int klo, khi, k;
    double h, b, a;

    klo = 0;
    khi = n - 1;
    while (khi - klo > 1) {
        k = (khi + klo) >> 1;
        if (xa[k] > x)
            khi = k;
        else
            klo = k;
    }
    h = xa[khi] - xa[klo];
    a = (xa[khi] - x) / h;
    b = (x - xa[klo]) / h;
    return (a * ya[klo] + b * ya[khi] +
            ((a * a * a - a) * y2a[klo] +
             (b * b * b - b) * y2a[khi]) * (h * h) / 6.0);
}

DRAW_FUNC_BEGIN (eq_graph_draw)
    static const double x[10] = {0, 11, 23, 35, 47, 59, 71, 83, 97, 109};

    skin_draw_pixbuf (cr, SKIN_EQMAIN, 0, 294, 0, 0, 113, 19);
    skin_draw_pixbuf (cr, SKIN_EQMAIN, 0, 314, 0, 9 + (aud_get_double (nullptr,
     "equalizer_preamp") * 9 + AUD_EQ_MAX_GAIN / 2) / AUD_EQ_MAX_GAIN, 113, 1);

    cairo_scale (cr, config.scale, config.scale);

    uint32_t cols[19];
    skin_get_eq_spline_colors(active_skin, cols);

    double bands[AUD_EQ_NBANDS];
    aud_eq_get_bands (bands);

    double yf[10];
    init_spline (x, bands, 10, yf);

    /* now draw a pixelated line with vector graphics ... -- jlindgren */
    int py = 0;
    for (int i = 0; i < 109; i ++)
    {
        int y = 9.5 - eval_spline (x, bands, yf, 10, i) * 9 / AUD_EQ_MAX_GAIN;
        y = aud::clamp (y, 0, 18);

        if (!i)
            py = y;

        int ymin, ymax;

        if (y > py)
        {
            ymin = py + 1;
            ymax = y;
        }
        else if (y < py)
        {
            ymin = y;
            ymax = py - 1;
        }
        else
            ymin = ymax = y;

        py = y;

        for (y = ymin; y <= ymax; y++)
        {
            cairo_rectangle (cr, i + 2, y, 1, 1);
            set_cairo_color (cr, cols[y]);
            cairo_fill (cr);
        }
    }
DRAW_FUNC_END

GtkWidget * eq_graph_new (void)
{
    GtkWidget * graph = gtk_drawing_area_new ();
    gtk_widget_set_size_request (graph, 113 * config.scale, 19 * config.scale);
    DRAW_CONNECT (graph, eq_graph_draw);
    return graph;
}

void eq_graph_update (GtkWidget * graph)
{
    gtk_widget_queue_draw (graph);
}
