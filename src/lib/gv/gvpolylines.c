/******************************************************************************
 * $Id: gvpolylines.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Polylines data container (superceeded by GvShapes)
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
 * $Log: gvpolylines.c,v $
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
 * Revision 1.10  2002/11/05 18:56:21  sduclos
 * fix gcc warning
 *
 * Revision 1.9  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.8  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gextra.h"
#include "gvpolylines.h"

typedef struct _GvLinesMemento GvLinesMemento;

struct _GvLinesMemento
{
    GvDataMemento base;
    GArray *line_ids;
    GPtrArray *lines;
};

static void gv_polylines_class_init(GvPolylinesClass *klass);
static void gv_polylines_init(GvPolylines *pline);
static void gv_polylines_replace_lines(GvPolylines *pline, gint num_lines, gint *line_ids, GArray **line);
static void gv_polylines_insert_lines(GvPolylines *pline, gint num_lines, gint *line_ids, GArray **line);
static void gv_polylines_get_memento(GvPolylines *pline, GvShapeChangeInfo *info, GvDataMemento **memento);
static void gv_polylines_set_memento(GvPolylines *pline, GvLinesMemento *memento);
static void gv_polylines_del_memento(GvPolylines *pline, GvLinesMemento *memento);
static void gv_polylines_changed(GvPolylines *pline, gpointer data);
static void gv_polylines_finalize(GObject *gobject);

GtkType
gv_polylines_get_type(void)
{
    static GtkType polylines_type = 0;

    if (!polylines_type)
    {
	static const GtkTypeInfo polylines_info =
	{
	    "GvPolylines",
	    sizeof(GvPolylines),
	    sizeof(GvPolylinesClass),
	    (GtkClassInitFunc) gv_polylines_class_init,
	    (GtkObjectInitFunc) gv_polylines_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	polylines_type = gtk_type_unique(gv_data_get_type(), &polylines_info);
    }
    return polylines_type;
}

static void
gv_polylines_init(GvPolylines *pline)
{
    pline->lines = g_ptr_array_new();
    pline->extents_valid = FALSE;
}

static void
gv_polylines_class_init(GvPolylinesClass *klass)
{
    typedef void (*f)();
    GvDataClass *data_class;

    /* ---- Override finalize ---- */
    (G_OBJECT_CLASS(klass))->finalize = gv_polylines_finalize;

    /* GTK2 PORT...
    object_class->finalize = gv_polylines_finalize;
    */

    data_class = (GvDataClass*) klass;

    data_class->changed     = (f) gv_polylines_changed;
    data_class->get_memento = (f) gv_polylines_get_memento;
    data_class->set_memento = (f) gv_polylines_set_memento;
    data_class->del_memento = (f) gv_polylines_del_memento;
}

GvData *
gv_polylines_new(void)
{
    return GV_DATA(gtk_type_new(gv_polylines_get_type()));
}

gint
gv_polylines_new_line(GvPolylines *pline)
{    
    GArray *line;
    int line_id;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 1, NULL};

    change_info.shape_id = &line_id;

    line_id = pline->lines->len;
    line = g_array_new(FALSE, FALSE, sizeof(GvVertex));
    g_return_val_if_fail(line, 0);

    gv_data_changing(GV_DATA(pline), &change_info);
    
    g_ptr_array_add(pline->lines, (gpointer)line);

    gv_data_changed(GV_DATA(pline), &change_info);
    
    return line_id;
}    

gint
gv_polylines_new_line_with_data(GvPolylines *pline, gint num_nodes,
			       GvVertex *vertex)
{
    GArray *line;
    gint line_id;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 1, NULL};

    change_info.shape_id = &line_id;

    line_id = pline->lines->len;
    line = g_array_new(FALSE, FALSE, sizeof(GvVertex));
    g_return_val_if_fail(line, 0);

    gv_data_changing(GV_DATA(pline), &change_info);
    
    g_ptr_array_add(pline->lines, (gpointer)line);
    g_array_append_vals(line, vertex, num_nodes);
    
    gv_data_changed(GV_DATA(pline), &change_info);
    
    return line_id;
}

