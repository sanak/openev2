/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Display layer for vector shapes.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
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
 * $Log: gvshapeslayer.c,v $
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
 * Revision 1.73  2004/04/08 18:03:18  gmwalter
 * Fix for line node editing, and for area
 * editing in the case where the first
 * and last nodes are different but share
 * either the x or y coordinate.
 *
 * Revision 1.72  2004/03/22 19:21:15  gmwalter
 * Fixed initialization problem.
 *
 * Revision 1.71  2004/03/08 18:23:43  gmwalter
 * Make sure layer's _point_size property is used.
 *
 * Revision 1.70  2004/02/12 22:11:21  gmwalter
 * Update selected shape display in Atlantis
 * build case.
 *
 * Revision 1.69  2004/01/21 01:13:44  sduclos
 * fix typo
 *
 * Revision 1.68  2004/01/20 16:13:01  warmerda
 * added render plugin support for S52 viewer
 *
 * Revision 1.67  2003/09/12 17:35:42  warmerda
 * Added logic to aggregate selection boxes in the drawinfo.selection_box
 * rectangle when draw_mode == NORMAL_GET_BOX.  This is intended to allow
 * us to draw a selection box around a complex shapes consisting of multiple
 * parts offset from the reference point.  In the past we only drew the
 * selection around the first part in such cases.  This seems to work though
 * the testing isn't ... extensive.
 *
 * Revision 1.66  2003/09/02 17:22:26  warmerda
 * added per-layer symbol manager support
 *
 * Revision 1.65  2003/08/27 20:03:05  warmerda
 * Added geo2screen_works to drawinfo structure.  It is set to false for text
 * drawn within symbols since symbol rescaling via glScale() will mess up
 * the bmfont_draw() logic for ensuring text is drawn "on screen".  True
 * otherwise.  It is passed on to gv_view_area_bmfont_draw().
 *
 * Revision 1.64  2003/07/03 16:12:05  pgs
 * fixed bug in y offset of symbols (typo in var name)
 *
 * Revision 1.63  2003/05/16 21:31:26  warmerda
 * added support for passing default color down to components of a symbol
 *
 * Revision 1.62  2003/05/16 18:26:33  pgs
 * added initial code for propogating colors to sub-symbols
 *
 * Revision 1.61  2003/05/16 17:42:39  warmerda
 * fix up pixel offsets for sub-symbols
 *
 * Revision 1.60  2003/05/08 19:51:05  warmerda
 * Fixed node picking for gv_shapes_layer_draw_pen().
 * Draw selection boxes as squares instead of diamonds.
 */

#include "gvshapeslayer.h"
#include "gvutils.h"
#include "gvrenderinfo.h"
#include "gvsymbolmanager.h"
#include "gvmanager.h"
#include "cpl_error.h"
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmodule.h>

#define DEFAULT_POINT_SIZE 6
#define DEFAULT_LINE_WIDTH 1.0

#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif

#ifndef PI
#define PI  3.1415927
#endif

static void gv_shapes_layer_class_init(GvShapesLayerClass *klass);
static void gv_shapes_layer_init(GvShapesLayer *layer);
static void gv_shapes_layer_display_change(GvLayer *data,gpointer change_info);
static void gv_shapes_layer_selection_change(GvLayer *data,
                                             gpointer change_info);
static void gv_shapes_layer_extents(GvLayer *layer, GvRect *rect);
static void gv_shapes_layer_data_change(GvData *layer, gpointer change_info);
static void gv_shapes_layer_delete_selected(GvShapeLayer *layer);
static void gv_shapes_layer_translate_selected(GvShapeLayer *layer,
                                               GvVertex *delta);
static void gv_shapes_layer_pick_shape(GvShapeLayer *layer);
static void gv_shapes_layer_pick_node(GvShapeLayer *rlayer);
static void gv_shapes_layer_get_node(GvShapeLayer *layer, GvNodeInfo *info);
static void gv_shapes_layer_move_node(GvShapeLayer *layer, GvNodeInfo *info);
static void gv_shapes_layer_insert_node(GvShapeLayer *layer, GvNodeInfo *info);
static void gv_shapes_layer_delete_node(GvShapeLayer *layer, GvNodeInfo *info);
static void gv_shapes_layer_node_motion(GvShapeLayer *layer, gint area_id);
static void gv_shapes_layer_finalize(GObject *gobject );
static void gv_shapes_layer_dispose( GObject *gobject );

static GvShapeLayerClass *parent_class = NULL;

/************************************************************************/
/*                      gv_shapes_layer_get_type()                      */
/************************************************************************/
GType
gv_shapes_layer_get_type(void)
{
    static GType shapes_layer_type = 0;

    if (!shapes_layer_type) {
        static const GTypeInfo shapes_layer_info =
        {
            sizeof(GvShapesLayerClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_shapes_layer_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvShapesLayer),
            0,
            (GInstanceInitFunc) gv_shapes_layer_init,
        };
        shapes_layer_type = g_type_register_static (GV_TYPE_SHAPE_LAYER,
                                                    "GvShapesLayer",
                                                    &shapes_layer_info, 0);
    }

    return shapes_layer_type;
}

/************************************************************************/
/*                     gv_shapes_layer_class_init()                     */
/************************************************************************/
static void
gv_shapes_layer_class_init(GvShapesLayerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GvDataClass *data_class = GV_DATA_CLASS (klass);
    GvLayerClass *layer_class = GV_LAYER_CLASS (klass);
    GvShapeLayerClass *shape_layer_class = GV_SHAPE_LAYER_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    data_class->changed = gv_shapes_layer_data_change;

    layer_class->draw = gv_shapes_layer_draw;
    layer_class->extents_request = gv_shapes_layer_extents;

    shape_layer_class->draw_selected = gv_shapes_layer_draw_selected;
    shape_layer_class->delete_selected = gv_shapes_layer_delete_selected;
    shape_layer_class->translate_selected = gv_shapes_layer_translate_selected;
    shape_layer_class->pick_shape = gv_shapes_layer_pick_shape;
    shape_layer_class->pick_node = gv_shapes_layer_pick_node;
    shape_layer_class->get_node = gv_shapes_layer_get_node;
    shape_layer_class->move_node = gv_shapes_layer_move_node;
    shape_layer_class->insert_node = gv_shapes_layer_insert_node;
    shape_layer_class->delete_node = gv_shapes_layer_delete_node;
    shape_layer_class->node_motion = gv_shapes_layer_node_motion;

    klass->draw_shape = gv_shapes_layer_draw_shape;

    object_class->finalize = gv_shapes_layer_finalize;
}

/************************************************************************/
/*                        gv_shapes_layer_init()                        */
/************************************************************************/
static void
gv_shapes_layer_init(GvShapesLayer *layer)
{
    GvColor default_shapes_color = {0.5, 1.0, 0.5, 1.0};

    /* 0 is always an invalid display list number.  */
    layer->display_list = 0;

    layer->data = NULL;
    layer->symbol_manager = NULL;

    gv_color_copy(GV_SHAPE_LAYER(layer)->color, default_shapes_color);

    gv_properties_set( &(GV_DATA(layer)->properties), "_point_color",
                       "0.5 1.0 0.5 1.0" );
    gv_properties_set( &(GV_DATA(layer)->properties), "_line_color",
                       "0.5 1.0 0.5 1.0" );
    gv_properties_set( &(GV_DATA(layer)->properties), "_line_width",
                       "1.0" );
    gv_properties_set( &(GV_DATA(layer)->properties), "_area_edge_color",
                       "0.5 1.0 0.5 1.0" );
    gv_properties_set( &(GV_DATA(layer)->properties), "_area_edge_width",
                       "1.0" );
    gv_properties_set( &(GV_DATA(layer)->properties), "_area_fill_color",
                       "0.5 1.0 0.5 0.5" );

    g_signal_connect_swapped(layer, "display-change",
                  G_CALLBACK(gv_shapes_layer_display_change),
                  layer);
    g_signal_connect_swapped(layer, "selection-changed",
                  G_CALLBACK(gv_shapes_layer_selection_change),
                  layer);
}

/************************************************************************/
/*                         gv_shape_layer_new()                         */
/************************************************************************/
GObject *
gv_shapes_layer_new(void *data)
{
    GvShapesLayer *layer = g_object_new (GV_TYPE_SHAPES_LAYER, NULL);

    if(( data != NULL ) && (GV_IS_SHAPES(data))) {
        gv_shapes_layer_set_data(layer, data);
    }
    else {
        GvShapes *shapes = GV_SHAPES(gv_shapes_new());
        gv_shapes_layer_set_data(layer, shapes);
        g_object_unref(shapes);
    }

    return G_OBJECT(layer);
}

/************************************************************************/
/*                      gv_shape_layer_set_data()                       */
/************************************************************************/
void gv_shapes_layer_set_data( GvShapesLayer *layer,
                               GvShapes *data )

{
//~     if (layer->data)
//~         g_object_unref(layer->data);

    layer->data = data;
    /*GV_DATA(layer)->parent = GV_DATA(layer->data);*/
    gv_data_set_parent(GV_DATA(layer), GV_DATA(data));
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
                  gv_shapes_num_shapes(data));

#ifdef GV_USE_RENDER_PLUGIN
    {
        const char   *prop_name  = "_ogr_driver_name";
        GvProperties *properties = gv_data_get_properties(GV_DATA(data));
        const char   *prop_value = gv_properties_get(properties, prop_name);

        if (g_module_supported() && (NULL != prop_value)) {
            gchar* (*get_drv_name) ();
            gchar   *plugin = g_module_build_path("/usr/local/lib", "S52");
           GModule *module = g_module_open(plugin, G_MODULE_BIND_LAZY);

            if (NULL == module)
                printf("module error:%s\n", g_module_error());
            else {
                if (g_module_symbol(module, prop_name, (gpointer) &get_drv_name)) {
                    void (*layer_init) (GvShapesLayer *layer);
                    gchar *drv_name = get_drv_name();

                    if (0 == strcmp(prop_value, drv_name)) {
                        if (g_module_symbol(module, "_layer_init", 
                                            (gpointer) & layer_init))
                            layer_init(layer);
                        else
                            printf("module error:%s\n", g_module_error());
                    }
                } else
                    printf("module error:%s\n", g_module_error());
            }
           g_free(plugin);
        }
    }
#endif

}

/************************************************************************/
/*                 gv_shapes_layer_get_symbol_manager()                 */
/************************************************************************/

GObject *gv_shapes_layer_get_symbol_manager( GvShapesLayer *layer, 
                                               int ok_to_create )

{
    if( ok_to_create && layer->symbol_manager == NULL )
        layer->symbol_manager = G_OBJECT(gv_symbol_manager_new());

    return layer->symbol_manager;
}

/************************************************************************/
/*                  gv_shapes_layer_select_new_shape()                  */
/************************************************************************/

gint gv_shapes_layer_select_new_shape( GvShapesLayer *layer, GvShape * shape )

