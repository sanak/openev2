/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Display layer for vector shapes.
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
 * $Log: gvshapeslayer.h,v $
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
 * Revision 1.18  2003/09/12 17:35:43  warmerda
 * Added logic to aggregate selection boxes in the drawinfo.selection_box
 * rectangle when draw_mode == NORMAL_GET_BOX.  This is intended to allow
 * us to draw a selection box around a complex shapes consisting of multiple
 * parts offset from the reference point.  In the past we only drew the
 * selection around the first part in such cases.  This seems to work though
 * the testing isn't ... extensive.
 *
 * Revision 1.17  2003/09/02 17:22:27  warmerda
 * added per-layer symbol manager support
 *
 * Revision 1.16  2003/08/27 20:03:05  warmerda
 * Added geo2screen_works to drawinfo structure.  It is set to false for text
 * drawn within symbols since symbol rescaling via glScale() will mess up
 * the bmfont_draw() logic for ensuring text is drawn "on screen".  True
 * otherwise.  It is passed on to gv_view_area_bmfont_draw().
 *
 * Revision 1.15  2003/05/16 21:31:26  warmerda
 * added support for passing default color down to components of a symbol
 *
 * Revision 1.14  2003/05/16 17:42:40  warmerda
 * fix up pixel offsets for sub-symbols
 *
 * Revision 1.13  2003/02/28 16:49:42  warmerda
 * preliminary support for symbols rendered from GvShapes
 *
 * Revision 1.12  2003/02/27 04:08:57  warmerda
 * added view to gv_shapes_layer_get_draw_info
 *
 * Revision 1.11  2003/02/14 22:55:57  pgs
 * added support for _line_width and _area_edge_width
 *
 * Revision 1.10  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.9  2002/09/27 18:16:21  pgs
 * added display list support and antialiasing support for line layers
 *
 * Revision 1.8  2001/04/09 18:22:22  warmerda
 * use renderinfo for ogrfs, added text selection box
 *
 * Revision 1.7  2001/03/21 22:40:18  warmerda
 * allow setting _gv_ogrfs on layer, and add support for text from fields
 *
 * Revision 1.6  2000/08/14 15:08:08  warmerda
 * make gv_shapes_layer_override_color public
 *
 * Revision 1.5  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_SHAPES_LAYER_H__
#define __GV_SHAPES_LAYER_H__

#include "gvshapelayer.h"
#include "gvshapes.h"

#define GV_TYPE_SHAPES_LAYER            (gv_shapes_layer_get_type ())
#define GV_SHAPES_LAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_SHAPES_LAYER,\
                                                   GvShapesLayer))
#define GV_SHAPES_LAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GV_TYPE_SHAPES_LAYER,\
                                                        GvShapesLayerClass))
#define GV_IS_SHAPES_LAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_SHAPES_LAYER))
#define GV_IS_SHAPES_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_SHAPES_LAYER))
#define GV_SHAPES_LAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),GV_TYPE_SHAPES_LAYER,\
                                                        GvShapesLayerClass))

typedef struct _GvShapesLayer       GvShapesLayer;
typedef struct _GvShapesLayerClass  GvShapesLayerClass;

struct _GvShapesLayer
{
    GvShapeLayer shape_layer;

    GvShapes *data;
    gint edit_ring;
    gint display_list;

    GObject *symbol_manager;
};

typedef enum
{
    NORMAL,
    SELECTED,
    PICKING,
    NORMAL_GET_BOX,
    PQUERY_POINTS,
    PQUERY_LABELS
} gv_draw_mode;

typedef struct
{
    GvColor  color;
    GvColor  point_color;
    GvColor  line_color;
    float line_width;
    GvColor  area_fill_color;
    GvColor  area_edge_color;
    float area_edge_width;
    gvgeocoord   point_size;
    gvgeocoord   dx, dy, dunit, dpixel;
    int      geo2screen_works; /* this doesn't work if extra GL scalings
                                  are in effect for sub-symbols */

    char     ogrfs[2048];
    
    /* used to aggregate and return selection box extents. */
    int      box_set;
    GvRect   selection_box;

    PangoLayout *pango_layout;
} GvShapeDrawInfo;

struct _GvShapesLayerClass
{
    GvShapeLayerClass parent_class;
    void (* draw_shape)         (GvViewArea *view, GvShapesLayer *layer,
                                int part_index, GvShape *shape_obj, 
                                gv_draw_mode draw_mode,
                                GvShapeDrawInfo *drawinfo);
};

void gv_draw_info_aggregate_select_region(GvShapeDrawInfo *, 
                                           double x, double y);

GType gv_shapes_layer_get_type(void);
GObject* gv_shapes_layer_new(void *data);
gint gv_shapes_layer_select_new_shape(GvShapesLayer *layer, GvShape *shape);
void gv_shapes_layer_set_data(GvShapesLayer *layer, GvShapes *data);
void gv_shapes_layer_draw_selected(GvShapeLayer *layer, GvViewArea *view);
void gv_shapes_layer_draw(GvLayer *layer, GvViewArea *view);
void gv_shapes_layer_get_draw_info(GvViewArea *view, GvShapesLayer *layer,
                                   GvShapeDrawInfo *drawinfo);
void gv_shapes_layer_override_color(GvShape *shape, GvColor color,
                                    const char *property_name);

GObject *gv_shapes_layer_get_symbol_manager(GvShapesLayer *layer, 
                                               int ok_to_create);

void gv_shapes_layer_draw_shape(GvViewArea *view, GvShapesLayer *layer,
                                int part_index, GvShape *shape_obj, 
                                gv_draw_mode draw_mode,
                                GvShapeDrawInfo *drawinfo);

#endif /* __GV_SHAPES_LAYER_H__ */
