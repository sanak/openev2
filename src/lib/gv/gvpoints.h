/******************************************************************************
 * $Id: gvpoints.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Points data container (superceed by GvShapes)
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
 * $Log: gvpoints.h,v $
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
 * Revision 1.4  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.3  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_POINTS_H__
#define __GV_POINTS_H__

#include "gvdata.h"

#define GV_TYPE_POINTS            (gv_points_get_type ())
#define GV_POINTS(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_POINTS, GvPoints))
#define GV_POINTS_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_POINTS, GvPointsClass))
#define GV_IS_POINTS(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_POINTS))
#define GV_IS_POINTS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_POINTS))

typedef struct _GvPoint        GvPoint;
typedef struct _GvPoints       GvPoints;
typedef struct _GvPointsClass  GvPointsClass;

struct _GvPoint
{
    GvVertex v;
    gpointer meta;
};

struct _GvPoints
{
    GvData data;

    GArray *points;
    GvRect extents;
    guint extents_valid : 1;
};

struct _GvPointsClass
{
    GvDataClass parent_class;
};

GtkType    gv_points_get_type (void);

GvData* gv_points_new(void);
gint gv_points_new_point(GvPoints *points, GvVertex *vertex);
void gv_points_delete_points(GvPoints *points, gint num_points, gint *point_id);
void gv_points_translate_points(GvPoints *points, gint num_points, gint *point_id, gvgeocoord dx, gvgeocoord dy);
void gv_points_set_point(GvPoints *points, gint point_id, GvVertex *vertex);
void gv_points_get_extents(GvPoints *points, GvRect *rect);

#define gv_points_num_points(pts) \
     (pts->points->len)
#define gv_points_get_point(pts,id) \
     (&g_array_index(pts->points, GvPoint, id))
#define gv_points_get_meta(pts,id) \
     (gv_points_get_point(pts,id)->meta)
#define gv_points_set_meta(pts,id,data) \
     gv_points_get_point(pts,id)->meta = data

#endif /*__GV_POINTS_H__ */
