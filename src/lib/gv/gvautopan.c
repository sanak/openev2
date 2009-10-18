/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Auto-panning tool.  Scans systematically over an area or along
 *           a preset path.
 * Author:   Gillian Walter
 *
 * Developed by Atlantis Scientific Inc. (www.atlantis-scientific.com) for
 * DRDC Ottawa.
 *
 ******************************************************************************
 * Copyright (c) Her majesty the Queen in right of Canada as represented
 * by the Minister of National Defence, 2004.
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
 * $Log: gvautopan.c,v $
 * Revision 1.1.2.10  2005/09/12 15:32:12  gmwalter
 * Update autopan tool for line paths.
 *
 * Revision 1.1.2.9  2005/02/04 19:18:45  gmwalter
 * Updated rectangle merging.
 *
 * Revision 1.1.2.8  2005/01/31 21:17:16  gmwalter
 * First pass at merging rectangles.
 *
 * Revision 1.1.2.7  2005/01/29 04:01:11  gmwalter
 * Initial support for autopan trail.
 *
 * Revision 1.1.2.6  2005/01/21 15:38:53  gmwalter
 * Fix some problems with speed.
 *
 * Revision 1.1.2.5  2005/01/20 15:20:07  gmwalter
 * Add ability to change standard path.
 *
 * Revision 1.1.2.4  2005/01/17 21:27:19  gmwalter
 * Add ability to cope with raw/georef
 * mismatch beteen main and secondary
 * view.
 *
 * Revision 1.1.2.3  2005/01/05 21:22:22  gmwalter
 * Updated autopan tool to add more functions.
 *
 * Revision 1.1.2.2  2004/12/21 15:10:26  gmwalter
 * Add ability to relocate zoomed region by
 * dragging in secondary view.
 *
 * Revision 1.1.2.1  2004/12/09 16:58:28  gmwalter
 * Add initial autopan support.
 *
 *
 *
 */

/* TODO:
 * - If more than one overview is ever used and cache becomes a problem, may want
 *   to consider context-sharing textures instead of different textures for each
 *   secondary view.  Might also want to consider resident textures, though not
 *   sure what this would do to the performance of the rest of the application...
 * - need to test with main view in georef mode and overview window in pixel/line
 *   mode, and vice versa.
 * - extension: reset high-res window zoom level from overview window (resize)
 */

/* Notes:
 * - It should be possible to have more than one overview window, with trails, but
 *   this has never been tested, so likely bugs would need to be worked out if
 *   anyone actually tried this...
 * - When overlap and boundaries are set, the tool will not follow them exactly
 *   because in order to have constant speed, it must step over regions of
 *   constant size.  Boundaries and overlap are rounded to the nearest allowable
 *   increment.  Likewise, with repositioning, zoom area will snap to the closest
 *   valid location.
 *
 * - Currently all translated locations (centers) are pre-calculated and stored
 *   in the "centers" array.  This might occupy too much memory for large panning
 *   areas at high resolution.  If this turns out to be the case, the
 *   compute_locations_and_zoom function should be replaced with a new function
 *   that gets called at every zoom iteration and avoids storage.
 *
 */

#include "gvautopan.h"

#include <stdio.h>
#include <math.h>

#define LOG2(x)         (log(x) / 0.69314718056)

#define PICK_SIZE 6.0

#ifndef MAX
#define MAX(a,b) ((a>b) ? a:b)
#endif
#ifndef MIN
#define MIN(a,b) ((a<b) ? a:b)
#endif

enum
{
    PICK_NONE = 0,
    PICK_TOP,
    PICK_RIGHT,
    PICK_BOTTOM,
    PICK_LEFT,
    PICK_CENTER
};

/* Ideas for future:
 *
 * - Ability to step between frames instead of panning
 *
 */


/* Signals */
enum
{
    ZOOMEXTENTS_CHANGED,  /* for when extents are reset by secondary views */
    ZOOMEXTENTS_CHANGING,
    LAST_SIGNAL
};

static void gv_autopan_tool_class_init(GvAutopanToolClass *klass);
static void gv_autopan_tool_init(GvAutopanTool *tool);

static void gv_autopan_tool_activate(GvTool *tool, GvViewArea *view);
static void gv_autopan_tool_deactivate(GvTool *tool, GvViewArea *view);
static void gv_autopan_tool_dispose(GObject *object);
static void gv_autopan_tool_finalize(GObject *object);

static void gv_autopan_tool_sv_draw(GvTool *tool, GvViewArea *view);

static gboolean gv_autopan_tool_sv_button_press(GtkWidget *view,
                                            GdkEventButton *event,
                                            gpointer *data_tool);

static gboolean gv_autopan_tool_sv_button_release(GtkWidget *view,
                                            GdkEventButton *event,
                                            gpointer *data_tool);

static gboolean gv_autopan_tool_sv_motion_notify(GtkWidget *view,
                                            GdkEventMotion *event,
                                            gpointer *data_tool);
static gint
gv_autopan_tool_pick(GvAutopanTool *tool, GvViewArea *view,
                     gvgeocoord x, gvgeocoord y);

static gint gv_autopan_tool_quit(gpointer data);

static void compute_locations_and_zoom(GvAutopanTool *tool);

static gint map_view_to_view_xy(GvViewArea *view1, GvViewArea *view2, gvgeocoord *x, gvgeocoord *y);

static void new_trail_tile( GvAutopanTool *tool, gint xindex, gint yindex);
static void update_trail( GvAutopanTool *tool, gvgeocoord xmin, gvgeocoord ymin,
                          gvgeocoord width, gvgeocoord height );

static void create_trail_textures( GvAutopanTool *tool, GvAutopanViewItem *item );
static void delete_trail_textures(GvAutopanViewItem *item);



static GvToolClass *parent_class = NULL;
static guint autopantool_signals[LAST_SIGNAL] = { 0 };

GType
gv_autopan_tool_get_type(void)
{
    static GType autopan_tool_type = 0;

    if (!autopan_tool_type)
    {
    	static const GTypeInfo autopan_tool_info =
			{
				sizeof(GvAutopanToolClass),
				(GBaseInitFunc) NULL,
				(GBaseFinalizeFunc)NULL,
				(GClassInitFunc) gv_autopan_tool_class_init,
				/* reserved_1 */ NULL,
				/* reserved_2 */ NULL,
				sizeof(GvAutopanTool),
				0,
				(GInstanceInitFunc) gv_autopan_tool_init,
			};

	autopan_tool_type = g_type_register_static (GV_TYPE_TOOL,
	                                            "GvAutopanTool",
	                                            &autopan_tool_info, 0);

    }
    return autopan_tool_type;
}

static void
gv_autopan_tool_class_init(GvAutopanToolClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GvToolClass *tool_class = GV_TOOL_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    autopantool_signals[ZOOMEXTENTS_CHANGED] =
    	g_signal_new ("zoomextents_changed",
    			G_TYPE_FROM_CLASS (klass),
    			G_SIGNAL_RUN_FIRST |G_SIGNAL_ACTION,
    			G_STRUCT_OFFSET (GvAutopanToolClass, zoomextents_changed),
    			NULL, NULL,
    			g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
    			G_TYPE_POINTER);

    autopantool_signals[ZOOMEXTENTS_CHANGING] =
    	g_signal_new ("zoomextents_changing",
    			G_TYPE_FROM_CLASS (klass),
    			G_SIGNAL_RUN_FIRST |G_SIGNAL_ACTION,
    			G_STRUCT_OFFSET (GvAutopanToolClass, zoomextents_changing),
    			NULL, NULL,
    			g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
    			G_TYPE_POINTER);

    object_class->dispose = gv_autopan_tool_dispose;
    object_class->finalize = gv_autopan_tool_finalize;

    klass->zoomextents_changed = NULL;
    klass->zoomextents_changing = NULL;

    tool_class->activate = gv_autopan_tool_activate;
    tool_class->deactivate = gv_autopan_tool_deactivate;

    /* NOTE: Currently, no gtk events are connected to, so
     *       the parent gvtool deactivation function is not
     *       called in gv_autopan_tool_deactivate.  If any
     *       gtk events do end up being connected to, then
     *       gv_autopan_tool_deactivate should be updated to
     *       uncomment this call (it was commented out because
     *       gtk gives a warning message if you disconnect
     *       and there were no handlers connected)
     */
}

static void
gv_autopan_tool_init(GvAutopanTool *tool)
{
    int i;

    tool->path_type = TL_R_D_L_D;

    tool->nonstandard_path_shapes = NULL;

    tool->play_flag = 0;

    tool->win_width = 0;
    tool->win_height = 0;
    tool->win_zoom = 0;

    tool->pan_region.x = 0;
    tool->pan_region.y = 0;
    tool->pan_region.width = -1;
    tool->pan_region.height = -1;

    tool->overlap = 0.1;
    tool->speed = 0.01;

    tool->block_size_mode = 0;
    tool->block_x_size = 0;
    tool->resolution = 0;

    tool->centers = NULL;
    tool->view_items = NULL;
    tool->num_views = 0;

    tool->trail_overview_region.x = 0;
    tool->trail_overview_region.y = 0;
    tool->trail_overview_region.width = -1;
    tool->trail_overview_region.height = -1;

    tool->trail_overview_width_pixels = 256;
    tool->trail_tile_pixels = 256;
    tool->trail_tile_lines = 256;

    tool->trail_x0 = 0.0;
    tool->trail_y0 = 0.0;
    tool->trail_tile_xsize = 0.0;
    tool->trail_tile_ysize = 0.0;

    tool->trail = NULL;
    tool->num_trail_tiles = 0;
    tool->trail_mode = 0;
    tool->trail_block = g_malloc(
                    tool->trail_tile_pixels*tool->trail_tile_lines*2);

    for (i=0; i<tool->trail_tile_pixels*tool->trail_tile_lines*2; i++)
        ((unsigned char *) tool->trail_block)[i] = 255;

    /* make sure panning stops when application is signalled to quit- */
    /* gtk won't quit until play function exits */
    //tool->quit_handler_id = gtk_quit_add(0, gv_autopan_tool_quit, (gpointer) tool);
}

GvTool *
gv_autopan_tool_new(void)
{
    GvAutopanTool *tool = g_object_new(GV_TYPE_AUTOPAN_TOOL, NULL);

    return GV_TOOL(tool);
}

/**************************************************************/


