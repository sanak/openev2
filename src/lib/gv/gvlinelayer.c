/******************************************************************************
 * $Id: gvlinelayer.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Display layer for GvLines.
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
 * $Log: gvlinelayer.c,v $
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
 * Revision 1.12  2002/11/05 18:56:24  sduclos
 * fix gcc warning
 *
 * Revision 1.11  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.10  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvlinelayer.h"
#include <gtk/gtksignal.h>
#include <GL/gl.h>

static void gv_line_layer_class_init(GvLineLayerClass *klass);
static void gv_line_layer_init(GvLineLayer *layer);
static void gv_line_layer_draw(GvLineLayer *layer, GvViewArea *view);
static void gv_line_layer_extents(GvLineLayer *layer, GvRect *rect);
static void gv_line_layer_data_change(GvLineLayer *layer, gpointer change_info);
static void gv_line_layer_draw_selected(GvLineLayer *layer, GvViewArea *view);
static void gv_line_layer_delete_selected(GvLineLayer *layer);
static void gv_line_layer_translate_selected(GvLineLayer *layer, GvVertex *delta);
static void gv_line_layer_pick_shape(GvLineLayer *layer);
static void gv_line_layer_pick_node(GvLineLayer *layer);
static void gv_line_layer_get_node(GvLineLayer *layer, GvNodeInfo *info);
static void gv_line_layer_move_node(GvLineLayer *layer, GvNodeInfo *info);
static void gv_line_layer_insert_node(GvLineLayer *layer, GvNodeInfo *info);
static void gv_line_layer_delete_node(GvLineLayer *layer, GvNodeInfo *info);

GtkType
gv_line_layer_get_type(void)
{
    static GtkType line_layer_type = 0;

    if (!line_layer_type)
    {
	static const GtkTypeInfo line_layer_info =
	{
	    "GvLineLayer",
	    sizeof(GvLineLayer),
	    sizeof(GvLineLayerClass),
	    (GtkClassInitFunc) gv_line_layer_class_init,
	    (GtkObjectInitFunc) gv_line_layer_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	line_layer_type = gtk_type_unique(gv_shape_layer_get_type(),
					  &line_layer_info);
    }
    return line_layer_type;
}

static void
gv_line_layer_class_init(GvLineLayerClass *klass)
{
    typedef void (*f)();
    GvDataClass *data_class;
    GvLayerClass *layer_class;
    GvShapeLayerClass *shape_layer_class;

    data_class = (GvDataClass*) klass;
    layer_class = (GvLayerClass*) klass;
    shape_layer_class = (GvShapeLayerClass*) klass;

    data_class->changed = (f) gv_line_layer_data_change;
    
    layer_class->draw            = (f) gv_line_layer_draw;
    layer_class->extents_request = (f) gv_line_layer_extents;

    shape_layer_class->draw_selected      = (f) gv_line_layer_draw_selected;
    shape_layer_class->delete_selected    = (f) gv_line_layer_delete_selected;
    shape_layer_class->translate_selected = (f) gv_line_layer_translate_selected;
    shape_layer_class->pick_shape         = (f) gv_line_layer_pick_shape;
    shape_layer_class->pick_node          = (f) gv_line_layer_pick_node;
    shape_layer_class->get_node           = (f) gv_line_layer_get_node;
    shape_layer_class->move_node          = (f) gv_line_layer_move_node;
    shape_layer_class->insert_node        = (f) gv_line_layer_insert_node;
    shape_layer_class->delete_node        = (f) gv_line_layer_delete_node;
}

static void
gv_line_layer_init(GvLineLayer *layer)
{
    GvColor default_line_color = {1.0, 1.0, 0.0, 1.0};  /* yellow */
    
    layer->data = NULL;
    gv_color_copy(GV_SHAPE_LAYER(layer)->color, default_line_color);


}

GtkObject *
gv_line_layer_new(GvPolylines *data)
{
    GvLineLayer *layer = GV_LINE_LAYER(gtk_type_new(gv_line_layer_get_type()));
    
    if (data)
    {
	layer->data = data;
    }
    else
    {
	layer->data = GV_POLYLINES(gv_polylines_new());
    }

    /* Set the number of shapes - case where data exists before layer was created */
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
				  gv_polylines_num_lines(layer->data));

    gv_data_set_parent(GV_DATA(layer), GV_DATA(layer->data));
    
    return GTK_OBJECT(layer);
}

gint
gv_line_layer_select_new_line(GvLineLayer *layer)
{
    gint line_id;
    
    line_id = gv_polylines_new_line(layer->data);
    
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
				  gv_polylines_num_lines(layer->data));
    gv_shape_layer_select_shape(GV_SHAPE_LAYER(layer), line_id);

    return line_id;
}

/*******************************************************/

