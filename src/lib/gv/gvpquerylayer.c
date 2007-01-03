/******************************************************************************
 * $Id: gvpquerylayer.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Point query layer.
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
 * $Log: gvpquerylayer.c,v $
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
 * Revision 1.11  2004/08/20 13:53:47  warmerda
 * allow GvShapes to be passed into constructor
 *
 * Revision 1.10  2003/08/27 19:58:43  warmerda
 * added force_simple flag for gv_view_area_bmfont_draw
 *
 * Revision 1.9  2003/02/27 03:59:21  warmerda
 * added view to gv_shapes_layer_get_draw_info
 *
 * Revision 1.8  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.7  2000/08/04 14:14:12  warmerda
 * GvShapes shape ids now persistent
 *
 * Revision 1.6  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvpquerylayer.h"
#include "gvpointlayer.h"
#include "gvpoints.h"
#include "gvutils.h"
#include <GL/gl.h>

static void gv_pquery_layer_class_init(GvPqueryLayerClass *klass);
static void gv_pquery_layer_init(GvPqueryLayer *layer);
static void gv_pquery_layer_setup(GvLayer *layer, GvViewArea *view);
static void gv_pquery_layer_draw(GvLayer *layer, GvViewArea *view);
static void gv_pquery_layer_draw_selected(GvShapeLayer *layer, GvViewArea *view);
static void gv_pquery_layer_translate_selected(GvShapeLayer *layer,
					       GvVertex *delta);

GtkType
gv_pquery_layer_get_type(void)
{
    static GtkType pquery_layer_type = 0;

    if (!pquery_layer_type)
    {
	static const GtkTypeInfo pquery_layer_info =
	{
	    "GvPqueryLayer",
	    sizeof(GvPqueryLayer),
	    sizeof(GvPqueryLayerClass),
	    (GtkClassInitFunc) gv_pquery_layer_class_init,
	    (GtkObjectInitFunc) gv_pquery_layer_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	pquery_layer_type = gtk_type_unique(gv_shapes_layer_get_type(),
					    &pquery_layer_info);
    }
    return pquery_layer_type;
}

static void
gv_pquery_layer_class_init(GvPqueryLayerClass *klass)
{
    GvLayerClass *layer_class;
    GvShapeLayerClass *shape_layer_class;
    GvShapesLayerClass *shapes_layer_class;

    layer_class = (GvLayerClass*) klass;
    shape_layer_class = (GvShapeLayerClass*) klass;
    shapes_layer_class = (GvShapesLayerClass*) klass;

    layer_class->setup = gv_pquery_layer_setup;
    layer_class->draw = gv_pquery_layer_draw;
    shape_layer_class->draw_selected = gv_pquery_layer_draw_selected;
    shape_layer_class->translate_selected = gv_pquery_layer_translate_selected;
    shapes_layer_class->draw_shape = gv_pquery_layer_draw_shape;
}

static void
gv_pquery_layer_init(GvPqueryLayer *layer)
{
    layer->font = 0;
    layer->drag_labels_only = TRUE;
}

GtkObject *
gv_pquery_layer_new( GvShapes *data )
{
    GvPqueryLayer *layer = GV_PQUERY_LAYER(gtk_type_new(
	gv_pquery_layer_get_type()));

    if( data == NULL )
    {
        data = GV_SHAPES(gv_shapes_new());
        gv_data_set_name( GV_DATA(data), "Query Points" );
    }

    gv_shapes_layer_set_data( GV_SHAPES_LAYER(layer), data );

    return GTK_OBJECT(layer);
}

static void
gv_pquery_layer_setup(GvLayer *rlayer, GvViewArea *view)
{
    GvPqueryLayer *layer = GV_PQUERY_LAYER(rlayer);

    layer->font = gv_view_area_bmfont_load(view, "Sans 12");
}

static void gv_pquery_layer_draw_text(GvViewArea * view, 
                                      GvPqueryLayer *layer, GvShape *shape,
                                      gvgeocoord dx, gvgeocoord dy )
{
    const char *text;
    gvgeocoord    x, y;

    x = gv_shape_get_x(shape, 0, 0 );
    y = gv_shape_get_y(shape, 0, 0 );

    /* FIXME: the label should be cached in the point structure */
    text = gv_view_area_format_point_query
	(view, gv_data_get_properties(GV_DATA(layer)), x, y);

    /* 'Simple' flag works better when dragging points off edge
       of view, and view scrolls. */
    gv_view_area_bmfont_draw(view, layer->font, x+dx, y+dy, 
                             (char *) text, 1);
}

