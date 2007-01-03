/******************************************************************************
 * $Id: gvrenderinfo.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Parse OGRFeatureStyle rendering information and build
 *           GvRenderPart structures.  This module is essentially associated
 *           with the GvShapeLayer.
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
 * $Log: gvrenderinfo.c,v $
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
 * Revision 1.23  2003/06/25 16:42:18  warmerda
 * gv_get_ogr_arg() made public as gv_ogrfs_get_arg().
 * gv_split_tools() made public as gv_ogrfs_split_tools().
 *
 * Revision 1.22  2003/05/27 15:17:42  warmerda
 * set pattern from pen id if available
 *
 * Revision 1.21  2003/05/16 18:26:32  pgs
 * added initial code for propogating colors to sub-symbols
 *
 * Revision 1.20  2003/04/09 16:52:21  pgs
 * added shadow, halo and bgcolor to LABELs
 *
 * Revision 1.19  2003/04/07 15:10:10  pgs
 * added pattern support to pen objects
 *
 * Revision 1.18  2003/03/02 17:05:11  warmerda
 * removed unit_vector from renderinfo args
 *
 * Revision 1.17  2003/02/28 16:49:06  warmerda
 * split up renderinfo parsing to use for vector symbols
 *
 * Revision 1.16  2003/02/27 03:59:59  warmerda
 * set scale_dep flag when updating renderinfo
 *
 * Revision 1.15  2003/02/14 20:12:43  pgs
 * added support for line widths in PENs
 *
 * Revision 1.14  2002/11/15 05:04:43  warmerda
 * added LABEL anchor point support
 *
 * Revision 1.13  2002/11/14 22:04:59  warmerda
 * implement offsets for symbols
 *
 * Revision 1.12  2002/11/14 20:11:21  warmerda
 * preliminary support for gvsymbolmanager from Paul
 *
 * Revision 1.11  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.10  2002/05/07 02:51:15  warmerda
 * preliminary support for GVSHAPE_COLLECTION
 *
 * Revision 1.9  2002/03/21 15:48:51  warmerda
 * added support for geometry specific _ogr_ogrfs_... layer properties
 *
 * Revision 1.8  2002/02/22 20:16:07  warmerda
 * added brush tool support
 *
 * Revision 1.7  2002/02/22 19:27:16  warmerda
 * added support for pen tools
 *
 * Revision 1.6  2002/01/21 20:45:39  warmerda
 * Avoid reporting "unsupported tool" too many times.
 *
 * Revision 1.5  2001/08/21 21:41:24  warmerda
 * recover if no text provided
 *
 * Revision 1.4  2001/05/01 19:27:09  warmerda
 * Create an X sized box for empty labels
 *
 * Revision 1.3  2001/04/25 20:35:03  warmerda
 * added proper support for descenders in label text
 *
 * Revision 1.2  2001/04/23 14:09:57  warmerda
 * ensure that GVP_LAST_PART is set if the tool list is corrupt or empty
 *
 * Revision 1.1  2001/04/09 18:22:54  warmerda
 * New
 *
 */

#include "gvrenderinfo.h"
#include "gvshapeslayer.h"
#include "cpl_string.h"
#include <GL/gl.h>

/************************************************************************/
/*                           gv_get_ogr_arg()                           */
/************************************************************************/

const char *
gv_ogrfs_get_arg( const char *def, char **next_def,
                  char **value, int *value_len )

{
    const char *ret;
    int        vlen;

    /* skip to the first argument.  It will be preceeded by '(' or ',' */
    while( *def != '(' && *def != ',' && *def != '\0' )
        def++;

    if( *def == '\0' )
        return NULL;

    def++;
    ret = def;

    /* find start of value */
    while( *def != ':' )
    {
        if( *def == '\0' )
            return NULL;

        def++;
    }

    def++;

    if( *def == '"' )
    {
        def++;
        if( value != NULL )
            *value = (char *) def;

        vlen = 0;
        while( *def != '"' && *def != '\0' )
        {
            def++;
            vlen++;
        }
        if( *def == '"' )
            def++;
    }
    else
    {
        if( value != NULL )
            *value = (char *) def;

        vlen = 0;
        while( *def != ')' && *def != ',' && *def != '\0' )
        {
            def++;
            vlen++;
        }
    }

    if( value_len != NULL )
        *value_len = vlen;

    if( next_def != NULL )
        *next_def = (char *) def;

    return ret;
}

