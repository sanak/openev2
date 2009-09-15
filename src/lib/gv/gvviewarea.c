/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  GTK/OpenGL View Canvas
 * Author:   OpenEV Team
 * Maintainer: Mario Beauchamp, starged@gmail.com
 *
 ******************************************************************************
 * New GTK2 GL Usage:
 *
 * // OpenGL BEGIN
 * if (!gv_view_area_gl_begin(view_area))
 *   return FALSE;
 *
 * Note: To both make current and begin, use gv_view_area_begin instead:
 * // OpenGL BEGIN
 * if (!gv_view_area_gl_begin(view_area))
 *   return FALSE;
 *
 * ...
 *
 * < GL stuff, including glBegin(), glEnd(), not to be confused with
 * gv_view_area_gl_begin/end, as these are unrelated >
 *
 * ...
 *
 * gv_view_area_swap_buffers (view_area);
 *   or
 * glFlush();
 *   as needed
 *
 * gv_view_area_gl_end (gldrawable);
 * // OpenGL END
 *
 ******************************************************************************
 * Copyright (c) 2004, Atlantis Scientific Inc. (www.atlsci.com)
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

#include "gvmarshal.h"
#include "gvviewarea.h"
#include "gvlayer.h"
#include "gvrasterlayer.h"
#include "gvundo.h"
#include "gvmanager.h"
#include "gvutils.h"
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#include "gextra.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#define DEG2RAD         0.01745329252
#define LOG2(x)         (log(x) / 0.69314718056)
#define DEAD_ZONE       8.0
#define DRAG_THRESHOLD  10
#define WHEEL_ZOOM_INC  0.1

/* 3D motion Stuff */
#define MOVE_SPEED      0.04
#define AZ_SCALE        0.15
#define EL_SCALE        0.15
#define HEIGHT_SCALE    0.1
#define CONTIN_3DMOVE_INC 20.0

/* 2D Zooming */
#define CONTIN_ZOOM_INC 1.5
#define ZOOM_STEP       1

/* useful 3D vector copy function */
#define vec_copy(s,d) {d[0]=s[0];d[1]=s[1];d[2]=s[2];}
#define vec_add(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
#define vec_scale(a,b,c) {c[0]=b*a[0];c[1]=b*a[1];c[2]=b*a[2];}

#define VIEW_EVENT_MASK (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | \
             GDK_POINTER_MOTION_MASK | \
             GDK_POINTER_MOTION_HINT_MASK | \
             GDK_KEY_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | \
             GDK_LEAVE_NOTIFY_MASK)

enum
{
    GLDRAW,
    GLCURSOR,
    ACTIVE_CHANGED,
    VIEW_STATE_CHANGED,
    LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_HADJUSTMENT,
  ARG_VADJUSTMENT,
};

static void gv_view_area_class_init(GvViewAreaClass *klass);
static void gv_view_area_init(GvViewArea *view);
static void gv_view_area_finalize( GObject *gobject );
static void gv_view_area_destroy(GtkObject *object);
static void gv_view_area_update_adjustments( GvViewArea *view );
static gint gv_view_area_configure(GtkWidget *view, GdkEventConfigure *event);
static gint gv_view_area_reset_projection(GvViewArea *view, gvgeocoord width, gvgeocoord height);
static void gv_view_area_realize(GtkWidget *view);
static void gv_view_area_unrealize(GtkWidget *view);
static gboolean gv_view_area_motion_handle_hint(GtkWidget *view, GdkEventMotion *event);
static gint gv_view_area_motion_notify(GtkWidget *view, GdkEventMotion *event);
static gint gv_view_area_button_press(GtkWidget *view, GdkEventButton *event);
static gint gv_view_area_button_release(GtkWidget*view, GdkEventButton *event);
static gint gv_view_area_key_press(GtkWidget *view, GdkEventKey *event);
static void gv_view_area_change_notify(GvViewArea *view, gpointer info);
static void gv_view_area_state_changed(GvViewArea *view);
static void gv_view_area_fit_layer(GvViewArea *view, GvLayer *layer);
static void gv_view_area_fit_extents_3D(GvViewArea *view,
                                     gvgeocoord llx, gvgeocoord lly, gvgeocoord minz,
                                     gvgeocoord width, gvgeocoord height, gvgeocoord deltaz );
static gint gv_view_area_zoompan_handler(gpointer data );
static GdkGLContext* gv_view_area_get_share_list(GvViewArea *view);
static PangoFontDescription *gv_view_XLFD_to_pango(gchar *XLFD_name);

static GtkDrawingAreaClass *parent_class = NULL;
static guint view_area_signals[LAST_SIGNAL] = { 0 };

static void  gv_view_area_set_arg        (GtkObject      *object,
                                          GtkArg         *arg,
                                          guint           arg_id);
static void  gv_view_area_get_arg        (GtkObject      *object,
                                          GtkArg         *arg,
                                          guint           arg_id);

/* GTK2 PORT - The following prototypes were missing, added 'static'... */
static void vec_point(vec3_t point, gvgeocoord az, gvgeocoord el);
static int inv_vec_point(vec3_t point, gvgeocoord *az, gvgeocoord *el);
static void vec_near_far_range( vec3_t point, double *volume,
                                double *min, double *max );

static struct { char *gvname, *gdkname; } bmfontmap[] =
{
    { "Fixed", "-*-fixed-*-*-*-*-*-*-*-*-*-*-iso8859-*" },
    { "Times, 20pt", "-*-times-*-*-*-*-20-*-*-*-*-*-iso8859-*" },
    { "Times, 14pt", "-*-times-*-*-*-*-14-*-*-*-*-*-iso8859-*" },
    { NULL, NULL }  /* Sentinel */
};

/* Incremented each render */
static int render_counter = 0;

static void
vec_point(vec3_t point, gvgeocoord az, gvgeocoord el)
{
    gvgeocoord c = cos(el * DEG2RAD);

    point[0] = c * cos(az * DEG2RAD);
    point[1] = c * sin(az * DEG2RAD);
    point[2] = sin(el * DEG2RAD);
}

/* Inverse of vec_point, given direction vector find az and el, az in [-180, 180] */
static int
inv_vec_point(vec3_t point, gvgeocoord *az, gvgeocoord *el)
{
    gvgeocoord c, az1, az2;

    *el = asin(point[2]) / DEG2RAD;
    c = cos( *el * DEG2RAD);
    if ((c < 0.000001) &&  (c > -0.000001))
        c = 0.00001;
    az1 = (asin(point[1]/c)) / DEG2RAD;
    az2 = (acos(point[0]/c)) / DEG2RAD;

    if ((point[0] >= 0.0) && (point[1] >= 0.0))
        *az = az1;
    else if ((point[0] >= 0.0) && (point[1] <= 0.0))
        *az = az1;
    else if ((point[0] <= 0.0) && (point[1] >= 0.0))
        *az = az2;
    else if ((point[0] <= 0.0) && (point[1] <= 0.0))
        *az = -az2;
    else
    {
        printf("ERROR!  in gvviewarea.c, inv_vec_point.  Should never reach here.\n");
        /* printf("point[0] %f, point[1] %f, c %f, az1 %f, az2 %f\n", point[0], point[1], c, az1, az2); */
        return 0;
    }

    return 1;

}


static void
vec_near_far_range( vec3_t point, double *volume, double *min, double *max )

{
    vec3_t   nearest, furthest;

    /* Find nearest X in volume */
    if( point[0] < volume[0] )
        nearest[0] = volume[0];
    else if( point[0] > volume[1] )
        nearest[0] = volume[1];
    else
        nearest[0] = point[0];

    /* Find nearest Y in volume */
    if( point[1] < volume[2] )
        nearest[1] = volume[2];
    else if( point[1] > volume[3] )
        nearest[1] = volume[3];
    else
        nearest[1] = point[1];

    /* Find nearest Z in volume */
    if( point[2] < volume[4] )
        nearest[1] = volume[4];
    else if( point[2] > volume[5] )
        nearest[2] = volume[5];
    else
        nearest[2] = point[2];

    /* Find furthest X in volume */
    if( ABS(point[0] - volume[0]) > ABS(point[0] - volume[1]) )
        furthest[0] = volume[0];
    else
        furthest[0] = volume[1];

    /* Find furthest Y in volume */
    if( ABS(point[1] - volume[2]) > ABS(point[1] - volume[3]) )
        furthest[1] = volume[2];
    else
        furthest[1] = volume[3];

    /* Find furthest Z in volume */
    if( ABS(point[2] - volume[4]) > ABS(point[2] - volume[5]) )
        furthest[2] = volume[4];
    else
        furthest[2] = volume[5];

    /* find distances */

    *min = sqrt( (nearest[0] - point[0]) * (nearest[0] - point[0])
                 + (nearest[1] - point[1]) * (nearest[1] - point[1])
                 + (nearest[2] - point[2]) * (nearest[2] - point[2]) );

    *max = sqrt( (furthest[0] - point[0]) * (furthest[0] - point[0])
                 + (furthest[1] - point[1]) * (furthest[1] - point[1])
                 + (furthest[2] - point[2]) * (furthest[2] - point[2]) );

}

GType gv_view_area_get_type()
{
  static GType view_area_type = 0;

  if (!view_area_type)
    {
      static const GTypeInfo view_area_info =
      {
        sizeof (GvViewAreaClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gv_view_area_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (GvViewArea),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) gv_view_area_init,
      };

      view_area_type = g_type_register_static (GTK_TYPE_DRAWING_AREA,
                                            "GvViewArea",
                                            &view_area_info, 0);
    }

    return view_area_type;
}

static void
gv_view_area_class_init(GvViewAreaClass *klass)
{
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    parent_class = g_type_class_peek_parent (klass);

    view_area_signals[GLDRAW] =
      g_signal_new ("gldraw",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvViewAreaClass, gldraw),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    view_area_signals[GLCURSOR] =
      g_signal_new ("glcursor",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvViewAreaClass, glcursor),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    view_area_signals[ACTIVE_CHANGED] =
      g_signal_new ("active-changed",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvViewAreaClass, active_changed),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    view_area_signals[VIEW_STATE_CHANGED] =
      g_signal_new ("view-state-changed",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvViewAreaClass, view_state_changed),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    /* ---- Override finalize ---- */
    G_OBJECT_CLASS(klass)->finalize = gv_view_area_finalize;

    object_class->destroy = gv_view_area_destroy;
    object_class->set_arg = gv_view_area_set_arg;
    object_class->get_arg = gv_view_area_get_arg;

    widget_class->realize = gv_view_area_realize;
    widget_class->unrealize = gv_view_area_unrealize;
    widget_class->configure_event = gv_view_area_configure;
    widget_class->expose_event = gv_view_area_expose;
    widget_class->button_press_event = gv_view_area_button_press;
    widget_class->button_release_event = gv_view_area_button_release;
    widget_class->motion_notify_event = gv_view_area_motion_notify;
    widget_class->key_press_event = gv_view_area_key_press;

    widget_class->set_scroll_adjustments_signal =
      g_signal_new ("set_scroll_adjustments",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_LAST,
                    G_STRUCT_OFFSET (GvViewAreaClass, set_scroll_adjustments),
                    NULL, NULL,
                    gvmarshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
                    GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

    klass->set_scroll_adjustments = gv_view_area_set_adjustments;
    klass->gldraw = NULL;
    klass->active_changed = NULL;
    klass->view_state_changed = NULL;
}

static GdkGLContext *_share_list = NULL;

static void
gv_view_area_init(GvViewArea *view)
{
    GdkGLConfig *glconfig = gdk_gl_config_new_by_mode ( GDK_GL_MODE_RGBA
                                                      | GDK_GL_MODE_DEPTH
                                                      | GDK_GL_MODE_STENCIL
                                                      | GDK_GL_MODE_DOUBLE
                                                      );

    /* ---- Enable GL capability ---- */
    gtk_widget_set_gl_capability (GTK_WIDGET(view), glconfig,
                                  _share_list, TRUE, GDK_GL_RGBA_TYPE);

    view->state.tx = view->state.ty = view->state.rot = 0.0;
    view->state.zoom = 0.0;
    view->state.linear_zoom = 1.0;
    view->state.flip_x = view->state.flip_y = 1.0;
    view->state.shape_x = view->state.shape_y = 0.0;
    view->volume_current = FALSE;
    view->linear_measure = 1000.0;

    /* 3D Specific stuff */
    view->state.eye_pos[0] = 0.0;
    view->state.eye_pos[1] = 0.0;
    view->state.eye_pos[2] = 500.0;
    vec_point(view->state.eye_dir, 90.0, -45.0); /* rot = 90,  tilt 45 deg down */
    view->state.z_scale = 1.0;

    view->exact_render = FALSE;

    view->flag_3d = 0;    /* 0=2D, 1=3D, default 2D */

    view->bmfonts = g_array_new(FALSE,TRUE,sizeof(GvBMFontInfo));

    view->background[0] = view->background[1] = view->background[2] = 0.0;
    view->background[3] = 1.0;

    view->layers = NULL;
    view->projection = NULL;

    view->last_button = 0;
    view->dragging_mode = FALSE;
    view->display_dirty = TRUE;

    /* ghost cursor stuff */
    view->next_valid = FALSE;
    view->last_valid = FALSE;
    view->next_x = 0.0;
    view->next_y = 0.0;
    view->last_x = 0.0;
    view->last_y = 0.0;
    /* end of ghost cursor stuff */

    view->last_draw_time = 0.0;
    view->redraw_timer = NULL;

    view->vadj = NULL;
    view->hadj = NULL;
    view->lock_adjustments = FALSE;

    view->properties = NULL;

    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(view), GTK_CAN_FOCUS);

    gtk_widget_set_events(GTK_WIDGET(view),
              gtk_widget_get_events(GTK_WIDGET(view)) |
              VIEW_EVENT_MASK);

    /* Need to handle motion hints BEFORE other handlers */
    g_signal_connect(G_OBJECT(view), "motion-notify-event",
               G_CALLBACK(gv_view_area_motion_handle_hint), NULL);
}

GtkWidget *gv_view_area_new()
{
    GvViewArea *view;

    /* SD check that geometric data type are in sync with OpenGL */
    g_assert(sizeof(gvgeocoord) == sizeof(GLgeocoord));

    view = g_object_new(GV_TYPE_VIEW_AREA, NULL);

    return GTK_WIDGET(view);
}

/**
 * Get a global GdkGLContext that can be used to share
 * OpenGL display lists between multiple drawables with
 * dynamic lifetimes.  Borrowed from GtkGLExt problem list,
 * search on "gtkglext get_share_list" for discussion.
 */
static GdkGLContext* gv_view_area_get_share_list(GvViewArea *view)
{
   static GdkGLContext* share_list = NULL;
   GdkPixmap* pixmap;
   GdkGLPixmap* glpixmap = NULL;
   GdkGLConfig* config;


   if(!share_list) {
     config = gdk_gl_config_new_by_mode
       (GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH  | GDK_GL_MODE_DOUBLE);
     if (config == NULL) {
         g_print ("*** Cannot find the double-buffered visual.\n");
         g_print ("*** Trying single-buffered visual.\n");
         config = gdk_gl_config_new_by_mode
             (GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH);
         if (config == NULL) {
             return NULL;
         }
     }
     pixmap = gdk_pixmap_new(0, 8, 8, gdk_gl_config_get_depth(config));
     if (pixmap != NULL) {
       glpixmap = gdk_pixmap_set_gl_capability(pixmap, config, 0);
     }
     if (glpixmap == NULL) {

         g_print("get share list, trying again..\n");

         pixmap = gdk_pixmap_new
             (GDK_DRAWABLE((GTK_WIDGET(view))->window), 8, 8, -1);
         glpixmap = gdk_pixmap_set_gl_capability(pixmap, config, 0);
     }

     share_list = gdk_gl_context_new
       (gdk_pixmap_get_gl_drawable(pixmap), NULL, TRUE, GDK_GL_RGBA_TYPE);
   }
   return share_list;
}