static void
gv_autopan_tool_activate(GvTool *rtool, GvViewArea *view)
{
    GvAutopanTool *tool = GV_AUTOPAN_TOOL(rtool);

    /* Call the parent class func */
    GV_TOOL_ACTIVATE(tool, view);

    /* TODO:
     * - get current view extents
     * - store window size parameters
     * - calculate locations for default speed, block size,
     *   overlap.
     * - calculate view zoom level
     * - connect view window's resize event to resize function
     * - activate secondary view signals, if there are any
     */
    gv_view_area_get_extents(view,
                             &tool->pan_region.x,
                             &tool->pan_region.y,
                             &tool->pan_region.width,
                             &tool->pan_region.height);
    tool->pan_region.width -= tool->pan_region.x;
    tool->pan_region.height -= tool->pan_region.y;

    tool->win_width = view->state.shape_x;
    tool->win_height = view->state.shape_y;

    /* default: use first standard path, 10% overlap at each
     * edge between successive passes, zoomed in at 8 times
     * the regular extents.
     */
    tool->block_size_mode = 0;
    tool->block_x_size = 1.0/8.0;

    tool->overlap = 0.1;
    tool->speed = 0.01;

    tool->current_index = 0;
}



static void
gv_autopan_tool_deactivate(GvTool *rtool, GvViewArea *view)
{
    GvAutopanTool *tool = GV_AUTOPAN_TOOL(rtool);
    GvAutopanViewItem *sv;
    int sv_idx;

    //if view has already been deactivated, print a warning and return
    g_return_if_fail (rtool->view != NULL);

    /* Call the parent class func- uncomment the line below
     * and remove the 3 lines following it if any gtk events
     * are connected to in future.
     */
    GV_TOOL_DEACTIVATE(tool, view);
    //g_return_if_fail(GV_TOOL(tool)->view == view);
    //GV_TOOL(tool)->view = NULL;
    //g_object_unref(view);

    gv_autopan_tool_stop(tool);
    if (tool->centers != NULL)
    {
        g_array_free(tool->centers, TRUE);
        tool->centers = NULL;
    }

    /* TODO:
     * - clear extent settings, zoom, block size
     * - disconnect secondary views
     */

    gv_view_area_queue_draw(view);
    if (tool->num_views > 0)
    {
        for (sv_idx = tool->num_views-1; sv_idx >= 0; sv_idx--)
        {
            sv = &(g_array_index(tool->view_items, GvAutopanViewItem,
                                 sv_idx));
            gv_autopan_tool_remove_view( tool, sv->view);
        }
    }
}

/*Dispose is called by the GObject infrastructure when the object reference count reaches zero.
 * All other object references need to be unref-ed here.
 * Any function may still be called after this so the functions should not assume the tool is activated.
 */
static void gv_autopan_tool_dispose(GObject *object)
{
	int i;
	GvTool *tool = GV_TOOL(object);
	GvAutopanTool *atool = GV_AUTOPAN_TOOL(object);
	GvAutopanViewItem *sv;

	//deactivate the tool if there is a view attached
	//this will unref the activated view and all the secondary views.
	if( tool->view != NULL )
	{
		gv_autopan_tool_deactivate(tool, tool->view);
	}
	//Secondary views will only be unref-ed if there is a main view activated
	//this should always be the case i think.  Is it possible to register secondary views without activating a main view??
	//clean up to prevent memory/reference leak
	if (atool->view_items != NULL)
	{
		//warn if we get here because I think this should not be possible.
		g_warning("gv_autopan_tool_dispose(): Secondary views should been removed. Cleaning up now.");
		//clean up just in case - remove all the views
        for (i = atool->view_items->len-1; i >= 0; i--)
        {
            sv = &(g_array_index(atool->view_items, GvAutopanViewItem, i));
            gv_autopan_tool_remove_view( atool, sv->view);
        }
	}

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->dispose(object);
}

/*Finalize is called by the GObject infrastructure when the object is being destroyed.
 * All memory should be freed here.
 */
static void gv_autopan_tool_finalize(GObject *object)
{
	//free memory associated with the trail
    gv_autopan_tool_clear_trail(GV_AUTOPAN_TOOL(object));
    g_free(GV_AUTOPAN_TOOL(object)->trail_block);
    //gtk_quit_remove(GV_AUTOPAN_TOOL(object)->quit_handler_id);

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->finalize(object);
}

/* Start panning */
gint gv_autopan_tool_play(GvAutopanTool *tool)
{
    /* TODO:
     * - Set zoom level and initial translation
     * - While tool is not paused:
     *    - translate to next position in list.
     *    - If secondary views are present, trigger
     *      a redraw (maybe not every time- if not
     *      then make sure one is at least triggered
     *      at the end
     *    - do a while loop of gtk_main_iterations
     *      on gtk_events_pending, limited by a timer.
     *      If panning has reached the very end, maybe
     *      allow a little more time?
     *      Is there any way to set a priority on
     *      an event (eg. if left click used to pause)?
     */
    GvVertex3d *nloc;
    GTimer *timer;
    int sv_idx;
    GvAutopanViewItem *sv;
    gvgeocoord xmin, ymin, xmax, ymax;

    /* Avoid recursive plays */
    if (tool->play_flag == 1)
        return TRUE;

    //the tool must be activated with a view before play
    g_return_val_if_fail(GV_TOOL(tool)->view != NULL, FALSE);

    /* If tool is not just paused, compute zoom locations */
    if (tool->play_flag == 0)
    {
        compute_locations_and_zoom(tool);

        if (tool->num_centers < 1)
            return FALSE;

        gv_view_area_zoom(GV_TOOL(tool)->view,
                      -1*gv_view_area_get_zoom(GV_TOOL(tool)->view));
        gv_view_area_zoom(GV_TOOL(tool)->view, tool->win_zoom);
    }
    else if (tool->num_centers < 1)
        return FALSE;

    tool->play_flag = 1;
    while (tool->play_flag == 1)
    {
        /* check if user has resized window; update if they have */
        if ((tool->win_width != GV_TOOL(tool)->view->state.shape_x) ||
            (tool->win_height != GV_TOOL(tool)->view->state.shape_y) ||
            (tool->centers->len == 0))	//also need to recompute if the array has been cleared - the code below assumes the array is not empty
            compute_locations_and_zoom(tool);

        /* check if user has zoomed; reset if they have */
        if (gv_view_area_get_zoom(GV_TOOL(tool)->view) != tool->win_zoom)
        {
            gv_view_area_zoom(GV_TOOL(tool)->view,
                  -1*gv_view_area_get_zoom(GV_TOOL(tool)->view));
            gv_view_area_zoom(GV_TOOL(tool)->view,tool->win_zoom);
        }

        //this assumes that tool->centers is not empty and that tool->current_index is a valid index
        g_assert(tool->current_index < tool->centers->len);

        nloc = &( g_array_index( tool->centers, GvVertex3d,
                            tool->current_index) );

        g_assert(nloc != NULL);

        gv_view_area_set_translation(GV_TOOL(tool)->view, nloc->x, nloc->y);

        if ( tool->trail_mode > 0 )
        {
            gv_view_area_get_extents( GV_TOOL(tool)->view, &xmin, &ymin,
                                      &xmax, &ymax );
            update_trail( tool, xmin, ymin, xmax-xmin, ymax-ymin );
        }

        if (tool->speed > 0)
        {
            tool->current_index++;
            if (tool->current_index >= tool->num_centers)
                tool->current_index = 0;
        }
        else
        {
            tool->current_index--;
            if (tool->current_index < 0)
                tool->current_index = tool->num_centers-1;

        }

        if (tool->num_views > 0)
        {
            for (sv_idx = 0; sv_idx < tool->num_views; sv_idx++)
            {
                sv = &(g_array_index(tool->view_items, GvAutopanViewItem,
                                       sv_idx));
                gv_view_area_queue_draw(sv->view);
            }
        }

		//this is quite the hack.  Used to be that gtk_quit_add() was used to register a function to be called when the main loop exits.
        //Now, with the GTK2 port, this doesn't work anymore and I don't know which signals will be emitted when the application exits.
        //For now, this just checks if the main view still has an active_layer (active_layer gets set to NULL in gv_view_area_destroy())
        //It works so far, I hope it doesn't cause any wierd crashes.
        //Without these lines, the autopan tool will continue to play after the main application exits.
        if(! GV_TOOL(tool)->view->active_layer)
        	break;

        /* continue to be interactive */
        timer = g_timer_new();
        g_timer_start(timer);
        //run the main iteration (to handle any user input), passing FALSE means the function will not be blocking
        //i guess it will take control back after 2 seconds or if there are no more events waiting to be handled
    	//if called from a signal handler, need to release the thread lock because g_main_context_iteration will block to acquire it
        gdk_threads_leave();
        while( g_timer_elapsed(timer,NULL) < 2.0 && g_main_context_iteration(NULL, FALSE) );
        gdk_threads_enter();
        g_timer_destroy(timer);

        //note: now that the main iteration has run, any function could have been called so no assumptions can be made here
        //ex. if deactivate was called, tool->view maybe be NULL
        //its probably best not to put any code here and hopefully the play_flag has been set properly
        //to avoid problems on the next iteration
        //this whole tool needs a re-write.
    }

    return TRUE;
}

/* Pause the tool, but maintain current position, resolution, and
 * speed settings.
 */

static gint gv_autopan_tool_quit(gpointer data)
{
    gv_autopan_tool_stop(GV_AUTOPAN_TOOL(data));
    return 0;
}

gint gv_autopan_tool_pause(GvAutopanTool *tool)
{

    if (tool->play_flag == 0)
        compute_locations_and_zoom(tool);

    tool->play_flag = 2;

    return TRUE;
}

gint gv_autopan_tool_stop(GvAutopanTool *tool)
{
    int sv_idx;
    GvAutopanViewItem *sv;

    if (tool->play_flag == 0)
        return TRUE;

    /* Same as pause, except that secondary views will be
     * refreshed without the current region drawn.
     */
    if (tool->num_views > 0)
    {
        /* If tool is not playing, force redraw of secondary views */
        for (sv_idx = 0; sv_idx < tool->num_views; sv_idx++)
        {
            sv = &(g_array_index(tool->view_items, GvAutopanViewItem,
                                   sv_idx));
            gv_view_area_queue_draw(sv->view);
        }
    }

    if (tool->centers != NULL)
    {
        g_array_free(tool->centers, TRUE);
        tool->centers = NULL;
        tool->current_index = 0;
    }
    tool->play_flag = 0;

    return TRUE;
}

/* Speed is adjusted by placing successive locations closer or
 * further apart in the direction of panning.  Function returns
 * the final speed setting (speed is forced to be between
 * -1.0 and 1.0, with an absolute value of at least 0.000001).
 */
