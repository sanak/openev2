/******************************************************************************
 * $Id: gvzoompantool.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Zoom/Pan editing mode (most zoompan code in gvviewarea.c)
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
 * $Log: gvzoompantool.c,v $
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
 * Revision 1.7  2000/07/10 13:37:05  srawlin
 * updated 3D controls to be more like 2D
 *
 * Revision 1.6  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvzoompantool.h"
#include <gtk/gtksignal.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>

static void gv_zoompan_tool_class_init(GvZoompanToolClass *klass);
static void gv_zoompan_tool_init(GvZoompanTool *tool);
static gboolean gv_zoompan_tool_button_press(GvTool *tool, GdkEventButton *event);
static gboolean gv_zoompan_tool_button_release(GvTool *tool, GdkEventButton *event);
static gboolean gv_zoompan_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static void gv_zoompan_tool_deactivate(GvTool *tool, GvViewArea *view);

GtkType
gv_zoompan_tool_get_type(void)
{
    static GtkType zoompan_tool_type = 0;

    if (!zoompan_tool_type)
    {
	static const GtkTypeInfo zoompan_tool_info =
	{
	    "GvZoompanTool",
	    sizeof(GvZoompanTool),
	    sizeof(GvZoompanToolClass),
	    (GtkClassInitFunc) gv_zoompan_tool_class_init,
	    (GtkObjectInitFunc) gv_zoompan_tool_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	zoompan_tool_type = gtk_type_unique(gv_tool_get_type(),
					      &zoompan_tool_info);
    }
    return zoompan_tool_type;
}

static void
gv_zoompan_tool_class_init(GvZoompanToolClass *klass)
{
    GvToolClass *tool_class;

    tool_class = (GvToolClass*)klass;
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
    return GV_TOOL(gtk_type_new(GV_TYPE_ZOOMPAN_TOOL));
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


