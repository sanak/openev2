/******************************************************************************
 * $Id: gvrastersource.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Code related to handling of GvRasterSource on a GvRasterLayer.
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
 * $Log: gvrastersource.c,v $
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
 * Revision 1.16  2004/06/23 14:35:05  gmwalter
 * Added support for multi-band complex imagery.
 *
 * Revision 1.15  2004/04/02 17:01:02  gmwalter
 * Updated nodata support for complex and
 * rgb data.
 *
 * Revision 1.14  2004/02/20 12:32:23  andrey_kiselev
 * Use turn on alfa blending in gv_raster_layer_nodata_set().
 *
 * Revision 1.13  2004/02/18 16:55:32  andrey_kiselev
 * Don't change the min/max levels in gv_raster_layer_set_source().
 *
 * Revision 1.12  2004/02/17 14:28:43  warmerda
 * check for NULL data in layer
 *
 * Revision 1.11  2004/01/22 20:45:27  andrey_kiselev
 * Added methods gv_raster_layer_nodata_set() and gv_raster_layer_nodata_get() to
 * work with nodata_* layer properties and method gv_raster_layer_type_get() to
 * query raster data type.
 *
 * Revision 1.10  2003/03/06 22:55:58  gmwalter
 * Fix pure phase display to remove residual dependence on magnitude.
 *
 * Revision 1.9  2001/10/19 13:30:58  warmerda
 * include gvrasterlut.h for compose call
 *
 * Revision 1.8  2001/10/17 16:23:52  warmerda
 * added support for composing complex lut and pct
 *
 * Revision 1.7  2001/01/30 19:34:11  warmerda
 * added gv_raster_layer_purge_all_textures calls
 *
 * Revision 1.6  2000/08/25 20:11:07  warmerda
 * added preliminary nodata support
 *
 * Revision 1.5  2000/08/16 20:58:46  warmerda
 * produce proper constant complex images
 *
 * Revision 1.4  2000/08/09 17:38:23  warmerda
 * keep reference on source GvRasters
 *
 * Revision 1.3  2000/08/02 19:17:21  warmerda
 * return NULL on illegal source in get_data()
 *
 * Revision 1.2  2000/06/27 21:26:24  warmerda
 * removed use of invalidated
 *
 * Revision 1.1  2000/06/23 12:51:07  warmerda
 * New
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include "gvrasterlayer.h"
#include "gvrastertypes.h"
#include "gvrasterlut.h"

#ifndef sign
#  define sign(a) ( (a < 0.0 )?-1.0:1.0 )
#endif

/* >>>> Local function prototypes <<<< */

static unsigned char *
gv_scale_tile_to_byte(void *src_data, int tile_x, int tile_y,
		      GvRasterSource *source);
static unsigned char *
gv_scale_tile_linear(void *src_data, int tile_x, int tile_y, 
		     float min, float max, GDALDataType gdal_type);


