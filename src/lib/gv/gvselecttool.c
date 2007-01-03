/******************************************************************************
 * $Id: gvselecttool.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Shape selection tool.
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
 * $Log: gvselecttool.c,v $
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
 * Revision 1.17  2003/09/16 15:43:11  gmwalter
 * Add single selection mode to select tool, checked in developer tutorials.
 *
 * Revision 1.16  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.15  2002/01/18 04:53:28  warmerda
 * fixed shift-leftclick to add/subtract from selection
 *
 * Revision 1.14  2001/10/23 02:43:45  pgs
 * changed deactivate to call base class last
 *
 * Revision 1.13  2001/05/07 19:08:03  warmerda
 * draw text with origin off viewport properly
 *
 * Revision 1.12  2001/04/09 18:16:22  warmerda
 * honour read-only layers
 *
 * Revision 1.11  2000/08/23 16:48:00  warmerda
 * don't drag select box if control/shift pressed
 *
 * Revision 1.10  2000/08/10 18:33:46  warmerda
 * removed a warning
 *
 * Revision 1.9  2000/08/08 20:58:47  warmerda
 * recover from layer destruction
 *
 * Revision 1.8  2000/07/21 01:31:11  warmerda
 * added read_only flag for GvData, and utilize for vector layers
 *
 * Revision 1.7  2000/06/29 14:38:58  warmerda
 * don't emit a warning if gvselecttool configure without layers
 *
 * Revision 1.6  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include <string.h>
#include <stdio.h>
#include "gvselecttool.h"
#include <gtk/gtksignal.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>

static void gv_selection_tool_class_init(GvSelectionToolClass *klass);
static void gv_selection_tool_init(GvSelectionTool *tool);
static gboolean gv_selection_tool_draw(GvTool *tool);
static gboolean gv_selection_tool_button_press(GvTool *tool, GdkEventButton *event);
static gboolean gv_selection_tool_button_release(GvTool *tool, GdkEventButton *event);
static gboolean gv_selection_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static void gv_selection_tool_key_press(GvTool *tool, GdkEventKey *event);
static void gv_selection_tool_deactivate(GvTool *tool, GvViewArea *view);
static gint gv_selection_tool_configure(GvSelectionTool *tool);

GtkType
gv_selection_tool_get_type(void)
{
    static GtkType selection_tool_type = 0;

    if (!selection_tool_type)
    {
	static const GtkTypeInfo selection_tool_info =
	{
	    "GvSelectionTool",
	    sizeof(GvSelectionTool),
	    sizeof(GvSelectionToolClass),
	    (GtkClassInitFunc) gv_selection_tool_class_init,
	    (GtkObjectInitFunc) gv_selection_tool_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	selection_tool_type = gtk_type_unique(gv_tool_get_type(),
					      &selection_tool_info);
    }
    return selection_tool_type;
}

static void
gv_selection_tool_class_init(GvSelectionToolClass *klass)
{
    GvToolClass *tool_class;

    tool_class = (GvToolClass*)klass;
    tool_class->deactivate = gv_selection_tool_deactivate;
    tool_class->button_press = gv_selection_tool_button_press;
    tool_class->button_release = gv_selection_tool_button_release;
    tool_class->motion_notify = gv_selection_tool_motion_notify;
    tool_class->key_press = gv_selection_tool_key_press;
}
static void
gv_selection_tool_init(GvSelectionTool *tool)
{
    tool->layer = NULL;
    tool->banding = FALSE;
    tool->dragging = FALSE;
}

GvTool *
gv_selection_tool_new(void)
{
    return GV_TOOL(gtk_type_new(GV_TYPE_SELECTION_TOOL));
}

static gint gv_selection_tool_layer_destroy( GtkObject *layer, gpointer data )

{
    GvSelectionTool *tool = (GvSelectionTool *) data;

    if( tool->layer == GV_SHAPE_LAYER(layer) )
        gv_selection_tool_set_layer( tool, NULL );

    return 0;
}

void
gv_selection_tool_set_layer(GvSelectionTool *tool, GvShapeLayer *layer)
{
    if (GV_TOOL(tool)->view == NULL)
    {
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
					GTK_SIGNAL_FUNC(gv_selection_tool_draw),
					GTK_OBJECT(tool));

        /* Recover if layer is destroyed */
        gtk_signal_connect(
            GTK_OBJECT(layer), "destroy",
            GTK_SIGNAL_FUNC(gv_selection_tool_layer_destroy),
            GTK_OBJECT(tool));
    }
}

/**************************************************************/

