/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Region of interest (box in raster coordinates) editing mode.
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

#ifndef __GV_ROI_TOOL_H__
#define __GV_ROI_TOOL_H__

#include "gvtypes.h"
#include "gvtool.h"

#define GV_TYPE_ROI_TOOL            (gv_roi_tool_get_type ())
#define GV_ROI_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_ROI_TOOL, GvRoiTool))
#define GV_ROI_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_ROI_TOOL, GvRoiToolClass))
#define GV_IS_ROI_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_ROI_TOOL))
#define GV_IS_ROI_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_ROI_TOOL))

typedef struct _GvRoiTool       GvRoiTool;
typedef struct _GvRoiToolClass  GvRoiToolClass;

struct _GvRoiTool
{
    GvTool tool;

    gint pick;

    gint roi_marked : 1;
    gint banding : 1;
    gint drag_right : 1;
    gint drag_bottom : 1;
    GvVertex v_head, v_tail, v_drag_offset;
};

struct _GvRoiToolClass
{
    GvToolClass parent_class;

    void (* roi_changed)(GvRoiTool *tool);
    void (* roi_changing)(GvRoiTool *tool);
};

GType gv_roi_tool_get_type(void);
GvTool* gv_roi_tool_new(void);

gint gv_roi_tool_get_rect(GvRoiTool *tool, GvRect *rect);
gint gv_roi_tool_new_rect(GvRoiTool *tool, GvRect *rect);

#endif /* __GV_ROI_TOOL_H__ */
