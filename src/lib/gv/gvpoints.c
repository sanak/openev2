/******************************************************************************
 * $Id: gvpoints.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Points data container (superceed by GvShapes)
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
 * $Log: gvpoints.c,v $
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
 * Revision 1.7  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.6  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gextra.h"
#include "gvpoints.h"

typedef struct _GvPointsMemento GvPointsMemento;

struct _GvPointsMemento
{
    GvDataMemento base;
    GArray *point_ids;
    GArray *points;
};

static void gv_points_class_init(GvPointsClass *klass);
static void gv_points_init(GvPoints *points);
static void gv_points_get_memento(GvData *points, gpointer info, GvDataMemento **memento);
static void gv_points_set_memento(GvData *points, GvDataMemento *memento);
static void gv_points_del_memento(GvData *points, GvDataMemento *memento);
static void gv_points_changed(GvData *points, gpointer data);
static void gv_points_finalize(GObject *gobject);

GtkType
gv_points_get_type(void)
{
    static GtkType points_type = 0;

    if (!points_type)
    {
	static const GtkTypeInfo points_info =
	{
	    "GvPoints",
	    sizeof(GvPoints),
	    sizeof(GvPointsClass),
	    (GtkClassInitFunc) gv_points_class_init,
	    (GtkObjectInitFunc) gv_points_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	points_type = gtk_type_unique(gv_data_get_type(), &points_info);
    }
    return points_type;
}

static void
gv_points_init(GvPoints *points)
{
    points->points = g_array_new(FALSE, FALSE, sizeof(GvPoint));
    points->extents_valid = FALSE;
}

static void
gv_points_class_init(GvPointsClass *klass)
{
    GvDataClass *data_class;

    data_class = (GvDataClass*) klass;

    /* ---- Override finalize ---- */
    (G_OBJECT_CLASS(klass))->finalize = gv_points_finalize;

    /* GTK2 PORT...
    object_class->finalize = gv_points_finalize;
    */

    data_class->changed = gv_points_changed;
    data_class->get_memento = gv_points_get_memento;
    data_class->set_memento = gv_points_set_memento;
    data_class->del_memento = gv_points_del_memento;
}

GvData *
gv_points_new(void)
{
    return GV_DATA(gtk_type_new(gv_points_get_type()));
}

gint
gv_points_new_point(GvPoints *points, GvVertex *vertex)
{    
    int point_id;
    GvPoint point;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 1, NULL};

    change_info.shape_id = &point_id;

    if (vertex)
    {
	point.v = *vertex;
    }
    else
    {
	point.v.x = point.v.y = 0.0;
    }
    point.meta = NULL;
    point_id = points->points->len;

    gv_data_changing(GV_DATA(points), &change_info);
    
    g_array_append_val(points->points, point);

    gv_data_changed(GV_DATA(points), &change_info);
    
    return point_id;
}    

void
gv_points_delete_points(GvPoints *points, gint num_points, gint *point_id)
{
    GvShapeChangeInfo change_info = {GV_CHANGE_DELETE, 0, NULL};

    change_info.num_shapes = num_points;
    change_info.shape_id = point_id;

    gv_data_changing(GV_DATA(points), &change_info);

    /* FIXME: can't use g_array_remove_index_fast() here since there is no
       g_array_insert_index_fast() function. */
    
    if (num_points == 1)
    {
	g_array_remove_index(points->points, *point_id);
    }
    else
    {
	/* Strategy: sort the line_id buffer and delete lines in desending
	   order, so that indicies remain valid */
	gint *id, i;

	id = g_memdup_type(point_id, int, num_points);
	g_sort_type(id, gint, num_points);

	for (i=num_points-1; i >= 0; --i)
	{
	    g_array_remove_index(points->points, id[i]);
	}
	g_free(id);
    }
    gv_data_changed(GV_DATA(points), &change_info);
}

void
gv_points_translate_points(GvPoints *points, gint num_points, gint *point_id,
			   gvgeocoord dx, gvgeocoord dy)
{
    GvPoint *point;
    gint i;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};

    change_info.num_shapes = num_points;
    change_info.shape_id = point_id;

    gv_data_changing(GV_DATA(points), &change_info);

    for (i=0; i < num_points; ++i)
    {
	g_return_if_fail(point_id[i] >= 0 &&
			 point_id[i] < points->points->len);
	point = gv_points_get_point(points, point_id[i]);

	point->v.x += dx;
	point->v.y += dy;
    }
    gv_data_changed(GV_DATA(points), &change_info);
}

void
gv_points_set_point(GvPoints *points, gint point_id, GvVertex *vertex)
{
    GvPoint *point;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &point_id;

    g_return_if_fail(vertex);
    g_return_if_fail(point_id >= 0 && point_id < points->points->len);

    gv_data_changing(GV_DATA(points), &change_info);

    point = gv_points_get_point(points, point_id);
    point->v.x = vertex->x;
    point->v.y = vertex->y;

    gv_data_changed(GV_DATA(points), &change_info);
}

