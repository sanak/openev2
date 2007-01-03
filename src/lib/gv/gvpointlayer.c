/******************************************************************************
 * $Id: gvpointlayer.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Display layer for GvPoints.
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
 * $Log: gvpointlayer.c,v $
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
 * Revision 1.6  2002/11/05 18:56:24  sduclos
 * fix gcc warning
 *
 * Revision 1.5  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.4  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvpointlayer.h"
#include <GL/gl.h>

#define DEFAULT_POINT_SIZE 6

static void gv_point_layer_class_init(GvPointLayerClass *klass);
static void gv_point_layer_init(GvPointLayer *layer);
static void gv_point_layer_draw(GvPointLayer *layer, GvViewArea *view);
static void gv_point_layer_extents(GvPointLayer *layer, GvRect *rect);
static void gv_point_layer_data_change(GvPointLayer *layer, gpointer change_info);
static void gv_point_layer_draw_selected(GvPointLayer *layer, GvViewArea *view);
static void gv_point_layer_delete_selected(GvPointLayer *layer);
static void gv_point_layer_translate_selected(GvPointLayer *layer, GvVertex *delta);
static void gv_point_layer_pick_shape(GvShapeLayer *layer);

GtkType
gv_point_layer_get_type(void)
{
    static GtkType point_layer_type = 0;

    if (!point_layer_type)
    {
	static const GtkTypeInfo point_layer_info =
	{
	    "GvPointLayer",
	    sizeof(GvPointLayer),
	    sizeof(GvPointLayerClass),
	    (GtkClassInitFunc) gv_point_layer_class_init,
	    (GtkObjectInitFunc) gv_point_layer_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	point_layer_type = gtk_type_unique(gv_shape_layer_get_type(),
					   &point_layer_info);
    }
    return point_layer_type;
}

static void
gv_point_layer_class_init(GvPointLayerClass *klass)
{
    typedef void (*f)();
    GvDataClass *data_class;
    GvLayerClass *layer_class;
    GvShapeLayerClass *shape_layer_class;

    data_class = (GvDataClass*) klass;
    layer_class = (GvLayerClass*) klass;
    shape_layer_class = (GvShapeLayerClass*) klass;

    data_class->changed = (f) gv_point_layer_data_change;
    
    layer_class->draw            = (f) gv_point_layer_draw;
    layer_class->extents_request = (f) gv_point_layer_extents;

    shape_layer_class->draw_selected      = (f) gv_point_layer_draw_selected;
    shape_layer_class->delete_selected    = (f) gv_point_layer_delete_selected;
    shape_layer_class->translate_selected = (f) gv_point_layer_translate_selected;
    shape_layer_class->pick_shape         = gv_point_layer_pick_shape;
}

static void
gv_point_layer_init(GvPointLayer *layer)
{
    GvColor default_point_color = {0.0, 1.0, 1.0, 1.0};  /* cyan */
    
    layer->data = NULL;
    layer->point_size = DEFAULT_POINT_SIZE;
    gv_color_copy(GV_SHAPE_LAYER(layer)->color, default_point_color);
}

GtkObject *
gv_point_layer_new(GvPoints *data)
{
    GvPointLayer *layer = GV_POINT_LAYER(gtk_type_new(
	gv_point_layer_get_type()));
    
    if (data)
    {
	layer->data = data;
    }
    else
    {
	layer->data = GV_POINTS(gv_points_new());
    }

    /* Set the selected array to reflect the data length if points already exist */
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
				  gv_points_num_points(layer->data));

    gv_data_set_parent(GV_DATA(layer), GV_DATA(layer->data));
    
    return GTK_OBJECT(layer);
}

gint
gv_point_layer_select_new_point(GvPointLayer *layer, GvVertex *vertex)
{
    gint point_id;

    point_id = gv_points_new_point(layer->data, vertex);

    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
				  gv_points_num_points(layer->data));
    gv_shape_layer_select_shape(GV_SHAPE_LAYER(layer), point_id);

    return point_id;
}

/*******************************************************/

