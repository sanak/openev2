/******************************************************************************
 * $Id: ipgcplayer.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  InSAR Peppers
 * Purpose:  Implementation of GCP Layer.
 *
 *           This layer is intended specifically for use in InSAR Peppers, and
 *           should only be changed on behalf of Atlantis, though it can 
 *           serve as an example of custom layer drawing for others.
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
 * $Log: ipgcplayer.c,v $
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
 * Revision 1.15  2004/02/23 20:16:36  gmwalter
 * Checked in changes for insar.
 *
 * Revision 1.14  2003/12/10 15:29:42  gmwalter
 * Undid changes as requested.
 *
 * Revision 1.13  2003/12/06 22:23:22  gmwalter
 * Checked in changes made by insar group.
 *
 * Revision 1.12  2003/02/27 03:59:21  warmerda
 * added view to gv_shapes_layer_get_draw_info
 *
 * Revision 1.11  2002/11/05 04:05:32  warmerda
 * geocoord update
 *
 * Revision 1.10  2001/08/08 17:44:12  warmerda
 * use gv_shape_type() macro
 *
 * Revision 1.9  2001/06/15 15:30:29  warmerda
 * try setting glPointSize to 1 when drawing GCPs
 *
 * Revision 1.8  2000/09/18 21:12:43  srawlin
 * reduced icon size from 10 to 5.5
 *
 * Revision 1.7  2000/08/30 14:06:19  warmerda
 * make it easier to change symbol size
 *
 * Revision 1.6  2000/08/16 14:52:43  warmerda
 * added excluded support
 *
 * Revision 1.5  2000/08/15 20:09:31  warmerda
 * default color properly
 *
 * Revision 1.4  2000/08/14 19:45:21  warmerda
 * use geo_x, geo_y for G point
 *
 * Revision 1.3  2000/08/14 15:05:52  warmerda
 * updated substantially
 *
 * Revision 1.2  2000/08/04 14:14:12  warmerda
 * GvShapes shape ids now persistent
 *
 * Revision 1.1  2000/07/14 18:24:57  warmerda
 * New
 *
 */

#include <math.h>
#include <stdlib.h>
#include "ipgcplayer.h"
#include "gvutils.h"
#include <GL/gl.h>
#include <string.h>

/* Approximate size on screen of symbols in pixels */
#define SYMBOL_SIZE 5.5

static void ip_gcp_layer_class_init(IpGcpLayerClass *klass);
static void ip_gcp_layer_init(IpGcpLayer *layer);
static void ip_gcp_layer_setup(GvLayer *layer, GvViewArea *view);
static void ip_gcp_layer_draw(GvLayer *layer, GvViewArea *view);
static void ip_gcp_layer_draw_selected(GvShapeLayer *layer, 
                                          GvViewArea *view);

