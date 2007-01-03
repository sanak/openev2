/******************************************************************************
 * $Id: gvnodetool.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Node editing mode.
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
 * $Log: gvnodetool.h,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:26  uid1026
 *
 * Imported sources
 *
 * Revision 1.4  2002/09/30 20:09:27  warmerda
 * Ensure that first/last vertex of closed rings are kept in sync.  Added
 * "track_node" concept.  Only works for GvShapesLayer areas, not the old
 * GvAreaLayer.
 *
 * Revision 1.3  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_NODE_TOOL_H__
#define __GV_NODE_TOOL_H__

#include "gvtool.h"
#include "gvshapelayer.h"

#define GV_TYPE_NODE_TOOL            (gv_node_tool_get_type ())
#define GV_NODE_TOOL(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_NODE_TOOL, GvNodeTool))
#define GV_NODE_TOOL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_NODE_TOOL, GvNodeToolClass))
#define GV_IS_NODE_TOOL(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_NODE_TOOL))
#define GV_IS_NODE_TOOL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_NODE_TOOL))

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

GtkType gv_node_tool_get_type(void);
GvTool* gv_node_tool_new(void);
void gv_node_tool_set_layer(GvNodeTool *tool, GvShapeLayer *layer);

#endif /* __GV_NODE_TOOL_H__ */
