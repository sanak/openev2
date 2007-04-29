/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Tesselation for GvAreaShape.
 * Author:   Frank Warmerdam, warmerda@home.com
 * Maintainer: Mario Beauchamp, starged@gmail.com
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
 */

#include "tess.h"
#include "gvshapes.h"
#include <string.h>

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

#define WIN_CALLBACK
#ifdef WIN32
#ifndef __MINGW32__
#define WIN_CALLBACK FAR PASCAL
#endif
#endif

static void WIN_CALLBACK tess_begin(GLenum type, void *in_area);
static void WIN_CALLBACK tess_end(void);
static void WIN_CALLBACK tess_vertex(void *data, void *in_area);
static void WIN_CALLBACK tess_error(GLenum err, void *in_area);
static void WIN_CALLBACK tess_combine(GLdouble coords[3], 
                                      gvgeocoord *vertex_data[4],
                                      GLfloat weight[4], void **dataOut);
static gint check_ring_lengths(GvAreaShape *area);
static void check_winding(GvAreaShape *area);
static gint find_winding(gvgeocoord *, int);
static void reverse_array(gvgeocoord *, int);

gint
gv_area_shape_tessellate(GvAreaShape *in_area)
{
    typedef void (*f);
    static GLUtesselator *tess = NULL;
    int i, j, node_count;
    GLdouble coords[3];
    gvgeocoord   *xyz_nodes;

    /* A fill_objects value of -2 is a special flag meaning don't tesselate
       or fill because we are in the midst of editing. */
    if( in_area->fill_objects == -2 )
        return FALSE;

    if (!tess)
    {
        tess = gluNewTess();
        g_return_val_if_fail(tess, FALSE);
        gluTessProperty(tess, GLU_TESS_TOLERANCE, .01);

        gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (f) tess_begin);

        gluTessCallback(tess, GLU_TESS_VERTEX_DATA,(f) tess_vertex);

        gluTessCallback(tess, GLU_TESS_COMBINE,(f) tess_combine);

        gluTessCallback(tess, GLU_TESS_ERROR_DATA, (f) tess_error);
    }

    /* Check for short ring lengths, or unclosed rings */
    if (!check_ring_lengths(in_area)) return FALSE;

    /* Fix ring winding before tesselation */
    check_winding(in_area);

    in_area->fill_objects = 0;
    if (in_area->fill)
    {
        g_array_set_size(in_area->fill, 0);
        g_array_set_size(in_area->mode_offset, 0);
    }
    else
    {
        //g_print("Creating..\n");

        in_area->fill = g_array_sized_new(FALSE, FALSE, sizeof(GvVertex3d), 1000);
        in_area->mode_offset = 
          g_array_sized_new(FALSE, FALSE, sizeof(gint), 1000);
    }
    gluTessBeginPolygon(tess, (void *)in_area);
    for (i=0; i < gv_shape_get_rings((GvShape *) in_area); ++i)
    {
        node_count = in_area->num_ring_nodes[i];
        xyz_nodes = in_area->xyz_ring_nodes[i];

        gluTessBeginContour(tess);

        for (j=0; j < node_count; ++j)
        {
            coords[0] = xyz_nodes[j*3+0];
            coords[1] = xyz_nodes[j*3+1];
            coords[2] = xyz_nodes[j*3+2];

        /*
            g_print("Adding vertex, ptr %p\n", (xyz_nodes + j*3));
            g_print("   x, y, z: %6.3lf, %6.3lf, %6.3lf\n",
                    (xyz_nodes + j * 3)[0],(xyz_nodes + j * 3)[1],
                    (xyz_nodes + j * 3)[2]);
        */

            gluTessVertex(tess, coords, xyz_nodes + j*3);
        }
        if ((coords[0] != xyz_nodes[0]) || 
            (coords[1] != xyz_nodes[1]) ||
            (coords[2] != xyz_nodes[2])) {
            g_print("area not closed!!\n");
            /*return FALSE;*/
        }

        gluTessEndContour(tess);
    }
    gluTessEndPolygon(tess);

    if( in_area->fill->len == 0 )
        in_area->fill_objects = 0;

    return (in_area->fill_objects > 0 
            && g_array_index(in_area->mode_offset,gint,0) != GV_TESS_NONE);
}

static void WIN_CALLBACK
tess_begin(GLenum type, void *in_area)
{
    GvAreaShape *area = (GvAreaShape *)in_area;

    area->fill_objects++;
    g_array_append_val(area->mode_offset, type );
    g_array_append_val(area->mode_offset, area->fill->len );
}

static void WIN_CALLBACK
tess_end(void)
{
    g_print("end callback getting called..\n");
}

