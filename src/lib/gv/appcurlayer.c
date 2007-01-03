/******************************************************************************
 * $Id: appcurlayer.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  APP/Currents
 * Purpose:  Implementation of current layer.
 *
 *           This layer is intended specifically for use in APP, and
 *           should only be changed on behalf of Atlantis, though it can 
 *           serve as an example of custom layer drawing for others.
 *
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
 * $Log: appcurlayer.c,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:35  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:25  uid1026
 *
 * Imported sources
 *
 * Revision 1.4  2003/02/27 03:59:21  warmerda
 * added view to gv_shapes_layer_get_draw_info
 *
 * Revision 1.3  2002/11/05 04:05:31  warmerda
 * geocoord update
 *
 * Revision 1.2  2001/08/08 17:44:12  warmerda
 * use gv_shape_type() macro
 *
 * Revision 1.1  2000/08/25 20:02:22  warmerda
 * New
 *
 */

#include <math.h>
#include <stdlib.h>
#include "appcurlayer.h"
#include "gvutils.h"
#include <GL/gl.h>

static void app_cur_layer_class_init(AppCurLayerClass *klass);
static void app_cur_layer_init(AppCurLayer *layer);
static void app_cur_layer_setup(GvLayer *layer, GvViewArea *view);
static void app_cur_layer_draw(GvLayer *layer, GvViewArea *view);
static void app_cur_layer_draw_selected(GvShapeLayer *layer, 
                                          GvViewArea *view);

#ifndef PI
#define PI  3.1415927
#endif

