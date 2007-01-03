/******************************************************************************
 * gvshape.h
 *
 * Project:  OpenEV
 * Purpose:  Definitions of GvShape (single object)
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
 *****************************************************************************/

#ifndef __GV_SHAPE_H__
#define __GV_SHAPE_H__

#include "gvdata.h"
#include "cpl_minixml.h"

#define GVSHAPE_NONE       0
#define GVSHAPE_POINT      1
#define GVSHAPE_LINE       2
#define GVSHAPE_AREA       3
#define GVSHAPE_COLLECTION 4

#define GVSF_TYPE_MASK      0x07
#define GVSF_CUSTOM_RENDER  0x08

typedef struct
{
    guint     flags;
    guint     ref_count;
    GvProperties properties;
} GvShape;

typedef struct
{
    guint     flags;
    guint     ref_count;
    GvProperties properties;
    gvgeocoord     x;
    gvgeocoord     y;
    gvgeocoord     z;
} GvPointShape;

typedef struct
{
    guint     flags;
    guint     ref_count;
    GvProperties properties;
    int       num_nodes;
    gvgeocoord     *xyz_nodes;
} GvLineShape;

typedef struct
{
    guint     flags;
    guint     ref_count;
    GvProperties properties;
    int       num_rings;
    int       *num_ring_nodes;
    gvgeocoord     **xyz_ring_nodes;

    /* tesselation information */
    gint      fill_objects; /* -1 is untesselated, -2 is `do not tesselate' */
    GArray    *mode_offset;
    GArray    *fill;
} GvAreaShape;

typedef struct
{
    guint     flags;
    guint     ref_count;
    GvProperties properties;
    int       geom_count;
    GvShape   **geom_list;
} GvCollectionShape;

#define gv_shape_get_properties(shape) (&(shape)->properties)
#define gv_shape_type(shape) ((shape)->flags & GVSF_TYPE_MASK)
#define GV_SHAPE_TYPE(shape) ((shape)->flags & GVSF_TYPE_MASK)

#define gv_shape_get_x(shape,ring,node) gv_shape_get_xyz(shape,ring,node,0)
#define gv_shape_get_y(shape,ring,node) gv_shape_get_xyz(shape,ring,node,1)
#define gv_shape_get_z(shape,ring,node) gv_shape_get_xyz(shape,ring,node,2)

GvShape* gv_shape_new(gint type);
GvShape* gv_shape_from_xml_tree(CPLXMLNode *);
CPLXMLNode *gv_shape_to_xml_tree(GvShape *psShape);

void     gv_shape_unref(GvShape *shape);
void     gv_shape_ref(GvShape *shape);
int      gv_shape_get_ref(GvShape *shape);
void     gv_shape_delete(GvShape *shape);
GvShape* gv_shape_copy(GvShape *shape);
gint     gv_shape_get_rings(GvShape *shape);
gint     gv_shape_get_nodes(GvShape *shape, gint ring);
gvgeocoord  gv_shape_get_xyz(GvShape *shape, gint ring, gint node, gint off);
void     gv_shape_get_extents(GvShape *shape, GvRect *rect);

gint     gv_shape_set_xyz(GvShape *shape, gint ring, gint node,
                           gvgeocoord x, gvgeocoord y, gvgeocoord z);
gint     gv_shape_add_node(GvShape *shape, gint ring,
                             gvgeocoord x, gvgeocoord y, gvgeocoord z);
gint     gv_shape_insert_node(GvShape *shape, gint ring, int node,
                               gvgeocoord x, gvgeocoord y, gvgeocoord z);
gint     gv_shape_delete_node(GvShape *shape, gint ring, gint node);
gint     gv_shape_delete_ring(GvShape *shape, gint ring);
gint     gv_shape_point_in_polygon(GvShape *shape_poly, double x, double y);
gdouble gv_shape_distance_from_polygon(GvShape *shape_poly, double x, double y);
GvShape *gv_shape_clip_to_rect(GvShape *shape, GvRect *rect);

void     gv_shape_collection_add_shape(GvShape *collection, GvShape *shape);
GvShape *gv_shape_collection_get_shape(GvShape *collection, int shp_index);
int      gv_shape_collection_get_count(GvShape *collection);

gint     gv_area_shape_tessellate(GvAreaShape *area);

int      gv_shape_get_count(void);
gint     gv_shape_update_attribute(GvShape *shape, const char *tool, 
                                    const char *attribute, 
                                    const char *update_value);
gint gv_shape_get_center(GvShape *shape, GvVertex3d *xyz);
gint gv_shape_rotate(GvShape *shape, double angle_in_degrees);
gint gv_shape_scale(GvShape *shape, double new_scale);

#endif /*__GV_SHAPE_H__ */
