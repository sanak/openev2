/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Polyline editing mode. 
 * Author:   Frank Warmerdam, warmerda@home.com
 * Maintainer: Mario Beauchamp, starged@gmail.com
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
 */

#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include "gvlinetool.h"
#include "gvundo.h"
#include <GL/gl.h>

static void gv_line_tool_class_init(GvLineToolClass *klass);
static void gv_line_tool_init(GvLineTool *tool);
static gboolean gv_line_tool_draw(GvLineTool *tool);
static gboolean gv_line_tool_button_press(GvTool *tool, GdkEventButton *event);
static gboolean gv_line_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static void gv_line_tool_key_press(GvTool *tool, GdkEventKey *event);
static void gv_line_tool_deactivate(GvTool *tool, GvViewArea *view);
static void gv_line_tool_stop_drawing(GvLineTool *tool);
static gint gv_line_tool_configure(GvLineTool *tool);

GType
gv_line_tool_get_type(void)
{
    static GType line_tool_type = 0;

    if (!line_tool_type) {
        static const GTypeInfo line_tool_info =
        {
            sizeof(GvLineToolClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_line_tool_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvLineTool),
            0,
            (GInstanceInitFunc) gv_line_tool_init,
        };
        line_tool_type = g_type_register_static (GV_TYPE_TOOL,
                                                  "GvLineTool",
                                                  &line_tool_info, 0);
        }

    return line_tool_type;
}

static void
gv_line_tool_class_init(GvLineToolClass *klass)
{
    GvToolClass *tool_class = GV_TOOL_CLASS (klass);

    tool_class->deactivate = gv_line_tool_deactivate;

    tool_class->button_press = gv_line_tool_button_press;
    tool_class->motion_notify = gv_line_tool_motion_notify;
    tool_class->key_press = gv_line_tool_key_press;
}

static void
gv_line_tool_init(GvLineTool *tool)
{
    GV_TOOL(tool)->cursor = gdk_cursor_new(GDK_TCROSS);
    tool->layer = NULL;
    tool->named_layer = NULL;
    tool->drawing = FALSE;
}

GvTool *
gv_line_tool_new(void)
{
    GvLineTool *tool = g_object_new(GV_TYPE_LINE_TOOL, NULL);

    return GV_TOOL(tool);
}

static gint gv_line_tool_layer_destroy(GvTool *rtool)
{
    GvLineTool *tool = GV_LINE_TOOL(rtool);

    if (tool->layer)
        gv_line_tool_set_layer(tool, NULL);

    return 0;
}

void
gv_line_tool_set_layer(GvLineTool *tool, GvShapeLayer *layer)
{
    if (GV_TOOL(tool)->view == NULL)
    {
        g_warning("gv_line_tool_set_layer(): inactive tool");
        return;
    }

    if( layer != NULL && gv_data_is_read_only( GV_DATA(layer) ) )
    {
        g_warning( "gv_line_tool_set_layer(): layer is read-only" );
        return;
    }

    /* Disconnect from the previous layer (for draw) */
    if (tool->layer)
    {
        if (tool->drawing)
            gv_line_tool_stop_drawing(tool);

        gv_shape_layer_clear_selection(GV_SHAPE_LAYER(tool->layer));
        g_signal_handlers_disconnect_matched (tool->layer, G_SIGNAL_MATCH_DATA,
                                                0, 0, NULL, NULL, tool);
        gv_view_area_queue_draw(GV_TOOL(tool)->view);
    }

    tool->layer = layer;

    if (layer)
    {
        gv_view_area_set_active_layer(GV_TOOL(tool)->view, G_OBJECT(layer));

        /* Redraw when the layer draws */
        g_signal_connect_object(layer, "draw",
                                G_CALLBACK(gv_line_tool_draw),
                                GV_TOOL(tool), G_CONNECT_SWAPPED | G_CONNECT_AFTER);

        /* recover if layer destroyed */
        g_signal_connect_swapped(layer, "teardown",
                                G_CALLBACK(gv_line_tool_layer_destroy),
                                tool);
    }
}

void
gv_line_tool_set_named_layer(GvLineTool *tool, gchar *name)
{
    if (tool->named_layer)
    {
        g_free(tool->named_layer);
        tool->named_layer = NULL;
    }
    if (name)
        tool->named_layer = g_strdup(name); 
    /* Tool layer will be updated next time it is configured */
}

/********************************************************/

static gboolean
gv_line_tool_draw(GvLineTool *tool)
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
    return FALSE;
}

