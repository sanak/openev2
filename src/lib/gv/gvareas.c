/******************************************************************************
 * $Id: gvareas.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
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
 * $Log: gvareas.c,v $
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
 * Revision 1.15  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.14  2000/06/20 13:26:54  warmerda
 * added standard headers
 *
 */

#include "gextra.h"
#include "gvareas.h"
#include "gvtess.h"

typedef struct _GvAreasMemento GvAreasMemento;

struct _GvAreasMemento
{
    GvDataMemento base;
    GArray *area_ids;
    GPtrArray *areas;
};

static void gv_areas_class_init(GvAreasClass *klass);
static void gv_areas_init(GvAreas *pgdata);
static void gv_areas_replace_areas(GvAreas *areas, gint num_areas, gint *area_id, GvArea **area);
static void gv_areas_insert_areas(GvAreas *areas, gint num_areas, gint *area_ids, GvArea **area);
static void gv_areas_get_memento(GvData *data, gpointer info, GvDataMemento **memento);
static void gv_areas_set_memento(GvData *data, GvDataMemento *memento);
static void gv_areas_del_memento(GvData *data, GvDataMemento *memento);
static void gv_areas_changed(GvData *data, gpointer change_info);
static void gv_areas_finalize(GObject *gobject);

GtkType
gv_areas_get_type(void)
{
    static GtkType areas_type = 0;

    if (!areas_type)
    {
	static const GtkTypeInfo areas_info =
	{
	    "GvAreas",
	    sizeof(GvAreas),
	    sizeof(GvAreasClass),
	    (GtkClassInitFunc) gv_areas_class_init,
	    (GtkObjectInitFunc) gv_areas_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	areas_type = gtk_type_unique(gv_data_get_type(), &areas_info);
    }
    return areas_type;
}

static void
gv_areas_init(GvAreas *areas)
{
    areas->areas = g_ptr_array_new();
    areas->extents_valid = FALSE;
}

static void
gv_areas_class_init(GvAreasClass *klass)
{
    GvDataClass *data_class;

    data_class = (GvDataClass*) klass;

    /* ---- Override finalize ---- */
    (G_OBJECT_CLASS(klass))->finalize = gv_areas_finalize;

    /* GTK2 PORT...
    GtkObjectClass *object_class;
    object_class = (GtkObjectClass*) klass;
    object_class->finalize = gv_areas_finalize;
    */

    data_class->changed = gv_areas_changed;
    data_class->get_memento = gv_areas_get_memento;
    data_class->set_memento = gv_areas_set_memento;
    data_class->del_memento = gv_areas_del_memento;
}

GvData *
gv_areas_new(void)
{
    return GV_DATA(gtk_type_new(gv_areas_get_type()));
}

gint
gv_areas_new_area(GvAreas *areas)
{
    return gv_areas_new_area_with_data(areas, NULL);
}

gint
gv_areas_new_area_with_data(GvAreas *areas, GvArea *area_data)
{
    GvArea *area;
    int area_id;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 1, NULL};

    change_info.shape_id = &area_id;

    area_id = areas->areas->len;
    if (area_data)
    {
	area = gv_area_copy(area_data);
    }
    else
    {
	area = gv_area_new(TRUE);
    }
    g_return_val_if_fail(area, 0);

    gv_data_changing(GV_DATA(areas), &change_info);
    
    g_ptr_array_add(areas->areas, area);
    
    gv_data_changed(GV_DATA(areas), &change_info);
    
    /* Generate tesselation if necessary */
    if (area_data)
    {
	gv_areas_tessellate_areas(areas, 1, &area_id);
    }

    return area_id;
}

void
gv_areas_delete_areas(GvAreas *areas, gint num_areas, gint *area_id)
{
    GvArea *area;
    GvShapeChangeInfo change_info = {GV_CHANGE_DELETE, 0, NULL};

    change_info.num_shapes = num_areas;
    change_info.shape_id = area_id;
    
    gv_data_changing(GV_DATA(areas), &change_info);
    
    if (num_areas == 1)
    {
	area = (GvArea*)g_ptr_array_remove_index_fast(areas->areas, *area_id);
	if (area) gv_area_delete(area);
    }
    else
    {
	/* Strategy: sort the area_id buffer and delete lines in descending
	   order, so that indicies remain valid */
	gint *id, i;

	id = g_memdup_type(area_id, gint, num_areas);
	g_sort_type(id, gint, num_areas);

	for (i=num_areas-1; i >= 0; --i)
	{
	    area = (GvArea*)g_ptr_array_remove_index_fast(areas->areas, id[i]);
	    if (area) gv_area_delete(area);
	}
	g_free(id);
    }
    gv_data_changed(GV_DATA(areas), &change_info);
}