/************************************************************************/
/*                          gv_get_ogr_color()                          */
/************************************************************************/

static int
gv_get_ogr_color( const char * sym_def, GvColor color )

{
    int      red, green, blue, alpha = 255;

    if( EQUALN(sym_def,"c:",2) )
        sym_def += 2;

    if( *sym_def != '#' )
        return FALSE;

    if( sscanf( sym_def+1, "%2x%2x%2x%2x", &red, &green, &blue, &alpha ) < 3 )
        return FALSE;

    color[0] =   red / 255.0;
    color[1] = green / 255.0;
    color[2] =  blue / 255.0;
    color[3] = alpha / 255.0;

    return TRUE;
}

/************************************************************************/
/*                           gv_parse_label()                           */
/************************************************************************/

static guint
gv_parse_label( GvShapesLayer *layer, GvShape *shape, const char * sym_def,
                int *scale_dep )

{
    const char  *id;
    char        *next, *value=NULL;
    guint       part_index = GVP_UNINITIALIZED_PART;
    int         value_len;
    GvLabelRenderPart *label_info;

    if( !EQUALN(sym_def,"LABEL",5) )
    {
        g_warning( "not a label" );
        return part_index;
    }

    *scale_dep = TRUE;

/* -------------------------------------------------------------------- */
/*      Create the new part.                                            */
/* -------------------------------------------------------------------- */
    part_index = gv_shape_layer_create_part( GV_SHAPE_LAYER(layer),
                                             GvLabelPart );

    label_info = (GvLabelRenderPart *)
        gv_shape_layer_get_part( GV_SHAPE_LAYER(layer), part_index );

    g_assert( label_info != NULL );

    label_info->font = -1;

/* -------------------------------------------------------------------- */
/*      Parse the tool information, and apply to the renderinfo part.   */
/* -------------------------------------------------------------------- */

    while( (id = gv_ogrfs_get_arg( sym_def, &next, &value,
                                 &value_len )) != NULL )
    {
        if( EQUALN(id,"c:",2) )
        {
            gv_get_ogr_color( value, label_info->color );
            label_info->b_color_initialized = 1;
        }
        if( EQUALN(id,"b:",2) )
        {
            gv_get_ogr_color( value, label_info->background_color );
            label_info->b_background_color_initialized = 1;
        }
        else if( EQUALN(id,"f:",2) )
        {
            char    *font;

            font = g_strdup(value);
            font[value_len] = '\0';

            label_info->font = gv_view_area_bmfont_load(
                GV_VIEW_AREA(GV_LAYER(layer)->view), font );

            g_free( font );
        }
        else if( EQUALN(id,"t:",2) )
        {
            if( value[0] == '{' )
            {
                GvProperties    *properties = gv_shape_get_properties(shape);
                char        *field_name;

                field_name = g_strdup(value+1);
                field_name[value_len-1] = '\0';
                if( field_name[value_len-2] == '}' )
                    field_name[value_len-2] = '\0';

                label_info->text = g_strdup(
                    gv_properties_get( properties, field_name ));

                g_free( field_name );
            }
            else
            {
                label_info->text = g_malloc(value_len+1);
                strncpy( label_info->text, value, value_len );
                label_info->text[value_len] = '\0';
            }
        }
        else if( EQUALN(id,"a:",2) )
        {
            label_info->angle = atof(value);
        }
        else if( EQUALN(id,"p:",2) )
        {
            label_info->anchor = atoi(value);
            if( label_info->anchor < 1 || label_info->anchor > 9 )
            {
                CPLDebug( "GVRENDERINFO",
                          "Illegal anchor position in label (%d), ignoring.",
                          label_info->anchor );
                label_info->anchor = GLRA_LOWER_LEFT;
            }
        }
        else if( EQUALN(id,"s:",2) )
        {
            label_info->scale = atof(value);
            if( label_info->scale == 0.0 )
                label_info->scale = 1.0;
        }
        else if( EQUALN(id,"dx:",3) )
        {
            if( EQUALN(value+value_len-1,"g",1) )
                label_info->x_offset_g = atof(value);
            else
                label_info->x_offset_px = atof(value);
        }
        else if( EQUALN(id,"dy:",3) )
        {
            if( EQUALN(value+value_len-1,"g",1) )
                label_info->y_offset_g = atof(value);
            else
                label_info->y_offset_px = atof(value);
        }
        else if( EQUALN(id,"h",1) )
        {
            label_info->halo = TRUE;
        }
        else if( EQUALN(id,"sh",2) )
        {
            label_info->shadow = TRUE;
        }

        sym_def = next;
    }

/* -------------------------------------------------------------------- */
/*      Default font to sans .                                          */
/* -------------------------------------------------------------------- */
    if( label_info->font == -1 ) {
	label_info->font = gv_view_area_bmfont_load
	    (GV_VIEW_AREA(GV_LAYER(layer)->view), "Sans 12" );
    }

/* -------------------------------------------------------------------- */
/*      Did we miss out on any critical components?  If so, remove      */
/*      this part.                                                      */
/* -------------------------------------------------------------------- */
    if( label_info->text == NULL ) {
        label_info->text = g_strdup("");
        g_warning( "gvrenderinfo: missing text for label" );

	/* Clearing the part here just causes openev to crash at an
           assertion point in gvshapelayer.  Need better error recovery...
        gv_shape_layer_clear_part( GV_SHAPE_LAYER(layer), part_index );
	*/
    }
    if ( label_info->font == -1 )
    {
        g_warning( "gvrenderinfo: missing font for label" );

	/* Clearing the part here just causes openev to crash at an
           assertion point in gvshapelayer.  Need better error recovery...
        gv_shape_layer_clear_part( GV_SHAPE_LAYER(layer), part_index );
	*/
    }

/* -------------------------------------------------------------------- */
/*      Get string extents.                                             */
/* -------------------------------------------------------------------- */
    if( label_info->font != -1 )
    {
        GvBMFontInfo *font_info;
        PangoContext *pango_context;
        PangoLayout *pango_layout;
        const char *pszText;

        font_info = gv_view_area_bmfont_get_info(
            GV_VIEW_AREA(GV_LAYER(layer)->view), label_info->font );

        if( label_info->text[0] == '\0' )
            pszText = "X";
        else
            pszText = label_info->text;

	/* ---- Get text layout size ---- */
        pango_context = gtk_widget_get_pango_context
	    (GTK_WIDGET((GV_LAYER(layer)->view)));
        pango_layout = pango_layout_new(pango_context);
        pango_layout_set_text(pango_layout, pszText, -1);
        pango_layout_set_font_description(pango_layout, font_info->pango_desc);
	pango_layout_get_pixel_size(pango_layout, &(label_info->width),
				    &(label_info->height));

	/* GTK2 PORT...
        guint   lbearing, rbearing, ascent, descent;
        gdk_string_extents( font_info->gdkfont, pszText,
                            &lbearing, &rbearing, &(label_info->width),
                            &ascent, &descent );

#ifdef WIN32
        //gdk on win32 reports the total height as the ascent
        label_info->height = ascent - descent - 1;
        label_info->descent = descent - 1;
#else
        label_info->height = ascent + descent;
        label_info->descent = descent;
#endif
	*/


    }

/* -------------------------------------------------------------------- */
/*      Adjust pixel offset to take into account the anchor point       */
/*      (justification).                                                */
/* -------------------------------------------------------------------- */
    switch( label_info->anchor )
    {
      case GLRA_LOWER_CENTER:
        label_info->x_offset_px -= label_info->width / 2.0;
        break;

      case GLRA_LOWER_RIGHT:
        label_info->x_offset_px -= label_info->width;
        break;

      case GLRA_CENTER_LEFT:
        label_info->y_offset_px += label_info->height / 2.0;
        break;

      case GLRA_CENTER_CENTER:
        label_info->x_offset_px -= label_info->width / 2.0;
        label_info->y_offset_px += label_info->height / 2.0;
        break;

      case GLRA_CENTER_RIGHT:
        label_info->x_offset_px -= label_info->width;
        label_info->y_offset_px += label_info->height / 2.0;
        break;

      case GLRA_UPPER_LEFT:
        label_info->y_offset_px += label_info->height;
        break;

      case GLRA_UPPER_CENTER:
        label_info->x_offset_px -= label_info->width / 2.0;
        label_info->y_offset_px += label_info->height;
        break;

      case GLRA_UPPER_RIGHT:
        label_info->x_offset_px -= label_info->width;
        label_info->y_offset_px += label_info->height;
        break;

      case GLRA_LOWER_LEFT:
      default:
        break;
    }

    return part_index;
}