static void WIN_CALLBACK
tess_vertex(void *data, void *in_area)
{

  /* for some unknown reason, we somes get called with a NULL 
     if the object doesn't tesselate properly (with libMesa). */

    GvAreaShape *area = (GvAreaShape *)in_area;

    /*
    g_print("tess_vertex, ptr %p\n", (data));
    */

    if( data != NULL ) {

        /*
        g_print("   x, y, z: %6.3lf, %6.3lf, %6.3lf\n",
                (double)((gvgeocoord *)data)[0],
                (double)((gvgeocoord *)data)[1],
                (double)((gvgeocoord *)data)[2]);
        */

        g_array_append_vals(area->fill, data, 1);
    }
    else {
        tess_error( 0, in_area );
    }
}

static void WIN_CALLBACK
tess_combine(GLdouble coords[3], 
             gvgeocoord *vertex_data[4],
             GLfloat weight[4], void **dataOut)
{
    GLdouble *vertex;
    int ii;

    /*
    g_print("tess_combine x, y, z: %6.3lf, %6.3lf, %6.3lf\n",
            coords[0], coords[1], coords[2]);
    */

    vertex = g_new(GLdouble, 3);
    vertex[0] = vertex_data[0][0] * weight[0];
    vertex[1] = vertex_data[0][1] * weight[0];
    vertex[2] = vertex_data[0][2] * weight[0];

    /*
    g_print("tess_combine x, y, z (%d%%): %6.3lf, %6.3lf, %6.3lf\n",
            (int)(weight[0] * 100), vertex_data[0][0],
            vertex_data[0][1], vertex_data[0][2]);
    */

    for (ii = 1; ii < 4; ii++) {
        if (vertex_data[ii] != NULL) {
            vertex[0] += vertex_data[ii][0] * weight[ii];
            vertex[1] += vertex_data[ii][1] * weight[ii];
            vertex[2] += vertex_data[ii][2] * weight[ii];

            /*
            g_print("tess_combine x, y, z (%d%%): %6.3lf, %6.3lf, %6.3lf\n",
                    (int)(weight[ii] * 100), vertex_data[ii][0],
                    vertex_data[ii][1], vertex_data[ii][2]);
            */
        }
    }

    /*
    g_print("tess_combine returning x, y, z: %6.3lf, %6.3lf, %6.3lf\n",
            vertex[0], vertex[1], vertex[2]);
    */

    *dataOut = vertex; 
}

static void WIN_CALLBACK
tess_error(GLenum err, void *in_area)
{
    GvAreaShape *area = (GvAreaShape *)in_area;
    GLenum bad_fill_mode = GV_TESS_NONE;
    gint   offset = 0;

    g_array_set_size(area->fill, 0);
    g_array_set_size(area->mode_offset, 0);

    area->fill_objects = 1;
    g_array_append_val(area->mode_offset, bad_fill_mode );
    g_array_append_val(area->mode_offset, offset );
}

static gint
check_ring_lengths(GvAreaShape *area)
{
    int r;

    for (r=0; r < area->num_rings; ++r)
    {
        int nodes = area->num_ring_nodes[r];

        if ( nodes < 3) 
            return FALSE;
    }

    return TRUE;
}

static void
check_winding(GvAreaShape *area)
{
    int r, winding;

    for (r=0; r < area->num_rings; ++r)
    {
        winding = find_winding(area->xyz_ring_nodes[r], 
                               area->num_ring_nodes[r]);

        if ((r == 0 && winding == GV_CW) || (r > 0 && winding == GV_CCW))
        {
            reverse_array(area->xyz_ring_nodes[r], 
                          area->num_ring_nodes[r]);
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
find_winding(gvgeocoord *xyz_nodes, int node_count)
{
    gvgeocoord *a, *b;
    gvgeocoord sum;
    int i;

    sum = 0.0;
    a = xyz_nodes + 0;
    b = xyz_nodes + 3;
    for (i=1; i < node_count; ++i, a += 3, b += 3)
    {
        sum += a[0] * b[1] - a[1] * b[0];
    }
    b = xyz_nodes;
    sum += a[0] * b[1] - a[1] * b[0];

    return (sum < 0.0 ? GV_CW : GV_CCW);
}

static void
reverse_array(gvgeocoord *xyz_nodes, int node_count)
{
    int i;
    gvgeocoord   xyz_temp[3];

    for (i=0; i < node_count/2; ++i)
    { 
        memcpy( xyz_temp, xyz_nodes+i*3, 
                sizeof(gvgeocoord)*3 );
        memcpy( xyz_nodes+i*3, xyz_nodes+(node_count-i-1)*3, 
                sizeof(gvgeocoord)*3);
        memcpy( xyz_nodes+(node_count-i-1)*3, xyz_temp, 
                sizeof(gvgeocoord)*3);
    }
}