double gv_autopan_tool_set_speed(GvAutopanTool *tool, gvgeocoord speed)
{
    /* TODO:
     * - if playing, set pause flag
     * - recalculate locations and zoom, and reset current position
     *   to equivalent point in new list
     * - if tool was playing, restart
     * - warn if speed is out of bounds
     */
    if (tool->speed == speed)
        return tool->speed;

    if (tool->speed == -1*speed)
    {
        tool->speed = speed;
        return tool->speed;
    }

    if (speed > 0)
    {
        if (speed < 0.000001)
            tool->speed = 0.000001;
        else if (speed > 1.0)
            tool->speed = 1.0;
        else
            tool->speed = speed;
    }
    else
    {
        if (speed < -1.0)
            tool->speed = -1.0;
        else if (speed > -0.000001)
            tool->speed = -0.000001;
        else
            tool->speed = speed;
    }

    /* If tool is stopped, don't recompute locations just yet in
     * case other things (eg. extents) also need to be reset.
     */
    if (tool->play_flag != 0)
        compute_locations_and_zoom(tool);

    return (double) tool->speed;
}

double gv_autopan_tool_get_speed(GvAutopanTool *tool)
{
    return (double) tool->speed;
}

gint gv_autopan_tool_set_standard_path(GvAutopanTool *tool, gint path_type)
{
    gint playing;

    if ( ( path_type < 0 ) || ( path_type >= STANDARD_PATHS ) )
    {
        g_error("GvAutopan: invalid value for standard path setting");
        return FALSE;
    }

    playing = tool->play_flag;
    if ( playing == 1 )
        gv_autopan_tool_pause(tool);

    tool->path_type = path_type;

    if ( playing != 0 )
        compute_locations_and_zoom(tool);

    if ( playing == 1 )
        gv_autopan_tool_play(tool);

    return TRUE;
}

gint gv_autopan_tool_set_lines_path( GvAutopanTool *tool, GvShapes *lines)
{
    int idx;
    GvShape *shp;

    if (tool->nonstandard_path_shapes != NULL)
    {
      /* clear old shapes; free space associated with them */
      g_object_unref( tool->nonstandard_path_shapes );
      tool->nonstandard_path_shapes = NULL;
    }
    tool->nonstandard_path_shapes = (GvShapes *) gv_shapes_new();

    for (idx = 0; idx<lines->shapes->len; idx++)
    {
        shp = gv_shapes_get_shape(lines, idx);
	if ( ( shp != NULL ) &&
             ( gv_shape_type( shp ) == GVSHAPE_LINE ) )
	{
	    gv_shapes_add_shape(tool->nonstandard_path_shapes, gv_shape_copy(shp));
        }
    }
    tool->path_type = LINES_PATH;
    compute_locations_and_zoom(tool);

    return TRUE;
}

/* Set the extents of the full panning region  */
/* Returns TRUE for success; FALSE for failure */
/* (eg. if panning extents parameter doesn't   */
/* apply for the current path type)            */
gint gv_autopan_tool_new_rect(GvAutopanTool *tool, GvRect *rect)
{


    tool->pan_region.x = rect->x;
    tool->pan_region.y = rect->y;
    tool->pan_region.width = rect->width;
    tool->pan_region.height = rect->height;

    if ( ( tool->trail_overview_region.width <= 0 ) && (tool->num_trail_tiles < 1) )
    {
        gv_autopan_tool_set_trail_parameters( tool, rect, tool->trail_overview_width_pixels );
        tool->trail_tile_ysize = tool->trail_tile_xsize;
    }

    /* If path is non-standard, region can be set, but won't be used */
    if ( tool->path_type < 0 )
      return TRUE;

    if (tool->centers != NULL)
    {
        g_array_free(tool->centers, TRUE);
        tool->centers = NULL;
    }

    if (tool->play_flag != 0)
        compute_locations_and_zoom(tool);

    return TRUE;
}
gint gv_autopan_tool_get_rect(GvAutopanTool *tool, GvRect *rect)
{
    if ( tool->path_type < 0 )
      return FALSE;

    rect->x = tool->pan_region.x;
    rect->y = tool->pan_region.y;
    rect->width = tool->pan_region.width;
    rect->height = tool->pan_region.height;

    return TRUE;
}

/* Reset the current position of the primary view window.  Snaps
 * to the closest listed location.
 */
gint gv_autopan_tool_set_location(GvAutopanTool *tool, gvgeocoord x,
                                  gvgeocoord y, gvgeocoord z)
{
    /* TODO:
     * - if playing, set pause flag
     * - find closest position in list and set new current position to it
     * - if tool was playing, restart
     */
    gvgeocoord mindiff, cdiff, cx, cy, cz;
    gint cidx=0, i;
    GvVertex3d *loc;

    if (GV_TOOL(tool)->view == NULL)
        return FALSE;

    if (tool->centers == NULL)
        compute_locations_and_zoom(tool);

    if (tool->num_centers < 1)
        return FALSE;

    loc = &(g_array_index( tool->centers, GvVertex3d, 0) );
    cx = -1*loc->x;
    cy = -1*loc->y;
    cz = -1*loc->z;
    cdiff = sqrt((cx-x)*(cx-x)+(cy-y)*(cy-y)+(cz-z)*(cz-z));
    mindiff = cdiff;
    for (i=1; i < tool->num_centers; i++)
    {
        loc = &(g_array_index( tool->centers, GvVertex3d, i) );
        cx = -1*loc->x;
        cy = -1*loc->y;
        cz = -1*loc->z;
        cdiff = sqrt((cx-x)*(cx-x)+(cy-y)*(cy-y)+(cz-z)*(cz-z));
        if (cdiff < mindiff)
        {
            mindiff = cdiff;
            cidx = i;
        }
    }

    tool->current_index = cidx;

    return TRUE;
}

/*
 * Get the current position.  Returns FALSE if there is no
 * current position; TRUE if the position is valid.
 */
gint gv_autopan_tool_get_location(GvAutopanTool *tool, gvgeocoord *x,
                                  gvgeocoord *y, gvgeocoord *z)
{
    GvVertex3d *oloc;

    /* If tool is deactivated or stopped, return FALSE because
     * there is no "current location".
     */

    *x = 0;
    *y = 0;
    *z = 0;

    if (GV_TOOL(tool)->view == NULL)
        return FALSE;

    if (tool->play_flag == 0)
        return FALSE;

    if (tool->centers == NULL)
        return FALSE;

    if (tool->current_index < tool->num_centers)
    {
        oloc = &(g_array_index(tool->centers, GvVertex3d,
                               tool->current_index));
        *x = -1*oloc->x;
        *y = -1*oloc->y;
    }
    else
        return FALSE;


    return TRUE;
}

/* Overlap is only relevant for standard paths covering the whole
 * panning region.  It determines the overlap between successive
 * passes perpendicular to the direction of panning.
 */

gint gv_autopan_tool_set_overlap(GvAutopanTool *tool, gvgeocoord overlap)
{

    tool->overlap = overlap;
    if ( (tool->play_flag != 0) && (tool->path_type >= 0) )
        compute_locations_and_zoom(tool);

    return TRUE;

}

double gv_autopan_tool_get_overlap(GvAutopanTool *tool)
{
    return (double) tool->overlap;
}

/* Either the size of the block in the x dimension OR the x resolution of
 * the view in terms of the pixels of a reference raster may be used to
 * determine the panning zoom level.
 */

gint gv_autopan_tool_set_block_x_size(GvAutopanTool *tool,
                                      gvgeocoord block_x_size,
                                      gint mode)
{
    /* mode 0: block size is a fraction of panning extents
     *         - block size will change depending on panning extents
     *         - zoom level will change when window is resized
     *      1: block size is in view coordinates
     *         - block size will not change with different panning extents
     *         - zoom level will change when window is resized
     */
    if ((mode != 0) && (mode != 1))
    {
        g_error("mode must be 0 or 1");
        return FALSE;
    }
    tool->block_size_mode = mode;
    tool->block_x_size = block_x_size;
    if (tool->play_flag != 0)
        compute_locations_and_zoom(tool);

    return TRUE;

}

gint gv_autopan_tool_set_x_resolution(GvAutopanTool *tool,
                                    gvgeocoord resolution)
{
    /* mode 2: block size is set so that the current layer resolution
     *         will remain constant.
     *         - block size will change when window is resized, and
     *           has no dependence on panning extents
     *         - zoom level will not change when window is resized
     */
    tool->block_size_mode = 2;
    tool->resolution = resolution;
    if (tool->play_flag != 0)
        compute_locations_and_zoom(tool);

    return TRUE;
}

/* Get state information about the tool */
void gv_autopan_tool_get_state(GvAutopanTool *tool,
                               gint *play_flag,
                               gint *path_type,
                               gint *block_size_mode,
                               gvgeocoord *block_x_size,
                               gvgeocoord *x_resolution,
                               gint *num_views)
{
    *play_flag = tool->play_flag;
    *path_type = tool->path_type;
    *block_size_mode = tool->block_size_mode;
    *block_x_size = tool->block_x_size;
    *x_resolution = tool->resolution;
    *num_views = tool->num_views;
}

void gv_autopan_tool_clear_trail( GvAutopanTool *tool )
{
    int i, j;
    GvAutopanViewItem *item;
    GvAutopanTrailTile *tile;

    if ( tool->trail != NULL )
    {
        while ( tool->num_trail_tiles > 0 )
        {
            tile = &(g_array_index(tool->trail, GvAutopanTrailTile, tool->num_trail_tiles - 1 ));
            g_free(tile->mask);
            g_array_remove_index(tool->trail, tool->num_trail_tiles-1);
            tool->num_trail_tiles--;
        }

        g_array_free(tool->trail, TRUE);
        tool->trail = NULL;

        for( i=0; i<tool->num_views; i++)
        {
        	g_assert(tool->view_items!=NULL);
            item = &(g_array_index(tool->view_items, GvAutopanViewItem, i));
            if ( item->trail_mode > 0)
            {
            	delete_trail_textures(item);
                gv_view_area_queue_draw(item->view);
            }
        }
        /* Force tool to reset panning region next time trail comes on */
        tool->trail_overview_region.width = -1;
    }

}

