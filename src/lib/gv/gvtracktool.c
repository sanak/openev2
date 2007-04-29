/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Tracking display of raster values and position of cursor.
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

#include "gvtracktool.h"
#include "gvrasterlayer.h"
#include "gvutils.h"
#include "gvmanager.h"

#include <gtk/gtklabel.h>

static void gv_track_tool_class_init(GvTrackToolClass *klass);
static void gv_track_tool_init(GvTrackTool *tool);
static gboolean gv_track_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static gboolean gv_track_tool_leave_notify(GvTool *tool, GdkEventCrossing *event);
static void gv_track_tool_dispose(GObject *object);

static GvToolClass *parent_class = NULL;

GType
gv_track_tool_get_type(void)
{
    static GType track_tool_type = 0;

    if (!track_tool_type) {
        static const GTypeInfo track_tool_info =
        {
            sizeof(GvTrackToolClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_track_tool_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvTrackTool),
            0,
            (GInstanceInitFunc) gv_track_tool_init,
        };
        track_tool_type = g_type_register_static (GV_TYPE_TOOL,
                                                  "GvTrackTool",
                                                  &track_tool_info, 0);
        }

    return track_tool_type;
}

static void
gv_track_tool_class_init(GvTrackToolClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GvToolClass *tool_class = GV_TOOL_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    object_class->dispose = gv_track_tool_dispose;

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

    tool = g_object_new(GV_TYPE_TRACK_TOOL, NULL);
    tool->label = g_object_ref(label);

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
gv_track_tool_dispose(GObject *object)
{
    GvTrackTool *tool = GV_TRACK_TOOL(object);

    g_object_unref(tool->label);
    tool->label = NULL;

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->dispose(object);         
}
