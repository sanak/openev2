/******************************************************************************
 * $Id: gvareatool.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Area (Polygon) editing mode.
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
 * $Log: gvareatool.c,v $
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
 * Revision 1.25  2004/10/28 21:59:52  gmwalter
 * Send a changed signal when editing finishes.
 *
 * Revision 1.24  2004/01/27 19:51:34  gmwalter
 * Make sure memento pointers are set to NULL
 * after being freed.
 *
 * Revision 1.23  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.22  2002/10/01 15:06:38  warmerda
 * ensure old selection clear when selecting new object
 *
 * Revision 1.21  2002/09/30 13:24:54  warmerda
 * ensure areas are closed properly
 *
 * Revision 1.20  2001/08/08 17:44:12  warmerda
 * use gv_shape_type() macro
 *
 * Revision 1.19  2001/04/09 18:13:51  warmerda
 * improved warning
 *
 * Revision 1.18  2000/08/10 15:47:35  warmerda
 * fixed one-off undo quirk when adding rings to existing area
 *
 * Revision 1.17  2000/08/08 20:58:47  warmerda
 * recover from layer destruction
 *
 * Revision 1.16  2000/07/27 20:06:23  warmerda
 * added boundary constraints
 *
 * Revision 1.15  2000/07/24 14:00:24  warmerda
 * set_layer with NULL should be allowed
 *
 * Revision 1.14  2000/07/21 01:31:11  warmerda
 * added read_only flag for GvData, and utilize for vector layers
 *
 * Revision 1.13  2000/07/17 20:20:32  warmerda
 * enable delete-last-vertex
 *
 * Revision 1.12  2000/06/20 13:26:54  warmerda
 * added standard headers
 *
 */

#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include "gvareatool.h"
#include "gvundo.h"
#include "gvshapeslayer.h"
#include <GL/gl.h>

static void gv_area_tool_class_init(GvAreaToolClass *klass);
static void gv_area_tool_init(GvAreaTool *tool);
static void gv_area_tool_draw(GvAreaTool *tool);
static void gv_area_tool_button_press(GvTool *tool, GdkEventButton *event);
static gboolean gv_area_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static void gv_area_tool_key_press(GvTool *tool, GdkEventKey *event);
static void gv_area_tool_deactivate(GvTool *tool, GvViewArea *view);
static void gv_area_tool_stop_drawing(GvAreaTool *tool);
static gint gv_area_tool_configure(GvAreaTool *tool);