gint gv_autopan_tool_set_trail_color(GvAutopanTool *tool, GvViewArea *view,
                                     float red, float green, float blue,
                                     float alpha)
{
    int i, viewidx;
    GvAutopanViewItem *item;

    viewidx = -1;
    for (i=0; i<tool->num_views; i++)
    {
        item = &(g_array_index(tool->view_items, GvAutopanViewItem, i));
        if (item->view == view)
            viewidx = i;
    }
    if (viewidx < 0)
    {
        g_error("View not registered with autopan tool!");
        return FALSE;
    }

    item = &(g_array_index(tool->view_items, GvAutopanViewItem, viewidx));
    item->trail_color[0] = red;
    item->trail_color[1] = green;
    item->trail_color[2] = blue;
    item->trail_color[3] = alpha;
    if ( tool->num_trail_tiles > 0 )
        gv_view_area_queue_draw( view );

    return TRUE;

}

gint gv_autopan_tool_set_trail_mode( GvAutopanTool *tool, GvViewArea *view,
                                     gint trail_mode )
{
    int i, j, viewidx, omode;
    GvAutopanViewItem *item;

    viewidx = -1;
    for (i=0; i<tool->num_views; i++)
    {
        item = &(g_array_index(tool->view_items, GvAutopanViewItem, i));
        if (item->view == view)
            viewidx = i;
    }
    if (viewidx < 0)
    {
        g_error("View not registered with autopan tool!");
        return FALSE;
    }

    if ( (trail_mode != 0) && (trail_mode != 1) )
    {
        g_warning("gvautopan: trail_mode must be 0 or 1- leaving alone");
        return FALSE;
    }

    item = &(g_array_index(tool->view_items, GvAutopanViewItem, viewidx));
    tool->trail_mode = tool->trail_mode - item->trail_mode + trail_mode;
    omode = item->trail_mode;
    item->trail_mode = trail_mode;

    /* Make sure trail is properly initialized if mode is turning on: set to pan_region region */
    if (( tool->trail_mode > 0 ) && ( tool->trail_overview_region.width < 0 ) && ( tool->pan_region.width > 0 ) )
        gv_autopan_tool_set_trail_parameters( tool, &tool->pan_region, tool->trail_overview_width_pixels );

    if ( ( omode == 1 ) && ( trail_mode == 0 ) && ( item->trail_textures != NULL ) )
    {
    	delete_trail_textures(item);
    }
    else if ( ( omode == 0 ) && ( trail_mode == 1 ) )
        create_trail_textures( tool, item );

    if ( tool->num_trail_tiles > 0 )
        gv_view_area_queue_draw( view );

    return TRUE;
}


/* Compute the zoom level and populate the list of locations to translate to */
#define MAX_LOCATION_BYTES 100000000	//used to limit the size of tool->centers in compute_locations_and_zoom
static void compute_locations_and_zoom(GvAutopanTool *tool)
{
    gvgeocoord cx=0, cy=0, mindiff=-1, cdiff;
    gint reset_position=0, cidx=0;
    GvVertex3d nloc;
    GvVertex3d *oloc;
    static gint warnOnce = FALSE;

    //make sure the view exists first - it will be NULL if the tool was deactivated
    g_return_if_fail(GV_TOOL(tool)->view != NULL);

    tool->win_width = GV_TOOL(tool)->view->state.shape_x;
    tool->win_height = GV_TOOL(tool)->view->state.shape_y;

    if (tool->centers != NULL)
    {
        if (tool->current_index < tool->num_centers)
        {
            oloc = &(g_array_index(tool->centers, GvVertex3d,
                                   tool->current_index));
            cx = oloc->x;
            cy = oloc->y;
            reset_position=1;
        }
        g_array_free(tool->centers, TRUE);
    }

    if ((tool->play_flag == 0) ||
        (( tool->path_type > -1 ) &&
	 ((-1*cx < tool->pan_region.x) ||
          (-1*cy < tool->pan_region.y) ||
          (-1*cx > tool->pan_region.x + tool->pan_region.width) ||
	  (-1*cy > tool->pan_region.y + tool->pan_region.height))))
        reset_position = 0;

    tool->centers = g_array_new(FALSE, FALSE, sizeof(GvVertex3d));

    nloc.z = 0.0;

    /* TODO: see comment above */
    if (tool->path_type < LINES_PATH)
    {
        g_error("non-standard paths other than lines not implemented");
    }
    else if ( tool->path_type == LINES_PATH )
    {
        GvShape *line;
        gvgeocoord bsx, delta, x1, y1, x2, y2, dist, dx, dy, doff;
        gint i, j, k, nnodes, nstop;

        if (tool->block_size_mode == 2)
            tool->block_x_size = tool->resolution * tool->win_width;

        bsx = tool->block_x_size;
        /* For now, ignore case where block size mode is 0; eventually
         * might make it related to total line length, or something
         * like that.  Here, block size modes 0 and 1 will both behave
         * the same way as block size mode 1 does in the standard path case.
         */

        delta = fabs(tool->speed*bsx);

        tool->num_centers = 0;

        for ( i = 0; i < tool->nonstandard_path_shapes->shapes->len; i++ )
	{
  	    line = gv_shapes_get_shape(tool->nonstandard_path_shapes, i);

            if (( line != NULL ) && ( gv_shape_get_nodes( line, 0 ) > 1 ))
	    {
	        nnodes = gv_shape_get_nodes( line, 0 );
	        for ( j = 0 ; j < nnodes - 1; j++ )
	        {
		    x1 = gv_shape_get_x( line, 0, j );
		    y1 = gv_shape_get_y( line, 0, j );
		    x2 = gv_shape_get_x( line, 0, j+1 );
		    y2 = gv_shape_get_y( line, 0, j+1 );

                    dx = x2-x1;
                    dy = y2-y1;
                    dist = sqrt(dx*dx + dy*dy);
                    nstop = (int) ceil(dist/delta) + 1;
                    /* offset ensures that centers calculated in one direction
                     * are same as the centers calculated if the speed is reversed.
                     */
                    doff = ( ceil(dist/delta) - dist/delta )/2;
                    if (nstop > 1)
		    {
                        dx = dx/(nstop-1);
                        dy = dy/(nstop-1);
                    }
                    else
 		        doff = 0.0;

                    for ( k = 0; k < nstop; k++ )
		    {
  	  	        nloc.x = -1*(dx*(k-doff) + x1);
		        nloc.y = -1*(dy*(k-doff) + y1);
                        g_array_append_val(tool->centers, nloc);
                        if (reset_position == 1)
                        {
                            cdiff = sqrt((nloc.x-cx)*(nloc.x-cx) +
                                         (nloc.y-cy)*(nloc.y-cy));
                            if ((mindiff == -1) || (cdiff < mindiff))
                            {
                                cidx = tool->num_centers;
                                mindiff = cdiff;
                            }
                        }
                        tool->num_centers += 1;
                    }
                }
            }
            else if (( line != NULL ) && ( gv_shape_get_nodes( line, 0 ) == 1 ))
	    {
	        /* line only has a single node- set path so that node
                 * gets visited once.
                 */
  	        nloc.x = -1*gv_shape_get_x(line,0,0);
                nloc.y = -1*gv_shape_get_y(line,0,0);
                g_array_append_val(tool->centers, nloc);
                if (reset_position == 1)
                {
                    cdiff = sqrt((nloc.x-cx)*(nloc.x-cx) +
                                 (nloc.y-cy)*(nloc.y-cy));
                    if ((mindiff == -1) || (cdiff < mindiff))
                    {
                        cidx = tool->num_centers;
                        mindiff = cdiff;
                    }
                }
                tool->num_centers += 1;
	    }
        }

        tool->win_zoom = LOG2(tool->win_width/tool->block_x_size);

    }
    else if ((tool->path_type == TL_R_D_L_D) ||
             (tool->path_type == BL_R_U_L_U) ||
             (tool->path_type == TR_L_D_R_D) ||
             (tool->path_type == BR_L_U_R_U))
    {
        /* Note: currently overlap is dealt with by showing extra
           along the last (bottom) pass if the blocks don't fit
           evenly into the panning extents.  May want to do this
           a bit differently later.
        */

        gvgeocoord bsx, bsy, delta, xoff, yoff;
        guint nxloc, npasses, nvert, i, j;

        if (tool->block_size_mode == 2)
            tool->block_x_size = tool->resolution * tool->win_width;

        bsx = tool->block_x_size;
        if (tool->block_size_mode == 0)
            bsx = bsx * tool->pan_region.width;

        bsy = bsx * tool->win_height/tool->win_width;

        npasses = (guint) ceil(
               (tool->pan_region.height - (tool->overlap*bsy))/
                           ((1.0 - tool->overlap)*bsy));
        //make sure there is at least one pass - setting the x resolution has been seen to make npasses==0
        //gv_autopan_tool_play will crash if tool->centers is empty
        if (npasses<1)
        	npasses=1;

        if (tool->speed < 0)
            delta = -1*tool->speed*bsx;
        else
            delta = tool->speed*bsx;

        nxloc = (guint) ceil((tool->pan_region.width - bsx)/delta) + 1;
        nvert = (guint) ceil(bsy*(1.0 - tool->overlap)/delta);

        /* NOTE: in order to keep speed constant for different
         *       sizes of panning region, the panning region is
         *       rounded to the nearest multiple of delta
         *       (otherwise delta would have to be modified
         *       to fit evenly into the panning region, which
         *       results in noticeable speed changes between
         *       small and large regions.  Likewise, the
         *       overlap is also rounded.
         */

        if (((tool->path_type == TL_R_D_L_D) &&
             (GV_TOOL(tool)->view->state.flip_y > 0)) ||
            ((tool->path_type == BL_R_U_L_U) &&
             (GV_TOOL(tool)->view->state.flip_y < 0)) ||
            ((tool->path_type == TR_L_D_R_D) &&
             (GV_TOOL(tool)->view->state.flip_y > 0)) ||
            ((tool->path_type == BR_L_U_R_U) &&
             (GV_TOOL(tool)->view->state.flip_y < 0)))
            bsy=-1*bsy;

        if ((tool->path_type == TR_L_D_R_D) ||
            (tool->path_type == BR_L_U_R_U))
            xoff = tool->pan_region.x + tool->pan_region.width - bsx/2.0;
        else
            xoff = tool->pan_region.x + bsx/2.0;

        if ( tool->pan_region.width < bsx )
        {
            /* Very narrow panning region- only one x location */
            xoff = tool->pan_region.x + tool->pan_region.width/2.0;
            nxloc = 1;
        }

        if (bsy > 0)
            yoff = tool->pan_region.y + bsy/2.0;
        else
            yoff = tool->pan_region.y + tool->pan_region.height + bsy/2.0;

        if ((tool->path_type == TR_L_D_R_D) ||
            (tool->path_type == BR_L_U_R_U))
            delta = -1.0*delta;

        tool->num_centers = 0;

        //this is a big problem with the current implementation.  It saves every single translation point in the image.
        //for a large image and slow speed, this quickly becomes a large number and then there is the inevitable out-of-memory exception.
        //this is an attempt to limit the resolution and avoid the exception
        //just choosing an arbitrarily large number that should not cause any problems
        //but this whole tool could use a good re-write
        if (npasses*nxloc*sizeof(GvVertex3d) > MAX_LOCATION_BYTES)
        {
        	if (!warnOnce)
        	{
        		g_warning("gv_autopan_tool: The requested resolution is too fine and will be automatically backed off.  This is a known problem with the autopan tool.\n");
        		warnOnce = TRUE;
        	}
			while (npasses*nxloc*sizeof(GvVertex3d) > MAX_LOCATION_BYTES)
			{
				npasses/=2;
				nxloc/=2;
			}
        }

        for ( i = 0; i < npasses; i++ )
        {
            for ( j = 0; j < nxloc; j++ )
            {
                if (nxloc > 1)
                {
                    if ( (i % 2) == 0)
                        nloc.x = -1*(xoff + j*delta);
                    else
                        nloc.x = -1*(xoff + (nxloc-1-j)*delta);
                }
                else
                    nloc.x = -1*xoff;

                nloc.y = -1*(yoff + i*bsy*(1.0 - tool->overlap));

                g_array_append_val(tool->centers, nloc);
                if (reset_position == 1)
                {
                    cdiff = sqrt((nloc.x-cx)*(nloc.x-cx) +
                                 (nloc.y-cy)*(nloc.y-cy));
                    if ((mindiff == -1) || (cdiff < mindiff))
                    {
                        cidx = tool->num_centers;
                        mindiff = cdiff;
                    }
                }
                tool->num_centers += 1;
            }

            if (nxloc > 1)
            {
                nloc.x = -1*xoff;
                if ( (i % 2) == 0)
                    nloc.x -= (nxloc-1)*delta;
            }

            if ( i != npasses - 1)
            {
                for ( j = 1; j < nvert; j++ )
                {
                    nloc.y = -1*(yoff+(i+j*1.0/nvert)*bsy*(1.0-tool->overlap));
                    g_array_append_val(tool->centers, nloc);
                    if (reset_position == 1)
                    {
                        cdiff = sqrt((nloc.x-cx)*(nloc.x-cx) +
                                     (nloc.y-cy)*(nloc.y-cy));
                        if ((mindiff == -1) || (cdiff < mindiff))
                        {
                            cidx = tool->num_centers;
                            mindiff = cdiff;
                        }
                    }
                    tool->num_centers += 1;
                }
            }
        }
        if (tool->block_size_mode == 0)
        {
            tool->win_zoom = LOG2(tool->win_width/
                      (tool->block_x_size*tool->pan_region.width));
        }
        else
            tool->win_zoom = LOG2(tool->win_width/tool->block_x_size);

    }
    else if (tool->path_type == TL_R_D_R_D)
    {
        gvgeocoord bsx, bsy, delta, xoff, yoff;
        guint nxloc, npasses, i, j;

        if (tool->block_size_mode == 2)
            tool->block_x_size = tool->resolution * tool->win_width;

        bsx = tool->block_x_size;
        if (tool->block_size_mode == 0)
            bsx = bsx * tool->pan_region.width;

        bsy = bsx * tool->win_height/tool->win_width;

        npasses = (guint) ceil(
               (tool->pan_region.height - (tool->overlap*bsy))/
                           ((1.0 - tool->overlap)*bsy));
        //make sure there is at least one pass - setting the x resolution has been seen to make npasses==0
        //gv_autopan_tool_play will crash if tool->centers is empty
        if (npasses<1)
        	npasses=1;

        if (tool->speed < 0)
            delta = -1*tool->speed*bsx;
        else
            delta = tool->speed*bsx;

        nxloc = (guint) ceil((tool->pan_region.width - bsx)/delta) + 1;

        if (GV_TOOL(tool)->view->state.flip_y > 0)
            bsy=-1*bsy;


        xoff = tool->pan_region.x + bsx/2.0;

        if ( tool->pan_region.width < bsx )
        {
            xoff = tool->pan_region.x + tool->pan_region.width/2.0;
            nxloc = 1;
        }

        if (bsy > 0)
            yoff = tool->pan_region.y + bsy/2.0;
        else
            yoff = tool->pan_region.y + tool->pan_region.height + bsy/2.0;


        tool->num_centers = 0;

        //this is a big problem with the current implementation.  It saves every single translation point in the image.
        //for a large image and slow speed, this quickly becomes a large number and then there is the inevitable out-of-memory exception.
        //this is an attempt to limit the resolution and avoid the exception
        //just choosing an arbitrarily large number that should not cause any problems
        //but this whole tool could use a good re-write
        if (npasses*nxloc*sizeof(GvVertex3d) > MAX_LOCATION_BYTES)
        {
        	if (!warnOnce)
        	{
        		g_warning("gv_autopan_tool: The requested resolution is too fine and will be automatically backed off.  This is a known problem with the autopan tool.\n");
        		warnOnce = TRUE;
        	}
        	while (npasses*nxloc*sizeof(GvVertex3d) > MAX_LOCATION_BYTES)
			{
				npasses/=2;
				nxloc/=2;
			}
        }

        for ( i = 0; i < npasses; i++ )
        {
            for ( j = 0; j < nxloc; j++ )
            {
                if (nxloc > 1)
                    nloc.x = -1*(xoff + j*delta);
                else
                    nloc.x = -1*xoff;

                nloc.y = -1*(yoff + i*bsy*(1.0 - tool->overlap));

                g_array_append_val(tool->centers, nloc);
                if (reset_position == 1)
                {
                    cdiff = sqrt((nloc.x-cx)*(nloc.x-cx) +
                                 (nloc.y-cy)*(nloc.y-cy));
                    if ((mindiff == -1) || (cdiff < mindiff))
                    {
                        cidx = tool->num_centers;
                        mindiff = cdiff;
                    }
                }
                tool->num_centers += 1;
            }
        }
        if (tool->block_size_mode == 0)
        {
            tool->win_zoom = LOG2(tool->win_width/
                      (tool->block_x_size*tool->pan_region.width));
        }
        else
            tool->win_zoom = LOG2(tool->win_width/tool->block_x_size);

    }
    else
    {
        g_error("path not implemented");
    }

    tool->current_index = cidx;
}


