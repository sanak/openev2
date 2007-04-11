/******************************************************************************
 * $Id: gvraster.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
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
 * $Log: gvraster.h,v $
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
 * Revision 1.28  2004/01/22 19:57:11  andrey_kiselev
 * Use gv_raster_get_nodata() function to fetch the NODATA value from the image
 * using GDALGetRasterNoDataValue().
 *
 * Revision 1.27  2003/09/11 20:00:30  gmwalter
 * Add ability to specify a preferred polynomial order for warping a raster,
 * and add "safe mode" (only used if ATLANTIS_BUILD is defined).
 *
 * Revision 1.26  2003/02/20 19:27:16  gmwalter
 * Updated link tool to include Diana's ghost cursor code, and added functions
 * to allow the cursor and link mechanism to use different gcps
 * than the display for georeferencing.  Updated raster properties
 * dialog for multi-band case.  Added some signals to layerdlg.py and
 * oeattedit.py to make it easier for tools to interact with them.
 * A few random bug fixes.
 *
 * Revision 1.25  2001/11/28 19:18:29  warmerda
 * Added set_gcps(), and get_gcps() methods on GvRaster, and the
 * geotransform-changed signal generated when the gcps change.
 *
 * Revision 1.24  2001/10/16 18:50:06  warmerda
 * now possible to pass sample set into autoscale
 *
 * Revision 1.23  2001/08/14 17:03:24  warmerda
 * added standard deviation autoscaling support
 *
 * Revision 1.22  2001/07/24 02:21:54  warmerda
 * added 8bit phase averaging
 *
 * Revision 1.21  2001/07/13 22:15:36  warmerda
 * added nodata aware averaging
 *
 * Revision 1.20  2001/04/02 18:10:46  warmerda
 * expose gv_raster_autoscale() to python
 *
 * Revision 1.19  2000/09/27 19:18:50  warmerda
 * Honour GvSMSample for real and complex images.
 * Add GvRaster.sm field.  If set to GvSMSample always let GDAL do the
 * decimation for faster loads.
 *
 * Revision 1.18  2000/06/26 15:12:47  warmerda
 * added get_min, get_max macros
 *
 * Revision 1.17  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_RASTER_H__
#define __GV_RASTER_H__

#include "gvdata.h"
#include "gvrastercache.h"
#include "gvmesh.h"

#include "gdal.h"

#define GV_TYPE_RASTER            (gv_raster_get_type ())
#define GV_RASTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_RASTER, GvRaster))
#define GV_RASTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_RASTER, GvRasterClass))
#define GV_IS_RASTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_RASTER))
#define GV_IS_RASTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_RASTER))

typedef enum {
    GvSMAverage = 0,
    GvSMSample = 1,
    GvSMAverage8bitPhase = 2
} GvSampleMethod;

typedef enum {
    GvASAAutomatic = 0, 
    GvASAPercentTailTrim = 1,
    GvASAStdDeviation = 2
} GvAutoScaleAlg;

typedef struct _GvRaster       GvRaster;
typedef struct _GvRasterClass  GvRasterClass;

#define GV_TILE_OVERLAP           2

struct _GvRaster
{
    GvData data;

    gint max_tiles,                         /* total tiles */
        width,                              /* width of raster */
        height,                             /* height of raster */
        tile_x,                             /* tile width */
        tile_y,                             /* tile height */
        tiles_across,                       /* tiles per row */
        tiles_down,                         /* tiles per column */
        max_lod;                            /* maximum level of detail(+1) */

    float min, max;                          /* suggested scaling min/max */
    
    GvSampleMethod sm;                       /* Sample method */
    void *(*average)(GvRaster *raster, void *buffer, int tsize_x, int tsize_y);

    int type;                                /* GV_RASTER_* */
    int item_size;                           /* size of pixel group */

    gv_raster_cache *cache;                  /* read cache */

    double  geotransform[6];                 /* pl to georef transform */

    int     gcp_count;                       /* 0 if not relativant */
    GDAL_GCP *gcp_list;
    int     poly_order;                      /* -1 if not intialized */
    double  *poly_pixel_coeff;
    double  *poly_line_coeff;
    double  *poly_x_coeff;
    double  *poly_y_coeff;
    double  *poly_z_coeff;

    /* Separate transformations to be used by ghost cursor and linking */
    /* DEFAULTS TO REGULAR TRANSFORMATION IF THESE AREN'T SET       */
    int     gcp_countCL;                             /* 0 if not relativant */
    GDAL_GCP *gcp_listCL;
    int     poly_orderCL;                      /* -1 if not intialized */
    double  *poly_pixel_coeffCL;
    double  *poly_line_coeffCL;
    double  *poly_x_coeffCL;
    double  *poly_y_coeffCL;
    double  *poly_z_coeffCL;

    /* End of cursor/link-specific transform stuff */

    GDALDatasetH     dataset;                /* source dataset */
    GDALRasterBandH  gdal_band;              /* real band */

    GDALDataType gdal_type;                  /* GDT_Byte, etc */
};

