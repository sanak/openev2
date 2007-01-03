/******************************************************************************
 * $Id: gvskirt.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Code to generate 3D skirts around a raster layer.  The skirts
 *           are generated as GvShapesLayer.  This feature is initially
 *           implemented for use in CIETmap.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2001, DM Solutions Group (www.dmsolutions.ca)
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
 * $Log: gvskirt.c,v $
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
 * Revision 1.6  2002/10/07 06:08:07  warmerda
 * Avoid unused variable warning.
 *
 * Revision 1.5  2002/09/27 18:17:18  pgs
 * modified skirt building to remove white lines.
 *
 * Revision 1.4  2002/09/12 15:34:41  warmerda
 * use odd modulus for facet counter
 *
 * Revision 1.3  2002/09/11 19:15:37  warmerda
 * changed way of deciding on white facets to avoid angular dependencies
 *
 * Revision 1.2  2001/07/13 22:14:18  warmerda
 * completed implementation
 *
 * Revision 1.1  2001/07/09 20:22:04  warmerda
 * New
 *
 */

#include "gvshapes.h"
#include "gvrasterlayer.h"
#include "cpl_error.h"
#include "gvshapeslayer.h"

static void add_edge( GvShapes *shapes, GvRasterLayer *dem_layer,
                      double base_z,
                      double x1, double y1, double z1,
                      double x2, double y2, double z2 )

{
    GvRaster    *dem = dem_layer->prototype_data;
    GvShape *facet;
    GvColor color;
    GvProperties *prop;
    char    color_prop[128];
    float   z1_mesh = 0, z2_mesh = 0;
    int     success, i;
    double  x_geo, y_geo;
    //static int  facet_counter = 0;
    float alpha = 0.8;

    facet = gv_shape_new( GVSHAPE_AREA );

/* -------------------------------------------------------------------- */
/*      Get sample value from the mesh.                                 */
/* -------------------------------------------------------------------- */
    x_geo = x1;
    y_geo = y1;
    gv_raster_pixel_to_georef( dem, &x_geo, &y_geo, NULL );

    z1_mesh = gv_mesh_get_height( dem_layer->mesh, x_geo, y_geo, &success );
    if( !success )
        z1_mesh = z1;

    x_geo = x2;
    y_geo = y2;
    gv_raster_pixel_to_georef( dem, &x_geo, &y_geo, NULL );

    z2_mesh = gv_mesh_get_height( dem_layer->mesh, x_geo, y_geo, &success );
    if( !success )
        z2_mesh = z2;

/* -------------------------------------------------------------------- */
/*      Setup the color of the facet.  We set some to white to          */
/*      provide a stripped effect.  The rest are set to the color of    */
/*      the raster at that point.                                       */
/* -------------------------------------------------------------------- */
/*    if( ((int) (x1+y1)) % 20 >= 0 && ((int) (x1+y1)) % 20 <= 2 ) */

    //use a transparent skirt instead
    /*
    if( facet_counter++ > 18 ) //facet_counter == 19 ) //facet_counter == 18 || facet_counter == 19)
    {
        if (facet_counter == 19)
            facet_counter = 0;
        color[0] = 1.0;
        color[1] = 1.0;
        color[2] = 1.0;
        color[3] = alpha;
    }
    else
    */
    {
        GvRasterSource *src = dem_layer->source_list + 0;
        int  r_class;

        r_class = (((((z1+z2)/2.0) - src->min)/ (src->max-src->min))*255);
        r_class = MAX(0,MIN(255,r_class));

        if( src->lut )
            r_class = src->lut[r_class];

        if( dem_layer->pc_lut != NULL )
        {
            color[0] = dem_layer->pc_lut[r_class * 4    ] / 255.0;
            color[1] = dem_layer->pc_lut[r_class * 4 + 1] / 255.0;
            color[2] = dem_layer->pc_lut[r_class * 4 + 2] / 255.0;
            //color[3] = dem_layer->pc_lut[r_class * 4 + 3] / 255.0;
            color[3] = alpha;
        }
        else
        {
            color[0] = color[1] = color[2] = r_class / 255.0;
            color[3] = alpha;
        }
    }

    for (i=0;i<3;i++)
        color[i] = color[i] + (1.0 - color[3]) * (1.0 - color[i]);
    color[3] = 1.0;

    prop = gv_shape_get_properties( facet );

    sprintf( color_prop, "%.3f %.3f %.3f %.3f", color[0], color[1],
                                                color[2], color[3] );
    gv_properties_set( prop, "_gv_fill_color", color_prop );
    //disable edges to speed up drawing
    gv_properties_set( prop, "_gv_color", "0.0 0.0 0.0 0.0" );

/* -------------------------------------------------------------------- */
/*      Create a polygon for the facet.                                 */
/* -------------------------------------------------------------------- */
    gv_raster_pixel_to_georef( dem, &x1, &y1, NULL );
    gv_raster_pixel_to_georef( dem, &x2, &y2, NULL );

    gv_shape_add_node( facet, 0, x1, y1, z1_mesh );
    gv_shape_add_node( facet, 0, x1, y1, base_z );
    gv_shape_add_node( facet, 0, x2, y2, base_z );
    gv_shape_add_node( facet, 0, x2, y2, z2_mesh );
    gv_shape_add_node( facet, 0, x1, y1, z1_mesh );

/* -------------------------------------------------------------------- */
/*      Add the shape to the shapes container.                          */
/* -------------------------------------------------------------------- */
    gv_shapes_add_shape( shapes, facet );
}