/* When the tool is panning, optional secondary views can be associated
 * with the primary view.  In these views, the area currently
 * covered by the primary view is drawn as a blue-green rectangle
 * in each secondary view.  The secondary views can be configured
 * so that the primary view resolution and location can be updated
 * by dragging the rectangle in the secondary view.
 *
 * can_resize: 0 if resetting the resolution is disabled; 1 if it
 *             is enabled.  If resizing is on, corners can be
 *             selected and dragged.  The shape of the rectangle
 *             must always be dictated by the aspect ratio of the
 *             primary view though, so dragging will snap to
 *             a rectangle of the appropriate shape at the end.
 *
 * can_reposition: 0 if resetting the position is disabled; 1 if it
 *               is enabled.  If repositioning is on, borders
 *               can be selected and dragged (whole box will move).
 *
 * trail_mode: 0 if secondary view should not indicate where tool
 *             has already travelled; 1 if it should display the trail.
 *
 */

gint gv_autopan_tool_register_view(GvAutopanTool *tool, GvViewArea *view,
                                   gint can_resize, gint can_reposition,
                                   gint trail_mode)
{
    GvAutopanViewItem newview;

    /* TODO:
     * - if playing, pause
     * - add view to list of secondary views
     * - connect the view's gldraw event to gv_autopan_tool_sv_draw
     * - if can_resize or can_reposition is 1, connect to button-press,
     *   button-release, and motion-notify events for that view.
     * Note: if things are too slow or flashy using the gldraw event,
     *       could do something similar to what was done for the
     *       ghost cursor.  Probably could avoid changes to gvviewarea
     *       by storing more parameters in the autopan tool and just
     *       calling gv_manager_set_busy, gtk_widget_queue draw, then
     *       connecting the view to GLCURSOR instead of GLDRAW.
     */

    /* First, make sure this is not the main panning view */
    if (view == GV_TOOL(tool)->view)
    {
        g_error("Autopan: Cannot register main view as a secondary view!");
        return FALSE;
    }

    if (can_resize != 0)
        g_warning("gvautopan: Secondary view resize not implemented yet!");

    if ( (trail_mode != 0) && (trail_mode != 1) )
    {
        g_warning("gvautopan: trail_mode must be 0 or 1");
        return FALSE;
    }

    newview.can_resize = 0;
    newview.can_reposition = can_reposition;
    newview.trail_mode = trail_mode;

    tool->trail_mode = tool->trail_mode + trail_mode;

    newview.trail_color[0] = 1.0;
    newview.trail_color[1] = 0.75;
    newview.trail_color[2] = 0.0;
    newview.trail_color[3] = 0.5;

    newview.banding = 0;
    newview.translating = 0;
    newview.play_flag = 0;

    /* Add a reference to this view */
    g_object_ref(view);

    newview.view = view;
    newview.tool = tool;
    newview.trail_textures = NULL;

    if (tool->view_items == NULL)
        tool->view_items = g_array_new(FALSE, FALSE, sizeof(GvAutopanViewItem));

    /* Connect to view area delete event */
    g_signal_connect_object(view, "destroy",
			      G_CALLBACK(gv_autopan_tool_remove_view),
			      GV_TOOL(tool), G_CONNECT_SWAPPED | G_CONNECT_AFTER);

    /* Connect to view area gldraw signal so that current region
     * can be drawn in the view whenever the view is refreshed.
     */
    g_signal_connect_object(view, "gldraw",
				  G_CALLBACK(gv_autopan_tool_sv_draw),
				  GV_TOOL(tool), G_CONNECT_SWAPPED | G_CONNECT_AFTER);

    /* Connect to button-press and button-release signals so that
     * translating/resizing can be detected.
     */
    newview.press_id = \
           g_signal_connect(view, "button-press-event",
        		   G_CALLBACK(gv_autopan_tool_sv_button_press),
                   tool);

    newview.release_id = \
           g_signal_connect(view, "button-release-event",
        		   G_CALLBACK(gv_autopan_tool_sv_button_release),
                   tool);

    newview.motion_id = \
           g_signal_connect(view, "motion-notify-event",
        		   G_CALLBACK(gv_autopan_tool_sv_motion_notify),
                   tool);


    g_array_append_val(tool->view_items, newview);


    tool->num_views = tool->num_views + 1;

    if ( newview.trail_mode > 0 )
    {
        create_trail_textures( tool, &newview );
        gv_view_area_queue_draw( newview.view );
    }

    return TRUE;
}

