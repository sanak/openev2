/******************************************************************************
 * $Id: gvpointtool.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Point editing mode.
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
 * $Log: gvpointtool.c,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:26  uid1026
 *
 * Imported sources
 *
 * Revision 1.11  2001/04/09 18:15:20  warmerda
 * improved warning
 *
 * Revision 1.10  2000/08/08 20:58:47  warmerda
 * recover from layer destruction
 *
 * Revision 1.9  2000/07/27 20:06:23  warmerda
 * added boundary constraints
 *
 * Revision 1.8  2000/07/24 14:00:24  warmerda
 * set_layer with NULL should be allowed
 *
 * Revision 1.7  2000/07/21 01:31:11  warmerda
 * added read_only flag for GvData, and utilize for vector layers
 *
 * Revision 1.6  2000/07/17 17:11:54  warmerda
 * added delete key support
 *
 * Revision 1.5  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvpointtool.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>

static void gv_point_tool_class_init(GvPointToolClass *klass);
static void gv_point_tool_init(GvPointTool *tool);
static void gv_point_tool_button_press(GvTool *tool, GdkEventButton *event);
static void gv_point_tool_key_press(GvTool *tool, GdkEventKey *event);
static void gv_point_tool_deactivate(GvTool *tool, GvViewArea *view);
static gint gv_point_tool_configure(GvTool *tool);

GtkType
gv_point_tool_get_type(void)
{
    static GtkType point_tool_type = 0;

    if (!point_tool_type)
    {
	static const GtkTypeInfo point_tool_info =
	{
	    "GvPointTool",
	    sizeof(GvPointTool),
	    sizeof(GvPointToolClass),
	    (GtkClassInitFunc) gv_point_tool_class_init,
	    (GtkObjectInitFunc) gv_point_tool_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	point_tool_type = gtk_type_unique(gv_tool_get_type(),
					  &point_tool_info);
    }
    return point_tool_type;
}

static void
gv_point_tool_class_init(GvPointToolClass *klass)
{
    GvToolClass *tool_class;

    tool_class = (GvToolClass*)klass;
    tool_class->deactivate = gv_point_tool_deactivate;
    tool_class->button_press = gv_point_tool_button_press;
    tool_class->key_press = gv_point_tool_key_press;
}

static void
gv_point_tool_init(GvPointTool *tool)
{
    GV_TOOL(tool)->cursor = gdk_cursor_new(GDK_TCROSS);
    tool->layer = NULL;
    tool->named_layer = NULL;
}

GvTool *
gv_point_tool_new(void)
{
    return GV_TOOL(gtk_type_new(GV_TYPE_POINT_TOOL));
}

static gint gv_point_tool_layer_destroy( GtkObject *layer, gpointer data )

{
    GvPointTool *tool = (GvPointTool *) data;

    if( tool->layer == GV_SHAPE_LAYER(layer) )
        gv_point_tool_set_layer( tool, NULL );
    
    return 0;
}

void
gv_point_tool_set_layer(GvPointTool *tool, GvShapeLayer *layer)
{
    if (GV_TOOL(tool)->view == NULL)
    {
	g_warning("gv_point_tool_set_layer(): inactive tool");
	return;
    }

    if( layer != NULL 
        && !GV_IS_POINT_LAYER(layer) && !GV_IS_SHAPES_LAYER(layer) )
    {
        g_warning( "gv_point_tool_set_layer(): not a point capable layer" );
        return;
    }

    if( layer != NULL && gv_data_is_read_only( GV_DATA(layer) ) )
    {
        g_warning( "gv_point_tool_set_layer(): layer is read-only" );
        return;
    }

    /* Disconnect from the previous layer */
    if (tool->layer)
    {
	gv_shape_layer_clear_selection(GV_SHAPE_LAYER(tool->layer));
	gv_view_area_queue_draw(GV_TOOL(tool)->view);
        gtk_signal_disconnect_by_data( GTK_OBJECT(tool->layer), 
                                       GTK_OBJECT(tool) );
    }
    
    tool->layer = layer;

    if (layer)
    {
	gv_view_area_set_active_layer(GV_TOOL(tool)->view, GTK_OBJECT(layer));
        gtk_signal_connect(
            GTK_OBJECT(layer), "destroy", 
            GTK_SIGNAL_FUNC(gv_point_tool_layer_destroy),
            GTK_OBJECT(tool));
    }
}