/************************************************************************/
/*                          gv_parse_symbol()                           */
/************************************************************************/

static guint
gv_parse_symbol( GvShapesLayer *layer, GvShape *shape, const char * sym_def,
                 int *scale_dep )

{
    const char  *id;
    char        *next, *value;
    int         value_len;
    guint       part_index = GVP_UNINITIALIZED_PART;
    GvSymbolRenderPart *symbol_info;

    if( !EQUALN(sym_def,"SYMBOL",6) )
    {
        g_warning( "not a symbol" );
        return part_index;
    }

    *scale_dep = TRUE;

/* -------------------------------------------------------------------- */
/*      Create the new part.                                            */
/* -------------------------------------------------------------------- */
    part_index = gv_shape_layer_create_part( GV_SHAPE_LAYER(layer),
                                             GvSymbolPart );

    symbol_info = (GvSymbolRenderPart *)
        gv_shape_layer_get_part( GV_SHAPE_LAYER(layer), part_index );

    g_assert( symbol_info != NULL );

/* -------------------------------------------------------------------- */
/*      Parse the tool arguments.                                       */
/* -------------------------------------------------------------------- */
    while( (id = gv_ogrfs_get_arg( sym_def, &next, &value,
                                 &value_len )) != NULL )
    {
        if( EQUALN(id,"c:",2) )
        {
            gv_get_ogr_color( value, symbol_info->color );
            symbol_info->b_color_initialized = 1;
        }
        else if( EQUALN(id,"id:",3) )
        {
            symbol_info->symbol_id = g_malloc(value_len+1);
            strncpy( symbol_info->symbol_id, value, value_len );
            symbol_info->symbol_id[value_len] = '\0';
        }
        else if( EQUALN(id,"a:",2) )
        {
            symbol_info->angle = atof(value);
        }
        else if( EQUALN(id,"s:",2) )
        {
            symbol_info->scale = atof(value);
            if( symbol_info->scale == 0.0 )
                symbol_info->scale = 1.0;
        }
        else if( EQUALN(id,"dx:",3) )
        {
            if( EQUALN(value+value_len-1,"g",1) )
                symbol_info->x_offset_g = atof(value);
            else
                symbol_info->x_offset_px = atof(value);
        }
        else if( EQUALN(id,"dy:",3) )
        {

            if( EQUALN(value+value_len-1,"g",1) )
                symbol_info->y_offset_g = atof(value);
            else
                symbol_info->y_offset_px = atof(value);
        }

        sym_def = next;
    }

    if( symbol_info->symbol_id == NULL )
    {
        symbol_info->symbol_id = g_strdup("ogr-sym-0");
    }

    return part_index;
}

