/******************************************************************************
 * $Id: gvpolylines.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Polylines data container (superceeded by GvShapes)
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
 * $Log: gvpolylines.h,v $
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
 * Revision 1.7  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.6  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_POLYLINES_H__
#define __GV_POLYLINES_H__

#include "gvdata.h"

#define GV_TYPE_POLYLINES            (gv_polylines_get_type ())
#define GV_POLYLINES(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_POLYLINES, GvPolylines))
#define GV_POLYLINES_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_POLYLINES, GvPolylinesClass))
#define GV_IS_POLYLINES(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_POLYLINES))
#define GV_IS_POLYLINES_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_POLYLINES))

typedef struct _GvPolylines       GvPolylines;
typedef struct _GvPolylinesClass  GvPolylinesClass;

struct _GvPolylines
{
    GvData data;

    GPtrArray *lines;
    GvRect extents;
    guint extents_valid : 1;
};

struct _GvPolylinesClass
{
    GvDataClass parent_class;
};

GtkType    gv_polylines_get_type (void);

GvData* gv_polylines_new(void);
gint gv_polylines_new_line(GvPolylines *pline);
gint gv_polylines_new_line_with_data(GvPolylines *pline, gint num_nodes, GvVertex *vertex);
gint gv_polylines_num_nodes(GvPolylines *pline, gint line_id);

void gv_polylines_delete_lines(GvPolylines *pline, gint num_lines, gint *line_id);
void gv_polylines_translate_lines(GvPolylines *pline, gint num_lines, gint *line_id, gvgeocoord dx, gvgeocoord dy);
void gv_polylines_set_nodes(GvPolylines *pline, gint line_id, gint num_nodes, GvVertex *vertex);
void gv_polylines_append_nodes(GvPolylines *pline, gint line_id, gint num_nodex, GvVertex *vertex);
void gv_polylines_insert_nodes(GvPolylines *pline, gint line_id, gint node_id, gint num_nodes, GvVertex *vertex);
void gv_polylines_delete_nodes(GvPolylines *pline, gint line_id, gint num_nodes, gint *node_id);
void gv_polylines_move_node(GvPolylines *pline, gint line_id, gint node_id, GvVertex *vertex);
GvVertex* gv_polylines_get_node(GvPolylines *pline, gint line_id, gint node_id);
void gv_polylines_get_extents(GvPolylines *pline, GvRect *rect);

#define gv_polylines_num_lines(pline) \
     (pline->lines->len)
#define gv_polylines_get_line(pline,id) \
     ((GArray*)g_ptr_array_index(pline->lines, id))

#endif /*__GV_POLYLINES_H__ */