GtkType
gv_area_tool_get_type(void)
{
    static GtkType area_tool_type = 0;

    if (!area_tool_type)
    {
	static const GtkTypeInfo area_tool_info =
	{
	    "GvAreaTool",
	    sizeof(GvAreaTool),
	    sizeof(GvAreaToolClass),
	    (GtkClassInitFunc) gv_area_tool_class_init,
	    (GtkObjectInitFunc) gv_area_tool_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	area_tool_type = gtk_type_unique(gv_tool_get_type(),
					 &area_tool_info);
    }
    return area_tool_type;
}

static void
gv_area_tool_class_init(GvAreaToolClass *klass)
{
    GvToolClass *tool_class;

    tool_class = (GvToolClass*)klass;
    tool_class->deactivate = gv_area_tool_deactivate;
    tool_class->button_press = gv_area_tool_button_press;
    tool_class->motion_notify = gv_area_tool_motion_notify;
    tool_class->key_press = gv_area_tool_key_press;
}

static void
gv_area_tool_init(GvAreaTool *tool)
{
    GV_TOOL(tool)->cursor = gdk_cursor_new(GDK_TCROSS);
    tool->layer = NULL;
    tool->named_layer = NULL;
    tool->drawing = FALSE;
    tool->ring_id = 0;
    tool->memento = NULL;
}

GvTool *
gv_area_tool_new(void)
{
    return GV_TOOL(gtk_type_new(GV_TYPE_AREA_TOOL));
}

static gint gv_area_tool_layer_destroy( GtkObject *layer, gpointer data )

{
    GvAreaTool *tool = (GvAreaTool *) data;

    if( tool->layer == GV_SHAPE_LAYER(layer) )
        gv_area_tool_set_layer( tool, NULL );
    
    return 0;
}

void
gv_area_tool_set_layer(GvAreaTool *tool, GvShapeLayer *layer)
{
    if (GV_TOOL(tool)->view == NULL)
    {
	g_warning("gv_area_tool_set_layer(): inactive tool");
	return;
    }

    if( layer != NULL && gv_data_is_read_only( GV_DATA(layer) ) )
    {
        g_warning( "gv_area_tool_set_layer(): layer is read-only" );
        return;
    }

    /* Disconnect from the previous layer (for draw) */
    if (tool->layer)
    {
	if (tool->drawing)
	{
	    gv_area_tool_stop_drawing(tool);
	}	
	gv_shape_layer_clear_selection(GV_SHAPE_LAYER(tool->layer));
	gtk_signal_disconnect_by_data(GTK_OBJECT(tool->layer), (gpointer)tool);
	gv_view_area_queue_draw(GV_TOOL(tool)->view);
    }
    
    tool->layer = layer;

    if (layer)
    {
	gv_view_area_set_active_layer(GV_TOOL(tool)->view, GTK_OBJECT(layer));
	
        /* Redraw when the layer draws */
	gtk_signal_connect_object_after(GTK_OBJECT(layer), "draw",
					GTK_SIGNAL_FUNC(gv_area_tool_draw),
					GTK_OBJECT(tool));

        /* recover if layer destroyed */
        gtk_signal_connect(
            GTK_OBJECT(layer), "destroy", 
            GTK_SIGNAL_FUNC(gv_area_tool_layer_destroy),
            GTK_OBJECT(tool));
    }
}

void
gv_area_tool_set_named_layer(GvAreaTool *tool, gchar *name)
{
    if (tool->named_layer)
    {
	g_free(tool->named_layer);
	tool->named_layer = NULL;
    }
    if (name)
    {
	tool->named_layer = g_strdup(name);	
    }
    /* Tool layer will be updated next time it is configured */
}

static void
gv_area_tool_draw(GvAreaTool *tool)
{
    if (tool->drawing)
    {
	/* Color is set when the layer is drawn,
	   so we don't need to repeat it here */

	glBegin(GL_LINES);
	glVertex2v((GLgeocoord*)&tool->v_head);
	glVertex2v((GLgeocoord*)&tool->v_tail);
	glEnd();
    }
}

static void
gv_area_tool_button_press(GvTool *rtool, GdkEventButton *event)
{
    GvAreaTool *tool = GV_AREA_TOOL(rtool);

    /* ignore control corded buttons -- these are for zooming and panning */
    if( event->state & GDK_CONTROL_MASK )
        return;

    if (event->button == 1)
    {
	gint area_id = -1;
	
	if (!gv_area_tool_configure(tool)) return;

	/* Map pointer position to tail vertex */
	gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
				 &tool->v_tail.x, &tool->v_tail.y);
	
	if (tool->drawing)
	{
            gv_tool_clamp_to_bounds( GV_TOOL(tool), 
                                     &(tool->v_tail.x), &(tool->v_tail.y) );

	    /* Filter out duplicate verticies */
	    if (tool->v_head.x == tool->v_tail.x &&
		tool->v_head.y == tool->v_tail.y)
	    {
		return;
	    }
	    
	    /* Add a new vertex to the area */
	    gv_shape_layer_selected(GV_SHAPE_LAYER(tool->layer), GV_FIRST,
				    &area_id);
            if( area_id == -1 )
            {
                g_warning( "gv_area_tool_button_press: get selection failed.");
                return;
            }
	}
	else
	{
            if( !gv_tool_check_bounds( GV_TOOL(tool), 
                                       tool->v_tail.x, tool->v_tail.y ) )
                return;
            
	    /* Start a new ring */
	    tool->drawing = TRUE;
	    
	    /* Close down undo.  We don't want undos pushed for each vertex */
	    gv_undo_close();
	    gv_undo_disable();

	    /* If the first click is inside an existing area,
	       start a hole ring */
	    if (gv_shape_layer_pick_shape(GV_SHAPE_LAYER(tool->layer),
					  GV_TOOL(tool)->view,
					  event->x, event->y, &area_id))
	    {
                if( GV_IS_AREA_LAYER(tool->layer) )
                    tool->ring_id = 
                        gv_area_layer_select_new_ring(
                            GV_AREA_LAYER(tool->layer), area_id);
                else
                {
                    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE,1,NULL};

                    GvShape     *area;
                    GvShapes    *shapes;

                    gv_shape_layer_clear_selection(
                        GV_SHAPE_LAYER(tool->layer));
                    gv_shape_layer_select_shape(GV_SHAPE_LAYER(tool->layer),
                                                area_id);

                    shapes = GV_SHAPES_LAYER(tool->layer)->data;
                    area = gv_shapes_get_shape( shapes, area_id );
                    tool->ring_id = gv_shape_get_rings( area );

                    /* force a changing() signal for the shape */
                    area = gv_shape_copy(area);
                    gv_shapes_replace_shapes( shapes, 1, &area_id, &area, 
                                              FALSE );

                    change_info.shape_id = &area_id;
                    if( tool->memento != NULL )
                        gv_data_del_memento( GV_DATA(shapes), tool->memento );
                    tool->memento = gv_data_get_memento( GV_DATA(shapes), 
                                                         &change_info );
                }
	    }
	    else
	    {
                /* Start a new area */
                if( GV_IS_AREA_LAYER(tool->layer) )
                {
                    area_id = gv_area_layer_select_new_area(
                        GV_AREA_LAYER(tool->layer));
                    tool->ring_id = 0;
                }
                else
                {
                    GvShape *area = gv_shape_new( GVSHAPE_AREA );

                    area_id = gv_shapes_layer_select_new_shape( 
                        GV_SHAPES_LAYER(tool->layer), area );
                    tool->ring_id = 0;
                }
	    }
	}

	tool->v_head = tool->v_tail;
        if( GV_IS_AREA_LAYER(tool->layer) )
            gv_areas_append_nodes(GV_AREA_LAYER(tool->layer)->data, area_id, 
                                  tool->ring_id, 1, &tool->v_tail);
        else
        {
            GvShapes *shapes = GV_SHAPES_LAYER(tool->layer)->data;
            GvShape *area = gv_shapes_get_shape(shapes, area_id );

            if( gv_shape_type(area) == GVSHAPE_AREA )
            {
                gv_shape_add_node(area, tool->ring_id, 
                                  tool->v_tail.x, tool->v_tail.y, 0);
                ((GvAreaShape *) area)->fill_objects = -2;
            }
            else
                g_warning( "selected object not area in gvareatool mode!\n" );
        }
    }
    else if (event->button == 3 && tool->drawing)
    {
	gv_area_tool_stop_drawing(tool);
    }	
}