/************************************************************************/
/*                            gv_parse_pen()                            */
/************************************************************************/

static guint
gv_parse_pen( GvShapesLayer *layer, GvShape *shape, const char * sym_def,
              int *scale_dep )

{
    const char  *id;
    char        *next, *value;
    int         value_len;
    guint       part_index = GVP_UNINITIALIZED_PART;
    GvPenRenderPart *pen_info;

    if( !EQUALN(sym_def,"PEN",3) )
    {
        g_warning( "not a pen" );
        return part_index;
    }

/* -------------------------------------------------------------------- */
/*      Create the new part.                                            */
/* -------------------------------------------------------------------- */
    part_index = gv_shape_layer_create_part( GV_SHAPE_LAYER(layer),
                                             GvPenPart );

    pen_info = (GvPenRenderPart *)
        gv_shape_layer_get_part( GV_SHAPE_LAYER(layer), part_index );

    g_assert( pen_info != NULL );

/* -------------------------------------------------------------------- */
/*      Parse the tool arguments.                                       */
/* -------------------------------------------------------------------- */
    while( (id = gv_ogrfs_get_arg( sym_def, &next, &value,
                                 &value_len )) != NULL )
    {
        if( EQUALN(id,"c:",2) )
        {
            gv_get_ogr_color( value, pen_info->color );
            pen_info->b_color_initialized = 1;
        }
        else if( EQUALN(id,"w:",2) )
        {
            pen_info->width = atof(value);
            //TODO: should validate using
            //glGetFloatv( GL_ALIAS_LINE_WIDTH_RANGE ) or
            //glGetFloatv( GL_SMOOTH_LINE_WIDTH_RANGE ) but
            //these constants aren't defined on Windows
            if (pen_info->width < 0.0)
                pen_info->width = 1.0;

        }
        else if (EQUALN(id, "p:", 2) )
        {
            //define a pattern for the pen
            pen_info->pattern = g_malloc(value_len+1);
            strncpy( pen_info->pattern, value, value_len );
            pen_info->pattern[value_len] = '\0';

        }
        else if (EQUALN(id, "id:", 3) && EQUALN(value,"ogr-pen-",8) )
        {
            //define a pattern for the pen
            pen_info->pattern = g_malloc(value_len+1);
            strncpy( pen_info->pattern, value, value_len );
            pen_info->pattern[value_len] = '\0';
        }

        sym_def = next;
    }

    if( pen_info->pattern == NULL )
    {
        pen_info->pattern = g_strdup("ogr-pen-0");
    }

    return part_index;
}

