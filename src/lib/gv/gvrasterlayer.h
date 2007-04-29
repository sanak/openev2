/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Raster display layer (managed textures, redraw, etc)
 * Author:   OpenEV Team
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

#ifndef __GV_RASTER_LAYER_H__
#define __GV_RASTER_LAYER_H__

#include <gtk/gtkgl.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include "gvlayer.h"
#include "gvraster.h"


#define GV_TYPE_RASTER_LAYER            (gv_raster_layer_get_type ())
#define GV_RASTER_LAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_RASTER_LAYER, GvRasterLayer))
#define GV_RASTER_LAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_RASTER_LAYER, GvRasterLayerClass))
#define GV_IS_RASTER_LAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_RASTER_LAYER))
#define GV_IS_RASTER_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_CLASS_LAYER))

typedef struct _GvRasterLayer       GvRasterLayer;
typedef struct _GvRasterLayerClass  GvRasterLayerClass;

/* Need to include here since we need GvRasterLayer definition */

enum {
    GV_RASTER_LAYER_BLEND_MODE_OFF = 0,
    GV_RASTER_LAYER_BLEND_MODE_FILTER,
    GV_RASTER_LAYER_BLEND_MODE_MULTIPLY,
    GV_RASTER_LAYER_BLEND_MODE_ADD,
    GV_RASTER_LAYER_BLEND_MODE_CUSTOM
};

typedef enum {
    GV_RLM_AUTO = 0,
    GV_RLM_SINGLE = 1,
    GV_RLM_RGBA = 2, 
    GV_RLM_COMPLEX = 3,
    GV_RLM_PSCI = 4
} GvRasterLayerMode;

typedef struct _GvRasterLayerTexObj
{
    struct _GvRasterLayerTexObj *prev;
    struct _GvRasterLayerTexObj *next;
    GvRasterLayer      *layer;

    int    texture;
    GLuint tex_obj; /* Name of the texture object for this tile */
    int    lod;
    int    size;
    int    last_render;
} GvRasterLayerTexObj;


struct _GvRasterLayerGLDispInfo
{
    /* Do we replace or modulate? */
    gint tex_env_mode; /* GL_REPLACE and GL_MODULATE */
    gint mag_filter;
    gint min_filter;
    gint s_wrap;
    gint t_wrap;
    GvColor fragment_color; /* Color if we modulate */

    /* Is blending enabled? */
    gint blend_enable;
    gint blend_src, blend_dst; /* Src and dst parameters */

    /* Is alpha test enabled? */
    gint alpha_test;
    gint alpha_test_mode;
    GLfloat alpha_test_val;
};
    
typedef struct _GvRasterLut GvRasterLut;

struct _GvRasterLut {
    gfloat min;
    gfloat max;
    gint enhance_type;
    gint size;
    unsigned char *table;
};

typedef struct {

    GvRaster    *data;
    unsigned char const_value;  /* used for whole tile if data is NULL */
    float       max, min;       /* for scaling */
    unsigned char *lut;         /* greyscale lut (uchar*256 or NULL) */
    GvRasterLut *gv_lut;        /* higher res lut created from 256 entry lut */

    /* lut_rgba_composed is used only in the RGBA case in the presence  */
    /* of complex bands.  It is equivalent to the pc_lut_composed       */
    /* used in the single complex band case, but since there are        */
    /* potentially multiple bands, it's stored here instead.            */
    unsigned char *lut_rgba_composed;

    int         nodata_active;  /* flag indicating of nodata value meaningful*/
    double      nodata_real;
    double      nodata_imaginary;

} GvRasterSource;

#define MAX_RASTER_SOURCE 4

struct _GvRasterLayer
{
    GvLayer layer;

    GvRasterLayerMode mode;

    gint      mesh_is_raw;
    gint      mesh_is_dirty;
    GvMesh   *mesh;

    int       tile_x, tile_y;

    GvRasterLayerTexObj **textures;

    /* Sources list */

    int       source_count;
    GvRasterSource source_list[MAX_RASTER_SOURCE];

    GvRaster *prototype_data;

    /* Final color table */
    unsigned char *pc_lut;
    unsigned char *pc_lut_composed;

    /* LUT to use in RGB/RGBA case for any complex bands found */
    unsigned char *pc_lut_rgba_complex;

    /* OpenGL display properties */

    struct _GvRasterLayerGLDispInfo gl_info;

    /* Texture load information */

    GArray *missing_tex;

    /* Current tilelist display */

    GArray *tile_list;
};

struct _GvRasterLayerClass
{
    GvLayerClass parent_class;
};

GType gv_raster_layer_get_type(void);
GObject *gv_raster_layer_new(int mode, 
                               GvRaster *prototype_data, 
                               GvProperties prop);

void gv_raster_layer_read(GvRasterLayer *layer, int mode, 
                           GvRaster *prototype_data, 
                           GvProperties prop);

