/******************************************************************************
 * $Id: gvpointlayer.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Display layer for GvPoints.
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
 * $Log: gvpointlayer.h,v $
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
 * Revision 1.2  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_POINT_LAYER_H__
#define __GV_POINT_LAYER_H__

#include "gvshapelayer.h"
#include "gvpoints.h"

#define GV_TYPE_POINT_LAYER            (gv_point_layer_get_type ())
#define GV_POINT_LAYER(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_POINT_LAYER, GvPointLayer))
#define GV_POINT_LAYER_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_POINT_LAYER, GvPointLayerClass))
#define GV_IS_POINT_LAYER(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_POINT_LAYER))
#define GV_IS_POINT_LAYER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_POINT_LAYER))

typedef struct _GvPointLayer       GvPointLayer;
typedef struct _GvPointLayerClass  GvPointLayerClass;

struct _GvPointLayer
{
    GvShapeLayer shape_layer;

    GvPoints *data;
    gvfloat point_size;
};

struct _GvPointLayerClass
{
    GvShapeLayerClass parent_class;
};

GtkType gv_point_layer_get_type(void);
GtkObject* gv_point_layer_new(GvPoints *data);

gint gv_point_layer_select_new_point(GvPointLayer *layer, GvVertex *vertex);

#endif /* __GV_POINT_LAYER_H__ */