gint
gv_polylines_num_nodes(GvPolylines *pline, gint line_id)
{
    GArray *line;

    g_return_val_if_fail(line_id >= 0 && line_id < pline->lines->len, 0);
    line = gv_polylines_get_line(pline, line_id);
    return line->len;
}

void
gv_polylines_translate_lines(GvPolylines *pline, gint num_lines, gint *line_id,
			     gvgeocoord dx, gvgeocoord dy)
{
    GArray *line;
    int i, j;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};

    change_info.shape_id = line_id;
    change_info.num_shapes = num_lines;

    gv_data_changing(GV_DATA(pline), &change_info);

    for (j=0; j < num_lines; ++j)
    {
	g_return_if_fail(line_id[j] >= 0 && line_id[j] < pline->lines->len);
	line = gv_polylines_get_line(pline, line_id[j]);

	for (i=0; i < line->len; ++i)
	{
	    g_array_index(line, GvVertex, i).x += dx;
	    g_array_index(line, GvVertex, i).y += dy;
	}
    }
    gv_data_changed(GV_DATA(pline), &change_info);
}

void
gv_polylines_delete_lines(GvPolylines *pline, gint num_lines, gint *line_id)
{
    GArray *line;
    GvShapeChangeInfo change_info = {GV_CHANGE_DELETE, 0, NULL};

    change_info.num_shapes = num_lines;
    change_info.shape_id = line_id;
    
    gv_data_changing(GV_DATA(pline), &change_info);
    
    if (num_lines == 1)
    {
	line = (GArray*)g_ptr_array_remove_index_fast(pline->lines, *line_id);
	if (line) g_array_free(line, TRUE);
    }
    else
    {
	/* Strategy: sort the line_id buffer and delete lines in desending
	   order, so that indicies remain valid */
	gint *id, i;

	id = g_memdup_type(line_id, gint, num_lines);
	g_sort_type(id, gint, num_lines);

	for (i=num_lines-1; i >= 0; --i)
	{
	    line = (GArray*)g_ptr_array_remove_index_fast(pline->lines, id[i]);
	    if (line) g_array_free(line, TRUE);
	}
	g_free(id);
    }
    gv_data_changed(GV_DATA(pline), &change_info);    
}

void
gv_polylines_set_nodes(GvPolylines *pline, gint line_id, gint num_nodes,
		       GvVertex *vertex)
{
    GArray *line;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &line_id;
    
    g_return_if_fail(line_id >= 0 && line_id < pline->lines->len);
    line = gv_polylines_get_line(pline, line_id);

    gv_data_changing(GV_DATA(pline), &change_info);
    
    g_array_set_size(line, 0);
    g_array_append_vals(line, vertex, num_nodes);

    gv_data_changed(GV_DATA(pline), &change_info);    
}

void
gv_polylines_append_nodes(GvPolylines *pline, gint line_id, gint num_nodes,
			  GvVertex *vertex)
{
    GArray *line;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &line_id;

    g_return_if_fail(line_id >= 0 && line_id < pline->lines->len);
    line = gv_polylines_get_line(pline, line_id);

    gv_data_changing(GV_DATA(pline), &change_info);
    
    g_array_append_vals(line, vertex, num_nodes);

    gv_data_changed(GV_DATA(pline), &change_info);        
}

void
gv_polylines_insert_nodes(GvPolylines *pline, gint line_id, gint node_id,
			  gint num_nodes, GvVertex *vertex)
{
    GArray *line;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &line_id;

    g_return_if_fail(line_id >= 0 && line_id < pline->lines->len);
    line = gv_polylines_get_line(pline, line_id);

    gv_data_changing(GV_DATA(pline), &change_info);
 
    g_array_insert_vals(line, node_id, vertex, num_nodes);

    gv_data_changed(GV_DATA(pline), &change_info);            
}

