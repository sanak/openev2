/******************************************************************************
 * $Id: testmain.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Sample C mainline with no python dependencies.
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
 * $Log: testmain.c,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:35  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:28  uid1026
 *
 * Imported sources
 *
 * Revision 1.33  2003/06/25 17:52:08  warmerda
 * added rotate tool
 *
 * Revision 1.32  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.31  2002/10/10 15:05:27  sduclos
 * add option [-ogr=<ogr_filename>]
 *
 * Revision 1.30  2002/09/30 20:05:18  warmerda
 * added support for shapefile
 *
 * Revision 1.29  2002/03/07 02:44:13  warmerda
 * updated add_height argument list
 *
 * Revision 1.28  2000/08/25 20:09:18  warmerda
 * improve testing if dataset raster fetch fails
 *
 * Revision 1.27  2000/08/18 20:34:53  warmerda
 * set initial window size
 *
 * Revision 1.26  2000/08/16 14:07:47  warmerda
 * added prototype scrollbar support
 *
 * Revision 1.25  2000/07/17 20:20:50  warmerda
 * stop using old style vector layers
 *
 * Revision 1.24  2000/07/12 16:21:53  warmerda
 * bail on failure to create view
 *
 * Revision 1.23  2000/06/26 15:13:33  warmerda
 * use manager for getting datasets
 *
 * Revision 1.22  2000/06/23 12:56:53  warmerda
 * added multiple GvRasterSource support
 *
 * Revision 1.21  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include <gtk/gtk.h>
#include <gtk/gtkscrolledwindow.h>
#include "gview.h"
#include "gextra.h"
#include "cpl_conv.h"
#include <GL/glu.h>

GvToolbox *toolbox;
GvViewLink *vlink;

static void key_press_cb( GtkObject * object, GdkEventKey * event )

{
    GvViewArea     *view = GV_VIEW_AREA(object);

    if( event->keyval == 't' )
    {
        double    start_time = g_get_current_time_as_double();
        double    end_time, spf;
        int       i, frame_count = 20;

        for( i = 0; i < frame_count; i++ )
            gv_view_area_expose(GTK_WIDGET(view), NULL);

        end_time =  g_get_current_time_as_double();

        spf = (end_time - start_time) / frame_count;
        printf( "Speed is %.2ffps\n", 1.0 / spf );
    }
}

GtkWidget *
create_view(GvShapes *shapes, GvRaster *raster,  GvRaster *height)
{
    GtkWidget *win;
    GtkWidget *view;
    GtkWidget *swin;

    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    view = gv_view_area_new();
    if( view == NULL )
        return NULL;

    gtk_window_set_default_size( GTK_WINDOW(win), 512, 512 );

    /* Set mode to 2D or 3D */
    if ( height == NULL)
    {
        gv_view_area_set_mode(GV_VIEW_AREA(view), 0);
    } else {
        gv_view_area_set_mode(GV_VIEW_AREA(view), 1);
    }

    gtk_drawing_area_size(GTK_DRAWING_AREA(view), 500, 500);

    swin = gtk_scrolled_window_new(NULL, NULL);

    gtk_container_add(GTK_CONTAINER(win), swin);    

    gtk_container_add(GTK_CONTAINER(swin), view);
    

    if( raster != NULL )
    {
        GtkObject *raster_layer;
        raster_layer = gv_raster_layer_new(GV_RLM_AUTO, raster, NULL);

        gv_view_area_add_layer(GV_VIEW_AREA(view), 
                               raster_layer);

        if( height != NULL)
        {
            gv_mesh_add_height(GV_RASTER_LAYER(raster_layer)->mesh, height,
                               0.0);
        }
    }   

    gv_view_area_add_layer(GV_VIEW_AREA(view), gv_shapes_layer_new(shapes));

    gtk_signal_connect_object(GTK_OBJECT(view), "key-press-event", 
                              GTK_SIGNAL_FUNC(key_press_cb), 
                              GTK_OBJECT(view) );

    gtk_widget_show(view);
    gtk_widget_show(swin);
    gtk_widget_show(win);
    gtk_widget_grab_focus(view);

    gtk_signal_connect(GTK_OBJECT(win), "delete-event",
		       GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
    
    gtk_quit_add_destroy(1, GTK_OBJECT(win));

    return view;
}

