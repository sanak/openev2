/******************************************************************************
 * $Id: gvpquerylayer.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Point query layer.
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
 * $Log: gvpquerylayer.h,v $
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
 * Revision 1.5  2004/08/20 13:53:48  warmerda
 * allow GvShapes to be passed into constructor
 *
 * Revision 1.4  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_PQUERY_LAYER_H__
#define __GV_PQUERY_LAYER_H__

#include "gvshapeslayer.h"

#define GV_TYPE_PQUERY_LAYER            (gv_pquery_layer_get_type ())
#define GV_PQUERY_LAYER(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_PQUERY_LAYER, GvPqueryLayer))
#define GV_PQUERY_LAYER_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_PQUERY_LAYER, GvPqueryLayerClass))
#define GV_IS_PQUERY_LAYER(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_PQUERY_LAYER))
#define GV_IS_PQUERY_LAYER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_PQUERY_LAYER))

/*
 * Properties for storing label shift from point
 */
#define GV_PQUERY_DX_PROP "_gv_pq_dx"
#define GV_PQUERY_DY_PROP "_gv_pq_dy"

typedef struct _GvPqueryLayer       GvPqueryLayer;
typedef struct _GvPqueryLayerClass  GvPqueryLayerClass;

struct _GvPqueryLayer
{
    GvShapesLayer layer;
    gint drag_labels_only;
    gint font;
};

struct _GvPqueryLayerClass
{
    GvShapesLayerClass parent_class;
};

GtkType gv_pquery_layer_get_type(void);
GtkObject* gv_pquery_layer_new(GvShapes *shapes);
void gv_pquery_layer_draw_shape(GvViewArea *view, GvShapesLayer *layer,
				int part_index, GvShape *shape_obj, 
				gv_draw_mode draw_mode,
				GvShapeDrawInfo *drawinfo);

#endif /* __GV_PQUERY_LAYER_H__ */