static unsigned char *
gv_scale_tile_linear(void *src_data, int tile_x, int tile_y, 
		     float min, float max,GDALDataType gdal_type)
{
    float scale;
    unsigned char *ret_data;
    int   i, pixel_count = tile_x * tile_y;

    if( max == min )
        scale = 1.0;
    else
        scale = 255.0 / (max - min);

    if( gdal_type == GDT_Byte )
    {
        unsigned char *raw_data = (unsigned char *) src_data;
        float          f;

        ret_data = (unsigned char *) g_malloc(pixel_count);

        for( i = 0; i < pixel_count; i++ )
        {
            f = (raw_data[i] - min) * scale;
            if( f < 0.0 )
                ret_data[i] = 0;
            else if( f > 255 )
                ret_data[i] = 255;
            else
                ret_data[i] = (int) f;
        }
    }
    else if( gdal_type == GDT_Float32 )
    {
        float 	*raw_data = (float *) src_data;
        float    f;

        ret_data = (unsigned char *) g_malloc(pixel_count);

        for( i = 0; i < pixel_count; i++ )
        {
            f = (raw_data[i] - min) * scale;
            if( f < 0.0 )
                ret_data[i] = 0;
            else if( f > 255 )
                ret_data[i] = 255;
            else
                ret_data[i] = (int) f;
        }
    }
    else if( gdal_type == GDT_CFloat32 )
    {
        float 	*raw_data = (float *) src_data;

        if( max == 0.0 )
            scale = 1.0;
        else
            scale = 0.5 / max;

        ret_data = (unsigned char *) g_malloc(pixel_count*2);

        for( i = 0; i < pixel_count; i++ )
        {
            float temp_i, temp_q;

            temp_i = raw_data[i*2] * scale;
            temp_q = raw_data[i*2+1] * scale;

            if( fabs( temp_i ) > 0.5 )
            {
                temp_q *= 0.5 / fabs( temp_i );
                temp_i = 0.5 * sign( temp_i );
            }
            
            if( fabs( temp_q ) > 0.5 )
            {
                temp_i *= 0.5 / fabs( temp_q );
                temp_q = 0.5 * sign( temp_q );
            }

            temp_i += 0.5;
            temp_q += 0.5;

            ret_data[i*2  ] = (int) (temp_i * 255.0);
            ret_data[i*2+1] = (int) (temp_q * 255.0);
        }
    }
    else
    {
        ret_data = (unsigned char *) g_malloc(pixel_count);

        g_warning( "unhandled data type in gv_scale_tile_to_byte().\n" );
        memset( ret_data, 0, pixel_count );
    }

    return ret_data;
}

/**
 * Scale a tile of source data to byte range 0 - 255.  Will use a lut
 * if available.  If source->lut (256 entry lut) exists, a new gv_lut
 * will be created so as not to lose any output gray values.  This takes
 * the place of source->lut, which is cleared.
 *
 * Prior to the addition of the GvRasterLut source->gv_lut, this
 * function was what is now gv_scale_tile_linear.  The source data
 * was always converted to byte range prior to any lut being applied,
 * resulting in the loss of some grayscale resolution.  Now, applying
 * the lut results in data scaled to byte range, and the lut is no
 * longer applied downstream in gv_raster_layer_srctile_xy_get.
 *
 * @param src_data raster data of type source->data->gdal_type
 * @param tile_x width of src_data raster
 * @param tile_y height of src_data raster
 * @param source the source of src_data; equates to a band, of which
 * the raster layer may have more than one depending upon the raster
 * layer mode.
 *
 * @return the src_data converted to byte range for image composite and
 * display.
 */
static unsigned char *
gv_scale_tile_to_byte(void *src_data, int tile_x, int tile_y,
		      GvRasterSource *source)
{
    float scale, ff, max, min;
    unsigned char *ret_data;
    int ii, index, pixel_count = tile_x * tile_y;

    /* ---- Perform original scaling if no LUTs specified ---- */
    if ((source->data->gdal_type == GDT_CFloat32) ||
	((source->lut == NULL) &&
	 (source->gv_lut == NULL))) {
	return gv_scale_tile_linear(src_data, tile_x, tile_y, source->min,
				    source->max, source->data->gdal_type);
    }

    /* ---- If original 256 entry lut specified, create gv_lut ---- */
    if (source->lut != NULL) {
	if (source->gv_lut != NULL) {
	    gv_raster_lut_free(source->gv_lut);
	}
	source->gv_lut = gv_raster_lut_create_from_byte_lut
	    (source->lut, source->min, source->max,
	     GV_RASTER_LUT_OPTIONS_NONE);
	g_free(source->lut);
	source->lut = NULL;
    }

    /* ---- Use gv_lut to return src_data scaled to byte range ---- */
    max = source->gv_lut->max;
    min = source->gv_lut->min;
    ret_data = (unsigned char *) g_malloc(pixel_count);

    scale = max - min;
    if( max == min )
        scale = 1.0;
    else
        scale = source->gv_lut->size / (max - min);

    if(source->data->gdal_type == GDT_Byte) {
        unsigned char *raw_data = (unsigned char *)src_data;

        for(ii = 0; ii < pixel_count; ii++ ) {
	    index = (raw_data[ii] - min) * scale;
	    if (index < 0) index = 0;
	    if (index >= source->gv_lut->size) {
		index = source->gv_lut->size - 1;
	    }
	    ret_data[ii] = source->gv_lut->table[index];
	}
    }
    else if(source->data->gdal_type == GDT_Float32) {
        float 	*raw_data = (float *)src_data;

        for(ii = 0; ii < pixel_count; ii++ ) {
	    index = (raw_data[ii] - min) * scale;
	    if (index < 0) index = 0;
	    if (index >= source->gv_lut->size) {
		index = source->gv_lut->size - 1;
	    }
	    ret_data[ii] = source->gv_lut->table[index];
	}
    }
    else {
        g_warning( "unhandled data type in gv_scale_tile_to_byte().\n" );
        memset( ret_data, 0, pixel_count );
    }

    return ret_data;
}

