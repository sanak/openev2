/******************************************************************************
 * $Id: gvarealayer.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Display layer of GvAreas.
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
 * $Log: gvarealayer.c,v $
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
 * Revision 1.18  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.17  2000/06/20 13:26:54  warmerda
 * added standard headers
 *
 */

#include "gvarealayer.h"
#include <gtk/gtksignal.h>
#include <GL/gl.h>

static void gv_area_layer_class_init(GvAreaLayerClass *klass);
static void gv_area_layer_init(GvAreaLayer *layer);
static void gv_area_layer_draw(GvAreaLayer *layer, GvViewArea *view);
static void gv_area_layer_extents(GvAreaLayer *layer, GvRect *rect);
static void gv_area_layer_data_change(GvAreaLayer *layer, gpointer change_info);
static void gv_area_layer_draw_selected(GvAreaLayer *layer, GvViewArea *view);
static void gv_area_layer_delete_selected(GvAreaLayer *layer);
static void gv_area_layer_translate_selected(GvAreaLayer *layer, GvVertex *delta);
static void gv_area_layer_pick_shape(GvAreaLayer *layer);
static void gv_area_layer_pick_node(GvAreaLayer *layer);
static void gv_area_layer_get_node(GvAreaLayer *layer, GvNodeInfo *info);
static void gv_area_layer_move_node(GvAreaLayer *layer, GvNodeInfo *info);
static void gv_area_layer_insert_node(GvAreaLayer *layer, GvNodeInfo *info);
static void gv_area_layer_delete_node(GvAreaLayer *layer, GvNodeInfo *info);
static void gv_area_layer_node_motion(GvAreaLayer *layer, gint area_id);

GtkType
gv_area_layer_get_type(void)
{
    static GtkType area_layer_type = 0;

    if (!area_layer_type)
    {
	static const GtkTypeInfo area_layer_info =
	{
	    "GvAreaLayer",
	    sizeof(GvAreaLayer),
	    sizeof(GvAreaLayerClass),
	    (GtkClassInitFunc) gv_area_layer_class_init,
	    (GtkObjectInitFunc) gv_area_layer_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	area_layer_type = gtk_type_unique(gv_shape_layer_get_type(),
					  &area_layer_info);
    }
    return area_layer_type;
}

static void
gv_area_layer_class_init(GvAreaLayerClass *klass)
{
    GvDataClass *data_class;
    GvLayerClass *layer_class;
    GvShapeLayerClass *shape_layer_class;

    data_class = (GvDataClass*) klass;
    layer_class = (GvLayerClass*) klass;
    shape_layer_class = (GvShapeLayerClass*) klass;

    data_class->changed = (void (*)(GvData*,gpointer))
        gv_area_layer_data_change;
    
    layer_class->draw = (void (*)(GvLayer*,GvViewArea*)) gv_area_layer_draw;
    layer_class->extents_request = (void (*)(GvLayer*,GvRect*))
        gv_area_layer_extents;

    shape_layer_class->draw_selected = (void (*)(GvShapeLayer*,GvViewArea*))
        gv_area_layer_draw_selected;
    shape_layer_class->delete_selected = (void (*)(GvShapeLayer*))
        gv_area_layer_delete_selected;
    shape_layer_class->translate_selected = (void (*)(GvShapeLayer*,GvVertex*))
        gv_area_layer_translate_selected;
    shape_layer_class->pick_shape = (void (*)(GvShapeLayer*))
        gv_area_layer_pick_shape;
    shape_layer_class->pick_node = (void (*)(GvShapeLayer*))
        gv_area_layer_pick_node;
    shape_layer_class->get_node = (void (*)(GvShapeLayer*,GvNodeInfo*))
        gv_area_layer_get_node;
    shape_layer_class->move_node = (void (*)(GvShapeLayer*,GvNodeInfo*))
        gv_area_layer_move_node;
    shape_layer_class->insert_node =  (void (*)(GvShapeLayer*,GvNodeInfo*))
        gv_area_layer_insert_node;
    shape_layer_class->delete_node =  (void (*)(GvShapeLayer*,GvNodeInfo*))
        gv_area_layer_delete_node;
    shape_layer_class->node_motion =  (void (*)(GvShapeLayer*,gint))
        gv_area_layer_node_motion;
}

static void
gv_area_layer_init(GvAreaLayer *layer)
{
    GvColor default_area_color = {0.0, 1.0, 0.0, 0.3};  /* green */
    
    layer->data = NULL;
    layer->edit_ring = -1;
    gv_color_copy(GV_SHAPE_LAYER(layer)->color, default_area_color);
}

GtkObject *
gv_area_layer_new(GvAreas *data)
{
    GvAreaLayer *layer = GV_AREA_LAYER(gtk_type_new(gv_area_layer_get_type()));
    
    if (data)
    {
	layer->data = data;
    }
    else
    {
	layer->data = GV_AREAS(gv_areas_new());
    }

    /* In case areas exist in data before layer is creatd */
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
				  gv_areas_num_areas(layer->data));

    gv_data_set_parent(GV_DATA(layer), GV_DATA(layer->data));

    
    
    return GTK_OBJECT(layer);
}

