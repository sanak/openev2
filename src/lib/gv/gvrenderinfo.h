/******************************************************************************
 * $Id: gvrenderinfo.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Functions for managing per-shape rendering information in
 *           in the GvShapeLayer.  Actual rendering of shapes using the
 *           rendering information is still in GvShapesLayer.
 * Author:   Frank Warmerdam <warmerdam@pobox.com>
 *
 ******************************************************************************
 * Copyright (c) 2001, Frank Warmerdam <warmerdam@pobox.com>
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
 * $Log: gvrenderinfo.h,v $
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
 * Revision 1.15  2003/06/25 16:42:18  warmerda
 * gv_get_ogr_arg() made public as gv_ogrfs_get_arg().
 * gv_split_tools() made public as gv_ogrfs_split_tools().
 *
 * Revision 1.14  2003/05/16 18:26:32  pgs
 * added initial code for propogating colors to sub-symbols
 *
 * Revision 1.13  2003/04/09 16:52:22  pgs
 * added shadow, halo and bgcolor to LABELs
 *
 * Revision 1.12  2003/04/07 15:10:12  pgs
 * added pattern support to pen objects
 *
 * Revision 1.11  2003/03/02 17:05:11  warmerda
 * removed unit_vector from renderinfo args
 *
 * Revision 1.10  2003/02/28 16:49:05  warmerda
 * split up renderinfo parsing to use for vector symbols
 *
 * Revision 1.9  2003/02/14 20:12:43  pgs
 * added support for line widths in PENs
 *
 * Revision 1.8  2002/11/15 05:04:43  warmerda
 * added LABEL anchor point support
 *
 * Revision 1.7  2002/11/14 22:05:00  warmerda
 * implement offsets for symbols
 *
 * Revision 1.6  2002/11/14 20:11:21  warmerda
 * preliminary support for gvsymbolmanager from Paul
 *
 * Revision 1.5  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.4  2002/02/22 20:16:07  warmerda
 * added brush tool support
 *
 * Revision 1.3  2002/02/22 19:27:16  warmerda
 * added support for pen tools
 *
 * Revision 1.2  2001/04/25 20:36:01  warmerda
 * added proper support for descenders in label text
 *
 * Revision 1.1  2001/04/09 18:22:54  warmerda
 * New
 *
 */

#ifndef __GV_RENDER_INFO_H__
#define __GV_RENDER_INFO_H__

#include "gvshapeslayer.h"

#define GvReservedPart  0x00
#define GvLabelPart 0x01
#define GvSymbolPart    0x02
#define GvPenPart   0x03
#define GvBrushPart 0x04

#define gv_part_index_to_type(x)    ((x) & 0x07)
#define gv_part_index_to_index(x)   ((x) >>3)

/* -------------------------------------------------------------------- */
/*      It is intended that GvRenderPart is like a subclass of the      */
/*      more specific render infos, but for symplicity of               */
/*      referencing fields we don't make this relationship explicit.    */
/* -------------------------------------------------------------------- */

#define GVP_UNINITIALIZED_PART  0x00
#define GVP_LAST_PART           0x08

typedef struct
{
  unsigned int  next_part;  /* 0 means this object is uninitialized.
                                 * 8 means this is the last part
                                 * Otherwise mask the GvXXXPart number
                                 * out of the lower 3 bits, and shift the
                                 * remainder down for the index into the table.
                                 */

} GvRenderPart;

typedef struct
{
    /* From GvRenderPart */
    guint         next_part;

    /* Specific to GvLabelRenderPart */
    GvColor   color;
    /* for tracking if the color was actually initialized
       as part of the symbol definition */
    int           b_color_initialized;
    GvColor   background_color;
    /* for tracking if the color was actually initialized
       as part of the symbol definition */
    int           b_background_color_initialized;
    char      *text;
    gint      font;
    gvgeocoord    x_offset_px;
    gvgeocoord    y_offset_px;
    gvgeocoord    x_offset_g;
    gvgeocoord    y_offset_g;
    gvgeocoord    angle;
    gvgeocoord    scale;

    int       width;
    int       height;
    int       descent;

    int       anchor; /* GLRA_ * */

    gboolean  shadow;
    gboolean  halo;

} GvLabelRenderPart;

#define GLRA_LOWER_LEFT             1
#define GLRA_LOWER_CENTER       2
#define GLRA_LOWER_RIGHT        3
#define GLRA_CENTER_LEFT                4
#define GLRA_CENTER_CENTER              5
#define GLRA_CENTER_RIGHT               6
#define GLRA_UPPER_LEFT                 7
#define GLRA_UPPER_CENTER               8
#define GLRA_UPPER_RIGHT                9

typedef struct
{
    /* From GvRenderPart */
    guint         next_part;

    /* Specific to GvSymbolRenderPart */
    GvColor       color;
    gchar        *symbol_id;
    gvgeocoord    scale;
    gvgeocoord    angle;

    gvgeocoord    x_offset_g;
    gvgeocoord    y_offset_g;
    gvgeocoord    x_offset_px;
    gvgeocoord    y_offset_px;

    /* for tracking if the color was actually initialized
       as part of the symbol definition */
    int           b_color_initialized;

    int           part_index; /* for vector symbols */

} GvSymbolRenderPart;

typedef struct
{
    /* From GvRenderPart */
    guint         next_part;

    /* Specific to GvPenRenderPart */
    GvColor       color;
    /* for tracking if the color was actually initialized
       as part of the pen definition */
    int           b_color_initialized;

    float         width;
    gchar        *pattern;
} GvPenRenderPart;

typedef struct
{
    /* From GvRenderPart */
    guint         next_part;

    /* Specific to GvBrushRenderPart */
    GvColor   fore_color;
    /* for tracking if the color was actually initialized
       as part of the brush definition */
    int           b_fore_color_initialized;
} GvBrushRenderPart;

/* -------------------------------------------------------------------- */
/*      Functions for operating on rendering info within                */
/*      GvShapeLayer, really an extension to GvShapeLayer API.          */
/* -------------------------------------------------------------------- */
GvRenderPart *gv_shape_layer_get_part(GvShapeLayer *layer, guint part_index);
guint gv_shape_layer_get_first_part_index(GvShapeLayer *layer, gint shape_id);

guint gv_shape_layer_add_part(GvShapeLayer *layer, gint shape_id,
                               gint part_type);
guint gv_shape_layer_create_part(GvShapeLayer *layer, gint part_type);
guint gv_shape_layer_chain_part(GvShapeLayer *layer, gint base_part_index,
                                 gint new_part_index);
void gv_shape_layer_clear_shape_parts(GvShapeLayer *layer, gint shape_id);
void gv_shape_layer_clear_part(GvShapeLayer *layer, guint part_index);
void gv_shape_layer_clear_all_renderinfo(GvShapeLayer *layer);

guint gv_shape_layer_build_renderinfo(GvShapeLayer *s_layer,
                                       GvShape *shape_obj,
                                       int *scale_dep);
void gv_shape_layer_update_renderinfo(GvShapeLayer *s_layer, int shape_id);
void gv_shape_layer_initialize_renderindex(GvShapeLayer *layer);

/* -------------------------------------------------------------------- */
/*      Helper functions for ogrfs parsing ... used in a few other      */
/*      places.                                                         */
/* -------------------------------------------------------------------- */
char **gv_ogrfs_split_tools(const char *tool_list_in);
const char *gv_ogrfs_get_arg(const char *def, char **next_def,
                              char **value, int *value_len);



#endif /*__GV_RENDER_INFO_H__ */



