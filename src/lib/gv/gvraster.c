/******************************************************************************
 * $Id: gvraster.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Raster data container. 
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
 * $Log: gvraster.c,v $
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
 * Revision 1.75  2004/09/20 13:15:35  pgs
 * added patch for isnan on win32
 *
 * Revision 1.74  2004/09/07 15:21:34  gmwalter
 * Check for nan's as well as nodata
 * when calculating scaling min/max
 * values.
 *
 * Revision 1.73  2004/07/03 07:39:11  andrey_kiselev
 * Grab double floats from the GDAL in gv_raster_get_sample().
 *
 * Revision 1.72  2004/01/22 19:57:10  andrey_kiselev
 * Use gv_raster_get_nodata() function to fetch the NODATA value from the image
 * using GDALGetRasterNoDataValue().
 *
 * Revision 1.71  2003/11/06 14:26:24  gmwalter
 * Avoid mismatch between tiles being downsampled from higher-resolution
 * tiles and tiles being loaded for the first time at lower resolution
 * when overview downsampling method does not match openev's.
 *
 * Revision 1.70  2003/09/11 20:00:29  gmwalter
 * Add ability to specify a preferred polynomial order for warping a raster,
 * and add "safe mode" (only used if ATLANTIS_BUILD is defined).
 *
 * Revision 1.69  2003/06/25 14:45:07  gmwalter
 * Fixed a bug in gv_georef_to_pixel in geotransform case (was ignoring
 * rotational terms).
 *
 * Revision 1.68  2003/03/02 04:43:58  warmerda
 * CInt32 and CFloat64 are complex too!
 *
 * Revision 1.67  2003/02/20 19:27:15  gmwalter
 * Updated link tool to include Diana's ghost cursor code, and added functions
 * to allow the cursor and link mechanism to use different gcps
 * than the display for georeferencing.  Updated raster properties
 * dialog for multi-band case.  Added some signals to layerdlg.py and
 * oeattedit.py to make it easier for tools to interact with them.
 * A few random bug fixes.
 *
 * Revision 1.66  2003/02/07 20:06:50  andrey_kiselev
 * Memory leaks fixed.
 *
 * Revision 1.65  2002/10/29 22:28:26  warmerda
 * fill out tile in reads that are not full resolution
 *
 * Revision 1.64  2002/10/29 05:44:05  warmerda
 * always flood image values to right edge and bottom of tile on load
 *
 * Revision 1.63  2002/10/08 22:56:18  warmerda
 * Modified gv_raster_build_poly_transform() to back off using the highest
 * possible order polynomial if the CRS_compute_georef_equations() call fails.
 * This ensures that underdetermined sets of GCPs (ie. those with linear
 * dependencies) can still produce useful polynomials, even if they are only
 * 1st order.
 *
 * Revision 1.62  2002/02/15 22:10:09  warmerda
 * ensure that setting zero gcps clear the poly transform
 *
 * Revision 1.61  2001/11/29 15:53:42  warmerda
 * added autoscale_samples preference
 *
 * Revision 1.60  2001/11/28 19:18:29  warmerda
 * Added set_gcps(), and get_gcps() methods on GvRaster, and the
 * geotransform-changed signal generated when the gcps change.
 *
 * Revision 1.59  2001/10/17 16:22:38  warmerda
 * added unhandled raster type check
 *
 * Revision 1.58  2001/10/16 18:50:06  warmerda
 * now possible to pass sample set into autoscale
 *
 * Revision 1.57  2001/08/22 02:34:52  warmerda
 * fixed failure to sort samples in some cases for autoscale
 *
 * Revision 1.56  2001/08/15 13:05:57  warmerda
 * modified default autoscale std_dev to 2.5
 *
 * Revision 1.55  2001/08/14 17:03:24  warmerda
 * added standard deviation autoscaling support
 *
 * Revision 1.54  2001/07/24 02:21:54  warmerda
 * added 8bit phase averaging
 *
 * Revision 1.53  2001/07/13 22:15:36  warmerda
 * added nodata aware averaging
 *
 * Revision 1.52  2001/04/02 18:10:46  warmerda
 * expose gv_raster_autoscale() to python
 *
 * Revision 1.51  2001/01/08 17:47:23  warmerda
 * fixed additional window edge conditions in gv_raster_tile_get_gdal
 *
 * Revision 1.50  2000/11/28 02:49:51  warmerda
 * fixed edge handling bugs with rasters smaller than one tile
 *
 * Revision 1.49  2000/11/01 03:47:45  warmerda
 * Fixed serious bug with memory corruption that is mostly likely to occur
 * with large images.  See Bug 120968 on SourceForge.
 *
 * Revision 1.48  2000/09/27 19:18:50  warmerda
 * Honour GvSMSample for real and complex images.
 * Add GvRaster.sm field.  If set to GvSMSample always let GDAL do the
 * decimation for faster loads.
 *
 * Revision 1.47  2000/08/25 20:06:34  warmerda
 * Added support for GDAL bands with arbitrary overviews (ie. OGDI)
 * Avoid having scaling min and max the same.
 *
 * Revision 1.46  2000/08/24 03:37:52  warmerda
 * added PIXEL as a coordinate system
 *
 * Revision 1.45  2000/08/16 14:08:23  warmerda
 * report data name, not file name
 *
 * Revision 1.44  2000/08/09 17:37:13  warmerda
 * debug on finalize
 *
 * Revision 1.43  2000/08/02 19:17:30  warmerda
 * added debug statement
 *
 * Revision 1.42  2000/07/27 20:33:12  warmerda
 * set max lod to 7 instead of 4 for 4x4 textures
 *
 * Revision 1.41  2000/07/18 14:53:54  warmerda
 * go directly to gdal in sample call, if full res raster not available
 *
 * Revision 1.40  2000/07/12 19:26:32  warmerda
 * try to avoid using gcps if geotransform is set
 *
 * Revision 1.39  2000/07/10 14:27:53  warmerda
 * use GRASS derived CRS code instead of Numerical Recipes gvgcpfit code
 *
 * Revision 1.38  2000/06/27 15:46:47  warmerda
 * added gv_closest_gdal_lod to make optimal use of overviews
 *
 * Revision 1.37  2000/06/26 15:12:33  warmerda
 * set name automatically
 *
 * Revision 1.36  2000/06/20 15:26:21  warmerda
 * fixed more free/g_free problems
 *
 * Revision 1.35  2000/06/20 14:37:26  warmerda
 * fixed free/g_free() problem
 *
 * Revision 1.34  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */



#include <stdlib.h>
#include <assert.h>
#include "gvraster.h"
#include "gvrastertypes.h"
#include "gvrasteraverage.h"
#include "gvmanager.h"
#include "cpl_conv.h"

#ifndef __MINGW32__
#ifdef WIN32
#include <float.h>   /* for isnan */
#define isnan _isnan
#endif
#endif

#define USE_CRS
#ifdef USE_CRS
#  include "crs.h"
#else
#  include "gvgcpfit.h"
#endif

enum
{
    GEOTRANSFORM_CHANGED,
    LAST_SIGNAL
};

typedef struct _GvRasterMemento
{
    GvDataMemento base;

    int      x_off;
    int      y_off;
    int      width;
    int      height;

    void     *data;
} GvRasterMemento;

static void gv_raster_class_init(GvRasterClass *klass);
static void gv_raster_init(GvRaster *raster);
static void gv_raster_finalize(GObject *object);
static void gv_raster_changed( GvRaster *raster, void * raw_change_info );
static gint gv_raster_build_poly_transform( GvRaster *raster );
static gint gv_raster_build_poly_transformCL( GvRaster *raster, int poly_order );
static void gv_raster_get_memento(GvData *raster, gpointer info, 
                                  GvDataMemento **memento);
static void gv_raster_set_memento(GvData *raster, GvDataMemento *memento);
static void gv_raster_del_memento(GvData *raster, GvDataMemento *memento);

static int gv_raster_check_poly_order( GvRaster *raster, int poly_order );

static guint raster_signals[LAST_SIGNAL] = { 0 };

