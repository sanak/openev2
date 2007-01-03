/******************************************************************************
 * $Id: gvselecttool.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Shape selection tool.
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
 * $Log: gvselecttool.h,v $
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

#ifndef __GV_SELECTION_TOOL_H__
#define __GV_SELECTION_TOOL_H__

#include "gvtool.h"
#include "gvshapelayer.h"

#define GV_TYPE_SELECTION_TOOL            (gv_selection_tool_get_type ())
#define GV_SELECTION_TOOL(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_SELECTION_TOOL, GvSelectionTool))
#define GV_SELECTION_TOOL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_SELECTION_TOOL, GvSelectionToolClass))
#define GV_IS_SELECTION_TOOL(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_SELECTION_TOOL))
#define GV_IS_SELECTION_TOOL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_SELECTION_TOOL))

typedef struct _GvSelectionTool       GvSelectionTool;
typedef struct _GvSelectionToolClass  GvSelectionToolClass;

struct _GvSelectionTool
{
    GvTool tool;

    GvShapeLayer *layer;
    gint banding  : 1;
    gint dragging : 1;
    GvVertex v_head, v_tail;
};

struct _GvSelectionToolClass
{
    GvToolClass parent_class;
};

GtkType gv_selection_tool_get_type(void);
GvTool* gv_selection_tool_new(void);
void gv_selection_tool_set_layer(GvSelectionTool *tool, GvShapeLayer *layer);

#endif /* __GV_SELECTION_TOOL_H__ */
