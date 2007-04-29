/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Area (Polygon) editing mode.
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
#include "gvareatool.h"
#include "gvundo.h"
#include "gvshapeslayer.h"
#include <GL/gl.h>

static void gv_area_tool_class_init(GvAreaToolClass *klass);
static void gv_area_tool_init(GvAreaTool *tool);
static void gv_area_tool_draw(GvAreaTool *tool);
static gboolean gv_area_tool_button_press(GvTool *tool, GdkEventButton *event);
static gboolean gv_area_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static void gv_area_tool_key_press(GvTool *tool, GdkEventKey *event);
static void gv_area_tool_deactivate(GvTool *tool, GvViewArea *view);
static void gv_area_tool_stop_drawing(GvAreaTool *tool);
static gint gv_area_tool_configure(GvAreaTool *tool);

GType
gv_area_tool_get_type(void)
{
    static GType area_tool_type = 0;

    if (!area_tool_type) {
        static const GTypeInfo area_tool_info =
        {
            sizeof(GvAreaToolClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_area_tool_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvAreaTool),
            0,
            (GInstanceInitFunc) gv_area_tool_init,
        };
        area_tool_type = g_type_register_static (GV_TYPE_TOOL,
                                                  "GvAreaTool",
                                                  &area_tool_info, 0);
        }

    return area_tool_type;
}

static void
gv_area_tool_class_init(GvAreaToolClass *klass)
{
    GvToolClass *tool_class = GV_TOOL_CLASS (klass);

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
    GvAreaTool *tool = g_object_new(GV_TYPE_AREA_TOOL, NULL);

    return GV_TOOL(tool);
}

static gint gv_area_tool_layer_destroy(GvTool *rtool)

{
    GvAreaTool *tool = GV_AREA_TOOL(rtool);

    if (tool->layer)
        gv_area_tool_set_layer(tool, NULL);

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
            gv_area_tool_stop_drawing(tool);

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
                                G_CALLBACK(gv_area_tool_draw),
                                GV_TOOL(tool), G_CONNECT_SWAPPED | G_CONNECT_AFTER);

        /* recover if layer destroyed */
        g_signal_connect_swapped(layer, "teardown",
                                G_CALLBACK(gv_area_tool_layer_destroy),
                                tool);
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
        tool->named_layer = g_strdup(name);     
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

static gboolean
gv_area_tool_button_press(GvTool *rtool, GdkEventButton *event)
{
    GvAreaTool *tool = GV_AREA_TOOL(rtool);
    GvShapes *shapes;
    GvShape *area;

    /* ignore control corded buttons -- these are for zooming and panning */
    if( event->state & GDK_CONTROL_MASK )
        return FALSE;

    if (event->button == 1)
    {
        gint area_id = -1;

        if (!gv_area_tool_configure(tool)) return FALSE;

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
                return FALSE;
            }

            /* Add a new vertex to the area */
            gv_shape_layer_selected(GV_SHAPE_LAYER(tool->layer), GV_FIRST,
                                    &area_id);
            if( area_id == -1 )
            {
                g_warning( "gv_area_tool_button_press: get selection failed.");
                return FALSE;
            }
        }
        else
        {
            if( !gv_tool_check_bounds( GV_TOOL(tool), 
                                       tool->v_tail.x, tool->v_tail.y ) )
                return FALSE;

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
            GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE,1,NULL};

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
            else
            {
            /* Start a new area */
            area = gv_shape_new( GVSHAPE_AREA );

            area_id = gv_shapes_layer_select_new_shape( 
                GV_SHAPES_LAYER(tool->layer), area );
            tool->ring_id = 0;
            }
        }

        tool->v_head = tool->v_tail;
        shapes = GV_SHAPES_LAYER(tool->layer)->data;
        area = gv_shapes_get_shape(shapes, area_id );

        if( gv_shape_type(area) == GVSHAPE_AREA )
        {
            gv_shape_add_node(area, tool->ring_id, 
                              tool->v_tail.x, tool->v_tail.y, 0);
            ((GvAreaShape *) area)->fill_objects = -2;
        }
        else
            g_warning( "selected object not area in gvareatool mode!\n" );
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
          if( tool->drawing )
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
        gv_area_tool_set_layer(tool, NULL);

    /* Call the parent class func */
    GV_TOOL_DEACTIVATE(tool, view);
}

static void
gv_area_tool_stop_drawing(GvAreaTool *tool)
{
    gint sel, push_undo = TRUE;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 1, NULL};

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

    gv_view_area_queue_draw(GV_TOOL(tool)->view);       
}

static gint
gv_area_tool_configure(GvAreaTool *tool)
{
    /* Check that we still are working on the active layer */
    if (!tool->layer || G_OBJECT(tool->layer) !=
        gv_view_area_active_layer(GV_TOOL(tool)->view))
    {
        GObject *layer;

        if (tool->named_layer)
            /* Look for named layer if given */
            layer = gv_view_area_get_named_layer(GV_TOOL(tool)->view,
                                                 tool->named_layer);
        else
            /* Attempt to find a area layer to edit */
            layer = gv_view_area_get_layer_of_type(GV_TOOL(tool)->view,
                                                    GV_TYPE_SHAPES_LAYER,
                                                    FALSE);
        if (!layer)
        {
            g_warning("gv_area_tool_configure(): no editable area layer in view");
            return FALSE;
        }

        gv_area_tool_set_layer(tool, GV_SHAPE_LAYER(layer));
    }

    return tool->layer != NULL;
}