static gboolean
gv_selection_tool_button_press(GvTool *rtool, GdkEventButton *event)
{
    GvSelectionTool *tool = GV_SELECTION_TOOL(rtool);

    /* ignore control corded buttons -- these are for zooming and panning */
    if( event->state & GDK_CONTROL_MASK )
        return FALSE;

    if (event->button == 1)
    {
	gint shape_id;

	if (!gv_selection_tool_configure(tool)) return FALSE;

	if (gv_shape_layer_pick_shape(tool->layer, GV_TOOL(tool)->view,
				      event->x, event->y, &shape_id))
	{
	    if (!(event->state & GDK_MOD1_MASK) && (event->state & GDK_SHIFT_MASK) &&
                ((gv_data_get_property( GV_DATA(tool->layer),
                 "selection_mode" ) == NULL) ||
                (strcmp(gv_data_get_property( GV_DATA(tool->layer),
                 "selection_mode" ),"single") != 0)))
	    {
		/* Shift click (de)selects without clearing other selections */
		if (gv_shape_layer_is_selected(tool->layer, shape_id))
		{
		    gv_shape_layer_deselect_shape(tool->layer, shape_id);
		}
		else
		{
		    gv_shape_layer_select_shape(tool->layer, shape_id);
		}
	    }
	    else if ((event->state & GDK_MOD1_MASK) || !(event->state & GDK_SHIFT_MASK))
	    {
		/* Non-shift click */
		if (!gv_shape_layer_is_selected(tool->layer, shape_id))
		{
		    /* Select this shape (and only this shape) */
		    gv_shape_layer_clear_selection(tool->layer);
		    gv_shape_layer_select_shape(tool->layer, shape_id);
		}

                if( !gv_data_is_read_only( GV_DATA(tool->layer) ) )
                {
                    /* Start a drag operation */
                    tool->dragging = TRUE;

                    /* Capture pointer position */
                    gv_view_area_map_pointer(GV_TOOL(tool)->view,
                                             event->x, event->y,
                                             &tool->v_tail.x, &tool->v_tail.y);
                    tool->v_head = tool->v_tail;

                    if (event->state & GDK_MOD1_MASK) {
                        tool->layer->flags |= GV_ALT_SELECTED;
		    }

                    /* Delay drawing the selected shapes */
                    gv_shape_layer_draw_selected(tool->layer, GV_LATER, NULL);
                }
	    }
	}
        else if ((event->state & (GDK_CONTROL_MASK|GDK_SHIFT_MASK)) == 0 )
        {
            if ((gv_data_get_property( GV_DATA(tool->layer),
                 "selection_mode" ) != NULL) &&
                (strcmp(gv_data_get_property( GV_DATA(tool->layer),
                         "selection_mode" ),"single") == 0))
            {
  	        gv_shape_layer_clear_selection(tool->layer);
            }
  	    else 
	    {
	        /* Begin rubber band */
	        gv_shape_layer_clear_selection(tool->layer);

	        /* Capture pointer position */
	        tool->v_tail.x = event->x - GV_TOOL(tool)->view->state.tx;
	        tool->v_tail.y = (GV_TOOL(tool)->view->state.shape_y - 
                                  event->y) - GV_TOOL(tool)->view->state.ty;
	        tool->v_head = tool->v_tail;

  	        tool->banding = TRUE;
	    }
        }
  	gv_view_area_queue_draw(GV_TOOL(tool)->view);
    }
    return FALSE;
}

static gboolean
gv_selection_tool_button_release(GvTool *rtool, GdkEventButton *event)
{
    GvSelectionTool *tool = GV_SELECTION_TOOL(rtool);

    if (tool->banding && event->button == 1)
    {
	/* End rubber band */
	gvgeocoord ax, ay, bx, by;
	GvRect rect;
	GvViewAreaState *state = &GV_TOOL(tool)->view->state;

	tool->banding = FALSE;

	/* Get view center coords for rubber band */
	ax = tool->v_head.x + state->tx - state->shape_x/2.0;
	ay = tool->v_head.y + state->ty - state->shape_y/2.0;
	bx = event->x - state->shape_x/2.0;
	by = state->shape_y/2.0 - event->y;

	rect.x = MIN(ax, bx);
	rect.y = MIN(ay, by);
	rect.width = ABS(ax - bx);
	rect.height = ABS(ay - by);

	/* Pick shapes in region */
	gv_shape_layer_select_region(tool->layer, GV_TOOL(tool)->view, &rect);
	gv_view_area_queue_draw(GV_TOOL(tool)->view);
    }
    else if (tool->dragging && event->button == 1)
    {
	/* End dragging */
	tool->dragging = FALSE;
	gv_shape_layer_draw_selected(tool->layer, GV_ALWAYS, NULL);

	gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
				 &tool->v_tail.x, &tool->v_tail.y);

	/* Filter out clicks that wern't really drags */
	if (tool->v_tail.x != tool->v_head.x ||
	    tool->v_tail.y != tool->v_head.y)
	{

	    /* Translate shapes */
	    gv_shape_layer_translate_selected(tool->layer,
					      tool->v_tail.x - tool->v_head.x,
					      tool->v_tail.y - tool->v_head.y);
	}
    }

    /* ---- Ensure alt selection flag turned off ---- */
    if ((tool->layer != NULL) && (tool->layer->flags & GV_ALT_SELECTED)) {
        tool->layer->flags &= ~GV_ALT_SELECTED;
    }

    return FALSE;
}

