/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Rectangle (in GvShapesLayer) editing mode.
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

#ifndef __GV_RECT_TOOL_H__
#define __GV_RECT_TOOL_H__

#include "gvtool.h"
#include "gvshapeslayer.h"

#define GV_TYPE_RECT_TOOL            (gv_rect_tool_get_type ())
#define GV_RECT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_RECT_TOOL, GvRectTool))
#define GV_RECT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_RECT_TOOL, GvRectToolClass))
#define GV_IS_RECT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_RECT_TOOL))
#define GV_IS_RECT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_RECT_TOOL))

typedef struct _GvRectTool       GvRectTool;
typedef struct _GvRectToolClass  GvRectToolClass;

struct _GvRectTool
{
    GvTool tool;

    GvShapesLayer *layer;
    gchar *named_layer;

    guint drawing : 1;
    GvVertex v_head, v_tail;

    guint reshaping : 1;
    int   picked;   /* which corner or edge */
    int   shape_id; /* shapeid of rectangle being reshaped */
    int   winding;  /* 1 = counterclockwise, 0 = clockwise */

};

struct _GvRectToolClass
{
    GvToolClass parent_class;
};

GType gv_rect_tool_get_type(void);
GvTool* gv_rect_tool_new(void);
void gv_rect_tool_set_layer(GvRectTool *tool, GvShapeLayer *layer);
void gv_rect_tool_set_named_layer(GvRectTool *tool, gchar *name);

#endif /* __GV_RECT_TOOL_H__ */
