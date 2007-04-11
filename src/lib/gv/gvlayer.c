/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Base class for all display layers.
 * Author:   Frank Warmerdam, warmerda@home.com
 *
 ******************************************************************************
 * Copyright (c) 2000, Atlantis Scientific Inc. (www.atlsci.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ******************************************************************************
 *
 * $Log: gvlayer.c,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:26  uid1026
 *
 * Imported sources
 *
 * Revision 1.13  2001/10/12 17:44:18  warmerda
 * avoid extra redraws when many raster layers displayed
 *
 * Revision 1.12  2001/04/09 18:14:49  warmerda
 * added view field to GvLayer
 *
 * Revision 1.11  2001/03/28 15:13:59  warmerda
 * added view to GvLayer
 *
 * Revision 1.10  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvlayer.h"
#include <stdio.h>

enum
{
    SETUP,
    TEARDOWN,
    DRAW,
    EXTENTS_REQUEST,
    DISPLAY_CHANGE,
    LAST_SIGNAL
};

static void gv_layer_class_init(GvLayerClass *klass);
static void gv_layer_init(GvLayer *layer);
static void gv_layer_finalize(GObject *gobject);

static GvDataClass *parent_class = NULL;
static guint layer_signals[LAST_SIGNAL] = { 0 };

GType
gv_layer_get_type(void)
{
    static GType layer_type = 0;

    if (!layer_type) {
        static const GTypeInfo layer_info =
        {
            sizeof(GvLayerClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_layer_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvLayer),
            0,
            (GInstanceInitFunc) gv_layer_init,
        };
        layer_type = g_type_register_static (GV_TYPE_DATA,
                                            "GvLayer",
                                            &layer_info, 0);
    }

    return layer_type;
}

static void
gv_layer_class_init(GvLayerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);;

    parent_class = g_type_class_peek_parent (klass);

    layer_signals[SETUP] =
        g_signal_new ("setup",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (GvLayerClass, setup),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                      G_TYPE_POINTER);

    layer_signals[TEARDOWN] =
        g_signal_new ("teardown",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (GvLayerClass, teardown),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                      G_TYPE_POINTER);

    layer_signals[DRAW] =
        g_signal_new ("draw",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GvLayerClass, draw),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                      G_TYPE_POINTER);

    layer_signals[EXTENTS_REQUEST] =
        g_signal_new ("get_extents",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (GvLayerClass, extents_request),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                      G_TYPE_POINTER);

    layer_signals[DISPLAY_CHANGE] =
        g_signal_new ("display-change",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (GvLayerClass, display_change),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                      G_TYPE_POINTER);

    object_class->finalize = gv_layer_finalize;

    klass->setup = NULL;
    klass->teardown = NULL;
    klass->draw = NULL;
    klass->extents_request = NULL;
    klass->reproject = NULL;
    klass->display_change = NULL;
}

void
gv_layer_finalize(GObject *gobject)
{
    CPLDebug( "OpenEV", "gv_layer_finalize(%s)",
              gv_data_get_name( GV_DATA(gobject) ) );
    GvLayer *layer = GV_LAYER(gobject);
    g_return_if_fail(GV_IS_LAYER(layer));

    if (layer->projection != NULL) {
      g_free (layer->projection);
      layer->projection = NULL;
    }

    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}

void
gv_layer_init(GvLayer *layer)
{
    layer->setup_count = 0;
    layer->invisible = FALSE;
    layer->presentation = FALSE;
    layer->projection = NULL;
    layer->view = NULL;
    layer->pending_idle = FALSE;
}

void
gv_layer_setup(GvLayer *layer, GvViewArea *view)
{
    if (++layer->setup_count == 1)
        g_signal_emit(layer, layer_signals[SETUP], 0, view);
}

void
gv_layer_teardown(GvLayer *layer, GvViewArea *view)
{
    if (--layer->setup_count < 1)
        g_signal_emit(layer, layer_signals[TEARDOWN], 0, view);
}

void
gv_layer_draw(GvLayer *layer, GvViewArea *view)
{
    g_assert( view == layer->view );

    if (!layer->invisible)
    {
        if (layer->presentation)
        {
            /* Avoid triggering tool drawing by not emitting a signal */
            GvLayerClass *klass = GV_LAYER_CLASS(G_OBJECT_GET_CLASS(layer));
            if (klass->draw)
                klass->draw(layer, view);
        }
        else
        {
            /* Same result as gtk_signal_emit... */
            g_signal_emit(layer, layer_signals[DRAW], 0, view);
        }
    }
}

void
gv_layer_extents(GvLayer *layer, GvRect *rect)
{
    rect->x = rect->y = rect->width = rect->height = 0.0;
    g_signal_emit(layer, layer_signals[EXTENTS_REQUEST], 0, rect);
}

void
gv_layer_display_change(GvLayer *layer, gpointer change_info)
{
    g_signal_emit(layer, layer_signals[DISPLAY_CHANGE], 0,
                    change_info);
}

gint
gv_layer_is_visible(GvLayer *layer)
{
    return (layer->invisible == FALSE);
}

void
gv_layer_set_visible(GvLayer *layer, gint visible)
{
    gint invisible = !visible;
    if (invisible != layer->invisible)
    {
        layer->invisible = invisible;
        gv_layer_display_change(layer, NULL);
    }    
}

gint
gv_layer_set_visible_temp(GvLayer *layer, gint visible)
{
    gint old_visible = !layer->invisible;
    layer->invisible = !visible;
    return old_visible;
}

void
gv_layer_set_presentation(GvLayer *layer, gint presentation)
{
    layer->presentation = presentation;
}

gint 
gv_layer_reproject(GvLayer *layer, const char *projection)
{
    GvLayerClass *klass = GV_LAYER_CLASS(G_OBJECT_GET_CLASS(layer));

    if (klass->reproject)
        return klass->reproject(layer, projection);
    else
        return FALSE;
}

GvViewArea *
gv_layer_get_view(GvLayer *layer)
{
    return GV_LAYER(layer)->view;
}
