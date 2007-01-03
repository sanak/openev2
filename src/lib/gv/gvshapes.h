/******************************************************************************
 * $Id: gvshapes.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Definitions of GvShape (single object) and GvShapes (data
 *           container).
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
 * $Log: gvshapes.h,v $
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
 * Revision 1.26  2005/01/14 16:51:51  gmwalter
 * Checked in Aude's gv_shapes_add_shape_last function
 * (allows shapes to be added without repeating
 * indices if others have been deleted).
 *
 * Revision 1.25  2005/01/04 18:50:31  gmwalter
 * Checked in Aude's new gvshape function changes.
 *
 * Revision 1.24  2003/08/29 20:52:43  warmerda
 * added to/from xml translation for GvShape
 *
 * Revision 1.23  2003/06/25 17:06:06  warmerda
 * added gv_shape_rotate(), gv_shape_scale() and related stuff
 *
 * Revision 1.22  2003/01/06 21:20:03  warmerda
 * added gv_shapes_from_ogr_layer
 *
 * Revision 1.21  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.20  2002/07/18 19:33:57  pgs
 * added gv_shapes_to_dbf
 *
 * Revision 1.19  2002/05/07 02:51:15  warmerda
 * preliminary support for GVSHAPE_COLLECTION
 *
 * Revision 1.18  2002/03/07 18:31:56  warmerda
 * added preliminary gv_shape_clip_to_rect() implementation
 *
 * Revision 1.17  2002/03/07 02:31:56  warmerda
 * added default_height to add_height functions
 *
 * Revision 1.16  2001/12/08 04:49:38  warmerda
 * added point in polygon test
 *
 * Revision 1.15  2001/08/08 17:45:48  warmerda
 * GvShape now referenced counted
 *
 * Revision 1.14  2001/01/18 16:48:14  warmerda
 * added gv_shapes_add_height() and wrappers
 *
 * Revision 1.13  2000/08/04 14:14:12  warmerda
 * GvShapes shape ids now persistent
 *
 * Revision 1.12  2000/07/14 14:51:01  warmerda
 * fixed insert, and delete node support
 *
 * Revision 1.11  2000/07/13 19:08:37  warmerda
 * added coping optional for gv_shapes_replace_shapes
 *
 * Revision 1.10  2000/06/28 13:10:42  warmerda
 * added preliminary OGR support
 *
 * Revision 1.9  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_SHAPES_H__
#define __GV_SHAPES_H__

#include <gtk/gtk.h>
#include "gvdata.h"
#include "gvshape.h"
#include "cpl_minixml.h"

#define GV_TYPE_SHAPES            (gv_shapes_get_type ())
#define GV_SHAPES(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_SHAPES,\
                                                   GvShapes))
#define GV_SHAPES_CLASS(klass)    (GTK_CHECK_CLASS_CAST((klass),GV_TYPE_SHAPES,\
                                                        GvShapesClass))
#define GV_IS_SHAPES(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_SHAPES))
#define GV_IS_SHAPES_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_SHAPES))

typedef struct _GvShapes GvShapes;
typedef struct _GvShapesClass GvShapesClass;

struct _GvShapes
{
    GvData data;
    GPtrArray *shapes;
    int actual_num_shapes;  /* not including NULLs in GPtrArray */
    GvRect extents;
    guint extents_valid : 1;
};

struct _GvShapesClass
{
    GvDataClass parent_class;
};

#define gv_shapes_num_shapes(adata) \
     (adata->shapes->len)

#define gv_shapes_get_shape(adata,id) \
     ((GvShape*)g_ptr_array_index(adata->shapes, id))

GtkType  gv_shapes_get_type (void);
GvData* gv_shapes_new(void);

GvData* gv_shapes_from_shapefile(const char *filename);
GvData *gv_shapes_from_ogr(const char *filename, int iLayer);
GvData* gv_shapes_from_ogr_layer(void *ogr_layer);

void    gv_shapes_add_height(GvShapes *shapes, GvData *raster, double offset,
                             double default_height);
int gv_shapes_to_shapefile(const char *filename, GvData *shapes_data,
                           int shp_type);
int gv_shapes_to_dbf(const char *filename, GvData *shapes_data);
void gv_shapes_get_extents(GvShapes *shapes, GvRect *rect);
gint gv_shapes_add_shape(GvShapes *shapes, GvShape *shape);
gint gv_shapes_add_shape_last(GvShapes *shapes, GvShape *shape);
void gv_shapes_replace_shapes(GvShapes *shapes, gint num_shapes,
                              gint *shape_id, GvShape **shps, int make_copy);
void gv_shapes_delete_shapes(GvShapes *shapes, gint num_shapes, gint*shapeids);
void gv_shapes_translate_shapes(GvShapes *shapes, gint num_shapes,
                                gint *shapeids, gvgeocoord dx, gvgeocoord dy);

#ifdef HAVE_OGR
#define gv_have_ogr_support() 1
#else
#define gv_have_ogr_support() 0
#endif

#endif /*__GV_SHAPES_H__ */