static unsigned char *
gv_scale_pure_phase_tile_to_byte( void *src_data, 
                       int tile_x, int tile_y, 
                       GDALDataType gdal_type )

{
    unsigned char *ret_data;
    int   i, pixel_count = tile_x * tile_y;

    if( gdal_type == GDT_CFloat32 )
    {
        float 	*raw_data = (float *) src_data;

        ret_data = (unsigned char *) g_malloc(pixel_count*2);

        for( i = 0; i < pixel_count; i++ )
        {
            float temp_i, temp_q;

            temp_i = raw_data[i*2];
            temp_q = raw_data[i*2+1];

            if ((temp_i == 0) && (temp_q == 0))
            {
                ret_data[i*2]=127;
                ret_data[i*2+1]=127;
            }
            else if (fabs(temp_i ) > fabs(temp_q))
            {
                ret_data[i*2]=(int) (0.5*(1.0+sign(temp_i))*255);
                ret_data[i*2+1]=(int) (0.5*(1.0+temp_q/fabs(temp_i))*255);
            }
            else
            {
                ret_data[i*2]=(int) (0.5*(1.0+temp_i/fabs(temp_q))*255);
                ret_data[i*2+1]=(int) (0.5*(1.0+sign(temp_q))*255);
            }
        }
    }
    else
    {
        ret_data = (unsigned char *) g_malloc(pixel_count);

        g_warning( "unhandled data type in gv_scale_pure_phase_tile_to_byte().\n" );
        memset( ret_data, 0, pixel_count );
    }

    return ret_data;
}

static void 
gv_raster_layer_srctile_check_nodata( GvRasterSource *source, int pixels, 
                                      unsigned char *data, 
                                      unsigned char *nodata_mask )

{
    int       i;

    if( nodata_mask == NULL || !source->nodata_active )
        return;

    /* assume all valid data in constant sources */
    if( source->data == NULL )
        return;

    if( source->data->gdal_type == GDT_Byte )
    {
        unsigned char nodata = (unsigned char) source->nodata_real;

        for( i = 0; i < pixels; i++ )
        {
            if( data[i] == nodata )
                nodata_mask[i] = 0;
        }
    }
    else if( source->data->gdal_type == GDT_Float32 )
    {
        float nodata = (float) source->nodata_real;

        for( i = 0; i < pixels; i++ )
        {
            if( ((float *) data)[i] == nodata )
                nodata_mask[i] = 0;
        }
    }
    else if( source->data->gdal_type == GDT_CFloat32 )
    {
        float 	*f_data = (float *) data;

        for( i = 0; i < pixels; i++ )
        {
            if( f_data[i*2] == (float) source->nodata_real
                && f_data[i*2+1] == (float) source->nodata_imaginary )
                nodata_mask[i] = 0;
        }
    }
    else
    {
        g_warning( "unhandled type in gv_raster_layer_srctile_check_nodata()");
    }
}
                                      