GtkType
ip_gcp_layer_get_type(void)
{
    static GtkType gcp_layer_type = 0;

    if (!gcp_layer_type)
    {
	static const GtkTypeInfo gcp_layer_info =
	{
	    "IpGcpLayer",
	    sizeof(IpGcpLayer),
	    sizeof(IpGcpLayerClass),
	    (GtkClassInitFunc) ip_gcp_layer_class_init,
	    (GtkObjectInitFunc) ip_gcp_layer_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	gcp_layer_type = gtk_type_unique(gv_shapes_layer_get_type(),
                                         &gcp_layer_info);
    }
    return gcp_layer_type;
}

static void
ip_gcp_layer_class_init(IpGcpLayerClass *klass)
{
    GvLayerClass *layer_class;
    GvShapeLayerClass *shape_layer_class;

    layer_class = (GvLayerClass*) klass;
    shape_layer_class = (GvShapeLayerClass*) klass;

    layer_class->setup = ip_gcp_layer_setup;
    layer_class->draw = ip_gcp_layer_draw;
    shape_layer_class->draw_selected = ip_gcp_layer_draw_selected;
}

static void
ip_gcp_layer_init(IpGcpLayer *layer)
{
}

GtkObject *
ip_gcp_layer_new(void)
{
    GvShapes *data;
    IpGcpLayer *layer = IP_GCP_LAYER(gtk_type_new(
	ip_gcp_layer_get_type()));

    data = GV_SHAPES(gv_shapes_new());
    gv_data_set_name( GV_DATA(data), "GCPs" );
    gv_shapes_layer_set_data( GV_SHAPES_LAYER(layer), data );
    gv_data_set_name( GV_DATA(layer), "GCPs" );
    
    return GTK_OBJECT(layer);
}

/*******************************************************/

static void
ip_gcp_layer_setup(GvLayer *rlayer, GvViewArea *view)
{
}

static void 
ip_gcp_draw_circle( gfloat center_x, gfloat center_y, gfloat radius )

{
    gvfloat angle;

#ifndef PI
#define PI  3.1415927
#endif

    glBegin( GL_LINE_LOOP );
    for( angle = 0.0; angle <= 2.00001*PI; angle += (PI/24.0) )
    {
        gfloat x, y;

        x = sin(angle) * radius + center_x;
        y = cos(angle) * radius + center_y;

        glVertex2f( x, y );
    }
    glEnd();
}

static void 
ip_gcp_layer_draw_gcp( GvViewArea *view, IpGcpLayer *layer, 
                       GvShape *shape, int selected )

{
    GvProperties *properties = gv_shape_get_properties(shape);
    const char *value;
    const char * type;
    gfloat       x, y, geo_x, geo_y, raw_x, raw_y;
    GLgeocoord   radius_x, radius_y;
    GvColor      color;
    GvShapeDrawInfo drawinfo;

    /* get and set color */
    gv_shapes_layer_get_draw_info( view, GV_SHAPES_LAYER(layer), &drawinfo );
    gv_color_copy( color, drawinfo.point_color );

    gv_shapes_layer_override_color( shape, color, "_gv_color" );

    glPointSize(1);
    glColor4fv( color );
    
    /* Compute main drawing location */
    x = gv_shape_get_x(shape, 0, 0 );
    y = gv_shape_get_y(shape, 0, 0 );

    radius_x = 1.0;
    radius_y = 0.0;
    gv_view_area_correct_for_transform( view, 
                                        radius_x, radius_y,
                                        &radius_x, &radius_y );
    radius_x = SYMBOL_SIZE * sqrt(radius_x*radius_x + radius_y*radius_y);

    /* fetch the georeferenced location in view coordinate system */
    value = gv_properties_get( properties, "geo_x" );
    if( value != NULL )
        geo_x = atof(value);
    else
        geo_x = x;

    value = gv_properties_get( properties, "geo_y" );
    if( value != NULL )
        geo_y = atof(value);
    else
        geo_y = y;

    /* fetch the features location in view coordinate system */
    value = gv_properties_get( properties, "raw_x" );
    if( value != NULL )
        raw_x = atof(value);
    else
        raw_x = x;

    value = gv_properties_get( properties, "raw_y" );
    if( value != NULL )
        raw_y = atof(value);
    else
        raw_y = y;

    /* draw based on type */
    type = gv_properties_get( properties, "gcp_type" );
    if( type == NULL )
        type = "F&G";

    value = gv_properties_get( properties, "excluded" );
    if( value != NULL && atoi(value) > 0 )
        type = "excluded";

    if( strcmp(type,"excluded") == 0 )
    {
        /* draw excluded X */
        glBegin(GL_LINES);
        glVertex2f( x-radius_x*0.707, y-radius_x*0.707 );
        glVertex2f( x+radius_x*0.707, y+radius_x*0.707 );
        glVertex2f( x+radius_x*0.707, y-radius_x*0.707 );
        glVertex2f( x-radius_x*0.707, y+radius_x*0.707 );
        glEnd();
    }


    if( strcmp(type,"G&E") == 0 )
    {
        /* draw triangle */
        glBegin(GL_LINE_LOOP);
        glVertex2f( geo_x, geo_y+radius_x );
        glVertex2f( geo_x-radius_x*0.866, geo_y-radius_x*0.5);
        glVertex2f( geo_x+radius_x*0.866, geo_y-radius_x*0.5);
        glVertex2f( geo_x, geo_y+radius_x );
        glEnd();

        ip_gcp_draw_circle( geo_x, geo_y, radius_x );
    }

    if( strcmp(type,"F&E") == 0 )
    {
        /* draw triangle */
        glBegin(GL_LINE_LOOP);
        glVertex2f( raw_x, raw_y+radius_x );
        glVertex2f( raw_x-radius_x*0.866, raw_y-radius_x*0.5);
        glVertex2f( raw_x+radius_x*0.866, raw_y-radius_x*0.5);
        glVertex2f( raw_x, raw_y+radius_x );
        glEnd();

        /* draw cross hair */
        glBegin(GL_LINES);
        glVertex2f( raw_x, raw_y-radius_x );
        glVertex2f( raw_x, raw_y+radius_x );
        glVertex2f( raw_x+radius_x, raw_y );
        glVertex2f( raw_x-radius_x, raw_y );
        glEnd();

    }

    if( strcmp(type,"F&G") == 0 )
    {
        /* draw cross hair */
        glBegin(GL_LINES);
        glVertex2f( raw_x, raw_y-radius_x );
        glVertex2f( raw_x, raw_y+radius_x );
        glVertex2f( raw_x+radius_x, raw_y );
        glVertex2f( raw_x-radius_x, raw_y );
        glEnd();

        ip_gcp_draw_circle( geo_x, geo_y, radius_x );

        /* connect with line */
        glBegin(GL_LINES);
        glVertex2f( raw_x, raw_y );
        glVertex2f( geo_x, geo_y );
        glEnd();
    }

    if( strcmp(type,"F&G&E") == 0 )
    {
        /* draw triangle */
        glBegin(GL_LINE_LOOP);
        glVertex2f( raw_x, raw_y+radius_x );
        glVertex2f( raw_x-radius_x*0.866, raw_y-radius_x*0.5);
        glVertex2f( raw_x+radius_x*0.866, raw_y-radius_x*0.5);
        glVertex2f( raw_x, raw_y+radius_x );
        glEnd();

        /* draw cross hair */
        glBegin(GL_LINES);
        glVertex2f( raw_x, raw_y-radius_x );
        glVertex2f( raw_x, raw_y+radius_x );
        glVertex2f( raw_x+radius_x, raw_y );
        glVertex2f( raw_x-radius_x, raw_y );
        glEnd();

        ip_gcp_draw_circle( geo_x, geo_y, radius_x );

        /* connect with line */
        glBegin(GL_LINES);
        glVertex2f( raw_x, raw_y );
        glVertex2f( geo_x, geo_y );
        glEnd();
    }

    /* Draw selection box around base point, if selected */
    if( selected )
    {
        /* draw triangle */
        glBegin(GL_LINE_LOOP);
        glVertex2f( x-radius_x*1.25, y-radius_x*1.25 );
        glVertex2f( x+radius_x*1.25, y-radius_x*1.25 );
        glVertex2f( x+radius_x*1.25, y+radius_x*1.25 );
        glVertex2f( x-radius_x*1.25, y+radius_x*1.25 );
        glVertex2f( x-radius_x*1.25, y-radius_x*1.25 );
        glEnd();
    }
}

static void
ip_gcp_layer_draw(GvLayer *rlayer, GvViewArea *view)
{
    IpGcpLayer *layer = IP_GCP_LAYER(rlayer);
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
            ip_gcp_layer_draw_gcp( view, layer, shape, FALSE );
    }

    if (hit_selected && ! GV_SHAPE_LAYER(layer)->flags & GV_DELAY_SELECTED)
    {
	ip_gcp_layer_draw_selected(GV_SHAPE_LAYER(layer), view);
    }     
}

static void
ip_gcp_layer_draw_selected(GvShapeLayer *rlayer, GvViewArea *view)
{
    IpGcpLayer *layer = IP_GCP_LAYER(rlayer);
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
            ip_gcp_layer_draw_gcp( view, layer, shape, TRUE );
    }
}


