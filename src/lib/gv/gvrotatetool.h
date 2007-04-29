/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Rotation and Scaling editing mode.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 * Maintainer: Mario Beauchamp, starged@gmail.com
 *
 ******************************************************************************
 * Copyright (c) 2003, Frank Warmerdam <warmerdam@pobox.com>
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

#ifndef __GV_ROTATE_TOOL_H__
#define __GV_ROTATE_TOOL_H__

#include "gvtool.h"
#include "gvshapeslayer.h"

#define GV_TYPE_ROTATE_TOOL            (gv_rotate_tool_get_type ())
#define GV_ROTATE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_ROTATE_TOOL, GvRotateTool))
#define GV_ROTATE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_ROTATE_TOOL, GvRotateToolClass))
#define GV_IS_ROTATE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_ROTATE_TOOL))
#define GV_IS_ROTATE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_ROTATE_TOOL))

typedef struct _GvRotateTool       GvRotateTool;
typedef struct _GvRotateToolClass  GvRotateToolClass;

struct _GvRotateTool
{
    GvTool tool;

    GvShapesLayer *layer;
    gchar *named_layer;

    GvVertex v_pivot;
    GvVertex v_up, v_right;

    GvVertex v_head, v_tail;

    int   rrmode;   /* current mode */
    int   shape_id; /* shapeid of rectangle being reshaped */

    double rotation;
    double scaling;

    GvShape *original;
};

struct _GvRotateToolClass
{
    GvToolClass parent_class;
};

GType gv_rotate_tool_get_type(void);
GvTool* gv_rotate_tool_new(void);
void gv_rotate_tool_set_layer(GvRotateTool *tool, GvShapeLayer *layer);
void gv_rotate_tool_set_named_layer(GvRotateTool *tool, gchar *name);

#endif /* __GV_ROTATE_TOOL_H__ */