void
gv_areas_translate_areas(GvAreas *areas, gint num_areas, gint *area_id,
						 gvgeocoord dx, gvgeocoord dy)
{
    int i, j, k;
    GArray *ring;
    GvArea *area;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};

    change_info.num_shapes = num_areas;
    change_info.shape_id = area_id;

    gv_data_changing(GV_DATA(areas), &change_info);

    for (k=0; k < num_areas; ++k)
    {
	g_return_if_fail(area_id[k] >= 0 && area_id[k] < areas->areas->len);
	area = gv_areas_get_area(areas, area_id[k]);
	
	for (i=0; i < area->rings->len; ++i)
	{
	    ring = gv_areas_get_ring(area, i);
	    for (j=0; j < ring->len; ++j)
	    {
		g_array_index(ring, GvVertex, j).x += dx;
		g_array_index(ring, GvVertex, j).y += dy;
	    }
	}
	
	if (area->fill_objects > 0)
	{
	    for (i=0; i < area->fill->len; ++i)
	    {
		g_array_index(area->fill, GvVertex, i).x += dx;
		g_array_index(area->fill, GvVertex, i).y += dy;
	    }
	}
    }
    
    gv_data_changed(GV_DATA(areas), &change_info);
}

gint
gv_areas_new_ring(GvAreas *areas, gint area_id)
{
    GArray *ring;
    GvArea *area;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &area_id;

    g_return_val_if_fail(area_id >= 0 && area_id < areas->areas->len, 0);
    area = gv_areas_get_area(areas, area_id);

    gv_data_changing(GV_DATA(areas), &change_info);
    
    ring = g_array_new(FALSE, FALSE, sizeof(GvVertex));
    g_ptr_array_add(area->rings, ring);

    gv_areas_clear_fill(areas, area_id);
    area->fill_objects = -1;
    
    gv_data_changed(GV_DATA(areas), &change_info);
    
    return area->rings->len - 1;
}

void
gv_areas_delete_ring(GvAreas *areas, gint area_id, gint ring_id)
{
    GvArea *area;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &area_id;

    /* Can't delete ring 0 (outer ring) */
    g_return_if_fail(ring_id > 0);
    
    g_return_if_fail(area_id >= 0 && area_id < areas->areas->len);
    area = gv_areas_get_area(areas, area_id);

    gv_data_changing(GV_DATA(areas), &change_info);
    
    g_ptr_array_remove_index(area->rings, ring_id);

    gv_data_changed(GV_DATA(areas), &change_info);    
}

void
gv_areas_append_nodes(GvAreas *areas, gint area_id, gint ring_id,
		      gint num_nodes, GvVertex *vertex)
{
    GvArea *area;
    GArray *ring;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &area_id;

    g_return_if_fail(area_id >= 0 && area_id < areas->areas->len);
    area = gv_areas_get_area(areas, area_id);
    g_return_if_fail(ring_id >= 0 && ring_id < area->rings->len);
    ring = gv_areas_get_ring(area, ring_id);

    gv_data_changing(GV_DATA(areas), &change_info);
    
    g_array_append_vals(ring, vertex, num_nodes);

    gv_data_changed(GV_DATA(areas), &change_info);    
}

GvVertex *
gv_areas_get_node(GvAreas *areas, gint area_id, gint ring_id, gint node_id)
{
    GvArea *area;
    GArray *ring;

    g_return_val_if_fail(area_id >= 0 && area_id < areas->areas->len, NULL);
    area = gv_areas_get_area(areas, area_id);
    g_return_val_if_fail(ring_id >= 0 && ring_id < area->rings->len, NULL);
    ring = gv_areas_get_ring(area, ring_id);
    g_return_val_if_fail(node_id >= 0 && node_id < ring->len, NULL);
    
    return &g_array_index(ring, GvVertex, node_id);
}

