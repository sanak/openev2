/******************************************************************************
 * $Id: gvnodetool.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Node editing mode.
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
 * $Log: gvnodetool.c,v $
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
 * Revision 1.13  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.12  2002/09/30 20:49:35  warmerda
 * maintain ring integrity when deleting a node
 *
 * Revision 1.11  2002/09/30 20:09:27  warmerda
 * Ensure that first/last vertex of closed rings are kept in sync.  Added
 * "track_node" concept.  Only works for GvShapesLayer areas, not the old
 * GvAreaLayer.
 *
 * Revision 1.10  2000/08/08 20:58:47  warmerda
 * recover from layer destruction
 *
 * Revision 1.9  2000/07/27 20:06:23  warmerda
 * added boundary constraints
 *
 * Revision 1.8  2000/07/24 14:00:24  warmerda
 * set_layer with NULL should be allowed
 *
 * Revision 1.7  2000/07/21 01:31:11  warmerda
 * added read_only flag for GvData, and utilize for vector layers
 *
 * Revision 1.6  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvnodetool.h"
#include "gvshapeslayer.h"
#include "gvundo.h"
#include "cpl_error.h"
#include <gtk/gtksignal.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>

static void gv_node_tool_class_init(GvNodeToolClass *klass);
static void gv_node_tool_init(GvNodeTool *tool);
static gboolean gv_node_tool_draw(GvTool *tool);
static gboolean gv_node_tool_button_press(GvTool *tool, GdkEventButton *event);
static gboolean gv_node_tool_button_release(GvTool *tool, GdkEventButton *event);
static gboolean gv_node_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static void gv_node_tool_key_press(GvTool *tool, GdkEventKey *event);
static void gv_node_tool_deactivate(GvTool *tool, GvViewArea *view);
static void gv_node_tool_layer_change(GvNodeTool *tool, gpointer info);
static gint gv_node_tool_configure(GvNodeTool *tool);
static GdkCursor* gv_node_tool_make_cursor(void);

GtkType
gv_node_tool_get_type(void)
{
    static GtkType node_tool_type = 0;

    if (!node_tool_type)
    {
	static const GtkTypeInfo node_tool_info =
	{
	    "GvNodeTool",
	    sizeof(GvNodeTool),
	    sizeof(GvNodeToolClass),
	    (GtkClassInitFunc) gv_node_tool_class_init,
	    (GtkObjectInitFunc) gv_node_tool_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	node_tool_type = gtk_type_unique(gv_tool_get_type(),
					 &node_tool_info);
    }
    return node_tool_type;
}

static void
gv_node_tool_class_init(GvNodeToolClass *klass)
{
    GvToolClass *tool_class;

    tool_class = (GvToolClass*)klass;
    tool_class->deactivate = gv_node_tool_deactivate;
    tool_class->button_press = gv_node_tool_button_press;
    tool_class->button_release = gv_node_tool_button_release;
    tool_class->motion_notify = gv_node_tool_motion_notify;
    tool_class->key_press = gv_node_tool_key_press;
}
static void
gv_node_tool_init(GvNodeTool *tool)
{
    GV_TOOL(tool)->cursor = gv_node_tool_make_cursor();
    tool->layer = NULL;
    tool->node_selected = FALSE;
    tool->dragging = FALSE;
    tool->tracking_node.shape_id = -1;
}

GvTool *
gv_node_tool_new(void)
{
    return GV_TOOL(gtk_type_new(GV_TYPE_NODE_TOOL));
}

static gint gv_node_tool_layer_destroy( GtkObject *layer, gpointer data )

{
    GvNodeTool *tool = (GvNodeTool *) data;

    if( tool->layer == GV_SHAPE_LAYER(layer) )
        gv_node_tool_set_layer( tool, NULL );
    
    return 0;
}

void
gv_node_tool_set_layer(GvNodeTool *tool, GvShapeLayer *layer)
{
    if (GV_TOOL(tool)->view == NULL)
    {
	g_warning("gv_node_tool_set_layer(): inactive tool");
	return;
    }

    if( layer != NULL && gv_data_is_read_only( GV_DATA(layer) ) )
    {
        g_warning( "gv_node_tool_set_layer(): layer is read-only" );
        return;
    }

    /* Disconnect from the previous layer (for draw) */
    if (tool->layer)
    {
	gv_shape_layer_clear_selection(GV_SHAPE_LAYER(tool->layer));
	gtk_signal_disconnect_by_data(GTK_OBJECT(tool->layer), (gpointer)tool);
    }
    
    tool->layer = layer;

    if (layer)
    {
	gv_view_area_set_active_layer(GV_TOOL(tool)->view, GTK_OBJECT(layer));

        /* Redraw when the layer draws */
	gtk_signal_connect_object_after(GTK_OBJECT(layer), "draw",
					GTK_SIGNAL_FUNC(gv_node_tool_draw),
					GTK_OBJECT(tool));

	/* Trap changes to the layer (e.g. undo) and clear node selection */
	gtk_signal_connect_object_after(GTK_OBJECT(layer), "changed",
					GTK_SIGNAL_FUNC(
					    gv_node_tool_layer_change),
					GTK_OBJECT(tool));	

        /* Recover if layer destroyed */
        gtk_signal_connect(
            GTK_OBJECT(layer), "destroy", 
            GTK_SIGNAL_FUNC(gv_node_tool_layer_destroy),
            GTK_OBJECT(tool));
    }
}

