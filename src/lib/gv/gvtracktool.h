/******************************************************************************
 * $Id: gvtracktool.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
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
 * $Log: gvtracktool.h,v $
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
 * Revision 1.2  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_TRACK_TOOL_H__
#define __GV_TRACK_TOOL_H__

#include "gvtool.h"

#define GV_TYPE_TRACK_TOOL            (gv_track_tool_get_type ())
#define GV_TRACK_TOOL(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_TRACK_TOOL, GvTrackTool))
#define GV_TRACK_TOOL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_TRACK_TOOL, GvTrackToolClass))
#define GV_IS_TRACK_TOOL(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_TRACK_TOOL))
#define GV_IS_TRACK_TOOL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_TRACK_TOOL))

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

GtkType gv_track_tool_get_type(void);
GvTool* gv_track_tool_new(GtkObject *label);

#endif /* __GV_TRACK_TOOL_H__ */