gint
gv_area_layer_select_new_area(GvAreaLayer *layer)
{
    gint area_id;
    
    layer->edit_ring = 0;
    area_id = gv_areas_new_area(layer->data);
    
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
				  gv_areas_num_areas(layer->data));
    gv_shape_layer_select_shape(GV_SHAPE_LAYER(layer), area_id);

    return area_id;
}

gint
gv_area_layer_select_new_ring(GvAreaLayer *layer, gint area_id)
{
    gint ring_id;
    
    ring_id = gv_areas_new_ring(layer->data, area_id);
    g_return_val_if_fail(ring_id > 0, 0);
    layer->edit_ring = ring_id;

    gv_shape_layer_clear_selection(GV_SHAPE_LAYER(layer));
    gv_shape_layer_select_shape(GV_SHAPE_LAYER(layer), area_id);

    return ring_id;
}

void
gv_area_layer_edit_done(GvAreaLayer *layer)
{
    layer->edit_ring = -1;
}

/*******************************************************/

static void
gv_area_layer_draw(GvAreaLayer *layer, GvViewArea *view)
{
    gint i, r, areas, rings;
    gint *selected, presentation;
    gint hit_selected = FALSE;
    GvArea *area;
    GArray *ring;

    presentation = GV_LAYER(layer)->presentation;
    selected = GV_SHAPE_LAYER_SELBUF(layer);
    areas = gv_areas_num_areas(layer->data);

    glColor4fv(GV_SHAPE_LAYER(layer)->color);

    glEnableClientState(GL_VERTEX_ARRAY);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    for (i=0; i < areas; ++i)
    {
        int     fill_object;
        
	if (selected[i] && !presentation)
	{
	    continue;
	}

	area = gv_areas_get_area(layer->data, i);

        if( area->fill != NULL )
            glVertexPointer(2, GL_GEOCOORD, 0, area->fill->data );
        for( fill_object = 0; fill_object < area->fill_objects; fill_object++ )
        {
            int f_offset=g_array_index(area->mode_offset,gint,fill_object*2+1);
            int f_mode = g_array_index(area->mode_offset,gint,fill_object*2);
            int f_len;

            if( fill_object == area->fill_objects-1 )
              f_len = area->fill->len - f_offset;
            else
              f_len = g_array_index(area->mode_offset,gint,fill_object*2+3)
                - f_offset;
            
	    glDrawArrays(f_mode, f_offset, f_len);
	}
    }
    glDisable(GL_BLEND);

    for (i=0; i < areas; ++i)
    {
	if (selected[i] && !presentation)
	{
	    hit_selected = 1;
	    continue;
	}

	area = gv_areas_get_area(layer->data, i);
	rings = gv_areas_num_rings(area);

	for (r=0; r < rings; ++r)
	{
	    ring = gv_areas_get_ring(area, r);
	    
	    glVertexPointer(2, GL_GEOCOORD, 0, ring->data);
	    glDrawArrays(GL_LINE_LOOP, 0, ring->len);
	}	
    }
    glDisableClientState(GL_VERTEX_ARRAY);

    if (hit_selected && ! GV_SHAPE_LAYER(layer)->flags & GV_DELAY_SELECTED)
    {
	gv_area_layer_draw_selected(layer, view);
    }    
}