gint gv_autopan_tool_remove_view(GvAutopanTool *tool, GvViewArea *view)
{
    int i, j, viewidx;
    GvAutopanViewItem *item;
    GLuint *texName;

   /* TODO:
     * - disconnect from view's gldraw event
     * - disconnect view from button-press/button-release/motion-notify
     *   events if necessary
     * - remove view from list
     * - redraw the view to get rid of trail artifact
     */

    viewidx = -1;
    for (i=0; i<tool->num_views; i++)
    {
        item = &(g_array_index(tool->view_items, GvAutopanViewItem, i));
        if (item->view == view)
            viewidx = i;
    }
    if (viewidx < 0)
    {
        g_error("View not registered for removal!");
        return FALSE;
    }

    item = &(g_array_index(tool->view_items, GvAutopanViewItem, viewidx));

    tool->trail_mode = tool->trail_mode - item->trail_mode;

    if ( item->trail_textures != NULL )
    {
    	delete_trail_textures(item);
    }

    g_signal_handler_disconnect(view, item->press_id);
    g_signal_handler_disconnect(view, item->release_id);
    g_signal_handler_disconnect(view, item->motion_id);

    g_array_remove_index(tool->view_items, viewidx);

    g_signal_handlers_disconnect_matched(view, G_SIGNAL_MATCH_DATA,
												0, 0, NULL, NULL, tool);
    g_object_unref(view);
    tool->num_views = tool->num_views - 1;
    if (tool->num_views == 0)
    {
        g_array_free(tool->view_items, TRUE);
        tool->view_items = NULL;
    }
    return TRUE;
}


static void gv_autopan_tool_sv_draw(GvTool *tool, GvViewArea *view)
{
    int i;
    GvAutopanViewItem *item;
    GvAutopanTrailTile *tile;
    GLuint texName;
    gvgeocoord xmin, xmax, ymin, ymax;
    gvgeocoord x1, y1, x2, y2, x3, y3, x4, y4;
    gvgeocoord dx=6.0, dy=0.0, xc=0.0, yc=0.0;

    if (GV_AUTOPAN_TOOL(tool)->play_flag == 0)
        return;

    //make sure the view has not been deactivated for the call to map_view_to_view_xy
    g_return_if_fail(GV_TOOL(tool)->view != NULL);

    item = &(g_array_index(GV_AUTOPAN_TOOL(tool)->view_items,
                           GvAutopanViewItem, 0));
    for (i=0; i<GV_AUTOPAN_TOOL(tool)->num_views; i++)
    {
        item = &(g_array_index(GV_AUTOPAN_TOOL(tool)->view_items,
                               GvAutopanViewItem, i));
        if (item->view == view)
            break;
    }

    if (( item->trail_mode > 0 ) &&
        (GV_AUTOPAN_TOOL(tool)->num_trail_tiles > 0))
    {
        glColor4fv( item->trail_color );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glEnable( GL_TEXTURE_2D );

        // May need to uncomment the next four lines at some point-
        // gvrasterlayer.c does in its draw function...not sure why...
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

        for (i=0; i<GV_AUTOPAN_TOOL(tool)->num_trail_tiles; i++)
        {
            tile = (GvAutopanTrailTile *) &g_array_index(
                                        GV_AUTOPAN_TOOL(tool)->trail,
                                        GvAutopanTrailTile, i);
            texName = g_array_index(item->trail_textures, GLuint, i);
            glBindTexture(GL_TEXTURE_2D, texName);

            xmin = tile->x0;
            ymin = tile->y0;
            xmax = tile->xf;
            ymax = tile->yf;

            x1 = x4 = xmin;
            y3 = y4 = ymin;
            x2 = x3 = xmax;
            y1 = y2 = ymax;

            if ( map_view_to_view_xy(tool->view, view, &x1, &y1) &&
                 map_view_to_view_xy(tool->view, view, &x2, &y2) &&
                 map_view_to_view_xy(tool->view, view, &x3, &y3) &&
                 map_view_to_view_xy(tool->view, view, &x4, &y4) )
            {
                glBegin(GL_QUADS);
                glTexCoord2f(0.0,1.0); glVertex2f(x1,y1);
                glTexCoord2f(1.0,1.0); glVertex2f(x2,y2);
                glTexCoord2f(1.0,0.0); glVertex2f(x3,y3);
                glTexCoord2f(0.0,0.0); glVertex2f(x4,y4);
                glEnd();
            }
        }
        glDisable( GL_TEXTURE_2D );
        glDisable( GL_BLEND );
    }

    gv_view_area_get_extents(tool->view, &xmin, &ymin, &xmax, &ymax);
    gv_view_area_correct_for_transform(view, dx, dy, &dx, &dy);

    x1 = xmin;
    y1 = ymin;
    x2 = xmin;
    y2 = ymax;
    x3 = xmax;
    y3 = ymax;
    x4 = xmax;
    y4 = ymin;

    xc = (xmin + xmax)/2.0;
    yc = (ymin + ymax)/2.0;

    if ( !map_view_to_view_xy(tool->view, view, &x1, &y1) ||
         !map_view_to_view_xy(tool->view, view, &x2, &y2) ||
         !map_view_to_view_xy(tool->view, view, &x3, &y3) ||
         !map_view_to_view_xy(tool->view, view, &x4, &y4) ||
         !map_view_to_view_xy(tool->view, view, &xc, &yc) )
    {
        CPLDebug( "GvAutopan", "gv_reproject_points(%s,%s) failed.",
                  gv_view_area_get_projection(tool->view),
                  gv_view_area_get_projection(view) );
        return;
    }


    glColor3f(0.0,0.5,1.0);
    glBegin(GL_LINE_LOOP);
    glVertex2(x1, y1);
    glVertex2(x2, y2);
    glVertex2(x3, y3);
    glVertex2(x4, y4);
    glEnd();

    glBegin(GL_LINES);
    glVertex3(xc-dx, yc,0.0);
    glVertex3( xc+dx, yc,0.0);
    glVertex3( xc, yc-dx,0.0);
    glVertex3( xc, yc+dx,0.0);
    glEnd();

}

static gboolean gv_autopan_tool_sv_button_press(GtkWidget *view,
                                            GdkEventButton *event,
                                            gpointer *data_tool)
{
    int i, viewidx;
    GvAutopanViewItem *item;
    GvAutopanTool *tool;
    gint pick;

    /* TODO:
     * - try to select a border (can_resize=1) and/or
     *   the center (can_reposition=1).  If neither is
     *   selected, return.
     * - record whether or not tool is playing
     * - if it is playing, pause
     * - send a zoomextents-changing signal
     */

    if ( (event->type != GDK_BUTTON_PRESS) ||
         (event->button != 1) )
        return FALSE;


    tool = (GvAutopanTool *) data_tool;

    viewidx = -1;
    for (i=0; i<tool->num_views; i++)
    {
        item = &(g_array_index(tool->view_items, GvAutopanViewItem, i));
        if ( item->view == (GvViewArea *) view)
            viewidx = i;
    }

    if (viewidx < 0)
    {
        g_error("View not found in autopan tool list!");
        return FALSE;
    }

    item = &(g_array_index(tool->view_items, GvAutopanViewItem, viewidx));

    if ((item->can_resize == 0) && (item->can_reposition == 0))
        return FALSE;

    pick = gv_autopan_tool_pick(tool, (GvViewArea *) view, event->x, event->y);
    if (( pick == PICK_CENTER ) && ( item->can_reposition == 1 ) )
    {
        item->translating = 1;
        item->play_flag = tool->play_flag;
        gv_autopan_tool_pause(tool);
        //the center is being repositioned - the signal has been handled so return TRUE
        //don't want other tools to get the signal
        return TRUE;
    }
    return FALSE;
}

static gboolean gv_autopan_tool_sv_button_release(GtkWidget *view,
                                            GdkEventButton *event,
                                            gpointer *data_tool)
{
    int i, viewidx;
    GvAutopanViewItem *item;
    GvAutopanTool *tool;
    gvgeocoord gx=0.0, gy=0.0;

    /* TODO:
     * - reset extents
     * - send a zoomextents-changed signal
     * - if tool was playing, restart
     */

    tool = (GvAutopanTool *) data_tool;

    //make sure the view has not been deactivated for the call to map_view_to_view_xy
    g_return_val_if_fail(GV_TOOL(tool)->view != NULL, FALSE);

    viewidx = -1;
    for (i=0; i<tool->num_views; i++)
    {
        item = &(g_array_index(tool->view_items, GvAutopanViewItem, i));
        if (item->view == (GvViewArea *) view)
            viewidx = i;
    }

    if (viewidx < 0)
    {
        g_error("View not found in autopan tool list!");
        return FALSE;
    }

    item = &(g_array_index(tool->view_items, GvAutopanViewItem, viewidx));

    if ((item->can_resize == 0) && (item->can_reposition == 0))
        return FALSE;

    if (item->translating == 1)
    {
        gv_view_area_map_pointer(item->view, event->x, event->y, &gx, &gy);
        map_view_to_view_xy(item->view, ((GvTool *) tool)->view, &gx, &gy);
        gv_autopan_tool_set_location(tool, gx, gy, 0);

        if ( item->play_flag == 1 )
        {
            item->play_flag = 0;
            item->translating = 0;
            gv_autopan_tool_play(tool);
        }
        else
        {
            item->play_flag = 0;
            item->translating = 0;
        }
    }
    return FALSE;
}

static gboolean gv_autopan_tool_sv_motion_notify(GtkWidget *view,
                                            GdkEventMotion *event,
                                            gpointer *data_tool)
{
    int i, viewidx, sv_idx;
    GvVertex3d *nloc;
    GvAutopanViewItem *item;
    GvAutopanViewItem *sv;
    GvAutopanTool *tool;
    gvgeocoord gx=0.0, gy=0.0;

    /* TODO:
     * - redraw the box in the view and in other secondary views,
     *   making sure that box is snapped to correct aspect ratio.
     */

    tool = (GvAutopanTool *) data_tool;

    viewidx = -1;
    for (i=0; i<tool->num_views; i++)
    {
        item = &(g_array_index(tool->view_items, GvAutopanViewItem, i));
        if (item->view == (GvViewArea *) view)
            viewidx = i;
    }

    if (viewidx < 0)
    {
        g_error("View not found in autopan tool list!");
        return FALSE;
    }

    item = &(g_array_index(tool->view_items, GvAutopanViewItem, viewidx));

    if ((item->can_resize == 0) && (item->can_reposition == 0))
        return FALSE;

    if ( item->translating == 1 )
    {
        gv_view_area_map_pointer(item->view, event->x, event->y, &gx, &gy);
        map_view_to_view_xy(item->view, ((GvTool *) tool)->view, &gx, &gy);
        gv_autopan_tool_set_location(tool, gx, gy, 0);
        nloc = &( g_array_index( tool->centers, GvVertex3d,
                            tool->current_index) );

        gv_view_area_set_translation(GV_TOOL(tool)->view, nloc->x, nloc->y);
        for (sv_idx = 0; sv_idx < tool->num_views; sv_idx++)
        {
            sv = &(g_array_index(tool->view_items, GvAutopanViewItem,
                                   sv_idx));
            gv_view_area_queue_draw(sv->view);
        }
    }

    return FALSE;
}

