/******************************************************************************
 * $Id: gvdata.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Base class for raster, vector and layer data containers.
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
 * $Log: gvdata.h,v $
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
 * Revision 1.13  2003/02/07 20:06:49  andrey_kiselev
 * Memory leaks fixed.
 *
 * Revision 1.12  2001/08/08 02:57:03  warmerda
 * implemented support for gv_data_registry_dump()
 *
 * Revision 1.11  2001/05/15 16:22:13  pgs
 * added meta-changed signal for notification of\nchanges to meta data
 *
 * Revision 1.10  2000/07/21 01:31:11  warmerda
 * added read_only flag for GvData, and utilize for vector layers
 *
 * Revision 1.9  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_DATA_H__
#define __GV_DATA_H__

/* Placed in this header...
#include <gtk/gtkdata.h>*/

/* TEMP - try locating gtkdata instead of defining it.. */

/* Added instead... */
#include <gtk/gtkobject.h>
#include "gvtypes.h"
#include "gvproperties.h"

#define GV_TYPE_DATA            (gv_data_get_type ())
#define GV_DATA(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_DATA, GvData))
#define GV_DATA_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_DATA, GvDataClass))
#define GV_IS_DATA(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_DATA))
#define GV_IS_DATA_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_DATA))

typedef struct _GvData       GvData;
typedef struct _GvDataClass  GvDataClass;
typedef struct _GvDataMemento GvDataMemento;


/* FROM_GTK_DATA_H... 
typedef struct _GtkData       GtkData;
typedef struct _GtkDataClass  GtkDataClass;

struct _GtkData
{
  GtkObject object;
};

struct _GtkDataClass
{
  GtkObjectClass parent_class;

  void (* disconnect) (GtkData *data);
};
*/

/* FROM_GTK_DATA_H */

struct _GvData
{
    GtkObject data;

    GvData *parent;
    gchar *name;

    guint frozen : 1;
    guint changed_while_frozen : 1;
    gint  read_only;

    char *projection;

    GvProperties properties;
};

struct _GvDataClass
{
    GtkObjectClass parent_class;

    void (* changing) (GvData *data, gpointer change_info);
    void (* changed) (GvData *data, gpointer change_info);
    void (* meta_changed) (GvData *data);
    void (* child_changed) (GvData *data, GvData *child, gpointer change_info);
    void (* get_memento) (GvData *data, gpointer change_info,
			  GvDataMemento **memento);
    void (* set_memento) (GvData *data, GvDataMemento *memento);
    void (* del_memento) (GvData *data, GvDataMemento *memento);
};

struct _GvDataMemento
{
    GvData *data;
    gint type;
    gint group;
};

GtkType    gv_data_get_type (void);

void gv_data_set_parent(GvData *data, GvData *parent);
GvData *gv_data_get_parent(GvData *data);
void gv_data_set_name(GvData *data, const gchar *name);
const gchar* gv_data_get_name(GvData *data);

void gv_data_changing(GvData *data, gpointer change_info);
void gv_data_changed(GvData *data, gpointer change_info);
void gv_data_meta_changed(GvData *data);
void gv_data_freeze(GvData *data);
void gv_data_thaw(GvData *data);
GvDataMemento* gv_data_get_memento(GvData *data, gpointer change_info);
void gv_data_set_memento(GvData *data, GvDataMemento *memento);
void gv_data_del_memento(GvData *data, GvDataMemento *memento);
const char *gv_data_get_projection(GvData *data);
void gv_data_set_projection(GvData *data, const char *projection);
void gv_data_set_read_only(GvData *data, gint read_only);
gint gv_data_is_read_only(GvData *data);

void   gv_data_set_property(GvData *data, const char *name, const char *value);
const char *gv_data_get_property(GvData *data, const char *name);
GvProperties *gv_data_get_properties(GvData *data);

void gv_data_registry_dump(void);

#endif /*__GV_DATA_H__ */