void gv_pquery_layer_draw_shape(GvViewArea *view, GvShapesLayer *layer,
                                int part_index, GvShape *shape,
                                gv_draw_mode draw_mode,
                                GvShapeDrawInfo *drawinfo)
{
    gint ii, points, width, height;
    gint *selected, presentation;
    gvgeocoord dx, dy, bx, by, x, y;
    float shift_x, shift_y;

    gvgeocoord ll_px, ll_py, ur_px, ur_py, ur_geox, ur_geoy;
    gvgeocoord ul_geox, ul_geoy, lr_geox, lr_geoy;
    GvBMFontInfo *font_info;
    const char *text;
    PangoContext *pango_context;
    PangoRectangle rect;
    char key[64];


    /* ---- Init ---- */
    presentation = GV_LAYER(layer)->presentation;

    /* ---- Create text layout if needed ---- */
    if (drawinfo->pango_layout == NULL) {
        pango_context = gtk_widget_get_pango_context(GTK_WIDGET(view));
        drawinfo->pango_layout = pango_layout_new(pango_context);
    }

    /* >>>> Draw points <<<< */

    /* Crosshairs are "sprites": always drawn upright, the same size */
    dx = drawinfo->point_size;
    dy = 0.0;
    gv_view_area_correct_for_transform(view, dx, dy, &dx, &dy);    
    bx = by = drawinfo->point_size + 2;
    gv_view_area_correct_for_transform(view, bx, by, &bx, &by);    

    x = gv_shape_get_x(shape, 0, 0);
    y = gv_shape_get_y(shape, 0, 0);

    /* Draw crosshairs */
    if (draw_mode != PQUERY_LABELS) {
        glBegin(GL_LINES);
	glVertex2(x-dx, y-dy);
	glVertex2(x+dx, y+dy);
	glVertex2(x+dy, y-dx);
	glVertex2(x-dy, y+dx);
	glEnd();

	if (draw_mode == PQUERY_POINTS) {
	    return;
	}

	if ((draw_mode == PICKING) || (draw_mode == SELECTED && !presentation)) {

	    /* Draw box around crosshairs */
	    glBegin(GL_LINE_LOOP);
	    glVertex2(x-bx, y-by);
	    glVertex2(x+by, y-bx);
	    glVertex2(x+bx, y+by);
	    glVertex2(x-by, y+bx);
	    glEnd();
	}
    }

    dy = drawinfo->point_size;

    /* ---- Check for shifted label ---- */
    shift_x = 0;
    shift_y = 0;
    sprintf(key, "%s", GV_PQUERY_DX_PROP);
    text = gv_properties_get(&shape->properties, key);
    if (text != NULL) {
	shift_x = atof(text);
    }
    sprintf(key, "%s", GV_PQUERY_DY_PROP);
    text = gv_properties_get(&shape->properties, key);
    if (text != NULL) {
	shift_y = atof(text);
    }

    if (draw_mode == PICKING) {
        text = gv_view_area_format_point_query
	    (view, gv_data_get_properties(GV_DATA(layer)), x + dx, y + dy);
	dx += shift_x;
	dy += shift_y;

	/* ---- Get font info ---- */
	font_info = &(g_array_index(view->bmfonts, GvBMFontInfo,
				    GV_PQUERY_LAYER(layer)->font));

	/* ---- Get text bounds ---- */
	pango_layout_set_text(drawinfo->pango_layout, text, -1);
	pango_layout_set_font_description
	    (drawinfo->pango_layout, font_info->pango_desc);
	pango_layout_get_pixel_size(drawinfo->pango_layout, &(width), &(height));
        gv_view_area_inverse_map_pointer(view, x + dx, y + dy,
					 &ll_px, &ll_py);
        ll_px -= 2;
        ll_py += 4;
        ur_px = ll_px + width + 4;
	ur_py = ll_py - height - 2;
        gv_view_area_map_pointer(view, ur_px, ur_py, &ur_geox, &ur_geoy);
        gv_view_area_map_pointer(view, ll_px, ll_py, &x, &y);
        gv_view_area_map_pointer(view, ll_px, ur_py, &ul_geox, &ul_geoy);
        gv_view_area_map_pointer(view, ur_px, ll_py, &lr_geox, &lr_geoy);

	/* ---- Draw filled box picking region to select text ---- */
	glBegin(GL_POLYGON);
	glVertex3(x, y, 0.0);
	glVertex3(ul_geox, ul_geoy, 0.0);
	glVertex3(ur_geox, ur_geoy, 0.0);
	glVertex3(lr_geox, lr_geoy, 0.0);
	glVertex3(x, y, 0.0);
	glEnd();
    }
    else {

	/* Draw text */
	/* Note: Drawing text in picking mode causes errors, not sure why -
	   filled box (above) should be sufficient, though. */

	dx += shift_x;
	dy += shift_y;
	gv_pquery_layer_draw_text(view, GV_PQUERY_LAYER(layer), shape, dx, dy);

	/* ---- Draw arrow to orig location ---- */
        if (draw_mode == PQUERY_LABELS) {
	    glBegin(GL_LINES);
            glVertex2(x + dx, y + dy);
            glVertex2(x - (GV_SHAPE_LAYER(layer))->selected_motion.x,
		      y - (GV_SHAPE_LAYER(layer))->selected_motion.y);
	    glEnd();
	}
	else if ((ABS(shift_x) > 8) || (ABS(shift_y) > 8)) {
	    glBegin(GL_LINES);
            glVertex2(x, y);
            glVertex2(x + shift_x, y + shift_y);
	    glEnd();
	}
    }
}