static gint
gv_autopan_tool_pick(GvAutopanTool *tool, GvViewArea *view,
                     gvgeocoord x, gvgeocoord y)
{
    gvgeocoord xmin, xmax, ymin, ymax;
    gvgeocoord dx=6.0, dy=0.0, xc=0.0, yc=0.0;
    gvgeocoord x1, y1, x2, y2, x3, y3, x4, y4;

    GLuint buf[20];
    GLint hits;
    GLint vp[4];

    //make sure the view has not been deactivated for the call to map_view_to_view_xy
    g_return_val_if_fail(GV_TOOL(tool)->view != NULL, PICK_NONE);

    gv_view_area_get_extents(((GvTool *) tool)->view, &xmin, &ymin, &xmax, &ymax);
    gv_view_area_correct_for_transform(view, dx, dy, &dx, &dy);

    x1 = xmin;
    y1 = ymin;
    x2 = xmin;
    y2 = ymax;
    x3 = xmax;
    y3 = ymax;
    x4 = xmax;
    y4 = ymin;

    xc = (xmin + xmax)/2.0;
    yc = (ymin + ymax)/2.0;

    if ( !map_view_to_view_xy(((GvTool *) tool)->view, view, &x1, &y1) ||
         !map_view_to_view_xy(((GvTool *) tool)->view, view, &x2, &y2) ||
         !map_view_to_view_xy(((GvTool *) tool)->view, view, &x3, &y3) ||
         !map_view_to_view_xy(((GvTool *) tool)->view, view, &x4, &y4) ||
         !map_view_to_view_xy(((GvTool *) tool)->view, view, &xc, &yc) )
    {
        CPLDebug( "GvAutopan", "gv_reproject_points(%s,%s) failed.",
                  gv_view_area_get_projection(((GvTool *) tool)->view),
                  gv_view_area_get_projection(view) );
    }

    vp[0] = vp[1]  = 0;
    vp[2] = (GLint)view->state.shape_x;
    vp[3] = (GLint)view->state.shape_y;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(x, vp[3]-y, PICK_SIZE, PICK_SIZE, vp);
    gluOrtho2D(-vp[2]/2.0, vp[2]/2.0, -vp[3]/2.0, vp[3]/2.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotate(view->state.rot, 0.0, 0.0, 1.0);
    glScale(view->state.linear_zoom * view->state.flip_x,
	     view->state.linear_zoom * view->state.flip_y, 1.0);
    glTranslate(view->state.tx, view->state.ty, 0.0);

    glSelectBuffer(20, buf);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(-1);

    /* Center */
    glLoadName(4);
    glBegin(GL_LINES);
    glVertex2(xc-dx, yc);
    glVertex2(xc+dx, yc);
    glVertex2(xc, yc-dx);
    glVertex2(xc, yc+dx);
    glEnd();

    /* Top */
    glLoadName(0);
    glBegin(GL_LINES);
    glVertex2(x1, y1);
    glVertex2(x4, y4);
    glEnd();

    /* Right */
    glLoadName(1);
    glBegin(GL_LINES);
    glVertex2(x4, y4);
    glVertex2(x3, y3);
    glEnd();

    /* Bottom */
    glLoadName(2);
    glBegin(GL_LINES);
    glVertex2(x2, y2);
    glVertex2(x3, y3);
    glEnd();

    /* Left */
    glLoadName(3);
    glBegin(GL_LINES);
    glVertex2(x1, y1);
    glVertex2(x2, y2);
    glEnd();


    hits = glRenderMode(GL_RENDER);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    /* We're only concerned with the first hit */
    if (hits >= 1)
        return buf[3]+1;

    /* No hits */
    return PICK_NONE;
}

/* Map xy in view1 to xy in view2.  Returns TRUE if
 * successful; FALSE otherwise.
 */

static gint map_view_to_view_xy(GvViewArea *view1, GvViewArea *view2,
                                gvgeocoord *x, gvgeocoord *y)
{
    GvRasterLayer *rst1, *rst2;
    double xd, yd;

    xd = (double) *x;
    yd = (double) *y;

    if (gv_view_area_get_primary_raster(view1) != NULL)
    {
        rst1 = GV_RASTER_LAYER(gv_view_area_get_primary_raster(view1));

	if( gv_view_area_get_raw(view1,NULL) )
            gv_raster_pixel_to_georef( rst1->prototype_data, &xd, &yd, NULL );
    }

    if( gv_view_area_get_projection(view1) != NULL
        && gv_view_area_get_projection(view2) != NULL
        && !EQUAL(gv_view_area_get_projection(view1),
      	    gv_view_area_get_projection(view2))
        && !EQUAL(gv_view_area_get_projection(view1),"PIXEL")
        && !EQUAL(gv_view_area_get_projection(view2),"PIXEL") )
    {
        if( !gv_reproject_points( gv_view_area_get_projection(view1),
      			    gv_view_area_get_projection(view2),
      			    1, &xd, &yd, NULL )
          )
            return FALSE;
    }

    if (gv_view_area_get_primary_raster(view2) != NULL)
    {
        rst2 = GV_RASTER_LAYER(gv_view_area_get_primary_raster(view2));

	if( gv_view_area_get_raw(view2,NULL) )
            gv_raster_georef_to_pixel( rst2->prototype_data, &xd, &yd, NULL );
    }

    *x = (gvgeocoord) xd;
    *y = (gvgeocoord) yd;

    return TRUE;

}

/* Trail-related functions */
gint gv_autopan_tool_set_trail_parameters(GvAutopanTool *tool,
                                          GvRect *overview_extents,
                                          int overview_width_pixels)
{
    gint i,j, xnum, ynum;

    gv_autopan_tool_clear_trail( tool );

    /* set extents */
    tool->trail_overview_region.x = overview_extents->x;
    tool->trail_overview_region.y = overview_extents->y;
    tool->trail_overview_region.width = overview_extents->width;
    tool->trail_overview_region.height = overview_extents->height;
    tool->trail_overview_width_pixels = overview_width_pixels;

    tool->trail_x0 = overview_extents->x;
    tool->trail_y0 = overview_extents->y;
    tool->trail_tile_xsize = overview_extents->width*tool->trail_tile_pixels/overview_width_pixels;
    tool->trail_tile_ysize = tool->trail_tile_xsize;

    /* initialize new tiles */
    xnum = (int) ceil(overview_width_pixels/tool->trail_tile_pixels);
    ynum = (int) ceil((overview_width_pixels/tool->trail_tile_lines)*
                      (overview_extents->height/overview_extents->width));

    for ( i = 0; i < xnum; i++ )
    {
        for ( j = 0; j < ynum; j++ )
            new_trail_tile( tool, i, j );
    }

    return TRUE;
}

void gv_autopan_tool_get_trail_parameters(GvAutopanTool *tool,
                                          GvRect *overview_extents,
                                          int *overview_width_pixels,
                                          int *num_trail_tiles)
{
    overview_extents->x = tool->trail_overview_region.x;
    overview_extents->y = tool->trail_overview_region.y;
    overview_extents->width = tool->trail_overview_region.width;
    overview_extents->height = tool->trail_overview_region.height;
    *overview_width_pixels = tool->trail_overview_width_pixels;
    *num_trail_tiles = tool->num_trail_tiles;
}

gint gv_autopan_tool_save_trail_tiles(GvAutopanTool *tool,
                                      const char *basename)
{
    GvAutopanTrailTile *tile;
    GDALDriverH   driver;
    GDALDatasetH  dataset;
    GDALRasterBandH band;
    double gt[6];

    int i, blen;
    char *name_buf;
    char xindex[25];
    char yindex[25];

    if (tool->num_trail_tiles < 1)
        return 0;

    driver = GDALGetDriverByName( "GTiff" );
    if( driver == NULL )
        return -1;

    blen = strlen(basename);
    name_buf = (char *) malloc( sizeof(char)*(blen+10) );

    for ( i = 0; i < tool->num_trail_tiles; i++ )
    {
        tile = &(g_array_index(tool->trail, GvAutopanTrailTile, i ));
        sprintf(name_buf,"%s%d",basename,i);
        sprintf(xindex,"%d",tile->xindex);
        sprintf(yindex,"%d",tile->yindex);
        //dataset = GDALCreate( driver, name_buf,
        //                      tile->pixels,
        dataset = GDALCreate( driver, name_buf,
                              2*tile->pixels, tile->lines, 1, GDT_Byte,
                              NULL );
        GDALSetMetadataItem( dataset, "xindex", xindex, NULL );
        GDALSetMetadataItem( dataset, "yindex", yindex, NULL );
        gt[0] = tile->x0;
        gt[1] = (tile->xf - tile->x0)/tile->pixels;
        gt[2] = 0;
        gt[3] = tile->y0;
        gt[4] = 0;
        gt[5] = (tile->yf - tile->y0)/tile->lines;
        GDALSetGeoTransform( dataset, gt );
        band = GDALGetRasterBand( dataset, 1 );
        //GDALRasterIO( band, GF_Write, 0, 0, tile->pixels, tile->lines,
        //              tile->mask, tile->pixels, tile->lines, GDT_Byte,
        //              2, 2*tile->pixels );
        GDALRasterIO( band, GF_Write, 0, 0, 2*tile->pixels, tile->lines,
                      tile->mask, 2*tile->pixels, tile->lines, GDT_Byte,
                      1, 2*tile->pixels );

        GDALClose( dataset );
    }

    free(name_buf);

    return tool->num_trail_tiles;

}

gint gv_autopan_tool_load_trail_tiles(GvAutopanTool *tool,
                                      const char *basename,
                                      int num_trail_tiles)
{
  /*
   * Note: trail parameters (overview region, overview width in pixels)
   * must be set properly before this function is called.
   */

    GvAutopanTrailTile *tile;
    GvAutopanViewItem *item;
    GLuint texName;
    GDALDatasetH  dataset;
    GDALRasterBandH band;
    int i, blen, xindex, yindex, idx;
    char *name_buf;

    gv_autopan_tool_clear_trail( tool );

    if (num_trail_tiles < 1)
        return 0;

    blen = strlen(basename);
    name_buf = (char *) malloc( sizeof(char)*(blen+10) );

    for ( i = 0; i < num_trail_tiles; i++ )
    {
        sprintf(name_buf,"%s%d",basename,i);
        dataset = GDALOpen( name_buf, GA_ReadOnly);
        xindex = atoi(GDALGetMetadataItem( dataset, "xindex", NULL ));
        yindex = atoi(GDALGetMetadataItem( dataset, "yindex", NULL ));
        new_trail_tile(tool,xindex,yindex);
        tile = &(g_array_index(tool->trail, GvAutopanTrailTile, tool->num_trail_tiles - 1 ));
        band = GDALGetRasterBand( dataset, 1 );
        //GDALRasterIO( band, GF_Read, 0, 0, tile->pixels, tile->lines,
        //              tile->mask, 2*tile->pixels, tile->lines, GDT_Byte,
        //              1, tile->pixels );
        GDALRasterIO( band, GF_Read, 0, 0, 2*tile->pixels, tile->lines,
                      tile->mask, 2*tile->pixels, tile->lines, GDT_Byte,
                      1, 2*tile->pixels );
        GDALClose( dataset );

        // update textures for views
        for ( idx = 0; idx < tool->num_views; idx++ )
        {
            item = &(g_array_index(tool->view_items,
                           GvAutopanViewItem, idx));

            if ( item->trail_mode < 1 )
                continue;

            /* Appropriate view must be active in order to update texture; */
            /* otherwise changes won't show up.                            */
            if (!gv_view_area_make_current( item->view ))
                g_warning("gv_autopan_tool_load_trail_tiles: Unable to make view current, trail may not update properly!");

            texName = g_array_index( item->trail_textures, GLuint, tool->num_trail_tiles - 1 );
            glBindTexture(GL_TEXTURE_2D,texName);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tile->pixels, tile->lines,
                     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tile->mask);
	}
    }

    free(name_buf);

    return num_trail_tiles;
}