void
toolbar_callback(GtkWidget *widget, gpointer data)
{
    gv_toolbox_activate_tool(toolbox, (gchar*)data);
}

void
link_callback(GtkWidget *widget, gpointer data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
    {
	gv_view_link_enable(vlink);
    }
    else
    {
	gv_view_link_disable(vlink);
    }
}

void
create_toolbar()
{
    GtkWidget *win;
    GtkWidget *toolbar;
    GtkWidget *but;

    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* GTK2 PORT... 
    toolbar = gtk_toolbar_new(GTK_ORIENTATION_VERTICAL, GTK_TOOLBAR_TEXT);
    */

    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar), GTK_ORIENTATION_VERTICAL);
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_TEXT);

    gtk_container_add(GTK_CONTAINER(win), toolbar);

    but = 
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
				   GTK_TOOLBAR_CHILD_RADIOBUTTON,
				   NULL,
				   "Zoom",
				   "Zoom tool",
				   NULL,
				   NULL,
				   GTK_SIGNAL_FUNC(toolbar_callback),
				   (void *)"zoompan");

    but = 
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
				   GTK_TOOLBAR_CHILD_RADIOBUTTON,
				   but,
				   "Select",
				   "Selection tool",
				   NULL,
				   NULL,
				   GTK_SIGNAL_FUNC(toolbar_callback),
				   (void *)"select");

    but = 
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
				   GTK_TOOLBAR_CHILD_RADIOBUTTON,
				   but,
				   "Draw Points",
				   "Point drawing tool",
				   NULL,
				   NULL,
				   GTK_SIGNAL_FUNC(toolbar_callback),
				   (void *)"point");
    but = 
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
				   GTK_TOOLBAR_CHILD_RADIOBUTTON,
				   but,
				   "Draw Line",
				   "Line drawing tool",
				   NULL,
				   NULL,
				   GTK_SIGNAL_FUNC(toolbar_callback),
				   (void *)"line");
    but = 
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
				   GTK_TOOLBAR_CHILD_RADIOBUTTON,
				   but,
				   "Rotate/Resize",
				   "Rotate/resize tool",
				   NULL,
				   NULL,
				   GTK_SIGNAL_FUNC(toolbar_callback),
				   (void *)"rotate");
    but = 
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
				   GTK_TOOLBAR_CHILD_RADIOBUTTON,
				   but,
				   "Draw Area",
				   "Area drawing tool",
				   NULL,
				   NULL,
				   GTK_SIGNAL_FUNC(toolbar_callback),
				   (void *)"area");

    but = 
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
				   GTK_TOOLBAR_CHILD_RADIOBUTTON,
				   but,
				   "Edit Node",
				   "Node edit tool",
				   NULL,
				   NULL,
				   GTK_SIGNAL_FUNC(toolbar_callback),
				   (void *)"node");
    but = 
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
				   GTK_TOOLBAR_CHILD_RADIOBUTTON,
				   but,
				   "Draw ROI",
				   "ROI drawing tool",
				   NULL,
				   NULL,
				   GTK_SIGNAL_FUNC(toolbar_callback),
				   (void *)"roi");
    but = 
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
				   GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
				   NULL,
				   "Link",
				   "Link views together",
				   NULL,
				   NULL,
				   GTK_SIGNAL_FUNC(link_callback),
				   NULL);

    gtk_signal_connect(GTK_OBJECT(win), "delete-event",
		       GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
    
    gtk_widget_show(toolbar);
    gtk_widget_show(win);	
}

static 
GvData *gv_raster_new_from_name( const char * filename )
{
    static int gdal_is_initialized = 0;
    GDALDatasetH  dataset;

    if( !gdal_is_initialized )
    {
        gdal_is_initialized = TRUE;
        GDALAllRegister();
    }

    dataset = gv_manager_get_dataset( gv_get_manager(), filename );
    if( dataset == NULL )
        return NULL;
    else
    {
        GvRaster *raster;

        raster = gv_manager_get_dataset_raster( gv_get_manager(), 
                                                dataset, 1);
        if( raster == NULL )
            return NULL;
        else
            return GV_DATA(raster);
    }
}