static gboolean
gv_area_tool_motion_notify(GvTool *rtool, GdkEventMotion *event)
{
    GvAreaTool *tool = GV_AREA_TOOL(rtool);

    if (tool->drawing)
    {
	/* Map pointer position to tail vertex */
	gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
				 &tool->v_tail.x, &tool->v_tail.y);
	
        gv_tool_clamp_to_bounds( GV_TOOL(tool), 
                                 &(tool->v_tail.x), &(tool->v_tail.y) );

	gv_view_area_queue_draw(GV_TOOL(tool)->view);
    }
    return FALSE;
}

static void
gv_area_tool_key_press(GvTool *rtool, GdkEventKey *event)
{
    GvAreaTool *tool = GV_AREA_TOOL(rtool);

    if (!gv_area_tool_configure(tool)) return;
    
    switch (event->keyval)
    {
	case GDK_Delete:
	case GDK_BackSpace:
          if( tool->drawing && !GV_IS_AREA_LAYER(tool->layer) )
          {
              gint line_id;
              GvNodeInfo  node_info;
              GvShape     *shape;
              GvShapes *shapes = GV_SHAPES_LAYER(tool->layer)->data;

              gv_shape_layer_selected(GV_SHAPE_LAYER(tool->layer), GV_FIRST,
                                      &line_id);

              shape = gv_shapes_get_shape( shapes, line_id );

              node_info.shape_id = line_id;
              node_info.ring_id = MAX(0,gv_shape_get_rings(shape) - 1);
              node_info.node_id = gv_shape_get_nodes( shape, 
                                                      node_info.ring_id) - 1;

              if( gv_shape_get_nodes( shape, node_info.ring_id ) > 1 )
              {
                  tool->v_head.x = gv_shape_get_x( shape, 
                                                   node_info.ring_id, 
                                                   node_info.node_id-1);
                  tool->v_head.y = gv_shape_get_y( shape, 
                                                   node_info.ring_id, 
                                                   node_info.node_id-1);
              }
              else
              {
                  tool->drawing = FALSE;
              }

              gv_shape_layer_delete_node(tool->layer, &node_info );


              if( !tool->drawing )
                  gv_area_tool_stop_drawing( tool );
              else
              {
                  shape = gv_shapes_get_shape( shapes, line_id );
                  if( shape != NULL && gv_shape_type(shape) == GVSHAPE_AREA )
                      ((GvAreaShape *) shape)->fill_objects = -2;
              }
          }
          break;
    }
}