static void
gv_line_layer_draw(GvLineLayer *layer, GvViewArea *view)
{
    gint i, lines;
    GArray *line;
    gint *selected, presentation;
    gint hit_selected = FALSE;

    presentation = GV_LAYER(layer)->presentation;
    selected = GV_SHAPE_LAYER_SELBUF(layer);
    lines = gv_polylines_num_lines(layer->data);
    
    glColor4fv(GV_SHAPE_LAYER(layer)->color);
    glEnableClientState(GL_VERTEX_ARRAY);
    for (i=0; i < lines; ++i)
    {
	if (selected[i] && !presentation)
	{
	    hit_selected = 1;
	    continue;
	}

	line = gv_polylines_get_line(layer->data, i);

	glVertexPointer(2, GL_GEOCOORD, 0, line->data);
	glDrawArrays(GL_LINE_STRIP, 0, line->len);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    
    if (hit_selected && ! GV_SHAPE_LAYER(layer)->flags & GV_DELAY_SELECTED)
    {
	gv_line_layer_draw_selected(layer, view);
    }    
}

static void
gv_line_layer_draw_selected(GvLineLayer *layer, GvViewArea *view)
{
    gint i, lines;
    GArray *line;
    gint *selected;

    selected = GV_SHAPE_LAYER_SELBUF(layer);
    lines = gv_polylines_num_lines(layer->data);

    glColor4fv(GV_SHAPE_LAYER(layer)->color);
    glPointSize(3.0);
    glEnableClientState(GL_VERTEX_ARRAY);
    for (i=0; i < lines; ++i)
    {
	if (selected[i])
	{
	    line = gv_polylines_get_line(layer->data, i);
	    
	    glVertexPointer(2, GL_GEOCOORD, 0, line->data);
	    glDrawArrays(GL_LINE_STRIP, 0, line->len);
	    
#ifdef HAVE_BROKEN_GL_POINTS
	    {
		int j;
		for (j=0; j < line->len; ++j)
		{
		    glBegin(GL_POINTS);
		    glArrayElement(j);
		    glEnd();
		}
	    }
#else	    
	    glDrawArrays(GL_POINTS, 0, line->len);
#endif /* HAVE_BROKEN_GL_POINTS */	    
	}
    }
    glDisableClientState(GL_VERTEX_ARRAY); 
}

static void
gv_line_layer_pick_shape(GvLineLayer *layer)
{
    gint i, lines;
    GArray *line;

    if (!gv_layer_is_visible(GV_LAYER(layer))) return;

    lines = gv_polylines_num_lines(layer->data);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    for (i=0; i < lines; ++i)
    {
	line = gv_polylines_get_line(layer->data, i);

	glLoadName(i);
	glVertexPointer(2, GL_GEOCOORD, 0, line->data);
	glDrawArrays(GL_LINE_STRIP, 0, line->len);
    }
    glDisableClientState(GL_VERTEX_ARRAY);     
}

static void
gv_line_layer_pick_node(GvLineLayer *layer)
{
    GArray *line;
    gint sel, i;
    
    if (!gv_layer_is_visible(GV_LAYER(layer))) return;

    if (!gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_FIRST, &sel))
    {
	return;
    }
    line = gv_polylines_get_line(layer->data, sel);
    
    glEnableClientState(GL_VERTEX_ARRAY);     
    glVertexPointer(2, GL_GEOCOORD, 0, line->data);

    /* Nodes first */
    glLoadName(0);
    glPushName(0); /* Ring id is always zero */
    glPushName(-1);
    glPointSize(1.0);
    for (i=0; i < line->len; ++i)
    {
	glLoadName(i);
	glBegin(GL_POINTS);
	glArrayElement(i);
	glEnd();
    }
    glPopName(); /* node id */
    glPopName(); /* ring id */
    
    /* Segments next */
    glLoadName(1);
    glPushName(0); /* Ring id is always zero */
    glPushName(-1);
    for (i=1; i < line->len; ++i)
    {
	glLoadName(i);
	glBegin(GL_LINES);
	glArrayElement(i-1);
	glArrayElement(i);
	glEnd();
    }
    glPopName(); /* node id */
    glPopName(); /* ring id */
    glDisableClientState(GL_VERTEX_ARRAY);
}

static void
gv_line_layer_delete_selected(GvLineLayer *layer)
{
    GArray *sel;

    sel = g_array_new(FALSE, FALSE, sizeof(gint));
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_ALL, sel))
    {
	/* This will force a selection clear */
	gv_polylines_delete_lines(layer->data, sel->len, (gint*)sel->data);
    }
    g_array_free(sel, TRUE);
}

static void
gv_line_layer_translate_selected(GvLineLayer *layer, GvVertex *delta)
{
    GArray *sel;

    sel = g_array_new(FALSE, FALSE, sizeof(gint));
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_ALL, sel))
    {
	gv_polylines_translate_lines(layer->data, sel->len, (gint*)sel->data,
				     delta->x, delta->y);
    }
    g_array_free(sel, TRUE);
}

static void
gv_line_layer_get_node(GvLineLayer *layer, GvNodeInfo *info)
{
    info->vertex = gv_polylines_get_node(layer->data, info->shape_id,
					 info->node_id);
}

static void
gv_line_layer_move_node(GvLineLayer *layer, GvNodeInfo *info)
{
    gv_polylines_move_node(layer->data, info->shape_id, info->node_id,
			   info->vertex);
}

static void
gv_line_layer_insert_node(GvLineLayer *layer, GvNodeInfo *info)
{
    gv_polylines_insert_nodes(layer->data, info->shape_id, info->node_id,
			      1, info->vertex);
}

static void
gv_line_layer_delete_node(GvLineLayer *layer, GvNodeInfo *info)
{
    gv_polylines_delete_nodes(layer->data, info->shape_id, 1, &info->node_id);
}

static void
gv_line_layer_extents(GvLineLayer *layer, GvRect *rect)
{
    gv_polylines_get_extents(layer->data, rect);
}

static void
gv_line_layer_data_change(GvLineLayer *layer, gpointer change_info)
{
    /* Reset the selected array to reflect the data length */
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
				  gv_polylines_num_lines(layer->data));
}

