/******************************************************************************
 * $Id: gvtess.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Tesselation for GvAreas (will be discarded with GvAreas)
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
 * $Log: gvtess.c,v $
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
 * Revision 1.12  2002/12/10 02:57:33  sduclos
 * update tess callback cast for WIN_CALLBACK
 *
 * Revision 1.11  2002/11/05 18:56:25  sduclos
 * fix gcc warning
 *
 * Revision 1.10  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.9  2001/05/01 21:48:01  warmerda
 * fixed Mesa3.3 compatibility fixes
 *
 * Revision 1.8  2001/04/24 16:22:12  warmerda
 * fixed mesa3.3 incompatibility
 *
 * Revision 1.7  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

/*
#include <GL/glu.h>
*/

#include "gluos.h"
#include "glu.h"

#include "gvtess.h"

/* gtkgl.h is to satisfy some Windows definition requirements */
#include <gtk/gtkgl.h>

/* Backward compatibility with glu ver. 1.1 */
#if !defined(GLU_VERSION_1_2)

#if !defined(GLU_TESS_BEGIN)
#define GLU_TESS_BEGIN  GLU_BEGIN
#define GLU_TESS_END    GLU_END
#define GLU_TESS_VERTEX GLU_VERTEX
#define GLU_TESS_ERROR  GLU_ERROR
#endif

#define gluTessBeginPolygon(tess,data) gluBeginPolygon(tess)
#define gluTessBeginContour(tess)      gluNextContour(tess, GLU_UNKNOWN)
#define gluTessEndContour(tess)
#define gluTessEndPolygon(tess)        gluEndPolygon(tess)

typedef GLUtriangulatorObj GLUtesselator;

#endif /* GLU_VERSION_1_2 */

enum { GV_CCW, GV_CW };

static GvArea *area;

#define WIN_CALLBACK
#ifdef WIN32
#ifndef __MINGW32__
#define WIN_CALLBACK FAR PASCAL
#endif
#endif

static void WIN_CALLBACK tess_begin(GLenum type);
static void WIN_CALLBACK tess_end(void);
static void WIN_CALLBACK tess_vertex(void *data);
static void WIN_CALLBACK tess_error(GLenum err);
static gint check_ring_lengths(void);
static void check_winding(void);
static gint find_winding(GArray *array);
static void reverse_array(GArray *array);

gint gv_area_tessellate(GvArea *in_area)
{
    typedef void (*f);
    static GLUtesselator *tess = NULL;
    GArray *ring;
    GvVertex *v;
    int i, j;
    GLdouble coords[3];

    
    if (!tess)
    {
	tess = gluNewTess();
	g_return_val_if_fail(tess, FALSE);

	gluTessCallback(tess, GLU_TESS_BEGIN, (f) tess_begin);
	gluTessCallback(tess, GLU_TESS_END,   (f) tess_end);
	gluTessCallback(tess, GLU_TESS_VERTEX,(f) tess_vertex);
	gluTessCallback(tess, GLU_TESS_ERROR, (f) tess_error);
    }

    /* Global is available to all tess callbacks */
    area = in_area;

    /* Check for short ring lengths */
    if (!check_ring_lengths()) return FALSE;
    
    /* Fix ring winding before tesselation */
    check_winding();
    
    area->fill_objects = 0;
    if (area->fill)
    {
	g_array_set_size(area->fill, 0);
	g_array_set_size(area->mode_offset, 0);
    }
    else
    {
	area->fill = g_array_new(FALSE, FALSE, sizeof(GvVertex));
	area->mode_offset = 
	  g_array_new(FALSE, FALSE, sizeof(gint));
    }

    coords[2] = 0.0;

    gluTessBeginPolygon(tess, NULL);
    for (i=0; i < area->rings->len; ++i)
    {
	ring = gv_areas_get_ring(area, i);
	
	gluTessBeginContour(tess);
	for (j=0; j < ring->len; ++j)
	{
	    v = &g_array_index(ring, GvVertex, j);
	    coords[0] = (GLdouble)v->x;
	    coords[1] = (GLdouble)v->y;
	    
	    gluTessVertex(tess, coords, v);
	}
	gluTessEndContour(tess);
    }
    gluTessEndPolygon(tess);

    return (area->fill_objects > 0 
	    && g_array_index(area->mode_offset,gint,0) != GV_TESS_NONE);
}

static void WIN_CALLBACK
tess_begin(GLenum type)
{
    area->fill_objects++;
    g_array_append_val(area->mode_offset, type );
    g_array_append_val(area->mode_offset, area->fill->len );
}

static void WIN_CALLBACK
tess_end(void)
{
}

static void WIN_CALLBACK
tess_vertex(void *data)
{
  /* for some unknown reason, we somes get called with a NULL 
     if the object doesn't tesselate properly (with libMesa). */

  if( data != NULL )
    g_array_append_vals(area->fill, data, 1);
  else
    tess_error( 0 );
}

static void WIN_CALLBACK
tess_error(GLenum err)
{
  GLenum bad_fill_mode = GV_TESS_NONE;
  gint   offset = 0;

  g_array_set_size(area->fill, 0);
  g_array_set_size(area->mode_offset, 0);

  area->fill_objects = 1;
  g_array_append_val(area->mode_offset, bad_fill_mode );
  g_array_append_val(area->mode_offset, offset );
}

static gint
check_ring_lengths(void)
{
    int r, rings;

    rings = gv_areas_num_rings(area);
    
    for (r=0; r < rings; ++r)
    {
	if (gv_areas_get_ring(area, r)->len < 3) return FALSE;
    }
    return TRUE;
}

static void
check_winding(void)
{
    int r, rings, winding;
    GArray *ring;

    rings = gv_areas_num_rings(area);
    
    for (r=0; r < rings; ++r)
    {
	ring = gv_areas_get_ring(area, r);
	winding = find_winding(ring);
	if ((r == 0 && winding == GV_CW) || (r > 0 && winding == GV_CCW))
	{
	    reverse_array(ring);
	}
    }
}

/* According to the comp.graphics.algorthms FAQ, item 2.07, the
 * orientation (winding) of a simple polygon can be determined by
 * looking at the sign of the following series:
 *    sum_{i=0}^{n-1} (x_i y_{i+1} - y_i x_{i+1})
 * A positive result means a counter clockwise winding.
 */ 
static gint
find_winding(GArray *array)
{
    GvVertex *a, *b;
    gvgeocoord sum;
    int i;

    sum = 0.0;
    a = &g_array_index(array, GvVertex, 0);
    b = a+1;
    for (i=1; i < array->len; ++i, ++a, ++b)
    {
	sum += a->x * b->y - a->y * b->x;
    }
    b = &g_array_index(array, GvVertex, 0);
    sum += a->x * b->y - a->y * b->x;

    return (sum < 0.0 ? GV_CW : GV_CCW);
}

static void
reverse_array(GArray *array)
{
    GvVertex temp;
    int i;

    for (i=0; i < array->len/2; ++i)
    { 
	temp = g_array_index(array, GvVertex, i);
	g_array_index(array, GvVertex, i) = g_array_index(array, GvVertex,
							  array->len - i - 1);
	g_array_index(array, GvVertex, array->len - i - 1) = temp;
    }
}