static void
gv_area_layer_draw_selected(GvAreaLayer *layer, GvViewArea *view)
{
    gint i, r, areas, rings;
    gint *selected;
    GvArea *area;
    GArray *ring;
    GLenum mode;

    selected = GV_SHAPE_LAYER_SELBUF(layer);
    areas = gv_areas_num_areas(layer->data);

    glColor4fv(GV_SHAPE_LAYER(layer)->color);
    
    glEnableClientState(GL_VERTEX_ARRAY);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    for (i=0; i < areas; ++i)
    {
	if (selected[i])
	{
            int   fill_object;
            
	    area = gv_areas_get_area(layer->data, i);
            if( area->fill != NULL )
              glVertexPointer(2, GL_GEOCOORD, 0, area->fill->data );
            
            for( fill_object = 0;
                 fill_object < area->fill_objects;
                 fill_object++ )
            {
                int f_offset =
                  g_array_index(area->mode_offset,gint,fill_object*2+1);
                int f_mode =
                  g_array_index(area->mode_offset,gint,fill_object*2);
                int f_len;
                
                if( fill_object == area->fill_objects-1 )
                  f_len = area->fill->len - f_offset;
                else
                  f_len = g_array_index(area->mode_offset,gint,fill_object*2+3)
                    - f_offset;
                
                glDrawArrays(f_mode, f_offset, f_len);
		
#ifdef SHOW_TESS_LINES		    
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glColor4f(1.0,1.0,1.0,1.0);
                glDrawArrays(f_mode, f_offset, f_len);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glColor4f(GV_SHAPE_LAYER(layer)->color);
#endif /* SHOW_TESS_LINES */		    
	    }
	}
    }
    glDisable(GL_BLEND);
    
    glPointSize(3.0);
    for (i=0; i < areas; ++i)
    {
	if (selected[i])
	{
	    area = gv_areas_get_area(layer->data, i);
	    rings = gv_areas_num_rings(area);
	    
	    for (r=0; r < rings; ++r)
	    {
		if (r == layer->edit_ring)
		{
		    mode = GL_LINE_STRIP;
		}
		else
		{
		    mode = GL_LINE_LOOP;
		}
		
		ring = gv_areas_get_ring(area, r);
		
		glVertexPointer(2, GL_GEOCOORD, 0, ring->data);
		glDrawArrays(mode, 0, ring->len);

#ifdef HAVE_BROKEN_GL_POINTS
		{
		    int j;
		    for (j=0; j < ring->len; ++j)
		    {
			glBegin(GL_POINTS);
			glArrayElement(j);
			glEnd();
		    }
		}
#else
		glDrawArrays(GL_POINTS, 0, ring->len);
#endif		
	    }
	}
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

static void
gv_area_layer_pick_shape(GvAreaLayer *layer)
{
    gint i, areas;
    GvArea *area;

    if (!gv_layer_is_visible(GV_LAYER(layer))) return;

    areas = gv_areas_num_areas(layer->data);

    glEnableClientState(GL_VERTEX_ARRAY);
    for (i=0; i < areas; ++i)
    {
        int    fill_object;
        
	glLoadName(i);
	area = gv_areas_get_area(layer->data, i);

        if( area->fill != NULL )
          glVertexPointer(2, GL_GEOCOORD, 0, area->fill->data );
        for( fill_object = 0; fill_object < area->fill_objects; fill_object++ )
        {
            int f_offset=g_array_index(area->mode_offset,gint,fill_object*2+1);
            int f_mode = g_array_index(area->mode_offset,gint,fill_object*2);
            int f_len;

            if( fill_object == area->fill_objects-1 )
              f_len = area->fill->len - f_offset;
            else
              f_len = g_array_index(area->mode_offset,gint,fill_object*2+3)
                - f_offset;
            
	    glDrawArrays(f_mode, f_offset, f_len);
	}
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

static void
gv_area_layer_pick_node(GvAreaLayer *layer)
{
    GvArea *area;
    GArray *ring;
    gint sel, rings, r, i;

    if (!gv_layer_is_visible(GV_LAYER(layer))) return;

    if (!gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_FIRST, &sel))
    {
	return;
    }
    area = gv_areas_get_area(layer->data, sel);
    rings = gv_areas_num_rings(area);

    glEnableClientState(GL_VERTEX_ARRAY);
    
    /* Nodes first */
    glLoadName(0);
    glPushName(-1);
    glPointSize(1.0);
    for (r=0; r < rings; ++r)
    {
	glLoadName(r);
	glPushName(-1);
	
	ring = gv_areas_get_ring(area, r);
	glVertexPointer(2, GL_GEOCOORD, 0, ring->data);
	for (i=0; i < ring->len; ++i)
	{
	    glLoadName(i);
	    glBegin(GL_POINTS);
	    glArrayElement(i);
	    glEnd();
	}
	glPopName(); /* node id */
    }
    glPopName(); /* ring id */

    /* Segments next */
    glLoadName(1);
    glPushName(-1);
    for (r=0; r < rings; ++r)
    {
	glLoadName(r);
	
	ring = gv_areas_get_ring(area, r);
	glVertexPointer(2, GL_GEOCOORD, 0, ring->data);

	/* First segment connects last vertex to first vertex:
	   "before" vertex zero */
	glPushName(0);
	glBegin(GL_LINES);
	glArrayElement(ring->len - 1);
	glArrayElement(0);
	glEnd();	
	for (i=1; i < ring->len; ++i)
	{
	    glLoadName(i);
	    glBegin(GL_LINES);
	    glArrayElement(i-1);
	    glArrayElement(i);
	    glEnd();
	}
	glPopName(); /* node id */
    }
    glPopName(); /* ring id */
    glDisableClientState(GL_VERTEX_ARRAY);
}

static void
gv_area_layer_delete_selected(GvAreaLayer *layer)
{
    GArray *sel;

    sel = g_array_new(FALSE, FALSE, sizeof(gint));
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_ALL, sel))
    {
	/* This will force a selection clear */
	gv_areas_delete_areas(layer->data, sel->len, (gint*)sel->data);
    }
    g_array_free(sel, TRUE);
}

static void
gv_area_layer_translate_selected(GvAreaLayer *layer, GvVertex *delta)
{
    GArray *sel;

    sel = g_array_new(FALSE, FALSE, sizeof(gint));
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_ALL, sel))
    {
	/* This will force a selection clear */
	gv_areas_translate_areas(layer->data, sel->len, (gint*)sel->data,
				 delta->x, delta->y);
    }
    g_array_free(sel, TRUE);
}

