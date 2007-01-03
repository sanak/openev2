/******************************************************************************
 * $Id: gvviewlink.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Manage linked GvViewAreas.
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
 * $Log: gvviewlink.c,v $
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
 * Revision 1.12  2005/01/18 19:55:42  gmwalter
 * Avoid georef->pixel->georef transformations
 * when they are not necessary: second order
 * warps don't invert exactly, and error
 * accumulates.
 *
 * Revision 1.11  2003/03/31 22:27:56  gmwalter
 * Alter copy state zoom level calculation so it doesn't change under rotation.
 *
 * Revision 1.10  2003/03/04 15:30:27  gmwalter
 * Avoid artifacts when cursor mode switched.
 *
 * Revision 1.9  2003/02/21 22:35:30  gmwalter
 * Removed unused variables.
 *
 * Revision 1.8  2003/02/20 19:27:19  gmwalter
 * Updated link tool to include Diana's ghost cursor code, and added functions
 * to allow the cursor and link mechanism to use different gcps
 * than the display for georeferencing.  Updated raster properties
 * dialog for multi-band case.  Added some signals to layerdlg.py and
 * oeattedit.py to make it easier for tools to interact with them.
 * A few random bug fixes.
 *
 * Revision 1.7  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.6  2002/03/25 19:53:41  warmerda
 * fix for NULL projection
 *
 * Revision 1.5  2002/01/30 17:26:11  warmerda
 * make link state copying smarter
 *
 * Revision 1.4  2000/08/10 16:52:40  warmerda
 * watch for destroy, not delete-event
 *
 * Revision 1.3  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvviewlink.h"
#include <gtk/gtksignal.h>
#include "gvrasterlayer.h"
#include "gvutils.h"

static void gv_view_link_class_init(GvViewLinkClass *klass);
static void gv_view_link_init(GvViewLink *link);
static void gv_view_link_view_state_changed(GvViewLink *link, GvViewArea *view);
static void gv_view_link_destroy(GtkObject *object);

/* Ghost cursor stuff */
static void gv_view_link_cursor_draw(GvViewLink *link, GvViewArea *view);

static gboolean gv_view_link_cursor_motion_notify(GvViewLink *link,GdkEventMotion *event, GvViewArea *view);
static gboolean gv_view_link_cursor_leave_notify(GvViewLink *link, GdkEventCrossing *event, GvViewArea *view);
static gboolean gv_view_link_cursor_enter_notify(GvViewLink *link, GdkEventCrossing *event, GvViewArea *view);
static void gv_view_link_cursor_set_x_y(GvViewLink *link, double x, double y, GvViewArea *view);
static void gv_view_link_cursor_get_geo_x_y(double *event_x, double *event_y, GvViewArea *view);