static gboolean
gv_line_tool_button_press(GvTool *r_tool, GdkEventButton *event)
{
    GvLineTool *tool = GV_LINE_TOOL(r_tool);
    GvShape *line;
    GvShapes *shapes;

    if (event->button == 1)
    {
        gint line_id;

        if (!gv_line_tool_configure(tool)) return FALSE;

        /* Map pointer position to tail vertex */
        gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
                                &tool->v_tail.x, &tool->v_tail.y);

        if (tool->drawing)
        {
            gv_tool_clamp_to_bounds( GV_TOOL(tool), 
                                    &(tool->v_tail.x), &(tool->v_tail.y) );

            /* Filter out duplicate verticies */
            if (tool->v_head.x == tool->v_tail.x && tool->v_head.y == tool->v_tail.y)
                return FALSE;

            /* Add a new vertex to the line */
            gv_shape_layer_selected(GV_SHAPE_LAYER(tool->layer), GV_FIRST, &line_id);
        }
        else
        {
            if (!gv_tool_check_bounds( GV_TOOL(tool), 
                                       tool->v_tail.x, tool->v_tail.y ))
                return FALSE;

            /* Start a new line */
            tool->drawing = TRUE;

            /* Close down undo.  A single operation describing the new
               line will be pushed to undo when drawing stops. */
            gv_undo_close();
            gv_undo_disable();

            line = gv_shape_new( GVSHAPE_LINE );

            line_id = gv_shapes_layer_select_new_shape( 
                    GV_SHAPES_LAYER(tool->layer), line );
        }

        tool->v_head = tool->v_tail;

        shapes = GV_SHAPES_LAYER(tool->layer)->data;
        line = gv_shapes_get_shape(shapes, line_id );

        if( gv_shape_type(line) == GVSHAPE_LINE )
            gv_shape_add_node(line, 0, tool->v_tail.x, tool->v_tail.y, 0);
        else
            g_warning( "selected object not line in gvlinetool mode!\n" );
    }
    else if (event->button == 3 && tool->drawing)
    {
        gv_line_tool_stop_drawing(tool);
    }

    return FALSE;
}

static gboolean
gv_line_tool_motion_notify(GvTool *r_tool, GdkEventMotion *event)
{
    GvLineTool *tool = GV_LINE_TOOL(r_tool);

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
gv_line_tool_key_press(GvTool *rtool, GdkEventKey *event)
{
    GvLineTool *tool = GV_LINE_TOOL(rtool);

    if (!gv_line_tool_configure(tool)) return;

    switch (event->keyval)
    {
        case GDK_Escape:
          gv_line_tool_stop_drawing(tool);
          break;

        case GDK_Delete:
        case GDK_BackSpace:
          if (tool->drawing)
          {
              gint line_id;
              GvNodeInfo  node_info;
              GvShape     *shape;
              GvShapes *shapes = GV_SHAPES_LAYER(tool->layer)->data;

              gv_shape_layer_selected(GV_SHAPE_LAYER(tool->layer), GV_FIRST,
                                      &line_id);

              shape = gv_shapes_get_shape( shapes, line_id );

              node_info.shape_id = line_id;
              node_info.ring_id = 0;
              node_info.node_id = gv_shape_get_nodes( shape, 0 ) - 1;

              if( gv_shape_get_nodes( shape, 0 ) > 1 )
              {
                  tool->v_head.x = gv_shape_get_x( shape, 0, 
                                                   node_info.node_id-1);
                  tool->v_head.y = gv_shape_get_y( shape, 0, 
                                                   node_info.node_id-1);
              }
              else
              {
                  tool->drawing = FALSE;
              }

              gv_shape_layer_delete_node(tool->layer, &node_info );

              if( !tool->drawing )
                  gv_line_tool_stop_drawing( tool );
          }
          break;
    }
}

static void
gv_line_tool_deactivate(GvTool *r_tool, GvViewArea *view)
{
    GvLineTool *tool = GV_LINE_TOOL(r_tool);

    /* Disconnect from layer */
    if (tool->layer)
        gv_line_tool_set_layer(tool, NULL);

    /* Call the parent class func */
    GV_TOOL_DEACTIVATE(tool, view);
}

static void
gv_line_tool_stop_drawing(GvLineTool *tool)
{
    gint sel, push_undo = TRUE;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 1, NULL};

    change_info.shape_id = &sel;

    tool->drawing = FALSE;

    /* Reject lines with only one node */
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(tool->layer), GV_FIRST, &sel))
    { 
        GvShapes   *shapes = GV_SHAPES_LAYER(tool->layer)->data;
        GvShape    *shape = gv_shapes_get_shape(shapes, sel);

        if( shape != NULL && gv_shape_type(shape) == GVSHAPE_LINE 
            && gv_shape_get_nodes(shape,0) < 2 )
        {
            gv_shape_layer_delete_selected(GV_SHAPE_LAYER(tool->layer));
            push_undo = FALSE;
        }
    }

    /* Reopen undo.  Push a memento describing the line addition. */
    gv_undo_enable();
    gv_undo_open();
    if (push_undo)
        gv_undo_push(gv_data_get_memento(GV_DATA(tool->layer), &change_info));

    gv_view_area_queue_draw(GV_TOOL(tool)->view);    
}

static gint
gv_line_tool_configure(GvLineTool *tool)
{
    /* Check that we still are working on the active layer */
    if (!tool->layer || G_OBJECT(tool->layer) !=
        gv_view_area_active_layer(GV_TOOL(tool)->view))
    {
        GObject *layer;

        if (tool->named_layer)
        {
            /* Look for named layer if given */
            layer = gv_view_area_get_named_layer(GV_TOOL(tool)->view,
                             tool->named_layer);
        }
        else
        {
            /* Attempt to find a line layer to edit */
            layer = gv_view_area_get_layer_of_type(GV_TOOL(tool)->view,
                                                   GV_TYPE_SHAPES_LAYER,
                                                   FALSE);
        }
        if (!layer)
        {
            g_warning("gv_line_tool_configure(): no editable line layer in view");
            return FALSE;
        }

        gv_line_tool_set_layer(tool, GV_SHAPE_LAYER(layer));
    }

    return tool->layer != NULL;
}