/**
 * Makes current and delimits beginning of OpenGL execution
 *
 * Returns TRUE if successful, FALSE otherwise
 */
gint gv_view_area_begin(GvViewArea *view)
{
  GdkGLContext *glcontext;
  GdkGLDrawable *gldrawable;
  gint ret_val;

  g_return_val_if_fail(GTK_WIDGET_REALIZED(view), FALSE);

  glcontext = gtk_widget_get_gl_context(GTK_WIDGET(view));
  gldrawable = GDK_GL_DRAWABLE(gtk_widget_get_gl_drawable(GTK_WIDGET(view)));

  if (!gdk_gl_drawable_make_current(gldrawable, glcontext)) {
    return FALSE;
  }

  ret_val = gdk_gl_drawable_gl_begin(gldrawable, glcontext);

  return ret_val;
}

/**
 * Get GL context for view area
 */
GdkGLContext *gv_view_area_get_gl_context(GvViewArea *view)
{
  return gtk_widget_get_gl_context(GTK_WIDGET(view));
}

/**
 * Prepare GL drawing
 */
gint gv_view_area_make_current(GvViewArea *view)
{
  GdkGLContext *glcontext;
  GdkGLDrawable *gldrawable;

  glcontext = gtk_widget_get_gl_context(GTK_WIDGET(view));
  gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(view));

  return gdk_gl_drawable_make_current(gldrawable, glcontext);
}

/**
 * Exchange GL front and back buffers
 */
void gv_view_area_swap_buffers(GvViewArea *view)
{
  gdk_gl_drawable_swap_buffers
    (gtk_widget_get_gl_drawable(GTK_WIDGET(view)));
}

/**
 * Delimits beginning of OpenGL execution
 *
 * Returns TRUE if successful, FALSE otherwise
 */
gint gv_view_area_gl_begin(GvViewArea *view)
{
  GdkGLContext *glcontext;
  GdkGLDrawable *gldrawable;

  g_return_val_if_fail(GTK_WIDGET_REALIZED(view), FALSE);

  glcontext = gtk_widget_get_gl_context(GTK_WIDGET(view));
  gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(view));

  return gdk_gl_drawable_gl_begin(gldrawable, glcontext);
}

/**
 * Delimits end of OpenGL execution
 */
void gv_view_area_gl_end(GvViewArea *view)
{
    gdk_gl_drawable_gl_end(gtk_widget_get_gl_drawable(GTK_WIDGET(view)));
}

gint
gv_view_area_set_projection( GvViewArea *view, const char *projection )

{
    if( view->projection != NULL )
        g_free( view->projection );

    view->projection = g_strdup( projection );

    return TRUE;
}

const char *
gv_view_area_get_projection( GvViewArea *view )

{
    return view->projection;
}


/* Set mode to 2D or 3D */
void
gv_view_area_set_mode(GvViewArea *view, int flag_3d)
{
    view->flag_3d= flag_3d;    /* 0=2D, 1=3D,  */

    /* Reset OpenGL Projection Method */
    if( GTK_WIDGET_REALIZED(GTK_WIDGET(view)) )
        gv_view_area_reset_projection(view,
                                      view->state.shape_x,
                                      view->state.shape_y);

    gv_view_area_state_changed(view);
}

int gv_view_area_get_mode(GvViewArea *view)

{
    return view->flag_3d;
}

/* Change 3D Scaling factor - has no effect unless in 3D mode */
void
gv_view_area_height_scale(GvViewArea *view, gvgeocoord scale)
{
    view->state.z_scale = scale;
    gv_view_area_state_changed(view);
}

gvgeocoord
gv_view_area_get_height_scale( GvViewArea *view )

{
    return view->state.z_scale;
}


/* Change 3D view position and direction */
void gv_view_area_set_3d_view( GvViewArea *view,
                               vec3_t eye_pos, vec3_t eye_dir )

{
    int    changed = FALSE;

    if( eye_pos != NULL
        && (eye_pos[0] != view->state.eye_pos[0]
            || eye_pos[1] != view->state.eye_pos[1]
            || eye_pos[2] != view->state.eye_pos[2]) )
    {
        vec_copy( eye_pos, view->state.eye_pos );
        changed = TRUE;
    }

    if( eye_dir != NULL
        && (eye_dir[0] != view->state.eye_dir[0]
            || eye_dir[1] != view->state.eye_dir[1]
            || eye_dir[2] != view->state.eye_dir[2]) )
    {
        gvgeocoord mag = sqrt(eye_dir[0] * eye_dir[0]
                         + eye_dir[1] * eye_dir[1]
                         + eye_dir[2] * eye_dir[2] );

        if( mag != 0.0 )
        {
            vec_scale( eye_dir, 1.0 / mag, view->state.eye_dir );
            changed = TRUE;
        }
    }

    if( changed )
        gv_view_area_state_changed(view);
}

/* Change 3D view position and direction to given location on z-plane */
void gv_view_area_set_3d_view_look_at( GvViewArea *view,
                                       vec3_t eye_pos, gvgeocoord *eye_look_at )
{
    if( eye_look_at != NULL )
    {
        vec3_t eye_dir;
        gvgeocoord mag;

        eye_dir[0] = eye_look_at[0] - eye_pos[0];
        eye_dir[1] = eye_look_at[1] - eye_pos[1];
        eye_dir[2] = -eye_pos[2];

        mag = sqrt(eye_dir[0] * eye_dir[0]
                         + eye_dir[1] * eye_dir[1]
                         + eye_dir[2] * eye_dir[2] );

        if( mag != 0.0 )
        {
            vec_scale( eye_dir, 1.0 / mag, view->state.eye_dir );
            gv_view_area_set_3d_view( view, eye_pos, eye_dir );
        }
    }
    else
    {
        gv_view_area_set_3d_view( view, eye_pos, view->state.eye_dir );
    }
}


/* Get point in z-plane that looking at, false if not meaningful */
gint
gv_view_area_get_look_at_pos( GvViewArea *view,
                              gvgeocoord *x, gvgeocoord *y)
{
    /* Check if looking up above z-plane */
    if (view->state.eye_dir[2] >= 0.0)
    {
        return FALSE;
    } else {

        *x = view->state.eye_pos[0] - view->state.eye_dir[0]
            * (view->state.eye_pos[2]/ view->state.eye_dir[2]);

        *y = view->state.eye_pos[1] - view->state.eye_dir[1]
            * (view->state.eye_pos[2]/ view->state.eye_dir[2]);
    }

    return TRUE;
}

void
gv_view_area_queue_draw(GvViewArea *view)
{
    gv_manager_set_busy( gv_get_manager(), TRUE );

    view->display_dirty = TRUE;

    gtk_widget_queue_draw(GTK_WIDGET(view));
}
void
gv_view_area_queue_cursor_draw(GvViewArea *view, int next_valid, gvgeocoord x, gvgeocoord y)
{
    gv_manager_set_busy( gv_get_manager(), TRUE );
    view->next_valid = next_valid;
    view->next_x = x;
    view->next_y = y;
    gtk_widget_queue_draw(GTK_WIDGET(view));
}

void
gv_view_area_zoom(GvViewArea *view, gvgeocoord zoom)
{
    view->state.zoom += zoom;
    view->state.linear_zoom = pow(2.0, view->state.zoom);

    gv_view_area_state_changed(view);
}

gvgeocoord
gv_view_area_get_zoom(GvViewArea *view)
{
    return view->state.zoom;
}

void
gv_view_area_rotate(GvViewArea *view, gvgeocoord angle)
{
    view->state.rot += angle;
    gv_view_area_state_changed(view);
}

void
gv_view_area_set_rotation(GvViewArea *view, gvgeocoord angle)
{
    view->state.rot = angle;
    gv_view_area_state_changed(view);
}

void
gv_view_area_translate(GvViewArea *view, gvgeocoord dx, gvgeocoord dy)
{
    gvgeocoord cr, sr;

    dx /= view->state.linear_zoom;
    dy /= view->state.linear_zoom;
    cr = cos(-view->state.rot * DEG2RAD);
    sr = sin(-view->state.rot * DEG2RAD);
    view->state.tx += (dx * cr - dy * sr) * view->state.flip_x;
    view->state.ty += (dx * sr + dy * cr) * view->state.flip_y;

    gv_view_area_state_changed(view);
}

void
gv_view_area_set_translation(GvViewArea *view, gvgeocoord x, gvgeocoord y)
{
    view->state.tx = x;
    view->state.ty = y;

    gv_view_area_state_changed(view);
}

int
gv_view_area_get_flip_x( GvViewArea *view )
{
    return view->state.flip_x;
}

int
gv_view_area_get_flip_y( GvViewArea *view )
{
    return view->state.flip_y;
}

void gv_view_area_set_flip_xy( GvViewArea *view, int flip_x, int flip_y )
{
    if( flip_x == view->state.flip_x && flip_y == view->state.flip_y )
        return;

    view->state.flip_x = flip_x;
    view->state.flip_y = flip_y;

    gv_view_area_state_changed(view);
}

void
gv_view_area_copy_state(GvViewArea *view, GvViewArea *copy)
{
    gv_view_area_set_state( view, &(copy->state) );
}

void
gv_view_area_set_state(GvViewArea *view, GvViewAreaState *state)
{
    /* consider preserving mouse position, and re-deriving zoom or log_zoom */

    view->state = *state;

    gv_view_area_state_changed(view);
}

void
gv_view_area_map_location(GvViewArea *view, gvgeocoord x, gvgeocoord y,
             gvgeocoord *geo_x, gvgeocoord *geo_y)
{
    const char *coord_sys = gv_view_area_get_projection(view);
    char    *latlong_srs = NULL;

    *geo_x = x;
    *geo_y = y;

    /* Don't need to do anything in the case of a view with
       raw PIXEL projection */
    if (coord_sys != NULL && !EQUAL(coord_sys,"PIXEL"))
    {
        latlong_srs = gv_make_latlong_srs( coord_sys );

        if( latlong_srs != NULL )
        {
            double      temp_x, temp_y, temp_z;

            temp_x = x;
            temp_y = y;
            temp_z = 0.0;

            if( gv_reproject_points( latlong_srs, coord_sys,
                                     1, &temp_x, &temp_y, &temp_z ) )
            {
                *geo_x = temp_x;
                *geo_y = temp_y;

                /* printf(" (x,y)=(%f,%f), (geo_x, geo_y)=(%f,%f)\n", x,y,temp_x,temp_y); */
            }
        }
    }

    /* geo_x, geo_y contains the original x,y pair or the reprojected values
       if view is lat/long */
}

void
gv_view_area_map_pointer(GvViewArea *view, gvgeocoord px, gvgeocoord py,
                                           gvgeocoord *x, gvgeocoord *y)
{
    gvgeocoord fx, fy, cr, sr;

    cr = cos(-view->state.rot * DEG2RAD);
    sr = sin(-view->state.rot * DEG2RAD);
    fx = px - view->state.shape_x / 2.0;
    fy = view->state.shape_y / 2.0 - py;
    fx /= view->state.linear_zoom;
    fy /= view->state.linear_zoom;
    *x = (fx * cr - fy * sr) * view->state.flip_x - view->state.tx;
    *y = (fx * sr + fy * cr) * view->state.flip_y - view->state.ty;
}

void

gv_view_area_inverse_map_pointer(GvViewArea *view, gvgeocoord x,   gvgeocoord y,
                                                   gvgeocoord *px, gvgeocoord *py)
{
    gvgeocoord fx, fy, cr, sr;

    cr = cos(view->state.rot * DEG2RAD);
    sr = sin(view->state.rot * DEG2RAD);

    x += view->state.tx;
    y += view->state.ty;

    fx = (x * cr * view->state.flip_x - y * sr * view->state.flip_y)
        * view->state.linear_zoom;
    fy = (x * sr * view->state.flip_x + y * cr * view->state.flip_y)
        * view->state.linear_zoom;

    *px = fx + view->state.shape_x / 2.0;
    *py = view->state.shape_y / 2.0 - fy;
}

void
gv_view_area_correct_for_transform(GvViewArea *view, gvgeocoord x, gvgeocoord y,
                   gvgeocoord *cx, gvgeocoord *cy)
{
    /* Correct a vector for rotation and zoom: used for sprites */
    gvgeocoord cr, sr;

    cr = cos(-view->state.rot * DEG2RAD);
    sr = sin(-view->state.rot * DEG2RAD);

    *cx = (x * cr - y * sr) * view->state.flip_x / view->state.linear_zoom;
    *cy = (x * sr + y * cr) * view->state.flip_y / view->state.linear_zoom;
}

void
gv_view_area_add_layer(GvViewArea *view, GObject *layer_obj)
{
    GvLayer *layer;

    g_return_if_fail(GV_IS_LAYER(layer_obj));
    layer = GV_LAYER(layer_obj);

    //first check if the layer is in this view - if it is already added, warn and return
    g_return_if_fail( g_list_find(view->layers, layer) == NULL );

    if (GTK_WIDGET_REALIZED(GTK_WIDGET(view)))
    {
        /* Call the one time setup function for the layer */
        /* If the view area is not yet realized, this is deferred to
           gv_view_area_realize() */
        if (gv_view_area_make_current(view))
        {
            gv_layer_setup(layer, view);
        }

        /* If this is the first layer, set initial view area
           transformation based on layer extents.  Also ensure
           we are reset back to unflipped even if we previously got
          flipped for a raw image. */
        if (view->layers == NULL)
        {
            view->state.flip_y = 1.0;
            gv_view_area_fit_layer(view, layer);
        }
    }

    g_assert( layer->view == NULL );

    view->volume_current = FALSE;
    view->layers = g_list_append(view->layers, layer);
    g_object_ref(layer_obj);

    layer->view = view;

    g_signal_connect_swapped(layer, "changed",
                              G_CALLBACK(gv_view_area_change_notify),
                              view);
    g_signal_connect_swapped(layer, "display-change",
                              G_CALLBACK(gv_view_area_change_notify),
                              view);

    g_signal_emit(view, view_area_signals[ACTIVE_CHANGED], 0);

    gv_view_area_state_changed(view);
}

void
gv_view_area_remove_layer(GvViewArea *view, GObject *layer_obj)
{
    GvLayer *layer;
    GList *link;

    g_return_if_fail(GV_IS_LAYER(layer_obj));
    layer = GV_LAYER(layer_obj);

    //first check if the layer is in this view - if not, just return
    link = g_list_find(view->layers, layer);
    if (link == NULL)
    {
        CPLDebug( "OpenEV",
                  "gv_view_area_remove_link(): layer %s is not in view",
                    g_type_name (G_TYPE_FROM_INSTANCE(layer)));
        return;
    }

    if (GTK_WIDGET_REALIZED(GTK_WIDGET(view)))
    {
        /* Allow layer to destroy gl handles if necessary */
        if (gv_view_area_make_current(view))
            gv_layer_teardown(layer, view);
    }

    if (layer_obj == view->active_layer)
        gv_view_area_set_active_layer(view, NULL);

    g_assert( layer->view == view );
    layer->view = NULL;

    view->layers = g_list_remove_link(view->layers, link);
    view->volume_current = FALSE;

    /* clear projection if no layers are left. */
    if( view->layers == NULL )
    {
        if( view->projection != NULL )
        {
            g_free( view->projection );
            view->projection = NULL;
        }
    }


    g_signal_handlers_disconnect_matched (layer_obj, G_SIGNAL_MATCH_DATA,
                                            0, 0, NULL, NULL, G_OBJECT(view));
    g_object_unref(layer);
    g_signal_emit(view, view_area_signals[ACTIVE_CHANGED], 0);

    gv_view_area_queue_draw(view);
}