void
gv_polylines_delete_nodes(GvPolylines *pline, gint line_id, gint num_nodes,
			  gint *node_id)
{
    GArray *line;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &line_id;
    
    g_return_if_fail(line_id >= 0 && line_id < pline->lines->len);
    line = gv_polylines_get_line(pline, line_id);

    gv_data_changing(GV_DATA(pline), &change_info);
    
    if (num_nodes == 1)
    {
	/* Need to preserve node order, so we can't use *_remove_fast */
	g_array_remove_index(line, *node_id);
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
	    g_array_remove_index(line, id[i]);
	}
	g_free(id);
    }
    gv_data_changed(GV_DATA(pline), &change_info);        
}

void
gv_polylines_move_node(GvPolylines *pline, gint line_id, gint node_id,
		       GvVertex *vertex)
{
    GArray *line;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 1, NULL};

    change_info.shape_id = &line_id;

    g_return_if_fail(line_id >= 0 && line_id < pline->lines->len);
    line = gv_polylines_get_line(pline, line_id);
    g_return_if_fail(node_id >= 0 && node_id < line->len);

    gv_data_changing(GV_DATA(pline), &change_info);
    
    g_array_index(line, GvVertex, node_id).x = vertex->x;
    g_array_index(line, GvVertex, node_id).y = vertex->y;

    gv_data_changed(GV_DATA(pline), &change_info);    
}

GvVertex *
gv_polylines_get_node(GvPolylines *pline, gint line_id, gint node_id)
{
    GArray *line;

    g_return_val_if_fail(line_id >= 0 && line_id < pline->lines->len, NULL);
    line = gv_polylines_get_line(pline, line_id);
    g_return_val_if_fail(node_id >= 0 && node_id < line->len, NULL);

    return &g_array_index(line, GvVertex, node_id);
}

void
gv_polylines_get_extents(GvPolylines *pline, GvRect *rect)
{
    if (!pline->extents_valid)
    {
	gint i, j, lines;
	GArray *line;
	GvVertex vmax, vmin, *v;

	vmin.x = vmin.y = GV_MAXFLOAT;
	vmax.x = vmax.y = -GV_MAXFLOAT;
	lines = gv_polylines_num_lines(pline);
	for (i=0; i < lines; ++i)
	{
	    line = gv_polylines_get_line(pline, i);
	    for (j=0; j < line->len; ++j)
	    {
		v = &g_array_index(line, GvVertex, j);
		if (v->x < vmin.x) vmin.x = v->x;
		if (v->x > vmax.x) vmax.x = v->x;
		if (v->y < vmin.y) vmin.y = v->y;
		if (v->y > vmax.y) vmax.y = v->y;
	    }
	}

	if (lines == 0 || (lines == 1 && line->len == 0))
	{
	    pline->extents.x = 0;
	    pline->extents.y = 0;
	    pline->extents.width = 0;
	    pline->extents.height = 0;
	}
	else
	{
	    pline->extents.x = vmin.x;
	    pline->extents.y = vmin.y;
	    pline->extents.width = vmax.x - vmin.x;
	    pline->extents.height = vmax.y - vmin.y;
	}
	pline->extents_valid = TRUE;
    }
    rect->x = pline->extents.x;
    rect->y = pline->extents.y;
    rect->width = pline->extents.width;
    rect->height = pline->extents.height;
}

/*********************************************/

static void
gv_polylines_replace_lines(GvPolylines *pline, gint num_lines, gint *line_id,
                           GArray **line)
{
    gint i;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};

    change_info.num_shapes = num_lines;
    change_info.shape_id = line_id;

    gv_data_changing(GV_DATA(pline), &change_info);

    for (i=0; i < num_lines; ++i)
    {
        g_array_free(gv_polylines_get_line(pline, line_id[i]), TRUE);

        g_ptr_array_index(pline->lines, line_id[i]) = line[i];
    }

    gv_data_changed(GV_DATA(pline), &change_info);
}