GtkType
gv_raster_get_type(void)
{
    static GtkType raster_type = 0;

    if (!raster_type)
    {
    static const GtkTypeInfo raster_info =
    {
        "GvRaster",
        sizeof(GvRaster),
        sizeof(GvRasterClass),
        (GtkClassInitFunc) gv_raster_class_init,
        (GtkObjectInitFunc) gv_raster_init,
        /* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
    };

    raster_type = gtk_type_unique(gv_data_get_type(), &raster_info);
    }
    return raster_type;
}

static void
gv_raster_init(GvRaster *raster)
{
    raster->poly_order = -1;
    raster->poly_pixel_coeff = NULL;
    raster->poly_line_coeff = NULL;
    raster->poly_x_coeff = NULL;
    raster->poly_y_coeff = NULL;
    raster->poly_z_coeff = NULL;

    raster->gcp_count = 0;
    raster->gcp_list = NULL;

    raster->geotransform[0] = 0.0;
    raster->geotransform[1] = 1.0;
    raster->geotransform[2] = 0.0;
    raster->geotransform[3] = 0.0;
    raster->geotransform[4] = 0.0;
    raster->geotransform[5] = 1.0;

    /* Linking/cursor specific transform (special case- default off) */
    raster->poly_orderCL = -1;
    raster->poly_pixel_coeffCL = NULL;
    raster->poly_line_coeffCL = NULL;
    raster->poly_x_coeffCL = NULL;
    raster->poly_y_coeffCL = NULL;
    raster->poly_z_coeffCL = NULL;
    raster->gcp_countCL = 0;
    raster->gcp_listCL = NULL;

}

static void
gv_raster_class_init(GvRasterClass *klass)
{
    GvDataClass *data_class;

    raster_signals[GEOTRANSFORM_CHANGED] =
      g_signal_new ("geotransform-changed",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvRasterClass, geotransform_changed),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 0);

    /* ---- Override finalize ---- */
    (G_OBJECT_CLASS(klass))->finalize = gv_raster_finalize;

    /* GTK2 PORT...
    GtkObjectClass *object_class;

    object_class = (GtkObjectClass*) klass;

    raster_signals[GEOTRANSFORM_CHANGED] =
    gtk_signal_new ("geotransform-changed",
            GTK_RUN_FIRST,
            object_class->type,
            GTK_SIGNAL_OFFSET(GvRasterClass,
                                          geotransform_changed),
            gtk_marshal_NONE__INT,
            GTK_TYPE_NONE, 0);
    gtk_object_class_add_signals(object_class, raster_signals,
                 LAST_SIGNAL);

    object_class->finalize = gv_raster_finalize;
    */

    data_class = (GvDataClass *) klass;
    data_class->get_memento = gv_raster_get_memento;
    data_class->set_memento = gv_raster_set_memento;
    data_class->del_memento = gv_raster_del_memento;



}

static void
gv_raster_finalize(GObject *gobject)
{
    GvRaster    *raster = GV_RASTER(gobject);
    GvRasterClass *parent_class;

    /* GTK2 PORT... Override GObject finalize, test for and set NULLs
       as finalize may be called more than once. */

    CPLDebug( "OpenEV", "gv_raster_finalize(%s)\n", 
              gv_data_get_name(GV_DATA(gobject)) );

    if( raster->cache != NULL ) {
        gv_raster_cache_free( raster->cache );
	raster->cache = NULL;
    }

    if( raster->dataset != NULL && GDALDereferenceDataset( raster->dataset ) < 1 ) {
        GDALClose( raster->dataset );
	raster->dataset = NULL;
    }

    if( raster->poly_order > -1 )
    {
        g_free( raster->poly_pixel_coeff );
        g_free( raster->poly_line_coeff );
        g_free( raster->poly_x_coeff );
        g_free( raster->poly_y_coeff );
        g_free( raster->poly_z_coeff );

	raster->poly_order = -1;
    }

    if( raster->poly_orderCL > -1 )
    {
        g_free( raster->poly_pixel_coeffCL );
        g_free( raster->poly_line_coeffCL );
        g_free( raster->poly_x_coeffCL );
        g_free( raster->poly_y_coeffCL );
        g_free( raster->poly_z_coeffCL );

	raster->poly_orderCL = -1;
    }

    if( raster->gcp_count > 0 )
    {
        GDALDeinitGCPs( raster->gcp_count, raster->gcp_list );
        CPLFree( raster->gcp_list );

	raster->gcp_list = NULL;
	raster->gcp_count = 0;
    }

    if( raster->gcp_countCL > 0 )
    {
        GDALDeinitGCPs( raster->gcp_countCL, raster->gcp_listCL );
        CPLFree( raster->gcp_listCL );

	raster->gcp_listCL = NULL;
	raster->gcp_countCL = 0;
    }

    /* Call parent class function */
    parent_class = gtk_type_class(gv_data_get_type());
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}


static void gv_raster_changed( GvRaster *raster,
                               void * raw_change_info )

{
    GvRasterChangeInfo *change_info = (GvRasterChangeInfo *) raw_change_info;

    if( change_info != NULL )
        gv_raster_flush_cache( raster, 
                               change_info->x_off, change_info->y_off,
                               change_info->width, change_info->height );
    else
        gv_raster_flush_cache( raster, 0, 0, -1, -1 );
}
    
GvData *
gv_raster_new( GDALDatasetH dataset, int real_band, 
               GvSampleMethod sm )
{
    char     *name;
    GvRaster *raster = GV_RASTER(gtk_type_new(gv_raster_get_type()));

    gtk_signal_connect( GTK_OBJECT(raster), "changed",
            (GtkSignalFunc)gv_raster_changed, NULL );

    raster->dataset = dataset;
    GDALReferenceDataset( dataset );

    raster->gdal_band = GDALGetRasterBand(dataset,real_band);

    raster->sm = sm;

    /* set the name */
    name = (char *) g_malloc(strlen(GDALGetDescription(dataset))+8);
    sprintf( name, "%s:%d", GDALGetDescription(dataset), real_band );
    gv_data_set_name( GV_DATA(raster), name );
    g_free( name );

    switch( GDALGetRasterDataType(raster->gdal_band) )
    {
    case GDT_Byte:
          if( sm == GvSMAverage )
              raster->average = (void *(*)(GvRaster*,void*,int,int))
                  gv_raster_byte_real_average;
          else if( sm == GvSMAverage8bitPhase )
              raster->average = (void *(*)(GvRaster*,void*,int,int))
                  gv_raster_byte_realphase_average;
          else
              raster->average = (void *(*)(GvRaster*,void*,int,int))
                  gv_raster_byte_real_sample;
          raster->type = GV_RASTER_BYTE_REAL;
          raster->gdal_type = GDT_Byte;
          break;

        case GDT_CInt16:
        case GDT_CInt32:
        case GDT_CFloat32:
        case GDT_CFloat64:
          if( sm == GvSMAverage )
              raster->average = (void *(*)(GvRaster*,void*,int,int))
                  gv_raster_float_complex_average;
          else
              raster->average = (void *(*)(GvRaster*,void*,int,int))
                  gv_raster_float_complex_sample;
          raster->type = GV_RASTER_FLOAT_COMPLEX;
          raster->gdal_type = GDT_CFloat32;
          break;
          
        default:
          if( sm == GvSMAverage && gv_raster_get_nodata( raster, NULL ) )
              raster->average = (void *(*)(GvRaster*,void*,int,int))
                  gv_raster_float_real_average_nodata;
          else if( sm == GvSMAverage )
              raster->average = (void *(*)(GvRaster*,void*,int,int))
                  gv_raster_float_real_average;
          else
              raster->average = (void *(*)(GvRaster*,void*,int,int))
                  gv_raster_float_real_sample;
          raster->type = GV_RASTER_FLOAT_REAL;
          raster->gdal_type = GDT_Float32;
          break;
    }

    if( raster->type == GV_RASTER_BYTE_REAL 
        || !gv_raster_autoscale(raster,GvASAAutomatic,-1.0,0,NULL,NULL,NULL) )
    {
        raster->min = 0;
        raster->max = 255;
    }

    raster->tile_x = 256;
    raster->tile_y = 256;
    raster->width = GDALGetRasterXSize(dataset);
    raster->height = GDALGetRasterYSize(dataset);
    raster->tiles_across = (raster->width + raster->tile_x-GV_TILE_OVERLAP-1)
        / (raster->tile_x-GV_TILE_OVERLAP);
    raster->tiles_down = (raster->height + raster->tile_y-GV_TILE_OVERLAP-1)
        / (raster->tile_y-GV_TILE_OVERLAP);
    raster->max_lod = 7;
    raster->item_size = GDALGetDataTypeSize(raster->gdal_type) / 8;
    
    if( GDALGetGeoTransform(dataset, raster->geotransform) != CE_None )
    {
        raster->geotransform[0] = 0.0;
        raster->geotransform[1] = 1.0;
        raster->geotransform[2] = 0.0;
        raster->geotransform[3] = 0.0;
        raster->geotransform[4] = 0.0;
        raster->geotransform[5] = 1.0;
    }

    if( GDALGetGCPCount(dataset) > 0
        && raster->geotransform[0] == 0.0
        && raster->geotransform[1] == 1.0
        && raster->geotransform[2] == 0.0
        && raster->geotransform[3] == 0.0
        && raster->geotransform[4] == 0.0
        && raster->geotransform[5] == 1.0 )
    {
        gv_data_set_projection( GV_DATA(raster), 
                                GDALGetGCPProjection( dataset ) );

        gv_raster_set_gcps( raster, 
                            GDALGetGCPCount(dataset), 
                            GDALGetGCPs(dataset) );
    }
    else
    {
        if( EQUAL(GDALGetProjectionRef( dataset ),"") 
            && raster->geotransform[0] == 0.0
            && raster->geotransform[1] == 1.0
            && raster->geotransform[2] == 0.0
            && raster->geotransform[3] == 0.0
            && raster->geotransform[4] == 0.0
            && raster->geotransform[5] == 1.0 )
        {
            gv_data_set_projection( GV_DATA(raster), "PIXEL" );
        }
        else
        {
            gv_data_set_projection( GV_DATA(raster), 
                                    GDALGetProjectionRef( dataset ) );
        }
    }

    raster->max_tiles = raster->tiles_across * raster->tiles_down;
    if( ( raster->cache = gv_raster_cache_new( raster->max_tiles,
                                               raster->max_lod ) ) == NULL )
    {
    g_free( raster );
    return NULL;
    }

    return GV_DATA(raster);
}

gint *
gv_raster_tile_xy_get( GvRaster *raster, gint tile, gint lod, gint *coords )
{
    gint tile_in_x, tile_in_y;

    if( coords == NULL )
    {
    if( ( coords = g_new( int, 4 ) ) == NULL )
    {
        return NULL;
    }
    }

    tile_in_x = tile % raster->tiles_across;

    tile_in_y = tile / raster->tiles_across;

    coords[0] = tile_in_x * (raster->tile_x - GV_TILE_OVERLAP) 
        - GV_TILE_OVERLAP/2;
    coords[1] = tile_in_y * (raster->tile_y - GV_TILE_OVERLAP)
        - GV_TILE_OVERLAP/2;
    coords[2] = coords[0] + raster->tile_x;
    coords[3] = coords[1] + raster->tile_y;

    return coords;
}

static int
gv_raster_tile_get_gdal( GvRaster *raster, GDALRasterBandH band, gint *coords, 
                         void *buffer, int buf_width, int buf_height, 
                         int pixel_offset, int line_offset )
    
{
    int    width, height;
    int    size, factor, i;
    int    win_xoff, win_yoff, win_xsize, win_ysize;
    int    rd_buf_width, rd_buf_height, rd_xoff = 0, rd_yoff = 0;
    unsigned char *buf_u;

    width = coords[2] - coords[0];
    height = coords[3] - coords[1];
    size = buf_width * buf_height * raster->item_size;
    factor = width / buf_width;

    if( coords[0] + width > raster->width
        || coords[1] + height > raster->height )
    {
        width = MIN(width,raster->width - coords[0] );
        rd_buf_width = width / factor;
        height = MIN(height,raster->height - coords[1] );
        rd_buf_height = height / factor;
    }
    else
    {
        rd_buf_width = width / factor;
        rd_buf_height = height / factor;
    }

    win_xoff = coords[0];
    win_yoff = coords[1];
    win_xsize = rd_buf_width * factor;
    win_ysize = rd_buf_height * factor;

    if( win_xsize == 0 || win_ysize == 0 )
    {
        /* We can't provide for the sliver requested, so return doing
           nothing. */
        return TRUE;
    }

    buf_u = (unsigned char *) buffer;

    if( coords[0] < 0 )
    {
        assert( coords[0] == -1 );
        win_xoff += 1;
        win_xsize -= 1;
        rd_buf_width -= 1;
        buf_u += pixel_offset;
        rd_xoff = 1;
    }

    if( coords[1] < 0 )
    {
        assert( coords[1] == -1 );
        win_yoff += 1;
        win_ysize -= 1;
        rd_buf_height -= 1;
        buf_u += line_offset;
        rd_yoff = 1;
    }

#ifdef notdef    
    CPLDebug( "OpenEV-RasterIO", 
              "%s:%d (%d,%d %dx%d) to (%dx%d) of (%dx%d)", 
              GDALGetDescription( raster->dataset ), 
              -1, win_xoff, win_yoff, win_xsize, win_ysize, 
              rd_buf_width, rd_buf_height, buf_width, buf_height ); 
#endif
    
    GDALRasterIO( band, GF_Read, win_xoff, win_yoff, win_xsize, win_ysize,
                  buf_u, rd_buf_width, rd_buf_height,
                  raster->gdal_type, pixel_offset, line_offset );
    buf_u = (unsigned char *) buffer;

    /* do we need to set the left most pixel from one in? */
    if( win_xoff == 0 && coords[0] == -1 )
    {
        for( i = 0; i < buf_height; i++ )
        {
            memcpy( buf_u + i*line_offset, 
                    buf_u + i*line_offset + pixel_offset, 
                    pixel_offset );
        }
    }

    /* do we need to set the top scanline from the second? */
    if( win_yoff == 0 && coords[1] == -1 )
    {
        memcpy( buf_u, buf_u + line_offset, line_offset );
    }

    /* do we need to set the right most real pixel + 1? */
    if( rd_xoff + rd_buf_width < buf_width )
    {
        int     pixel_to_set;
        unsigned char *u_buf_base = (unsigned char *) buffer;
        
        assert( rd_buf_width < buf_width );
        pixel_to_set = rd_buf_width + rd_xoff;
        assert( pixel_to_set >= 1 && pixel_to_set < buf_width );
        
        while( pixel_to_set < buf_width )
        {
            for( i = 0; i < buf_height; i++ )
            {
                assert( i >= 0 && i < raster->tile_y );
                memcpy( u_buf_base + i*line_offset + pixel_to_set*pixel_offset,
                        u_buf_base + i*line_offset + (pixel_to_set-1)*pixel_offset, 
                        pixel_offset );
            }
            pixel_to_set++;
        }
    }

    /* do we need to set the bottom most real pixel + 1? */
    if( rd_yoff + rd_buf_height < buf_height )
    {
        int     line_to_set;
        unsigned char *u_buf_base = (unsigned char *) buffer;

        assert( buf_height != rd_buf_height );
        line_to_set = rd_buf_height + rd_yoff;
        assert( line_to_set >= 1 && line_to_set < buf_height );
        assert( line_offset*factor == raster->item_size * raster->tile_x );

        /* we will actually keep copying the line till we get to the bottom
           of the buffer */
        while( line_to_set < buf_height )
        {
            memcpy( u_buf_base + line_to_set*line_offset,
                    u_buf_base + (line_to_set-1)*line_offset, 
                    line_offset );
            line_to_set++;
        }
    }

    return TRUE;
}

/*
 * Based on the available overviews, identify the closest appropriate
 * lod available from the GDAL band. 
 */

static int gv_closest_gdal_lod( GDALRasterBandH band, int desired_lod )

{
    int  ov_index;
    int  best_lod;
    double best_factor = 1.0;
    double desired_factor = (1 << desired_lod);

#ifdef ATLANTIS_BUILD
    if ((gv_manager_get_preference(gv_get_manager(),"safe_mode") != NULL) &&
        (strcmp(gv_manager_get_preference(gv_get_manager(),"safe_mode"),"on") == 0))
        return 0;
#endif

    if( GDALHasArbitraryOverviews( band ) )
        return desired_lod;

    for( ov_index = 0; ov_index < GDALGetOverviewCount( band ); ov_index++ )
    {
        GDALRasterBandH  oband = GDALGetOverview( band, ov_index );
        double           ofactor;

        ofactor = GDALGetRasterBandXSize( band ) / 
            (double) GDALGetRasterBandXSize( oband );

        /* We already found something better */
        if( ofactor < best_factor )
            continue;

        /* We don't want to upsample by much! */
        if( ofactor > desired_factor * 1.2 )
            continue;

        best_factor = ofactor;
    }

    best_lod = 0;
    while( best_factor*0.8 > (1 << best_lod) )
        best_lod++;

    return best_lod;
}

void *
gv_raster_tile_get( GvRaster *raster, gint tile, gint lod )
{
    gint *coords = NULL;
    gint cur_lod;
    gint i;
    int need_to_free_buffer = 0;
    void *buffer = NULL;
    void *temp_buffer;
    

    if( raster->cache != NULL )
    {
    cur_lod = gv_raster_cache_get_best_lod( raster->cache, tile, lod );

    if( cur_lod == lod )
    {
        return gv_raster_cache_get( raster->cache, tile, lod );
    }
        else if( cur_lod > lod || cur_lod == -1 )
    {
            int     width, height, pixel_offset, line_offset;
            int     buf_width, buf_height;
            int     size, factor;

        coords = gv_raster_tile_xy_get( raster, tile, lod, coords );
            
            width = coords[2] - coords[0];
            height = coords[3] - coords[1];

            /* If sample mode is average, retrieve the closest resolution
               overview tile from gdal and then average down as much as
               necessary; otherwise (sample mode is decimate), let gdal do 
               the downsampling (retrieve the tile from gdal using coordinates 
               for cur_lod=lod). 
            */

            if( raster->sm == GvSMAverage )
                cur_lod = gv_closest_gdal_lod( raster->gdal_band, lod );
            else
                cur_lod = lod;

#ifdef ATLANTIS_BUILD
            /* safe mode ignores overviews */
            if ((gv_manager_get_preference(gv_get_manager(),"safe_mode") != NULL) &&
                (strcmp(gv_manager_get_preference(gv_get_manager(),"safe_mode"),"on") == 0))
            {
                cur_lod = 0;
            }
#endif

            factor = 1 << cur_lod;
            buf_width = width / factor;
            buf_height = height / factor;

            size = buf_width * buf_height * raster->item_size;
            buffer = g_malloc(size);
            
            pixel_offset = raster->item_size;
            line_offset = buf_width * raster->item_size;

            if( coords[0] + width > raster->width
                || coords[1] + height > raster->height 
                || coords[0] < 0 || coords[1] < 0 )
                memset( buffer, 0, size );
                
            if( buffer != NULL )
            {
                if( !gv_raster_tile_get_gdal( raster, raster->gdal_band,
                                              coords, 
                                              buffer, buf_width, buf_height,
                                              pixel_offset, line_offset ) )
                {
                    g_free( buffer );
                    return NULL;
                }
            }
        
        if ( coords )
        {
        g_free( coords );
        coords = NULL;
        }
            need_to_free_buffer = 1;

    } else {
            int     width, height, pixel_offset, line_offset;
            int     buf_width, buf_height;
            int     size, factor, gdal_lod;

            /* When a tile is loaded, gdal returns overview values 
               if present; otherwise it downsamples.  To avoid a
               mismatch between tiles downsampled from higher
               resolution tiles and tiles being loaded at lower
               resolution from scratch, check for a gdal overview
               close to this lod before downsampling the higher
               resolution tile.  This code used to point buffer to
               the higher-resolution tile which then got downsampled
               in the for-loop below, but this caused display issues in
               some cases (eg. if overviews were generated
               using averaging, but openev's overview sampling is
               set to decimate, then zooming out on an image with averaged
               overviews will result in the area just zoomed out from
               looking specklier than the surrounding areas
               because the surroundings are loaded from the low-res averaged
               overviews, but the area just zoomed out is decimated
               from a high res averaged overview).  Now, this is changed so the
               code first checks for a gdal overview that is better for 
               downsampling than the current closest resolution cached
               tile, but uses the cached tile if no better overview
               is found.  This should hopefully avoid display 
               inconsistencies, but still make use of the cached tile
               if no overviews are present rather than downsampling
               from scratch (otherwise this else could go entirely,
               and the else if above could handle all cases where
               cur_lod != lod).               
            */
            gdal_lod = gv_closest_gdal_lod( raster->gdal_band, lod );
            if ((gdal_lod <= cur_lod) || (gdal_lod > lod))
            {
                /* No better overviews available to downsample */
            buffer = gv_raster_cache_get( raster->cache, tile, cur_lod );
            }
            else
            {
                cur_lod=gdal_lod;
                coords = gv_raster_tile_xy_get( raster, tile, lod, coords );
            
                width = coords[2] - coords[0];
                height = coords[3] - coords[1];

                if ((gv_manager_get_preference(gv_get_manager(),"safe_mode") != NULL) &&
                (strcmp(gv_manager_get_preference(gv_get_manager(),"safe_mode"),"on") == 0))
                {
                    cur_lod = 0;
                }

                factor = 1 << cur_lod;
                buf_width = width / factor;
                buf_height = height / factor;

                size = buf_width * buf_height * raster->item_size;
                buffer = g_malloc(size);
            
                pixel_offset = raster->item_size;
                line_offset = buf_width * raster->item_size;

                if( coords[0] + width > raster->width
                    || coords[1] + height > raster->height 
                    || coords[0] < 0 || coords[1] < 0 )
                    memset( buffer, 0, size );
                
                if( buffer != NULL )
                {
                    if( !gv_raster_tile_get_gdal( raster, raster->gdal_band,
                                                  coords, 
                                                  buffer, buf_width, buf_height,
                                                  pixel_offset, line_offset ) )
                    {
                        g_free( buffer );
                        return NULL;
                    }
                }
        
            if ( coords )
            {
            g_free( coords );
                coords = NULL;
            }
                need_to_free_buffer = 1;
            }
    }

    for( i = cur_lod; i < lod; i++ )
    {
        temp_buffer = raster->average( raster,
                                           buffer, raster->tile_x >> i,
                                           raster->tile_y >> i );

        if( need_to_free_buffer )
        {
        g_free( buffer );
        } else {
        need_to_free_buffer = 1;
        }

        buffer = temp_buffer;
    }

        if( buffer != NULL )
            gv_raster_cache_put( raster->cache, tile, lod, buffer,
                                 (raster->item_size * ( raster->tile_x >> lod )
                                  * ( raster->tile_y >> lod ) ) );
    
    return buffer;
    }

    return NULL;
}

void
gv_raster_flush_cache( GvRaster *raster, 
                       int x_off, int y_off, int width, int height )
{
    if( raster->gdal_band != NULL )
        GDALFlushRasterCache( raster->gdal_band );

    if( width < 1 || height < 1 )
        gv_raster_cache_flush_all( raster->cache );
    else
    {
        gint tile, lod;

        for( tile = 0; tile < raster->cache->max_tiles; tile++ )
        {
            gint coords[4];

            gv_raster_tile_xy_get( raster, tile, 0, coords );

            if( x_off < coords[2] && y_off < coords[3] 
                && x_off+width > coords[0] && y_off+height > coords[1] )
            {
                for( lod = 0; lod < raster->cache->max_lod; lod++ )
                    gv_raster_cache_del( raster->cache, tile, lod );
            }
        }
    }
}

/************************************************************************/
/*                         gv_raster_get_nodata()                       */
/************************************************************************/

/**
 * Queries NODATA value for the specified raster.
 *
 * @param raster Pointer to GvRaster object.
 *
 * @param nodata Pointer to the variable which should be filled with the
 * NODATA value.
 *
 * @return TRUE if specified raster object has NODATA set and FALSE otherwise.
 */

gint
gv_raster_get_nodata(GvRaster *raster, double *nodata )

{
    int nodata_set = FALSE;

    g_return_val_if_fail( GV_IS_RASTER( raster ), FALSE );
    g_return_val_if_fail( raster != NULL, FALSE);

    if ( nodata )
        *nodata = GDALGetRasterNoDataValue( raster->gdal_band, &nodata_set );
    else
        GDALGetRasterNoDataValue( raster->gdal_band, &nodata_set );

    if( !nodata_set )
        return FALSE;

    return TRUE;
}

gint
gv_raster_get_sample(GvRaster *raster, double x, double y, 
                     double *real, double *imaginary )

{
    gint      pixel, line, tile_x_off, tile_y_off;
    gint      tile, x_within_tile, y_within_tile;
    void      *data;

    pixel = (int) floor(x);
    line = (int) floor(y);

    if( pixel < 0 || line < 0 
        || pixel >= raster->width || line >= raster->height )
        return FALSE;

    tile_x_off = pixel / (raster->tile_x-GV_TILE_OVERLAP);
    tile_y_off = line / (raster->tile_y-GV_TILE_OVERLAP);
    x_within_tile = pixel 
        - (tile_x_off * (raster->tile_x-GV_TILE_OVERLAP) - GV_TILE_OVERLAP/2);
    y_within_tile = line 
        - (tile_y_off * (raster->tile_y-GV_TILE_OVERLAP) - GV_TILE_OVERLAP/2);

    tile = tile_x_off + tile_y_off * raster->tiles_across;
    if( gv_raster_cache_get_best_lod( raster->cache, tile, 0 ) == 0 )
    {
        data = gv_raster_tile_get( raster, tile, 0 );

        data = ((unsigned char *) data) 
       + (x_within_tile + y_within_tile * raster->tile_x) * raster->item_size;

        switch( raster->type )
        {
          case GV_RASTER_BYTE_REAL:
            *real = ((unsigned char *) data)[0];
            *imaginary = 0.0;
            break;

          case GV_RASTER_BYTE_COMPLEX:
            *real = ((unsigned char *) data)[0];
            *imaginary = ((unsigned char *) data)[1];
            break;

          case GV_RASTER_FLOAT_REAL:
            *real = ((float *) data)[0];
            *imaginary = 0.0;
            break;

          case GV_RASTER_FLOAT_COMPLEX:
            *real = ((float *) data)[0];
            *imaginary = ((float *) data)[1];
            break;

          default:
            printf( "Unsupported raster type in gv_raster_get_sample().\n" );
            return FALSE;
        }
    }
    else
    {
        /* use GDAL directly to get value, on assumption it can do a single
           pixel more efficiently than a whole GvRaster tile */

        double   value[2];

        GDALRasterIO( raster->gdal_band, GF_Read, pixel, line, 1, 1, 
                      value, 1, 1, GDT_CFloat64, 0, 0 );

        *real = value[0];
        *imaginary = value[1];
    }

    return TRUE;
}

static int 
gv_float_compare( const void *float1, const void *float2 )
{
    const float *f1 = (float *) float1;
    const float *f2 = (float *) float2;

    if( *f1 < *f2 )
        return -1;
    else if( *f1 > *f2 )
        return 1;
    else
        return 0;
}

gint
gv_raster_autoscale( GvRaster *raster, GvAutoScaleAlg alg, double alg_param,
                     int sample_count, float *sample_set,
                     double *min_out, double *max_out )


{
    int        local_samples = FALSE;
    int        tail_size, i, sorted = FALSE;
    double     no_data, raster_min=0.0, raster_max=255.0;

/* -------------------------------------------------------------------- */
/*      Collect a set of sample points.                                 */
/* -------------------------------------------------------------------- */
    if( sample_set == NULL )
    {
        local_samples = TRUE;
        if( gv_manager_get_preference(gv_get_manager(),"autoscale_samples") )
        {
            sample_count = atoi(
              gv_manager_get_preference(gv_get_manager(),"autoscale_samples"));
            sample_count = MAX(10,sample_count);
        }
        else
            sample_count = 10000;

        sample_set = g_new(float,sample_count);
        sample_count = GDALGetRandomRasterSample(raster->gdal_band, 
                                                 sample_count, sample_set );
    }

/* -------------------------------------------------------------------- */
/*      Strip out any nodata values found.                              */
/* -------------------------------------------------------------------- */
    if( gv_raster_get_nodata( raster, &no_data ) )
    {
        int j = 0;

        for( i = 0; i < sample_count; i++ )
        {
            if( ( sample_set[i] != (float)no_data ) &&
                !(isnan(sample_set[i])) )
                sample_set[j++] = sample_set[i];
        }

        sample_count = j;
    }

/* -------------------------------------------------------------------- */
/*      If we didn't find a nodata value, try stripping away the        */
/*      most extreme value after sorting.  This should let us get       */
/*      rid of stuff that is likely a nodata value, but in a            */
/*      classified image will unfortunately get rid of the lowest       */
/*      and highest classes.                                            */
/* -------------------------------------------------------------------- */
    else if( sample_count > 2 )
    {
        int j = 0;

        for( i = 0; i < sample_count; i++ )
        {
            if ( !(isnan(sample_set[i])) )
                sample_set[j++] = sample_set[i];
        }

        sample_count = j;

        qsort( sample_set, sample_count, sizeof(float), gv_float_compare );
        sorted = TRUE;

        if( sample_set[0] != sample_set[sample_count-1] )
        {
            for( i = 1; 
                 i < sample_count && sample_set[i] == sample_set[0]; 
                 i++ ) {}

            if( i > 1 )
            {
                memmove( sample_set, sample_set + i, 
                         sizeof(float) * (sample_count - i) );
                sample_count -= i;
            }
        }

        if( sample_set[0] != sample_set[sample_count-1] )
        {
            for( i = 1; 
                 i < sample_count 
                     && sample_set[sample_count-i-1] == sample_set[sample_count-1]; 
                 i++ ) {}

            if( i > 1 )
                sample_count -= i;
        }
    }

    if( sample_count < 2 )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Determine the algorithm to use.                                 */
/* -------------------------------------------------------------------- */
    if( alg == GvASAAutomatic )
    {
        const char *alg_str;

        alg_str= gv_manager_get_preference(gv_get_manager(),"scale_algorithm");

        if( alg_str != NULL && EQUAL(alg_str,"percent_tail_trim") )
        {
            alg = GvASAPercentTailTrim;
            alg_param = -1.0;
        }
        else if( alg_str != NULL && EQUAL(alg_str,"std_deviation") )
        {
            alg = GvASAStdDeviation;
            alg_param = -1.0;
        }
        else
        {
#ifdef ATLANTIS_BUILD
            alg = GvASAStdDeviation;
#else
            alg = GvASAPercentTailTrim;
#endif
            alg_param = -1.0;
        }
    }

/* -------------------------------------------------------------------- */
/*      Determine the parameter value to use.  A -1 means use what's    */
/*      in the preferences.                                             */
/* -------------------------------------------------------------------- */
    if( alg_param < -0.9 && alg == GvASAStdDeviation )
    {
        const char *alg_param_str;
        
        alg_param = 2.5;
        alg_param_str = 
            gv_manager_get_preference(gv_get_manager(),
                                      "scale_std_deviations");
        if( alg_param_str != NULL )
            alg_param = atof(alg_param_str);
    }
    else if( alg_param < -0.9 && alg == GvASAPercentTailTrim )
    {
        const char *alg_param_str;
        
        alg = GvASAPercentTailTrim;
        alg_param = 0.02;
        alg_param_str = 
            gv_manager_get_preference(gv_get_manager(),"scale_percent_tail");
        if( alg_param_str != NULL )
            alg_param = atof(alg_param_str);
    }

/* -------------------------------------------------------------------- */
/*      Implementation for percent tail trim method.                    */
/* -------------------------------------------------------------------- */
    if( alg == GvASAPercentTailTrim )
    {
        if( !sorted )
        {
            qsort( sample_set, sample_count, sizeof(float), gv_float_compare );
            sorted = TRUE;
        }
        
        if( alg_param > 0.5 )
            alg_param = alg_param / 100.0;

        tail_size = (int) (alg_param * sample_count);
        
        raster_min = sample_set[tail_size];
        raster_max = sample_set[sample_count - tail_size - 1];
    }

/* -------------------------------------------------------------------- */
/*      Implement standard deviation method.                            */
/* -------------------------------------------------------------------- */
    else if( alg == GvASAStdDeviation )
    {
        double  sum = 0.0, sum_squares = 0.0, mean, std_dev;

        /* compute the std deviation & mean */

        for( i = 0; i < sample_count; i++ )
            sum += sample_set[i];

        mean = sum / sample_count;

        for( i = 0; i < sample_count; i++ )
            sum_squares += (mean - sample_set[i]) * (mean - sample_set[i]);

        std_dev = sqrt(sum_squares / (sample_count - 1));

        raster_min = mean - std_dev * alg_param;
        raster_max = mean + std_dev * alg_param;
    }

/* -------------------------------------------------------------------- */
/*      Ensure we don't end up with a degenerate range.                 */
/* -------------------------------------------------------------------- */
    if( raster_min == raster_max )
    {
        raster_min = raster_min - 0.5;
        raster_max = raster_max + 0.5;
    }

/* -------------------------------------------------------------------- */
/*      Return results or assign directly to raster.                    */
/* -------------------------------------------------------------------- */
    if( min_out != NULL && max_out != NULL )
    {
        *min_out = raster_min;
        *max_out = raster_max;
    }
    else
    {
        raster->min = raster_min;
        raster->max = raster_max;
    }

    if( local_samples )
        g_free( sample_set );

    return TRUE;
}

/************************************************************************/
/*                  gv_raster_collect_random_sample()                   */
/*                                                                      */
/*      Collect random samples from available loaded tiles.             */
/************************************************************************/

int  gv_raster_collect_random_sample( GvRaster *raster, 
                                      int sample_count, float *sample_set,
                                      int xoff, int yoff, int xsize, int ysize,
                                      GArray *tile_list )

{
    int     result, tile, as_count, itile, cs_count;
    float       sampling_rate;

    if( xoff >= raster->width || yoff >= raster->height
        || xoff + xsize < 0 || yoff + ysize < 0 )
        return 0;

/* -------------------------------------------------------------------- */
/*      If we don't have a tile list, create one locally from the       */
/*      bounding rectangle.                                             */
/* -------------------------------------------------------------------- */
    if( tile_list == NULL )
    {
        tile_list = g_array_new( FALSE, FALSE, sizeof(int) );

        for( tile = 0; tile < raster->max_tiles; tile++ )
        {
            gint  coords[4];
            
            gv_raster_tile_xy_get( raster, tile, 0, coords );
            if( coords[0] > xoff + xsize || coords[2] < xoff
                || coords[1] > yoff + ysize || coords[3] < yoff )
                continue;

            g_array_append_val( tile_list, tile );
        }

        result = gv_raster_collect_random_sample( 
            raster, sample_count, sample_set, 
            xoff, yoff, xsize, ysize, 
            tile_list );

        g_array_free( tile_list, TRUE );

        return result;
    }

/* -------------------------------------------------------------------- */
/*      Count available values so we can work out a sampling rate.      */
/* -------------------------------------------------------------------- */
    as_count = 0;
    for( itile = 0; itile < tile_list->len; itile++ )
    {
        int lod, lod_factor;
        gint    coords[4];

        /* do we have this tile? Pick largested avail tile */

        tile = ((int *) tile_list->data)[itile];
        if( tile == -1 )
            continue;

        for( lod = 0; lod < raster->max_lod; lod++ )
        {
            if( raster->cache->tiles[lod][tile] != NULL )
                break;
        }

        if( lod == raster->max_lod )
            continue;

        /* compute the applicable region in full res */
        gv_raster_tile_xy_get( raster, tile, lod, coords );

        if( xoff > coords[2] || yoff > coords[3] 
            || xoff + xsize < coords[0] || yoff + ysize < coords[1] )
            continue;
        
        if( xoff > coords[0] )
            coords[0] = xoff; 
        if( yoff > coords[1] )
            coords[1] = yoff;
        if( xoff+xsize < coords[2] )
            coords[2] = xoff+xsize;
        if( yoff+ysize < coords[3] )
            coords[3] = yoff+ysize;

        if( lod > 0 )
            lod_factor = 2 << (lod-1);
        else
            lod_factor = 1;

        as_count += (((coords[2] - coords[0]) / lod_factor)
                   * ((coords[3] - coords[1]) / lod_factor));
    }

/* -------------------------------------------------------------------- */
/*      Compute sampling rate.                                          */
/* -------------------------------------------------------------------- */
    if( as_count <= sample_count )
        sampling_rate = 1;
    else
        sampling_rate = as_count / (double) sample_count;

/* -------------------------------------------------------------------- */
/*      collect samples from tiles.                                     */
/* -------------------------------------------------------------------- */
    cs_count = 0;

    for( itile = 0; itile < tile_list->len; itile++ )
    {
        int lod, lod_factor, i, x, y, lod_xsize, lod_ysize;
        gint    coords[4];
        float   fi;
        void    *data = NULL;

        /* do we have this tile? Pick largested avail tile */

        tile = ((int *) tile_list->data)[itile];
        if( tile == -1 )
            continue;

        for( lod = 0; lod < raster->max_lod; lod++ )
        {
            if( raster->cache->tiles[lod][tile] != NULL )
            {
                data = raster->cache->tiles[lod][tile]->data;
                break;
            }
        }

        if( data == NULL )
            continue;

        if( lod > 0 )
            lod_factor = 2 << (lod-1);
        else
            lod_factor = 1;

        /* compute the applicable region in full res */
        gv_raster_tile_xy_get( raster, tile, lod, coords );

        if( xoff > coords[2] || yoff > coords[3] 
            || xoff + xsize < coords[0] || yoff + ysize < coords[1] )
            continue;

        as_count = (((coords[2] - coords[0]) / lod_factor)
                   * ((coords[3] - coords[1]) / lod_factor));

        lod_xsize = (coords[2] - coords[0]) / lod_factor;
        lod_ysize = (coords[2] - coords[0]) / lod_factor;

        for( fi = 0.5; fi < as_count; fi += sampling_rate )
        {
            i = (int) fi;

            x = coords[0] + (i % lod_xsize) * lod_factor;
            y = coords[1] + (i / lod_xsize) * lod_factor;

            if( x < xoff || x >= xoff+xsize || y < yoff || y >= yoff+ysize )
                continue;
            
            if( cs_count == sample_count )
                continue;

            if( raster->type == GV_RASTER_BYTE_REAL )
                sample_set[cs_count++] = ((unsigned char *) data)[i];
            else if( raster->type == GV_RASTER_FLOAT_COMPLEX )
            {
                float   real, imag;

                real = ((float *) data)[i*2];
                imag = ((float *) data)[i*2+1];
                sample_set[cs_count++] = sqrt(real*real + imag*imag);
            }
            else if( raster->type == GV_RASTER_FLOAT_REAL )
                sample_set[cs_count++] = ((float *) data)[i];
        }
    }

    return cs_count;
}

/************************************************************************/
/*                    gv_raster_collect_histogram()                     */
/*                                                                      */
/*      Collect histogram information from available loaded tiles.      */
/************************************************************************/

int  gv_raster_collect_histogram( GvRaster *raster, 
                                  double scale_min, double scale_max, 
                                  int bucket_count, int *histogram,
                                  int include_out_of_range,
                                  int xoff, int yoff, int xsize, int ysize,
                                  GArray *tile_list )

{
    int     tile, itile, cs_count, tile_values, i;
    float   scale_coef;

    if( xoff >= raster->width || yoff >= raster->height
        || xoff + xsize < 0 || yoff + ysize < 0 )
        return 0;

    if( (scale_max - scale_min) == 0.0 )
        return 0;

    scale_coef = bucket_count / (scale_max - scale_min);

/* -------------------------------------------------------------------- */
/*      If we don't have a tile list, create one locally from the       */
/*      bounding rectangle.                                             */
/* -------------------------------------------------------------------- */
    if( tile_list == NULL )
    {
        int result;

        tile_list = g_array_new( FALSE, FALSE, sizeof(int) );

        for( tile = 0; tile < raster->max_tiles; tile++ )
        {
            gint  coords[4];
            
            gv_raster_tile_xy_get( raster, tile, 0, coords );
            if( coords[0] > xoff + xsize || coords[2] < xoff
                || coords[1] > yoff + ysize || coords[3] < yoff )
                continue;

            g_array_append_val( tile_list, tile );
        }

        result = gv_raster_collect_histogram( 
            raster, scale_min, scale_max, bucket_count, histogram, 
            include_out_of_range,
            xoff, yoff, xsize, ysize, 
            tile_list );

        g_array_free( tile_list, TRUE );

        return result;
    }

/* -------------------------------------------------------------------- */
/*      Zero histogram.                                                 */
/* -------------------------------------------------------------------- */
    for( i = 0; i < bucket_count; i++ )
        histogram[i] = 0;

/* -------------------------------------------------------------------- */
/*      collect samples from tiles.                                     */
/* -------------------------------------------------------------------- */
    cs_count = 0;

    for( itile = 0; itile < tile_list->len; itile++ )
    {
        int lod, lod_factor, x, y, lod_xsize, lod_ysize;
        gint    coords[4];
        void    *data = NULL;

        /* do we have this tile? Pick largested avail tile */

        tile = ((int *) tile_list->data)[itile];
        if( tile == -1 )
            continue;

        for( lod = 0; lod < raster->max_lod; lod++ )
        {
            if( raster->cache->tiles[lod][tile] != NULL )
            {
                data = raster->cache->tiles[lod][tile]->data;
                break;
            }
        }

        if( data == NULL )
            continue;

        if( lod > 0 )
            lod_factor = 2 << (lod-1);
        else
            lod_factor = 1;

        /* compute the applicable region in full res */
        gv_raster_tile_xy_get( raster, tile, lod, coords );

        if( xoff > coords[2] || yoff > coords[3] 
            || xoff + xsize < coords[0] || yoff + ysize < coords[1] )
            continue;

        lod_xsize = (coords[2] - coords[0]) / lod_factor;
        lod_ysize = (coords[2] - coords[0]) / lod_factor;
        tile_values = lod_xsize * lod_ysize;

        for( i = 0; i < tile_values; i++ )
        {
            float   value;

            x = coords[0] + (i % lod_xsize) * lod_factor;
            y = coords[1] + (i / lod_xsize) * lod_factor;

            if( x < xoff || x >= xoff+xsize || y < yoff || y >= yoff+ysize )
                continue;
            
            if( raster->type == GV_RASTER_BYTE_REAL )
                value = ((unsigned char *) data)[i];
            else if( raster->type == GV_RASTER_FLOAT_COMPLEX )
            {
                float   real, imag;

                real = ((float *) data)[i*2];
                imag = ((float *) data)[i*2+1];
                value = sqrt(real*real + imag*imag);
            }
            else if( raster->type == GV_RASTER_FLOAT_REAL )
                value = ((float *) data)[i];
            else
            {
                assert( FALSE );
                value = 0.0;
            }

            if( value < scale_min )
            {
                if( include_out_of_range )
                {
                    histogram[0]++;
                    cs_count++;
                }
            }
            else if( value > scale_max )
            {
                if( include_out_of_range )
                {
                    histogram[bucket_count-1]++;
                    cs_count++;
                }
            }
            else
            {
                int bucket;

                bucket = (int) ((value - scale_min) * scale_coef);
                bucket = MIN(bucket,bucket_count-1);
                histogram[bucket]++;
                cs_count++;
            }
        }
    }

    return cs_count;
}

/* Approximate width or height of central pixel in georef coordinates */

double gv_raster_pixel_size( GvRaster *raster )

{
    double  x1, y1, x2, y2;

    x1 = raster->width / 2; 
    y1 = raster->height / 2;

    x2 = x1 + 1.0;
    y2 = y1 + 1.0;
    
    gv_raster_pixel_to_georef( raster, &x1, &y1, NULL );
    gv_raster_pixel_to_georef( raster, &x2, &y2, NULL );

    return (ABS(x2-x1) + ABS(y2-y1)) * 0.5;
}

static void
gv_raster_get_memento(GvData *data, gpointer change_info,
                      GvDataMemento **memento)
{
    GvRaster       *raster = GV_RASTER(data);
    GvRasterMemento *mem;
    GvRasterChangeInfo *info = (GvRasterChangeInfo *) change_info;

    *memento = NULL;
    if( change_info == NULL )
        return;

    mem = g_new(GvRasterMemento, 1);
    mem->base.data = data;
    mem->base.type = info->change_type;

    mem->x_off = info->x_off;
    mem->y_off = info->y_off;
    mem->width = info->width;
    mem->height = info->height;

    mem->data = g_malloc(info->width * info->height * raster->item_size);
    if( mem->data == NULL )
        return;

    GDALRasterIO( raster->gdal_band, GF_Read, 
                  mem->x_off, mem->y_off, mem->width, mem->height, 
                  mem->data, mem->width, mem->height, raster->gdal_type, 
                  0, 0 );

    *memento = (GvDataMemento*)mem;
}

static void
gv_raster_set_memento(GvData *data, GvDataMemento *data_memento)
{
    GvRaster       *raster = GV_RASTER(data);
    GvRasterMemento *mem = (GvRasterMemento *) data_memento;
    GvRasterChangeInfo change_info;

    GDALRasterIO( raster->gdal_band, GF_Write, 
                  mem->x_off, mem->y_off, mem->width, mem->height, 
                  mem->data, mem->width, mem->height, raster->gdal_type, 
                  0, 0 );

    change_info.change_type = GV_CHANGE_REPLACE;
    change_info.x_off = mem->x_off;
    change_info.y_off = mem->y_off;
    change_info.width = mem->width;
    change_info.height = mem->height;
    gv_data_changed( GV_DATA(raster), &change_info );

    gv_raster_del_memento( data, data_memento );
}

static void
gv_raster_del_memento(GvData *data, GvDataMemento *data_memento)
{
    GvRasterMemento *memento = (GvRasterMemento *) data_memento;
    
    if (memento->data)
        g_free( memento->data );

    g_free(memento);    
}

/************************************************************************/
/*                         gv_raster_set_gcps()                         */
/************************************************************************/

int gv_raster_set_gcps( GvRaster *raster, int gcp_count, 
                        const GDAL_GCP *gcps )

{
    int success;

    if( raster->gcp_count > 0 )
    {
        GDALDeinitGCPs( raster->gcp_count, raster->gcp_list );
        CPLFree( raster->gcp_list );
        raster->gcp_list = NULL;
    }

    raster->gcp_count = gcp_count;
    if( gcp_count == 0 )
    {
        raster->poly_order = -1;
        gtk_signal_emit(GTK_OBJECT(raster), 
                        raster_signals[GEOTRANSFORM_CHANGED]);
        return TRUE;
    }

    raster->gcp_list = GDALDuplicateGCPs( gcp_count, gcps );

    success = gv_raster_build_poly_transform( raster );

    if( !success )
        gtk_signal_emit(GTK_OBJECT(raster), 
                        raster_signals[GEOTRANSFORM_CHANGED]);

    return success;
}

int gv_raster_set_gcpsCL( GvRaster *raster, int gcp_count, 
                        const GDAL_GCP *gcps,int poly_order )

{
    int success;

    if( raster->gcp_countCL > 0 )
    {
        GDALDeinitGCPs( raster->gcp_countCL, raster->gcp_listCL );
        CPLFree( raster->gcp_listCL );
        raster->gcp_listCL = NULL;
    }

    raster->gcp_countCL = gcp_count;
    if( gcp_count == 0 )
    {
        raster->poly_orderCL = -1;
        return TRUE;
    }

    raster->gcp_listCL = GDALDuplicateGCPs( gcp_count, gcps );

    success = gv_raster_build_poly_transformCL( raster,poly_order );

    return success;
}

/************************************************************************/
/*                      gv_raster_get_gcp_count()                       */
/************************************************************************/

int gv_raster_get_gcp_count( GvRaster *raster )

{
    return raster->gcp_count;
}

int gv_raster_get_gcp_countCL( GvRaster *raster )

{
    return raster->gcp_countCL;
}

/************************************************************************/
/*                         gv_raster_get_gcps()                         */
/************************************************************************/

const GDAL_GCP *gv_raster_get_gcps( GvRaster *raster )

{
    return raster->gcp_list;
}

const GDAL_GCP *gv_raster_get_gcpsCL( GvRaster *raster )

{
    return raster->gcp_listCL;
}

GDALDatasetH gv_raster_get_dataset(GvRaster *raster) {
    return raster->dataset;
}

/* ==================================================================== */
/*      USE_CRS: GCP fitting using GRASS derived crs.c/h code.          */
/* ==================================================================== */

#if defined(USE_CRS)

/* build polynomial coefficients based on gcps */

static gint gv_raster_build_poly_transform( GvRaster *raster )

{
    double  *pixel, *line, *x, *y;
#ifdef ATLANTIS_BUILD
    double   diff_x=0.0, diff_y=0.0;
#endif
    int         i, *status, success;
    struct Control_Points cps;

    int poly_order_pref=1; /* polynomial order preference- only used if preference is set */

    /* defaults */
#ifdef ATLANTIS_BUILD
    if( raster->gcp_count == 0 )
        return FALSE;
#else
    if( raster->gcp_count < 3 )
        return FALSE;
#endif

    if( raster->gcp_count >= 10 )
        raster->poly_order = 2; /* 3rd order not too stable with CRS code */
    else if( raster->gcp_count >= 6 )
        raster->poly_order = 2;
    else
        raster->poly_order = 1;


    /* User has specified a preference for polynomial order.  Do a sanity */
    /* check, then use it if it's reasonable */
    if (gv_data_get_property( GV_DATA(raster), "poly_order_preference" ) != NULL )
    {
        poly_order_pref = atoi(gv_data_get_property( GV_DATA(raster), "poly_order_preference" ));
        raster->poly_order = gv_raster_check_poly_order(raster,poly_order_pref);
        if (poly_order_pref != raster->poly_order)
        {
            g_warning("gv_raster_build_poly_transform(): Inappropriate polynomial order setting.");
        }

    }

    pixel = g_new(double,raster->gcp_count);
    line = g_new(double,raster->gcp_count);
    x = g_new(double,raster->gcp_count);
    y = g_new(double,raster->gcp_count);
    status = g_new(int,raster->gcp_count);

    for( i = 0; i < raster->gcp_count; i++ )
    {
        x[i] = raster->gcp_list[i].dfGCPX;
        y[i] = raster->gcp_list[i].dfGCPY;
        pixel[i] = raster->gcp_list[i].dfGCPPixel;
        line[i] = raster->gcp_list[i].dfGCPLine;
        status[i] = 1; /* use this point */
    }

    raster->poly_x_coeff = g_new(double,20);
    raster->poly_y_coeff = g_new(double,20);
    raster->poly_z_coeff = NULL;
    raster->poly_pixel_coeff = g_new(double,20);
    raster->poly_line_coeff = g_new(double,20);

    cps.count = raster->gcp_count;
    cps.e1 = pixel;
    cps.n1 = line;
    cps.e2 = x;
    cps.n2 = y;
    cps.status = status;

#ifdef ATLANTIS_BUILD
    if( raster->gcp_count <3 )
    {
        for( i = 0; i < raster->gcp_count; i++ )
        {
            diff_x += (x[i]-pixel[i]);
            diff_y += (y[i]-line[i]);
        } 
        raster->poly_pixel_coeff[0] = -diff_x/(double)raster->gcp_count;
        raster->poly_line_coeff[0] = -diff_y/(double)raster->gcp_count;
        raster->poly_x_coeff[0] = -raster->poly_pixel_coeff[0];
        raster->poly_y_coeff[0] = -raster->poly_line_coeff[0];
        raster->poly_x_coeff[1] = raster->poly_pixel_coeff[1]=1.0;
        raster->poly_y_coeff[2] = raster->poly_line_coeff[2]=1.0;

        return TRUE;
    }
    
    success = CRS_compute_georef_equations( &cps, 
                                      raster->poly_x_coeff, 
                                      raster->poly_y_coeff,
                                      raster->poly_pixel_coeff, 
                                      raster->poly_line_coeff,
                                      raster->poly_order );

#else    
    while( (success = 
            CRS_compute_georef_equations( &cps, 
                                          raster->poly_x_coeff, 
                                          raster->poly_y_coeff,
                                          raster->poly_pixel_coeff, 
                                          raster->poly_line_coeff,
                                          raster->poly_order )) != 1 
           && raster->poly_order > 1 )
    {
        raster->poly_order--;
    }
#endif

    if( success != 1 )
    {
        g_warning( "CRS_compute_georef_equations failed." );
        raster->poly_order = -1;
        return FALSE;
    }

    g_free( x );
    g_free( y );
    g_free( pixel );
    g_free( line );
    g_free( status );

    gtk_signal_emit(GTK_OBJECT(raster), 
                    raster_signals[GEOTRANSFORM_CHANGED]);

    return TRUE;
}

static gint gv_raster_build_poly_transformCL( GvRaster *raster, int poly_order )

{
    double  *pixel, *line, *x, *y;
    double   diff_x=0.0, diff_y=0.0;
    int         i, *status;
    struct Control_Points cps;

    if( raster->gcp_countCL == 0 )
        return FALSE;

    if (poly_order < 0)
    {
        /* default to 1 (poly_order < 0 used to indicate defaults should be used)*/
        raster->poly_orderCL = 1;
    }
    else
    {
        raster->poly_orderCL = poly_order;
        if ((raster->gcp_countCL < 6) && (poly_order > 1))
        {
            g_warning("Not enough gcp's for polynomial order > 1: resetting to 1.");
            raster->poly_orderCL = 1;
        }   
        else if ((raster->gcp_countCL < 10) && (poly_order > 2))
        {
            g_warning("Not enough gcp's for polynomial order > 2: resetting to 2.");
            raster->poly_orderCL = 2;
        }   
    }

    pixel = g_new(double,raster->gcp_countCL);
    line = g_new(double,raster->gcp_countCL);
    x = g_new(double,raster->gcp_countCL);
    y = g_new(double,raster->gcp_countCL);
    status = g_new(int,raster->gcp_countCL);

    for( i = 0; i < raster->gcp_countCL; i++ )
    {
        x[i] = raster->gcp_listCL[i].dfGCPX;
        y[i] = raster->gcp_listCL[i].dfGCPY;
        pixel[i] = raster->gcp_listCL[i].dfGCPPixel;
        line[i] = raster->gcp_listCL[i].dfGCPLine;
        status[i] = 1; /* use this point */
    }

    raster->poly_x_coeffCL = g_new(double,20);
    raster->poly_y_coeffCL = g_new(double,20);
    raster->poly_z_coeffCL = NULL;
    raster->poly_pixel_coeffCL = g_new(double,20);
    raster->poly_line_coeffCL = g_new(double,20);

    cps.count = raster->gcp_countCL;
    cps.e1 = pixel;
    cps.n1 = line;
    cps.e2 = x;
    cps.n2 = y;
    cps.status = status;

    if( raster->gcp_countCL <3 )
    {
        for( i = 0; i < raster->gcp_countCL; i++ )
        {
            diff_x += (x[i]-pixel[i]);
            diff_y += (y[i]-line[i]);
        } 
        raster->poly_pixel_coeffCL[0] = -diff_x/(double)raster->gcp_countCL;
        raster->poly_line_coeffCL[0] = -diff_y/(double)raster->gcp_countCL;
        raster->poly_x_coeffCL[0] = -raster->poly_pixel_coeffCL[0];
        raster->poly_y_coeffCL[0] = -raster->poly_line_coeffCL[0];
        raster->poly_x_coeffCL[1] = raster->poly_pixel_coeffCL[1]=1.0;
        raster->poly_y_coeffCL[2] = raster->poly_line_coeffCL[2]=1.0;

        return TRUE;
    }
    
    if( CRS_compute_georef_equations( &cps, 
                                      raster->poly_x_coeffCL, 
                                      raster->poly_y_coeffCL,
                                      raster->poly_pixel_coeffCL, 
                                      raster->poly_line_coeffCL,
                                      raster->poly_orderCL ) != 1 )
    {
        g_warning( "CRS_compute_georef_equations failed." );
        raster->poly_orderCL = -1;
        return FALSE;
    }

    g_free( x );
    g_free( y );
    g_free( pixel );
    g_free( line );
    g_free( status );

    return TRUE;
}
                                    
/* Transform pixel/line coordinates to georeferenced coordinates */

gint gv_raster_pixel_to_georef( GvRaster *raster, 
                                double *x, double *y, double *z )

{
    if( raster->poly_order > -1 )
    {
        CRS_georef( *x, *y, x, y, 
                    raster->poly_x_coeff, raster->poly_y_coeff, 
                    raster->poly_order );

        /* z is left unchanged */
    }
    else
    {
        double  x_out, y_out;

        x_out = raster->geotransform[0] 
            + *x * raster->geotransform[1] + *y * raster->geotransform[2];
        y_out = raster->geotransform[3] 
            + *x * raster->geotransform[4] + *y * raster->geotransform[5];

        *x = x_out;
        *y = y_out;
    }

    return TRUE;
}

gint gv_raster_pixel_to_georefCL( GvRaster *raster, 
                                double *x, double *y, double *z )
{                  
    if (raster->poly_orderCL > -1 )
    {
        CRS_georef( *x, *y, x, y, 
                    raster->poly_x_coeffCL, raster->poly_y_coeffCL, 
                    raster->poly_orderCL );

        /* z is left unchanged */
    }
    else
    {
        /* Default to standard transformation if cursor-link */
        /* specific transforms aren't defined.               */
        gv_raster_pixel_to_georef(raster,x,y,z);
    }

    return TRUE;
}   

/* Transform georeferenced coordinates to pixel/line coordinates */

gint gv_raster_georef_to_pixel( GvRaster *raster, 
                                double *x, double *y, double *z )

{
    if( raster->poly_order > -1 )
    {
        CRS_georef( *x, *y, x, y, 
                    raster->poly_pixel_coeff, raster->poly_line_coeff, 
                    raster->poly_order );

        /* z is left unchanged */
    }
    else
    {
        double x_out, y_out, det;

        det = ((raster->geotransform[1]*raster->geotransform[5])-
               (raster->geotransform[2]*raster->geotransform[4]));

        if (fabs(det) < 0.000000000000001)
            return FALSE;

        /* Original code commented out below: it left out the rotational terms! */
        /*        *x = (*x - raster->geotransform[0]) / raster->geotransform[1]; */
        /*  *y = (*y - raster->geotransform[3]) / raster->geotransform[5]; */

        x_out = ((raster->geotransform[5]*(*x - raster->geotransform[0])) -
                 (raster->geotransform[2]*(*y - raster->geotransform[3])))/ det;

        y_out = ((raster->geotransform[1]*(*y - raster->geotransform[3])) -
                 (raster->geotransform[4]*(*x - raster->geotransform[0])))/ det;
 
        *x = x_out;
        *y = y_out;

    }

    return TRUE;
}

gint gv_raster_georef_to_pixelCL( GvRaster *raster, 
                                double *x, double *y, double *z )

{

    if( raster->poly_orderCL > -1 )
    {
        CRS_georef( *x, *y, x, y, 
                    raster->poly_pixel_coeffCL, raster->poly_line_coeffCL, 
                    raster->poly_orderCL );
        /* z is left unchanged */
    }
    else
    {
        /* Default to standard transformation if cursor-link */
        /* specific transforms aren't defined.               */
        gv_raster_georef_to_pixel(raster,x,y,z);
    }

    return TRUE;
}

#endif /* defined(USE_CRS) */

/* ==================================================================== */
/*      not defined USE_CRS: GCP fitting using EarthView derived        */
/*      code in gvgcpfit.c                                              */
/* ==================================================================== */

#if !defined(USE_CRS)

/* build polynomial coefficients based on gcps */

static gint gv_raster_build_poly_transform( GvRaster *raster )

{
    double  *pixel, *line, *x, *y, *z, rms;
    int         i;

    int poly_order_pref=1; /* polynomial order preference- only used if preference is set */

    if( raster->gcp_count < 3 )
        return FALSE;

    if( raster->gcp_count >= 10 )
        raster->poly_order = 3;
    else if( raster->gcp_count >= 6 )
        raster->poly_order = 2;
    else
        raster->poly_order = 1;

    /* User has specified a preference for polynomial order.  Do a sanity */
    /* check, then use it if it's reasonable */
    if (gv_data_get_property( GV_DATA(raster), "poly_order_preference" ) != NULL )
    {
        poly_order_pref = atoi(gv_data_get_property( GV_DATA(raster), "poly_order_preference" ));
        raster->poly_order = gv_raster_check_poly_order(raster,poly_order_pref);
        if (poly_order_pref != raster->poly_order)
        {
            g_warning("gv_raster_build_poly_transform(): Inappropriate polynomial order setting.");
        }

    }

    pixel = g_new(double,raster->gcp_count);
    line = g_new(double,raster->gcp_count);
    x = g_new(double,raster->gcp_count);
    y = g_new(double,raster->gcp_count);
    z = g_new(double,raster->gcp_count);

    for( i = 0; i < raster->gcp_count; i++ )
    {
        x[i] = raster->gcp_list[i].dfGCPX;
        y[i] = raster->gcp_list[i].dfGCPY;
        z[i] = raster->gcp_list[i].dfGCPZ;
        pixel[i] = raster->gcp_list[i].dfGCPPixel;
        line[i] = raster->gcp_list[i].dfGCPLine;
    }

    raster->poly_x_coeff = g_new(double,20);
    raster->poly_y_coeff = g_new(double,20);
    raster->poly_z_coeff = g_new(double,20);
    raster->poly_pixel_coeff = g_new(double,20);
    raster->poly_line_coeff = g_new(double,20);

    if( TwoDPolyFit( &rms, raster->poly_x_coeff, raster->poly_order, 
                     raster->gcp_count, x, pixel, line ) != SUCCESS )
    {
        raster->poly_order = -1;
        return FALSE;                           
    }

    if( TwoDPolyFit( &rms, raster->poly_y_coeff, raster->poly_order, 
                     raster->gcp_count, y, pixel, line ) != SUCCESS )
    {
        raster->poly_order = -1;
        return FALSE;                           
    }

    if( TwoDPolyFit( &rms, raster->poly_z_coeff, raster->poly_order, 
                     raster->gcp_count, z, pixel, line ) != SUCCESS )
    {
        raster->poly_order = -1;
        return FALSE;                           
    }

    if( TwoDPolyFit( &rms, raster->poly_pixel_coeff, raster->poly_order, 
                     raster->gcp_count, pixel, x, y ) != SUCCESS )
    {
        raster->poly_order = -1;
        return FALSE;                           
    }

    if( TwoDPolyFit( &rms, raster->poly_line_coeff, raster->poly_order, 
                     raster->gcp_count, line, x, y ) != SUCCESS )
    {
        raster->poly_order = -1;
        return FALSE;
    }

    g_free( x );
    g_free( y );
    g_free( z );
    g_free( pixel );
    g_free( line );

    gtk_signal_emit(GTK_OBJECT(raster), 
                    raster_signals[GEOTRANSFORM_CHANGED]);

    return TRUE;
}
static gint gv_raster_build_poly_transformCL( GvRaster *raster, int poly_order )

{
    double  *pixel, *line, *x, *y, *z, rms;
    int         i;

    if( raster->gcp_countCL < 3 )
        return FALSE;

    if (poly_order < 0)
    {
        /* poly_order < 0 used to indicate defaults should be used */
        if( raster->gcp_countCL >= 10 )
            raster->poly_orderCL = 3;
        else if( raster->gcp_countCL >= 6 )
            raster->poly_orderCL = 2;
        else
            raster->poly_orderCL = 1;
        
    }
    else
    {
        raster->poly_orderCL = poly_order;
        if ((raster->gcp_countCL < 6) && (poly_order > 1))
        {
            g_warning("Not enough gcp's for polynomial order > 1: resetting to 1.");
            raster->poly_orderCL = 1;
        }   
        else if ((raster->gcp_countCL < 10) && (poly_order > 2))
        {
            g_warning("Not enough gcp's for polynomial order > 2: resetting to 2.");
            raster->poly_orderCL = 2;
        }   
    }


    pixel = g_new(double,raster->gcp_countCL);
    line = g_new(double,raster->gcp_countCL);
    x = g_new(double,raster->gcp_countCL);
    y = g_new(double,raster->gcp_countCL);
    z = g_new(double,raster->gcp_countCL);

    for( i = 0; i < raster->gcp_countCL; i++ )
    {
        x[i] = raster->gcp_listCL[i].dfGCPX;
        y[i] = raster->gcp_listCL[i].dfGCPY;
        z[i] = raster->gcp_listCL[i].dfGCPZ;
        pixel[i] = raster->gcp_listCL[i].dfGCPPixel;
        line[i] = raster->gcp_listCL[i].dfGCPLine;
    }

    raster->poly_x_coeffCL = g_new(double,20);
    raster->poly_y_coeffCL = g_new(double,20);
    raster->poly_z_coeffCL = g_new(double,20);
    raster->poly_pixel_coeffCL = g_new(double,20);
    raster->poly_line_coeffCL = g_new(double,20);

    if( TwoDPolyFit( &rms, raster->poly_x_coeffCL, raster->poly_orderCL, 
                     raster->gcp_countCL, x, pixel, line ) != SUCCESS )
    {
        raster->poly_orderCL = -1;
        return FALSE;                           
    }

    if( TwoDPolyFit( &rms, raster->poly_y_coeffCL, raster->poly_orderCL, 
                     raster->gcp_countCL, y, pixel, line ) != SUCCESS )
    {
        raster->poly_orderCL = -1;
        return FALSE;                           
    }

    if( TwoDPolyFit( &rms, raster->poly_z_coeffCL, raster->poly_orderCL, 
                     raster->gcp_countCL, z, pixel, line ) != SUCCESS )
    {
        raster->poly_orderCL = -1;
        return FALSE;                           
    }

    if( TwoDPolyFit( &rms, raster->poly_pixel_coeffCL, raster->poly_orderCL, 
                     raster->gcp_countCL, pixel, x, y ) != SUCCESS )
    {
        raster->poly_orderCL = -1;
        return FALSE;                           
    }

    if( TwoDPolyFit( &rms, raster->poly_line_coeffCL, raster->poly_orderCL, 
                     raster->gcp_countCL, line, x, y ) != SUCCESS )
    {
        raster->poly_orderCL = -1;
        return FALSE;
    }

    g_free( x );
    g_free( y );
    g_free( z );
    g_free( pixel );
    g_free( line );

    return TRUE;
}
                                          
/* Transform pixel/line coordinates to georeferenced coordinates */

gint gv_raster_pixel_to_georef( GvRaster *raster, 
                                double *x, double *y, double *z )

{
    if( raster->poly_order > -1 )
    {
        double  x_out, y_out, z_out;

        x_out = TwoDPolyEval( raster->poly_x_coeff, raster->poly_order, 
                              *x, *y );

        y_out = TwoDPolyEval( raster->poly_y_coeff, raster->poly_order, 
                              *x, *y );

        z_out = TwoDPolyEval( raster->poly_z_coeff, raster->poly_order, 
                              *x, *y );

        *x = x_out;
        *y = y_out;
        
        if( z != NULL )
            *z = z_out;
    }
    else
    {
        double  x_out, y_out;

        x_out = raster->geotransform[0] 
            + *x * raster->geotransform[1] + *y * raster->geotransform[2];
        y_out = raster->geotransform[3] 
            + *x * raster->geotransform[4] + *y * raster->geotransform[5];

        *x = x_out;
        *y = y_out;
    }

    return TRUE;
}

gint gv_raster_pixel_to_georefCL( GvRaster *raster, 
                                double *x, double *y, double *z )

{
    if( raster->poly_orderCL > -1 )
    {
        double  x_out, y_out, z_out;

        x_out = TwoDPolyEval( raster->poly_x_coeffCL, raster->poly_orderCL, 
                              *x, *y );

        y_out = TwoDPolyEval( raster->poly_y_coeffCL, raster->poly_orderCL, 
                              *x, *y );

        z_out = TwoDPolyEval( raster->poly_z_coeffCL, raster->poly_orderCL, 
                              *x, *y );

        *x = x_out;
        *y = y_out;
        
        if( z != NULL )
            *z = z_out;
    }
    else
    {
        /* Default to standard transformation if cursor-link */
        /* specific transforms aren't defined.               */
        gv_raster_pixel_to_georef(raster,x,y,z);
    }

    return TRUE;
}

/* Transform georeferenced coordinates to pixel/line coordinates */

gint gv_raster_georef_to_pixel( GvRaster *raster, 
                                double *x, double *y, double *z )

{
    if( raster->poly_order > -1 )
    {
        double  x_out, y_out;

        x_out = TwoDPolyEval( raster->poly_pixel_coeff, raster->poly_order, 
                              *x, *y );

        y_out = TwoDPolyEval( raster->poly_line_coeff, raster->poly_order, 
                              *x, *y );

        *x = x_out;
        *y = y_out;
    }
    else
    {
        *x = (*x - raster->geotransform[0]) / raster->geotransform[1];
        *y = (*y - raster->geotransform[3]) / raster->geotransform[5];
    }

    return TRUE;
}
gint gv_raster_georef_to_pixelCL( GvRaster *raster, 
                                double *x, double *y, double *z )

{
    if( raster->poly_orderCL > -1 )
    {
        double  x_out, y_out;

        x_out = TwoDPolyEval( raster->poly_pixel_coeffCL, raster->poly_orderCL, 
                              *x, *y );

        y_out = TwoDPolyEval( raster->poly_line_coeffCL, raster->poly_orderCL, 
                              *x, *y );

        *x = x_out;
        *y = y_out;
    }
    else
    {
        /* Default to standard transformation if cursor-link */
        /* specific transforms aren't defined.               */
        gv_raster_georef_to_pixel(raster,x,y,z); 
    }

    return TRUE;
}

#endif /* !defined(USE_CRS) */

/* ========================================================================= */
/* gv_raster_check_poly_order: Sanity check the desired polynomial order     */
/* input: raster, desired polynomial order.                                  */
/* output: closest matching polynomial order.                                */
/* ========================================================================= */
static int gv_raster_check_poly_order( GvRaster *raster, 
                                       int desired_poly_order )
{
    int poly_order;

    poly_order = desired_poly_order;

    if (poly_order > 3)
    {

        if( raster->gcp_count >= 10 )
            poly_order = 3;
        else if( raster->gcp_count >= 6 )
            poly_order = 2;
        else
            poly_order = 1;
        
    }
    else if (poly_order == 3)
    {
        if( raster->gcp_count < 6 )
            poly_order = 1;
        else if (raster->gcp_count < 10)
            poly_order = 2;

    }
    else if (poly_order == 2)
    {
        if( raster->gcp_count < 6 )
            poly_order = 1;
        
    }
    else if (poly_order < 1)
        poly_order = 1;
    
    return poly_order;

}

void gv_raster_set_poly_order_preference( GvRaster *raster, int poly_order )
{
    char txt[16];
    int success;

    sprintf(txt,"%d",poly_order);
    gv_data_set_property(GV_DATA(raster),"poly_order_preference",txt );

    if (raster->gcp_count > 0)
    {
        /* If gcps have already been set, rebuild poly transform */
        success = gv_raster_build_poly_transform( raster );

        if( !success )
            gtk_signal_emit(GTK_OBJECT(raster), 
                            raster_signals[GEOTRANSFORM_CHANGED]);
    }

}