void
gv_areas_move_node(GvAreas *areas, gint area_id, gint ring_id, gint node_id,
		   GvVertex *vertex)
{
    GvArea *area;
    GArray *ring;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &area_id;

    g_return_if_fail(area_id >= 0 && area_id < areas->areas->len);
    area = gv_areas_get_area(areas, area_id);
    g_return_if_fail(ring_id >= 0 && ring_id < area->rings->len);
    ring = gv_areas_get_ring(area, ring_id);
    g_return_if_fail(node_id >= 0 && node_id < ring->len);

    gv_data_changing(GV_DATA(areas), &change_info);
    
    g_array_index(ring, GvVertex, node_id).x = vertex->x;
    g_array_index(ring, GvVertex, node_id).y = vertex->y;

    gv_data_changed(GV_DATA(areas), &change_info);
}

void
gv_areas_insert_nodes(GvAreas *areas, gint area_id, gint ring_id,
		      gint node_id, gint num_nodes, GvVertex *vertex)
{
    GvArea *area;
    GArray *ring;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &area_id;

    g_return_if_fail(area_id >= 0 && area_id < areas->areas->len);
    area = gv_areas_get_area(areas, area_id);
    g_return_if_fail(ring_id >= 0 && ring_id < area->rings->len);
    ring = gv_areas_get_ring(area, ring_id);
    g_return_if_fail(node_id >= 0 && node_id < ring->len);

    gv_data_changing(GV_DATA(areas), &change_info);
    
    g_array_insert_vals(ring, node_id, vertex, num_nodes);

    gv_data_changed(GV_DATA(areas), &change_info);
}

void
gv_areas_delete_nodes(GvAreas *areas, gint area_id, gint ring_id,
		      gint num_nodes, gint *node_id)
{
    GvArea *area;
    GArray *ring;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &area_id;

    g_return_if_fail(area_id >= 0 && area_id < areas->areas->len);
    area = gv_areas_get_area(areas, area_id);
    g_return_if_fail(ring_id >= 0 && ring_id < area->rings->len);
    ring = gv_areas_get_ring(area, ring_id);

    gv_data_changing(GV_DATA(areas), &change_info);
    
    if (num_nodes == 1)
    {
	/* Need to preserve node order, so we can't use *_remove_fast */
	g_array_remove_index(ring, *node_id);
    }
    else
    {
	/* Strategy: sort the node_id buffer and delete nodes in desending
	   order, so that indicies remain valid */
	gint *id, i;

	id = g_memdup_type(node_id, gint, num_nodes);
	g_sort_type(id, gint, num_nodes);

	for (i=num_nodes-1; i >= 0; --i)
	{
	    g_array_remove_index(ring, id[i]);
	}
	g_free(id);
    }
    gv_data_changed(GV_DATA(areas), &change_info);
}

gint
gv_areas_tessellate_areas(GvAreas *areas, gint num_areas, gint *area_id)
{
    gint i, hit, ok;
    GvArea *area;

    hit = FALSE;
    ok = TRUE;

    for (i=0; i < num_areas; ++i)
    {
	area = gv_areas_get_area(areas, area_id[i]);
	if (area->fill_objects == 0)
	{
	    if (!gv_area_tessellate(area)) ok = FALSE;
	    hit = TRUE;	    
	}
    }

    if (hit)
    {
	/* FIXME: need a change_info param which describes a
	   tesselation change.  For now use NULL */
	gv_data_changed(GV_DATA(areas), NULL);
    }

    return ok;
}

void
gv_areas_clear_fill(GvAreas *areas, gint area_id)
{
    GvArea *area;

    g_return_if_fail(area_id >= 0 && area_id < areas->areas->len);
    area = gv_areas_get_area(areas, area_id);
    area->fill_objects = 0;
    if (area->mode_offset) g_array_set_size(area->mode_offset, 0);
    if (area->fill) g_array_set_size(area->fill, 0);
}