static void
gv_polylines_insert_lines(GvPolylines *pline, gint num_lines, gint *line_ids,
			  GArray **line)
{
    /* The line_id array must be in ascending order! */
    gint i;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 0, NULL};

    change_info.num_shapes = num_lines;
    change_info.shape_id = line_ids;

    gv_data_changing(GV_DATA(pline), &change_info);

    for (i=0; i < num_lines; ++i)
    {
	g_ptr_array_insert_fast(pline->lines, line_ids[i], line[i]);
    }
    
    gv_data_changed(GV_DATA(pline), &change_info);
}

static void
gv_polylines_get_memento(GvPolylines *pline, GvShapeChangeInfo *info,
			 GvDataMemento **memento)
{
    GvLinesMemento *mem;
    int i;

    mem = g_new(GvLinesMemento, 1);
    mem->base.data = GV_DATA(pline);
    mem->base.type = info->change_type;

    mem->line_ids = g_array_new(FALSE, FALSE, sizeof(gint));
    g_array_append_vals(mem->line_ids, info->shape_id,	info->num_shapes);

    /* Grab areas in ascending order */
    if (info->num_shapes > 1)
    {
	g_sort_type(mem->line_ids->data, gint, mem->line_ids->len);
    }

    if (info->change_type == GV_CHANGE_ADD)
    {
	mem->lines = NULL;
    }
    else
    {
	mem->lines = g_ptr_array_new();
	for (i=0; i < info->num_shapes; ++i)
	{
	    GArray *line = gv_polylines_get_line(pline, info->shape_id[i]);
	    GArray *copy = g_array_new(FALSE, FALSE, sizeof(GvVertex));
	    g_array_append_vals(copy, line->data, line->len);
	    g_ptr_array_add(mem->lines, copy);
	}
    }
    
    *memento = (GvDataMemento*)mem;    
}

static void
gv_polylines_set_memento(GvPolylines *pline, GvLinesMemento *memento)
{
    switch (memento->base.type)
    {
	case GV_CHANGE_ADD:
	    gv_polylines_delete_lines(pline, memento->line_ids->len,
				      (gint*)memento->line_ids->data);
	    break;

	case GV_CHANGE_REPLACE:
	    gv_polylines_replace_lines(pline, memento->line_ids->len,
				       (gint*)memento->line_ids->data,
				       (GArray**)memento->lines->pdata);
	    break;
	    
	case GV_CHANGE_DELETE:
	    gv_polylines_insert_lines(pline, memento->line_ids->len,
				      (gint*)memento->line_ids->data,
				      (GArray**)memento->lines->pdata);
	    break;
    }

    if (memento->lines)
    {
	g_ptr_array_free(memento->lines, TRUE);
	memento->lines = NULL;
    }
    gv_polylines_del_memento(pline, memento);
}

static void
gv_polylines_del_memento(GvPolylines *pline, GvLinesMemento *memento)
{
    int i;

    if (memento->lines)
    {
	for (i=0; i < memento->lines->len; ++i)
	{
	    g_array_free(g_ptr_array_index(memento->lines, i), TRUE);
	}
	g_ptr_array_free(memento->lines, TRUE);
    }
    g_array_free(memento->line_ids, TRUE);
    g_free(memento);
}

static void
gv_polylines_changed(GvPolylines *pline, gpointer data)
{
    pline->extents_valid = FALSE;
}

static void
gv_polylines_finalize(GObject *gobject)
{
    GvDataClass *parent_class;
    GvPolylines *pline;
    int i;

    /* GTK2 PORT... Override GObject finalize, test for and set NULLs
       as finalize may be called more than once. */

    pline = GV_POLYLINES(gobject);

    for (i=0; i < gv_polylines_num_lines(pline); i++)
    {
	g_array_free(gv_polylines_get_line(pline, i), TRUE);
    }
    g_ptr_array_free(pline->lines, TRUE);
    
    /* Call parent class function */
    parent_class = gtk_type_class(gv_data_get_type());
    G_OBJECT_CLASS(parent_class)->finalize(gobject);

    /* Call parent class function 
    parent_class = gtk_type_class(gv_data_get_type());
    GTK_OBJECT_CLASS(parent_class)->finalize(object);
    */        
}