GtkType
gv_view_link_get_type(void)
{
    static GtkType view_link_type = 0;

    if (!view_link_type)
    {
	static const GtkTypeInfo view_link_info =
	{
	    "GvViewLink",
	    sizeof(GvViewLink),
	    sizeof(GvViewLinkClass),
	    (GtkClassInitFunc) gv_view_link_class_init,
	    (GtkObjectInitFunc) gv_view_link_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	view_link_type = gtk_type_unique(gtk_object_get_type(),
					 &view_link_info);
    }
    return view_link_type;
}

static void
gv_view_link_class_init(GvViewLinkClass *klass)
{
    GtkObjectClass *object_class;

    object_class = (GtkObjectClass*) klass;

    object_class->destroy = gv_view_link_destroy;
}

static void
gv_view_link_init(GvViewLink *link)
{
    link->views = NULL;
    link->enabled = FALSE;
    link->blocked = FALSE;

    /* Ghost cursor stuff */
    link->cursor_mode = GV_LINK_CURSOR_OFF;
    link->src_view=NULL;
}

GtkObject *
gv_view_link_new(void)
{
    return GTK_OBJECT(gtk_type_new(GV_TYPE_VIEW_LINK));
}

void
gv_view_link_register_view(GvViewLink *link, GvViewArea *view)
{
    /* Add a reference to this view */
    gtk_object_ref(GTK_OBJECT(view));
    
    /* Add view to registered views list */
    link->views = g_list_prepend(link->views, view);

    /* Connect to the view state changed event */
    gtk_signal_connect_object(GTK_OBJECT(view), "view-state-changed",
			      GTK_SIGNAL_FUNC(gv_view_link_view_state_changed),
			      GTK_OBJECT(link));

    /* Connect to view area delete event */
    gtk_signal_connect_object(GTK_OBJECT(view), "destroy",
			      GTK_SIGNAL_FUNC(gv_view_link_remove_view),
			      GTK_OBJECT(link));

    /* Next two connections are only needed for ghost cursor drawing */
    /* Connect to view area glcursor signal */
    gtk_signal_connect_object(GTK_OBJECT(view), "glcursor",
			      GTK_SIGNAL_FUNC(gv_view_link_cursor_draw),
			      GTK_OBJECT(link));

    /* Connect to motion event */
    gtk_signal_connect_object(GTK_OBJECT(view), "motion-notify-event",
			      GTK_SIGNAL_FUNC(gv_view_link_cursor_motion_notify),
			      GTK_OBJECT(link));

    /* Connect to view area leave signal  */
    gtk_signal_connect_object(GTK_OBJECT(view), "leave-notify-event",
			      GTK_SIGNAL_FUNC(gv_view_link_cursor_leave_notify),
			      GTK_OBJECT(link));

    /* Connect to view area enter signal  */
    gtk_signal_connect_object(GTK_OBJECT(view), "enter-notify-event",
			      GTK_SIGNAL_FUNC(gv_view_link_cursor_enter_notify),
			      GTK_OBJECT(link));

}

void
gv_view_link_remove_view(GvViewLink *link, GvViewArea *view)
{
    GList *list;

    list = g_list_find(link->views, view);
    if (list == NULL)
    {
	g_warning("gv_view_link_remove_view(): view not linked.");
	return;
    }

    link->views = g_list_remove_link(link->views, list);
    
    gtk_signal_disconnect_by_data(GTK_OBJECT(view), GTK_OBJECT(link));
    gtk_object_unref(GTK_OBJECT(view));
}

void
gv_view_link_enable(GvViewLink *link)
{
    link->enabled = TRUE;
}

void
gv_view_link_disable(GvViewLink *link)
{
    GList *list;

    link->enabled = FALSE;
    list = link->views;
    while (list)
    {
	GvViewArea *view = (GvViewArea*)list->data;
	gv_view_area_queue_cursor_draw(view,FALSE,0.0,0.0);
	list = g_list_next(list);
    }
}

/**************************************************************/

static int
gv_view_link_copy_state(GvViewLink *link, 
                        GvViewArea *src_view, 
                        GvViewArea *dst_view)

{
    GvViewAreaState  state;
    GvRasterLayer   *src_raster, *dst_raster;
    double	x, y, xo, yo;
    gvgeocoord       f_xo, f_yo;

    state = src_view->state;
    x = -state.tx;
    y = -state.ty;
    gv_view_area_map_pointer( src_view, 0.0, 0.0, &f_xo, &f_yo );
    xo = f_xo;
    yo = f_yo;
    if (gv_view_area_get_primary_raster(src_view) != NULL)
    {
        src_raster = 
                GV_RASTER_LAYER(gv_view_area_get_primary_raster(src_view));
        if( gv_view_area_get_raw(src_view,NULL) )
        {
            /* If no cursor-link transformation has been set, these default to
             * the usual gv_raster_pixel_to_georef.
             */
            gv_raster_pixel_to_georefCL( src_raster->prototype_data, &x, &y, NULL );
            gv_raster_pixel_to_georefCL( src_raster->prototype_data, &xo, &yo, NULL);
        }
        else if ( src_raster->prototype_data->poly_orderCL > -1 )
        {
            /* Only need the georef->pixel->georefCL transformation if specific
             * cursor-link transformations have been set.
             */
            gv_raster_georef_to_pixel( src_raster->prototype_data, &x, &y, NULL );
            gv_raster_georef_to_pixel( src_raster->prototype_data, &xo, &yo, NULL);
            gv_raster_pixel_to_georefCL( src_raster->prototype_data, &x, &y, NULL );
            gv_raster_pixel_to_georefCL( src_raster->prototype_data, &xo, &yo, NULL);
        }

    }
    
    if( gv_view_area_get_projection(src_view) != NULL
        && gv_view_area_get_projection(dst_view) != NULL
        && !EQUAL(gv_view_area_get_projection(src_view),
                  gv_view_area_get_projection(dst_view)) 
        && !EQUAL(gv_view_area_get_projection(src_view),"PIXEL")
        && !EQUAL(gv_view_area_get_projection(dst_view),"PIXEL") )
    {
        if( !gv_reproject_points( gv_view_area_get_projection(src_view),
                                  gv_view_area_get_projection(dst_view),
                                  1, &x, &y, NULL ) )
        {
            CPLDebug( "GvViewLink", "gv_reproject_points(%s,%s) failed.", 
                      gv_view_area_get_projection(src_view), 
                      gv_view_area_get_projection(dst_view) );
            return FALSE;
        }
        gv_reproject_points( gv_view_area_get_projection(src_view),
                             gv_view_area_get_projection(dst_view),
                             1, &xo, &yo, NULL );
    }

    if (gv_view_area_get_primary_raster(dst_view) != NULL)
    {
        dst_raster = 
                GV_RASTER_LAYER(gv_view_area_get_primary_raster(dst_view));
        if( gv_view_area_get_raw(dst_view,NULL) )
        {
            gv_raster_georef_to_pixelCL( dst_raster->prototype_data, &x, &y, NULL );
            gv_raster_georef_to_pixelCL( dst_raster->prototype_data, &xo, &yo, NULL);
        }
        else if ( dst_raster->prototype_data->poly_orderCL > -1 )
        {
            gv_raster_georef_to_pixelCL( dst_raster->prototype_data, &x, &y, NULL );
            gv_raster_georef_to_pixelCL( dst_raster->prototype_data, &xo, &yo, NULL);
            gv_raster_pixel_to_georef( dst_raster->prototype_data, &x, &y, NULL );
            gv_raster_pixel_to_georef( dst_raster->prototype_data, &xo, &yo, NULL);

        }

    }

    state.tx = -x;
    state.ty = -y;

/* -------------------------------------------------------------------- */
/*      Try to set the zoom level so that the same region from the      */
/*      source view will be seen in the destination view, even if it    */
/*      is a different size.  Try to account for rotation.              */
/* -------------------------------------------------------------------- */

    state.linear_zoom = 0.5*sqrt((dst_view->state.shape_y*dst_view->state.shape_y +
                                  dst_view->state.shape_x*dst_view->state.shape_x)/
                                 ((yo-y)*(yo-y) + (xo-x)*(xo-x)));  
 
    state.zoom = log(state.linear_zoom) / log(2.0);

/* -------------------------------------------------------------------- */
/*      The shape is view specific, and should not be updated.  The     */
/*      flip value is less obvious, but if one view is                  */
/*      georeferenced, and the other is "raw" we don't want the flip    */
/*      values propagating and screwing stuff up.  This may require     */
/*      future review.                                                  */
/* -------------------------------------------------------------------- */
    state.shape_x = dst_view->state.shape_x;
    state.shape_y = dst_view->state.shape_y;

    state.flip_x = dst_view->state.flip_x;
    state.flip_y = dst_view->state.flip_y;

    /* we could try to derive a rough rotation from the change in the
       direction of the (x,y) to (xd,yd) vector */
    gv_view_area_set_state( dst_view, &state );
    return TRUE;
}

/**************************************************************/

static void
gv_view_link_view_state_changed(GvViewLink *link, GvViewArea *view)
{
    GList *list;
    int x, y;
    double geo_xd,geo_yd;
    
    if (! link->enabled) return;
    if (link->blocked) return;

    /* Copy the view state to the other views in the list */
    list = link->views;
    link->blocked = TRUE;

    /* ghost cursor needs to be reset because motion notify not always done! */
    gtk_widget_get_pointer(GTK_WIDGET(view), &x, &y);
    geo_xd = (double)x;
    geo_yd = (double)y;
    gv_view_link_cursor_get_geo_x_y(&geo_xd, &geo_yd, view);

    while (list)
    {
	GvViewArea *other_view = (GvViewArea*)list->data;
	if (other_view != view)
	{
	    gv_view_link_copy_state(link, view, other_view);
	    other_view->next_valid = 1;
	    other_view->next_x = geo_xd;
	    other_view->next_y = geo_yd;
	}	
	list = g_list_next(list);
    }
    
    link->blocked = FALSE;
}

/* Ghost cursor-related functions */
static gboolean
gv_view_link_cursor_motion_notify(GvViewLink *link,GdkEventMotion *event, GvViewArea *view)
{    

    if (link->blocked) return FALSE; /* don't think this is necessary, cause of interrupt processing */
    if (link->cursor_mode == GV_LINK_CURSOR_OFF) return FALSE;
    gv_view_link_cursor_set_x_y(link,event->x,event->y,view);
    return FALSE;
}

static gboolean gv_view_link_cursor_enter_notify(GvViewLink *link,GdkEventCrossing *event,GvViewArea *view)
{

    if (link->blocked) return FALSE; /* don't think this is necessary, cause of interrupt processing */
    if (link->cursor_mode == GV_LINK_CURSOR_OFF) return FALSE;
    gv_view_link_cursor_set_x_y(link,event->x,event->y,view);
    return FALSE;
}

static gboolean gv_view_link_cursor_leave_notify(GvViewLink *link,GdkEventCrossing *event,GvViewArea *view)
{
    GList *list;

    if (link->blocked) return FALSE; /* don't think this is necessary, cause of interrupt processing */
    if (link->cursor_mode == GV_LINK_CURSOR_OFF) return FALSE;

    list = link->views;
    while (list)
    {
	GvViewArea *other_view = (GvViewArea*)list->data;
	/* ALL views should be redrawn on leave event, to avoid artifacts in logical cursor case */
	gv_view_area_queue_cursor_draw(other_view,FALSE,0.0,0.0);
	list = g_list_next(list);
    }
    return FALSE;
}

static void
gv_view_link_cursor_get_geo_x_y(double *event_x, double *event_y, GvViewArea *view)
{
    GvRasterLayer *src_raster;
    gvgeocoord geo_x, geo_y;
    double geo_xd,geo_yd;

    gv_view_area_map_pointer(view, *event_x, *event_y,
			     &geo_x, &geo_y);

    /* hack to get around the fact that some things expect double, others gvgeocoord */
    geo_xd=(double) geo_x;
    geo_yd=(double) geo_y;
    if (gv_view_area_get_primary_raster(view) != NULL)
    {
        src_raster = GV_RASTER_LAYER(gv_view_area_get_primary_raster(view));
        if( gv_view_area_get_raw(view,NULL) )
        {
            gv_raster_pixel_to_georefCL( src_raster->prototype_data, &geo_xd, &geo_yd, NULL );
        }
        else if ( src_raster->prototype_data->poly_orderCL > -1 )
        {
            /* convert from display georeferenced coordinates to pixel coordinates to */
            /* link-cursor georeferenced coordinates                                  */

            gv_raster_georef_to_pixel( src_raster->prototype_data, &geo_xd, &geo_yd, NULL );
            gv_raster_pixel_to_georefCL( src_raster->prototype_data, &geo_xd, &geo_yd, NULL );
        }
    }
    *event_x = geo_xd;
    *event_y = geo_yd;
}

static void
gv_view_link_cursor_set_x_y(GvViewLink *link, double event_x, double event_y, GvViewArea *view)
{
    GList *list;
    double geo_xd,geo_yd;
    
    
    geo_xd = event_x;
    geo_yd = event_y;
    gv_view_link_cursor_get_geo_x_y(&geo_xd, &geo_yd, view);
    link->src_view = view;

    list = link->views;
    while (list)
    {
	GvViewArea *other_view = (GvViewArea*)list->data;
	if(other_view != view) {
	  gv_view_area_queue_cursor_draw(other_view,TRUE,geo_xd,geo_yd);
        }
	list = g_list_next(list);
    }


}


static void
gv_view_link_cursor_draw(GvViewLink *link, GvViewArea *view)
{
    gvgeocoord dx, dy, x, y;
    GvRasterLayer   *dst_raster;
    double xdouble, ydouble;

    /* Uncomment next line to get rid of cursor in non-linked case */
    /*if (! link->enabled) return;*/
    if (link->cursor_mode == GV_LINK_CURSOR_OFF) return;
    
    /* Don't draw cursor if application has just started up (src_view */
    /* is NULL) */
    if (link->src_view == NULL) return;

    dx = 6.0;  /* Later should make this flexible */
    dy = 0.0;

    gv_view_area_correct_for_transform(view, dx, dy, &dx, &dy);    

    if(view->next_valid) {
      xdouble = (double) view->next_x;
      ydouble = (double) view->next_y;

      if( gv_view_area_get_projection(link->src_view) != NULL
	  && gv_view_area_get_projection(view) != NULL
	  && !EQUAL(gv_view_area_get_projection(link->src_view),
		    gv_view_area_get_projection(view)) 
	  && !EQUAL(gv_view_area_get_projection(link->src_view),"PIXEL")
	  && !EQUAL(gv_view_area_get_projection(view),"PIXEL") )
	{
	  if( !gv_reproject_points( gv_view_area_get_projection(link->src_view),
				    gv_view_area_get_projection(view),
				    1, &xdouble, &ydouble, NULL ) )
	    {
	      CPLDebug( "GvViewLink", "gv_reproject_points(%s,%s) failed.", 
			gv_view_area_get_projection(link->src_view), 
			gv_view_area_get_projection(view) );
	    }
	}
      if (gv_view_area_get_primary_raster(view) != NULL)
	{
  	  dst_raster = 
                GV_RASTER_LAYER(gv_view_area_get_primary_raster(view));

	  if( gv_view_area_get_raw(view,NULL) )
	    {
	      gv_raster_georef_to_pixelCL( dst_raster->prototype_data, &xdouble, &ydouble, NULL );
	    }
          else if ( dst_raster->prototype_data->poly_orderCL > -1 )
          {
	      gv_raster_georef_to_pixelCL( dst_raster->prototype_data, &xdouble, &ydouble, NULL );
	      gv_raster_pixel_to_georef( dst_raster->prototype_data, &xdouble, &ydouble, NULL );
          }
	} 
      x = (gvgeocoord) xdouble;
      y = (gvgeocoord) ydouble;
    } else {
      x = 0.0;
      y = 0.0;
    }

    if (link->cursor_mode == GV_LINK_CURSOR_ON_LOGICAL)
    {
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
    }
    else
    {
        glDrawBuffer(GL_FRONT_LEFT);
    }

    /*glRenderMode(GL_RENDER);*/
    if (link->cursor_mode == GV_LINK_CURSOR_ON_LOGICAL)
        glColor3f(1.0,1.0,1.0);
    else
        glColor3f(0.0,1.0,0.0);

    /* Draw crosshairs */
    glBegin(GL_LINES);

    if ((link->cursor_mode == GV_LINK_CURSOR_ON_LOGICAL) && (view->last_valid))
    {
        glVertex2f(view->last_x-dx, view->last_y-dy);
        glVertex2f(view->last_x+dx, view->last_y+dy);
        glVertex2f(view->last_x+dy, view->last_y-dx);
        glVertex2f(view->last_x-dy, view->last_y+dx);
    }

    if (view->next_valid)
    {
        glVertex2f(x-dx, y-dy);
        glVertex2f(x+dx, y+dy);
        glVertex2f(x+dy, y-dx);
        glVertex2f(x-dy, y+dx);
    }
    glEnd();

    if (link->cursor_mode == GV_LINK_CURSOR_ON_LOGICAL)
    {
        glDisable(GL_COLOR_LOGIC_OP);
        gv_view_area_swap_buffers(view); 
        if (view->next_valid)
        {   
            view->last_x = x;
            view->last_y = y;
            view->last_valid = TRUE;
        }
        else 
            view->last_valid = FALSE;
    }
    else
    {
        glFinish();
        glDrawBuffer(GL_BACK);
        /*gtk_gl_area_swap_buffers(GTK_GL_AREA(view));*/    
    }
    

}

void
gv_view_link_set_cursor_mode(GvViewLink *link, int cursor_mode)
{
    GList *list;

    if (link->cursor_mode != GV_LINK_CURSOR_OFF)
    {
        /* If cursor wasn't off, make sure no artifacts are left */
        list = link->views;
        while (list)
        {
	    GvViewArea *other_view = (GvViewArea*)list->data;
	    /* ALL views should be redrawn on leave event, to avoid artifacts in logical cursor case */
	    gv_view_area_queue_cursor_draw(other_view,FALSE,0.0,0.0);
	    list = g_list_next(list);
        }  
    }

    if (cursor_mode == GV_LINK_CURSOR_OFF)
        link->cursor_mode = GV_LINK_CURSOR_OFF;
    else if (cursor_mode == GV_LINK_CURSOR_ON_DEFAULT)
        link->cursor_mode = GV_LINK_CURSOR_ON_DEFAULT;
    else if (cursor_mode == GV_LINK_CURSOR_ON_LOGICAL)
        link->cursor_mode = GV_LINK_CURSOR_ON_LOGICAL;
    else
        g_warning("set_gv_view_link_cursor_mode(): invalid cursor mode");
           
}      

/* End of ghost cursor-related functions */

static void
gv_view_link_destroy(GtkObject *object)
{
    GtkObjectClass *parent_class;
    GvViewLink *link;

    link = GV_VIEW_LINK(object);

    /* Remove all views */
    while (link->views)
    {
	GvViewArea *view = (GvViewArea*)link->views->data;
	gv_view_link_remove_view(link, view);
    }

    /* ---- Call parent class function ---- */ 
    parent_class = gtk_type_class(gtk_object_get_type());
    GTK_OBJECT_CLASS(parent_class)->destroy(object);


    /* GTK2 PORT...

       I don't see how gtk_gl_area here could have been right...

    parent_class = gtk_type_class(gtk_gl_area_get_type());
    GTK_OBJECT_CLASS(parent_class)->destroy(object);
    */
}

