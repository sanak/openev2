/******************************************************************************
 * $Id: gvviewlink.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Manage linked GvViewAreas.
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
 * $Log: gvviewlink.h,v $
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
 * Revision 1.5  2003/02/21 22:35:31  gmwalter
 * Removed unused variables.
 *
 * Revision 1.4  2003/02/20 19:27:19  gmwalter
 * Updated link tool to include Diana's ghost cursor code, and added functions
 * to allow the cursor and link mechanism to use different gcps
 * than the display for georeferencing.  Updated raster properties
 * dialog for multi-band case.  Added some signals to layerdlg.py and
 * oeattedit.py to make it easier for tools to interact with them.
 * A few random bug fixes.
 *
 * Revision 1.3  2002/01/30 17:26:11  warmerda
 * make link state copying smarter
 *
 * Revision 1.2  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_VIEW_LINK_H__
#define __GV_VIEW_LINK_H__

#include <gtk/gtkobject.h>
#include "gvviewarea.h"

#define GV_TYPE_VIEW_LINK            (gv_view_link_get_type ())
#define GV_VIEW_LINK(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_VIEW_LINK, GvViewLink))
#define GV_VIEW_LINK_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_VIEW_LINK, GvViewLinkClass))
#define GV_IS_VIEW_LINK(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_VIEW_LINK))
#define GV_IS_VIEW_LINK_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_VIEW_LINK))

typedef struct _GvViewLink       GvViewLink;
typedef struct _GvViewLinkClass  GvViewLinkClass;

typedef enum {
    GV_LINK_CURSOR_OFF = 0,
    GV_LINK_CURSOR_ON_DEFAULT = 1,
    GV_LINK_CURSOR_ON_LOGICAL = 2
} GvLinkCursorMode;


struct _GvViewLink
{
    GtkObject object;

    GList *views;
    gint enabled : 1;
    gint blocked : 1;

    /* Ghost cursor stuff */
    GvLinkCursorMode cursor_mode;
    GvViewArea *src_view;
};

struct _GvViewLinkClass
{
    GtkObjectClass parent_class;
};

GtkType    gv_view_link_get_type(void);
GtkObject* gv_view_link_new(void);
void gv_view_link_register_view(GvViewLink *link, GvViewArea *view);
void gv_view_link_remove_view(GvViewLink *link, GvViewArea *view);
void gv_view_link_enable(GvViewLink *link);
void gv_view_link_disable(GvViewLink *link);
/* Ghost cursor function */
void gv_view_link_set_cursor_mode(GvViewLink *link, int cursor_mode);

#endif /*__GV_VIEW_LINK_H__ */