void
gv_points_get_extents(GvPoints *points, GvRect *rect)
{
    if (!points->extents_valid)
    {
	gint i, num_points;
	GvVertex vmax, vmin, v;

	vmin.x = vmin.y = GV_MAXFLOAT;
	vmax.x = vmax.y = -GV_MAXFLOAT;
	num_points = gv_points_num_points(points);
	for (i=0; i < num_points; ++i)
	{
	    v = gv_points_get_point(points, i)->v;
	    if (v.x < vmin.x) vmin.x = v.x;
	    if (v.x > vmax.x) vmax.x = v.x;
	    if (v.y < vmin.y) vmin.y = v.y;
	    if (v.y > vmax.y) vmax.y = v.y;
	}

	if (num_points == 0)
	{
	    points->extents.x = 0;
	    points->extents.y = 0;
	    points->extents.width = 0;
	    points->extents.height = 0;
	}
	else
	{
	    points->extents.x = vmin.x;
	    points->extents.y = vmin.y;
	    points->extents.width = vmax.x - vmin.x;
	    points->extents.height = vmax.y - vmin.y;
	}
	points->extents_valid = TRUE;
    }

    *rect = points->extents;
}

/*********************************************/

static void
gv_points_replace_points(GvPoints *points, gint num_points, gint *point_id,
			 GvPoint *pts)
{
    gint i;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};

    change_info.num_shapes = num_points;
    change_info.shape_id = point_id;

    gv_data_changing(GV_DATA(points), &change_info);

    for (i=0; i < num_points; ++i)
    {
	g_array_index(points->points, GvPoint, point_id[i]) = pts[i];
    }

    gv_data_changed(GV_DATA(points), &change_info);
}

static void
gv_points_insert_points(GvPoints *points, gint num_points, gint *point_id,
			GvPoint *pts)
{
    /* The point_id array must be in ascending order! */
    gint i;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 0, NULL};

    change_info.num_shapes = num_points;
    change_info.shape_id = point_id;

    gv_data_changing(GV_DATA(points), &change_info);

    for (i=0; i < num_points; ++i)
    {
	/* FIXME: need to write a g_array_insert_index_fast() function,
	   but that requires access to the elt_size private member of
	   GArray */
	g_array_insert_val(points->points, point_id[i], pts[i]);
    }

    gv_data_changed(GV_DATA(points), &change_info);    
}

static void
gv_points_get_memento(GvData *gv_data, gpointer data,
		      GvDataMemento **memento)
{
    GvPoints	*points = GV_POINTS(gv_data);
    GvPointsMemento *mem;
    GvShapeChangeInfo *info = (GvShapeChangeInfo *) data;
    int i;

    mem = g_new(GvPointsMemento, 1);
    mem->base.data = GV_DATA(points);
    mem->base.type = info->change_type;

    mem->point_ids = g_array_new(FALSE, FALSE, sizeof(gint));
    g_array_append_vals(mem->point_ids, info->shape_id, info->num_shapes);

    /* Grab points in ascending order */
    if (info->num_shapes > 1)
    {
	g_sort_type(mem->point_ids->data, gint, mem->point_ids->len);
    }

    if (info->change_type == GV_CHANGE_ADD)
    {
	mem->points = NULL;
    }
    else
    {
	mem->points = g_array_new(FALSE, FALSE, sizeof(GvPoint));
	for (i=0; i < info->num_shapes; ++i)
	{
	    GvPoint *point = gv_points_get_point(points, info->shape_id[i]);
	    g_array_append_val(mem->points, *point);
	}
    }

    *memento = (GvDataMemento*)mem;
}

static void
gv_points_set_memento(GvData *gv_data, GvDataMemento *data_memento)
{
    GvPoints	*points = GV_POINTS(gv_data);
    GvPointsMemento *memento = (GvPointsMemento *) data_memento;

    switch (memento->base.type)
    {
	case GV_CHANGE_ADD:
	    gv_points_delete_points(points, memento->point_ids->len,
				    (gint*)memento->point_ids->data);
	    break;

	case GV_CHANGE_REPLACE:
	    gv_points_replace_points(points, memento->point_ids->len,
				     (gint*)memento->point_ids->data,
				     (GvPoint*)memento->points->data);
	    break;

	case GV_CHANGE_DELETE:
	    gv_points_insert_points(points, memento->point_ids->len,
				     (gint*)memento->point_ids->data,
				     (GvPoint*)memento->points->data);
	    break;
    }

    gv_points_del_memento((GvData *) points, (GvDataMemento *) memento);
}

static void
gv_points_del_memento(GvData *gv_data, GvDataMemento *data_memento)
{
    GvPointsMemento *memento = (GvPointsMemento *) data_memento;

    if (memento->points)
    {
	g_array_free(memento->points, TRUE);
    }
    g_array_free(memento->point_ids, TRUE);
    g_free(memento);
}

static void
gv_points_changed(GvData *gv_data, gpointer data)
{
    GvPoints	*points = GV_POINTS(gv_data);

    points->extents_valid = FALSE;
}

static void
gv_points_finalize(GObject *gobject)
{
    GvDataClass *parent_class;
    GvPoints *points = GV_POINTS(gobject);

    /* GTK2 PORT... Override GObject finalize, test for and set NULLs
       as finalize may be called more than once. */

    /* FIXME - GTK2, Should this be in destroy? */ 
    if (points->points != NULL) {
      g_array_free(points->points, TRUE);
      points->points = NULL;
    }

    /* Call parent class function */
    parent_class = gtk_type_class(gv_data_get_type());
    G_OBJECT_CLASS(parent_class)->finalize(gobject);

    /*
    parent_class = gtk_type_class(gv_data_get_type());
    GTK_OBJECT_CLASS(parent_class)->finalize(object);
    */        
}