static void
gv_point_layer_draw(GvPointLayer *layer, GvViewArea *view)
{
    gint i, points;
    GvPoint *point;
    gint *selected, presentation;
    gvgeocoord dx, dy, x, y;
    gint hit_selected = FALSE;

    presentation = GV_LAYER(layer)->presentation;
    selected = GV_SHAPE_LAYER_SELBUF(layer);
    points = gv_points_num_points(layer->data);

    /* Crosshairs are "sprites": always drawn upright, the same size */
    dx = layer->point_size;
    dy = 0.0;
    gv_view_area_correct_for_transform(view, dx, dy, &dx, &dy);    
    
    glColor4fv(GV_SHAPE_LAYER(layer)->color);
    glBegin(GL_LINES);
    for (i=0; i < points; ++i)
    {
	if (selected[i] && !presentation)
	{
	    hit_selected = 1;
	    continue;
	}

	point = gv_points_get_point(layer->data, i);
	x = point->v.x;
	y = point->v.y;

	glVertex2(x-dx, y-dy);
	glVertex2(x+dx, y+dy);
	glVertex2(x+dy, y-dx);
	glVertex2(x-dy, y+dx);
    }
    glEnd();

    if (hit_selected && ! GV_SHAPE_LAYER(layer)->flags & GV_DELAY_SELECTED)
    {
	gv_point_layer_draw_selected(layer, view);
    } 
}

static void
gv_point_layer_draw_selected(GvPointLayer *layer, GvViewArea *view)
{
    gint i, points;
    GvPoint *point;
    gint *selected;
    gvgeocoord dx, dy, bx, by, x, y;

    selected = GV_SHAPE_LAYER_SELBUF(layer);
    points = gv_points_num_points(layer->data);

    /* Crosshairs are "sprites": always drawn upright, the same size */
    dx = layer->point_size;
    dy = 0.0;
    gv_view_area_correct_for_transform(view, dx, dy, &dx, &dy);    
    bx = by = layer->point_size + 2;
    gv_view_area_correct_for_transform(view, bx, by, &bx, &by);    

    glColor4fv(GV_SHAPE_LAYER(layer)->color);
    for (i=0; i < points; ++i)
    {
	if (selected[i])
	{
	    point = gv_points_get_point(layer->data, i);
	    x = point->v.x;
	    y = point->v.y;

	    /* Draw crosshairs */
	    glBegin(GL_LINES);
	    glVertex2(x-dx, y-dy);
	    glVertex2(x+dx, y+dy);
	    glVertex2(x+dy, y-dx);
	    glVertex2(x-dy, y+dx);
	    glEnd();

	    /* Draw box around crosshairs */
	    glBegin(GL_LINE_LOOP);
	    glVertex2(x-bx, y-by);
	    glVertex2(x+by, y-bx);
	    glVertex2(x+bx, y+by);
	    glVertex2(x-by, y+bx);
	    glEnd();
	}
    }    
}

static void
gv_point_layer_pick_shape(GvShapeLayer *shape_layer)
{
    gint i, points;
    GvPoint *point;
    GvPointLayer *layer;

    if (!gv_layer_is_visible(GV_LAYER(layer))) return;
    layer = GV_POINT_LAYER(shape_layer);
    points = gv_points_num_points(layer->data);

    glPointSize(layer->point_size * 2.0);
    for (i=0; i < points; ++i)
    {
	point = gv_points_get_point(layer->data, i);

	glLoadName(i);
	glBegin(GL_POINTS);
	glVertex2v((GLgeocoord*)&point->v);
	glEnd();
    }
}

static void
gv_point_layer_delete_selected(GvPointLayer *layer)
{
    GArray *sel;

    sel = g_array_new(FALSE, FALSE, sizeof(gint));
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_ALL, sel))
    {
	/* This will force a selection clear */
	gv_points_delete_points(layer->data, sel->len, (gint*)sel->data);
    }
    g_array_free(sel, TRUE);
}

static void
gv_point_layer_translate_selected(GvPointLayer *layer, GvVertex *delta)
{
    GArray *sel;

    sel = g_array_new(FALSE, FALSE, sizeof(gint));
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_ALL, sel))
    {
	/* This will force a selection clear */
	gv_points_translate_points(layer->data, sel->len, (gint*)sel->data,
				   delta->x, delta->y);
    }
    g_array_free(sel, TRUE);
}

static void
gv_point_layer_extents(GvPointLayer *layer, GvRect *rect)
{
    gv_points_get_extents(layer->data, rect);
}

static void
gv_point_layer_data_change(GvPointLayer *layer, gpointer change_info)
{
    /* Reset the selected array to reflect the data length */
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
				  gv_points_num_points(layer->data));
}