/************************************************************************/
/*                           gv_parse_brush()                           */
/************************************************************************/

static guint
gv_parse_brush( GvShapesLayer *layer, GvShape *shape, const char * sym_def,
                int *scale_dep )

{
    const char  *id;
    char        *next, *value;
    int         value_len;
    guint       part_index = GVP_UNINITIALIZED_PART;
    GvBrushRenderPart *brush_info;

    if( !EQUALN(sym_def,"BRUSH",5) )
    {
        g_warning( "not a brush" );
        return part_index;
    }

/* -------------------------------------------------------------------- */
/*      Create the new part.                                            */
/* -------------------------------------------------------------------- */
    part_index = gv_shape_layer_create_part( GV_SHAPE_LAYER(layer),
                                             GvBrushPart );

    brush_info = (GvBrushRenderPart *)
        gv_shape_layer_get_part( GV_SHAPE_LAYER(layer), part_index );

    g_assert( brush_info != NULL );

/* -------------------------------------------------------------------- */
/*      Parse the tool arguments.                                       */
/* -------------------------------------------------------------------- */
    while( (id = gv_ogrfs_get_arg( sym_def, &next, &value,
                                 &value_len )) != NULL )
    {
        if( EQUALN(id,"fc:",3) )
        {
            gv_get_ogr_color( value, brush_info->fore_color );
            brush_info->b_fore_color_initialized = 1;
        }

        sym_def = next;
    }

    return part_index;
}

/************************************************************************/
/*                        gv_ogrfs_split_tools()                        */
/************************************************************************/

char **gv_ogrfs_split_tools( const char *tool_list_in )

{
    int    i_src, word_start=0;
    char   **result = NULL, *tool_list;

    tool_list = g_strdup(tool_list_in);

    /* split on semi-colons that are not part of quoted strings */

    for( i_src = 0; tool_list[i_src] != '\0'; i_src++ )
    {
        if( tool_list[i_src] == '"' )
        {
            for( i_src++;
                 tool_list[i_src] != '\0' && tool_list[i_src] != '"';
                 i_src++ ) {}
        }
        else if( tool_list[i_src] == ';' )
        {
            tool_list[i_src] = '\0';

            result = CSLAddString( result, tool_list + word_start );
            word_start = i_src + 1;
        }
    }

    result = CSLAddString( result, tool_list + word_start );

    g_free( tool_list );

    return result;
}

/************************************************************************/
/*                  gv_shape_layer_build_renderinfo()                   */
/************************************************************************/

guint gv_shape_layer_build_renderinfo( GvShapeLayer *s_layer,
                                       GvShape *shape_obj,
                                       int *scale_dep )