static void
gv_area_layer_get_node(GvAreaLayer *layer, GvNodeInfo *info)
{
    info->vertex = gv_areas_get_node(layer->data, info->shape_id,
				     info->ring_id, info->node_id);
}

static void
gv_area_layer_move_node(GvAreaLayer *layer, GvNodeInfo *info)
{
    gv_areas_move_node(layer->data, info->shape_id, info->ring_id,
		       info->node_id, info->vertex);
}

static void
gv_area_layer_insert_node(GvAreaLayer *layer, GvNodeInfo *info)
{
    gv_areas_insert_nodes(layer->data, info->shape_id, info->ring_id,
			  info->node_id, 1, info->vertex);
}

static void
gv_area_layer_delete_node(GvAreaLayer *layer, GvNodeInfo *info)
{
    gv_areas_clear_fill(layer->data, info->shape_id);
    gv_areas_delete_nodes(layer->data, info->shape_id, info->ring_id,
			  1, &info->node_id);
}

static void
gv_area_layer_node_motion(GvAreaLayer *layer, gint area_id)
{
    gv_areas_clear_fill(layer->data, area_id);
}

static void
gv_area_layer_extents(GvAreaLayer *layer, GvRect *rect)
{
    gv_areas_get_extents(layer->data, rect);
}

static void
gv_area_layer_data_change(GvAreaLayer *layer, gpointer change_info)
{
    /* Reset the selected array to reflect the data length */
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
				  gv_areas_num_areas(layer->data));
}