/* Raster value setup functions */
long gv_raster_layer_texture_clamp_set(GvRasterLayer *layer, int s_clamp, int t_clamp);
long gv_raster_layer_zoom_set(GvRasterLayer *layer, int mag_mode, int min_mode);
long gv_raster_layer_zoom_get(GvRasterLayer *layer, int *mag_mode, int *min_mode);
long gv_raster_layer_alpha_set(GvRasterLayer *layer, int alpha_mode, float alpha_check_val);
long gv_raster_layer_texture_mode_set(GvRasterLayer *layer, int texture_mode, GvColor color);
long gv_raster_layer_blend_mode_set(GvRasterLayer *layer, int blend_mode, int sfactor, int dfactor);
void gv_raster_layer_purge_all_textures(GvRasterLayer *layer);

/* Raster value query functions */
int gv_raster_layer_get_mode(GvRasterLayer *layer);
long gv_raster_layer_blend_mode_get(GvRasterLayer *layer, int *blend_mode, int *sfactor, int *dfactor);
long gv_raster_layer_alpha_get(GvRasterLayer *layer, int *alpha_mode, float *alpha_check_val);
long gv_raster_layer_texture_mode_get(GvRasterLayer *layer, int *texture_mode, GvColor *color);

/* source related */
float gv_raster_layer_max_get(GvRasterLayer *layer, int isource);
float gv_raster_layer_min_get(GvRasterLayer *layer, int isource);
unsigned char *gv_raster_layer_source_get_lut(GvRasterLayer *layer, int isource);
unsigned char gv_raster_layer_get_const_value(GvRasterLayer *layer, int isource);

GvRaster *gv_raster_layer_get_data(GvRasterLayer *layer, int isource);

int gv_raster_layer_set_source(GvRasterLayer *layer, int isource, 
                                GvRaster *data, float min, float max, 
                                unsigned char const_value, 
                                unsigned char *lut,
                                int nodata_active, 
                                double nodata_real,
                                double nodata_imaginary);

int gv_raster_layer_min_set(GvRasterLayer *layer, int isource, float min);
int gv_raster_layer_max_set(GvRasterLayer *layer, int isource, float max);
gint gv_raster_layer_nodata_set(GvRasterLayer *layer, int isource,
                                 double nodata_real, double nodata_imaginary);
gint gv_raster_layer_nodata_get(GvRasterLayer *layer, int isource,
                                 double *nodata_real,
                                 double *nodata_imaginary);
GDALDataType gv_raster_layer_type_get(GvRasterLayer *layer, int isource);
    
/* transformation */
gint gv_raster_layer_pixel_to_view(GvRasterLayer *raster, 
                                   double *x, double *y, double *z);
gint gv_raster_layer_view_to_pixel(GvRasterLayer *raster, 
                                   double *x, double *y, double *z);

/* other */
gint gv_raster_layer_autoscale_view(GvRasterLayer *rlayer, int isrc, 
                                     GvAutoScaleAlg alg, double alg_param, 
                                     double *out_min, double *out_max);

gint gv_raster_layer_histogram_view(GvRasterLayer *rlayer, int isrc,
                                     double scale_min, double scale_max,
                                     int include_out_of_range,
                                     int bucket_count, int *histogram);

double gv_raster_layer_pixel_size(GvRasterLayer *layer);
void gv_raster_layer_add_height(GvRasterLayer *layer, 
                                 GvRaster *height_raster,
                                 double default_height);
void gv_raster_layer_clamp_height(GvRasterLayer *layer, 
                                   int bclamp_min, int bclamp_max,
                                   double min_height, double max_height);
unsigned char *
gv_raster_layer_srctile_xy_get(GvRasterLayer *layer, int isource, 
                                int tile, int lod, int * needs_free,
                                unsigned char *nodata_mask);
unsigned char *gv_raster_layer_gltile_get(GvRasterLayer *layer, 
                                           int tile, int lod,
                                           int *format, int *type, 
                                           int *needs_free);

int gv_raster_layer_set_raw(GvRasterLayer *layer, int raw_enable);
void gv_raster_layer_refresh_mesh(GvRasterLayer *layer);

/* texture cache functions */
void gv_raster_layer_purge_texture(GvRasterLayer *layer, int texture);
void gv_raster_layer_create_texture(GvRasterLayer *layer, int texture,
                                     GLuint tex_obj, int lod, int size);
void gv_raster_layer_touch_texture(GvRasterLayer *layer, int texture);
void gv_raster_layer_reset_texture(GvRasterLayer *layer, int texture,
                                    int lod, int size);

void gv_texture_cache_set_max(int max);
int gv_texture_cache_get_max();
int gv_texture_cache_get_used();
void gv_texture_cache_dump();

GArray *gv_mesh_tilelist_get(GvMesh *mesh, GvViewArea *view,
                             GvRasterLayer *rlayer, GArray *tilelist);

/* Skirt functions */
GvLayer *gv_build_skirt(GvRasterLayer *dem_layer, double base_z);

#endif /* __GV_RASTER_LAYER_H__ */

