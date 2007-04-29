/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Definitions of GvShape (single object) and GvShapes (data
 *           container).
 * Author:   Frank Warmerdam, warmerda@home.com
 * Maintainer: Mario Beauchamp, starged@gmail.com
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
 */

#ifndef __GV_SHAPES_H__
#define __GV_SHAPES_H__

#include "gvdata.h"
#include "gvshape.h"
#include "cpl_minixml.h"

#define GV_TYPE_SHAPES            (gv_shapes_get_type ())
#define GV_SHAPES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_SHAPES,\
                                                   GvShapes))
#define GV_SHAPES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GV_TYPE_SHAPES,\
                                                        GvShapesClass))
#define GV_IS_SHAPES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_SHAPES))
#define GV_IS_SHAPES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_SHAPES))

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

GType  gv_shapes_get_type (void);
GvData* gv_shapes_new(void);

GvData* gv_shapes_from_shapefile(const char *filename);
void gv_shapes_read_from_file(const char *filename, GvShapes *shapes);
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