/**************************************************************/

static gboolean
gv_node_tool_button_press(GvTool *rtool, GdkEventButton *event)
{
    GvNodeTool *tool = GV_NODE_TOOL(rtool);

    if (event->button == 1)
    {
	gint shape_id, before;
	
	if (!gv_node_tool_configure(tool)) return FALSE;

	if (gv_shape_layer_pick_shape(tool->layer, GV_TOOL(tool)->view,
				      event->x, event->y, &shape_id))
	{
	    if (!gv_shape_layer_is_selected(tool->layer, shape_id))
	    {
		/* Select this shape (and only this shape) */
		gv_shape_layer_clear_selection(tool->layer);
		gv_shape_layer_select_shape(tool->layer, shape_id);

                /* New shape: deselect current node */
		tool->node_selected = FALSE;
                tool->tracking_node.shape_id = -1;
	    }
	    else
	    {
		if (gv_shape_layer_pick_node(tool->layer, GV_TOOL(tool)->view,
					     event->x, event->y, &before,
					     &tool->edit_node))
		{
		    /* Capture pointer position */
		    gv_view_area_map_pointer(GV_TOOL(tool)->view,
					     event->x, event->y,
					     &tool->v_tail.x, &tool->v_tail.y);
		    tool->v_head = tool->v_tail;

                    if( !gv_tool_check_bounds( GV_TOOL(tool), 
                                               tool->v_tail.x,
                                               tool->v_tail.y ) )
                        return FALSE;
		    
		    /* Select the node for edit and start dragging */
		    tool->node_selected = TRUE;
		    tool->dragging = TRUE;

                    /* If this node is the first in a ring of an area and
                       the last vertex of the ring matches it, capture that
                       node for "tracking" purposes. */

                    tool->tracking_node.shape_id = -1;
                    if( GV_IS_SHAPES_LAYER(tool->layer) 
                        && tool->edit_node.node_id == 0 )
                    {
                        GvShape *shape;
                        int ring = tool->edit_node.ring_id;
                        int last_node;

                        shape = gv_shapes_get_shape( 
                            GV_SHAPES_LAYER(tool->layer)->data,
                            tool->edit_node.shape_id );
                        last_node = gv_shape_get_nodes( shape, ring ) - 1;
                        
                        if( gv_shape_type(shape) == GVSHAPE_AREA 
                            && gv_shape_get_x( shape, ring, 0 )
                               == gv_shape_get_x( shape, ring, last_node )
                            && gv_shape_get_y( shape, ring, 0 )
                               == gv_shape_get_y( shape, ring, last_node )
                            && gv_shape_get_z( shape, ring, 0 )
                               == gv_shape_get_z( shape, ring, last_node ) )
                        {
                            tool->tracking_node.shape_id = 
                                tool->edit_node.shape_id;
                            tool->tracking_node.ring_id = 
                                tool->edit_node.ring_id;
                            tool->tracking_node.node_id = last_node;
                            gv_shape_layer_get_node( tool->layer, 
                                                     &tool->tracking_node );
                            tool->v_orig_tracking = 
                                *(tool->tracking_node.vertex);
                        }
                    }

		    if (before)
		    {
			/* Insert a new node */
			tool->edit_node.vertex = &tool->v_tail;
			tool->changing = TRUE;
			gv_shape_layer_insert_node(tool->layer,
						   &tool->edit_node);
			tool->changing = FALSE;
		    }

		    /* Get pointer to vertex */
		    gv_shape_layer_get_node(tool->layer, &tool->edit_node);

		    /* Snapshot the original node position */
		    tool->v_orig = *(tool->edit_node.vertex);
		}
		else
		{
		    /* Click away from a node: deselect the current node */
		    tool->node_selected = FALSE;
		}
	    }
	}
	else
	{
	    /* Click away from shape: deselect the current node,
	       but not the current shape */
	    tool->node_selected = FALSE;
            tool->tracking_node.shape_id = -1;
	}
	gv_view_area_queue_draw(GV_TOOL(tool)->view);
    }
    return FALSE;
}

