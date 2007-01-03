/******************************************************************************
 * $Id: gvtool.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Base class for editing mode tools.
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
 * $Log: gvtool.h,v $
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
 * Revision 1.8  2005/01/17 18:37:43  gmwalter
 * Add ability to reset tool cursor type.
 *
 * Revision 1.7  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.6  2000/07/27 20:06:23  warmerda
 * added boundary constraints
 *
 * Revision 1.5  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_TOOL_H__
#define __GV_TOOL_H__

#include <gdk/gdk.h>
#include <gtk/gtkobject.h>
#include "gvviewarea.h"
#include "gvtypes.h"

#define GV_TYPE_TOOL            (gv_tool_get_type ())
#define GV_TOOL(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_TOOL, GvTool))
#define GV_TOOL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_TOOL, GvToolClass))
#define GV_IS_TOOL(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_TOOL))
#define GV_IS_TOOL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_TOOL))

/* Shortcuts to parent class functions */
#define GV_TOOL_ACTIVATE(t,v)  (*((GvToolClass*)gtk_type_class(GV_TYPE_TOOL))->activate)(GV_TOOL(t),v)
#define GV_TOOL_DEACTIVATE(t,v)  (*((GvToolClass*)gtk_type_class(GV_TYPE_TOOL))->deactivate)(GV_TOOL(t),v)

typedef struct _GvTool       GvTool;
typedef struct _GvToolClass  GvToolClass;

struct _GvTool
{
    GtkObject object;

    GvViewArea *view;
    GdkCursor *cursor;

    GvRect boundary;      /* Constraints of where ROI can be dragged out */
    gint bounded : 1;     /* Boolean, have boundary constraints on ROI be set*/
};

struct _GvToolClass
{
    GtkObjectClass parent_class;

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

GtkType gv_tool_get_type(void);

void gv_tool_activate(GvTool *tool, GvViewArea *view);
void gv_tool_deactivate(GvTool *tool, GvViewArea *view);
GvViewArea *gv_tool_get_view(GvTool *tool);
gint gv_tool_set_boundary(GvTool *tool, GvRect *rect);
void gv_tool_clamp_to_bounds(GvTool *tool, gvgeocoord *x, gvgeocoord *y);
gint gv_tool_check_bounds(GvTool *tool, gvgeocoord x, gvgeocoord y);
void gv_tool_set_cursor(GvTool *tool, gint cursor_type);

#endif /* __GV_TOOL_H__ */
