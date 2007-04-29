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

#ifndef __GV_TRACK_TOOL_H__
#define __GV_TRACK_TOOL_H__

#include <gtk/gtkobject.h>
#include "gvtool.h"

#define GV_TYPE_TRACK_TOOL            (gv_track_tool_get_type ())
#define GV_TRACK_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_TRACK_TOOL, GvTrackTool))
#define GV_TRACK_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_TRACK_TOOL, GvTrackToolClass))
#define GV_IS_TRACK_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_TRACK_TOOL))
#define GV_IS_TRACK_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_TRACK_TOOL))

typedef struct _GvTrackTool       GvTrackTool;
typedef struct _GvTrackToolClass  GvTrackToolClass;

struct _GvTrackTool
{
    GvTool tool;

    GtkObject *label;
};

struct _GvTrackToolClass
{
    GvToolClass parent_class;
};

GType gv_track_tool_get_type(void);
GvTool* gv_track_tool_new(GtkObject *label);

#endif /* __GV_TRACK_TOOL_H__ */
