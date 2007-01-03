/******************************************************************************
 * $Id: appcurlayer.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  APP/Currents
 * Purpose:  APP Currents Layer declarations.
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
 * $Log: appcurlayer.h,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:35  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:25  uid1026
 *
 * Imported sources
 *
 * Revision 1.1  2000/08/25 20:02:22  warmerda
 * New
 *
 */

#ifndef __APPCURLAYER_H__
#define __APPCURLAYER_H__

#include "gvshapeslayer.h"

#define APP_TYPE_CUR_LAYER          (app_cur_layer_get_type ())
#define APP_CUR_LAYER(obj)          (GTK_CHECK_CAST ((obj), APP_TYPE_CUR_LAYER, AppCurLayer))
#define APP_CUR_LAYER_CLASS(klass)  (GTK_CHECK_CLASS_CAST ((klass), APP_TYPE_CUR_LAYER, AppCurLayerClass))
#define APP_IS_CUR_LAYER(obj)       (GTK_CHECK_TYPE ((obj),APP_TYPE_CUR_LAYER))
#define APP_IS_CUR_LAYER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), \
                                                           APP_TYPE_CUR_LAYER))

typedef struct _AppCurLayer       AppCurLayer;
typedef struct _AppCurLayerClass  AppCurLayerClass;

struct _AppCurLayer
{
    GvShapesLayer layer;
};

struct _AppCurLayerClass
{
    GvShapesLayerClass parent_class;
};

GtkType app_cur_layer_get_type(void);
GtkObject* app_cur_layer_new(GvShapes *shapes);

#endif /* __APPCURLAYER_H__ */