static void _load_ogr(GtkWidget *view, char *filename)
{
    GvData     *raw_data;
    GvShapes   *shape_data;
    GtkObject  *layer;

    int index = 0;

    while (NULL != (raw_data = gv_shapes_from_ogr(filename, index++))){

        if (NULL == (shape_data = GV_SHAPES(raw_data))){
            printf( __FILE__ "_load_ogr(): error loading layer no: %i\n", index);
            return;
        }

        gv_undo_register_data(GV_DATA(shape_data));

        layer = gv_shapes_layer_new( shape_data );

        gv_view_area_add_layer(GV_VIEW_AREA(view), layer);
        gv_view_area_set_active_layer(GV_VIEW_AREA(view), layer);
    }

    return;
}

void 
Usage()

{
    printf( "Usage: gvtest [-2] [-ogr=<ogr_filename>] [shapefile ...] [raster_filename [dem_filename]]\n" );
    exit( 0 );
}

int
main(int argc, char **argv)
{
    GtkWidget *view;
    GvData *shapes;
    GvRaster *raster = NULL;
    GvRaster *dem = NULL;
    int two_views = FALSE;
    char *raster_filename = NULL;
    char *dem_filename = NULL;
    char *shapefile = NULL;
    char *ogr_filename = NULL;
    int   i;

    gtk_init(&argc, &argv);
    
    /* ---- Init GtkGLExt ---- */
    gtk_gl_init (&argc, &argv);

    for( i = 1; i < argc; i++ )
    {
        if( strcmp(argv[i],"-1") == 0 )
            /* normal case */;
        else if( strcmp(argv[i],"-2") == 0 )
            two_views = TRUE;
        else if( strncmp(argv[i], "-ogr=", 5) == 0)
            ogr_filename = argv[i] + 5;
        else if( argv[i][0] == '-' )
            Usage();
        else if( EQUAL(CPLGetExtension(argv[i]),"shp") )
            shapefile = argv[i];
        else if( raster_filename == NULL )
            raster_filename = argv[i];
        else if( dem_filename == NULL )
            dem_filename = argv[i];
        else
            Usage();
    }

    toolbox = GV_TOOLBOX(gv_toolbox_new());
    gv_toolbox_add_tool(toolbox, "select", gv_selection_tool_new());
    gv_toolbox_add_tool(toolbox, "zoompan", gv_zoompan_tool_new());
    gv_toolbox_add_tool(toolbox, "point", gv_point_tool_new());
    gv_toolbox_add_tool(toolbox, "line", gv_line_tool_new());
    gv_toolbox_add_tool(toolbox, "rotate", gv_rotate_tool_new());
    gv_toolbox_add_tool(toolbox, "area", gv_area_tool_new());
    gv_toolbox_add_tool(toolbox, "node", gv_node_tool_new());
    gv_toolbox_add_tool(toolbox, "roi", gv_roi_tool_new());

    vlink = GV_VIEW_LINK(gv_view_link_new());

    if( shapefile == NULL )
    {
        shapes = gv_shapes_new();
    }
    else
    {
        shapes = gv_shapes_from_shapefile(shapefile);
    }

    if( raster_filename != NULL )
    {
        GvData *data;
        GvData *dem_data;

        data = gv_raster_new_from_name( raster_filename );

        if( data != NULL )
            raster = GV_RASTER(data);

        /* add height */
        if (argc > 3)
        {
            dem_data = gv_raster_new_from_name( dem_filename );
            
            if( dem_data != NULL )
                dem = GV_RASTER(dem_data);
        }
    }

    gv_undo_register_data(shapes);

    view = create_view(GV_SHAPES(shapes), raster, dem );

    if( view == NULL )
    {
        fprintf( stderr, "Unable to create view.  Is OpenGL working?\n" );
        exit( 100 );
    }

    if (ogr_filename != NULL) {
        _load_ogr(view, ogr_filename);
    }
    gv_tool_activate(GV_TOOL(toolbox), GV_VIEW_AREA(view));
    gv_view_link_register_view(vlink, GV_VIEW_AREA(view));
    gv_toolbox_activate_tool( toolbox, "zoompan" );

    if (two_views)
    {
	view = create_view(GV_SHAPES(shapes), raster, dem );
	gv_tool_activate(GV_TOOL(toolbox), GV_VIEW_AREA(view));
	gv_view_link_register_view(vlink, GV_VIEW_AREA(view));
    }

    create_toolbar();

    gtk_main();

    return 0;
}