struct _GvRasterClass
{
    GvDataClass parent_class;
    void (* geotransform_changed)(GvRaster *raster);
    void (* disconnected)(GvRaster *raster);
};

GType gv_raster_get_type(void);
GvData* gv_raster_new(GDALDatasetH dataset, int real_band, GvSampleMethod sm);
void gv_raster_read(GvRaster *raster, GDALDatasetH dataset, int real_band, GvSampleMethod sm);
void *gv_raster_tile_get(GvRaster *raster, int tile, int lod);
gint *gv_raster_tile_xy_get(GvRaster *raster, int tile, int lod, gint *coords);
void gv_raster_flush_cache(GvRaster *raster,
                           int x_off, int y_off, int width, int height);
gint gv_raster_get_nodata(GvRaster *raster, double *nodata);
gint gv_raster_get_sample(GvRaster *raster, double x, double y,
                          double *real, double *imaginary);
double gv_raster_pixel_size(GvRaster *raster);
gint gv_raster_pixel_to_georef(GvRaster *raster, 
                               double *x, double *y, double *z);
gint gv_raster_georef_to_pixel(GvRaster *raster, 
                               double *x, double *y, double *z);
gint gv_raster_autoscale(GvRaster *raster, GvAutoScaleAlg alg, 
                          double alg_param, 
                          int sample_count, float *sample_set,
                          double *out_min, double *out_max);
int  gv_raster_collect_random_sample(GvRaster *raster, 
                                      int sample_count, float *sample_set,
                                      int xoff, int yoff, int xsize, int ysize,
                                      GArray *tile_list);
                                  
int  
gv_raster_collect_histogram(GvRaster *raster, 
                            double scale_min, double scale_max, 
                            int bucket_count, int *histogram,
                            int include_out_of_range, 
                            int xoff, int yoff, int xsize, int ysize,
                            GArray *tile_list);
                                  
#define gv_raster_get_min(raster) ((GV_RASTER(raster))->min)
#define gv_raster_get_max(raster) ((GV_RASTER(raster))->max)

int gv_raster_set_gcps(GvRaster *raster, int gcp_count, const GDAL_GCP *gcps);
int gv_raster_get_gcp_count(GvRaster *raster);
const GDAL_GCP *gv_raster_get_gcps(GvRaster *raster);

/* Cursor/Link specific transforms (only used if set)*/
int gv_raster_set_gcpsCL(GvRaster *raster, int gcp_count, const GDAL_GCP *gcps, int poly_order);
int gv_raster_get_gcp_countCL(GvRaster *raster);
const GDAL_GCP *gv_raster_get_gcpsCL(GvRaster *raster);
gint gv_raster_pixel_to_georefCL(GvRaster *raster, 
                               double *x, double *y, double *z);
gint gv_raster_georef_to_pixelCL(GvRaster *raster, 
                               double *x, double *y, double *z);

/* Set preferred polynomial order for this raster (optional) */
void gv_raster_set_poly_order_preference(GvRaster *raster, int poly_order);

GDALDatasetH gv_raster_get_dataset(GvRaster *raster);

/* Non-existent functions?  Removed prototypes GTK2 PORT */
/*
gint gv_raster_datatype_get(GvRaster *raster);
gint gv_raster_dataformat_get(GvRaster *raster);
*/


#endif /* __GV_RASTER_H__ */