GObject *
gv_view_area_active_layer(GvViewArea *view)
{
    return view->active_layer;
}

void
gv_view_area_set_active_layer(GvViewArea *view, GObject *layer)
{
    g_return_if_fail(layer == NULL || GV_IS_LAYER(layer));

    /* Ignore if this is already the active layer */
    if (layer == view->active_layer) return;

    if (layer)
    {
        /* Make sure this layer is in the list */
        if (g_list_find(view->layers, (gpointer)layer) == NULL)
        {
            CPLDebug( "OpenEV",
                          "gv_view_area_set_active_layer(): layer %s not in view",
                        g_type_name (G_TYPE_FROM_INSTANCE(layer)));
            return;
        }
    }

    view->active_layer = layer;

    g_signal_emit(view, view_area_signals[ACTIVE_CHANGED], 0);
}

GObject *
gv_view_area_get_layer_of_type(GvViewArea *view, GType layer_type,
                               gint read_only_ok )
{
    GList *list;

    /* First check the active layer */
    if(view->active_layer &&
       g_type_is_a(G_OBJECT_TYPE(view->active_layer), layer_type)
       && (read_only_ok || !gv_data_is_read_only(GV_DATA(view->active_layer))))
    {
    return view->active_layer;
    }

    /* Next move through the list in order */
    list = view->layers;
    while (list)
    {
    GvLayer *layer = (GvLayer*)list->data;
    if( g_type_is_a(G_OBJECT_TYPE(layer), layer_type)
            && (read_only_ok || !gv_data_is_read_only(GV_DATA(layer))) )
    {
        return G_OBJECT(layer);
    }
    list = g_list_next(list);
    }
    return NULL;
}

GObject *
gv_view_area_get_named_layer(GvViewArea *view, const char *name)
{
    GList *list;

    list = view->layers;
    while (list)
    {
    GvLayer *layer = (GvLayer*)list->data;
    if (GV_DATA(layer)->name && strcmp(GV_DATA(layer)->name, name) == 0)
    {
        return G_OBJECT(layer);
    }
    list = g_list_next(list);
    }
    return NULL;
}

GList *
gv_view_area_list_layers(GvViewArea *view)
{
    return g_list_copy(view->layers);
}

void
gv_view_area_swap_layers(GvViewArea *view, gint index_a, gint index_b)
{
    gpointer *layer_a, *layer_b;

    g_return_if_fail(index_a != index_b);

    /* Put indicies in ascending order */
    if (index_a > index_b)
    {
    gint tmp = index_b;
    index_b = index_a;
    index_a = tmp;
    }

    layer_a = g_list_nth_data(view->layers, index_a);
    layer_b = g_list_nth_data(view->layers, index_b);
    g_return_if_fail(layer_a && layer_b);

    /* Perform swap */
    view->layers = g_list_remove(view->layers, layer_a);
    view->layers = g_list_remove(view->layers, layer_b);
    view->layers = g_list_insert(view->layers, layer_b, index_a);
    view->layers = g_list_insert(view->layers, layer_a, index_b);

    gv_view_area_queue_draw(view);
}

GdkPixmap *
gv_view_area_create_thumbnail(GvViewArea *view, GObject *layerobj,
                  gint width, gint height)
{

    /* This was already commented out for windows, so I've changed
       it to be commented out for everyone until this is ported to
       gtk2 and made non platform specific at the same time. - pete */

#ifndef PENDING_GTK2
    return NULL;
#else
    GvLayer *layer;
    GdkPixmap *pixmap;
    GdkVisual *visual;
    GdkGLContext *glcontext;
    GdkGLPixmap *glpixmap;
    GvRect extents;
    gvgeocoord aspect;
    gint visible;

    if( !GTK_WIDGET_REALIZED(view) )
        return NULL;

    g_return_val_if_fail(GV_IS_LAYER(layerobj), NULL);

    layer = GV_LAYER(layerobj);

    visual = gtk_widget_get_visual(GTK_WIDGET(view));
    pixmap = gdk_pixmap_new(NULL, width, height, visual->depth);
    glcontext = gv_view_area_get_gl_context(view);
    glpixmap = gdk_gl_pixmap_new(visual, pixmap);

    if (!gdk_gl_pixmap_make_current(glpixmap, glcontext))
    {
    CPLDebug( "OpenEV",
                  "gv_view_area_create_thumbnail(): offscreen render failed.");
    gdk_gl_pixmap_unref(glpixmap);
    gdk_pixmap_unref(pixmap);
    return NULL;
    }

    /* Add 5% to extents in each direction to get a nice border */
    gv_layer_extents(layer, &extents);
    extents.x -= extents.width * 0.05;
    extents.y -= extents.height * 0.05;
    extents.width *= 1.1;
    extents.height *= 1.1;

    /* Fix aspect ratio */
    aspect = (gvgeocoord)width / (gvgeocoord)height;
    if( (extents.width / extents.height) > aspect )
    {
    gvgeocoord newheight = extents.width / aspect;
    extents.y -= (newheight - extents.height) / 2.0;
    extents.height = newheight;
    }
    else
    {
    gvgeocoord newwidth = extents.height * aspect;
    extents.x -= (newwidth - extents.width) / 2.0;
    extents.width = newwidth;
    }

    /* glpixmap is in the front left buffer */
    glDrawBuffer(GL_FRONT_LEFT);
    glViewport(0, 0, width, height);

    /* Set background to black */
    glClearColor(view->background[0], view->background[1], view->background[2],
                 view->background[3] );
    glClear(GL_COLOR_BUFFER_BIT);

    /* Make sure there is something to draw */
    if (extents.width > 0 || extents.height > 0)
    {
    /* Temporarily make visible, and turn on presentation mode */
    visible = gv_layer_set_visible_temp(layer, TRUE);
    gv_layer_set_presentation(layer, TRUE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
        if( view->state.flip_y < 0 )
        {
            gluOrtho2D(extents.x, extents.x + extents.width,
                       extents.y + extents.height, extents.y );
        }
        else
        {
            gluOrtho2D(extents.x, extents.x + extents.width,
                       extents.y, extents.y + extents.height);
        }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gv_layer_draw(layer, view);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    /* Turn off presentation mode and restore visibility */
    gv_layer_set_visible_temp(layer, visible);
    gv_layer_set_presentation(layer, FALSE);
    }
    glFinish();

    /* Restore the view as the current context before destroying
       the glpixmap */
    if (!gv_view_area_make_current(view))
    {
    g_warning("gv_view_area_create_thumbnail(): view context restore failed.");
    }
    glViewport(0, 0, view->state.shape_x, view->state.shape_y);
    glDrawBuffer(GL_BACK);

    gdk_gl_pixmap_unref(glpixmap);
    return pixmap;
#endif
}

gint
gv_view_area_render_to_func(GvViewArea *view,
                            gint width, gint height,
                            gint (*scanline_handler)( void *, void * ),
                            void *cb_data)
{
    GvRect extents;
    gvgeocoord aspect;
    guint8 *scanline;
    gvgeocoord xmin, xmax, ymin, ymax;
    gint line, errcode = 0;
    gint x_chunk, x_chunk_size, y_chunk, y_chunk_size;
    GList *list;
    guint8 *row_of_chunks;
    int     direct_render, save_exact;


    /* ---- OpenGL BEGIN ---- */
    if (!gv_view_area_begin(view)) return -1;

    direct_render = (width == view->state.shape_x)
                 && (height == view->state.shape_y);

    save_exact = view->exact_render;
    view->exact_render = TRUE;

    x_chunk_size = view->state.shape_x;
    y_chunk_size = view->state.shape_y;

    gv_view_area_get_extents(view, &xmin, &ymin, &xmax, &ymax );
    extents.x = xmin;
    extents.width = xmax - xmin;
    extents.y = ymin;
    extents.height = ymax - ymin;

    /* Fix aspect ratio */
    aspect = (gvgeocoord)width / (gvgeocoord)height;
    if ( (extents.width / extents.height) > aspect )
    {
    gvgeocoord newheight = extents.width / aspect;
    extents.y -= (newheight - extents.height) / 2.0;
    extents.height = newheight;
    }
    else
    {
    gvgeocoord newwidth = extents.height * aspect;
    extents.x -= (newwidth - extents.width) / 2.0;
    extents.width = newwidth;
    }

    /* allocate a buffer large enough to hold one RGB row of chunks */
    row_of_chunks = g_malloc(width * y_chunk_size * 3);
    if( row_of_chunks == NULL )
    {
        g_warning( "out of memory in gv_view_area_render_to_func" );
        view->exact_render = save_exact;
        gv_view_area_gl_end(view);
        return -1;
    }
    memset( row_of_chunks, 0, width*y_chunk_size*3 );

    /* glpixmap is in the front left buffer */
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    /* Make sure there is something to draw */
    if (extents.width == 0 || extents.height == 0)
    {
        view->exact_render = save_exact;
        gv_view_area_gl_end(view);
        return -1;
    }

    for( y_chunk=0; y_chunk < height && errcode == 0; y_chunk += y_chunk_size)
    {
        GvRect chunk_extents;

        chunk_extents.height =
            extents.height * (y_chunk_size / (double) height);
        if( view->state.flip_y < 0 )
        {
            chunk_extents.y = extents.y
                + extents.height * (y_chunk / (double) height);
        }
        else
        {
            chunk_extents.y = extents.y + extents.height
                - extents.height * ((y_chunk+y_chunk_size) / (double) height);
        }

        for( x_chunk = 0;
             x_chunk < width && errcode == 0;
             x_chunk += x_chunk_size )
        {
            chunk_extents.width =
                extents.width * (x_chunk_size / (double) width);
            chunk_extents.x = extents.x
                + extents.width * (x_chunk / (double) width);

            /* Set background to black */
            glClearColor(view->background[0], view->background[1],
                         view->background[2], view->background[3] );
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            render_counter++;

            if( !direct_render )
            {
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                if( view->state.flip_y < 0 )
                {
                    gluOrtho2D(chunk_extents.x,
                               chunk_extents.x + chunk_extents.width,
                               chunk_extents.y + chunk_extents.height,
                               chunk_extents.y );
                }
                else
                {
                    gluOrtho2D(chunk_extents.x,
                               chunk_extents.x + chunk_extents.width,
                               chunk_extents.y,
                               chunk_extents.y + chunk_extents.height);
                }

                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
            }

            for( list = view->layers; list != NULL; list = g_list_next(list) )
            {
                GvLayer  *layer = GV_LAYER( list->data );

                gv_layer_set_presentation(layer, TRUE);
                gv_layer_draw(layer, view);
                gv_layer_set_presentation(layer, FALSE);
            }

            if( !direct_render )
            {
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
            }

            glFinish();

            for( line = 0;
                 line < y_chunk_size && errcode == 0
                     && (line + y_chunk) < height;
                 line++ )
            {
                scanline = row_of_chunks +
                    x_chunk * 3
                    + line * width * 3;

                scanline[0] = 253;
                scanline[1] = 101;
                glReadPixels( 0, y_chunk_size - line - 1,
                              MIN(x_chunk_size,width-x_chunk), 1,
                              GL_RGB, GL_UNSIGNED_BYTE, scanline );

                if( scanline[0] == 253 && scanline[1] == 101 )
                {
                    CPLDebug( "OpenEV",
                              "glReadPixels() appears to have failed." );
                    errcode = 1;
                }
            }
        }

        for( line = 0;
             line < y_chunk_size && errcode == 0
                 && (line + y_chunk) < height;
             line++ )
        {
            scanline = row_of_chunks + line * width * 3;
            errcode =  scanline_handler( cb_data, scanline );
        }
    }

    g_free( row_of_chunks );

    view->exact_render = save_exact;
    gv_view_area_gl_end(view);

    return errcode;
}

GPtrArray *gv_view_area_get_fontnames( GvViewArea *view )

{
    GPtrArray   *ret_list;
    int     font;

    ret_list = g_ptr_array_new();

    for( font = 0; bmfontmap[font].gvname != NULL; font++ )
        g_ptr_array_add( ret_list, bmfontmap[font].gvname );

    return ret_list;
}

gint
gv_view_area_bmfont_load(GvViewArea *view, gchar *name)
{
    char fn[] = "gv_view_area_bmfont_load";
    gint     font;
    GvBMFontInfo new_finfo;
    PangoFontDescription *pango_desc;
    PangoFont *pango_font;


    /* ---- Check for XLFD format font description ---- */
    pango_desc = gv_view_XLFD_to_pango(name);
    if (pango_desc == NULL) {
        pango_desc = pango_font_description_from_string(name);
        if (pango_desc == NULL) {
            g_warning("%s: pango font load failed: %s", fn, name );
            return -1;
        }
    }

    /* Do we already have this font loaded? */
    for (font=0; font < view->bmfonts->len; font++ )
    {
        GvBMFontInfo    *finfo;
        finfo = &(g_array_index( view->bmfonts, GvBMFontInfo, font ));
        if (strcmp(pango_font_description_to_string(pango_desc), finfo->name) == 0) {
            return font;
        }
    }

    /* >>>> Try to load the font <<<< */

    new_finfo.listbase = glGenLists(96);
    if (new_finfo.listbase == 0)
    {
        g_warning("%s: font allocation failed", fn);
        return -1;
    }

    /* We will try to create the pango font for the specified pango_desc,
       but the pango_font returned may be something completely different.

       Currently storing the name of the called-for font and not the name
       of the returned font, so that if the same font is called for again
       it can be found in the bmfonts list.
    */

    pango_font = gdk_gl_font_use_pango_font
      (pango_desc, 0, 127, new_finfo.listbase);
    if (pango_font == NULL) {
        pango_desc = pango_font_description_from_string("Sans 12");
        pango_font = gdk_gl_font_use_pango_font
            (pango_desc, 0, 127, new_finfo.listbase);
        if (pango_font == NULL) {
            g_warning("%s: Unable to obtain font: %s", fn,
                pango_font_description_to_string(pango_desc));
            gv_view_area_gl_end(view);
            return -1;
        }
        else {
            g_warning("%s:\n   Unable to obtain font '%s'\n   Using '%s'\n", fn,
                      name, pango_font_description_to_string(pango_desc));
        }
    }

    new_finfo.pango_desc = pango_desc;
    new_finfo.name = pango_font_description_to_string(new_finfo.pango_desc);
    g_array_append_val( view->bmfonts, new_finfo);

    return view->bmfonts->len-1;
}

/**
 * Create a pango font description from an XLFD font name.
 */
static PangoFontDescription *gv_view_XLFD_to_pango(gchar *XLFD_name) {
  gchar *pango_name, *tempptr;
  gchar **XLFD_tokens;
  int n_tokens;
  PangoFontDescription *pango_desc;


  /* ---- Init ---- */
  if ((XLFD_name == NULL) || (XLFD_name[0]!= '-')) return NULL;

  /* ---- Obtain tokens from XLFD name ---- */
  XLFD_tokens = g_strsplit_set(&XLFD_name[1], "-", 20);
  if (XLFD_tokens == NULL) return NULL;

  /* ---- Count tokens ---- */
  for (n_tokens = 0; n_tokens < 15; n_tokens++) {
    if (XLFD_tokens[n_tokens] == NULL) break;
  }
  if ((n_tokens > 15) || (n_tokens < 12)) {
    g_strfreev(XLFD_tokens);
    return NULL;
  }

  /* ---- Create pango string from tokens of interest ---- */
  pango_name = (gchar *)g_malloc(strlen(XLFD_tokens[1]) +
                                 strlen(XLFD_tokens[2]) +
                                 strlen(XLFD_tokens[4]) +
                                 strlen(XLFD_tokens[5]) + 32);

  if (pango_name == NULL) return NULL;

  /* ---- Start with font family ---- */
  tempptr = g_stpcpy(pango_name, XLFD_tokens[1]);

  /* ---- Add style properties ---- */
  if ((strlen(XLFD_tokens[2]) > 0) && (XLFD_tokens[2][0] != '*')) {
    *tempptr = ' ';
    tempptr++;
    tempptr = g_stpcpy(tempptr, XLFD_tokens[2]);
  }
  if ((XLFD_tokens[3][0] == 'I') || (XLFD_tokens[3][0] == 'i')) {
    *tempptr = ' ';
    tempptr++;
    tempptr = g_stpcpy(tempptr, "Italic");
  }
  if ((XLFD_tokens[3][0] == 'O') || (XLFD_tokens[3][0] == 'o')) {
    *tempptr = ' ';
    tempptr++;
    tempptr = g_stpcpy(tempptr, "Oblique");
  }
  if ((strlen(XLFD_tokens[4]) > 0) && (XLFD_tokens[4][0] != '*')) {
    *tempptr = ' ';
    tempptr++;
    tempptr = g_stpcpy(tempptr, XLFD_tokens[4]);
  }
  if ((strlen(XLFD_tokens[5]) > 0) && (XLFD_tokens[4][0] != '*')) {
    *tempptr = ' ';
    tempptr++;
    tempptr = g_stpcpy(tempptr, XLFD_tokens[5]);
  }

  /* ---- Add size at end ---- */
  *tempptr = ' ';
  tempptr++;
  if ((strlen(XLFD_tokens[6]) > 0) && (XLFD_tokens[4][0] != '*')) {
    g_stpcpy(tempptr, XLFD_tokens[6]);
  }
  else {
    g_stpcpy(tempptr, "12");
  }

  /* ---- Attempt to obtain PangoFontDescription ---- */
  pango_desc = pango_font_description_from_string(pango_name);
  if (pango_desc == NULL) {

    /* >>>> Try scaled down description <<<< */

    /* ---- Start with font family ---- */
    tempptr = g_stpcpy(pango_name, XLFD_tokens[1]);

    /* ---- Add size at end ---- */
    *tempptr = ' ';
    tempptr++;
    if (strlen(XLFD_tokens[6]) > 0) {
      g_stpcpy(tempptr, XLFD_tokens[6]);
    }
    else {
      g_stpcpy(tempptr, "12");
    }

    pango_desc = pango_font_description_from_string(pango_name);

    if (pango_desc == NULL) {

      /* ---- Try known good font ---- */
      pango_desc = pango_font_description_from_string("Sans 12");
    }
  }

  /* ---- Return PangoFontDescription ---- */ 
  return pango_desc;

}

GvBMFontInfo *
gv_view_area_bmfont_get_info(GvViewArea *view, gint font)
{
    GvBMFontInfo    *finfo;

    if (font < 0 || font >= view->bmfonts->len )
        return NULL;

    finfo = &(g_array_index( view->bmfonts, GvBMFontInfo, font ));
    return finfo;
}

void
gv_view_area_bmfont_draw(GvViewArea *view, gint font, gvgeocoord x,
                         gvgeocoord y, gchar *text, int force_simple )
{
    GvBMFontInfo    *finfo;

    if (font < 0 || font >= view->bmfonts->len )
        return;
    finfo = &(g_array_index( view->bmfonts, GvBMFontInfo, font ));
    if ((finfo == NULL) || (text == NULL)) {
        return;
    }

    /*
     * In 3D it would be too hard to determine whats in/out of view, so
     * we just do the simple thing.  This will result in strings with an
     * origin out of the view frustrum not being drawn, but that's life.
     */
    if( view->flag_3d || force_simple )
    {
        glRasterPos2(x, y);
    }
    /*
     * Ensure we initially position the raster position within the
     * view frustrum, then offset to our desired location.
     */
    else
    {
        gvgeocoord  vx, vy, vx_screen, vy_screen, tx_screen, ty_screen;
        vx_screen = view->state.shape_x/2;
        vy_screen = view->state.shape_y/2;
        gv_view_area_map_pointer( view, vx_screen, vy_screen,
                                  &vx, &vy );
        gv_view_area_inverse_map_pointer( view, x, y, &tx_screen, &ty_screen );

        /* position in frustrum */
        glRasterPos2(vx, vy);

        /* shift to desired location. */
        glBitmap( 0, 0, 0, 0,
                  (int) -(vx_screen - floor(tx_screen)),
                  (int) (vy_screen - ceil(ty_screen)),
                  NULL );
    }
    glPushAttrib(GL_LIST_BASE);
    glListBase(finfo->listbase);
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, (GLubyte*)text);
    glPopAttrib();
}

