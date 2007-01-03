/******************************************************************************
 * $Id: gvareas.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Data container of areas (superceeded by GvShapes-will be removed)
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
 * $Log: gvareas.h,v $
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
 * Revision 1.11  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.10  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_AREAS_H__
#define __GV_AREAS_H__

#include "gvdata.h"

#define GV_TYPE_AREAS            (gv_areas_get_type ())
#define GV_AREAS(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_AREAS, GvAreas))
#define GV_AREAS_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_AREAS, GvAreasClass))
#define GV_IS_AREAS(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_AREAS))
#define GV_IS_AREAS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_AREAS))

typedef struct _GvArea        GvArea;
typedef struct _GvAreas       GvAreas;
typedef struct _GvAreasClass  GvAreasClass;

struct _GvArea
{
    GPtrArray *rings;
    gint      fill_objects;
    GArray    *mode_offset;
    GArray    *fill;
};

struct _GvAreas
{
    GvData data;

    GPtrArray *areas;
    GvRect extents;
    guint extents_valid : 1;
};

struct _GvAreasClass
{
    GvDataClass parent_class;
};

GvArea* gv_area_new(gint alloc_outer_ring);
GvArea* gv_area_copy(GvArea *area);
void gv_area_delete(GvArea *area);

GtkType    gv_areas_get_type (void);

GvData* gv_areas_new(void);
gint gv_areas_new_area(GvAreas *areas);
gint gv_areas_new_area_with_data(GvAreas *areas, GvArea *area_data);

void gv_areas_delete_areas(GvAreas *areas, gint num_areas, gint *area_id);
void gv_areas_translate_areas(GvAreas *areas, gint num_areas, gint *area_id, gvgeocoord dx, gvgeocoord dy);

gint gv_areas_new_ring(GvAreas *areas, gint area_id);
void gv_areas_delete_ring(GvAreas *areas, gint area_id, gint ring_id);
void gv_areas_append_nodes(GvAreas *areas, gint area_id, gint ring_id, gint num_nodes, GvVertex *vertex);
GvVertex* gv_areas_get_node(GvAreas *areas, gint area_id, gint ring_id, gint node_id);
void gv_areas_move_node(GvAreas *areas, gint area_id, gint ring_id, gint node_id, GvVertex *vertex);
void gv_areas_insert_nodes(GvAreas *areas, gint area_id, gint ring_id, gint node_id, gint num_nodes, GvVertex *vertex);
void gv_areas_delete_nodes(GvAreas *areas, gint area_id, gint ring_id, gint num_nodes, gint *node_id);

gint gv_areas_tessellate_areas(GvAreas *areas, gint num_areas, gint *area_id);
void gv_areas_clear_fill(GvAreas *areas, gint area_id);
void gv_areas_get_extents(GvAreas *areas, GvRect *rect);

#define gv_areas_num_areas(adata) \
     (adata->areas->len)
#define gv_areas_get_area(adata,id) \
     ((GvArea*)g_ptr_array_index(adata->areas, id))
#define gv_areas_num_rings(area) \
     (area->rings->len)
#define gv_areas_get_ring(area,id) \
     ((GArray*)g_ptr_array_index(area->rings, id))

#endif /*__GV_AREAS_H__ */
