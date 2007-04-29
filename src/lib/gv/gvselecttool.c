/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Shape selection tool.
 * Author:   OpenEV Team
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

#include <string.h>
#include <stdio.h>
#include "gvselecttool.h"
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

static GvToolClass *parent_class = NULL;

GType
gv_selection_tool_get_type(void)
{
    static GType selection_tool_type = 0;

    if (!selection_tool_type) {
        static const GTypeInfo selection_tool_info =
        {
            sizeof(GvSelectionToolClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_selection_tool_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvSelectionTool),
            0,
            (GInstanceInitFunc) gv_selection_tool_init,
        };
        selection_tool_type = g_type_register_static (GV_TYPE_TOOL,
                                                      "GvSelectionTool",
                                                      &selection_tool_info, 0);
        }

    return selection_tool_type;
}

static void
gv_selection_tool_class_init(GvSelectionToolClass *klass)
{
    GvToolClass *tool_class = GV_TOOL_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

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
    GvSelectionTool *tool = g_object_new(GV_TYPE_SELECTION_TOOL, NULL);

    return GV_TOOL(tool);
}

static gint gv_selection_tool_layer_destroy(GvTool *rtool)
{
    GvSelectionTool *tool = GV_SELECTION_TOOL(rtool);

    if (tool->layer)
        gv_selection_tool_set_layer(tool, NULL);

    return 0;
}

void
gv_selection_tool_set_layer(GvSelectionTool *tool, GvShapeLayer *layer)
{
    if (GV_TOOL(tool)->view == NULL)
    {
        g_warning("gv_selection_tool_set_layer(): inactive tool");
        return;
    }

    /* Disconnect from the previous layer (for draw) */
    if (tool->layer)
    {
        gv_shape_layer_clear_selection(GV_SHAPE_LAYER(tool->layer));
        gv_view_area_queue_draw(GV_TOOL(tool)->view);
        g_signal_handlers_disconnect_matched (tool->layer, G_SIGNAL_MATCH_DATA,
                                                0, 0, NULL, NULL, tool);
    }

    tool->layer = layer;

    if (layer)
    {
        /*g_object_ref(tool->layer);*/
        gv_view_area_set_active_layer(GV_TOOL(tool)->view, G_OBJECT(layer));

        /* Redraw when the layer draws */
        g_signal_connect_object(layer, "draw",
                                G_CALLBACK(gv_selection_tool_draw),
                                GV_TOOL(tool), G_CONNECT_SWAPPED | G_CONNECT_AFTER);

        /* Recover if layer is destroyed */
        g_signal_connect_swapped(layer, "teardown",
                        G_CALLBACK(gv_selection_tool_layer_destroy),
                        GV_TOOL(tool));
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
    if (!tool->layer || (G_OBJECT(tool->layer) !=
        gv_view_area_active_layer(GV_TOOL(tool)->view)))
    {
        GObject *layer;

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