static gboolean
gv_selection_tool_motion_notify(GvTool *rtool, GdkEventMotion *event)
{
    GvSelectionTool *tool = GV_SELECTION_TOOL(rtool);

    if (tool->banding)
    {
	/* Resize rubber band */
	/* Capture pointer position to tail vertex */
	tool->v_tail.x = event->x - GV_TOOL(tool)->view->state.tx;
	tool->v_tail.y = (GV_TOOL(tool)->view->state.shape_y - event->y) -
	    GV_TOOL(tool)->view->state.ty;

	gv_view_area_queue_draw(GV_TOOL(tool)->view);
    }
    else if (tool->dragging)
    {
	/* Drag selected shapes */
	/* Map pointer position to tail vertex */
	gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
				 &tool->v_tail.x, &tool->v_tail.y);
	gv_view_area_queue_draw(GV_TOOL(tool)->view);
    }
    return FALSE;
}

static void
gv_selection_tool_key_press(GvTool *rtool, GdkEventKey *event)
{
    GvSelectionTool *tool = GV_SELECTION_TOOL(rtool);

    if (!gv_selection_tool_configure(tool)) return;

    switch (event->keyval)
    {
      case GDK_Delete:
      case GDK_BackSpace:
        /* Delete the currently selected lines (forces redraw) */
        if( !gv_data_is_read_only( GV_DATA(tool->layer) ) )
            gv_shape_layer_delete_selected(tool->layer);
        break;
    }
}

static gboolean
gv_selection_tool_draw(GvTool *rtool)
{
    GvSelectionTool *tool = GV_SELECTION_TOOL(rtool);

    if (tool->banding)
    {
	/* Resolve head and tail positions in view coordinates */
	GvViewAreaState *state = &GV_TOOL(tool)->view->state;

	/* Draw the rubber band rectangle */
	glColor3f(1.0, 0.0, 0.0);
	glPushMatrix();
	glLoadIdentity();
	glTranslate(state->tx - state->shape_x/2.0,
		     state->ty - state->shape_y/2.0, 0.0);
	glBegin(GL_LINE_LOOP);
	glVertex2(tool->v_head.x, tool->v_head.y);
	glVertex2(tool->v_head.x, tool->v_tail.y);
	glVertex2(tool->v_tail.x, tool->v_tail.y);
	glVertex2(tool->v_tail.x, tool->v_head.y);
	glEnd();
	glPopMatrix();
    }
    else if (tool->dragging)
    {
	gvgeocoord dx, dy;

	/* Inform layer of shape motion */
	dx = tool->v_tail.x - tool->v_head.x;
	dy = tool->v_tail.y - tool->v_head.y;
	gv_shape_layer_selected_motion(tool->layer, dx, dy);

	/* Draw selected objects translated */
	glPushMatrix();
	glTranslate(dx, dy, 0.0);
	gv_shape_layer_draw_selected(tool->layer, GV_NOW,
				     GV_TOOL(tool)->view);
	glPopMatrix();
	gv_shape_layer_selected_motion(tool->layer, 0.0, 0.0 );
    }
    return FALSE;
}

static void
gv_selection_tool_deactivate(GvTool *rtool, GvViewArea *view)
{
    GvSelectionTool *tool = GV_SELECTION_TOOL(rtool);


    if (tool->layer)
    {
	if (tool->dragging)
	{
	    gv_shape_layer_draw_selected(tool->layer, GV_ALWAYS, NULL);
	}
	gv_shape_layer_clear_selection(tool->layer);
	gv_view_area_queue_draw(view);
        gv_selection_tool_set_layer(tool, NULL);
    }
    tool->banding = FALSE;
    tool->dragging = FALSE;

    /* Call the parent class func */
    GV_TOOL_DEACTIVATE(tool, view);
}

static gint
gv_selection_tool_configure(GvSelectionTool *tool)
{
    /* Check that we still are working on the active layer */
    if (!tool->layer ||	GTK_OBJECT(tool->layer) !=
	gv_view_area_active_layer(GV_TOOL(tool)->view))
    {
	GtkObject *layer;

	/* Attempt to find a line layer to edit */
	layer = gv_view_area_get_layer_of_type(GV_TOOL(tool)->view,
					       GV_TYPE_SHAPE_LAYER,
                                               TRUE);
	if (!layer)
	    return FALSE;

	gv_selection_tool_set_layer(tool, GV_SHAPE_LAYER(layer));
    }
    return TRUE;
}
