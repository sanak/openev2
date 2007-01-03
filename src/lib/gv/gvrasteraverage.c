/******************************************************************************
 * $Id: gvrasteraverage.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Raster downsampling kernels (averaged and otherwise)
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
 * $Log: gvrasteraverage.c,v $
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
 * Revision 1.18  2004/02/18 16:07:27  andrey_kiselev
 * Use gv_raster_get_nodata() instead of GDALGetRasterNoDataValue().
 *
 * Revision 1.17  2002/11/27 04:35:59  warmerda
 * try to mitigate the downward bias of averaging due to truncation in 8bit
 *
 * Revision 1.16  2001/07/24 02:23:53  warmerda
 * removed debugging printf
 *
 * Revision 1.15  2001/07/24 02:21:54  warmerda
 * added 8bit phase averaging
 *
 * Revision 1.14  2001/07/16 15:07:45  warmerda
 * rewrote complex averaging to precisely average magnitudes
 *
 * Revision 1.13  2001/07/13 22:15:36  warmerda
 * added nodata aware averaging
 *
 * Revision 1.12  2001/01/30 19:27:51  warmerda
 * added logic to try and preserve stddev in complex average code
 *
 * Revision 1.11  2000/09/27 19:17:28  warmerda
 * added new _sample functions
 *
 * Revision 1.10  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <math.h>
#include "gvraster.h"
#include "gvrasteraverage.h"

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

/* This function does the equivelent of the GDAL "MAGPHASE" averaging.
 * It produces a magnitude weighted average of phase (by operating in 
 * real/imaginary space) and then scales the resulting magnitude by 
 * the averaged magnitude to preserve the average magnitude.
 */

float *gv_raster_float_complex_average( GvRaster *raster, 
                                        float *iq, int tsize_x, int tsize_y )
{
    int i, base, averaged_size, next_base;
    float *iq_average, target_mag, ratio, prefix_mag;

    averaged_size = (tsize_x * tsize_y) / 4;

    if( ( iq_average = g_new( float, averaged_size*2 ) ) == NULL )
    {
	return NULL;
    }

    for( i = 0; i < averaged_size; i++ )
    {
	base = ( ( ( i * 2 ) / tsize_x ) * tsize_x * 2 ) + ( 4 * i );
        next_base = base + tsize_x*2;

	iq_average[2*i] = ( iq[base] + iq[base+2] + iq[next_base] + iq[next_base+2] ) / 4;
	iq_average[2*i+1] = ( iq[base+1] + iq[base+3] + iq[next_base+1] + iq[next_base+3] ) / 4;

        /*
         * Compute average of input magnitudes and use this to scale the output
         * magnitude.
         */
        target_mag = (
            sqrt(iq[base]*iq[base] + iq[base+1]*iq[base+1])
          + sqrt(iq[base+2]*iq[base+2] + iq[base+3]*iq[base+3]) 
          + sqrt(iq[next_base]*iq[next_base] + iq[next_base+1]*iq[next_base+1])
          + sqrt(iq[next_base+2]*iq[next_base+2] + iq[next_base+3]*iq[next_base+3])
            ) / 4.0;

        prefix_mag = sqrt(iq_average[2*i]*iq_average[2*i]
                          + iq_average[2*i+1]*iq_average[2*i+1]);

        if( prefix_mag != 0.0 && target_mag != 0.0 )
        {
            ratio = target_mag / prefix_mag;
            iq_average[2*i] *= ratio;
            iq_average[2*i+1] *= ratio;
        }
    }
    return iq_average;
}

float *gv_raster_float_complex_sample( GvRaster *raster,
                                       float *iq, int tsize_x, int tsize_y )
{
    float *avg, *buf_out;
    int i, e;

    if( ( avg = g_new( float, tsize_x * tsize_y / 2 ) ) == NULL )
    {
	return NULL;
    }

    buf_out = avg;
    for( i = 0; i < tsize_y; i += 2 )
    {
        float	*buf_in;

        buf_in = iq + i * tsize_x * 2;

	for( e = tsize_x/2; e > 0; e-- )
	{
            *(buf_out++) = *(buf_in++);
            *(buf_out++) = *(buf_in++);
            buf_in += 2;
	}
    }

    return avg;
}

float *gv_raster_float_real_average( GvRaster *raster, 
                                     float *buffer, int tsize_x, int tsize_y )
{
    float *avg, total;
    int i, e, base;

    if( ( avg = g_new( float, tsize_x * tsize_y / 4 ) ) == NULL )
    {
	return NULL;
    }

    for( i = 0; i < tsize_y >> 1; i++ )
    {
	for( e = 0; e < tsize_x >> 1; e++ )
	{
	    base = 2*((i*tsize_x)+e);
	    total = buffer[base];
	    total += buffer[base+1];
	    total += buffer[base+tsize_x];
	    total += buffer[base+tsize_x+1];

	    avg[i*(tsize_y>>1)+e] = total / 4;
	}
    }

    return avg;

}

