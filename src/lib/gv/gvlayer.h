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
 * $Log: gvlayer.h,v $
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
 * Revision 1.10  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_LAYER_H__
#define __GV_LAYER_H__

#include <gdk/gdk.h>
#include "gvdata.h"
#include "gvviewarea.h"

#define GV_TYPE_LAYER            (gv_layer_get_type ())
#define GV_LAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_LAYER, GvLayer))
#define GV_LAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_LAYER, GvLayerClass))
#define GV_IS_LAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_LAYER))
#define GV_IS_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_LAYER))

typedef struct _GvLayer       GvLayer;
typedef struct _GvLayerClass  GvLayerClass;

struct _GvLayer
{
    GvData data;

    gint setup_count;        /* Prevents multiple setup/teardown events */
    guint invisible : 1;
    guint presentation : 1;  /* Presentation mode drawing (e.g. printing) */

    char *projection;        /* in WKT format */

    GvViewArea	*view;

    int		pending_idle;/* Is there pending idle work for this layer? */
};

struct _GvLayerClass
{
    GvDataClass parent_class;

    /* the following are signals */
    void (* setup)           (GvLayer *layer, GvViewArea *view);
    void (* teardown)        (GvLayer *layer, GvViewArea *view);
    void (* draw)            (GvLayer *layer, GvViewArea *view);
    void (* extents_request) (GvLayer *layer, GvRect *rect);
    void (* display_change)  (GvLayer *layer, gpointer change_info);

    /* this is just a hook function, not a signal */
    gint (* reproject)       (GvLayer *layer, const char *);
};

GType    gv_layer_get_type (void);

void gv_layer_setup(GvLayer *layer, GvViewArea *view);
void gv_layer_teardown(GvLayer *layer, GvViewArea *view);
void gv_layer_draw(GvLayer *layer, GvViewArea *view);
void gv_layer_extents(GvLayer *layer, GvRect *rect);
void gv_layer_display_change(GvLayer *layer, gpointer change_info);
gint gv_layer_is_visible(GvLayer *layer);
void gv_layer_set_visible(GvLayer *layer, gint visible);
gint gv_layer_set_visible_temp(GvLayer *layer, gint visible);
void gv_layer_set_presentation(GvLayer *layer, gint presentation);
GvViewArea *gv_layer_get_view(GvLayer *layer);
gint gv_layer_reproject(GvLayer *layer, const char *projection);

#endif /*__GV_LAYER_H__ */

