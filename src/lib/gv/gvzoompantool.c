/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Zoom/Pan editing mode (most zoompan code in gvviewarea.c)
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

#include "gvzoompantool.h"
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>

static void gv_zoompan_tool_class_init(GvZoompanToolClass *klass);
static void gv_zoompan_tool_init(GvZoompanTool *tool);
static gboolean gv_zoompan_tool_button_press(GvTool *tool, GdkEventButton *event);
static gboolean gv_zoompan_tool_button_release(GvTool *tool, GdkEventButton *event);
static gboolean gv_zoompan_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static void gv_zoompan_tool_deactivate(GvTool *tool, GvViewArea *view);

GType
gv_zoompan_tool_get_type(void)
{
    static GType zoompan_tool_type = 0;

    if (!zoompan_tool_type) {
        static const GTypeInfo zoompan_tool_info =
        {
            sizeof(GvZoompanToolClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_zoompan_tool_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvZoompanTool),
            0,
            (GInstanceInitFunc) gv_zoompan_tool_init,
        };
        zoompan_tool_type = g_type_register_static (GV_TYPE_TOOL,
                                                      "GvZoompanTool",
                                                      &zoompan_tool_info, 0);
        }

    return zoompan_tool_type;
}

static void
gv_zoompan_tool_class_init(GvZoompanToolClass *klass)
{
    GvToolClass *tool_class = GV_TOOL_CLASS (klass);

    tool_class->deactivate = gv_zoompan_tool_deactivate;
    tool_class->button_press = gv_zoompan_tool_button_press;
    tool_class->button_release = gv_zoompan_tool_button_release;
    tool_class->motion_notify = gv_zoompan_tool_motion_notify;
}

static void
gv_zoompan_tool_init(GvZoompanTool *tool)
{
}

GvTool *
gv_zoompan_tool_new(void)
{
    GvZoompanTool *tool = g_object_new(GV_TYPE_ZOOMPAN_TOOL, NULL);

    return GV_TOOL(tool);
}

static gboolean
gv_zoompan_tool_button_press(GvTool *tool, GdkEventButton *event)
{
    if( (event->state & GDK_CONTROL_MASK) 
        || (event->state & GDK_SHIFT_MASK) 
        || tool->view->flag_3d)
        return FALSE;

    gv_view_area_zoompan_event(tool->view, event);
    return FALSE;
}

static gboolean
gv_zoompan_tool_button_release(GvTool *tool, GdkEventButton *event)
{
    gv_view_area_zoompan_event( tool->view, event );
    return FALSE;
}

static gboolean
gv_zoompan_tool_motion_notify(GvTool *tool, GdkEventMotion *event)
{
    GvViewArea *view = tool->view;

    if( (event->state & GDK_BUTTON2_MASK)
        && !(event->state & (GDK_CONTROL_MASK|GDK_SHIFT_MASK)) )

    {
        gv_view_area_translate(view, event->x - view->state.mpos_x,
                               view->state.mpos_y - event->y);
    }

    /*
    if (event->state & GDK_BUTTON3_MASK)
    {
        gv_view_area_3d_direction_event(tool->view, event);
    }
    */

    return FALSE;
}

static void
gv_zoompan_tool_deactivate(GvTool *tool, GvViewArea *view)
{
    /* Call the parent class func */
    GV_TOOL_DEACTIVATE(tool, view);

    if( tool->view )
        tool->view->last_button = 0;
}