static void new_trail_tile( GvAutopanTool *tool, gint xindex, gint yindex)
{
    GvAutopanTrailTile newtile;
    int idx;
    int pixel_count = tool->trail_tile_pixels*tool->trail_tile_lines;
    GvAutopanViewItem *item;
    GLuint texName;


    newtile.xindex = xindex;
    newtile.yindex = yindex;
    newtile.pixels = tool->trail_tile_pixels;
    newtile.lines = tool->trail_tile_lines;
    newtile.x0 = tool->trail_x0 + tool->trail_tile_xsize*xindex;
    newtile.y0 = tool->trail_y0 + tool->trail_tile_ysize*yindex;
    newtile.xf = tool->trail_x0 + tool->trail_tile_xsize*(xindex + 1);
    newtile.yf = tool->trail_y0 + tool->trail_tile_ysize*(yindex + 1);

    newtile.mask = (unsigned char *) g_malloc( pixel_count*2 );
    memset( newtile.mask, 0, pixel_count*2 );

    if ( tool->trail == NULL )
        tool->trail = g_array_new(FALSE,FALSE, sizeof(GvAutopanTrailTile));

    g_array_append_val( tool->trail, newtile);
    tool->num_trail_tiles = tool->num_trail_tiles + 1;

    for (idx = 0; idx < tool->num_views; idx++)
    {
        item = &(g_array_index(tool->view_items,
                               GvAutopanViewItem, 0));

        if ( item->trail_mode < 1 )
            continue;

        if (item->trail_textures == NULL)
        {
            create_trail_textures( tool, item );
        }
        else
        {
            /* View that texture is going to be displayed in must be the  */
            /* current context when that texture is generated or updated  */
            /* There is supposedly a way to share contexts between views, */
            /* but that hasn't been used here...                          */
            if (!gv_view_area_make_current( item->view ))
                g_warning("new_trail_tile: Unable to make view current, trail may not update properly!");

            glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            glGenTextures(1,&texName);
            glBindTexture(GL_TEXTURE_2D, texName);
            /* The next parameters need to be set whenever a texture is
             * created (tried just putting them in draw function;
             * things didn't work- entire rectangle goes solid-coloured)
             */
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE_ALPHA,newtile.pixels,
                newtile.lines, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, newtile.mask);
            g_array_append_val( item->trail_textures, texName);
        }

    }
}


static void update_trail( GvAutopanTool *tool, gvgeocoord xmin, gvgeocoord ymin,
                          gvgeocoord width, gvgeocoord height )
{
    int xs,xe,ys,ye, i, j, k, p1, p2, l1, l2, offset, pd, tidx;
    GvAutopanTrailTile *tile;
    GvAutopanViewItem *item;
    GLuint texName;


    xs = (int) floor((xmin - tool->trail_x0)/tool->trail_tile_xsize);
    xe = (int) floor((xmin + width - tool->trail_x0)/tool->trail_tile_xsize);
    ys = (int) floor((ymin - tool->trail_y0)/tool->trail_tile_ysize);
    ye = (int) floor((ymin + height - tool->trail_y0)/tool->trail_tile_ysize);

    /* Note: tiles currently aren't indexed/sorted because it is assumed there
      won't be very many, so the search time shouldn't significantly
      slow things down.
    */
    for ( i = xs; i <= xe; i++ )
    {
        for ( j = ys; j <= ye; j++ )
        {
            tile = NULL;
            for (k = 0; k < tool->num_trail_tiles; k++)
            {
                tile = &(g_array_index(tool->trail, GvAutopanTrailTile, k));
                if ((i == tile->xindex) && (j == tile->yindex))
                    break;
                tile = NULL;
            }
            tidx = k;
            if (tile == NULL)
            {
                new_trail_tile( tool, i, j );
                tile = &(g_array_index(tool->trail, GvAutopanTrailTile,
                                       tool->num_trail_tiles - 1 ));
                tidx = tool->num_trail_tiles - 1;
            }
            p1 = (int) floor(((xmin - tile->x0)*
                       tool->trail_tile_pixels/tool->trail_tile_xsize) + 0.5);
            p2 = (int) floor(((xmin + width - tile->x0)*
                       tool->trail_tile_pixels/tool->trail_tile_xsize) + 0.5);
            l1 = (int) floor(((ymin - tile->y0)*
                       tool->trail_tile_lines/tool->trail_tile_ysize) + 0.5);
            l2 = (int) floor(((ymin + height - tile->y0)*
                       tool->trail_tile_lines/tool->trail_tile_ysize) + 0.5);

            p1 = MIN(tool->trail_tile_pixels-1,MAX(p1,0));
            p2 = MIN(tool->trail_tile_pixels-1,MAX(p2,0));
            l1 = MIN(tool->trail_tile_lines-1,MAX(l1,0));
            l2 = MIN(tool->trail_tile_lines-1,MAX(l2,0));

            if ((l1 >= l2) || (p1 >= p2))
                continue;
            pd = (p2-p1+1)*2;
            /* If saving wasn't needed, could use mask only for initializing
             * and work with textures only after that point, I think.
             */
            for (k = l1; k<=l2; k++)
            {
                offset = p1*2 + k*tool->trail_tile_pixels*2;
                memset(tile->mask + offset, 255, pd);
            }

            for ( k = 0; k < tool->num_views; k++ )
            {
                item = &(g_array_index(tool->view_items,
                               GvAutopanViewItem, k));

                if ( item->trail_mode < 1 )
                    continue;

                /* Appropriate view must be active in order to update texture; */
                /* otherwise changes won't show up.                            */
                if (!gv_view_area_make_current( item->view ))
                    g_warning("update_trail: Unable to make view current, trail may not update properly!");

                //this is just a safety net here
                //I have seen a crash because trail_textures==NULL but I'm not sure how it got in that state
                //for now just check for NULL and recreate the textures if they don't exist.
                if (item->trail_textures == NULL)
                {
                	create_trail_textures(tool, item);
                }

                texName = g_array_index( item->trail_textures, GLuint, tidx );
                glBindTexture(GL_TEXTURE_2D,texName);
                glTexSubImage2D(GL_TEXTURE_2D, 0, p1, l1, p2-p1+1, l2-l1+1,
                         GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tool->trail_block);
            }
        }
    }
}

static void create_trail_textures(GvAutopanTool *tool, GvAutopanViewItem *item)
{
    int i;
    GvAutopanTrailTile *tile;
    GLuint texName;

    //A. Klein - suppressing this warning.  It prints out every time the autopan tool is initialized but there does not seem to be any problems.
    //there is no comment here on why this would be unexpected
    //for now, just delete the textures to avoid a memory leak - hopefully this is right
    if ( item->trail_textures != NULL )
    	delete_trail_textures(item);
    //    g_warning("create_trail_textures: textures unexpectedly not null!");

    item->trail_textures = g_array_new(FALSE,FALSE,sizeof(GLuint));

    if (!gv_view_area_make_current( item->view ))
        g_warning("new_trail_tile: Unable to make view current, trail may not update properly!");

    for ( i = 0; i < tool->num_trail_tiles; i++ )
    {
        tile = &(g_array_index(tool->trail,
                               GvAutopanTrailTile, i));

        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glGenTextures(1,&texName);
        glBindTexture(GL_TEXTURE_2D, texName);
        /* The next parameters need to be set whenever a texture is
         * created (tried just putting them in draw function;
         * things didn't work- entire rectangle goes solid-coloured)
         */
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE_ALPHA,tile->pixels,
            tile->lines, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tile->mask);
        g_array_append_val( item->trail_textures, texName);
    }
}

/*deletes all of the textures in item->trail_textures
 * item->trail_textures is freed and set to NULL
 */
static void delete_trail_textures(GvAutopanViewItem *item)
{
	int i;
	GLuint *texName;

	if (item->trail_textures == NULL)
		g_warning("no textures to delete\n");
		return;

	//set the context before glDeleteTextures.  This fixes a bug where white squares are displayed in the main view.
	gv_view_area_make_current(item->view);

	//loop through the tiles and delete them all
    for ( i = item->trail_textures->len-1; i >= 0; i-- )
    {
        texName = &(g_array_index(item->trail_textures, GLuint, i));
        glDeleteTextures(1,texName);
    }
    //finally free the g_array and set to NULL
    g_array_free(item->trail_textures,TRUE);
    item->trail_textures = NULL;
}

