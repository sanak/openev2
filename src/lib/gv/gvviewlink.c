/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Manage linked GvViewAreas.
 * Author:   OpenEV Team
 * Maintainer: Mario Beauchamp, starged@gmail.com
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
 */

#include "gvviewlink.h"
#include "gvrasterlayer.h"
#include "gvutils.h"

static void gv_view_link_class_init(GvViewLinkClass *klass);
static void gv_view_link_init(GvViewLink *link);
static void gv_view_link_view_state_changed(GvViewLink *link, GvViewArea *view);
static void gv_view_link_dispose(GObject *object);

/* Ghost cursor stuff */
static void gv_view_link_cursor_draw(GvViewLink *link, GvViewArea *view);

static gboolean gv_view_link_cursor_motion_notify(GvViewLink *link,GdkEventMotion *event, GvViewArea *view);
static gboolean gv_view_link_cursor_leave_notify(GvViewLink *link, GdkEventCrossing *event, GvViewArea *view);
static gboolean gv_view_link_cursor_enter_notify(GvViewLink *link, GdkEventCrossing *event, GvViewArea *view);
static void gv_view_link_cursor_set_x_y(GvViewLink *link, double x, double y, GvViewArea *view);
static void gv_view_link_cursor_get_geo_x_y(double *event_x, double *event_y, GvViewArea *view);

static GObjectClass *parent_class = NULL;

GType
gv_view_link_get_type(void)
{
    static GType view_link_type = 0;

    if (!view_link_type) {
        static const GTypeInfo view_link_info =
        {
            sizeof(GvViewLinkClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_view_link_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvViewLink),
            0,
            (GInstanceInitFunc) gv_view_link_init,
        };
        view_link_type = g_type_register_static (G_TYPE_OBJECT,
                                          "GvViewLink",
                                          &view_link_info, 0);
        }

    return view_link_type;
}

static void
gv_view_link_class_init(GvViewLinkClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    object_class->dispose = gv_view_link_dispose;
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

GObject *
gv_view_link_new(void)
{
    GvViewLink *vlink = g_object_new(GV_TYPE_VIEW_LINK, NULL);

    return G_OBJECT(vlink);
}

void
gv_view_link_register_view(GvViewLink *link, GvViewArea *view)
{
    /* Add a reference to this view */
    g_object_ref(view);

    /* Add view to registered views list */
    link->views = g_list_prepend(link->views, view);

    /* Connect to the view state changed event */
    g_signal_connect_swapped(view, "view-state-changed",
                            G_CALLBACK(gv_view_link_view_state_changed),
                            link);

    /* Connect to view area delete event */
    g_signal_connect_swapped(view, "destroy",
                            G_CALLBACK(gv_view_link_remove_view),
                            link);

    /* Next two connections are only needed for ghost cursor drawing */
    /* Connect to view area glcursor signal */
    g_signal_connect_swapped(view, "glcursor",
                            G_CALLBACK(gv_view_link_cursor_draw),
                            link);

    /* Connect to motion event */
    g_signal_connect_swapped(view, "motion-notify-event",
                            G_CALLBACK(gv_view_link_cursor_motion_notify),
                            link);

    /* Connect to view area leave signal  */
    g_signal_connect_swapped(view, "leave-notify-event",
                            G_CALLBACK(gv_view_link_cursor_leave_notify),
                            link);

    /* Connect to view area enter signal  */
    g_signal_connect_swapped(view, "enter-notify-event",
                            G_CALLBACK(gv_view_link_cursor_enter_notify),
                            link);
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

    g_signal_handlers_disconnect_matched (view, G_SIGNAL_MATCH_DATA,
                                            0, 0, NULL, NULL, link);
    g_object_unref(view);
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
    double      x, y, xo, yo;
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

static gboolean
gv_view_link_cursor_enter_notify(GvViewLink *link,GdkEventCrossing *event,GvViewArea *view)
{
    if (link->blocked) return FALSE; /* don't think this is necessary, cause of interrupt processing */
    if (link->cursor_mode == GV_LINK_CURSOR_OFF) return FALSE;
    gv_view_link_cursor_set_x_y(link,event->x,event->y,view);
    return FALSE;
}

static gboolean
gv_view_link_cursor_leave_notify(GvViewLink *link,GdkEventCrossing *event,GvViewArea *view)
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
gv_view_link_dispose(GObject *object)
{
    GvViewLink *link = GV_VIEW_LINK(object);

    /* Remove all views */
    while (link->views)
    {
        GvViewArea *view = GV_VIEW_AREA(link->views->data);
        gv_view_link_remove_view(link, view);
    }

    /* ---- Call parent class function ---- */ 
    G_OBJECT_CLASS(parent_class)->dispose(object);
}
