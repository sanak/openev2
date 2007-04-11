/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Base class for vector display layers (eventually this will
 *           merge with GvShapesLayer).
 * Author:   OpenEV Team
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
 * $Log: gvshapelayer.h,v $
 * Revision 1.1.1.1  2005/04/18 16:38:34  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:26  uid1026
 *
 * Imported sources
 *
 * Revision 1.11  2003/02/27 04:00:19  warmerda
 * added scale_dep flag handling
 *
 * Revision 1.10  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.9  2002/02/22 20:16:07  warmerda
 * added brush tool support
 *
 * Revision 1.8  2002/02/22 19:27:16  warmerda
 * added support for pen tools
 *
 * Revision 1.7  2001/04/09 18:17:34  warmerda
 * added subselection, and renderinfo support
 *
 * Revision 1.6  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_SHAPE_LAYER_H__
#define __GV_SHAPE_LAYER_H__

#include <gdk/gdk.h>
#include "gvlayer.h"
#include "gvviewarea.h"

#define GV_TYPE_SHAPE_LAYER            (gv_shape_layer_get_type ())
#define GV_SHAPE_LAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_SHAPE_LAYER,\
                                                   GvShapeLayer))
#define GV_SHAPE_LAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GV_TYPE_SHAPE_LAYER,\
                                                        GvShapeLayerClass))
#define GV_IS_SHAPE_LAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_SHAPE_LAYER))
#define GV_IS_SHAPE_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_SHAPE_LAYER))

/* Access macro for selected buffer (for use by subclass) */
#define GV_SHAPE_LAYER_SELBUF(layer) \
     ((gint*)(GV_SHAPE_LAYER(layer)->selected->data))

typedef struct _GvShapeLayer       GvShapeLayer;
typedef struct _GvShapeLayerClass  GvShapeLayerClass;

struct _GvShapeLayer
{
    GvLayer layer;
    GArray *selected;
    gint    subselection;
    guint flags;
    GvColor color;
    GvVertex selected_motion;

    /* All related to keeping feature rendering information */
    GArray  *render_index;

    GArray  *label_render;
    GArray  *symbol_render;
    GArray  *pen_render;
    GArray  *brush_render;

    gint    *scale_dep_flags;
};

struct _GvShapeLayerClass
{
    GvLayerClass parent_class;

    void (* draw_selected)      (GvShapeLayer *layer, GvViewArea *view);
    void (* delete_selected)    (GvShapeLayer *layer);
    void (* translate_selected) (GvShapeLayer *layer, GvVertex *delta);
    void (* pick_shape)         (GvShapeLayer *layer);
    void (* pick_node)          (GvShapeLayer *layer);
    void (* get_node)           (GvShapeLayer *layer, GvNodeInfo *info);
    void (* move_node)          (GvShapeLayer *layer, GvNodeInfo *info);
    void (* insert_node)        (GvShapeLayer *layer, GvNodeInfo *info);
    void (* delete_node)        (GvShapeLayer *layer, GvNodeInfo *info);
    void (* node_motion)        (GvShapeLayer *layer, gint shape_id);
    void (* selection_changed)  (GvShapeLayer *layer);
    void (* subselection_changed)(GvShapeLayer *layer);
};

GType    gv_shape_layer_get_type (void);

void gv_shape_layer_select_shape(GvShapeLayer *layer, gint shape_id);
void gv_shape_layer_deselect_shape(GvShapeLayer *layer, gint shape_id);
void gv_shape_layer_clear_selection(GvShapeLayer *layer);
void gv_shape_layer_select_all(GvShapeLayer *layer);
void gv_shape_layer_draw_selected(GvShapeLayer *layer, guint when, GvViewArea *view);
gint gv_shape_layer_is_selected(GvShapeLayer *layer, gint shape_id);
gint gv_shape_layer_selected(GvShapeLayer *layer, gint what, void *retval);
gint gv_shape_layer_get_selected(GvShapeLayer *layer, GArray *shapes);
void gv_shape_layer_delete_selected(GvShapeLayer *layer);
void gv_shape_layer_translate_selected(GvShapeLayer *layer, gvgeocoord dx, gvgeocoord dy);
void gv_shape_layer_selected_motion(GvShapeLayer *layer, gvgeocoord dx, gvgeocoord dy);

void gv_shape_layer_subselect_shape(GvShapeLayer *layer, gint shape_id);
gint gv_shape_layer_get_subselection(GvShapeLayer *layer);

void gv_shape_layer_set_scale_dep(GvShapeLayer *layer, gint shape_id, int dep);
int  gv_shape_layer_get_scale_dep(GvShapeLayer *layer, gint shape_id);

void gv_shape_layer_get_node          (GvShapeLayer *layer, GvNodeInfo *info);
void gv_shape_layer_move_node         (GvShapeLayer *layer, GvNodeInfo *info);
void gv_shape_layer_insert_node       (GvShapeLayer *layer, GvNodeInfo *info);
void gv_shape_layer_delete_node       (GvShapeLayer *layer, GvNodeInfo *info);
void gv_shape_layer_node_motion       (GvShapeLayer *layer, gint shape_id);

void gv_shape_layer_select_region     (GvShapeLayer *layer, GvViewArea *view, GvRect *rect);
gint gv_shape_layer_pick_shape        (GvShapeLayer *layer, GvViewArea *view, gvgeocoord x, gvgeocoord y, gint *shape_id);
gint gv_shape_layer_pick_node         (GvShapeLayer *layer, GvViewArea *view, gvgeocoord x, gvgeocoord y, gint *before, GvNodeInfo *node_info);

void gv_shape_layer_set_num_shapes(GvShapeLayer *layer, gint num_shapes);
void gv_shape_layer_set_color(GvShapeLayer *layer, GvColor color);

#endif /*__GV_SHAPE_LAYER_H__ */