void gv_view_area_set_background_color( GvViewArea *view, GvColor new_color )

{
    if( new_color[0] != view->background[0]
        || new_color[1] != view->background[1]
        || new_color[2] != view->background[2]
        || new_color[3] != view->background[3] )
    {
        gv_color_copy( view->background, new_color );
        gv_view_area_state_changed(view);
    }
}

void gv_view_area_get_background_color( GvViewArea *view, GvColor color )

{
    color[0] = view->background[0];
    color[1] = view->background[1];
    color[2] = view->background[2];
    color[3] = view->background[3];
}

/*****************************************************************/

static gint
gv_view_area_reset_projection(GvViewArea *view, gvgeocoord width, gvgeocoord height)
{

    if( !gv_view_area_begin(view))
        return 1;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (! view->flag_3d)
    {
        /* Assumes clipping plane of [-1.0, 1.0]
        gluOrtho2D(-width/2.0, width/2.0, -height/2.0, height/2.0); */
        glOrtho(-width/2.0, width/2.0, -height/2.0, height/2.0, -1000.0f, 1000.0f);
    } else {
        gluPerspective(90.0f,(GLgeocoord)width/(GLgeocoord)height,10.0f,3000000.0f);
    }
    glMatrixMode(GL_MODELVIEW);

    gv_view_area_gl_end(view);
    return 0;

}
static gint
gv_view_area_configure(GtkWidget *widget, GdkEventConfigure *event)
{
    GvViewArea *view = GV_VIEW_AREA(widget);
    int w, h;

    if (!gv_view_area_begin(view)) return 0;

    /*CPLDebug( "OpenEV", "VENDOR = %s", glGetString( GL_VENDOR ) );
    CPLDebug( "OpenEV", "RENDERER = %s", glGetString( GL_RENDERER ) );
    CPLDebug( "OpenEV", "VERSION = %s", glGetString( GL_VERSION ) );
    CPLDebug( "OpenEV", "EXTENSIONS = %s", glGetString( GL_EXTENSIONS ) );*/

    w = event->width;
    h = event->height;

    gv_view_area_reset_projection(view, (gvgeocoord)w, (gvgeocoord)h);

    view->state.shape_x = (gvgeocoord)w;
    view->state.shape_y = (gvgeocoord)h;

    gv_view_area_state_changed( view );

    gv_view_area_gl_end(view);

    return 0;
}

