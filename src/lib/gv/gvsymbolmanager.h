/******************************************************************************
 * $Id: gvsymbolmanager.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  manage file-based symbols
 * Author:   Paul Spencer (pgs@magma.ca)
 *
 ******************************************************************************
 * Copyright (c) 2002, Paul Spencer
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
 * $Log: gvsymbolmanager.h,v $
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
 * Revision 1.5  2003/09/02 17:25:08  warmerda
 * Added _has_symbol(), and _get_names() methods.  Use outside GvShape
 * serialize/deserialize code.  Don't store symbols with absolute paths in
 * the hash.
 *
 * Revision 1.4  2003/04/08 11:56:32  andrey_kiselev
 * Implemented gv_symbol_manager_save_vector_symbol() function.
 *
 * Revision 1.3  2003/02/28 16:46:46  warmerda
 * added partial support for vector symbols
 *
 * Revision 1.2  2003/01/08 03:25:40  warmerda
 * fiddle with include files for win build
 *
 * Revision 1.1  2002/11/14 20:10:41  warmerda
 * New
 *
 *
 */

#ifndef __GV_SYMBOL_H__
#define __GV_SYMBOL_H__

#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <gtk/gtkobject.h>
#include "gvtypes.h"
#include "gvshapes.h"


#define GV_TYPE_SYMBOL_MANAGER       (gv_symbol_manager_get_type ())
#define GV_SYMBOL_MANAGER(obj)       (GTK_CHECK_CAST ((obj), GV_TYPE_SYMBOL_MANAGER, GvSymbolManager))
#define GV_SYMBOL_MANAGER_CLASS(klass)      (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_SYMBOL_MANAGER, GvSymbolManagerClass))
#define GV_IS_SYMBOL_MANAGER(obj)           (GTK_CHECK_TYPE ((obj), GV_TYPE_SYMBOL_MANAGER))
#define GV_IS_SYMBOL_MANAGER_CLASS(klass)   (GTK_CHECK_CLASS_TYPE ((klass), GV_SYMBOL_TYPE_MANAGER))

enum { GV_SYMBOL_RASTER = 0, GV_SYMBOL_VECTOR = 1 };

typedef struct _GvSymbolManager         GvSymbolManager;
typedef struct _GvSymbolManagerClass    GvSymbolManagerClass;

typedef struct _GvSymbolObj
{
    guint        type;           /* GV_SYMBOL_RASTER or GV_SYMBOL_VECTOR */

    /* for rasters */
    GvColor     foreground;     /* foreground color */
    GvColor     background;     /* background color */
    guint       width;          /* width of the symbol */
    guint       height;         /* height of the symbol */

    /* type specific storage (RGBA image or GvShape *) */
    void *      buffer;         
} GvSymbolObj;

struct _GvSymbolManager
{
    GtkObject object;
    GHashTable *symbol_cache;
};

struct _GvSymbolManagerClass
{
    GtkObjectClass parent_class;
};

GtkType          gv_symbol_manager_get_type  (void);
GvSymbolManager* gv_symbol_manager_new       (void);
GvSymbolManager* gv_get_symbol_manager       (void);

int           gv_symbol_manager_has_symbol(GvSymbolManager *manager, 
                                            const char *name);

GvSymbolObj*  gv_symbol_manager_get_symbol(GvSymbolManager *manager,
                                            const char  *pszFilename);

void gv_symbol_manager_inject_raster_symbol(GvSymbolManager *manager, 
                                             const char *symbol_name, 
                                             int width, int height, 
                                             void *rgba_buffer);
void gv_symbol_manager_inject_vector_symbol(GvSymbolManager *manager, 
                                             const char *symbol_name, 
                                             GvShape *shape);
int  gv_symbol_manager_eject_symbol(GvSymbolManager *manager, 
                                     const char *symbol_name);
int gv_symbol_manager_save_vector_symbol(GvSymbolManager *manager, 
                                          const char *symbol_name, 
                                          const char *new_name);

char **gv_symbol_manager_get_names(GvSymbolManager *manager);


#endif /* __GV_SYMBOL_H__ */