float *gv_raster_float_real_average_nodata( GvRaster *raster, float *buffer, 
                                            int tsize_x, int tsize_y )
{
    int     i, e, base;
    float   *avg, total;
    double  nodata;

    gv_raster_get_nodata( raster, &nodata );

    if( ( avg = g_new( float, tsize_x * tsize_y / 4 ) ) == NULL )
    {
	return NULL;
    }

    for( i = 0; i < tsize_y >> 1; i++ )
    {
	for( e = 0; e < tsize_x >> 1; e++ )
	{
            int		count = 0;

            total = 0;
	    base = 2*((i*tsize_x)+e);
            
            if( buffer[base] != (float)nodata )
            {
                total += buffer[base];
                count++;
            }
            if( buffer[base+1] != (float)nodata )
            {
                total += buffer[base+1];
                count++;
            }
            if( buffer[base+tsize_x] != (float)nodata )
            {
                total += buffer[base+tsize_x];
                count++;
            }
            if( buffer[base+tsize_x+1] != (float)nodata )
            {
                total += buffer[base+tsize_x+1];
                count++;
            }

            if( count > 0 )
                avg[i*(tsize_y>>1)+e] = total / count;
            else
                avg[i*(tsize_y>>1)+e] = (float)nodata;
	}
    }

    return avg;

}

float *gv_raster_float_real_sample( GvRaster *raster, 
                                    float *buffer, int tsize_x, int tsize_y )
{
    float *avg, *buf_out;
    int i, e;

    if( ( avg = g_new( float, tsize_x * tsize_y / 4 ) ) == NULL )
    {
	return NULL;
    }

    buf_out = avg;
    for( i = 0; i < tsize_y; i += 2 )
    {
        float	*buf_in;

        buf_in = buffer + i * tsize_x;

	for( e = tsize_x/2; e > 0; e-- )
	{
            *(buf_out++) = *buf_in;
            buf_in += 2;
	}
    }

    return avg;
}

/* Convert 8bit phase into complex value with magnitude 1. */

static void gvrf_8bit_phase_to_complex( int in_phase, float *r, float *i )
{
    float	phase = (in_phase / 256.0) * 2 * M_PI;

    if( phase > M_PI )
        phase -= 2*M_PI;

    *r = cos(phase);
    *i = sin(phase);
}

/*
** This is a specialized algorithm to average 8bit phase information.
** The phase is rescaled into 0->2PI (from 0-255), translated into unit
** length complex vectors and then averaged.  The resulting complex value
** is converted back into phase and rescaled into 0-255. 
*/

unsigned char *gv_raster_byte_realphase_average( GvRaster *raster, 
                          unsigned char *buffer, int tsize_x, int tsize_y )
{
    unsigned char *avg;
    int i, e, base;

    if( ( avg = g_new( unsigned char, tsize_x * tsize_y / 4 ) ) == NULL )
    {
	return NULL;
    }

    for( i = 0; i < tsize_y >> 1; i++ )
    {
	for( e = 0; e < tsize_x >> 1; e++ )
	{
            float total_r = 0.0, total_i = 0.0;
            float real, imag, phase;

	    base = 2*((i*tsize_x)+e);

            gvrf_8bit_phase_to_complex( buffer[base], &real, &imag );
            total_r += real;
            total_i += imag;

            gvrf_8bit_phase_to_complex( buffer[base+1], &real, &imag );
            total_r += real;
            total_i += imag;

            gvrf_8bit_phase_to_complex( buffer[base+tsize_x], &real, &imag );
            total_r += real;
            total_i += imag;

            gvrf_8bit_phase_to_complex( buffer[base+tsize_x+1], &real, &imag );
            total_r += real;
            total_i += imag;

            real = total_r * 0.25;
            imag = total_i * 0.25;
            
            phase = atan2(imag, real);
            while( phase < 0.0 )
                phase += 2 * M_PI;
            while( phase >= 2*M_PI )
                phase -= 2 * M_PI;

	    avg[i*(tsize_y>>1)+e] = floor((phase * 256) / (2*M_PI));
	}
    }

    return avg;
}

unsigned char *gv_raster_byte_real_average( GvRaster *raster, 
                                            unsigned char *buffer, 
                                            int tsize_x, int tsize_y )
{
    unsigned char *avg;
    int i, e, total, base;

    if( ( avg = g_new( unsigned char, tsize_x * tsize_y / 4 ) ) == NULL )
    {
	return NULL;
    }

    for( i = 0; i < tsize_y >> 1; i++ )
    {
	for( e = 0; e < tsize_x >> 1; e++ )
	{
	    base = 2*((i*tsize_x)+e);
	    total = buffer[base];
	    total += buffer[base+1];
	    total += buffer[base+tsize_x];
	    total += buffer[base+tsize_x+1];

	    avg[i*(tsize_y>>1)+e] = (total+2) / 4;
	}
    }

    return avg;
}

unsigned char *gv_raster_byte_real_sample( GvRaster *raster, 
                                           unsigned char *buffer, 
                                           int tsize_x, int tsize_y )
{
    unsigned char *avg;
    int i, e, base;

    if( ( avg = g_new( unsigned char, tsize_x * tsize_y / 4 ) ) == NULL )
    {
	return NULL;
    }

    for( i = 0; i < tsize_y >> 1; i++ )
    {
	for( e = 0; e < tsize_x >> 1; e++ )
	{
	    base = 2*((i*tsize_x)+e);
	    avg[i*(tsize_y>>1)+e] = buffer[base];
	}
    }

    return avg;
}

