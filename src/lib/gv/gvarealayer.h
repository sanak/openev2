/******************************************************************************
 * $Id: gvarealayer.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Display layer of GvAreas.
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
 * $Log: gvarealayer.h,v $
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
 * Revision 1.4  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_AREA_LAYER_H__
#define __GV_AREA_LAYER_H__

#include "gvshapelayer.h"
#include "gvareas.h"

#define GV_TYPE_AREA_LAYER            (gv_area_layer_get_type ())
#define GV_AREA_LAYER(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_AREA_LAYER, GvAreaLayer))
#define GV_AREA_LAYER_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_AREA_LAYER, GvAreaLayerClass))
#define GV_IS_AREA_LAYER(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_AREA_LAYER))
#define GV_IS_AREA_LAYER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_AREA_LAYER))

typedef struct _GvAreaLayer       GvAreaLayer;
typedef struct _GvAreaLayerClass  GvAreaLayerClass;

struct _GvAreaLayer
{
    GvShapeLayer shape_layer;

    GvAreas *data;
    gint edit_ring;
};

struct _GvAreaLayerClass
{
    GvShapeLayerClass parent_class;
};

GtkType gv_area_layer_get_type(void);
GtkObject* gv_area_layer_new(GvAreas *data);

gint gv_area_layer_select_new_area(GvAreaLayer *layer);
gint gv_area_layer_select_new_ring(GvAreaLayer *layer, gint area_id);
void gv_area_layer_edit_done(GvAreaLayer *layer);

#endif /* __GV_AREA_LAYER_H__ */