static gboolean
gv_node_tool_button_release(GvTool *rtool, GdkEventButton *event)
{
    GvNodeTool *tool = GV_NODE_TOOL(rtool);

    if (tool->dragging && event->button == 1)
    {
	/* End dragging */
	tool->dragging = FALSE;
	
	gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
				 &tool->v_tail.x, &tool->v_tail.y);

        gv_tool_clamp_to_bounds( GV_TOOL(tool), 
				 &tool->v_tail.x, &tool->v_tail.y );

	/* Filter out clicks that wern't really drags */
	if (tool->v_tail.x != tool->v_head.x ||
	    tool->v_tail.y != tool->v_head.y)
	{
	    GvVertex temp;
            GvVertex temp_tracking;
            gint undo_group;
	    
	    /* Actually the node is already moved, but the layer doesn't
	       know this, so we put back the original vertex position
	       and move it properly */
	    temp.x = tool->v_orig.x + (tool->v_tail.x - tool->v_head.x);
	    temp.y = tool->v_orig.y + (tool->v_tail.y - tool->v_head.y);
	
	    *(tool->edit_node.vertex) = tool->v_orig;
	    tool->edit_node.vertex = &temp;

            if( tool->tracking_node.shape_id != -1 )
            {
                temp_tracking = temp;
                *(tool->tracking_node.vertex) = tool->v_orig_tracking;
                tool->tracking_node.vertex = &temp_tracking;
            }

	    tool->changing = TRUE;
            undo_group = gv_undo_start_group();

	    gv_shape_layer_move_node(tool->layer, &tool->edit_node);

            if( tool->tracking_node.shape_id != -1 )
            {
                gv_shape_layer_move_node(tool->layer, &tool->tracking_node);
                gv_shape_layer_get_node(tool->layer, &tool->tracking_node);
            }

	    gv_shape_layer_get_node(tool->layer, &tool->edit_node);

            gv_undo_end_group( undo_group );
	    tool->changing = FALSE;
	}
    }
    return FALSE;
}

static gboolean
gv_node_tool_motion_notify(GvTool *rtool, GdkEventMotion *event)
{
    GvNodeTool *tool = GV_NODE_TOOL(rtool);

    if (tool->dragging)
    {
	/* Drag selected node */
	/* Map pointer position to tail vertex */
	gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
				 &tool->v_tail.x, &tool->v_tail.y);

        gv_tool_clamp_to_bounds( GV_TOOL(tool), 
				 &tool->v_tail.x, &tool->v_tail.y );

	/* Move the node by dereferencing the vertex pointer (!).
	   We promise to put it back when the mouse button is released. */
	tool->edit_node.vertex->x = tool->v_orig.x + (tool->v_tail.x -
						      tool->v_head.x);
	tool->edit_node.vertex->y = tool->v_orig.y + (tool->v_tail.y -
						      tool->v_head.y);

        if( tool->tracking_node.shape_id != -1 )
            *(tool->tracking_node.vertex) = *(tool->edit_node.vertex);

	/* Inform layer of node motion */
	gv_shape_layer_node_motion(tool->layer, tool->edit_node.shape_id);
	
	gv_view_area_queue_draw(GV_TOOL(tool)->view);	
    }
    return FALSE;
}

