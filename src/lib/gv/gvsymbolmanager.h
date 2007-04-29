/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  manage file-based symbols
 * Author:   Paul Spencer (pgs@magma.ca)
 * Maintainer: Mario Beauchamp, starged@gmail.com
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
 */

#ifndef __GV_SYMBOL_H__
#define __GV_SYMBOL_H__

#include <gtk/gtkgl.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include "gvtypes.h"
#include "gvshapes.h"


#define GV_TYPE_SYMBOL_MANAGER       (gv_symbol_manager_get_type ())
#define GV_SYMBOL_MANAGER(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_SYMBOL_MANAGER, GvSymbolManager))
#define GV_SYMBOL_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_SYMBOL_MANAGER, GvSymbolManagerClass))
#define GV_IS_SYMBOL_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_SYMBOL_MANAGER))
#define GV_IS_SYMBOL_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_SYMBOL_TYPE_MANAGER))

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
    GObject object;
    GHashTable *symbol_cache;
};

struct _GvSymbolManagerClass
{
    GObjectClass parent_class;
};

GType            gv_symbol_manager_get_type  (void);
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