{
    gint id;

    id = gv_shapes_add_shape(layer->data, shape);

    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
                  gv_shapes_num_shapes(layer->data));
    gv_shape_layer_select_shape(GV_SHAPE_LAYER(layer), id);

    return id;
}

/************************************************************************/
/*                   gv_shapes_layer_override_color()                   */
/************************************************************************/

void
gv_shapes_layer_override_color( GvShape * shape, GvColor color,
                                const char * property_name )

{
    GvProperties *properties = gv_shape_get_properties(shape);

    if( properties != NULL )
    {
        const char * user_color;

        user_color = gv_properties_get( properties, property_name);

        if( user_color != NULL )
        {
            gv_set_color_from_string( color, user_color,
                                      color[0], color[1],
                                      color[2], color[3] );
        }
    }
}

/************************************************************************/
/*                   gv_shapes_layer_get_draw_info()                    */
/************************************************************************/

void
gv_shapes_layer_get_draw_info(GvViewArea *view, GvShapesLayer *layer,
                              GvShapeDrawInfo *drawinfo )

{
    GvColor  def_color;

    gv_color_copy(def_color, GV_SHAPE_LAYER(layer)->color);
    gv_color_copy(drawinfo->color, def_color );

    gv_set_color_from_string(
        drawinfo->point_color,
        gv_properties_get( &(GV_DATA(layer)->properties), "_point_color"),
        def_color[0], def_color[1], def_color[2], def_color[3]);

    gv_set_color_from_string(
        drawinfo->line_color,
        gv_properties_get( &(GV_DATA(layer)->properties), "_line_color"),
        def_color[0], def_color[1], def_color[2], def_color[3]);

    if( gv_properties_get(&(GV_DATA(layer)->properties),"_line_width") != NULL)
        drawinfo->line_width =
          atof(gv_properties_get(&(GV_DATA(layer)->properties),"_line_width"));
    else
        drawinfo->line_width = DEFAULT_LINE_WIDTH;

    gv_set_color_from_string(
        drawinfo->area_edge_color,
        gv_properties_get( &(GV_DATA(layer)->properties), "_area_edge_color"),
        def_color[0], def_color[1], def_color[2], def_color[3]);

    if( gv_properties_get(&(GV_DATA(layer)->properties),"_area_edge_width") != NULL)
        drawinfo->area_edge_width =
          atof(gv_properties_get(&(GV_DATA(layer)->properties),"_area_edge_width"));
    else
        drawinfo->area_edge_width = DEFAULT_LINE_WIDTH;

    gv_set_color_from_string(
        drawinfo->area_fill_color,
        gv_properties_get( &(GV_DATA(layer)->properties), "_area_fill_color"),
        def_color[0], def_color[1], def_color[2], 0.6);

    if( gv_properties_get(&(GV_DATA(layer)->properties),"_point_size") != NULL)
        drawinfo->point_size =
          atof(gv_properties_get(&(GV_DATA(layer)->properties),"_point_size"));
    else
        drawinfo->point_size = DEFAULT_POINT_SIZE;

    /*
     * Transform a 1 pixel "right" vector into geo-space.  We can compose
     * a variety of deltas from this for selection boxes, crosshair sizes
     * and so forth.
     */
    drawinfo->dx = 1.0;
    drawinfo->dy = 0.0;
    gv_view_area_correct_for_transform(view, drawinfo->dx, drawinfo->dy,
                                       &(drawinfo->dx), &(drawinfo->dy) );
    drawinfo->dunit =
        sqrt( drawinfo->dx * drawinfo->dx + drawinfo->dy * drawinfo->dy );
    drawinfo->dpixel = drawinfo->dunit;
    drawinfo->geo2screen_works = TRUE;
    drawinfo->pango_layout = NULL;
    drawinfo->box_set = FALSE;
}

/************************************************************************/
/*                gv_draw_info_aggregate_select_region()                */
/*                                                                      */
/*      Add the passed point to the current selection_box stored in     */
/*      the drawinfo.                                                   */
/************************************************************************/

void gv_draw_info_aggregate_select_region( GvShapeDrawInfo *drawinfo,
                                           double x, double y )

{
    if( !drawinfo->box_set )
    {
        drawinfo->box_set = TRUE;
        drawinfo->selection_box.x = x;
        drawinfo->selection_box.y = y;
        drawinfo->selection_box.width = 0;
        drawinfo->selection_box.height = 0;
    }
    else
    {
        if( x < drawinfo->selection_box.x )
        {
            drawinfo->selection_box.width += drawinfo->selection_box.x - x;
            drawinfo->selection_box.x = x;
        }
        else if( x > drawinfo->selection_box.x+drawinfo->selection_box.width )
        {
            drawinfo->selection_box.width = x - drawinfo->selection_box.x;
        }

        if( y < drawinfo->selection_box.y )
        {
            drawinfo->selection_box.height += drawinfo->selection_box.y - y;
            drawinfo->selection_box.y = y;
        }
        else if( y > drawinfo->selection_box.y+drawinfo->selection_box.height )
        {
            drawinfo->selection_box.height = y - drawinfo->selection_box.y;
        }
    }
}

/************************************************************************/
/*                       gv_draw_info_grow_box()                        */
/*                                                                      */
/*      Expand the "selection box" by the indicated factor around       */
/*      its center.  This is used before drawing it to move it out a    */
/*      bit from the object it encloses.  Normally the growth factor    */
/*      would be 1.2.                                                   */
/************************************************************************/

static void gv_draw_info_grow_box( GvShapeDrawInfo *drawinfo, double factor )

{
    double center_x, center_y;

    center_x = drawinfo->selection_box.x + drawinfo->selection_box.width * 0.5;
    center_y = drawinfo->selection_box.y + drawinfo->selection_box.height *0.5;
    
    drawinfo->selection_box.width *= factor;
    drawinfo->selection_box.height *= factor;

    /* Don't let the width or height of a selection box be too much smaller
       than the other dimension. */
    
    if( drawinfo->selection_box.width < drawinfo->selection_box.height/4.0 )
        drawinfo->selection_box.width = drawinfo->selection_box.height/4.0;

    if( drawinfo->selection_box.height < drawinfo->selection_box.width/4.0 )
        drawinfo->selection_box.height = drawinfo->selection_box.width/4.0;

    drawinfo->selection_box.x = center_x - drawinfo->selection_box.width * 0.5;
    drawinfo->selection_box.y = center_y - drawinfo->selection_box.height *0.5;
}

/************************************************************************/
/*                     gv_shapes_layer_draw_label()                     */
/************************************************************************/

static void
gv_shapes_layer_draw_label( GvViewArea *view, GvShapesLayer *layer,
                            GvShape *shape, GvShapeDrawInfo *drawinfo,
                            gvgeocoord x, gvgeocoord y,
                            GvLabelRenderPart *label_info,
                            gv_draw_mode draw_mode )