static void
gv_area_tool_deactivate(GvTool *rtool, GvViewArea *view)
{
    GvAreaTool *tool = GV_AREA_TOOL(rtool);

    /* Disconnect from layer */
    if (tool->layer)
    {
	gv_area_tool_set_layer(tool, NULL);
    }

    /* Call the parent class func */
    GV_TOOL_DEACTIVATE(tool, view);
}

static void
gv_area_tool_stop_drawing(GvAreaTool *tool)
{
    gint sel, push_undo = TRUE;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 1, NULL};

    if( GV_IS_AREA_LAYER(tool->layer) )
    {
        GvAreaLayer *alayer = GV_AREA_LAYER(tool->layer);

        change_info.shape_id = &sel;

        tool->drawing = FALSE;
        gv_area_layer_edit_done(alayer);

        gv_shape_layer_selected(GV_SHAPE_LAYER(alayer), GV_FIRST, &sel);
        gv_areas_clear_fill(alayer->data, sel);
        if (!gv_areas_tessellate_areas(alayer->data, 1, &sel))
        {
            g_warning("Invalid area drawn");
            if(tool->ring_id != 0 )
            {
                gv_areas_delete_ring(alayer->data,sel,tool->ring_id);
                gv_areas_clear_fill(alayer->data, sel);
                gv_areas_tessellate_areas(alayer->data, 1, &sel);
            }
            else
            {
                gv_shape_layer_delete_selected(GV_SHAPE_LAYER(alayer));
            }
            push_undo = FALSE;
        }

        /* Reopen undo.  Push a memento describing the ring addition */
        gv_undo_enable();
        gv_undo_open();
        if (push_undo)
        {
            if (tool->ring_id > 0)
            {
                if( tool->memento != NULL )
                {
                    gv_undo_push(tool->memento);
                    tool->memento = NULL;
                }
            }
            else
            {
                gv_undo_push(gv_data_get_memento(GV_DATA(alayer->data),
                                                 &change_info));
            }
        }

        if( tool->memento != NULL )
        {
            gv_data_del_memento( GV_DATA(alayer->data), tool->memento );
            tool->memento = NULL;
        }
    }
    else /* GV_IS_SHAPES_LAYER(tool->layer) */
    {
        GvShapesLayer *slayer = GV_SHAPES_LAYER(tool->layer);
        GvShapes      *shapes = slayer->data;
        GvAreaShape   *area;
        GvShapeChangeInfo sent_change_info = {GV_CHANGE_REPLACE, 1, NULL};

        change_info.shape_id = &sel;

        tool->drawing = FALSE;

        if( gv_shape_layer_selected(GV_SHAPE_LAYER(slayer), GV_FIRST, &sel) )
            area = (GvAreaShape *) gv_shapes_get_shape(shapes, sel);
        else
            area = NULL;

        if( area != NULL && gv_shape_type(area) == GVSHAPE_AREA )
        {
            GvShape *shape = (GvShape *) area;
            int   vert_count = gv_shape_get_nodes( shape, tool->ring_id );
            /* If the ring isn't properly closed, do now. */
            if( gv_shape_get_x( shape, tool->ring_id, 0 )
                    != gv_shape_get_x( shape, tool->ring_id, vert_count-1 )
                || gv_shape_get_y( shape, tool->ring_id, 0 )
                    != gv_shape_get_y( shape, tool->ring_id, vert_count-1 )
                || gv_shape_get_z( shape, tool->ring_id, 0 )
                    != gv_shape_get_z( shape, tool->ring_id, vert_count-1 ) )
            {
                gv_shape_add_node(shape, tool->ring_id, 
                                  gv_shape_get_x( shape, tool->ring_id, 0 ),
                                  gv_shape_get_y( shape, tool->ring_id, 0 ),
                                  gv_shape_get_z( shape, tool->ring_id, 0 ) );
            }

            area->fill_objects = -1;

            if (!gv_area_shape_tessellate(area))
            {
                g_warning("Invalid area drawn");
                if(tool->ring_id != 0 )
                {
                    gv_shape_delete_ring((GvShape *) area,tool->ring_id);
                    gv_area_shape_tessellate(area);
                }
                else
                {
                    gv_shape_layer_delete_selected(GV_SHAPE_LAYER(slayer));
                }
                push_undo = FALSE;
            }
            
            /* Reopen undo.  Push a memento describing the ring addition */
            gv_undo_enable();
            gv_undo_open();
            if (push_undo)
            {
                if (tool->ring_id > 0)
                {
                    if( tool->memento != NULL )
                    {
                        gv_undo_push(tool->memento);
                        tool->memento = NULL;
                    }
                }
                else
                {
                    gv_undo_push(gv_data_get_memento(GV_DATA(shapes),
                                                     &change_info));
                }

                /* Force a changed() signal for the finalized shape,
                   but send it as a "replace" signal, since "add"
                   was already sent if necessary when first node 
                   was created. */
                sent_change_info.shape_id = &sel;
                gv_data_changed(GV_DATA(shapes), &sent_change_info);
            }
        }
        else
        {
            gv_undo_enable();
            gv_undo_open();
        }

        if( tool->memento != NULL )
        {
            gv_data_del_memento( GV_DATA(shapes), tool->memento );
            tool->memento = NULL;
        }

    }

    gv_view_area_queue_draw(GV_TOOL(tool)->view);	
}

static gint
gv_area_tool_configure(GvAreaTool *tool)
{
    /* Check that we still are working on the active layer */
    if (!tool->layer ||	GTK_OBJECT(tool->layer) !=
	gv_view_area_active_layer(GV_TOOL(tool)->view))
    {
	GtkObject *layer;

	if (tool->named_layer)
	{
	    /* Look for named layer if given */
	    layer = gv_view_area_get_named_layer(GV_TOOL(tool)->view,
						 tool->named_layer);
	}
	else
	{
	    /* Attempt to find a area layer to edit */
	    layer = gv_view_area_get_layer_of_type(GV_TOOL(tool)->view,
						   GV_TYPE_AREA_LAYER,
                                                   FALSE);
            if( layer == NULL )
                layer = gv_view_area_get_layer_of_type(GV_TOOL(tool)->view,
                                                       GV_TYPE_SHAPES_LAYER,
                                                       FALSE);
	}
	if (!layer)
	{
	    g_warning("gv_area_tool_configure(): no editable area layer in view");
	    return FALSE;
	}

	gv_area_tool_set_layer(tool, GV_SHAPE_LAYER(layer));
    }

    return tool->layer != NULL;
}