void
gv_areas_get_extents(GvAreas *areas, GvRect *rect)
{
    if (!areas->extents_valid)
    {
	gint i, j, num_areas;
	GvArea *area;
	GArray *ring = NULL;
	GvVertex vmax, vmin, *v;

	vmin.x = vmin.y = GV_MAXFLOAT;
	vmax.x = vmax.y = -GV_MAXFLOAT;
	num_areas = gv_areas_num_areas(areas);
	for (i=0; i < num_areas; ++i)
	{
	    area = gv_areas_get_area(areas, i);
	    ring = gv_areas_get_ring(area, 0);
	    for (j = 0; j < ring->len; ++j)
	    {
		v = &g_array_index(ring, GvVertex, j);
		if (v->x < vmin.x) vmin.x = v->x;
		if (v->x > vmax.x) vmax.x = v->x;
		if (v->y < vmin.y) vmin.y = v->y;
		if (v->y > vmax.y) vmax.y = v->y;
	    }
	}
	if (num_areas == 0 || (num_areas == 1 && ring->len == 0))
	{
	    areas->extents.x = 0;
	    areas->extents.y = 0;
	    areas->extents.width = 0;
	    areas->extents.height = 0;
	}
	else
	{
	    areas->extents.x = vmin.x;
	    areas->extents.y = vmin.y;
	    areas->extents.width = vmax.x - vmin.x;
	    areas->extents.height = vmax.y - vmin.y;
	}	    
	areas->extents_valid = TRUE;
    }
    rect->x = areas->extents.x;
    rect->y = areas->extents.y;
    rect->width = areas->extents.width;
    rect->height = areas->extents.height;
}

/***************************************************/

GvArea *
gv_area_new(gint alloc_outer_ring)
{
    GvArea *area;    

    area = g_new(GvArea, 1);
    g_return_val_if_fail(area, NULL);
    
    area->rings = g_ptr_array_new();
    g_return_val_if_fail(area->rings, NULL);

    if (alloc_outer_ring)
    {
	/* New area is created with ring 0 (outer ring) allocated, but empty */
	GArray *ring = g_array_new(FALSE, FALSE, sizeof(GvVertex));
	g_return_val_if_fail(ring, NULL);
	g_ptr_array_add(area->rings, ring);
    }
    
    area->fill_objects = -1;
    area->mode_offset = NULL;
    area->fill = NULL;

    return area;
}    

GvArea *
gv_area_copy(GvArea *area)
{
    GvArea *copyarea;
    GArray *ring, *copyring;
    gint rings, i;

    copyarea = g_new(GvArea, 1);
    g_return_val_if_fail(copyarea, NULL);
    
    copyarea->rings = g_ptr_array_new();
    g_return_val_if_fail(copyarea->rings, NULL);

    rings = gv_areas_num_rings(area);
    for (i=0; i < rings; ++i)
    {
	copyring = g_array_new(FALSE, FALSE, sizeof(GvVertex));
	g_ptr_array_add(copyarea->rings, copyring);

	ring = gv_areas_get_ring(area, i);
	g_array_append_vals(copyring, ring->data, ring->len);
    }

    /* Don't bother to copy tessellation info, as this can be regenerated */
    copyarea->fill_objects = 0;
    copyarea->mode_offset = NULL;
    copyarea->fill = NULL;

    return copyarea;
}

void
gv_area_delete(GvArea *area)
{
    int i;

    for (i=0; i < area->rings->len; ++i)
    {
	g_array_free(gv_areas_get_ring(area, i), TRUE);
    }
    g_ptr_array_free(area->rings, TRUE);

    if (area->fill)
    {
	g_array_free(area->fill, TRUE);
    }

    if (area->mode_offset)
    {
	g_array_free(area->mode_offset, TRUE);
    }

    g_free(area);
}

/***************************************************/

static void
gv_areas_replace_areas(GvAreas *areas, gint num_areas, gint *area_id,
		       GvArea **area)
{
    int i;
    
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};

    change_info.num_shapes = num_areas;
    change_info.shape_id = area_id;
    
    gv_data_changing(GV_DATA(areas), &change_info);

    for (i=0; i < num_areas; ++i)
    {
	gv_area_delete(gv_areas_get_area(areas, area_id[i]));
        g_ptr_array_index(areas->areas, area_id[i]) = area[i];
    }
    
    gv_data_changed(GV_DATA(areas), &change_info);    
}

static void
gv_areas_insert_areas(GvAreas *areas, gint num_areas, gint *area_ids,
		      GvArea **area)
{
    /* The area_id array must be in ascending order! */
    gint i;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 0, NULL};

    change_info.num_shapes = num_areas;
    change_info.shape_id = area_ids;

    gv_data_changing(GV_DATA(areas), &change_info);
    
    for (i=0; i < num_areas; ++i)
    {
	g_ptr_array_insert_fast(areas->areas, area_ids[i], area[i]);
    }

    gv_data_changed(GV_DATA(areas), &change_info);    
}