unsigned char *
gv_raster_layer_srctile_xy_get( GvRasterLayer * layer, int isource,
                                int tile, int lod, int * needs_free,
                                unsigned char * nodata_mask )

{
    GvRasterSource *source;
    unsigned char * ret_data = NULL;
    int  pixel_count;

    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), NULL );
    g_return_val_if_fail( layer != NULL, NULL );
    g_return_val_if_fail( isource >= 0 
                          && isource < layer->source_count, NULL );

    source = layer->source_list + isource;
    pixel_count = (layer->tile_x >> lod) * (layer->tile_y >> lod);

    if( source->data != NULL )
    {
        *needs_free = FALSE;

        ret_data = gv_raster_tile_get( source->data, tile, lod );
        if( ret_data == NULL )
            return NULL;

        if( nodata_mask != NULL && source->nodata_active )
            gv_raster_layer_srctile_check_nodata( source, pixel_count, 
                                                  ret_data, nodata_mask );

        if( source->data->gdal_type != GDT_Byte 
            || source->min != 0.0 
            || source->max != 255.0 )
        {
            *needs_free = TRUE;
            
            if (( layer->mode == GV_RLM_COMPLEX ) &&
                (gv_data_get_property( GV_DATA(layer),"last_complex_lut" ) != NULL) &&
                (strcmp(gv_data_get_property( GV_DATA(layer),"last_complex_lut" ),"phase") == 0))
            {
                ret_data = gv_scale_pure_phase_tile_to_byte( ret_data, 
                                              layer->tile_x >> lod, 
                                              layer->tile_y >> lod, 
                                              source->data->gdal_type );
            }
            else if (( layer->mode == GV_RLM_RGBA ) &&
                     (gv_data_get_property( GV_DATA(layer),"last_complex_lut" ) != NULL) &&
                     (strcmp(gv_data_get_property( GV_DATA(layer),"last_complex_lut" ),"phase") == 0) &&
                     (source->data->gdal_type == GDT_CFloat32) )
            {
                ret_data = gv_scale_pure_phase_tile_to_byte( ret_data, 
                                              layer->tile_x >> lod, 
                                              layer->tile_y >> lod, 
                                              source->data->gdal_type );
            }
            else
            {
                ret_data = gv_scale_tile_to_byte(ret_data, layer->tile_x >> lod,
						 layer->tile_y >> lod, source);
            }
        }

    }
    else if( layer->mode == GV_RLM_COMPLEX )
    {
        int  i;

        ret_data = (unsigned char *) g_malloc(pixel_count*2);
        *needs_free = TRUE;

        for( i = 0; i < pixel_count; i++ )
        {
            ret_data[i*2  ] = source->const_value;
            ret_data[i*2+1] = 0;
        }
    }
    else
    {
        ret_data = (unsigned char *) g_malloc(pixel_count);
        *needs_free = TRUE;

        memset( ret_data, source->const_value, pixel_count );
    }

    /* Do we have an LUT to apply?   Note: The following will now never
       be called, as source->lut is cleared when a new gv_lut is created
       and applied in scale_tile_to_byte.

    if( source->lut != NULL && layer->mode != GV_RLM_COMPLEX &&
        ((source->data == NULL) || 
         (source->data != NULL && source->data->gdal_type != GDT_CFloat32)))
    {
        int  i;
        unsigned char *lut = source->lut;

        if( ! *needs_free )
        {
            unsigned char *temp;

            temp = (unsigned char *) g_malloc(pixel_count);
            memcpy( temp, ret_data, pixel_count );
            ret_data = temp;
            *needs_free = TRUE;
        }

        for( i = 0; i < pixel_count; i++ )
        {
            ret_data[i] = lut[ret_data[i]];
        }
    }
    */

    return ret_data;
}

