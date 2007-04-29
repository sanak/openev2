/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Geometric mesh mapping tile s/t coordinates to display x/y/z
 *           coordinates.
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

#ifndef __GV_MESH_H__
#define __GV_MESH_H__

#include "gvdata.h"
#include "gvviewarea.h"
#include "gvraster.h"

#define GV_TYPE_MESH            (gv_mesh_get_type ())
#define GV_MESH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_MESH, GvMesh))
#define GV_MESH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_MESH, GvMeshClass))
#define GV_IS_MESH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_MESH))
#define GV_IS_MESH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_MESH))

typedef struct _GvMesh       GvMesh;
typedef struct _GvMeshClass  GvMeshClass;
typedef struct _GvMeshTile   GvMeshTile;

struct _GvMeshTile {
    gint list_type; /* 0 = TRIANGLES, 1 = TRISTRIPS, 2 = QUADS, 3 = QUADSTRIPGS */
    gint range;     /* Only used for "strip" processing */
    gint restarts;
    gint *indices;
    gfloat *vertices;
    gfloat *tex_coords;
};

struct _GvMesh
{
    GvData data;

    struct _GvRaster *raster;

    gint max_tiles;
    gint tile_x, tile_y;
    gint width, height;
    gint detail;
    gfloat corner_coords[8];

    GArray *vertices;

    GPtrArray *tex_coords;
    GPtrArray *tex_coords_right;
    GPtrArray *tex_coords_bottom;
    GPtrArray *tex_coords_bottom_right;
};

struct _GvMeshClass
{
    GvDataClass parent_class;
};

struct _GvRaster;

GType gv_mesh_get_type(void);

GvMeshTile *gv_mesh_get(GvMesh *mesh, gint tile, gint raster_lod, gint detail,
                        GvMeshTile *tile_info);
gint * gv_mesh_get_tile_corner_coords(GvMesh *mesh, int tile);
GvMesh *gv_mesh_new_identity(struct _GvRaster *raster, gint detail);
void gv_mesh_reset_to_identity(GvMesh *);
void gv_mesh_add_height(GvMesh *mesh, struct _GvRaster *raster, 
                        double default_height);
void gv_mesh_clamp_height(GvMesh *mesh, int bclamp_min, int bclamp_max,
                          double min_height, double max_height);
void gv_mesh_set_transform(GvMesh *mesh, gint xsize, gint ysize,
                            double *geotransform);
void gv_mesh_extents(GvMesh *mesh, GvRect *rect);
float gv_mesh_get_height(GvMesh *mesh, double x, double y, int *success);

#endif /* __GV_RASTER_H__ */