void
gv_point_tool_set_named_layer(GvPointTool *tool, gchar *name)
{
    if (tool->named_layer)
    {
	g_free(tool->named_layer);
	tool->named_layer = NULL;
    }
    if (name)
    {
	tool->named_layer = g_strdup(name);	
    }
    /* Tool layer will be updated next time it is configured */
}

static void
gv_point_tool_button_press(GvTool *tool, GdkEventButton *event)
{
    if (event->button == 1)
    {
	GvVertex vertex;
	
	if (!gv_point_tool_configure(tool)) return;

	/* Get pointer location */
	gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
				 &vertex.x, &vertex.y);

        if( !gv_tool_check_bounds( GV_TOOL(tool), vertex.x, vertex.y ) )
            return;

	/* Add a new point */
        if( GV_IS_POINT_LAYER(GV_POINT_TOOL(tool)->layer) )
        {
            gv_point_layer_select_new_point(
                GV_POINT_LAYER(GV_POINT_TOOL(tool)->layer), 
                &vertex);
        }
        else
        {
            GvShape   *new_point;

            new_point = gv_shape_new( GVSHAPE_POINT );
            gv_shape_set_xyz( new_point, 0, 0, vertex.x, vertex.y, 0.0 );
            gv_shapes_layer_select_new_shape( 
                GV_SHAPES_LAYER(GV_POINT_TOOL(tool)->layer), 
                new_point );
        }
    }
}

static void
gv_point_tool_key_press(GvTool *rtool, GdkEventKey *event)
{
    GvPointTool *tool = GV_POINT_TOOL(rtool);

    if (!gv_point_tool_configure(GV_TOOL(tool))) return;
    
    switch (event->keyval)
    {
	case GDK_Delete:
	case GDK_BackSpace:
	    /* Delete the currently selected lines (forces redraw) */
	    gv_shape_layer_delete_selected(tool->layer);
	    break;
    }
}

static void
gv_point_tool_deactivate(GvTool *r_tool, GvViewArea *view)
{
    GvPointTool  *tool = GV_POINT_TOOL(r_tool);

    /* Disconnect from layer */
    if (tool->layer)
    {
	gv_point_tool_set_layer(tool, NULL);
    }

    /* Call the parent class func */
    GV_TOOL_DEACTIVATE(tool, view);
}

static gint
gv_point_tool_configure(GvTool *r_tool)
{
    GvPointTool  *tool = GV_POINT_TOOL(r_tool);

    /* Check that we still are working on the active layer */
    if (!tool->layer ||	GTK_OBJECT(tool->layer) !=
	gv_view_area_active_layer(GV_TOOL(tool)->view))
    {
	GtkObject *layer;

	if (tool->named_layer)
	{
	    /* Look for named layer if given */
	    layer = gv_view_area_get_named_layer(GV_TOOL(tool)->view,
						 tool->named_layer);
	}
	else
	{
	    /* Attempt to find a point layer to edit */
	    layer = gv_view_area_get_layer_of_type(GV_TOOL(tool)->view,
						   GV_TYPE_POINT_LAYER,
                                                   FALSE);
            if(  layer == NULL )
                layer = gv_view_area_get_layer_of_type(GV_TOOL(tool)->view,
                                                       GV_TYPE_SHAPES_LAYER,
                                                       FALSE);
	}
	if (!layer)
	{
	    g_warning("gv_point_tool_configure(): no editable point layer in view");
	    return FALSE;
	}

	gv_point_tool_set_layer(tool, GV_SHAPE_LAYER(layer));
    }
    return tool->layer != NULL;
}
