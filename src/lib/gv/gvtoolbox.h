/******************************************************************************
 * $Id: gvtoolbox.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Container for available editing tools, manages which is active.
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
 * $Log: gvtoolbox.h,v $
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

#ifndef __GV_TOOLBOX_H__
#define __GV_TOOLBOX_H__

#include "gvtool.h"

#define GV_TYPE_TOOLBOX            (gv_toolbox_get_type ())
#define GV_TOOLBOX(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_TOOLBOX, GvToolbox))
#define GV_TOOLBOX_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_TOOLBOX, GvToolboxClass))
#define GV_IS_TOOLBOX(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_TOOLBOX))
#define GV_IS_TOOLBOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_TOOLBOX))

typedef struct _GvToolbox       GvToolbox;
typedef struct _GvToolboxClass  GvToolboxClass;

struct _GvToolbox
{
    GvTool tool;

    GHashTable *tools;
    GvTool *active_tool;
    GList *views;
};

struct _GvToolboxClass
{
    GvToolClass parent_class;
};

GtkType gv_toolbox_get_type(void);
GvTool* gv_toolbox_new(void);

void gv_toolbox_add_tool(GvToolbox *toolbox, gchar *name, GvTool *tool);
void gv_toolbox_activate_tool(GvToolbox *toolbox, gchar *tool_name);

#endif /* __GV_TOOLBOX_H__ */
