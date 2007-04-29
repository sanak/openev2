/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Node editing mode.
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

#ifndef __GV_NODE_TOOL_H__
#define __GV_NODE_TOOL_H__

#include "gvtool.h"
#include "gvshapelayer.h"

#define GV_TYPE_NODE_TOOL            (gv_node_tool_get_type ())
#define GV_NODE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_NODE_TOOL, GvNodeTool))
#define GV_NODE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_NODE_TOOL, GvNodeToolClass))
#define GV_IS_NODE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_NODE_TOOL))
#define GV_IS_NODE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_NODE_TOOL))

typedef struct _GvNodeTool       GvNodeTool;
typedef struct _GvNodeToolClass  GvNodeToolClass;

struct _GvNodeTool
{
    GvTool tool;

    GvShapeLayer *layer;
    gint node_selected : 1;
    gint dragging : 1;
    gint changing : 1;
    GvVertex v_orig;
    GvNodeInfo edit_node;
    GvVertex v_head;
    GvVertex v_tail;

    GvNodeInfo tracking_node;
    GvVertex v_orig_tracking;
};

struct _GvNodeToolClass
{
    GvToolClass parent_class;
};

GType gv_node_tool_get_type(void);
GvTool* gv_node_tool_new(void);
void gv_node_tool_set_layer(GvNodeTool *tool, GvShapeLayer *layer);

#endif /* __GV_NODE_TOOL_H__ */
