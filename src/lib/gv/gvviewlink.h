/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Manage linked GvViewAreas.
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

#ifndef __GV_VIEW_LINK_H__
#define __GV_VIEW_LINK_H__

#include "gvviewarea.h"

#define GV_TYPE_VIEW_LINK            (gv_view_link_get_type ())
#define GV_VIEW_LINK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_VIEW_LINK, GvViewLink))
#define GV_VIEW_LINK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_VIEW_LINK, GvViewLinkClass))
#define GV_IS_VIEW_LINK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_VIEW_LINK))
#define GV_IS_VIEW_LINK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_VIEW_LINK))

typedef struct _GvViewLink       GvViewLink;
typedef struct _GvViewLinkClass  GvViewLinkClass;

typedef enum {
    GV_LINK_CURSOR_OFF = 0,
    GV_LINK_CURSOR_ON_DEFAULT = 1,
    GV_LINK_CURSOR_ON_LOGICAL = 2
} GvLinkCursorMode;

struct _GvViewLink
{
    GObject object;

    GList *views;
    gint enabled : 1;
    gint blocked : 1;

    /* Ghost cursor stuff */
    GvLinkCursorMode cursor_mode;
    GvViewArea *src_view;
};

struct _GvViewLinkClass
{
    GObjectClass parent_class;
};

GType    gv_view_link_get_type(void);
GObject* gv_view_link_new(void);
void gv_view_link_register_view(GvViewLink *link, GvViewArea *view);
void gv_view_link_remove_view(GvViewLink *link, GvViewArea *view);
void gv_view_link_enable(GvViewLink *link);
void gv_view_link_disable(GvViewLink *link);
/* Ghost cursor function */
void gv_view_link_set_cursor_mode(GvViewLink *link, int cursor_mode);

#endif /*__GV_VIEW_LINK_H__ */