unsigned char *
gv_raster_layer_source_get_lut( GvRasterLayer * layer, int isource )
{
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), NULL );
    g_return_val_if_fail( layer != NULL, NULL );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count,NULL);

    return layer->source_list[isource].lut;
}

unsigned char 
gv_raster_layer_get_const_value( GvRasterLayer * layer, int isource )
{
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 0 );
    g_return_val_if_fail( layer != NULL, 0 );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count, 0 );

    return layer->source_list[isource].const_value;
}

GvRaster *
gv_raster_layer_get_data( GvRasterLayer * layer, int isource )
{
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), NULL );
    g_return_val_if_fail( layer != NULL, NULL );
    if( isource < 0 || isource >= layer->source_count )
        return NULL;
    else
        return layer->source_list[isource].data;
}

int
gv_raster_layer_min_set( GvRasterLayer *layer, int isource, float min )
{
    GvRasterSource *source;

    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 1 );
    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count, 1 );

    source = layer->source_list + isource;

    if( min != source->min )
    {
        source->min = min;
        gv_raster_layer_purge_all_textures( layer );
        gv_layer_display_change( GV_LAYER(layer), NULL );
    }

    return 0;
}

float
gv_raster_layer_min_get( GvRasterLayer *layer, int isource )
{
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 1 );
    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count, 1 );

    return layer->source_list[isource].min;
}

int
gv_raster_layer_max_set( GvRasterLayer *layer, int isource, float max )
{
    GvRasterSource *source;

    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 1 );
    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count, 1 );

    source = layer->source_list + isource;

    if( max != source->max )
    {
        source->max = max;
        gv_raster_layer_purge_all_textures( layer );
        gv_layer_display_change( GV_LAYER(layer), NULL );
    }

    return 0;
}

float gv_raster_layer_max_get( GvRasterLayer *layer, int isource )
{
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 1 );
    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count, 1 );

    return layer->source_list[isource].max;
}

/************************************************************************/
/*                       gv_raster_layer_nodata_set()                   */
/************************************************************************/

/**
 * Sets NODATA value for the specified layer.
 *
 * @param layer Pointer to GvRasterLayer object.
 * 
 * @param isource Source index.
 *
 * @param nodata_real Real part of the variable containing NODATA value.
 * 
 * @param nodata_imaginary Imaginary part of the variable containing
 * NODATA value.
 *
 * @return TRUE in case of success and FALSE otherwise.
 */

gint
gv_raster_layer_nodata_set( GvRasterLayer *layer, int isource,
                            double nodata_real, double nodata_imaginary )
{
    GvRasterSource *source;

    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), FALSE );
    g_return_val_if_fail( layer != NULL, FALSE );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count,
                          FALSE );

    source = layer->source_list + isource;
    if( nodata_real != source->nodata_real
        || nodata_imaginary != source->nodata_imaginary )
    {
        source->nodata_real = nodata_real;
        source->nodata_imaginary = nodata_imaginary;
        source->nodata_active = TRUE;
        gv_raster_layer_blend_mode_set( layer, 
                                        GV_RASTER_LAYER_BLEND_MODE_FILTER,
                                        0, 0 );
        gv_raster_layer_purge_all_textures( layer );
        gv_layer_display_change( GV_LAYER(layer), NULL );
    }

    return TRUE;
}

/************************************************************************/
/*                       gv_raster_layer_nodata_get()                   */
/************************************************************************/

/**
 * Queries NODATA value for the specified layer.
 *
 * @param layer Pointer to GvRasterLayer object.
 * 
 * @param isource Source index.
 *
 * @param nodata_real Pointer to the varible which will be filled with real
 * part of the NODATA value, may be NULL
 * 
 * @param nodata_imaginary Pointer to the varible which will be filled with
 * imaginary part of the NODATA value, may be NULL.
 *
 * @return TRUE if specified layer has NODATA set and FALSE otherwise.
 */

