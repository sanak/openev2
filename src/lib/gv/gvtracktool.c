/******************************************************************************
 * $Id: gvtracktool.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Tracking display of raster values and position of cursor.
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
 * $Log: gvtracktool.c,v $
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
 * Revision 1.8  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.7  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvtracktool.h"
#include "gvrasterlayer.h"
#include "gvutils.h"
#include "gvmanager.h"

#include <gtk/gtklabel.h>

static void gv_track_tool_class_init(GvTrackToolClass *klass);
static void gv_track_tool_init(GvTrackTool *tool);
static gboolean gv_track_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static gboolean gv_track_tool_leave_notify(GvTool *tool, GdkEventCrossing *event);
static void gv_track_tool_destroy(GtkObject *object);

GtkType
gv_track_tool_get_type(void)
{
    static GtkType track_tool_type = 0;

    if (!track_tool_type)
    {
	static const GtkTypeInfo track_tool_info =
	{
	    "GvTrackTool",
	    sizeof(GvTrackTool),
	    sizeof(GvTrackToolClass),
	    (GtkClassInitFunc) gv_track_tool_class_init,
	    (GtkObjectInitFunc) gv_track_tool_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	track_tool_type = gtk_type_unique(gv_tool_get_type(),
					  &track_tool_info);
    }
    return track_tool_type;
}

static void
gv_track_tool_class_init(GvTrackToolClass *klass)
{
    GtkObjectClass *object_class;
    GvToolClass *tool_class;

    object_class = (GtkObjectClass*)klass;
    tool_class = (GvToolClass*)klass;

    object_class->destroy = gv_track_tool_destroy;
    
    tool_class->motion_notify = gv_track_tool_motion_notify;
    tool_class->leave_notify = gv_track_tool_leave_notify;
}

static void
gv_track_tool_init(GvTrackTool *tool)
{
    tool->label = NULL;
}

GvTool *
gv_track_tool_new(GtkObject *label)
{
    GvTrackTool *tool;

    g_return_val_if_fail(GTK_IS_LABEL(label), NULL);

    tool = GV_TRACK_TOOL(gtk_type_new(GV_TYPE_TRACK_TOOL));
    tool->label = label;
    gtk_object_ref(label);

    return GV_TOOL(tool);
}

/********************************************************/

static gboolean
gv_track_tool_motion_notify(GvTool *tool_in, GdkEventMotion *event)
{
    GvTrackTool *tool = GV_TRACK_TOOL(tool_in);
    gvgeocoord geo_x, geo_y;
    const char  *text;
    GvProperties *properties = gv_manager_get_preferences( gv_get_manager() );

    gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
			     &geo_x, &geo_y);

    text = gv_view_area_format_point_query(GV_TOOL(tool)->view, 
                                           properties, geo_x, geo_y);
    gtk_label_set_text(GTK_LABEL(tool->label), text);

    return FALSE;
}

static gboolean
gv_track_tool_leave_notify(GvTool *tool, GdkEventCrossing *event)
{
    gtk_label_set_text(GTK_LABEL(GV_TRACK_TOOL(tool)->label), "");
    return FALSE;
}

static void
gv_track_tool_destroy(GtkObject *object)
{
    GvToolClass *parent_class;
    GvTrackTool *tool;

    tool = GV_TRACK_TOOL(object);

    gtk_object_unref(tool->label);
    tool->label = NULL;

    /* Call parent class function */
    parent_class = gtk_type_class(gv_tool_get_type());
    GTK_OBJECT_CLASS(parent_class)->destroy(object);         
}
