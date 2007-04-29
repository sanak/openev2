/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Area (Polygon) editing mode.
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

#ifndef __GV_AREA_TOOL_H__
#define __GV_AREA_TOOL_H__

#include "gvtool.h"
#include "gvshapelayer.h"

#define GV_TYPE_AREA_TOOL            (gv_area_tool_get_type ())
#define GV_AREA_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_AREA_TOOL, GvAreaTool))
#define GV_AREA_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_AREA_TOOL, GvAreaToolClass))
#define GV_IS_AREA_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_AREA_TOOL))
#define GV_IS_AREA_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_AREA_TOOL))

typedef struct _GvAreaTool       GvAreaTool;
typedef struct _GvAreaToolClass  GvAreaToolClass;

struct _GvAreaTool
{
    GvTool tool;

    GvShapeLayer *layer;
    gchar *named_layer;
    guint drawing : 1;
    gint ring_id;
    GvVertex v_head, v_tail;
    GvDataMemento   *memento;
};

struct _GvAreaToolClass
{
    GvToolClass parent_class;
};

GType gv_area_tool_get_type(void);
GvTool* gv_area_tool_new(void);
void gv_area_tool_set_layer(GvAreaTool *tool, GvShapeLayer *layer);
void gv_area_tool_set_named_layer(GvAreaTool *tool, gchar *name);

#endif /* __GV_AREA_TOOL_H__ */
