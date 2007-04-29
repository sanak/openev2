/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Base class for editing mode tools.
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

#include "gvtool.h"

enum
{
    ACTIVATE,
    DEACTIVATE,
    DRAW,
    BUTTON_PRESS,
    BUTTON_RELEASE,
    MOTION_NOTIFY,
    KEY_PRESS,
    ENTER_NOTIFY,
    LEAVE_NOTIFY,
    LAST_SIGNAL
};

static void gv_tool_class_init(GvToolClass *klass);
static void gv_tool_init(GvTool *tool);
static void gv_tool_real_activate(GvTool *tool, GvViewArea *area);
static void gv_tool_real_deactivate(GvTool *tool, GvViewArea *area);

static guint tool_signals[LAST_SIGNAL] = { 0 };

GType
gv_tool_get_type(void)
{
    static GType tool_type = 0;

    if (!tool_type) {
        static const GTypeInfo tool_info =
        {
            sizeof(GvToolClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_tool_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvTool),
            0,
            (GInstanceInitFunc) gv_tool_init,
        };
        tool_type = g_type_register_static (G_TYPE_OBJECT,
                                          "GvTool",
                                          &tool_info, 0);
        }

    return tool_type;
}

static void
gv_tool_class_init(GvToolClass *klass)
{
    tool_signals[ACTIVATE] =
      g_signal_new ("activate",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvToolClass, activate),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);
    tool_signals[DEACTIVATE] =
      g_signal_new ("deactivate",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvToolClass, deactivate),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);
    tool_signals[DRAW] =
      g_signal_new ("draw",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvToolClass, draw),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
    tool_signals[BUTTON_PRESS] =
      g_signal_new ("button-press",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvToolClass, button_press),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);
    tool_signals[BUTTON_RELEASE] =
      g_signal_new ("button-release",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvToolClass, button_release),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);
    tool_signals[MOTION_NOTIFY] =
      g_signal_new ("motion-notify",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvToolClass, motion_notify),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);
    tool_signals[KEY_PRESS] =
      g_signal_new ("key-press",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvToolClass, key_press),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);
    tool_signals[ENTER_NOTIFY] =
      g_signal_new ("enter-notify",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvToolClass, enter_notify),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);
    tool_signals[LEAVE_NOTIFY] =
      g_signal_new ("leave-notify",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvToolClass, leave_notify),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);

    klass->activate = gv_tool_real_activate;
    klass->deactivate = gv_tool_real_deactivate;
    klass->draw = NULL;
    klass->button_press = NULL;
    klass->button_release = NULL;
    klass->motion_notify = NULL;
    klass->key_press = NULL;
    klass->enter_notify = NULL;
    klass->leave_notify = NULL;
}

static void
gv_tool_init(GvTool *tool)
{
    tool->view = NULL;
    tool->cursor = NULL;
    tool->bounded = 0;
}

void
gv_tool_activate(GvTool *tool, GvViewArea *view)
{
    g_signal_emit(tool, tool_signals[ACTIVATE], 0, view);
}

void
gv_tool_deactivate(GvTool *tool, GvViewArea *view)
{
    g_signal_emit(tool, tool_signals[DEACTIVATE], 0, view);
}

/**************************************************************/

static void
gv_tool_real_activate(GvTool *tool, GvViewArea *view)
{
    GvToolClass *klass;

    if (tool->view)
    {
        g_warning("gv_tool_activate(): tool %s is already active on a view",
              g_type_name (G_TYPE_FROM_INSTANCE (tool)));
        return;
    }
    /* let´s assume for now that gvtoolbox will own the ref to the view */
    tool->view = view;
    klass = GV_TOOL_CLASS (G_OBJECT_GET_CLASS(tool));

    /* This could be done through an indirect function call which
       emits a tool signal.  Probably better but more overhead... */

    if (klass->draw)
    {
        g_signal_connect_swapped(view, "gldraw",
                                G_CALLBACK(klass->draw),
                                tool);      
    }

    if (klass->button_press)
    {
        g_signal_connect_swapped(view, "button_press_event",
                                G_CALLBACK(klass->button_press),
                                tool);
    }

    if (klass->button_release)
    {
        g_signal_connect_swapped(view, "button_release_event",
                                G_CALLBACK(klass->button_release),
                                tool);
    }

    if (klass->motion_notify)
    {
        g_signal_connect_swapped(view, "motion_notify_event",
                                G_CALLBACK(klass->motion_notify),
                                tool);
    }

    if (klass->key_press)
    {
        g_signal_connect_swapped(view, "key_press_event",
                                G_CALLBACK(klass->key_press),
                                tool);
    }

    if (klass->enter_notify)
    {
        g_signal_connect_swapped(view, "enter_notify_event",
                                G_CALLBACK(klass->enter_notify),
                                tool);
    }

    if (klass->leave_notify)
    {
        g_signal_connect_swapped(view, "leave_notify_event",
                                G_CALLBACK(klass->leave_notify),
                                tool);
    }

    /* Install cursor for this tool */
    if (GTK_WIDGET_REALIZED(GTK_WIDGET(view)))
        gdk_window_set_cursor(GTK_WIDGET(view)->window, tool->cursor);
}

static void
gv_tool_real_deactivate(GvTool *tool, GvViewArea *view)
{
    g_return_if_fail(tool->view == view);

    g_signal_handlers_disconnect_matched (tool->view, G_SIGNAL_MATCH_DATA,
                                            0, 0, NULL, NULL, tool);
    /* let´s assume for now that gvtoolbox will own the ref to the view 
    g_object_unref(view);*/
    tool->view = NULL;
}

GvViewArea *gv_tool_get_view(GvTool *tool)
{
    return tool->view;
}

gint
gv_tool_set_boundary(GvTool *tool, GvRect *rect)
{
    if( rect == NULL )
    {
        tool->bounded = FALSE;
        return TRUE;
    }

    /* check rect is valid */
    if (rect->width <= 0 || rect->height <= 0)
    {
        return FALSE;
    }

    tool->bounded = TRUE;
    tool->boundary = *rect;

    return TRUE;
}

void gv_tool_clamp_to_bounds( GvTool *tool, gvgeocoord *x, gvgeocoord *y )
{
    if( tool->bounded )
    {
        *x = MAX(MIN(*x,tool->boundary.x+tool->boundary.width),
                tool->boundary.x);
        *y = MAX(MIN(*y,tool->boundary.y+tool->boundary.height),
                tool->boundary.y);
    }
}

gint gv_tool_check_bounds( GvTool *tool, gvgeocoord x, gvgeocoord y )
{
    if( !tool->bounded )
        return TRUE;

    if( x < tool->boundary.x || x > tool->boundary.x + tool->boundary.width )
        return FALSE;

    if( y < tool->boundary.y || y > tool->boundary.y + tool->boundary.height )
        return FALSE;

    return TRUE;
}

void
gv_tool_set_cursor(GvTool *tool, gint cursor_type)
{
    if (tool->cursor != NULL)
        gdk_cursor_destroy(tool->cursor);

    tool->cursor = gdk_cursor_new(cursor_type);

    if ((tool->view != NULL) && (GTK_WIDGET_REALIZED(GTK_WIDGET(tool->view))))
    {
        gdk_window_set_cursor(GTK_WIDGET(tool->view)->window, tool->cursor);
    }
}