GtkType
app_cur_layer_get_type(void)
{
    static GtkType cur_layer_type = 0;

    if (!cur_layer_type)
    {
	static const GtkTypeInfo cur_layer_info =
	{
	    "AppCurLayer",
	    sizeof(AppCurLayer),
	    sizeof(AppCurLayerClass),
	    (GtkClassInitFunc) app_cur_layer_class_init,
	    (GtkObjectInitFunc) app_cur_layer_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	cur_layer_type = gtk_type_unique(gv_shapes_layer_get_type(),
                                         &cur_layer_info);
    }
    return cur_layer_type;
}

static void
app_cur_layer_class_init(AppCurLayerClass *klass)
{
    GvLayerClass *layer_class;
    GvShapeLayerClass *shape_layer_class;

    layer_class = (GvLayerClass*) klass;
    shape_layer_class = (GvShapeLayerClass*) klass;

    layer_class->setup = app_cur_layer_setup;
    layer_class->draw = app_cur_layer_draw;
    shape_layer_class->draw_selected = app_cur_layer_draw_selected;
}

static void
app_cur_layer_init(AppCurLayer *layer)
{
}

GtkObject *
app_cur_layer_new(GvShapes *data)
{
    AppCurLayer *layer = APP_CUR_LAYER(gtk_type_new(
	app_cur_layer_get_type()));

    if( data == NULL )
    {
        data = GV_SHAPES(gv_shapes_new());
        gv_data_set_name( GV_DATA(data), "Currents" );
    }

    gv_shapes_layer_set_data( GV_SHAPES_LAYER(layer), data );
    gv_data_set_name( GV_DATA(layer), gv_data_get_name(GV_DATA(data)) );
    
    return GTK_OBJECT(layer);
}

/*******************************************************/

static void
app_cur_layer_setup(GvLayer *rlayer, GvViewArea *view)
{
}

static void 
app_cur_draw_arrow( gfloat base_x, gfloat base_y, 
                    gfloat head_x, gfloat head_y )

{
    gfloat     vx1, vy1, rl1, vx2, vy2, angle2;

    glBegin( GL_LINES );

    glVertex2f( head_x, head_y );
    glVertex2f( base_x, base_y );

    vx1 = base_x - head_x;
    vy1 = base_y - head_y;
    rl1 = sqrt(vx1*vx1+vy1*vy1);

    angle2 = atan2(vy1,vx1) + PI/4;
    vy2 = (rl1/3.0) * sin(angle2);
    vx2 = (rl1/3.0) * cos(angle2);
    
    glVertex2f( head_x, head_y );
    glVertex2f( head_x + vx2, head_y + vy2 );

    angle2 = atan2(vy1,vx1) - PI/4;
    vy2 = (rl1/3.0) * sin(angle2);
    vx2 = (rl1/3.0) * cos(angle2);
    
    glVertex2f( head_x, head_y );
    glVertex2f( head_x + vx2, head_y + vy2 );
    
    glEnd();
}

static void 
app_cur_layer_draw_arrow( GvViewArea *view, AppCurLayer *layer, 
                          GvShape *shape, int selected )

{
    gfloat       x, y, x_head, y_head;
    GLgeocoord   dx, dy;
    GvColor      color;
    GvShapeDrawInfo drawinfo;

    /* get and set color */
    gv_shapes_layer_get_draw_info( view, GV_SHAPES_LAYER(layer), &drawinfo );
    gv_color_copy( color, drawinfo.point_color );

    gv_shapes_layer_override_color( shape, color, "_gv_color" );

    glColor4fv( color );
    
    /* Compute main drawing location */
    x = gv_shape_get_x(shape, 0, 0 );
    y = gv_shape_get_y(shape, 0, 0 );

    /* Compute head location */
    
    x_head = x - 30;
    y_head = y + 20;

    app_cur_draw_arrow( x, y, x_head, y_head );

    /* compute selected box size */
    dx = 5.0;
    dy = 5.0;
    gv_view_area_correct_for_transform(view, dx, dy, &dx, &dy);    
    
    /* Draw selection box around base point, if selected */
    if( selected )
    {
        glBegin(GL_LINE_LOOP);
        glVertex2f( x-dx, y-dy );
        glVertex2f( x+dy, y-dx );
        glVertex2f( x+dx, y+dy );
        glVertex2f( x-dy, y+dx );
        glVertex2f( x-dx, y-dy );
        glEnd();
    }
}

static void
app_cur_layer_draw(GvLayer *rlayer, GvViewArea *view)
{
    AppCurLayer *layer = APP_CUR_LAYER(rlayer);
    gint i, points;
    gint *selected, presentation;
    gint hit_selected = FALSE;
    GvShape           *shape;
    
    presentation = GV_LAYER(layer)->presentation;
    selected = GV_SHAPE_LAYER_SELBUF(layer);

    points = gv_shapes_num_shapes(GV_SHAPES_LAYER(layer)->data);
    
    for (i=0; i < points; ++i)
    {
	if (selected[i] && !presentation)
	{
	    hit_selected = 1;
	    continue;
	}

        shape = gv_shapes_get_shape(GV_SHAPES_LAYER(layer)->data, i);
        if( shape != NULL && gv_shape_type(shape) == GVSHAPE_POINT )
            app_cur_layer_draw_arrow( view, layer, shape, FALSE );
    }

    if (hit_selected && ! GV_SHAPE_LAYER(layer)->flags & GV_DELAY_SELECTED)
    {
	app_cur_layer_draw_selected(GV_SHAPE_LAYER(layer), view);
    }     
}

static void
app_cur_layer_draw_selected(GvShapeLayer *rlayer, GvViewArea *view)
{
    AppCurLayer *layer = APP_CUR_LAYER(rlayer);
    gint i, points;
    gint *selected;
    GvShape           *shape;
    
    selected = GV_SHAPE_LAYER_SELBUF(layer);

    points = gv_shapes_num_shapes(GV_SHAPES_LAYER(layer)->data);
    
    for (i=0; i < points; ++i)
    {
	if ( !selected[i] )
	    continue;

        shape = gv_shapes_get_shape(GV_SHAPES_LAYER(layer)->data, i);
        if( shape != NULL && gv_shape_type(shape) == GVSHAPE_POINT )
            app_cur_layer_draw_arrow( view, layer, shape, TRUE );
    }
}


