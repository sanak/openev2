/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Base class for editing mode tools.
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

#ifndef __GV_TOOL_H__
#define __GV_TOOL_H__

#include <gdk/gdk.h>
#include "gvviewarea.h"
#include "gvtypes.h"

#define GV_TYPE_TOOL            (gv_tool_get_type ())
#define GV_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_TOOL, GvTool))
#define GV_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_TOOL, GvToolClass))
#define GV_IS_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_TOOL))
#define GV_IS_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_TOOL))

/* Shortcuts to parent class functions */
#define GV_TOOL_ACTIVATE(t,v)  (*((GvToolClass*)g_type_class_peek (GV_TYPE_TOOL))->activate)(GV_TOOL(t),v)
#define GV_TOOL_DEACTIVATE(t,v)  (*((GvToolClass*)g_type_class_peek (GV_TYPE_TOOL))->deactivate)(GV_TOOL(t),v)

typedef struct _GvTool       GvTool;
typedef struct _GvToolClass  GvToolClass;

struct _GvTool
{
    GObject object;

    GvViewArea *view;
    GdkCursor *cursor;

    GvRect boundary;      /* Constraints of where ROI can be dragged out */
    gint bounded : 1;     /* Boolean, have boundary constraints on ROI be set*/
};

struct _GvToolClass
{
    GObjectClass parent_class;

    void (* activate)       (GvTool *tool, GvViewArea *view);
    void (* deactivate)     (GvTool *tool, GvViewArea *view);
    gboolean (* draw)           (GvTool *tool);
    gboolean (* button_press)   (GvTool *tool, GdkEventButton *event);
    gboolean (* button_release) (GvTool *tool, GdkEventButton *event);
    gboolean (* motion_notify)  (GvTool *tool, GdkEventMotion *event);
    void (* key_press)      (GvTool *tool, GdkEventKey *event);
    gboolean (* enter_notify)   (GvTool *tool, GdkEventCrossing *event);
    gboolean (* leave_notify)   (GvTool *tool, GdkEventCrossing *event);
};

GType gv_tool_get_type(void);

void gv_tool_activate(GvTool *tool, GvViewArea *view);
void gv_tool_deactivate(GvTool *tool, GvViewArea *view);
GvViewArea *gv_tool_get_view(GvTool *tool);
gint gv_tool_set_boundary(GvTool *tool, GvRect *rect);
void gv_tool_clamp_to_bounds(GvTool *tool, gvgeocoord *x, gvgeocoord *y);
gint gv_tool_check_bounds(GvTool *tool, gvgeocoord x, gvgeocoord y);
void gv_tool_set_cursor(GvTool *tool, gint cursor_type);

#endif /* __GV_TOOL_H__ */
