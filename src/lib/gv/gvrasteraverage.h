/******************************************************************************
 * $Id: gvrasteraverage.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
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
 * $Log: gvrasteraverage.h,v $
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
 * Revision 1.8  2001/07/24 02:21:54  warmerda
 * added 8bit phase averaging
 *
 * Revision 1.7  2001/07/13 22:15:36  warmerda
 * added nodata aware averaging
 *
 * Revision 1.6  2000/09/27 19:17:28  warmerda
 * added new _sample functions
 *
 * Revision 1.5  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef _GV_RASTER_AVERAGE_H_
#define _GV_RASTER_AVERAGE_H_

unsigned char *gv_raster_byte_real_average(GvRaster *raster, 
                                            unsigned char *buffer,
                                            int tsize_x, int tsize_y);
unsigned char *gv_raster_byte_real_sample(GvRaster *raster,
                                           unsigned char *buffer,
                                           int tsize_x, int tsize_y);

unsigned char *gv_raster_byte_realphase_average(GvRaster *raster,
                             unsigned char *buffer, int tsize_x, int tsize_y);

float *gv_raster_float_real_average_nodata(GvRaster *raster, float *buffer, 
                                            int tsize_x, int tsize_y);
float *gv_raster_float_real_average(GvRaster *raster,
                                     float *buffer, int tsize_x, int tsize_y);
float *gv_raster_float_real_sample(GvRaster *raster,
                                    float *buffer, int tsize_x, int tsize_y);

float *gv_raster_float_complex_average(GvRaster *raster,
                                       float *buffer,int tsize_x,int tsize_y);
float *gv_raster_float_complex_sample(GvRaster *raster,
                                      float *buffer,int tsize_x,int tsize_y);

#endif