static void
gv_pquery_layer_draw(GvLayer *rlayer, GvViewArea *view)
{
    GvPqueryLayer *layer = GV_PQUERY_LAYER(rlayer);
    gint ii, draw_mode, points;
    gint *selected;
    GvShapeDrawInfo    drawinfo;
    GvShape           *shape;


    /* ---- Init ---- */
    selected = GV_SHAPE_LAYER_SELBUF(layer);
    gv_shapes_layer_get_draw_info(view, GV_SHAPES_LAYER(rlayer), &drawinfo);
    points = gv_shapes_num_shapes(GV_SHAPES_LAYER(layer)->data);

    glColor4fv(drawinfo.point_color);

    /* ---- Loop points ---- */
    for (ii=0; ii < points; ++ii) {
        shape = gv_shapes_get_shape(GV_SHAPES_LAYER(layer)->data, ii);
        if(shape == NULL) continue;
        draw_mode = NORMAL;

        if (selected[ii]) {
	    draw_mode = SELECTED;
            if (GV_SHAPE_LAYER(layer)->flags & GV_DELAY_SELECTED) {
                if (GV_SHAPE_LAYER(layer)->flags & GV_ALT_SELECTED) {

		    /* ---- Draw point (no text) at original location ---- */
		    draw_mode = PQUERY_POINTS;
		}
		else {

		    /* ---- Draw nothing at original location ---- */
		    continue;
		}
	    }
	}

        gv_pquery_layer_draw_shape(view, GV_SHAPES_LAYER(layer), 0,
				   shape, draw_mode, &drawinfo);
    }
}

static void
gv_pquery_layer_draw_selected(GvShapeLayer *rlayer, GvViewArea *view)
{
    GvPqueryLayer *layer = GV_PQUERY_LAYER(rlayer);
    gint ii, draw_mode, points;
    gint *selected;
    GvShapeDrawInfo    drawinfo;
    GvShape           *shape;


    /* ---- Init ---- */
    selected = GV_SHAPE_LAYER_SELBUF(layer);
    gv_shapes_layer_get_draw_info(view, GV_SHAPES_LAYER(rlayer), &drawinfo);
    points = gv_shapes_num_shapes(GV_SHAPES_LAYER(layer)->data);

    glColor4fv(drawinfo.point_color);

    /* ---- Loop points ---- */
    for (ii=0; ii < points; ++ii) {
        shape = gv_shapes_get_shape(GV_SHAPES_LAYER(layer)->data, ii);
        if(shape == NULL) continue;
        draw_mode = SELECTED;

        if (selected[ii]) {

            if (GV_SHAPE_LAYER(layer)->flags & GV_DELAY_SELECTED) {
                if (GV_SHAPE_LAYER(layer)->flags & GV_ALT_SELECTED) {

		    /* ---- Draw label only at drag location ---- */
		    draw_mode = PQUERY_LABELS;
		}
	    }

            gv_pquery_layer_draw_shape(view, GV_SHAPES_LAYER(layer), 0,
				       shape, draw_mode, &drawinfo);
	}
    }
}