{
    int     tool_index;
    guint   base_part_index = GVP_UNINITIALIZED_PART;
    GvShapesLayer *layer = GV_SHAPES_LAYER( s_layer );
    const char  *ogrfs;
    char    **tool_list;

/* -------------------------------------------------------------------- */
/*      Get the ogrfs string to apply.                                  */
/* -------------------------------------------------------------------- */
    ogrfs = gv_properties_get( gv_shape_get_properties(shape_obj),
                               "_gv_ogrfs" );

    if( ogrfs == NULL )
    {
        if( gv_shape_type(shape_obj) == GVSHAPE_POINT )
            ogrfs = gv_properties_get(&(GV_DATA(layer)->properties),
                                      "_gv_ogrfs_point");
        else if( gv_shape_type(shape_obj) == GVSHAPE_LINE )
            ogrfs = gv_properties_get(&(GV_DATA(layer)->properties),
                                      "_gv_ogrfs_line");
        else if( gv_shape_type(shape_obj) == GVSHAPE_AREA )
            ogrfs = gv_properties_get(&(GV_DATA(layer)->properties),
                                      "_gv_ogrfs_area");
    }

    if( ogrfs == NULL )
        ogrfs = gv_properties_get(&(GV_DATA(layer)->properties),"_gv_ogrfs");

    if( ogrfs == NULL )
        return GVP_LAST_PART;

/* -------------------------------------------------------------------- */
/*      Split tool actions apart.                                       */
/* -------------------------------------------------------------------- */
    tool_list = gv_ogrfs_split_tools( ogrfs );

    for( tool_index = 0; tool_index < CSLCount(tool_list); tool_index++ )
    {
        guint part_index = GVP_UNINITIALIZED_PART;

        if( EQUALN(tool_list[tool_index],"LABEL",5)
            && gv_shape_type(shape_obj) == GVSHAPE_POINT )
        {
            part_index =
                gv_parse_label( layer, shape_obj,
                                tool_list[tool_index], scale_dep );
        }
        else if( EQUALN(tool_list[tool_index],"SYMBOL",6)
            && gv_shape_type(shape_obj) == GVSHAPE_POINT )
        {
            part_index =
                gv_parse_symbol( layer, shape_obj,
                                 tool_list[tool_index], scale_dep );
        }
        else if( EQUALN(tool_list[tool_index],"PEN",3)
            && (gv_shape_type(shape_obj) == GVSHAPE_LINE
                || gv_shape_type(shape_obj) == GVSHAPE_AREA
                || gv_shape_type(shape_obj) == GVSHAPE_COLLECTION) )
        {
            part_index =
                gv_parse_pen( layer, shape_obj,
                              tool_list[tool_index], scale_dep );
        }
        else if( EQUALN(tool_list[tool_index],"BRUSH",5)
                 && (gv_shape_type(shape_obj) == GVSHAPE_AREA
                     || gv_shape_type(shape_obj) == GVSHAPE_COLLECTION) )
        {
            part_index =
                gv_parse_brush( layer, shape_obj,
                                tool_list[tool_index], scale_dep );
        }
        else
        {
            static int nReportedUnsupportedTools = 0;
            char message[512];
            const char *object = "unknown";

            if( gv_shape_type(shape_obj) == GVSHAPE_POINT )
                object = "point";
            else if( gv_shape_type(shape_obj) == GVSHAPE_LINE )
                object = "line";
            else if( gv_shape_type(shape_obj) == GVSHAPE_AREA )
                object = "area";

            sprintf( message, "Unsupported tool (%s) for %s object.",
                     tool_list[tool_index], object);

            if( nReportedUnsupportedTools++ < 15 )
                g_warning(message);
            else if( nReportedUnsupportedTools++ == 15 )
                g_warning("Unsupported tool - rest of warnings supressed.");

            if( nReportedUnsupportedTools < 100 )
                CPLDebug( "GVRENDERINFO", "%s", message );

        }

        if( part_index != GVP_UNINITIALIZED_PART )
            base_part_index =
                gv_shape_layer_chain_part( s_layer, base_part_index,
                                           part_index);
    }

    CSLDestroy( tool_list );

    return base_part_index;
}

/************************************************************************/
/*                 gv_shape_layer_update_renderinfo()                   */
/************************************************************************/

void gv_shape_layer_update_renderinfo( GvShapeLayer *s_layer, int shape_id )

{
    GvShapesLayer *layer = GV_SHAPES_LAYER( s_layer );
    GvShape     *shape_obj;
    int          scale_dep = FALSE;
    guint        base_part_index;

    base_part_index = gv_shape_layer_get_first_part_index( s_layer, shape_id );
    if( base_part_index != GVP_UNINITIALIZED_PART )
        return;

    if( !GV_IS_SHAPES_LAYER( s_layer ) )
        return;

    if( s_layer->render_index == NULL )
        gv_shape_layer_initialize_renderindex( s_layer );

    shape_obj = gv_shapes_get_shape(layer->data,shape_id);

    base_part_index = gv_shape_layer_build_renderinfo( s_layer, shape_obj,
                                                       &scale_dep );

    if( base_part_index == GVP_UNINITIALIZED_PART )
        base_part_index = GVP_LAST_PART;

    g_array_index(s_layer->render_index,guint,shape_id) = base_part_index;

    gv_shape_layer_set_scale_dep( s_layer, shape_id, scale_dep );
}

