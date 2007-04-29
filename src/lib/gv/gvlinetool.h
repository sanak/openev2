/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Polyline editing mode. 
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

#ifndef __GV_LINE_TOOL_H__
#define __GV_LINE_TOOL_H__

#include "gvtool.h"
#include "gvshapeslayer.h"

#define GV_TYPE_LINE_TOOL            (gv_line_tool_get_type ())
#define GV_LINE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_LINE_TOOL, GvLineTool))
#define GV_LINE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_LINE_TOOL, GvLineToolClass))
#define GV_IS_LINE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_LINE_TOOL))
#define GV_IS_LINE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_LINE_TOOL))

typedef struct _GvLineTool       GvLineTool;
typedef struct _GvLineToolClass  GvLineToolClass;

struct _GvLineTool
{
    GvTool tool;

    GvShapeLayer *layer;
    gchar *named_layer;
    guint drawing : 1;
    GvVertex v_head, v_tail;
};

struct _GvLineToolClass
{
    GvToolClass parent_class;
};

GType gv_line_tool_get_type(void);
GvTool* gv_line_tool_new(void);
void gv_line_tool_set_layer(GvLineTool *tool, GvShapeLayer *layer);
void gv_line_tool_set_named_layer(GvLineTool *tool, gchar *name);

#endif /* __GV_LINE_TOOL_H__ */