static void
gv_areas_get_memento(GvData *data, gpointer change_info,
		     GvDataMemento **memento)
{
    GvAreas *areas = GV_AREAS(data);
    GvShapeChangeInfo *info = (GvShapeChangeInfo *) change_info;
    GvAreasMemento *mem;
    int i;

    mem = g_new(GvAreasMemento, 1);
    mem->base.data = data;
    mem->base.type = info->change_type;

    mem->area_ids = g_array_new(FALSE, FALSE, sizeof(gint));
    g_array_append_vals(mem->area_ids, info->shape_id,	info->num_shapes);
    
    /* Grab areas in ascending order */
    if (info->num_shapes > 1)
    {
	g_sort_type(mem->area_ids->data, gint, mem->area_ids->len);
    }

    if (info->change_type == GV_CHANGE_ADD)
    {
	mem->areas = NULL;
    }
    else
    {
	mem->areas = g_ptr_array_new();
	for (i=0; i < info->num_shapes; ++i)
	{
	    GvArea *area = gv_area_copy(gv_areas_get_area(areas,
							  info->shape_id[i]));
	    g_ptr_array_add(mem->areas, area);
	}
    }
    
    *memento = (GvDataMemento*)mem;
}

static void
gv_areas_set_memento(GvData *data, GvDataMemento *data_memento)
{
    GvAreasMemento *memento = (GvAreasMemento *) data_memento;
    GvAreas *areas = GV_AREAS(data);

    switch (memento->base.type)
    {
	case GV_CHANGE_ADD:
	    gv_areas_delete_areas(areas, memento->area_ids->len,
				  (gint*)memento->area_ids->data);
	    break;

	case GV_CHANGE_REPLACE:
	    gv_areas_replace_areas(areas, memento->area_ids->len,
				  (gint*)memento->area_ids->data,
				  (GvArea**)memento->areas->pdata);
	    break;
		
	case GV_CHANGE_DELETE:
	    gv_areas_insert_areas(areas, memento->area_ids->len,
				  (gint*)memento->area_ids->data,
				  (GvArea**)memento->areas->pdata);
	    break;
    }

    if (memento->areas)
    {
	g_ptr_array_free(memento->areas, TRUE);
	memento->areas = NULL;
    }
    gv_areas_del_memento(GV_DATA(areas), (GvDataMemento *) memento);
}

static void
gv_areas_del_memento(GvData *data, GvDataMemento *data_memento)
{
    GvAreasMemento *memento = (GvAreasMemento *) data_memento;
    int i;
    
    if (memento->areas)
    {
	for (i=0; i < memento->areas->len; ++i)
	{
	    gv_area_delete(g_ptr_array_index(memento->areas, i));
	}
	g_ptr_array_free(memento->areas, TRUE);
    }
    g_array_free(memento->area_ids, TRUE);
    g_free(memento);    
}

static void
gv_areas_changed(GvData *data, gpointer change_info)
{
    GvAreas *areas = GV_AREAS(data);
    GvShapeChangeInfo *info = (GvShapeChangeInfo *) change_info;

    /* NULL info indicates change due to tesselation: ignore */
    if (info && info->change_type == GV_CHANGE_REPLACE)
    {
	/* Retesselate changed areas if necessary */
	gv_areas_tessellate_areas(areas, info->num_shapes, info->shape_id);
    }
    areas->extents_valid = FALSE;
}

static void
gv_areas_finalize(GObject *gobject)
{
    GvDataClass *parent_class;
    GvAreas *areas;
    int i;

    /* GTK2 PORT... Override GObject finalize, test for and set NULLs
       as finalize may be called more than once. */

    areas = GV_AREAS(gobject);
    
    if (areas->areas != NULL) {
      for (i=0; i < gv_areas_num_areas(areas); i++) {
	gv_area_delete(gv_areas_get_area(areas, i));
      }
      g_ptr_array_free(areas->areas, TRUE);
      areas->areas = NULL;
    }

    /* Call parent class function */
    parent_class = gtk_type_class(gv_data_get_type());
    G_OBJECT_CLASS(parent_class)->finalize(gobject);

    /* Call parent class function
    parent_class = gtk_type_class(gv_data_get_type());
    GTK_OBJECT_CLASS(parent_class)->finalize(gobject); 
    */
}