static void
gv_node_tool_key_press(GvTool *rtool, GdkEventKey *event)
{
    GvNodeTool *tool = GV_NODE_TOOL(rtool);

    if (!gv_node_tool_configure(tool)) return;

    switch (event->keyval)
    {
	case GDK_Delete:
	case GDK_BackSpace:
        {
            gint undo_group;

            if( !tool->node_selected )
                return;

	    /* Delete the currently selected node (forces redraw) */
	    /* This also clears the node selection (via. *_layer_changed) */
          
            undo_group = gv_undo_start_group();
            if( tool->tracking_node.shape_id != -1 )
            {
                GvNodeInfo second_node;
                GvVertex  new_first_vert;

                second_node.shape_id = tool->edit_node.shape_id;
                second_node.ring_id = tool->edit_node.ring_id;
                second_node.node_id = 1;

                gv_shape_layer_get_node( tool->layer, &second_node );
                new_first_vert = *(second_node.vertex);

                tool->tracking_node.vertex = &new_first_vert;

                gv_shape_layer_move_node(tool->layer, &tool->tracking_node);
            }

	    gv_shape_layer_delete_node(tool->layer, &tool->edit_node);
            gv_undo_end_group( undo_group );

            tool->tracking_node.shape_id = -1;
            tool->node_selected = FALSE;
        }
        break;
    }
}

static gboolean
gv_node_tool_draw(GvTool *rtool)
{
    GvNodeTool *tool = GV_NODE_TOOL(rtool);

    if (tool->node_selected)
    {
	glPointSize(4.0);
	glBegin(GL_POINTS);
	glVertex2v((GLgeocoord*)tool->edit_node.vertex);
	glEnd();
    }
    return FALSE;
}

static void
gv_node_tool_deactivate(GvTool *rtool, GvViewArea *view)
{
    GvNodeTool *tool = GV_NODE_TOOL(rtool);

    /* Call the parent class func */
    GV_TOOL_DEACTIVATE(tool, view);

    if (tool->layer)
    {
	gv_shape_layer_clear_selection(tool->layer);
	gv_view_area_queue_draw(view);	
    }
    tool->node_selected = FALSE;
    tool->dragging = FALSE;
}

static void
gv_node_tool_layer_change(GvNodeTool *tool, gpointer info)
{
    if (!tool->changing)
    {
	/* Unexpected change in layer (or node delete) occured:
	   clear node selection. */
	tool->node_selected = FALSE;
	tool->dragging = FALSE;
    }
}

static gint
gv_node_tool_configure(GvNodeTool *tool)
{
    /* Check that we still are working on the active layer */
    if (!tool->layer ||	GTK_OBJECT(tool->layer) !=
	gv_view_area_active_layer(GV_TOOL(tool)->view))
    {
	GtkObject *layer;

	/* Attempt to find a line layer to edit */
	layer = gv_view_area_get_layer_of_type(GV_TOOL(tool)->view,
					       GV_TYPE_SHAPE_LAYER,
                                               FALSE);
	if (!layer)
	{
	    g_warning("gv_node_tool_configure(): no shape layer in view");
	    return FALSE;
	}

	gv_node_tool_set_layer(tool, GV_SHAPE_LAYER(layer));
    }

    return tool->layer != NULL;
}

#include "pics/node_cursor.xbm"
#include "pics/node_cursor_mask.xbm"

static GdkCursor *
gv_node_tool_make_cursor(void)
{
    GdkCursor *cursor;
    GdkPixmap *source, *mask;
    GdkColor fg, bg;

    gdk_color_parse("black", &fg);
    gdk_color_parse("white", &bg);    
    source = gdk_bitmap_create_from_data(NULL, node_cursor_bits,
					 node_cursor_width,
					 node_cursor_height);
    mask = gdk_bitmap_create_from_data(NULL, node_cursor_mask_bits,
				       node_cursor_mask_width,
				       node_cursor_mask_height);
    cursor = gdk_cursor_new_from_pixmap(source, mask, &fg, &bg,
					node_cursor_x_hot,
					node_cursor_y_hot);
    gdk_pixmap_unref(source);
    gdk_pixmap_unref(mask);

    return cursor;
}