/************************************************************************/
/*                           gv_build_skirt()                           */
/************************************************************************/
GvLayer *gv_build_skirt(GvRasterLayer *dem_layer, double base_z)
{
    GvShapes    *shapes;
    int     x, y;
    GvRaster    *dem = dem_layer->prototype_data;
    double  no_data = 1000000;
    float   *this_line, *next_line;

    this_line = g_new(float,dem->width+2);
    next_line = g_new(float,dem->width+2);

    if( dem_layer->source_list[0].nodata_active )
        no_data = dem_layer->source_list[0].nodata_real;

    for( x = 0; x < dem->width+2; x++)
        next_line[x] = no_data;

    shapes = GV_SHAPES(gv_shapes_new());

    for( y = -1; y < dem->height; y++ )
    {
        memcpy( this_line, next_line, sizeof(float) * (dem->width+2) );

        if( y+1 < dem->height )
        {
            GDALRasterIO( dem->gdal_band, GF_Read,
                          0, y+1, dem->width, 1,
                          next_line + 1, dem->width, 1, GDT_Float32, 0, 0 );
        }
        else
        {
            for( x = 0; x < dem->width+2; x++)
                next_line[x] = no_data;
        }

        for( x = -1; x < dem->width; x++ )
        {
            double  pix_00, pix_10, pix_01, pix_11;
            int     nd_00, nd_10, nd_01, nd_11;

            /*
            ** Fetch the four local pixels.
            */

            pix_00 = this_line[x+1];
            pix_10 = this_line[x+2];
            pix_01 = next_line[x+1];
            pix_11 = next_line[x+2];

            nd_00 = (pix_00 == no_data);
            nd_01 = (pix_01 == no_data);
            nd_10 = (pix_10 == no_data);
            nd_11 = (pix_11 == no_data);

            if( (nd_00 == nd_01) && (nd_00 == nd_10) && (nd_00 == nd_11) )
                continue;
#ifndef notdef
            if( !nd_00 && nd_01 )
            {
                add_edge( shapes, dem_layer, base_z,
                          x, y+1, pix_00,
                          x+1, y+1, pix_00);
            }
            if( nd_00 && !nd_01 )
            {
                add_edge( shapes, dem_layer, base_z,
                          x+1, y+1, pix_01,
                          x, y+1, pix_01);
            }
            if( !nd_00 && nd_10 )
            {
                add_edge( shapes, dem_layer, base_z,
                          x+1, y+1, pix_00,
                          x+1, y, pix_00);
            }
            if( nd_00 && !nd_10 )
            {
                add_edge( shapes, dem_layer, base_z,
                          x+1, y, pix_10,
                          x+1, y+1, pix_10);
            }
#endif
#ifdef notdef
            if( nd_00 && nd_01 && !nd_10 && !nd_11 )
            {
                /* vertical edge. */

                add_edge( shapes, dem_layer, base_z,
                          x+0.5, y, pix_10,
                          x+0.5, y+1, pix_11 );
            }
            else if( !nd_00 && !nd_01 && nd_10 && nd_11 )
            {
                /* vertical edge, reversed. */

                add_edge( shapes, dem_layer, base_z,
                          x+0.5, y+1, pix_00,
                          x+0.5, y, pix_01 );
            }
            else if( nd_00 && nd_10 && !nd_01 && !nd_11 )
            {
                /* horizontal edge. */

                add_edge( shapes, dem_layer, base_z,
                          x, y+0.5, pix_01,
                          x+1.0, y+0.5, pix_11 );
            }
            else if( !nd_00 && !nd_10 && nd_01 && nd_11 )
            {
                /* horizontal edge, reversed. */

                add_edge( shapes, dem_layer, base_z,
                          x+1.0, y+0.5, pix_00,
                          x, y+0.5, pix_10 );
            }
            else if( nd_00 && !nd_10 && !nd_01 && !nd_11 )
            {
                /* diag, top left missing, reversed */

                add_edge( shapes, dem_layer, base_z,
                          x+0.5, y, pix_10,
                          x, y+0.5, pix_01 );
            }
            else if( !nd_00 && nd_10 && nd_01 && nd_11 )
            {
                /* diag, top left present */

                add_edge( shapes, dem_layer, base_z,
                          x+0.5, y, pix_00,
                          x, y+0.5, pix_00 );
            }
            else if( !nd_00 && !nd_10 && nd_01 && !nd_11 )
            {
                /* diag, bottom left missing */

                add_edge( shapes, dem_layer, base_z,
                          x+0.5, y+1, pix_11,
                          x, y+0.5, pix_00 );
            }
            else if( nd_00 && nd_10 && !nd_01 && nd_11 )
            {
                /* diag, bottom left present */

                add_edge( shapes, dem_layer, base_z,
                          x+0.5, y+1, pix_01,
                          x, y+0.5, pix_01 );
            }
            else if( !nd_00 && nd_10 && !nd_01 && !nd_11 )
            {
                /* diag, top right missing */

                add_edge( shapes, dem_layer, base_z,
                          x+0.5, y, pix_00,
                          x+1, y+0.5, pix_11 );
            }
            else if( nd_00 && !nd_10 && nd_01 && nd_11 )
            {
                /* diag, top right present */

                add_edge( shapes, dem_layer, base_z,
                          x+0.5, y, pix_10,
                          x+1, y+0.5, pix_10 );
            }
            else if( !nd_10 && !nd_10 && !nd_01 && nd_11 )
            {
                /* diag, bottom right missing */

                add_edge( shapes, dem_layer, base_z,
                          x+1, y+0.5, pix_10,
                          x+0.5, y+1, pix_01 );
            }
            else if( nd_00 && nd_10 && nd_01 && !nd_11 )
            {
                /* diag, bottom right present */

                add_edge( shapes, dem_layer, base_z,
                          x+1, y+0.5, pix_11,
                          x+0.5, y+1, pix_11 );
            }
            else
            {
                printf( "%d,%d,%d,%d\n", nd_00, nd_10, nd_01, nd_11 );
            }
#endif
        }
    }

    g_free( this_line );
    g_free( next_line );

/* -------------------------------------------------------------------- */
/*      Turn this into a layer.                                         */
/* -------------------------------------------------------------------- */
    return GV_LAYER(gv_shapes_layer_new(shapes));
}
