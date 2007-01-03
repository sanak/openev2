/******************************************************************************
 * $Id: gvrasterlut.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Compute, and apply LUT to GvRaster.
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
 * $Log: gvrasterlut.h,v $
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
 * Revision 1.14  2004/06/23 14:35:05  gmwalter
 * Added support for multi-band complex imagery.
 *
 * Revision 1.13  2001/10/17 16:23:52  warmerda
 * added support for composing complex lut and pct
 *
 * Revision 1.12  2001/07/24 21:21:45  warmerda
 * added EV style phase colormap
 *
 * Revision 1.11  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef _GV_RASTER_LAYER_LUT_H_
#define _GV_RASTER_LAYER_LUT_H_

#include <gtk/gtk.h>
#include "gvrasterlayer.h"

enum {
    GV_RASTER_LAYER_LUT_MAGNITUDE,
    GV_RASTER_LAYER_LUT_PHASE_ANGLE,
    GV_RASTER_LAYER_LUT_SCALAR,
    GV_RASTER_LAYER_LUT_REAL,
    GV_RASTER_LAYER_LUT_IMAGINARY
};

enum {
    GV_RASTER_LAYER_LUT_NONE = 0,
    GV_RASTER_LAYER_LUT_1D,
    GV_RASTER_LAYER_LUT_2D
};

typedef enum {
    GV_RASTER_LUT_ENHANCE_UNKNOWN = 0,
    GV_RASTER_LUT_ENHANCE_LINEAR,
    GV_RASTER_LUT_ENHANCE_LOG,
    GV_RASTER_LUT_ENHANCE_ROOT,
    GV_RASTER_LUT_ENHANCE_SQUARE,
    GV_RASTER_LUT_ENHANCE_EQUALIZE,
    GV_RASTER_LUT_ENHANCE_GAMMA
} GvRasterLutEnhancement;

/**
 * Options for creating GvRasterLut from 256 entry byte lut
 * using gv_raster_lut_create_from_byte_lut.  Multiple options
 * may be set by or'ing them together.
 * 
 * GV_RASTER_LUT_OPTIONS_ALWAYS_INCREASE - To create cyclical lut
 */
#define GV_RASTER_LUT_OPTIONS_NONE            0x0
#define GV_RASTER_LUT_OPTIONS_ALWAYS_INCREASE 0x1

typedef struct _GvRasterLayerLutInterpolate GvRasterLayerLutInterpolate;

struct _GvRasterLayerLutInterpolate {
    guchar color[4];
    gint index;
};

int gv_raster_layer_lut_color_wheel_new_ev(GvRasterLayer *layer, gint set_phase, gint set_magnitude);
int gv_raster_layer_lut_color_wheel_new(GvRasterLayer *layer, gint h_type, gfloat h_param, gint s_type, gfloat s_param, gint v_type, gfloat v_param);
int gv_raster_layer_lut_color_wheel_1d_new(GvRasterLayer *layer, float s, float v, float offset);
int gv_raster_layer_lut_interpolated_new(GvRasterLayer *layer, GvRasterLayerLutInterpolate *color_pair, int offset);
int gv_raster_layer_lut_put(GvRasterLayer *layer, guchar *lut, gint height);
char *gv_raster_layer_lut_get(GvRasterLayer *layer, int *width, int *height, int rgba_complex);
int gv_raster_layer_lut_compose(GvRasterLayer *layer);
long gv_raster_layer_lut_type_get(GvRasterLayer *layer);
int gv_raster_layer_apply_gdal_color_table(GvRasterLayer *layer, GDALColorTableH color_table);
GvRasterLut *gv_raster_lut_create_from_byte_lut(unsigned char *byte_lut, float min, float max, int lut_options);
void gv_raster_lut_free(GvRasterLut *gv_lut);
void gv_raster_lut_print(GvRasterLut *gv_lut, FILE *report);

/* Non-existent functions?  Removed prototypes GTK2 PORT */
/*
int gv_raster_layer_lut_new(GvRasterLayer *layer, gint h_type, gfloat h_param, gint s_type, gfloat s_param, gint v_type, gfloat v_param);
*/

#endif