gint
gv_view_area_expose(GtkWidget *widget, GdkEventExpose *event)
{
    GvViewArea *view = GV_VIEW_AREA(widget);
    GList *list;
    GvLayer *layer;
    GTimer *timer;


    if (!gv_view_area_begin(view)) return 0;

    /*
    ** If the display state hasn't changed in any way, then just refresh from
    ** our display buffer.  Otherwise we will actually need to rerender with
    ** OpenGL.  We avoid rerendering for interactivity on software rendered
    ** systems.
    */

    if( (!view->display_dirty)
        && ((strstr(glGetString(GL_RENDERER), "Mesa Windows") != NULL) ||
        (strstr(glGetString(GL_RENDERER),"Mesa X11") != NULL) ||
        (strstr(glGetString(GL_RENDERER), "Mesa GLX Indirect") != NULL)))
    {

        gv_view_area_swap_buffers(view);
        g_signal_emit(view, view_area_signals[GLCURSOR], 0);
        gv_view_area_gl_end(view);
        return 0;
    }

    view->display_dirty = FALSE;

    render_counter++;

    gv_manager_set_busy( gv_get_manager(), TRUE );

    timer = g_timer_new();
    g_timer_start( timer );

    glDisable(GL_DITHER);
    glShadeModel(GL_FLAT);
    glClearColor(view->background[0], view->background[1],
                 view->background[2], view->background[3] );

    /* Do we need these??? */
    if (view->flag_3d)
    {
#ifdef notdef
        glEnable(GL_CULL_FACE);
#endif
        glEnable(GL_NORMALIZE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }
    else
    {
        glDisable(GL_NORMALIZE);
        glDisable(GL_DEPTH_TEST);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    if (view->flag_3d)
    {
        double     min_range, max_range;
        vec3_t eye_pos, eye_dir, flip_eye_pos;
        int   debug3d = 0;

        if( gv_manager_get_preference(gv_get_manager(),"DEBUG3D") != NULL )
            debug3d =
                atoi(gv_manager_get_preference(gv_get_manager(),"DEBUG3D"));

        eye_pos[0] = view->state.eye_pos[0];
        eye_pos[1] = view->state.eye_pos[1];
        eye_pos[2] = view->state.eye_pos[2];

        eye_dir[0] = view->state.eye_dir[0];
        eye_dir[1] = view->state.eye_dir[1];
        eye_dir[2] = view->state.eye_dir[2];


        /* Compute the near and far clipping planes - taking into account any image flips */
        gv_view_area_get_volume( view, NULL );

        flip_eye_pos[0] = view->state.flip_x * eye_pos[0];
        flip_eye_pos[1] = view->state.flip_y * eye_pos[1];
        flip_eye_pos[2] = eye_pos[2];

        vec_near_far_range( flip_eye_pos, view->view_volume,
                            &min_range, &max_range);

        if( min_range < max_range / 500.0 || min_range >= max_range )
            min_range = max_range / 500.0;

        max_range = max_range * 1.2;
        min_range = min_range * 0.5;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(90.0f,
                       (gvgeocoord)view->state.shape_x/(gvgeocoord)view->state.shape_y,
                       min_range*0.5, max_range );
        glMatrixMode(GL_MODELVIEW);

        if( debug3d )
        {
            printf( "================ Redraw Start ===================\n" );
            printf( "Clipping planes: near=%g, far=%g\n",
                    min_range, max_range );
        }

        /* Set view orientation */
        gluLookAt(eye_pos[0], eye_pos[1], eye_pos[2],
              eye_pos[0] + eye_dir[0],
                  eye_pos[1] + eye_dir[1],
                  eye_pos[2] + eye_dir[2],
                  0.0, 0.0, 1.0);

        glScale(1.0 * view->state.flip_x, 1.0 * view->state.flip_y, view->state.z_scale);

        if( debug3d )
        {
            printf( "eye=(%g,%g,%g)  eye_dir=(%g,%g,%g)\n",
                    eye_pos[0], eye_pos[1], eye_pos[2],
                    eye_dir[0], eye_dir[1], eye_dir[2] );
        }

    } else {
        glRotate(view->state.rot, 0.0, 0.0, 1.0);

        glScale(view->state.linear_zoom * view->state.flip_x,
                 view->state.linear_zoom * view->state.flip_y, 1.0);

        glTranslate(view->state.tx, view->state.ty, 0.0);
    }

    list = view->layers;
    while (list)
    {
        layer = (GvLayer*)list->data;
        gv_layer_draw(layer, view);
        list = g_list_next(list);
    }

    /* Give other interested objects a chance to add to the drawing */
    g_signal_emit(view, view_area_signals[GLDRAW], 0);

    /* draw banded zoom area */
    if (view->dragging_mode)
    {
        gvgeocoord x[4], y[4];

        gv_view_area_map_pointer( view,
                                  view->state.mpos_x, view->state.mpos_y,
                                  x+0, y+0 );
        gv_view_area_map_pointer( view,
                                  view->last_mpos_x, view->state.mpos_y,
                                  x+1, y+1 );
        gv_view_area_map_pointer( view,
                                  view->last_mpos_x, view->last_mpos_y,
                                  x+2, y+2 );
        gv_view_area_map_pointer( view,
                                  view->state.mpos_x, view->last_mpos_y,
                                  x+3, y+3 );

        glColor3f(1.0, 0.5, 0.0);
        glBegin(GL_LINE_LOOP);
        glVertex2(x[0], y[0]);
        glVertex2(x[1], y[1]);
        glVertex2(x[2], y[2]);
        glVertex2(x[3], y[3]);
        glEnd();
    }

    gv_view_area_swap_buffers(view);
    /* Don't XOR last ghost cursor position, since view has been redrawn */
    /* (only has effect in logical cursor mode) */
    view->last_valid=FALSE;
    g_signal_emit(view,view_area_signals[GLCURSOR], 0);

    glFinish();

    /*
     * Capture time this draw took.
     */
    view->last_draw_time = g_timer_elapsed(timer,NULL);
    g_timer_stop( timer );
    g_timer_destroy( timer );

    /*
     * Setup timer so we know when we should do the next redraw, even if
     * all the idle work isn't done yet.
     */
    if( view->redraw_timer != NULL )
        g_timer_destroy( view->redraw_timer );

    view->redraw_timer = g_timer_new();
    g_timer_start( view->redraw_timer );

    gv_view_area_gl_end(view);

    return 0;
}

static void
gv_view_area_realize(GtkWidget *widget)
{
    GvViewArea *view = GV_VIEW_AREA(widget);
    GList *list;
    GvLayer *layer;
    const char *auto_fit_flag;

    /* Call parent class function first */
    GTK_WIDGET_CLASS(parent_class)->realize(widget);

    g_return_if_fail(GTK_WIDGET_REALIZED(widget));

    if (NULL==_share_list && gtk_widget_is_gl_capable(widget)) {
        _share_list = gtk_widget_get_gl_context(widget);
    }

    /* Make gl area current so layers can create gl handles */
    if (!gv_view_area_make_current(view)) return;

    /* Call the one time setup function for all layers */
    list = view->layers;
    while (list)
    {
        layer = (GvLayer*)list->data;
        gv_layer_setup(layer, view);
        list = g_list_next(list);
    }

    /* Set up the inital view transformation using the first layer */
    auto_fit_flag = gv_properties_get(&(view->properties),
                                      "_supress_realize_auto_fit");
    if (view->layers && (auto_fit_flag == NULL || EQUAL(auto_fit_flag,"off")))
    {
        layer = (GvLayer*)view->layers->data;
        gv_view_area_fit_layer(view, layer);
    }
}

static void
gv_view_area_unrealize(GtkWidget *widget)
{
    GvViewArea *view = GV_VIEW_AREA(widget);
    GList *list;
    gint font;

    /* Make gl area current so layers can clean up gl handles */

    /* Note: Making the view area current at this point does not work!  I get
       the error GdkGLExt-CRITICAL **: file gdkgldrawable.c: 
       assertion `GDK_IS_GL_DRAWABLE (gldrawable)' failed

       Is this really needed so layers can clean up gl handles?  Some other
       approach seems to be necessary...
    if (!gv_view_area_make_current(view)) {
      g_warning("gv_view_area_unrealize(): can't make view area current");
      return;
    }
    */

    /* Teardown all layers */
    view->active_layer = NULL;
    list = view->layers;
    while (list)
    {
        GvLayer *layer = (GvLayer*)list->data;
        gv_layer_teardown(layer, view);
        list = g_list_next(list);
    }

    /* Destroy bmfont listbase array */
    for( font=0; font < view->bmfonts->len; font++ )
    {
        GvBMFontInfo    *finfo;

        finfo = &(g_array_index( view->bmfonts, GvBMFontInfo, font ));

        if( finfo->pango_desc != NULL )
            pango_font_description_free(finfo->pango_desc);

        g_free( finfo->name );
        glDeleteLists(finfo->listbase, 96);
    }
    g_array_free( view->bmfonts, TRUE );

    /* Call parent class function */
    GTK_WIDGET_CLASS(parent_class)->unrealize(widget);
}

static void
gv_view_area_fit_layer(GvViewArea *view, GvLayer *layer)
{
    GvRect rect;

    /* Set the view transformation to show the full extents of the layer */

    /* Make sure the view area is realized so the state.shape has been
       set (a configure event has been recieved) */
    if (!GTK_WIDGET_REALIZED(GTK_WIDGET(view))) return;

    /*
     * Set the view projection.
     */
    if( view->projection != NULL )
    {
        g_free( view->projection );
        view->projection = NULL;
    }

    if( gv_data_get_projection(GV_DATA(layer)) != NULL )
    {
        view->projection = g_strdup(gv_data_get_projection(GV_DATA(layer)));
    }

    /*
     * Get the layer extents.
     */
    gv_layer_extents(layer, &rect);

    /*
     * If this is an image, we want to try and orient the image with
     * pixel (0,0) in the top left if possible.
     */
    if( GV_IS_RASTER_LAYER(layer) )
    {
        GvRaster     *raster;

        raster = GV_RASTER_LAYER(layer)->prototype_data;

        if( GV_RASTER_LAYER(layer)->mesh_is_raw
            || (raster->gcp_count == 0
                && raster->geotransform[2] == 0.0
                && raster->geotransform[4] == 0.0
                && raster->geotransform[5] > 0.0) )
        {
            rect.y = rect.y + rect.height;
            rect.height *= -1;
        }
    }

    gv_view_area_fit_extents(view, rect.x, rect.y, rect.width, rect.height );

    /* Setup 3D specific extent information */
    gv_view_area_fit_extents_3D(view,
                                rect.x, rect.y, 0.0,
                                rect.width, rect.height,
                                (rect.width+rect.height)*0.5 );
}

void
gv_view_area_fit_all_layers(GvViewArea *view)

{
    GvRect rect;
    double volume[6];

    /*
     * Set 2D view.
     */
    gv_view_area_get_world_extents( view, &rect );
    gv_view_area_fit_extents(view, rect.x, rect.y, rect.width, rect.height );

    /*
     * Also do a default 3D view.
     */
    gv_view_area_get_volume( view, volume );
    gv_view_area_fit_extents_3D( view,
                                 volume[0],
                                 volume[2],
                                 volume[4],
                                 volume[1] - volume[0],
                                 volume[3] - volume[2],
                                 volume[5] - volume[4] );
}

void
gv_view_area_get_world_extents(GvViewArea *view, GvRect *extents )

{
    GvRect rect;
    int    flip_y = FALSE, first = TRUE, first_raster = TRUE;
    GList *list;

    rect.x = 0;
    rect.y = 0;
    rect.width = 0;
    rect.height = 0;

    /* Make sure the view area is realized so the state.shape has been
       set (a configure event has been recieved) */
    if (!GTK_WIDGET_REALIZED(GTK_WIDGET(view)))
    {
        *extents = rect;
        return;
    }

    /*
     * Get the layers extents.
     */
    list = view->layers;
    while (list)
    {
        GvLayer *layer = (GvLayer*)list->data;
        GvRect  layer_rect;

        gv_layer_extents(layer, &layer_rect);

        if( layer_rect.width == 0 && layer_rect.height == 0 )
            /* do nothing */;

        else if( first )
        {
            rect = layer_rect;
            first = FALSE;

            if( first_raster && GV_IS_RASTER_LAYER(layer) )
            {
                GvRasterLayer *rlayer = GV_RASTER_LAYER(layer);
                GvRaster     *raster = rlayer->prototype_data;

                if( rlayer->mesh_is_raw )
                {
                    flip_y = TRUE;
                }
                else if( raster->gcp_count == 0
                         && raster->geotransform[2] == 0.0
                         && raster->geotransform[4] == 0.0 )
                {
                    if( raster->geotransform[5] > 0.0 )
                    {
                        flip_y = TRUE;
                    }
                }

                first_raster = FALSE;
            }
        }
        else
        {
            gvgeocoord     new_x, new_y;

            new_x = MIN(rect.x,layer_rect.x);
            rect.width = MAX(rect.x+rect.width,
                             layer_rect.x+layer_rect.width) - new_x;
            new_y = MIN(rect.y,layer_rect.y);
            rect.height = MAX(rect.y+rect.height,
                              layer_rect.y+layer_rect.height) - new_y;
            rect.x = new_x;
            rect.y = new_y;
        }

    list = g_list_next(list);
    }

    /*
     * If we found a "raw" raster layer, flip the coordinate system.
     */
    if( flip_y )
    {
        rect.y = rect.y + rect.height;
        rect.height *= -1;
    }

    *extents = rect;
}

void
gv_view_area_fit_extents(GvViewArea *view,
                         gvgeocoord llx,   gvgeocoord lly,
                         gvgeocoord width, gvgeocoord height )
{
    /* Make sure the view area is realized so the state.shape has been
       set (a configure event has been recieved) */
    if (!GTK_WIDGET_REALIZED(GTK_WIDGET(view))) return;

    if (width == 0.0 || height == 0.0)
    {
    view->state.linear_zoom = 1.0;
    }
    else
    {
    gvgeocoord zoomx, zoomy;

    zoomx = view->state.shape_x / width;
    zoomy = view->state.shape_y / ABS(height);

    view->state.linear_zoom = MIN(zoomx, zoomy);
    }

    view->state.rot = 0.0;
    view->state.tx = -(width/2.0 + llx);
    view->state.ty = -(height/2.0 + lly);
    view->state.zoom = LOG2(view->state.linear_zoom);

    if( height < 0.0 )
    {
        view->state.flip_y = -1.0;
    }

    gv_view_area_state_changed(view);
}

void
gv_view_area_fit_extents_3D(GvViewArea *view,
                            gvgeocoord llx, gvgeocoord lly, gvgeocoord minz,
                            gvgeocoord width, gvgeocoord height, gvgeocoord delta_z )

{
    vec3_t  new_eye_pos, new_eye_dir;
    double  eye_az, eye_el, linear_measure;

    /* Make sure the view area is realized so the state.shape has been
       set (a configure event has been recieved) */
    if (!GTK_WIDGET_REALIZED(GTK_WIDGET(view))) return;

    if (width == 0.0 || height == 0.0) return;

    linear_measure = MAX(MAX(width,height),delta_z);

    new_eye_pos[0] = llx + width * 0.5;
    new_eye_pos[1] = lly - linear_measure * 0.5;
    new_eye_pos[2] = minz + linear_measure * 0.5;

    eye_az = 90.0;   /* rotation ? */
    eye_el = -45.0;  /* tilt  0 --> extreem  -90 --> none*/
    vec_point(new_eye_dir, eye_az, eye_el);

    gv_view_area_set_3d_view( view, new_eye_pos, new_eye_dir );
}

void
gv_view_area_get_extents(GvViewArea *view,
                         gvgeocoord *xmin, gvgeocoord *ymin,
                         gvgeocoord *xmax, gvgeocoord *ymax )

{
    gvgeocoord       x[4], y[4];

    gv_view_area_map_pointer( view, 0, 0,
                              x+0, y+0 );
    gv_view_area_map_pointer( view, 0, view->state.shape_y,
                              x+1, y+1 );
    gv_view_area_map_pointer( view, view->state.shape_x, 0,
                              x+2, y+2 );
    gv_view_area_map_pointer( view, view->state.shape_x, view->state.shape_y,
                              x+3, y+3 );

    *xmin = MIN(MIN(x[0],x[1]),MIN(x[2],x[3]));
    *ymin = MIN(MIN(y[0],y[1]),MIN(y[2],y[3]));
    *xmax = MAX(MAX(x[0],x[1]),MAX(x[2],x[3]));
    *ymax = MAX(MAX(y[0],y[1]),MAX(y[2],y[3]));
}

void
gv_view_area_get_volume( GvViewArea *view, double *volume )

{
    GvRect rect;
    int    first = TRUE;
    GList *list;

    rect.x = 0;
    rect.y = 0;
    rect.width = 0;
    rect.height = 0;

    if( view->volume_current )
    {
        if( volume != NULL )
            memcpy( volume, view->view_volume, sizeof(double) * 6 );
        return;
    }

    view->view_volume[0] = 0.0;
    view->view_volume[1] = 1000.0;
    view->view_volume[2] = 0.0;
    view->view_volume[3] = 1000.0;
    view->view_volume[4] = 0.0;
    view->view_volume[5] = 1000.0;
    view->linear_measure = 1000.0;

    /* Make sure the view area is realized so the state.shape has been
       set (a configure event has been recieved) */
    if (!GTK_WIDGET_REALIZED(GTK_WIDGET(view))) return;

    /*
     * Get the layers extents.
     */
    list = view->layers;
    while (list)
    {
        GvLayer *layer = (GvLayer*)list->data;
        GvRect  layer_rect;

        gv_layer_extents(layer, &layer_rect);

        if( layer_rect.width == 0 && layer_rect.height == 0 )
        {
            /* do nothing */;
        }

        else if( first )
        {
            rect = layer_rect;
            first = FALSE;
        }
        else
        {
            gvgeocoord    new_x, new_y;
            new_x = MIN(rect.x,layer_rect.x);
            rect.width = MAX(rect.x+rect.width,
                             layer_rect.x+layer_rect.width) - new_x;
            new_y = MIN(rect.y,layer_rect.y);
            rect.height = MAX(rect.y+rect.height,
                              layer_rect.y+layer_rect.height) - new_y;
            rect.x = new_x;
            rect.y = new_y;
        }

    list = g_list_next(list);
    }

    if( !first )
    {
        view->view_volume[0] = rect.x;
        view->view_volume[1] = rect.x + rect.width;
        view->view_volume[2] = rect.y;
        view->view_volume[3] = rect.y + rect.height;
        view->view_volume[4] = 0.0;
        view->view_volume[5] = (rect.width + rect.height) * 0.5;
        view->volume_current = TRUE;

        view->linear_measure = MAX(rect.width,rect.height);
    }

    if( volume != NULL )
        memcpy( volume, view->view_volume, sizeof(double) * 6 );

#ifdef notdef
    printf( "gv_view_area_get_volume(): %g, %g, %g, %g, %g, %g\n",
            view->view_volume[0],
            view->view_volume[1],
            view->view_volume[2],
            view->view_volume[3],
            view->view_volume[4],
            view->view_volume[5] );
#endif
}

static gboolean
gv_view_area_motion_handle_hint(GtkWidget *view, GdkEventMotion *event)
{
    /* If this is a hint place the event mouse position */
    /* with the real value (round trip query) */

    if (event->is_hint)
    {
        int x, y;
        gtk_widget_get_pointer(view, &x, &y);
        event->x = (gdouble)x;
        event->y = (gdouble)y;
    }

    return FALSE;
}

void
motion(GvViewArea *view, gvgeocoord move, gvgeocoord strafe, gvgeocoord vert)
{
    vec3_t delta;
    vec3_t new_eye_pos, new_eye_dir;

    /* scaling factor for all motion, always > 0 */
    gvgeocoord scaling_factor = MOVE_SPEED * view->state.eye_pos[2] + (0.0001 * view->linear_measure);

    vec_copy( view->state.eye_dir, new_eye_dir);
    vec_copy( view->state.eye_pos, new_eye_pos);

    if (move != 0.0)
    {
    vec_copy(new_eye_dir, delta);
        /* Scale movement based on height above z plane, previously scaled
           by view->linear_measure */
    vec_scale(delta, move * scaling_factor, delta);
    vec_add(new_eye_pos, delta, new_eye_pos);
    }

    if (strafe != 0.0)
    {
    gvgeocoord norm = sqrt(new_eye_dir[0]*new_eye_dir[0]
                          + new_eye_dir[1]*new_eye_dir[1]);
    if (norm > 0.0)
    {
        delta[0] = new_eye_dir[1] / norm;
        delta[1] = -new_eye_dir[0] / norm;
        delta[2] = 0.0;
        vec_scale(delta, strafe * scaling_factor ,delta);
        vec_add(new_eye_pos, delta, new_eye_pos);
    }
    }

    if (vert != 0.0)
    {
        vec3_t perp;
        gvgeocoord perp_norm;
        gvgeocoord norm =  sqrt(new_eye_dir[0]*new_eye_dir[0]
                          + new_eye_dir[1]*new_eye_dir[1]
                          + new_eye_dir[2]*new_eye_dir[2]);

        /* Get a vector perpendicular to eye_dir */
        perp[0] = - new_eye_dir[0];
        perp[1] = - new_eye_dir[1];
        perp[2] =  (norm/ new_eye_dir[2]) - new_eye_dir[2];

        perp_norm = 1 / sqrt(perp[0]*perp[0]
                         + perp[1]*perp[1]
                         + perp[2]*perp[2]);

        vec_scale(perp, perp_norm * vert * scaling_factor, perp);
        vec_add(new_eye_pos, perp, new_eye_pos);
    }

    gv_view_area_set_3d_view( view, new_eye_pos, new_eye_dir );
}

static gint
gv_view_area_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
    GvViewArea *view = GV_VIEW_AREA(widget);

    if (event->state & GDK_BUTTON1_MASK && view->dragging_mode)
    {
        gv_view_area_queue_draw( view );
    }

    else if( view->last_button != 0
             && (ABS(view->last_mpos_x - event->x) > DRAG_THRESHOLD
                 || ABS(view->last_mpos_y - event->y) > DRAG_THRESHOLD )
             && (g_get_current_time_as_double() - view->last_zoom_time) > 0.0
             &&  !view->flag_3d)
    {
        view->dragging_mode = TRUE;
        view->last_button = 0;

        gv_view_area_queue_draw(view);
        return FALSE;
    }

    /* zoom */
    else if (event->state & GDK_BUTTON2_MASK
             && event->state & GDK_SHIFT_MASK )
    {
        gvgeocoord ax, ay, bx, by, dz;

        /* Translate origin to the center of the view */
        ax = view->state.mpos_x - view->state.shape_x / 2.0;
        ay = view->state.shape_y / 2.0 - view->state.mpos_y;
        bx = event->x - view->state.shape_x / 2.0;
        by = view->state.shape_y / 2.0 - event->y;

        /* Put an insensitive area about the origin */
        if (!(fabs(bx) < DEAD_ZONE && fabs(by) < DEAD_ZONE) &&
            !(fabs(ax) < DEAD_ZONE && fabs(ay) < DEAD_ZONE))
        {
            /* The "delta" zoom (multiplicative) is the projection of
               b (new pos) on a (old pos) divided by the length of a.
               That is: (a.b)/|a|^2 */
            if (ax*ax+ay*ay != 0.0)
            {
                dz = (ax*bx+ay*by) / (ax*ax+ay*ay);

                /* Sanity check on dz */
                dz = MAX(dz, 1.0e-2);
                dz = MIN(dz, 1.0e2);

                /* gv_view_area_zoom() expects a log (base 2)
                   zoom factor */
                gv_view_area_zoom(view, LOG2(dz));
            }
        }
    }
    /* rotate */
    else if ((event->state & GDK_SHIFT_MASK) &&
             (event->state & GDK_BUTTON1_MASK) &&
             !(event->state & GDK_MOD1_MASK))
    {
        gvgeocoord ax, ay, bx, by, dr;

        /* Translate origin to the center of the view */
        ax = view->state.mpos_x - view->state.shape_x / 2.0;
        ay = view->state.shape_y / 2.0 - view->state.mpos_y;
        bx = event->x - view->state.shape_x / 2.0;
        by = view->state.shape_y / 2.0 - event->y;

        /* Put an insensitive area about the origin */
        if (!(fabs(bx) < DEAD_ZONE && fabs(by) < DEAD_ZONE))
        {
            /* The "delta" rotation is found by taking arc sin of the
               the normalized length of the cross product of b (new pos)
               and a (old pos).  That is: arcsin(|axb|/(|a||b|)) */
            dr = asin((ax*by-ay*bx) /
                      (sqrt(ax*ax+ay*ay) * sqrt(bx*bx+by*by)));

            /* gv_view_area_rotate() expects an angle in degrees */
            gv_view_area_rotate(view, dr / DEG2RAD);
        }
    }

    /* translate */
    else if (event->state & GDK_CONTROL_MASK
             && event->state & GDK_BUTTON2_MASK )
    {
        gv_view_area_translate(view, event->x - view->state.mpos_x,
                               view->state.mpos_y - event->y);
    }

    /* Auto-scroll as we drag outside of window */
    if (event->state & GDK_BUTTON1_MASK)
    {
    if (event->x < 0.0 || event->x > view->state.shape_x ||
        event->y < 0.0 || event->y > view->state.shape_y)
    {
        /* Scroll to mouse pos */
        gvgeocoord dx = 0, dy = 0;
        if (event->x < 0.0)
        dx = -event->x;
        if (event->x > view->state.shape_x)
        dx = view->state.shape_x - event->x;
        if (event->y < 0.0)
        dy = event->y;
        if (event->y > view->state.shape_y)
        dy = event->y - view->state.shape_y;
        gv_view_area_translate(view, dx, dy);

        /* Need to put this event back on the queue
           so it keeps scrolling -- see gtktext.c for example */
    }
    }

    /* Change 3D view direction */
    if (view->flag_3d
        && !(event->state & GDK_CONTROL_MASK)
        && ((event->state & GDK_BUTTON1_MASK)
            || (event->state & GDK_BUTTON2_MASK)
            || (event->state & GDK_BUTTON3_MASK)))
    {
        gvgeocoord eye_az = 0, eye_el;
        int return_value;

        return_value = inv_vec_point(view->state.eye_dir, &eye_az, &eye_el);

        eye_az += (event->x - view->state.mpos_x) * AZ_SCALE;
        eye_el += (event->y - view->state.mpos_y) * EL_SCALE;

        if (eye_el < -89.9) eye_el = -89.9;
        if (eye_el > 45.1) eye_el = 45.1;

        vec_point(view->state.eye_dir, eye_az, eye_el);

        gv_view_area_state_changed(view);

        /* For case when trying to zoom and pan - force zoom update */
        gv_view_area_zoompan_handler((gpointer)view);
    }

    /* Change 3D position (translate on when CTRL) */
    if (view->flag_3d
        && (event->state & GDK_CONTROL_MASK)
        && ((event->state & GDK_BUTTON1_MASK) ||
            (event->state & GDK_BUTTON2_MASK) ||
            (event->state & GDK_BUTTON3_MASK)))
    {
        motion(view, 0.0, -(event->x - view->state.mpos_x) * 0.10, -(event->y - view->state.mpos_y)* 0.10);
    }
    view->state.mpos_x = event->x;
    view->state.mpos_y = event->y;

    return 0;
}


static gint
gv_view_area_zoompan_handler(gpointer data )

{
    GvViewArea *view = (GvViewArea *) data;
    double      time_elapsed, cur_time;
    gvgeocoord  zoom = 0.0;

    if( view->last_button == 0 )
        return( FALSE );

    cur_time = g_get_current_time_as_double();
    time_elapsed = cur_time - view->last_zoom_time;
    view->last_zoom_time = MAX(cur_time,view->last_zoom_time);
    if( time_elapsed < 0 )
    {
        time_elapsed = 0;
    }
    else if( time_elapsed > 1.0 )
        time_elapsed = 1.0;
    else if( !view->flag_3d
             && (ABS(view->last_mpos_x - view->state.mpos_x) > DRAG_THRESHOLD
                 || ABS(view->last_mpos_y - view->state.mpos_y) > DRAG_THRESHOLD ))
    {
        view->dragging_mode = TRUE;
        view->last_button = 0;

        gv_view_area_queue_draw(view);

        return FALSE;
    }

    if( view->last_button == 1 )
        zoom = CONTIN_ZOOM_INC * time_elapsed;
    else if( view->last_button == 3 )
        zoom = -(CONTIN_ZOOM_INC * time_elapsed);

    if( zoom != 0.0 )
    {
        gv_view_area_translate(view,
                               view->state.shape_x/2 - view->state.mpos_x,
                               view->state.mpos_y - view->state.shape_y/2 );
        gv_view_area_zoom(view, zoom );
        gv_view_area_translate(view,
                               view->state.mpos_x - view->state.shape_x/2,
                               view->state.shape_y/2 - view->state.mpos_y );
    }

    return TRUE;
}

static gint
gv_view_area_3d_motion_handler(gpointer data )

{
    GvViewArea *view = (GvViewArea *) data;
    double      time_elapsed, cur_time;
    gvgeocoord  move = 0.0;

    if( view->last_button == 0 )
        return( FALSE );

    cur_time = g_get_current_time_as_double();
    time_elapsed = cur_time - view->last_zoom_time;
    view->last_zoom_time = MAX(cur_time,view->last_zoom_time);
    if( time_elapsed < 0 )
    {
        time_elapsed = 0;
    }
    else if( time_elapsed > 1.0 )
    {
        time_elapsed = 1.0;
    }

    if( view->last_button == 1 )
        move = CONTIN_3DMOVE_INC * time_elapsed;
    else if( view->last_button == 3 )
        move = -(CONTIN_3DMOVE_INC * time_elapsed);

    /* Move Forward or backwards */
    if( move != 0.0 )
        motion(view, move, 0.0, 0.0);

    return TRUE;
}

void
gv_view_area_zoompan_event(GvViewArea *view, GdkEventButton *event)

{
    /* note: we want this function to work properly for "zoom mode"
       with no control, and the modeless "cntl-button" method. */

    if( event->type == GDK_2BUTTON_PRESS )
    {
        view->dragging_mode = FALSE;
        gv_view_area_translate(view,
                               view->state.shape_x/2 - event->x,
                               event->y - view->state.shape_y/2 );

        view->last_button = 0;

        if( event->button == 1 )
            gv_view_area_zoom( view, ZOOM_STEP );
        else if( event->button == 3 )
            gv_view_area_zoom( view, -ZOOM_STEP );

    }
    else if( event->type == GDK_BUTTON_PRESS
             && (event->button == 1 || event->button == 3) )
    {
        view->last_button = event->button;
        view->last_button_time = g_get_current_time_as_double();
        view->last_zoom_time = view->last_button_time + 0.25;
        view->last_mpos_x = view->state.mpos_x;
        view->last_mpos_y = view->state.mpos_y;

        gv_manager_queue_task( gv_get_manager(), "zoompan-handler", 2,
                               gv_view_area_zoompan_handler, view );
    }
    else if( event->type == GDK_BUTTON_PRESS
             || event->type == GDK_BUTTON_RELEASE )
    {
        view->last_button = 0;
    }
}

void
gv_view_area_3d_move_event(GvViewArea *view, GdkEventButton *event)

{
    /* note: we want this function to work properly for "zoom mode"
       with no control, and the modeless "cntl-button" method. */

    /* Double Click */
    if( event->type == GDK_2BUTTON_PRESS )
    {
        view->last_button = 0;

        if( event->button == 1 )
            motion(view, 10.0, 0.0, 0.0);
        else if( event->button == 3 )
            motion(view, -10.0, 0.0, 0.0);

    }
    /* Click and hold */
    else if( event->type == GDK_BUTTON_PRESS
             && (event->button == 1 || event->button == 3) )
    {
        view->last_button = event->button;
        view->last_button_time = g_get_current_time_as_double();
        view->last_zoom_time = view->last_button_time + 0.25;
        view->last_mpos_x = view->state.mpos_x;
        view->last_mpos_y = view->state.mpos_y;

        gv_manager_queue_task( gv_get_manager(), "3d-motion-handler", 2,
                               gv_view_area_3d_motion_handler, view );
    }
    /* Click and release */
    else if( event->type == GDK_BUTTON_PRESS
             || event->type == GDK_BUTTON_RELEASE )
    {
        view->last_button = 0;
    }
}

static gint
gv_view_area_button_press(GtkWidget *widget, GdkEventButton *event)
{
    GvViewArea *view = GV_VIEW_AREA(widget);

    if (event->button == 4)
    {
        gv_view_area_zoom(view, -WHEEL_ZOOM_INC);
    }
    else if (event->button == 5)
    {
        gv_view_area_zoom(view, WHEEL_ZOOM_INC);
    }

    /* new zoom logic stuff */
    if( (event->button == 1 || event->button == 3)
        && (event->state & GDK_CONTROL_MASK) )
    {
        if ( view->flag_3d )
        {
            gv_view_area_3d_move_event( view, event);
        } else {
            gv_view_area_zoompan_event( view, event );
        }
    }
    else if (view->flag_3d && (event->button == 1 || event->button == 2 || event->button == 3))
    {
            gv_view_area_3d_move_event( view, event);
    }
    else if( event->state & GDK_CONTROL_MASK )
    {
        if ( view->flag_3d )
        {
            /* Nothing? */
        } else {
            gv_view_area_zoompan_event( view, event );
        }
    }


    view->state.mpos_x = event->x;
    view->state.mpos_y = event->y;

    return 0;
}

static gint
gv_view_area_button_release(GtkWidget *widget, GdkEventButton *event)
{
    GvViewArea *view = GV_VIEW_AREA(widget);

    view->state.mpos_x = event->x;
    view->state.mpos_y = event->y;

    if( view->dragging_mode )
    {
        gvgeocoord x[4], y[4], min_x, max_x, min_y, max_y;

        gv_view_area_map_pointer( view,
                                  view->state.mpos_x, view->state.mpos_y,
                                  x+0, y+0 );
        gv_view_area_map_pointer( view,
                                  view->last_mpos_x, view->state.mpos_y,
                                  x+1, y+1 );
        gv_view_area_map_pointer( view,
                                  view->last_mpos_x, view->last_mpos_y,
                                  x+2, y+2 );
        gv_view_area_map_pointer( view,
                                  view->state.mpos_x, view->last_mpos_y,
                                  x+3, y+3 );

        min_x = MIN(MIN(x[0],x[1]),MIN(x[2],x[3]));
        min_y = MIN(MIN(y[0],y[1]),MIN(y[2],y[3]));
        max_x = MAX(MAX(x[0],x[1]),MAX(x[2],x[3]));
        max_y = MAX(MAX(y[0],y[1]),MAX(y[2],y[3]));

        gv_view_area_fit_extents( view, min_x, min_y,
                                  max_x - min_x, max_y - min_y );

        view->dragging_mode = FALSE;
    }

    /* new zoom logic stuff */
    if( (event->button == 1 || event->button == 3) )
    {
        if ( view->flag_3d )
        {
            gv_view_area_3d_move_event(view, event);
        } else {
            gv_view_area_zoompan_event( view, event );
        }
    }

    return 0;
}

static gint
gv_view_area_key_press(GtkWidget *widget, GdkEventKey *event)
{
    GvViewArea *view = GV_VIEW_AREA(widget);

    gvgeocoord move = 0.0;
    gvgeocoord strafe = 0.0;
    gvgeocoord vert = 0.0;

    int     big_step_x, big_step_y;

    big_step_x = (int) (view->state.shape_x / 1.3);
    big_step_y = (int) (view->state.shape_x / 1.3);

    switch (event->keyval)
    {
      case GDK_z:
        if (event->state & GDK_CONTROL_MASK)
        {
            gv_undo_pop();
        }
        break;

      case GDK_Escape:
        if( view->dragging_mode )
        {
            view->dragging_mode = FALSE;
            gv_view_area_queue_draw( view );
        }
        break;

      case '-':
          if (view->flag_3d)
          {
              view->state.z_scale -= HEIGHT_SCALE*0.1;
              gv_view_area_state_changed(view);
          }
      break;

      case '_':
          if (view->flag_3d)
          {
              view->state.z_scale -= HEIGHT_SCALE;
              gv_view_area_state_changed(view);
          }
      break;

      case '=':
          if (view->flag_3d)
          {
              view->state.z_scale += HEIGHT_SCALE*0.1;
              gv_view_area_state_changed(view);
          }
      break;

      case '+':
          if (view->flag_3d)
          {
              view->state.z_scale += HEIGHT_SCALE;
              gv_view_area_state_changed(view);
          }
      break;

      case GDK_F1:
        if( event->state & GDK_CONTROL_MASK )
        {
            view->state.flip_x *= -1;
            gv_view_area_state_changed(view);
        }
        break;

      case GDK_F2:
          /* Flip along y */
        if( event->state & GDK_CONTROL_MASK )
        {
            view->state.flip_y *= -1;
            gv_view_area_state_changed(view);

        } else {

            /* Switch between 2D and 3D Modes on-the-fly */
            if ( view->flag_3d )
            {
                gv_view_area_set_mode(view, 0);
                gv_view_area_state_changed(view);
            } else {
                gv_view_area_set_mode(view, 1);
                gv_view_area_state_changed(view);
            }
        }
        break;

      case GDK_Page_Up:
        if (view->flag_3d)
        {
            move = 0.1;
            if (event->state & GDK_SHIFT_MASK)
                move = 1.0; /* Use bigger inc. if shifted */

            motion(view, move, strafe, vert);
        } else {
            gv_view_area_zoom( view, ZOOM_STEP );
        }
        break;

      case GDK_Page_Down:
        if (view->flag_3d)
        {
            move = -0.1;
            if (event->state & GDK_SHIFT_MASK)
                move = -1.0; /* Use bigger inc. if shifted */

            motion(view, move, strafe, vert);
        } else {
            gv_view_area_zoom( view, -ZOOM_STEP );
        }
        break;

      case GDK_Right:
          if (view->flag_3d)
          {
              strafe = 1.0;
              motion(view, move, strafe, vert);
          } else {

              if (event->state & GDK_CONTROL_MASK)
                  gv_view_area_translate( view, -big_step_x, 0 );
              else if (event->state & GDK_SHIFT_MASK)
                  gv_view_area_translate( view, -big_step_x/2, 0 );
              else
                  gv_view_area_translate( view, -10, 0 );
          }
          return TRUE; /* Don't let focus change */
          break;

      case GDK_Left:
          if (view->flag_3d)
          {
              strafe = -1.0;
              motion(view, move, strafe, vert);
          } else {
              if (event->state & GDK_CONTROL_MASK)
                  gv_view_area_translate( view, big_step_x, 0 );
              else if (event->state & GDK_SHIFT_MASK)
                  gv_view_area_translate( view, big_step_x/2, 0 );
              else
                  gv_view_area_translate( view, 10, 0 );
          }
          return TRUE; /* Don't let focus change */
          break;

      case GDK_Up:
          if (view->flag_3d)
          {
              vert = 1.0;
              motion(view, move, strafe, vert);
          } else {
              if (event->state & GDK_CONTROL_MASK)
                  gv_view_area_translate( view, 0, -big_step_y );
              else if (event->state & GDK_SHIFT_MASK)
                  gv_view_area_translate( view, 0, -big_step_y/2 );
              else
                  gv_view_area_translate( view, 0, -10 );
          }
          return TRUE; /* Don't let focus change */
          break;

      case GDK_Down:
          if (view->flag_3d)
          {
              vert = -1.0;
              motion(view, move, strafe, vert);
          } else {
              if (event->state & GDK_CONTROL_MASK)
                  gv_view_area_translate( view, 0, big_step_y );
              else if (event->state & GDK_SHIFT_MASK)
                  gv_view_area_translate( view, 0, big_step_y/2 );
              else
                  gv_view_area_translate( view, 0, 10 );
          }
          return TRUE; /* Don't let focus change */
          break;

      case GDK_Home:
        gv_view_area_fit_all_layers( view );
        break;
    }
    /* FIXME: add flipping controls, etc. */

    return 0;
}

static void
gv_view_area_change_notify(GvViewArea *view, gpointer info)
{
    gv_view_area_queue_draw(view);
}

static void
gv_view_area_state_changed(GvViewArea *view)
{
    g_signal_emit(view, view_area_signals[VIEW_STATE_CHANGED], 0);
    gv_view_area_queue_draw(view);
    gv_view_area_update_adjustments( view );
}

static void
gv_view_area_destroy(GtkObject *object)
{
    GvViewArea *view = GV_VIEW_AREA(object);

    CPLDebug( "OpenEV", "gv_view_area destroy" );

    /* Remove all layers */
    view->active_layer = NULL;
    while (view->layers != NULL)
    {
        GvLayer *layer = (GvLayer*)view->layers->data;
        gv_view_area_remove_layer(view, G_OBJECT(layer));
    }

    gv_view_area_set_adjustments( view, NULL, NULL );

    /* Call parent class function */
    GTK_OBJECT_CLASS(parent_class)->destroy(object);
}

static void
gv_view_area_finalize(GObject *gobject)
{
    /* GTK2 PORT... Override GObject finalize, test for and set NULLs
       as finalize may be called more than once. */

    CPLDebug( "OpenEV", "gv_view_area_finalize" );

    /* Call parent class finalize */
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}

static void
gv_view_area_set_arg (GtkObject        *object,
                      GtkArg           *arg,
                      guint             arg_id)
{
    GvViewArea *view = GV_VIEW_AREA (object);

    switch (arg_id)
    {
      case ARG_HADJUSTMENT:
        gv_view_area_set_adjustments (view,
                                      GTK_VALUE_POINTER (*arg),
                                      view->vadj);
        break;
      case ARG_VADJUSTMENT:
        gv_view_area_set_adjustments (view,
                                      view->hadj,
                                      GTK_VALUE_POINTER (*arg));
        break;
      default:
        break;
    }
}

static void
gv_view_area_get_arg (GtkObject        *object,
                      GtkArg           *arg,
                      guint             arg_id)
{
    GvViewArea *view = GV_VIEW_AREA (object);

    switch (arg_id)
    {
      case ARG_HADJUSTMENT:
        GTK_VALUE_POINTER (*arg) = view->hadj;
        break;
      case ARG_VADJUSTMENT:
        GTK_VALUE_POINTER (*arg) = view->vadj;
        break;
      default:
        arg->type = GTK_TYPE_INVALID;
        break;
    }
}

static void
gv_view_area_update_adjustments( GvViewArea *view )

{
    GvRect   world_extents;
    GvRect   view_extents;

    if( !GTK_WIDGET_REALIZED(GTK_WIDGET(view)) )
        return;

    if( view->hadj == NULL && view->vadj == NULL )
        return;

    if( view->lock_adjustments )
        return;

    gv_view_area_get_world_extents( view, &world_extents );
    if( world_extents.width == 0 || world_extents.height == 0 )
        return;

    if( world_extents.height < 0.0 )
    {
        world_extents.y += world_extents.height;
        world_extents.height *= -1;
    }

    gv_view_area_get_extents( view, &view_extents.x, &view_extents.y,
                              &view_extents.width, &view_extents.height);
    view_extents.width = ABS(view_extents.x-view_extents.width);
    view_extents.height = ABS(view_extents.y-view_extents.height);

    view->lock_adjustments = TRUE;

    if( view->hadj != NULL )
    {
        if( view->state.flip_x < 0 )
        {
            view->hadj->upper = -world_extents.x;
            view->hadj->lower = -(world_extents.x + world_extents.width);
            view->hadj->value = -(view_extents.x+view_extents.width);
            view->hadj->page_size = ABS(view_extents.width);
        }
        else
        {
            view->hadj->lower = world_extents.x;
            view->hadj->upper = world_extents.x + world_extents.width;
            view->hadj->value = view_extents.x;
            view->hadj->page_size = view_extents.width;
        }

        if( view->hadj->page_size > view->hadj->upper - view->hadj->lower )
            view->hadj->page_size = view->hadj->upper - view->hadj->lower;
        if( view->hadj->value < view->hadj->lower )
            view->hadj->value = view->hadj->lower;
        if( view->hadj->value+view->hadj->page_size > view->hadj->upper )
            view->hadj->value = view->hadj->upper - view->hadj->page_size;

        view->hadj->page_increment = view->hadj->page_size;
        view->hadj->step_increment = view->hadj->page_increment / 4;

        gtk_adjustment_changed( view->hadj );
    }

    if( view->vadj != NULL )
    {
        if( view->state.flip_y > 0 )
        {
            view->vadj->upper = -world_extents.y;
            view->vadj->lower = -(world_extents.y + world_extents.height);
            view->vadj->value = -(view_extents.y+view_extents.height);
            view->vadj->page_size = ABS(view_extents.height);
        }
        else
        {
            view->vadj->lower = world_extents.y;
            view->vadj->upper = world_extents.y + world_extents.height;
            view->vadj->value = view_extents.y;
            view->vadj->page_size = view_extents.height;
        }

        if( view->vadj->page_size > view->vadj->upper - view->vadj->lower )
            view->vadj->page_size = view->vadj->upper - view->vadj->lower;
        if( view->vadj->value < view->vadj->lower )
            view->vadj->value = view->vadj->lower;
        if( view->vadj->value+view->vadj->page_size > view->vadj->upper )
            view->vadj->value = view->vadj->upper - view->vadj->page_size;

        view->vadj->page_increment = view->vadj->page_size;
        view->vadj->step_increment = view->vadj->page_increment / 4;

        gtk_adjustment_changed( view->vadj );
    }

    view->lock_adjustments = FALSE;
}

static void
gv_view_area_adjustment (GtkAdjustment *adjustment,
                         GvViewArea    *view)
{
    gvgeocoord     xmin, ymin, width, height;
    static int local_lock = FALSE;

    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
    g_return_if_fail (view != NULL);
    g_return_if_fail (GV_IS_VIEW_AREA (view));

    if( !GTK_WIDGET_REALIZED(GTK_WIDGET(view)) )
        return;

    if( view->lock_adjustments || local_lock )
        return;

    if( view->vadj == NULL || view->hadj == NULL )
        return;

    xmin = view->hadj->value * view->state.flip_x;
    width = view->hadj->page_size * view->state.flip_x;

    if( view->state.flip_y > 0 )
    {
        ymin = (-1 * view->vadj->value) - view->vadj->page_size;
        height = view->vadj->page_size;
    }
    else
    {
        ymin = view->vadj->value;
        height = view->vadj->page_size;
    }

    view->lock_adjustments = TRUE;
    gv_view_area_fit_extents( view, xmin, ymin, width, height );
    view->lock_adjustments = FALSE;

    local_lock = TRUE;
    gv_view_area_update_adjustments( view );
    local_lock = FALSE;
}

static void
gv_view_area_disconnect (GtkAdjustment *adjustment,
                         GvViewArea    *view)
{
    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
    g_return_if_fail (view != NULL);
    g_return_if_fail (GV_IS_VIEW_AREA (view));

    if (adjustment == view->hadj)
        gv_view_area_set_adjustments (view, NULL, view->vadj);
    if (adjustment == view->vadj)
        gv_view_area_set_adjustments (view, view->hadj, NULL);
}

void
gv_view_area_set_adjustments (GvViewArea    *view,
                              GtkAdjustment *hadj,
                              GtkAdjustment *vadj)
{
    g_return_if_fail (view != NULL);
    g_return_if_fail (GV_VIEW_AREA (view));

    if (hadj)
        g_return_if_fail (GTK_IS_ADJUSTMENT (hadj));

    if (vadj)
        g_return_if_fail (GTK_IS_ADJUSTMENT (vadj));

    if (view->hadj && (view->hadj != hadj))
    {
        g_signal_handlers_disconnect_matched (G_OBJECT(view->hadj), G_SIGNAL_MATCH_DATA,
                                                0, 0, NULL, NULL, G_OBJECT(view));
        g_object_unref (view->hadj);
    }

    if (view->vadj && (view->vadj != vadj))
    {
        g_signal_handlers_disconnect_matched (G_OBJECT(view->vadj), G_SIGNAL_MATCH_DATA,
                                                0, 0, NULL, NULL, G_OBJECT(view));
        g_object_unref (view->vadj);
    }

    if( hadj == NULL )
        view->hadj = NULL;
    else if (view->hadj != hadj)
    {
        view->hadj = hadj;
        g_object_ref (view->hadj);
        /*gtk_object_sink (GTK_OBJECT (view->hadj));*/

        g_signal_connect (view->hadj, "changed",
                            G_CALLBACK (gv_view_area_adjustment),
                            view);
        g_signal_connect (view->hadj, "value_changed",
                            G_CALLBACK (gv_view_area_adjustment),
                            view);
        /* GTK2 PORT.. PENDING - Need alternative for this?
        gtk_signal_connect (GTK_OBJECT (view->hadj), "disconnect",
                            (GtkSignalFunc) gv_view_area_disconnect,
                            view);
        */
        gv_view_area_adjustment (hadj, view);
    }

    if( vadj == NULL )
        view->vadj = NULL;
    else if (view->vadj != vadj)
    {
        view->vadj = vadj;
        g_object_ref (view->vadj);
        /*gtk_object_sink (GTK_OBJECT (view->vadj));*/

        g_signal_connect (view->vadj, "changed",
                            G_CALLBACK (gv_view_area_adjustment),
                            view);
        g_signal_connect (view->vadj, "value_changed",
                            G_CALLBACK (gv_view_area_adjustment),
                            view);
        /* GTK2 PORT.. PENDING - Need alternative for this?
        gtk_signal_connect (GTK_OBJECT (view->vadj), "disconnect",
                            (GtkSignalFunc) gv_view_area_disconnect,
                            view);
        */
        gv_view_area_adjustment (vadj, view);
    }

    gv_view_area_update_adjustments( view );
}

/************************************************************************/
/*                        gv_view_area_set_raw()                        */
/*                                                                      */
/*      Try to reset whether this layer is in georeferenced or raw      */
/*      mode relative to the indicated raster layer.                    */
/************************************************************************/

int gv_view_area_set_raw(GvViewArea *view, GObject *ref_layer, int raw_enable)
{
    GvRasterLayer *rlayer = NULL;
    gvgeocoord    xmin, ymin, xmax, ymax;
    double    pl_xmin, pl_ymin, pl_xmax, pl_ymax;

    if( ref_layer != NULL && GV_IS_RASTER_LAYER(ref_layer) )
        rlayer = GV_RASTER_LAYER(ref_layer);
    else
        rlayer = GV_RASTER_LAYER(gv_view_area_get_primary_raster(view));

    if( !raw_enable == !gv_view_area_get_raw(view, ref_layer) )
        return TRUE;

    /* For now we need a raster layer. */
    if( rlayer == NULL )
        return FALSE;

    /* Get the current view extents, and transform to raster PL coordinates.*/
    gv_view_area_get_extents( view, &xmin, &ymin, &xmax, &ymax );

    pl_xmin = xmin;
    pl_ymin = ymin;
    pl_xmax = xmax;
    pl_ymax = ymax;

    if( !gv_raster_layer_view_to_pixel( rlayer, &pl_xmin, &pl_ymin, NULL )
        || !gv_raster_layer_view_to_pixel( rlayer, &pl_xmax, &pl_ymax, NULL ) )
        return FALSE;

    /* Now force the raster layer to change it's mesh coordinate system. */
    if( !gv_raster_layer_set_raw( rlayer, raw_enable ) )
        return FALSE;

    /* Clear or set projection on view. */
    if( raw_enable || gv_data_get_projection(GV_DATA(rlayer)) == NULL )
        gv_view_area_set_projection( view, "PIXEL" );
    else
        gv_view_area_set_projection( view,
                                     gv_data_get_projection(GV_DATA(rlayer)) );

    /* Reset the view */
    if( !gv_raster_layer_pixel_to_view( rlayer, &pl_xmin, &pl_ymin, NULL )
        || !gv_raster_layer_pixel_to_view( rlayer, &pl_xmax, &pl_ymax, NULL ) )
        return FALSE;

    /*
     * A bunch of hacky stuff to ensure the correct orientation of the
     * new extents even if this requires some flipping.  We must always
     * keep an upper-right origin in pixel/line space (raw) and a lower left
     * origin in georeferenced space.
     */
    if( !raw_enable && pl_ymax < pl_ymin )
    {
        double  temp;

        temp = pl_ymax;
        pl_ymax = pl_ymin;
        pl_ymin = temp;
    }
    else if( raw_enable && pl_ymax > pl_ymin )
    {
        double  temp;

        temp = pl_ymax;
        pl_ymax = pl_ymin;
        pl_ymin = temp;
    }
    if( pl_xmax < pl_xmin )
    {
        double  temp;

        temp = pl_xmax;
        pl_xmax = pl_xmin;
        pl_xmin = temp;

    }

    if( !raw_enable )
        view->state.flip_y = 1.0;

    gv_view_area_fit_extents( view,
                              pl_xmin, pl_ymin,
                              pl_xmax - pl_xmin, pl_ymax - pl_ymin );

    return TRUE;
}

/************************************************************************/
/*                        gv_view_area_get_raw()                        */
/*                                                                      */
/*      Determine whether this view is in "raw" mode relative to the    */
/*      given layer.  TRUE is returned for raw mode.                    */
/************************************************************************/

int gv_view_area_get_raw( GvViewArea *view, GObject *ref_layer )

{
    GvRasterLayer *rlayer = NULL;

    if( ref_layer != NULL && GV_IS_RASTER_LAYER(ref_layer) )
        rlayer = GV_RASTER_LAYER(ref_layer);
    else
        rlayer = GV_RASTER_LAYER(gv_view_area_get_primary_raster(view));

    if( rlayer )
        return rlayer->mesh_is_raw;
    else
        return FALSE;
}

/************************************************************************/
/*                    gv_view_area_redraw_timeout()                     */
/*                                                                      */
/*      Returns TRUE if "quite a while" has elapsed since the last      */
/*      view redraw.  This is used by the various layers to decide      */
/*      if they should stop doing idle work.                            */
/************************************************************************/

int gv_view_area_redraw_timeout( GvViewArea *view )

{
    float   max_work_time;

    if( view->redraw_timer == NULL )
        return TRUE;

    max_work_time = MAX(0.25,MIN(2.0,view->last_draw_time*3));

    if( g_timer_elapsed(view->redraw_timer,NULL) > max_work_time )
    {
        g_timer_destroy( view->redraw_timer );
        view->redraw_timer = NULL;
        return TRUE;
    }
    else
        return FALSE;
}

/************************************************************************/
/*                   gv_view_area_pending_idle_work()                   */
/*                                                                      */
/*      Returns TRUE if any of the layers displayed on this view        */
/*      report they still have idle work pending, else it returns       */
/*      FALSE.                                                          */
/************************************************************************/

int gv_view_area_pending_idle_work( GvViewArea *view )

{
    GList *list;


    list = view->layers;
    while (list)
    {
    GvLayer *layer = (GvLayer*)list->data;
        if( layer->pending_idle )
            return TRUE;
    list = g_list_next(list);
    }

    return FALSE;
}

/************************************************************************/
/*                       gv_get_render_counter()                        */
/************************************************************************/

int gv_get_render_counter()

{
    return render_counter;
}

/************************************************************************/
/*                  gv_view_area_get_primary_raster()                   */
/************************************************************************/

GObject *gv_view_area_get_primary_raster( GvViewArea *view )

{
    GList *layer_list, *node;

    layer_list = gv_view_area_list_layers( view );

    for( node = layer_list; node != NULL; node = node->next )
    {
        GvLayer *layer;

        layer = GV_LAYER(node->data);
        if( gv_layer_is_visible(layer) && GV_IS_RASTER_LAYER( layer ) )
            return G_OBJECT(layer);
    }

    return NULL;
}

const char *
gv_view_area_get_property(GvViewArea *data, const char *name)
{
    return gv_properties_get( &(data->properties), name );
}

void
gv_view_area_set_property(GvViewArea *data, const char *name, const char *value)
{
    gv_properties_set( &(data->properties), name, value );
}

GvProperties *
gv_view_area_get_properties(GvViewArea *data)
{
    return &(data->properties);
}

/*
 * Moved from gvutils, renamed from gv_format_point_query
 */
const char *gv_view_area_format_point_query(GvViewArea *view,
                                            GvProperties *properties,
                                            double geo_x, double geo_y)

{
    static gchar buf[256];
    double raster_x=0.0, raster_y=0.0, pix_real, pix_imaginary;
    GvLayer *layer;
    GvRaster *raster = NULL;
    GvRasterLayer *raster_layer = NULL;
    GList *layer_list, *node;
    const char *coord_mode, *pixel_mode, *degree_mode;
    const char *coord_sys = gv_view_area_get_projection(view);
    char    *latlong_srs = NULL;
    char east_west, north_south;
    int isource;
    double nodata[2];


    /* Check the properties to see what options are in effect */
    coord_mode = gv_properties_get( properties, "_coordinate_mode" );
    if( coord_mode == NULL )
        coord_mode = "georef";

    pixel_mode = gv_properties_get( properties, "_pixel_mode" );
    if( pixel_mode == NULL )
        pixel_mode = "on";

    degree_mode = gv_properties_get( properties, "_degree_mode" );
    if( degree_mode == NULL )
        degree_mode = "dms";

    /* This code should be factored out into gvviewarea at some point. */
    layer_list = gv_view_area_list_layers( view );

    for( node = layer_list; node != NULL; node = node->next )
    {
        layer = GV_LAYER(node->data);
        if( gv_layer_is_visible(layer) && GV_IS_RASTER_LAYER( layer ) )
        {
            raster = GV_RASTER_LAYER(layer)->prototype_data;
            raster_layer = GV_RASTER_LAYER(layer);

            raster_x = geo_x;
            raster_y = geo_y;

            if( !gv_raster_layer_view_to_pixel( raster_layer,
                                                &raster_x, &raster_y, NULL )
                ||raster_x < 0 || raster_x >= raster->width
                || raster_y < 0 || raster_y >= raster->height )
            {
                raster = NULL;
                raster_layer = NULL;
            }
        }
    }

    /* Do we want to transform coordinate into a real `geo' space? */
    if( raster != NULL
        && (gv_view_area_get_projection(view) == NULL
            || EQUAL(gv_view_area_get_projection(view),"PIXEL"))
        && raster_layer->mesh_is_raw )
    {
        double geo_x_dbl, geo_y_dbl;

        geo_x_dbl = raster_x;
        geo_y_dbl = raster_y;
        gv_raster_pixel_to_georef( raster,
                                   &geo_x_dbl, &geo_y_dbl, NULL );
        coord_sys = gv_data_get_projection( GV_DATA(raster) );
        geo_x = geo_x_dbl;
        geo_y = geo_y_dbl;
    }

    if( g_strcasecmp(coord_mode,"latlong") == 0
        && (coord_sys != NULL && !EQUAL(coord_sys,"PIXEL")) )
    {
        latlong_srs = gv_make_latlong_srs( coord_sys );

        if( latlong_srs != NULL )
        {
            double      x, y, z;

            x = geo_x;
            y = geo_y;
            z = 0.0;

            if( gv_reproject_points( coord_sys, latlong_srs,
                                     1, &x, &y, &z ) )
            {
                geo_x = x;
                geo_y = y;
                coord_sys = latlong_srs;
            }
        }
    }

    buf[0] = '\0';
    if( g_strcasecmp(coord_mode,"off") == 0 )
    {
        /* do nothing */
    }
    else if( g_strcasecmp(coord_mode,"raster") == 0 )
    {
        if( raster != NULL )
            g_snprintf(buf, 64, "(%.2fP, %.2fL)", raster_x, raster_y );
    }
    else
    {
        if( coord_sys != NULL
            && strstr(coord_sys,"PROJCS") == NULL
            && strstr(coord_sys,"GEOGCS") != NULL )
        {
            /* Display in Degree Minute Second format */
            if ( g_strcasecmp(degree_mode, "dms") == 0 )
            {
                strcat( buf, "(");
                strcat( buf, GDALDecToDMS( geo_x, "Long", 2 ));
                strcat( buf, "," );
                strcat( buf, GDALDecToDMS( geo_y, "Lat", 2 ));
                strcat( buf, ")" );
            } else {
                /* Display in decimal format */
                if (geo_x < 0)
                    east_west = 'W';
                else
                    east_west = 'E';

                if (geo_y < 0)
                    north_south = 'S';
                else
                    north_south = 'N';

                g_snprintf(buf, 64, "(%.7f%c, %.7f%c)",
                           fabs(geo_x), east_west, fabs(geo_y), north_south );
            }
        }
        else if( coord_sys != NULL
                 && g_strcasecmp(coord_sys,"PIXEL") == 0 )
        {
            g_snprintf(buf, 64, "(%.2fP, %.2fL)", geo_x, geo_y );
        }
        else
        {
            g_snprintf(buf, 64, "(%.2fE, %.2fN)", geo_x, geo_y );
        }
    }

    if( latlong_srs != NULL )
        g_free( latlong_srs );

    /* Try to get a raster value */
    if( raster_layer != NULL
        && g_strcasecmp(pixel_mode, "on") == 0 )
    {
        const char  *nodata_mode;
        GvRaster    *rsrc;

        nodata_mode = gv_properties_get( properties, "_nodata_mode" );
        if( nodata_mode == NULL )
            nodata_mode = "on";

        if( buf[0] != '\0' )
            strcat( buf, ": " );

        if( raster_layer->mode == GV_RLM_COMPLEX )
        {
            rsrc = gv_raster_layer_get_data(raster_layer,0);
            if( rsrc != NULL
                && gv_raster_get_sample( rsrc, raster_x, raster_y,
                                         &pix_real, &pix_imaginary ) )
            {
                float   phase, magnitude;

                if( pix_imaginary < 0.0 )
                    g_snprintf(buf+strlen(buf), 64, "%g%gi",
                               pix_real, pix_imaginary );
                else
                    g_snprintf(buf+strlen(buf), 64, "%g+%gi",
                               pix_real, pix_imaginary );

                gv_complex_to_phase_mag( pix_real, pix_imaginary,
                                         &phase, &magnitude );
                g_snprintf(buf+strlen(buf), 64, ", phase:%g, magnitude:%g",
                           phase, magnitude );

                if ( g_strcasecmp( nodata_mode, "on") == 0 )
                {
                    if( gv_raster_layer_nodata_get( raster_layer, 0,
                                                    &nodata[0], &nodata[1] )
                        && ABS(pix_real - nodata[0]) < 0.0000000001
                        && ABS(pix_imaginary - nodata[1]) < 0.0000000001 )
                    {
                        g_snprintf(buf+strlen(buf), 64, " [NODATA]" );
                    }
                }
            }
        }
        else if( raster_layer->mode == GV_RLM_RGBA )
        {
            double  rgba[4];
            double  rgba_imag[4];

            for( isource = 0; isource < 4; isource++ )
            {

                rsrc = gv_raster_layer_get_data(raster_layer,isource);
                if( rsrc == NULL
                    || !gv_raster_get_sample( rsrc, raster_x, raster_y,
                                              rgba + isource, 
                                              rgba_imag + isource) )
                {
                    rgba[isource] =
                        gv_raster_layer_get_const_value(raster_layer,isource);
                }
            }

            if( gv_raster_layer_get_data(raster_layer,3) == NULL )
            {
                if ((rgba_imag[0] != 0.0) || (rgba_imag[1] != 0.0) ||
                    (rgba_imag[2] != 0.0))
                {
                    if (rgba_imag[0] < 0.0)
                        g_snprintf(buf+strlen(buf), 64, "%.5g%.5gi r ",
                            rgba[0], rgba_imag[0] );
                    else
                        g_snprintf(buf+strlen(buf), 64, "%.5g+%.5gi r ",
                            rgba[0], rgba_imag[0] );

                    if (rgba_imag[1] < 0.0)
                        g_snprintf(buf+strlen(buf), 64, "%.5g%.5gi g ",
                            rgba[1], rgba_imag[1] );
                    else
                        g_snprintf(buf+strlen(buf), 64, "%.5g+%.5gi g ",
                            rgba[1], rgba_imag[1] );

                    if (rgba_imag[2] < 0.0)
                        g_snprintf(buf+strlen(buf), 64, "%.5g%.5gi b ",
                            rgba[2], rgba_imag[2] );
                    else
                        g_snprintf(buf+strlen(buf), 64, "%.5g+%.5gi b ",
                            rgba[2], rgba_imag[2] );
                }
                else
                    g_snprintf(buf+strlen(buf), 64, "%gr %gg %gb ",
                           rgba[0], rgba[1], rgba[2] );
            }
            else
            {
                if ((rgba_imag[0] != 0) || (rgba_imag[1] != 0) ||
                    (rgba_imag[2] != 0) || (rgba_imag[3] != 0))
                {
                    if (rgba_imag[0] < 0.0)
                        g_snprintf(buf+strlen(buf), 64, "%.4g%.4gi r ",
                            rgba[0], rgba_imag[0] );
                    else
                        g_snprintf(buf+strlen(buf), 64, "%.4g+%.4gi r ",
                            rgba[0], rgba_imag[0] );

                    if (rgba_imag[1] < 0.0)
                        g_snprintf(buf+strlen(buf), 64, "%.4g%.4gi g ",
                            rgba[1], rgba_imag[1] );
                    else
                        g_snprintf(buf+strlen(buf), 64, "%.4g+%.4gi g ",
                            rgba[1], rgba_imag[1] );

                    if (rgba_imag[2] < 0.0)
                        g_snprintf(buf+strlen(buf), 64, "%.4g%.4gi b ",
                            rgba[2], rgba_imag[2] );
                    else
                        g_snprintf(buf+strlen(buf), 64, "%.4g+%.4gi b ",
                            rgba[2], rgba_imag[2] );

                    if (rgba_imag[3] < 0.0)
                        g_snprintf(buf+strlen(buf), 64, "%.4g%.4gi a ",
                            rgba[3], rgba_imag[3] );
                    else
                        g_snprintf(buf+strlen(buf), 64, "%.4g+%.4gi a ",
                            rgba[3], rgba_imag[3] );
                }
                else
                    g_snprintf(buf+strlen(buf), 64, "%gr %gg %gb %ga",
                           rgba[0], rgba[1], rgba[2], rgba[3] );
            }

            if ( g_strcasecmp( nodata_mode, "on") == 0 )
            {
                if( (gv_raster_layer_nodata_get(raster_layer, 0, &nodata[0], NULL) 
                     && ABS(rgba[0] - nodata[0]) < 0.0000000001) || 
                    (gv_raster_layer_nodata_get(raster_layer, 1, &nodata[1], NULL) 
                     && ABS(rgba[1] - nodata[1]) < 0.0000000001) ||
                    (gv_raster_layer_nodata_get(raster_layer, 2, &nodata[2], NULL) 
                     && ABS(rgba[2] - nodata[2]) < 0.0000000001) )
                {
                    g_snprintf(buf+strlen(buf), 64, " [NODATA]" );
                }
            }
        }

        else if( raster_layer->mode == GV_RLM_PSCI )
        {
            double psci[2];
            double psci_imag[2];

            for( isource = 0; isource < 2; isource++ ) {
                rsrc = gv_raster_layer_get_data(raster_layer,isource);
                if( rsrc == NULL
                    || !gv_raster_get_sample( rsrc, raster_x, raster_y,
                                              psci + isource, 
                                              psci_imag + isource) )
                {
                    psci[isource] =
                        gv_raster_layer_get_const_value(raster_layer,isource);
                }
            }

            g_snprintf(buf+strlen(buf), 64, "%g hue %g intensity ",
                       psci[0], psci[1] );

            if ( g_strcasecmp( nodata_mode, "on") == 0 )
            {
                if( (gv_raster_layer_nodata_get(raster_layer, 0, &nodata[0], NULL) 
                     && ABS(psci[0] - nodata[0]) < 0.0000000001) || 
                    (gv_raster_layer_nodata_get(raster_layer, 1, &nodata[1], NULL) 
                     && ABS(psci[1] - nodata[1]) < 0.0000000001) )
                {
                    g_snprintf(buf+strlen(buf), 64, " [NODATA]" );
                }
            }
        }

        else
        {
            rsrc = gv_raster_layer_get_data(raster_layer,0);
            if( rsrc != NULL &&
                gv_raster_get_sample( rsrc, raster_x, raster_y,
                                      &pix_real, &pix_imaginary ) )
            {
                g_snprintf(buf+strlen(buf), 64, "%g", pix_real);
            }

            if ( g_strcasecmp( nodata_mode, "on") == 0 )
            {
                if( gv_raster_layer_nodata_get( raster_layer, 0, &nodata[0], NULL )
                    && ABS(pix_real - nodata[0]) < 0.0000000001 )
                {
                    g_snprintf(buf+strlen(buf), 64, " [NODATA]" );
                }
            }
        }
    }

    return buf;
}