gint
gv_raster_layer_nodata_get( GvRasterLayer *layer, int isource,
                            double *nodata_real, double *nodata_imaginary )
{
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), FALSE );
    g_return_val_if_fail( layer != NULL, FALSE );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count,
                          FALSE );

    if ( layer->source_list[isource].nodata_active )
    {
        if ( nodata_real )
            *nodata_real = layer->source_list[isource].nodata_real;
        if ( nodata_imaginary )
            *nodata_imaginary = layer->source_list[isource].nodata_imaginary;
        return TRUE;
    }

    return FALSE;
}

/************************************************************************/
/*                       gv_raster_layer_type_get()                     */
/************************************************************************/

/**
 * Queries type of the values for specified layer and source.
 *
 * @param layer Pointer to GvRasterLayer object.
 * 
 * @param isource Source index.
 * 
 * @return GDAL dtat type of the specified raster.
 */

GDALDataType
gv_raster_layer_type_get( GvRasterLayer *layer, int isource )
{
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 0 );
    g_return_val_if_fail( layer != NULL, 0 );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count, 0 );
    g_return_val_if_fail( layer->source_list[isource].data != NULL, 0 );

    return layer->source_list[isource].data->gdal_type;
}

int gv_raster_layer_set_source( GvRasterLayer *layer, int isource, 
                                GvRaster *data, float min, float max,
                                unsigned char const_value,
                                unsigned char *lut,
                                int nodata_active, 
                                double nodata_real, 
                                double nodata_imaginary )

{
    GvRasterSource *source;

    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 1 );
    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( isource >= 0 && isource < layer->source_count, 1 );

    if( !nodata_active )
    {
        nodata_real = -1e8;
        nodata_imaginary = 0.0;
    }

    if( data != NULL )
    {
        if( data->tile_x != layer->tile_x
            || data->tile_y != layer->tile_y
            || data->width != layer->prototype_data->width
            || data->height != layer->prototype_data->height )
        {
            g_warning( "Attempt to use different GvRaster that doesn't match\n"
                       "prototype raster in gv_raster_layer_set_source()." );
            return 1;
        }
    }

    source = layer->source_list + isource;

    if( source->data == data 
        && source->min == min
        && source->max == max
        && source->const_value == const_value
        && !source->nodata_active == !nodata_active
        && source->nodata_real == nodata_real
        && source->nodata_imaginary == nodata_imaginary
        && source->lut == NULL
        && lut == NULL )
        return 0;

    /* Manage a reference for each source */

    if( source->data != NULL )
        gtk_object_unref( GTK_OBJECT(source->data) );

    if( data != NULL )
    {
        gtk_object_ref( GTK_OBJECT(data) );
        gtk_object_sink( GTK_OBJECT(data) );
    }

    source->data = data;
    source->min = min;
    source->max = max;
    source->const_value = const_value;
    source->nodata_active = nodata_active;
    source->nodata_real = nodata_real;
    source->nodata_imaginary = nodata_imaginary;
    
    if( source->lut != NULL )
    {
        g_free( source->lut );
        source->lut = NULL;
    }

    if( lut != NULL )
    {
        source->lut = (unsigned char *) g_malloc(256);
        memcpy( source->lut, lut, 256 );
    }

    if( source->lut_rgba_composed != NULL )
    {
        g_free( source->lut_rgba_composed );
        source->lut_rgba_composed = NULL;
    }
        
    gv_raster_layer_purge_all_textures( layer );
    gv_layer_display_change( GV_LAYER(layer), NULL );

    /*
    ** Regenerate 2D composed Complex lut if needed.
    */
    gv_raster_layer_lut_compose( layer );

    /* If we have a nodata value, we have to ensure that we have 
       alpha blending turned on */
    if( source->nodata_active )
        gv_raster_layer_blend_mode_set( layer, 
                                        GV_RASTER_LAYER_BLEND_MODE_FILTER,
                                        0, 0 );

    return 0;
}