static void
gv_pquery_layer_translate_selected(GvShapeLayer *layer, GvVertex *delta)
{
    GArray *sel;
    GvShape *shape;
    int ii, index, found;
    float shift_x, shift_y;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};
    const char *val;
    char prop[64], szPropName[64];
    GvProperties *layer_prop;
    GvData *data;

    sel = g_array_new(FALSE, FALSE, sizeof(gint));
    if (gv_shape_layer_selected(GV_SHAPE_LAYER(layer), GV_ALL, sel))
    {

	/* ---- Check if dragging labels ---- */
	if (GV_SHAPE_LAYER(layer)->flags & GV_ALT_SELECTED) {
	    change_info.num_shapes = sel->len;
	    change_info.shape_id = (gint*)sel->data;
	    gv_data_changing(GV_DATA(GV_SHAPES_LAYER(layer)->data), &change_info);

	    layer_prop = &GV_DATA(GV_SHAPES_LAYER(layer)->data)->properties;

	    found = FALSE;
	    for (ii = 0; TRUE; ii++) {
		sprintf(szPropName, "_field_name_%d", ii + 1);
		val = gv_properties_get(layer_prop, szPropName);
		if (val == NULL) break;
		if (strcmp(val, GV_PQUERY_DX_PROP) == 0) {
		    found = TRUE;
		}
	    }

	    if (!found) {

		/* ---- Add fields to layer if needed ---- */
		sprintf(szPropName, "_field_name_%d", ii + 1);
		gv_properties_set(layer_prop, szPropName, GV_PQUERY_DX_PROP);
		sprintf(szPropName, "_field_width_%d", ii + 1);
		gv_properties_set(layer_prop, szPropName, "36");
		sprintf(szPropName, "_field_type_%d", ii + 1);
		gv_properties_set(layer_prop, szPropName, "string");

		sprintf(szPropName, "_field_name_%d", ii + 2);
    		gv_properties_set(layer_prop, szPropName, GV_PQUERY_DY_PROP);
		sprintf(szPropName, "_field_width_%d", ii + 2);
		gv_properties_set(layer_prop, szPropName, "36");
		sprintf(szPropName, "_field_type_%d", ii + 2);
		gv_properties_set(layer_prop, szPropName, "string");
	    }

	    /* ---- Translate labels only ---- */
	    for (ii=0; ii < sel->len; ii++) {
		index = ((gint*)sel->data)[ii];
		shape = gv_shapes_get_shape(GV_SHAPES_LAYER(layer)->data,
					    index);
		if( shape == NULL )
		    continue;

		shift_x = 0;
		shift_y = 0;
		val = gv_properties_get(&shape->properties, GV_PQUERY_DX_PROP);
		if (val != NULL) {
		    shift_x = atof(val);
		}
		val = gv_properties_get(&shape->properties, GV_PQUERY_DY_PROP);
		if (val != NULL) {
		    shift_y = atof(val);
		}
		shift_x += delta->x;
		shift_y += delta->y;

                sprintf(prop, "%22.10f", shift_x);
                gv_properties_set(&shape->properties, GV_PQUERY_DX_PROP, prop); 
                sprintf(prop, "%22.10f", shift_y);
                gv_properties_set(&shape->properties, GV_PQUERY_DY_PROP, prop);
            }

	    gv_data_changed(GV_DATA(GV_SHAPES_LAYER(layer)->data), &change_info);
	}
	else {

	    /* This will force a selection clear */

            /* ---- Perform usual translate if not dragging labels ---- */
	    gv_shapes_translate_shapes(GV_SHAPES_LAYER(layer)->data,
				       sel->len, (gint*)sel->data,
				       delta->x, delta->y);
        }
    }

    g_array_free(sel, TRUE);
}