{
    gvgeocoord x_offset; /* for shadow and halo effects */
    gvgeocoord y_offset;
/* -------------------------------------------------------------------- */
/*      Apply any offsets.                                              */
/* -------------------------------------------------------------------- */
    x += label_info->x_offset_g;
    y += label_info->y_offset_g;

    if( label_info->x_offset_px != 0.0 || label_info->y_offset_px != 0.0 )
    {
        x += label_info->x_offset_px * drawinfo->dpixel;
        y -= label_info->y_offset_px * drawinfo->dpixel;
    }

    /* Why does this get done when PICKING?  Also, why does drawing text
       comment below, regarding frustrum, not apply to effects?
    */

    /* draw text effects first */
    if (label_info->halo || label_info->shadow)
    {
        x_offset = drawinfo->dpixel;
        y_offset = drawinfo->dpixel;

        if (label_info->halo)
        {
            glColor4fv(label_info->background_color);
            gv_view_area_bmfont_draw( view, label_info->font,
                                      x - x_offset, y - y_offset,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
            gv_view_area_bmfont_draw( view, label_info->font,
                                      x - x_offset, y ,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
            gv_view_area_bmfont_draw( view, label_info->font,
                                      x - x_offset, y + y_offset,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
            gv_view_area_bmfont_draw( view, label_info->font,
                                      x, y + y_offset,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
            gv_view_area_bmfont_draw( view, label_info->font,
                                      x + x_offset, y + y_offset,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
            gv_view_area_bmfont_draw( view, label_info->font,
                                      x + x_offset, y,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
            gv_view_area_bmfont_draw( view, label_info->font,
                                      x + x_offset, y - y_offset,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
            gv_view_area_bmfont_draw( view, label_info->font,
                                      x, y - y_offset,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
        }

        if (label_info->shadow)
        {
            glColor4fv(label_info->background_color);
            gv_view_area_bmfont_draw( view, label_info->font,
                                      x + x_offset, y + y_offset,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
        }
    }

    if (label_info->b_color_initialized)
        glColor4fv( label_info->color );
    else
        glColor4fv( drawinfo->color );

/* -------------------------------------------------------------------- */
/*      Draw the text itself.                                           */
/* -------------------------------------------------------------------- */
    if( draw_mode != PICKING )
    {
        /* When dragging text, there is an extra translation applied
           which gv_view_area_bmfont_draw() isn't able to account for
           while trying to keep the RasterPos in the frustrum.  Restore
           the original translation value, and offset the x,y passed to
           the bmfont drawer. */

        if( draw_mode == SELECTED
            && (GV_SHAPE_LAYER(layer)->selected_motion.x != 0
                || GV_SHAPE_LAYER(layer)->selected_motion.y != 0) )
        {
            glTranslate(-GV_SHAPE_LAYER(layer)->selected_motion.x,
                         -GV_SHAPE_LAYER(layer)->selected_motion.y,
                         0.0 );
            gv_view_area_bmfont_draw(
                view, label_info->font,
                x + GV_SHAPE_LAYER(layer)->selected_motion.x,
                y + GV_SHAPE_LAYER(layer)->selected_motion.y,
                label_info->text, 
                !drawinfo->geo2screen_works );
            glTranslate(GV_SHAPE_LAYER(layer)->selected_motion.x,
                         GV_SHAPE_LAYER(layer)->selected_motion.y,
                         0.0 );
        }
        else
        {
            gv_view_area_bmfont_draw( view, label_info->font, x, y,
                                      label_info->text, 
                                      !drawinfo->geo2screen_works );
        }
    }

/* -------------------------------------------------------------------- */
/*      Draw the selection box, or a filled polygon depending on mode.  */
/* -------------------------------------------------------------------- */
    if( draw_mode != NORMAL )
    {
        gvgeocoord  ll_px, ll_py, ur_px, ur_py, ur_geox, ur_geoy;
        gvgeocoord      ul_geox, ul_geoy, lr_geox, lr_geoy;

        gv_view_area_inverse_map_pointer( view, x, y, &ll_px, &ll_py );
        ll_px -= 2;
        ll_py += 2; // + label_info->descent;
        ur_px = ll_px + label_info->width + 4;
        ur_py = ll_py - label_info->height - 4;
        gv_view_area_map_pointer( view, ur_px, ur_py, &ur_geox, &ur_geoy );
        gv_view_area_map_pointer( view, ll_px, ll_py, &x, &y );
        gv_view_area_map_pointer( view, ll_px, ur_py, &ul_geox, &ul_geoy );
        gv_view_area_map_pointer( view, ur_px, ll_py, &lr_geox, &lr_geoy );

        if( draw_mode == NORMAL_GET_BOX )
        {
            gv_draw_info_aggregate_select_region( drawinfo, x, y );
            gv_draw_info_aggregate_select_region( drawinfo, ur_geox, ur_geoy );
        }
        else if( draw_mode == SELECTED )
        {
            /* Draw box around text */
            glBegin(GL_LINE_LOOP);
            glVertex3(x, y, 0.0);
            glVertex3(ul_geox, ul_geoy, 0.0);
            glVertex3(ur_geox, ur_geoy, 0.0);
            glVertex3(lr_geox, lr_geoy, 0.0);
            glEnd();
        }
        else /* picking ... draw filled box */
        {
            glBegin(GL_POLYGON);
            glVertex3(x, y, 0.0);
            glVertex3(ul_geox, ul_geoy, 0.0);
            glVertex3(ur_geox, ur_geoy, 0.0);
            glVertex3(lr_geox, lr_geoy, 0.0);
            glVertex3(x, y, 0.0);
            glEnd();

        }
    }
}

/************************************************************************/
/*                    gv_shapes_layer_draw_symbol()                     */
/************************************************************************/

static void
gv_shapes_layer_draw_symbol( GvViewArea *view, GvShapesLayer *layer,
                             GvShape *shape, GvShapeDrawInfo *drawinfo,
                             gvgeocoord x, gvgeocoord y, gvgeocoord z,
                             GvSymbolRenderPart *symbol_info,
                             int part_index, gv_draw_mode draw_mode )

{
    guint sym_id = -1;
    GvSymbolObj *poSymbol = NULL;

    if( EQUALN(symbol_info->symbol_id, "ogr-sym-", 8) )
    {
        sym_id = atoi(symbol_info->symbol_id + 8);
    }
    else
    {
        if( layer->symbol_manager != NULL 
            && gv_symbol_manager_has_symbol( 
                GV_SYMBOL_MANAGER(layer->symbol_manager), 
                symbol_info->symbol_id ) )
            poSymbol = gv_symbol_manager_get_symbol( 
                GV_SYMBOL_MANAGER(layer->symbol_manager),
                symbol_info->symbol_id );
        else
            poSymbol = gv_symbol_manager_get_symbol( gv_get_symbol_manager(),
                                                     symbol_info->symbol_id );

        if (poSymbol == NULL)
            CPLDebug( "OpenEV", "poSymbol is NULL !!! " );
    }

/* -------------------------------------------------------------------- */
/*      Render raster symbol.                                           */
/* -------------------------------------------------------------------- */
    if( poSymbol != NULL && poSymbol->type == GV_SYMBOL_RASTER )
    {
        gvgeocoord bx, by;

        /* Compute the lower left corner of the raster assuming it is to be
           centered on the point before offsets */
        bx = x + symbol_info->x_offset_g;
        by = y + symbol_info->y_offset_g;

        bx = bx + (symbol_info->x_offset_px - poSymbol->width/2)
            * drawinfo->dpixel;
        by = by - (symbol_info->y_offset_px - poSymbol->height/2)
            * drawinfo->dpixel;

        /* Draw an outline slightly larger than the raster to indicate
           selection.  Draw a filled polygon the same size as the raster
           when picking */

        if( draw_mode != NORMAL )
        {
            gvgeocoord geo_width, geo_height, mult_fac = 1.0, box_x, box_y;

            if( draw_mode == SELECTED )
                mult_fac = 1.2;

            geo_width = poSymbol->width * mult_fac * drawinfo->dpixel;
            geo_height = poSymbol->height * mult_fac * drawinfo->dpixel;

            box_x = x + symbol_info->x_offset_g - geo_width/2.0
                + symbol_info->x_offset_px * drawinfo->dpixel;
            box_y = y + symbol_info->y_offset_g - geo_height/2.0
                - symbol_info->y_offset_px * drawinfo->dpixel;

            if( draw_mode == NORMAL_GET_BOX )
            {
                gv_draw_info_aggregate_select_region( drawinfo, box_x, box_y);
                gv_draw_info_aggregate_select_region( drawinfo, 
                                                      box_x + geo_width,
                                                      box_y + geo_height );
            }
            else
            {
                /* Draw box around symbol */
                glBegin( draw_mode == SELECTED ? GL_LINE_LOOP : GL_POLYGON );
                glVertex3(box_x,box_y,0.0);
                glVertex3(box_x+geo_width,box_y,0.0);
                glVertex3(box_x+geo_width,box_y+geo_height,0.0);
                glVertex3(box_x,box_y+geo_height,0.0);
                glVertex3(box_x,box_y,0.0);
                glEnd();
            }
        }

        /* Draw the raster (as long as we aren't picking) */
        if( draw_mode != PICKING )
        {
            glRasterPos3( bx, by, z );
            glPixelZoom(1.0,-1.0);
            glDrawPixels( poSymbol->width, poSymbol->height, GL_RGBA,
                          GL_UNSIGNED_INT_8_8_8_8_REV, poSymbol->buffer );
        }

    }

/* -------------------------------------------------------------------- */
/*      Render vector symbols.                                          */
/* -------------------------------------------------------------------- */
    else if( sym_id != -1
             || (poSymbol != NULL && poSymbol->type == GV_SYMBOL_VECTOR) )
    {
        gvgeocoord cx, cy;
        gvgeocoord base_vector = drawinfo->dunit * drawinfo->point_size;

        if (symbol_info->b_color_initialized)
            glColor4fv( symbol_info->color );
        else
            glColor4fv( drawinfo->color );

        /* Perform any required pixel or georeferenced translation of the
           center point */

        cx = x + symbol_info->x_offset_g;
        cy = y + symbol_info->y_offset_g;

        if(symbol_info->x_offset_px != 0.0 || symbol_info->y_offset_px != 0.0)
        {
            cx = cx + symbol_info->x_offset_px * drawinfo->dpixel;
            cy = cy - symbol_info->y_offset_px * drawinfo->dpixel;
        }

        glTranslate( cx, cy, z );

        if( symbol_info->scale != 1.0 )
            glScale( symbol_info->scale,
                     symbol_info->scale,
                     symbol_info->scale );

        if( symbol_info->angle != 0.0 )
            glRotate( symbol_info->angle, 0.0, 0.0, 1.0 );

        switch( sym_id )
        {
          case -1: /* general vector symbol */
          {
              GvShape *shape_obj = (GvShape *) poSymbol->buffer;
              gvgeocoord base_scale;
              GvShapeDrawInfo sub_drawinfo;

              /* Initialize renderinfo */
              if( symbol_info->part_index == GVP_UNINITIALIZED_PART )
              {
                  int scale_dep = FALSE;
                  int sub_part_index;

                  sub_part_index =
                      gv_shape_layer_build_renderinfo( GV_SHAPE_LAYER(layer),
                                                       shape_obj,
                                                       &scale_dep );

                  /* symbol_info may have moved, so re-fetch based on index */
                  symbol_info = (GvSymbolRenderPart *)
                      gv_shape_layer_get_part( GV_SHAPE_LAYER(layer),
                                               part_index );
                  symbol_info->part_index = sub_part_index;
                  if( symbol_info->part_index == GVP_UNINITIALIZED_PART )
                      symbol_info->part_index = GVP_LAST_PART;
              }

              memcpy( &sub_drawinfo, drawinfo, sizeof(GvShapeDrawInfo) );
              sub_drawinfo.dx = 1.0;
              sub_drawinfo.dy = 0.0;
              sub_drawinfo.dunit = 1.0;
              sub_drawinfo.dpixel *= 1/(drawinfo->dunit * symbol_info->scale);
              sub_drawinfo.geo2screen_works = FALSE;
              sub_drawinfo.box_set = FALSE;

              if (symbol_info->b_color_initialized)
                  gv_color_copy( sub_drawinfo.color, symbol_info->color );

              base_scale = drawinfo->dunit;

              glScale( base_scale, base_scale, base_scale );

              GV_SHAPES_LAYER_GET_CLASS(layer)->draw_shape( view, layer, 
                                symbol_info->part_index,
                                shape_obj,
                                draw_mode == SELECTED ? NORMAL_GET_BOX : draw_mode,
                                &sub_drawinfo );

              if( draw_mode == SELECTED && sub_drawinfo.box_set )
              {
                  draw_mode = NORMAL; /* disable later selection drawing */

                  gv_draw_info_grow_box( &sub_drawinfo, 1.2 );
                  
                  glBegin(GL_LINE_LOOP);
                  glVertex2( 
                      sub_drawinfo.selection_box.x, 
                      sub_drawinfo.selection_box.y );
                  glVertex2( 
                      sub_drawinfo.selection_box.x + sub_drawinfo.selection_box.width, 
                      sub_drawinfo.selection_box.y );
                  glVertex2( 
                      sub_drawinfo.selection_box.x + sub_drawinfo.selection_box.width, 
                      sub_drawinfo.selection_box.y + sub_drawinfo.selection_box.height);
                  glVertex2( 
                      sub_drawinfo.selection_box.x, 
                      sub_drawinfo.selection_box.y + sub_drawinfo.selection_box.height);
                  glEnd();
              }
              else if( draw_mode == NORMAL_GET_BOX )
              {
                  /* We should transform the box in sub_drawinfo and move
                     it into drawinto ... but thats hard so I am deferring
                     it till needed. */
              }
              glScale( 1.0/base_scale, 1.0/base_scale, 1.0/base_scale );
          }
          break;

          case 0: /* cross */
            glBegin(GL_LINES);
            glVertex3(-base_vector, 0,0.0);
            glVertex3( base_vector, 0,0.0);
            glVertex3( 0, -base_vector,0.0);
            glVertex3( 0, base_vector,0.0);
            glEnd();
            break;

          case 1: /* X */
            glBegin(GL_LINES);
            glVertex3(-base_vector, -base_vector,0.0);
            glVertex3( base_vector, base_vector,0.0);
            glVertex3( -base_vector, base_vector,0.0);
            glVertex3( base_vector, -base_vector,0.0);
            glEnd();
            break;

          case 2: /* unfilled circle */
            glBegin(GL_LINE_LOOP);
            glVertex3(base_vector*0.0,base_vector*1.0,0.0);
            glVertex3(base_vector*0.342020148171,
                       base_vector*0.939692619022,0.0);
            glVertex3(base_vector*0.642787617587,
                       base_vector*0.76604443649,0.0);
            glVertex3(base_vector*0.866025411519,
                       base_vector*0.499999986603,0.0);
            glVertex3(base_vector*0.984807756594,
                       base_vector*0.173648157354,0.0);
            glVertex3(base_vector*0.984807748535,
                       base_vector*-0.173648203059,0.0);
            glVertex3(base_vector*0.866025388314,
                       base_vector*-0.500000026795,0.0);
            glVertex3(base_vector*0.642787582035,
                       base_vector*-0.766044466322,0.0);
            glVertex3(base_vector*0.34202010456,
                       base_vector*-0.939692634895,0.0);
            glVertex3(base_vector*0,
                       base_vector*-1.0,0.0);
            glVertex3(base_vector*-0.342020191783,
                       base_vector*-0.93969260314,0.0);
            glVertex3(base_vector*-0.642787653139,
                       base_vector*-0.76604440665,0.0);
            glVertex3(base_vector*-0.866025434725,
                       base_vector*-0.49999994641,0.0);
            glVertex3(base_vector*-0.984807764653,
                       base_vector*-0.17364811164,0.0);
            glVertex3(base_vector*-0.984807740476,
                       base_vector*0.173648248764,0.0);
            glVertex3(base_vector*-0.866025365109,
                       base_vector*0.500000066987,0.0);
            glVertex3(base_vector*-0.642787546482,
                       base_vector*0.766044496153,0.0);
            glVertex3(base_vector*-0.342020060949,
                       base_vector*0.939692650769,0.0);
            glVertex3(base_vector*9.28204133326e-08,
                       base_vector*1.0,0.0);
            glEnd();
            break;

          case 3: /* filled circle */
            glBegin(GL_POLYGON);
            glVertex3(base_vector*0.0,base_vector*1.0,0.0);
            glVertex3(base_vector*0.342020148171,
                       base_vector*0.939692619022,0.0);
            glVertex3(base_vector*0.642787617587,
                       base_vector*0.76604443649,0.0);
            glVertex3(base_vector*0.866025411519,
                       base_vector*0.499999986603,0.0);
            glVertex3(base_vector*0.984807756594,
                       base_vector*0.173648157354,0.0);
            glVertex3(base_vector*0.984807748535,
                       base_vector*-0.173648203059,0.0);
            glVertex3(base_vector*0.866025388314,
                       base_vector*-0.500000026795,0.0);
            glVertex3(base_vector*0.642787582035,
                       base_vector*-0.766044466322,0.0);
            glVertex3(base_vector*0.34202010456,
                       base_vector*-0.939692634895,0.0);
            glVertex3(base_vector*0,base_vector*-1.0,0.0);
            glVertex3(base_vector*-0.342020191783,
                       base_vector*-0.93969260314,0.0);
            glVertex3(base_vector*-0.642787653139,
                       base_vector*-0.76604440665,0.0);
            glVertex3(base_vector*-0.866025434725,
                       base_vector*-0.49999994641,0.0);
            glVertex3(base_vector*-0.984807764653,
                       base_vector*-0.17364811164,0.0);
            glVertex3(base_vector*-0.984807740476,
                       base_vector*0.173648248764,0.0);
            glVertex3(base_vector*-0.866025365109,
                       base_vector*0.500000066987,0.0);
            glVertex3(base_vector*-0.642787546482,
                       base_vector*0.766044496153,0.0);
            glVertex3(base_vector*-0.342020060949,
                       base_vector*0.939692650769,0.0);
            glVertex3(base_vector*9.28204133326e-08,base_vector*1.0,0.0);
            glEnd();
            break;

          case 4: /* square */
            glBegin( GL_LINE_LOOP );
            glVertex3( -base_vector, -base_vector ,0.0);
            glVertex3( -base_vector, base_vector ,0.0);
            glVertex3( base_vector, base_vector ,0.0);
            glVertex3( base_vector, -base_vector ,0.0);
            glVertex3( -base_vector, -base_vector ,0.0);
            glEnd();
            break;

          case 5: /* filled square */
            glBegin( GL_POLYGON );
            glVertex3( -base_vector, -base_vector ,0.0);
            glVertex3( -base_vector, base_vector ,0.0);
            glVertex3( base_vector, base_vector ,0.0);
            glVertex3( base_vector, -base_vector ,0.0);
            glVertex3( -base_vector, -base_vector ,0.0);
            glEnd();
            break;

          case 6: /* triangle */
            glBegin(GL_LINE_LOOP);
            glVertex3( 0, base_vector ,0.0);
            glVertex3( -base_vector*0.866, -base_vector*0.5,0.0);
            glVertex3(  base_vector*0.866, -base_vector*0.5,0.0);
            glVertex3( 0, base_vector ,0.0);
            glEnd();
            break;

          case 7: /* filled triangle */
            glBegin(GL_POLYGON);
            glVertex3( 0, base_vector ,0.0);
            glVertex3(  base_vector*0.866, -base_vector*0.5,0.0);
            glVertex3( -base_vector*0.866, -base_vector*0.5,0.0);
            glVertex3( 0, base_vector ,0.0);
            glEnd();
            break;

          case 8: /* star */
            glBegin( GL_LINE_LOOP );
            glVertex3(0.0,base_vector*1.0,0.0);
            glVertex3(base_vector*0.587785222255,
                       base_vector*-0.809017016198,0.0);
            glVertex3(base_vector*-0.951056493349,
                       base_vector*0.309017064997,0.0);
            glVertex3(base_vector*0.951056522032,
                       base_vector*0.309016976719,0.0);
            glVertex3(base_vector*-0.587785297348,
                       base_vector*-0.80901696164,0.0);
            glVertex3(0.0,base_vector*1.0,0.0);
            glEnd();
            break;

          case 9: /* filled star */
            glBegin( GL_POLYGON );
            glVertex3(base_vector * -0.195928, base_vector *  0.269672,0.0);
            glVertex3(base_vector *  0.000000, base_vector *  1.000000,0.0);
            glVertex3(base_vector *  0.195928, base_vector *  0.269672,0.0);
            glEnd();
            glBegin( GL_POLYGON );
            glVertex3(base_vector *  0.195928, base_vector *  0.269672,0.0);
            glVertex3(base_vector *  0.951057, base_vector *  0.309017,0.0);
            glVertex3(base_vector *  0.317019, base_vector * -0.103006,0.0);
            glEnd();
            glBegin( GL_POLYGON );
            glVertex3(base_vector *  0.317019, base_vector * -0.103006,0.0);
            glVertex3(base_vector *  0.587785, base_vector * -0.809017,0.0);
            glVertex3(base_vector *  0.000000, base_vector * -0.333333,0.0);
            glEnd();
            glBegin( GL_POLYGON );
            glVertex3(base_vector *  0.000000, base_vector * -0.333333,0.0);
            glVertex3(base_vector * -0.587785, base_vector * -0.809017,0.0);
            glVertex3(base_vector * -0.317019, base_vector * -0.103006,0.0);
            glEnd();
            glBegin( GL_POLYGON );
            glVertex3(base_vector * -0.317019, base_vector * -0.103006,0.0);
            glVertex3(base_vector * -0.951057, base_vector *  0.309017,0.0);
            glVertex3(base_vector * -0.195928, base_vector *  0.269672,0.0);
            glEnd();
            glBegin( GL_POLYGON );
            glVertex3(base_vector *  0.195928, base_vector *  0.269672,0.0);
            glVertex3(base_vector *  0.317019, base_vector * -0.103006,0.0);
            glVertex3(base_vector *  0.000000, base_vector * -0.333333,0.0);
            glVertex3(base_vector * -0.317019, base_vector * -0.103006,0.0);
            glVertex3(base_vector * -0.195928, base_vector *  0.269672,0.0);
            glEnd();

          case 10: /* vertical bar */
            glBegin(GL_LINES);
            glVertex3( 0, 0 ,0.0);
            glVertex3( 0, base_vector ,0.0);
            glEnd();
            break;
        }

        if( symbol_info->angle != 0.0 )
            glRotate( -symbol_info->angle,0.0,0.0, 1.0 );

        if( draw_mode == SELECTED )
        {
            float   bx = base_vector * 1.2;
            
            /* Draw box around crosshairs */
            glBegin(GL_LINE_LOOP);
            glVertex3(-bx, -bx,0.0);
            glVertex3( bx, -bx,0.0);
            glVertex3( bx,  bx,0.0);
            glVertex3(-bx,  bx,0.0);
            glEnd();
        }
        else if( draw_mode == NORMAL_GET_BOX )
        {
            float   bx = base_vector;

            /* We do not currently take symbol rotation angle into account. */

            gv_draw_info_aggregate_select_region( drawinfo, 
                                                  cx - bx*symbol_info->scale,
                                                  cy - bx*symbol_info->scale );
            gv_draw_info_aggregate_select_region( drawinfo, 
                                                  cx + bx*symbol_info->scale,
                                                  cy + bx*symbol_info->scale );
        }

        if( symbol_info->scale != 1.0 )
        {
            float one_over_scale;

            one_over_scale = 1/symbol_info->scale;
            glScale( one_over_scale, one_over_scale, one_over_scale );
        }

        glTranslate( -cx, -cy, -z );
    }
    else
    {
        CPLDebug( "OpenEV", "symbol not understood" );
    }
}

/************************************************************************/
/*                      gv_shapes_layer_draw_pen()                      */
/************************************************************************/

static void
gv_shapes_layer_draw_pen( GvViewArea *view, GvShape *shape,
                          GvShapeDrawInfo *drawinfo,
                          GvPenRenderPart *pen_info,
                          gv_draw_mode draw_mode )

{
    double box_vector = drawinfo->dunit*(DEFAULT_POINT_SIZE+pen_info->width/2-1);
    guint pen_id = -1;

    GLint factor = 1;
    GLushort stipple = 0xFFFF;

    if (pen_info->b_color_initialized)
        glColor4fv( pen_info->color );
    else
        glColor4fv( drawinfo->color );

    glLineWidth( pen_info->width );

    if( draw_mode != PICKING
        && EQUALN(pen_info->pattern, "ogr-pen-", 8) )
    {
        pen_id = atoi(pen_info->pattern + 8);
    }

    /*
    A note about pen stippling.  According to the OpenGL spec,
    the stipple pattern is a 16 bit number used as a mask.  Any
    bit that is set is visible in the output, any bit not set
    ends up not visible.  Factor is a linear multiplication of
    the stipple pattern so a factor of 2 gives 32 bits but each
    bit controls two pixels instead of 1.  Another important
    piece of information: stippling is used from the lowest bit
    to the highest bit (right to left as you look at the number).

    0xFFFF is all bits on
    0x0000 is all bits off
    0x0F0F is a dash pattern with four on/four off

    All patterns must have a repeat of 16.
    */

    if (pen_id != -1)
    {
        switch( pen_id )
        {
            case 0: /* default pen, don't use stippling */
                pen_id = -1;
                break;
            case 1: /* invisible pen */
                factor = 1;
                stipple = 0x0000;
                break;
            case 2: /* dash */
                factor = 1;
                stipple = 0x0F0F;
                break;
            case 3: /* short dash */
                factor = 1;
                stipple = 0x0F0F;
                break;
            case 4: /* long dash */
                factor = 3;
                stipple = 0x0F0F;
                break;
            case 5: /* dot */
                factor = 1;
                stipple = 0x3333;
                break;
            case 6: /* dash-dot */
                factor = 2;
                stipple = 0x2727;
                break;
            case 7: /* dash-dot-dot */
                factor = 2;
                stipple = 0x333F;
                break;
            case 8: /*alternate pixels */
                factor = 1;
                stipple = 0x5555;
                break;
            default: /* default to pen 0 which is no stipple */
                pen_id = -1;
        }
    }

    if (pen_id != -1)
    {
        // On some ATI OpenGL drivers (eg Xpert2000) we need
        // to set line width to 1 to get stipples working.
        // glLineWidth( 1.0 );

        glEnable( GL_LINE_STIPPLE );
        glLineStipple( factor, stipple );
    }

    if( gv_shape_type(shape) == GVSHAPE_LINE )
    {
        GvLineShape   *line = (GvLineShape *) shape;
        int j;

        glVertexPointer( 3, GL_GEOCOORD, 0, line->xyz_nodes );
        glDrawArrays( GL_LINE_STRIP, 0, line->num_nodes );

        //revert line width for drawing the boxes
        glLineWidth( 1.0 );
        glLineStipple( 1, 0xffff );

        /* Draw small box around each point, in picking mode we fill it. */
        if( draw_mode == PICKING || draw_mode == SELECTED )
        {
            for (j=0; j < line->num_nodes; ++j)
            {
                gvgeocoord x, y;
                
                x = line->xyz_nodes[j*3];
                y = line->xyz_nodes[j*3+1];
                
                if( draw_mode == PICKING )
                    glBegin(GL_POLYGON);
                else
                    glBegin(GL_LINE_LOOP);
                glVertex2(x-box_vector, y-box_vector);
                glVertex2(x+box_vector, y-box_vector);
                glVertex2(x+box_vector, y+box_vector);
                glVertex2(x-box_vector, y+box_vector);
                glEnd();
            }
        }
        else if( draw_mode == NORMAL_GET_BOX )
        {
            int j;
            for (j=0; j < line->num_nodes; ++j)
            {
                gv_draw_info_aggregate_select_region( drawinfo, 
                                                      line->xyz_nodes[j*3],
                                                      line->xyz_nodes[j*3+1] );
            }
        }
    }
    else if( gv_shape_type(shape) == GVSHAPE_AREA )
    {
        GvAreaShape   *area = (GvAreaShape *) shape;
        int           ring, j;

        for( ring = 0; ring < area->num_rings; ring++ )
        {
            if( area->num_ring_nodes[ring] > 1 )
            {
                glVertexPointer( 3, GL_GEOCOORD, 0,
                                 area->xyz_ring_nodes[ring] );
                glDrawArrays( GL_LINE_STRIP, 0,
                              area->num_ring_nodes[ring] );
            }
        }
        /* revert to 1 pixel lines for the boxes */
        glLineWidth( 1.0 );
        glLineStipple( 1, 0xffff );

        /* selection/picking per-node work */
        for( ring = 0;
             ring < area->num_rings && draw_mode != NORMAL;
             ring++ )
        {
            for (j=0; j < area->num_ring_nodes[ring]; ++j)
            {
                gvgeocoord x, y;

                x = area->xyz_ring_nodes[ring][j*3];
                y = area->xyz_ring_nodes[ring][j*3+1];

                if( draw_mode == NORMAL_GET_BOX )
                {
                    gv_draw_info_aggregate_select_region( drawinfo, x, y );
                }
                else
                {
                    if( draw_mode == PICKING )
                        glBegin(GL_POLYGON);
                    else
                        glBegin(GL_LINE_LOOP);
                    glVertex2(x-box_vector, y-box_vector);
                    glVertex2(x+box_vector, y-box_vector);
                    glVertex2(x+box_vector, y+box_vector);
                    glVertex2(x-box_vector, y+box_vector);
                    glEnd();
                }
            }
        }
    }
    else if( gv_shape_type(shape) == GVSHAPE_COLLECTION )
    {
        int  i, count = gv_shape_collection_get_count( shape );

        for( i = 0; i < count; i++ )
            gv_shapes_layer_draw_pen( view,
                                      gv_shape_collection_get_shape(shape,i),
                                      drawinfo, pen_info, draw_mode );
    }
    else
    {
        g_warning( "GvShapesLayer: pen tool on point ignored." );
    }

    if (pen_id != -1)
        glDisable( GL_LINE_STIPPLE );
}

/************************************************************************/
/*                     gv_shapes_layer_draw_brush()                     */
/************************************************************************/

static void
gv_shapes_layer_draw_brush( GvViewArea *view, GvShape *shape,
                            GvShapeDrawInfo *drawinfo,
                            GvBrushRenderPart *brush_info,
                            gv_draw_mode draw_mode )

{

    GvAreaShape   *area = (GvAreaShape *) shape;
    int            fill_object, ring;
    double box_vector = drawinfo->dunit * DEFAULT_POINT_SIZE;

    if( gv_shape_type(shape) != GVSHAPE_AREA )
    {
        if( gv_shape_type(shape) == GVSHAPE_COLLECTION )
        {
            int  i, count = gv_shape_collection_get_count( shape );

            for( i = 0; i < count; i++ )
                gv_shapes_layer_draw_brush(
                    view, gv_shape_collection_get_shape(shape,i),
                    drawinfo, brush_info, draw_mode );
        }
        else
        {
            static int bWarningIssued = FALSE;

            if( !bWarningIssued )
            {
                bWarningIssued = TRUE;
                g_warning( "GvShapesLayer: BRUSH tool on non-area ignored." );
            }
        }

        return;
    }

    if (brush_info->b_fore_color_initialized)
        glColor4fv( brush_info->fore_color );
    else
        glColor4fv( drawinfo->color );

    if( area->fill_objects == -1 )
    {
        gv_area_shape_tessellate( area );
    }

    if( area->fill != NULL && area->fill_objects > 0 )
    {
        glVertexPointer(3, GL_GEOCOORD, 0, area->fill->data );

        for( fill_object = 0;
             fill_object < area->fill_objects;
             fill_object++ )
        {
            int f_offset=g_array_index(area->mode_offset,gint,
                                       fill_object*2+1);
            int f_mode = g_array_index(area->mode_offset,gint,
                                       fill_object*2);
            int f_len;

            if( fill_object == area->fill_objects-1 )
                f_len = area->fill->len - f_offset;
            else
                f_len = g_array_index(area->mode_offset,gint,
                                      fill_object*2+3)
                    - f_offset;

            glDrawArrays(f_mode, f_offset, f_len);
        }
    }

    /* Draw small box around each point in selected mode */
    for( ring = 0;
         ring < area->num_rings && draw_mode == SELECTED;
         ring++ )
    {
        int j;

        for (j=0; j < area->num_ring_nodes[ring]; ++j)
        {
            gvgeocoord x, y;

            x = area->xyz_ring_nodes[ring][j*3];
            y = area->xyz_ring_nodes[ring][j*3+1];

            glBegin(GL_LINE_LOOP);
            glVertex2(x-box_vector, y-box_vector);
            glVertex2(x+box_vector, y-box_vector);
            glVertex2(x+box_vector, y+box_vector);
            glVertex2(x-box_vector, y+box_vector);
            glEnd();
        }
    }

    if( draw_mode == NORMAL_GET_BOX )
    {
        int j;

        for (j=0; j < area->num_ring_nodes[ring]; ++j)
        {
            gvgeocoord x, y;

            x = area->xyz_ring_nodes[ring][j*3];
            y = area->xyz_ring_nodes[ring][j*3+1];

            gv_draw_info_aggregate_select_region( drawinfo, x, y );
        }
    }
}

/************************************************************************/
/*                     gv_shapes_layer_draw_shape()                     */
/************************************************************************/

void gv_shapes_layer_draw_shape( GvViewArea *view, GvShapesLayer *layer,
                                 int part_index, GvShape *shape_obj,
                                 gv_draw_mode draw_mode,
                                 GvShapeDrawInfo *drawinfo )
{
    gvgeocoord x, y, z;
    GvShapeDrawInfo sAltDrawInfo;

/* -------------------------------------------------------------------- */
/*      If we weren't provided with layer drawing defaults, compute     */
/*      some now.                                                       */
/* -------------------------------------------------------------------- */
    if( drawinfo == NULL )
    {
        gv_shapes_layer_get_draw_info( view, layer, &sAltDrawInfo );
        drawinfo = &sAltDrawInfo;
    }

    if( part_index != GVP_LAST_PART )
    {
        int      part_draw_mode = draw_mode;

        while( part_index != GVP_LAST_PART )
        {
            GvRenderPart    *part_info;
            int      part_type, next_part_index;

            part_type = gv_part_index_to_type( part_index );
            part_info = gv_shape_layer_get_part( GV_SHAPE_LAYER(layer),
                                                 part_index );
            next_part_index = part_info->next_part;

            /* if there are multiple parts, we need to aggregate 
               selection boxes. */

            if( part_draw_mode == SELECTED 
                && gv_shape_type(shape_obj) == GVSHAPE_POINT
                && next_part_index != GVP_LAST_PART )
                part_draw_mode = NORMAL_GET_BOX;


            g_assert( part_info != NULL );

            if( part_type == GvLabelPart
                && gv_shape_type(shape_obj) == GVSHAPE_POINT )
            {
                x = ((GvPointShape *) shape_obj)->x;
                y = ((GvPointShape *) shape_obj)->y;
                z = ((GvPointShape *) shape_obj)->z;

                gv_shapes_layer_draw_label( view, layer, shape_obj, drawinfo,
                                            x, y,
                                            (GvLabelRenderPart *) part_info,
                                            part_draw_mode );
            }
            else if( part_type == GvSymbolPart
                     && gv_shape_type(shape_obj) == GVSHAPE_POINT )
            {
                x = ((GvPointShape *) shape_obj)->x;
                y = ((GvPointShape *) shape_obj)->y;
                z = ((GvPointShape *) shape_obj)->z;

                gv_shapes_layer_draw_symbol( view, layer, shape_obj, drawinfo,
                                             x, y, z,
                                             (GvSymbolRenderPart *) part_info,
                                             part_index, part_draw_mode );
            }
            else if( part_type == GvPenPart )
            {
                gv_shapes_layer_draw_pen( view, shape_obj, drawinfo,
                                          (GvPenRenderPart *) part_info,
                                          part_draw_mode );
            }
            else if( part_type == GvBrushPart )
            {
                gv_shapes_layer_draw_brush(view, shape_obj, drawinfo,
                                           (GvBrushRenderPart *) part_info,
                                           part_draw_mode );
            }

            part_index = next_part_index;

            /* We only draw the first part with a selection box. */
            if( part_draw_mode == SELECTED )
                part_draw_mode = NORMAL;
        }

        /* do we need to draw the selection box now? */
        if( part_draw_mode == NORMAL_GET_BOX && draw_mode == SELECTED )
        {
            gv_draw_info_grow_box( drawinfo, 1.2 );

            /* draw box around cross hair */
            glBegin(GL_LINE_LOOP);
            glVertex2( 
                drawinfo->selection_box.x, 
                drawinfo->selection_box.y );
            glVertex2( 
                drawinfo->selection_box.x + drawinfo->selection_box.width, 
                drawinfo->selection_box.y );
            glVertex2( 
                drawinfo->selection_box.x + drawinfo->selection_box.width, 
                drawinfo->selection_box.y + drawinfo->selection_box.height);
            glVertex2( 
                drawinfo->selection_box.x, 
                drawinfo->selection_box.y + drawinfo->selection_box.height);
            glEnd();
        }
    }

    else if( gv_shape_type(shape_obj) == GVSHAPE_POINT )
    {
        if( draw_mode != PICKING )
        {
            GvColor      point_color;
            double       dx = drawinfo->dx * drawinfo->point_size;
            double       dy = drawinfo->dy * drawinfo->point_size;

            gv_color_copy( point_color, drawinfo->point_color );

            gv_shapes_layer_override_color( shape_obj, point_color,
                                            "_gv_color" );
            glColor4fv(point_color);

            x = ((GvPointShape *) shape_obj)->x;
            y = ((GvPointShape *) shape_obj)->y;
            z = ((GvPointShape *) shape_obj)->z;

            /* default to simple cross */
            glBegin(GL_LINES);
            glVertex3(x-dx, y-dy, z);
            glVertex3(x+dx, y+dy, z);
            glVertex3(x+dy, y-dx, z);
            glVertex3(x-dy, y+dx, z);
            glEnd();
        }

        if( draw_mode == SELECTED || draw_mode == PICKING )
        {
            double delta = drawinfo->dunit *(drawinfo->point_size+2);

            x = ((GvPointShape *) shape_obj)->x;
            y = ((GvPointShape *) shape_obj)->y;
            z = ((GvPointShape *) shape_obj)->z;

            /* draw box around cross hair */
            if( draw_mode == SELECTED )
                glBegin(GL_LINE_LOOP);
            else
                glBegin(GL_POLYGON);

            glVertex3(x-delta, y-delta, z);
            glVertex3(x+delta, y-delta, z);
            glVertex3(x+delta, y+delta, z);
            glVertex3(x-delta, y+delta, z);
            glEnd();
        }
        else if( draw_mode == NORMAL_GET_BOX )
        {
            double delta = drawinfo->dunit *(drawinfo->point_size+2);

            x = ((GvPointShape *) shape_obj)->x;
            y = ((GvPointShape *) shape_obj)->y;

            gv_draw_info_aggregate_select_region( drawinfo, 
                                                  x - delta, y - delta );
            gv_draw_info_aggregate_select_region( drawinfo, 
                                                  x + delta, y + delta );
        }
    }

    else if( gv_shape_type(shape_obj) == GVSHAPE_LINE )
    {
        GvLineShape   *line = (GvLineShape *) shape_obj;

        if( draw_mode != PICKING )
        {
            GvColor       color;

            gv_color_copy( color, drawinfo->line_color );

            gv_shapes_layer_override_color( shape_obj, color, "_gv_color" );
            glColor4fv(color);
        }

        glLineWidth( drawinfo->line_width );

        glVertexPointer( 3, GL_GEOCOORD, 0, line->xyz_nodes );
        glDrawArrays( GL_LINE_STRIP, 0, line->num_nodes );

        glLineWidth( DEFAULT_LINE_WIDTH );

        if( draw_mode == SELECTED || draw_mode == PICKING )
        {
#ifdef ATLANTIS_BUILD
                /* Atlantis builds: polygon selection draws small
                   diamonds around nodes instead of large squares */
                double delta = drawinfo->dunit
                    * (DEFAULT_POINT_SIZE + drawinfo->line_width - 1 ) * 0.5;

#else
            double delta = drawinfo->dunit
                * (DEFAULT_POINT_SIZE + drawinfo->line_width - 1 );
#endif
            int j;

            /* Draw small box around each point */
            for (j=0; j < line->num_nodes; ++j)
            {
                x = line->xyz_nodes[j*3];
                y = line->xyz_nodes[j*3+1];

                if( draw_mode == SELECTED )
                    glBegin(GL_LINE_LOOP);
                else
                    glBegin(GL_POLYGON);
#ifdef ATLANTIS_BUILD
                glVertex2(x, y-delta);
                glVertex2(x-delta, y);
                glVertex2(x, y+delta);
                glVertex2(x+delta, y);

#else                        
                glVertex2(x-delta, y-delta);
                glVertex2(x+delta, y-delta);
                glVertex2(x+delta, y+delta);
                glVertex2(x-delta, y+delta);
#endif
                glEnd();
            }
        }
        else if( draw_mode == NORMAL_GET_BOX )
        {
            int j;
            for (j=0; j < line->num_nodes; ++j)
            {
                x = line->xyz_nodes[j*3];
                y = line->xyz_nodes[j*3+1];

            }
            gv_draw_info_aggregate_select_region( drawinfo, x, y );
        }
    }

    else if( gv_shape_type(shape_obj) == GVSHAPE_AREA )
    {
        GvAreaShape   *area = (GvAreaShape *) shape_obj;
        int            want_fill = drawinfo->area_fill_color[3] > 0.001;
        int            fill_object, ring;
        int            gl_line_mode = GL_LINE_STRIP;
        GvColor        color;


        if( want_fill && area->fill_objects == -1 )
            gv_area_shape_tessellate( area );

        if( area->fill != NULL && area->fill_objects > 0 && want_fill )
        {
            /* set fill color */
            if( draw_mode != PICKING )
            {
                gv_color_copy( color, drawinfo->area_fill_color );

                gv_shapes_layer_override_color( shape_obj, color,
                                                "_gv_fill_color" );
                glColor4fv(color);
            }

            glVertexPointer(3, GL_GEOCOORD, 0, area->fill->data );
            gl_line_mode = GL_LINE_LOOP;
        }

        for( fill_object = 0;
             fill_object < area->fill_objects && want_fill;
             fill_object++ )
        {
            int f_offset=g_array_index(area->mode_offset,gint,
                                       fill_object*2+1);
            int f_mode = g_array_index(area->mode_offset,gint,
                                       fill_object*2);
            int f_len;

            if( fill_object == area->fill_objects-1 )
                f_len = area->fill->len - f_offset;
            else
                f_len = g_array_index(area->mode_offset,gint,
                                      fill_object*2+3)
                    - f_offset;

            glDrawArrays(f_mode, f_offset, f_len);
        }

        /* Get edge color */
        gv_color_copy( color, drawinfo->area_edge_color );
        gv_shapes_layer_override_color( shape_obj, color, "_gv_color" );

        /* only draw the edge if it's not transparent */
        if (color[3] > 0.001 || draw_mode != NORMAL )
        {
            glLineWidth( drawinfo->area_edge_width );

            if( draw_mode != PICKING )
                glColor4fv(color);

            for( ring = 0; ring < area->num_rings; ring++ )
            {
                if( area->num_ring_nodes[ring] > 1 )
                {
                    glVertexPointer( 3, GL_GEOCOORD, 0,
                                     area->xyz_ring_nodes[ring] );
                    glDrawArrays( gl_line_mode, 0,
                                  area->num_ring_nodes[ring] );
                }
            }
            glLineWidth( DEFAULT_LINE_WIDTH );

            if( draw_mode == SELECTED || draw_mode == PICKING )
            {
#ifdef ATLANTIS_BUILD
                /* Atlantis builds: polygon selection draws small
                   diamonds around nodes instead of large squares */
                double delta = drawinfo->dunit
                    * (DEFAULT_POINT_SIZE + drawinfo->line_width - 1 ) * 0.5;

#else
                double delta = drawinfo->dunit
                    * (DEFAULT_POINT_SIZE + drawinfo->line_width - 1 );
#endif
                int    j;

                for( ring = 0; ring < area->num_rings; ring++ )
                {
                    /* Draw small box around each point */
                    if( draw_mode == SELECTED )
                        glColor4f(color[0], color[1], color[2], 1.0);

                    for (j=0; j < area->num_ring_nodes[ring]; ++j)
                    {
                        x = area->xyz_ring_nodes[ring][j*3];
                        y = area->xyz_ring_nodes[ring][j*3+1];

                        if( draw_mode == NORMAL_GET_BOX )
                            gv_draw_info_aggregate_select_region( drawinfo, 
                                                                  x, y );
                        else
                        {
                            if( draw_mode == SELECTED )
                                glBegin(GL_LINE_LOOP);
                            else
                                glBegin(GL_POLYGON);

#ifdef ATLANTIS_BUILD
                            glVertex2(x, y-delta);
                            glVertex2(x-delta, y);
                            glVertex2(x, y+delta);
                            glVertex2(x+delta, y);

#else                            
                            glVertex2(x-delta, y-delta);
                            glVertex2(x+delta, y-delta);
                            glVertex2(x+delta, y+delta);
                            glVertex2(x-delta, y+delta);
#endif
                            glEnd();
                        }
                    }
                }
            }
        }
    }
    else if( gv_shape_type(shape_obj) == GVSHAPE_COLLECTION )
    {
        int  i_sub, count = gv_shape_collection_get_count( shape_obj );

        for( i_sub = 0; i_sub < count; i_sub++ )
        {
            GvShape *sub_shape
                = gv_shape_collection_get_shape(shape_obj, i_sub);

              GV_SHAPES_LAYER_GET_CLASS(layer)->draw_shape( view, layer,
                                        GVP_LAST_PART,
                                        sub_shape, draw_mode, drawinfo );
        }
    }
}

/************************************************************************/
/*                        gv_shapes_layer_draw()                        */
/************************************************************************/
void
gv_shapes_layer_draw(GvLayer *r_layer, GvViewArea *view)
{
    GvShapesLayer *layer = GV_SHAPES_LAYER(r_layer);
    gint i, shape_count;
    gint *selected, presentation;
    GvShapeDrawInfo   drawinfo;
    gint bAntialiased, in_list_count = 0, out_list_count = 0;
    const char * pszAntialiased;
#ifdef ATLANTIS_BUILD
    int          bDisplayListsEnabled = 0; /* display lists unreliable */ 
#else
    int          bDisplayListsEnabled = 0;
        /*gv_manager_get_preference(gv_get_manager(),"display_lists") == NULL
        || EQUAL(gv_manager_get_preference(gv_get_manager(),
                                           "display_lists"),"ON"); */
#endif
    presentation = GV_LAYER(layer)->presentation;
    selected = GV_SHAPE_LAYER_SELBUF(layer);
    shape_count = gv_shapes_num_shapes(layer->data);

/* -------------------------------------------------------------------- */
/*      Setup layer wide drawing state.                                 */
/* -------------------------------------------------------------------- */
    gv_shapes_layer_get_draw_info( view, layer, &drawinfo );

    glEnableClientState(GL_VERTEX_ARRAY);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    //this enables anti-aliasing for lines, points and polygons
    pszAntialiased = gv_properties_get( &(GV_DATA(layer)->properties),
                                        "_gl_antialias");

    //the property can be NULL or 0 to turn it off, otherwise it is on
    if (pszAntialiased == NULL )
    {
        bAntialiased = FALSE;
    }
    else if (strcmp(pszAntialiased, "0") == 0)
    {
        bAntialiased = FALSE;
    }
    else
    {
        bAntialiased = TRUE;
    }

    if (bAntialiased)
        glEnable( GL_LINE_SMOOTH );

/* -------------------------------------------------------------------- */
/*      Draw all shapes that are not scale dependent, potentially       */
/*      using a display list.                                           */
/* -------------------------------------------------------------------- */
    if( glIsList( layer->display_list ) )
    {
        glCallList( layer->display_list );
    }
    else if( bDisplayListsEnabled )
    {
        layer->display_list = glGenLists( 1 );
        glNewList( layer->display_list, GL_COMPILE_AND_EXECUTE );

        for (i=0; i < shape_count; ++i)
        {
            GvShape *shape_obj;
            guint    part_index;

            if (selected[i] && !presentation)
                continue;

            shape_obj = gv_shapes_get_shape(layer->data,i);
            if( shape_obj == NULL )
                continue;

            /*
             * We need to ensure the parts are initialized as this sets the
             * scale dependent flag we need to test.
             */
            part_index =
                gv_shape_layer_get_first_part_index( GV_SHAPE_LAYER(layer), i );
            if( part_index == GVP_UNINITIALIZED_PART )
            {
                gv_shape_layer_update_renderinfo( GV_SHAPE_LAYER(layer), i );
                part_index =
                    gv_shape_layer_get_first_part_index( GV_SHAPE_LAYER(layer), i);
                g_assert( part_index != GVP_UNINITIALIZED_PART );
            }

            /* Cross hairs are scale dependent drawing. */
            if( part_index == GVP_LAST_PART
                && gv_shape_type(shape_obj) == GVSHAPE_POINT )
                gv_shape_layer_set_scale_dep( GV_SHAPE_LAYER(layer),
                                              i, TRUE );

            /*
             * Only draw this shape as part of the display list if it is
             * not scale dependent.
             */
            if( !gv_shape_layer_get_scale_dep( GV_SHAPE_LAYER(layer), i ) )
            {
                in_list_count++;
                drawinfo.box_set = FALSE;
                GV_SHAPES_LAYER_GET_CLASS(layer)->draw_shape( view, layer, 
                                        part_index, shape_obj,
                                        NORMAL, &drawinfo );
            }
        }

        glEndList();
    }

/* -------------------------------------------------------------------- */
/*      Draw all scale dependent shapes.                                */
/* -------------------------------------------------------------------- */
    for (i=0; i < shape_count; ++i)
    {
        GvShape *shape_obj;
        guint    part_index;

        if (selected[i] && !presentation)
            continue;

        shape_obj = gv_shapes_get_shape(layer->data,i);
        if( shape_obj == NULL )
            continue;

        part_index =
            gv_shape_layer_get_first_part_index( GV_SHAPE_LAYER(layer), i );

        if( part_index == GVP_UNINITIALIZED_PART )
        {
            gv_shape_layer_update_renderinfo( GV_SHAPE_LAYER(layer), i );
            part_index =
                gv_shape_layer_get_first_part_index( GV_SHAPE_LAYER(layer), i);
            g_assert( part_index != GVP_UNINITIALIZED_PART );
        }

        if( !bDisplayListsEnabled
            || gv_shape_layer_get_scale_dep( GV_SHAPE_LAYER(layer), i ) )
        {
            out_list_count++;
            drawinfo.box_set = FALSE;
            GV_SHAPES_LAYER_GET_CLASS(layer)->draw_shape( view, layer, 
                                        part_index, shape_obj,
                                        NORMAL, &drawinfo );
        }
    }

/* -------------------------------------------------------------------- */
/*      Restore drawing state.                                          */
/* -------------------------------------------------------------------- */
    glDisable(GL_BLEND);
    glDisableClientState(GL_VERTEX_ARRAY);

    if (bAntialiased)
        glDisable( GL_LINE_SMOOTH );


    if( in_list_count > 0 )
    {
        CPLDebug( "OpenEV", "GvShapesLayer: %d in display list, %d not.",
                  in_list_count, out_list_count );
    }

/* -------------------------------------------------------------------- */
/*      Do we need to draw selected shapes now?                         */
/* -------------------------------------------------------------------- */
    if( gv_shape_layer_selected( GV_SHAPE_LAYER(layer), GV_FIRST, &i )
        && !GV_SHAPE_LAYER(layer)->flags & GV_DELAY_SELECTED)
    {
        gv_shapes_layer_draw_selected(GV_SHAPE_LAYER(layer), view);
    }
}

/************************************************************************/
/*                   gv_shapes_layer_draw_selected()                    */
/************************************************************************/

void
gv_shapes_layer_draw_selected(GvShapeLayer *r_layer, GvViewArea *view)
{
    GvShapesLayer *layer = GV_SHAPES_LAYER(r_layer);
    gint i, shape_count;
    gint *selected;
    GvShapeDrawInfo   drawinfo;

    gv_shapes_layer_get_draw_info( view, layer, &drawinfo );

    selected = GV_SHAPE_LAYER_SELBUF(layer);
    shape_count = gv_shapes_num_shapes(layer->data);

    /*
      ideal box size for around point cross hairs.

    bx = by = drawinfo.point_size + 2;
    gv_view_area_correct_for_transform(view, bx, by, &bx, &by);
    */

    glEnableClientState(GL_VERTEX_ARRAY);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    for (i=0; i < shape_count; ++i)
    {
        GvShape     *shape_obj;
        guint       part_index;

        /* skip unselected shapes */
        if (!selected[i])
            continue;

        /* fetch the shape itself */
        shape_obj = gv_shapes_get_shape(layer->data, i);
        if( shape_obj == NULL )
            continue;

        /* get information on first part, ogrfs driven. */
        part_index =
            gv_shape_layer_get_first_part_index( GV_SHAPE_LAYER(layer), i );
        if( part_index == GVP_UNINITIALIZED_PART )
        {
            gv_shape_layer_update_renderinfo( GV_SHAPE_LAYER(layer), i );
            part_index =
                gv_shape_layer_get_first_part_index( GV_SHAPE_LAYER(layer), i);
            g_assert( part_index != GVP_UNINITIALIZED_PART );
        }

        /* draw the shape in selected mode. */
        drawinfo.box_set = FALSE;
        GV_SHAPES_LAYER_GET_CLASS(layer)->draw_shape( view, layer, 
                                    part_index, shape_obj,
                                    SELECTED, &drawinfo );
    }
    glDisable(GL_BLEND);
    glDisableClientState(GL_VERTEX_ARRAY);
}

/************************************************************************/
/*                     gv_shapes_layer_pick_shape()                     */
/************************************************************************/
static void
gv_shapes_layer_pick_shape(GvShapeLayer *r_layer)
{
    GvShapesLayer *layer = GV_SHAPES_LAYER(r_layer);
    gint ii, shape_count;
    GvShapeDrawInfo   drawinfo;
    GvViewArea   *view;

    view = GV_VIEW_AREA(GV_LAYER(r_layer)->view);
    gv_shapes_layer_get_draw_info(view, layer, &drawinfo);

    if (!gv_layer_is_visible(GV_LAYER(layer))) return;
    shape_count = gv_shapes_num_shapes(layer->data);

    glEnableClientState(GL_VERTEX_ARRAY);
    for (ii=0; ii < shape_count; ++ii)
    {
        GvShape * shape_obj = gv_shapes_get_shape(layer->data, ii);
        guint   part_index;

        if(shape_obj == NULL)
            continue;

        /* get information on first part, ogrfs driven. */
        part_index =
            gv_shape_layer_get_first_part_index(GV_SHAPE_LAYER(layer), ii);
        if(part_index == GVP_UNINITIALIZED_PART)
        {
            gv_shape_layer_update_renderinfo(GV_SHAPE_LAYER(layer), ii);
            part_index =
                gv_shape_layer_get_first_part_index(GV_SHAPE_LAYER(layer), ii);
            g_assert(part_index != GVP_UNINITIALIZED_PART);
        }

        /* Load the "ID" of the shape being drawn. */
        glLoadName(ii);

        /* Draw it in PICKING mode */
        drawinfo.box_set = FALSE;
        GV_SHAPES_LAYER_GET_CLASS(layer)->draw_shape(view, layer, 
                                    part_index, shape_obj,
                                    PICKING, &drawinfo);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

/************************************************************************/
/*                     gv_shapes_layer_pick_node()                      */
/************************************************************************/
static void
gv_shapes_layer_pick_node(GvShapeLayer *rlayer)
{
    GvShape *shape;
    gint sel, i, r;
    GvShapeDrawInfo drawinfo;
    double dx, dy, dsize;

    if (!gv_layer_is_visible(GV_LAYER(rlayer))) return;

    if (!gv_shape_layer_selected(rlayer, GV_FIRST, &sel))
        return;

    shape = gv_shapes_get_shape(GV_SHAPES_LAYER(rlayer)->data, sel);
    if( shape == NULL )
        return;

    /* How big should node be drawn?  We want this to match the size
       they are normally drawn visibly */

    gv_shapes_layer_get_draw_info( GV_LAYER(rlayer)->view,
                                   GV_SHAPES_LAYER(rlayer), &drawinfo );

    dsize = drawinfo.dunit;
    dx = dsize * (drawinfo.point_size+2);
    dy = dsize * (drawinfo.point_size+2);

    glEnableClientState(GL_VERTEX_ARRAY);

    /* Nodes first */
    glLoadName(0);
    glPushName(-1);
    glPointSize(2.0);
    for (r=0; r < gv_shape_get_rings(shape); ++r)
    {
        int num_nodes = gv_shape_get_nodes(shape,r);

        glLoadName(r);
        glPushName(-1);

        for (i=0; i < num_nodes; ++i)
        {
            double x, y, z;

            x = gv_shape_get_x(shape,r,i);
            y = gv_shape_get_y(shape,r,i);
            z = gv_shape_get_z(shape,r,i);

            glLoadName(i);

            glBegin(GL_POLYGON);
            glVertex3(x-dx, y-dy, z);
            glVertex3(x+dy, y-dx, z);
            glVertex3(x+dx, y+dy, z);
            glVertex3(x-dy, y+dx, z);
            glVertex3(x-dx, y-dy, z);
            glEnd();

        }
        glPopName(); /* node id */
    }
    glPopName(); /* ring id */

    /* Segments next */
    glLoadName(1);
    glPushName(-1);
    for (r=0; r <  gv_shape_get_rings(shape); ++r)
    {
        int num_nodes = gv_shape_get_nodes(shape,r);

        glLoadName(r);

        /* First segment connects last vertex to first vertex:
           "before" vertex zero as long as the line wouldn't be degenerate. */

        /* April, 2004 bug fix: the last vertex-first vertex segment
         * should not be drawn for line shapes, as this can cause problems
         * with editing if another segment of the line is contained within
         * the segment, eg:
         *      N6----------N7
         *      |           |
         * N1   N5--N4      N8
         * |        |
         * N2-------N3
         * clicking to create a new node on the N4-N5 segment would
         * actually result in a node being inserted before N1.
         */
        glPushName(0);
        if( (gv_shape_type(shape) != GVSHAPE_LINE) &&
            (gv_shape_get_x(shape,r,num_nodes-1) != gv_shape_get_x(shape,r,0)
         || gv_shape_get_y(shape,r,num_nodes-1) != gv_shape_get_y(shape,r,0)))
        {
            glBegin(GL_LINES);
            glVertex3( gv_shape_get_x(shape,r,num_nodes-1),
                       gv_shape_get_y(shape,r,num_nodes-1),
                       gv_shape_get_z(shape,r,num_nodes-1));
            glVertex3( gv_shape_get_x(shape,r,0),
                       gv_shape_get_y(shape,r,0),
                       gv_shape_get_z(shape,r,0));
            glEnd();
        }

        for (i=1; i < num_nodes; ++i)
        {
            glLoadName(i);
            glBegin(GL_LINES);
            glVertex3( gv_shape_get_x(shape,r,i),
                       gv_shape_get_y(shape,r,i),
                       gv_shape_get_z(shape,r,i));
            glVertex3( gv_shape_get_x(shape,r,i-1),
                       gv_shape_get_y(shape,r,i-1),
                       gv_shape_get_z(shape,r,i-1));
            glEnd();
        }
        glPopName(); /* node id */
    }
    glPopName(); /* ring id */
    glDisableClientState(GL_VERTEX_ARRAY);
}

/************************************************************************/
/*                      gv_shapes_layer_get_node()                      */
/************************************************************************/
static void
gv_shapes_layer_get_node(GvShapeLayer *layer, GvNodeInfo *info)
{
    GvShape *shape;

    shape = gv_shapes_get_shape( GV_SHAPES_LAYER(layer)->data,
                                 info->shape_id );
    if( shape == NULL )
        return;

    if( gv_shape_type(shape) == GVSHAPE_POINT )
        info->vertex = (GvVertex *) &((GvPointShape *) shape)->x;
    else if( gv_shape_type(shape) == GVSHAPE_LINE )
    {
        GvLineShape *line = (GvLineShape *) shape;

        info->vertex = (GvVertex *) (line->xyz_nodes + 3*info->node_id);
    }

    else if( gv_shape_type(shape) == GVSHAPE_AREA )
    {
        GvAreaShape *area = (GvAreaShape *) shape;
        gvgeocoord       *ring = area->xyz_ring_nodes[info->ring_id];

        info->vertex = (GvVertex *) (ring + 3*info->node_id);
    }
}

/************************************************************************/
/*                     gv_shapes_layer_move_node()                      */
/************************************************************************/
static void
gv_shapes_layer_move_node(GvShapeLayer *layer, GvNodeInfo *info)
{
    GvShape *shape;

    shape = gv_shapes_get_shape( GV_SHAPES_LAYER(layer)->data,
                                 info->shape_id );
    if( shape == NULL )
        return;

    shape = gv_shape_copy( shape );

    gv_shape_set_xyz( shape, info->ring_id, info->node_id,
                      info->vertex->x,
                      info->vertex->y,
                      0.0 );

    gv_shapes_replace_shapes( GV_SHAPES_LAYER(layer)->data,
                              1, &(info->shape_id),
                              &shape, FALSE );
}

/************************************************************************/
/*                    gv_shapes_layer_insert_node()                     */
/************************************************************************/
static void
gv_shapes_layer_insert_node(GvShapeLayer *layer, GvNodeInfo *info)
{
    GvShape *shape;

    shape = gv_shapes_get_shape( GV_SHAPES_LAYER(layer)->data,
                                 info->shape_id );
    if( shape == NULL )
        return;

    shape = gv_shape_copy( shape );

    gv_shape_insert_node( shape, info->ring_id, info->node_id,
                          info->vertex->x, info->vertex->y, 0.0 );

    gv_shapes_replace_shapes( GV_SHAPES_LAYER(layer)->data,
                              1, &(info->shape_id),
                              &shape, FALSE );
}

/************************************************************************/
/*                    gv_shapes_layer_delete_node()                     */
/************************************************************************/
static void
gv_shapes_layer_delete_node(GvShapeLayer *layer, GvNodeInfo *info)
{
    GvShape *shape;

    shape = gv_shapes_get_shape( GV_SHAPES_LAYER(layer)->data,
                                 info->shape_id );
    if( shape == NULL )
        return;

    shape = gv_shape_copy( shape );

    gv_shape_delete_node( shape, info->ring_id, info->node_id );

    /* delete the ring, if it is now empty */
    if( gv_shape_get_nodes( shape, info->ring_id ) == 0 )
        gv_shape_delete_ring( shape, info->ring_id );

    /* Delete the shape if it is now empty */
    if( gv_shape_get_rings( shape ) == 0 )
    {
        gv_shape_layer_clear_selection( layer );
        gv_shapes_delete_shapes( GV_SHAPES_LAYER(layer)->data,
                                 1, &(info->shape_id) );
        gv_shape_delete( shape );
    }
    else
    {
        gv_shapes_replace_shapes( GV_SHAPES_LAYER(layer)->data,
                                  1, &(info->shape_id),
                                  &shape, FALSE );
    }
}

/************************************************************************/
/*                    gv_shapes_layer_node_motion()                     */
/************************************************************************/
static void
gv_shapes_layer_node_motion(GvShapeLayer *layer, gint area_id)
{
    GvShape *shape;

    shape = gv_shapes_get_shape( GV_SHAPES_LAYER(layer)->data, area_id );
    if( shape == NULL )
        return;

    if( gv_shape_type(shape) == GVSHAPE_AREA )
    {
        ((GvAreaShape *) shape)->fill_objects = -2;
    }
}

/************************************************************************/
/*                  gv_shapes_layer_delete_selected()                   */
/************************************************************************/
static void
gv_shapes_layer_delete_selected(GvShapeLayer *layer)
{
    GArray *sel;

    sel = g_array_new(FALSE, FALSE, sizeof(gint));
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_ALL, sel))
    {
    gv_shapes_delete_shapes(GV_SHAPES_LAYER(layer)->data,
                                sel->len, (gint*)sel->data);
        gv_shape_layer_clear_selection(layer);
    }
    g_array_free(sel, TRUE);
}

/************************************************************************/
/*                 gv_shapes_layer_translate_selected()                 */
/************************************************************************/
static void
gv_shapes_layer_translate_selected(GvShapeLayer *layer, GvVertex *delta)
{
    GArray *sel;

    sel = g_array_new(FALSE, FALSE, sizeof(gint));
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_ALL, sel))
    {
    /* This will force a selection clear */
    gv_shapes_translate_shapes(GV_SHAPES_LAYER(layer)->data,
                                   sel->len, (gint*)sel->data,
                   delta->x, delta->y);
    }
    g_array_free(sel, TRUE);
}

/************************************************************************/
/*                      gv_shapes_layer_extents()                       */
/************************************************************************/
static void
gv_shapes_layer_extents(GvLayer *layer, GvRect *rect)
{
    gv_shapes_get_extents(GV_SHAPES_LAYER(layer)->data, rect);
}

/************************************************************************/
/*                    gv_shapes_layer_data_change()                     */
/************************************************************************/
static void
gv_shapes_layer_data_change(GvData *data, gpointer change_info)
{
    GvShapesLayer  *layer = GV_SHAPES_LAYER(data);

    gv_shape_layer_clear_all_renderinfo( GV_SHAPE_LAYER(data) );

    /* Reset the selected array to reflect the data length */
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(layer),
                  gv_shapes_num_shapes(layer->data));

    /* clear the display list if the data changes, this will cause a new
       list to be created the next time it is drawn */
    if (glIsList(layer->display_list))
    {
        glDeleteLists( layer->display_list, 1 );
        layer->display_list = 0;
    }

}

/************************************************************************/
/*                  gv_shapes_layer_selection_change()                  */
/************************************************************************/
static void gv_shapes_layer_selection_change(GvLayer *data,
                                             gpointer change_info )

{
    /* If the selection changes, we need to clear the display list. */

    if (glIsList(GV_SHAPES_LAYER(data)->display_list))
    {
        glDeleteLists( GV_SHAPES_LAYER(data)->display_list, 1 );
        GV_SHAPES_LAYER(data)->display_list = 0;
    }
}

/************************************************************************/
/*                   gv_shapes_layer_display_change()                   */
/************************************************************************/
static void
gv_shapes_layer_display_change(GvLayer *data, gpointer change_info)

{
    gv_shape_layer_clear_all_renderinfo( GV_SHAPE_LAYER(data) );

    /* clear the display list if the data changes, this will cause a new
       list to be created the next time it is drawn */
    if (glIsList(GV_SHAPES_LAYER(data)->display_list))
    {
        glDeleteLists( GV_SHAPES_LAYER(data)->display_list, 1 );
        GV_SHAPES_LAYER(data)->display_list = 0;
    }
}

/************************************************************************/
/*                      gv_shapes_layer_dispose()                       */
/************************************************************************/

static void gv_shapes_layer_dispose( GObject *gobject )

{
    GvShapesLayer *slayer = GV_SHAPES_LAYER(gobject);

    CPLDebug( "OpenEV", "gv_shapes_layer_dispose(%s)", 
              gv_data_get_name(GV_DATA(slayer)) );

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->dispose(gobject);         
}

/************************************************************************/
/*                      gv_shapes_layer_finalize()                      */
/************************************************************************/
static void
gv_shapes_layer_finalize(GObject *gobject)

{
    GvShapesLayer *layer = GV_SHAPES_LAYER(gobject);

    /* GTK2 PORT... Override GObject finalize, test for and set NULLs
       as finalize may be called more than once. */

    CPLDebug( "OpenEV", "gv_shapes_layer_finalize(%s)",
              gv_data_get_name( GV_DATA(gobject) ) );

    if( layer->data != NULL )
    {
//~         g_object_unref( layer->data );
        layer->data = NULL;
    }

    if( layer->symbol_manager != NULL )
    {
        g_object_unref( layer->symbol_manager );
        layer->symbol_manager = NULL;
    }

    /* delete the display list if it is valid */
    if (layer->display_list != -1) {
      if (glIsList(layer->display_list)) {
        glDeleteLists( layer->display_list, 1 );
      }
      layer->display_list = -1;
    }

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}
