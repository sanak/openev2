/******************************************************************************
 * $Id: gvviewarea.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  GTK/OpenGL View Canvas
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
 * $Log: gvviewarea.h,v $
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
 * Revision 1.60  2005/01/14 15:27:28  warmerda
 * added flip flag access
 *
 * Revision 1.59  2003/08/27 19:58:43  warmerda
 * added force_simple flag for gv_view_area_bmfont_draw
 *
 * Revision 1.58  2003/03/07 22:18:25  warmerda
 * const correctness fix for get_layer_by_name
 *
 * Revision 1.57  2003/02/20 19:27:18  gmwalter
 * Updated link tool to include Diana's ghost cursor code, and added functions
 * to allow the cursor and link mechanism to use different gcps
 * than the display for georeferencing.  Updated raster properties
 * dialog for multi-band case.  Added some signals to layerdlg.py and
 * oeattedit.py to make it easier for tools to interact with them.
 * A few random bug fixes.
 *
 * Revision 1.56  2002/11/05 04:14:54  warmerda
 * fixed prototype
 *
 * Revision 1.55  2002/11/05 04:14:11  warmerda
 * added prototype
 *
 * Revision 1.54  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.53  2002/09/10 13:26:43  warmerda
 * added get_height_scale method
 *
 * Revision 1.52  2002/07/16 14:17:06  warmerda
 * added support for getting background color
 *
 * Revision 1.51  2002/07/08 19:44:39  warmerda
 * added properties on GvViewArea
 *
 * Revision 1.50  2002/03/20 19:19:14  warmerda
 * added exact_render flag
 *
 * Revision 1.49  2002/01/30 17:25:19  warmerda
 * added set_state and get_primary_raster functions
 *
 * Revision 1.48  2001/12/13 03:29:17  warmerda
 * avoid purging textures used in this render
 *
 * Revision 1.47  2001/10/12 17:44:18  warmerda
 * avoid extra redraws when many raster layers displayed
 *
 * Revision 1.46  2001/10/12 01:58:19  warmerda
 * avoid re-rendering if backing store OK
 *
 * Revision 1.45  2001/07/03 14:26:05  warmerda
 * added set/get raw ability
 *
 * Revision 1.44  2001/04/09 18:20:14  warmerda
 * added ability to query list of available fonts
 *
 * Revision 1.43  2001/03/26 19:18:35  warmerda
 * restructure bmfont handling to preserve GdkFont handle
 *
 * Revision 1.42  2001/02/03 22:21:08  warmerda
 * added gv_view_area_get_mode() and python covers
 *
 * Revision 1.41  2000/10/06 16:48:56  warmerda
 * added GvViewArea background color
 *
 * Revision 1.40  2000/09/29 16:09:17  srawlin
 * added Goto function requring fuction to map lat/long to view coordinates
 *
 * Revision 1.39  2000/09/27 19:16:33  warmerda
 * *** empty log message ***
 *
 * Revision 1.38  2000/09/21 02:57:20  warmerda
 * reorganized bitmap font support to allow any gdk supported font at runtime
 *
 * Revision 1.37  2000/09/13 15:58:55  srawlin
 * added python bindings for gv_view_area_get_zoom
 *
 * Revision 1.36  2000/08/16 14:07:47  warmerda
 * added prototype scrollbar support
 *
 * Revision 1.35  2000/08/07 17:18:13  warmerda
 * added windows printing support
 *
 * Revision 1.34  2000/08/03 18:20:41  warmerda
 * implemented print scaling and paper sizes properly
 *
 * Revision 1.33  2000/07/21 01:31:11  warmerda
 * added read_only flag for GvData, and utilize for vector layers
 *
 * Revision 1.32  2000/07/20 03:21:26  warmerda
 * added is_rgb for print_to_file()
 *
 * Revision 1.31  2000/07/17 19:10:00  warmerda
 * added tentative support for scaling wait between redraws to actual redraw time
 *
 * Revision 1.30  2000/07/13 18:05:22  srawlin
 * removed use of view.state.eye_az and .eye_el, contained same information 
 * as view.state.eye_dir
 *
 * Revision 1.29  2000/07/11 20:56:23  srawlin
 * added methods to get and set viewing direction relative to z-plane in 3D
 *
 * Revision 1.28  2000/07/10 16:20:50  srawlin
 * removed unused function def
 *
 * Revision 1.27  2000/07/10 13:36:58  srawlin
 * updated 3D controls to be more like 2D
 *
 * Revision 1.26  2000/07/03 20:58:31  warmerda
 * eye_pos in georef coordinates now
 *
 * Revision 1.25  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_VIEW_AREA_H__
#define __GV_VIEW_AREA_H__

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include "gvtypes.h"
#include "gvproperties.h"

#define GV_TYPE_VIEW_AREA            (gv_view_area_get_type ())
#define GV_VIEW_AREA(obj)            (GTK_CHECK_CAST ((obj), GV_TYPE_VIEW_AREA, GvViewArea))
#define GV_VIEW_AREA_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GV_TYPE_VIEW_AREA, GvViewAreaClass))
#define GV_IS_VIEW_AREA(obj)         (GTK_CHECK_TYPE ((obj), GV_TYPE_VIEW_AREA))
#define GV_IS_VIEW_AREA_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_VIEW_AREA))

typedef struct _GvViewArea GvViewArea;
typedef struct _GvViewAreaClass GvViewAreaClass;
typedef struct _GvViewAreaState GvViewAreaState;

typedef gvgeocoord vec3_t[3];  /* xyz */

struct _GvViewAreaState
{
    gvgeocoord tx, ty;                        /* translation of center (georef) */
    gvgeocoord rot;                           /* xy-plane rotation in degrees */
    gvgeocoord zoom;                          /* log 2 based zoom, 0 is 1:1 */
    gvgeocoord linear_zoom;                   /* xy plane zoom */
    gvgeocoord flip_x,  flip_y;
    gvgeocoord shape_x, shape_y;              /* width/height of window in pixels */
    gvgeocoord mpos_x,  mpos_y;               /* mouse position (pixel/line)*/

    vec3_t eye_pos, eye_dir;             /* 3D only, eye location and view 
                                            direction */
    gvgeocoord z_scale;
};

typedef struct 
{
    char *name;
    gint listbase;
    PangoFontDescription *pango_desc;
} GvBMFontInfo;

struct _GvViewArea
{
    GtkDrawingArea glarea;

    GvViewAreaState state;

    GList *layers;
    GtkObject *active_layer;

    GArray   *bmfonts;

    char     *projection;                /* projection of view is in, eg UTM */

    GvColor   background;                /* background color used in clears */

    int       exact_render;              /* normally set for prints */

    int flag_3d;                         /* 2D or 3D mode flag */

    double    linear_measure;            /* Diaganonal of view extents for
                                            scaling linear step sizes */
    int       volume_current;
    double    view_volume[6];            /* xmin/xmax/ymin/ymax/zmin/zmax */
    
    gint      last_button;               /* zero mean no current button */
    double    last_button_time;
    double    last_zoom_time;

    gvgeocoord   last_mpos_x, last_mpos_y;
    gint      dragging_mode;

    GtkAdjustment *hadj;
    GtkAdjustment *vadj;
    int        lock_adjustments;

    int        display_dirty;            /*does the display need rerendering?*/

    /* ghost cursor stuff */
    gvgeocoord   next_x, next_y;
    gvgeocoord   last_x, last_y;
    int        next_valid;    /* flag to indicate if ghost cursor should draw next_x, next_y */
    int        last_valid;    /* flag to indicate if last cursor should be erased (logical cursor) */
    /* end of ghost cursor stuff */

    float     last_draw_time;
    GTimer   *redraw_timer;

    GvProperties  properties;
};

struct _GvViewAreaClass
{
    GtkDrawingAreaClass parent_class;

    void (* gldraw) (GvViewArea *view);
    void (* glcursor) (GvViewArea *view);
    void (* active_changed) (GvViewArea *view);
    void (* view_state_changed) (GvViewArea *view);
    void  (*set_scroll_adjustments)   (GvViewArea    *view,
                                       GtkAdjustment  *hadjustment,
                                       GtkAdjustment  *vadjustment);
};

#define gv_view_area_get_width(view) (view)->state.shape_x
#define gv_view_area_get_height(view) (view)->state.shape_y

GtkType    gv_view_area_get_type();
GtkWidget* gv_view_area_new();

void gv_view_area_set_mode(GvViewArea *view, int flag_3d);
int  gv_view_area_get_mode(GvViewArea *view);
void gv_view_area_height_scale(GvViewArea *view, gvgeocoord scale);
gvgeocoord gv_view_area_get_height_scale(GvViewArea *view);
void gv_view_area_set_3d_view(GvViewArea *view, vec3_t eye_pos, vec3_t eye_dir);
void gv_view_area_set_3d_view_look_at(GvViewArea *view, vec3_t eye_pos, gvgeocoord *eye_look_at);
gint gv_view_area_get_look_at_pos(GvViewArea *view, gvgeocoord *x, gvgeocoord *y);

gint gv_view_area_set_raw(GvViewArea *view, GtkObject *ref_layer, int raw_enable);
gint gv_view_area_get_raw(GvViewArea *view, GtkObject *ref_layer);

void gv_view_area_queue_draw(GvViewArea *view);
void gv_view_area_zoom(GvViewArea *view, gvgeocoord zoom);
gvgeocoord gv_view_area_get_zoom(GvViewArea *view);
void gv_view_area_rotate(GvViewArea *view, gvgeocoord angle);
void gv_view_area_translate(GvViewArea *view, gvgeocoord dx, gvgeocoord dy);
void gv_view_area_set_translation(GvViewArea *view, gvgeocoord x, gvgeocoord y);
int gv_view_area_get_flip_x(GvViewArea *view);
int gv_view_area_get_flip_y(GvViewArea *view);
void gv_view_area_set_flip_xy(GvViewArea *view, int flip_x, int flip_y);
void gv_view_area_fit_all_layers(GvViewArea *view);
void gv_view_area_fit_extents(GvViewArea *view, gvgeocoord llx, gvgeocoord lly, gvgeocoord width, gvgeocoord height);
gint gv_view_area_expose(GtkWidget *view, GdkEventExpose *event);
void gv_view_area_get_extents(GvViewArea *view, gvgeocoord *xmin, gvgeocoord *ymin, gvgeocoord *xmax, gvgeocoord *ymax);
void gv_view_area_get_world_extents(GvViewArea *view, GvRect *extents);
void gv_view_area_get_volume(GvViewArea *view, double *volume);

void gv_view_area_map_location(GvViewArea *view, gvgeocoord x, gvgeocoord y, gvgeocoord *geo_x, gvgeocoord *geo_y);
void gv_view_area_copy_state(GvViewArea *view, GvViewArea *copy);
void gv_view_area_set_state(GvViewArea *view, GvViewAreaState *state);
void gv_view_area_map_pointer(GvViewArea *view, gvgeocoord px, gvgeocoord py, gvgeocoord *x, gvgeocoord *y);
void gv_view_area_inverse_map_pointer(GvViewArea *view, gvgeocoord x, gvgeocoord y, gvgeocoord *px, gvgeocoord *py);
void gv_view_area_correct_for_transform(GvViewArea *view, gvgeocoord x, gvgeocoord y, gvgeocoord *cx, gvgeocoord *cy);

void gv_view_area_add_layer(GvViewArea *view, GtkObject *layer);
void gv_view_area_remove_layer(GvViewArea *view, GtkObject *layer);
GtkObject* gv_view_area_active_layer(GvViewArea *view);
void gv_view_area_set_active_layer(GvViewArea *view, GtkObject *layer);
GtkObject* gv_view_area_get_layer_of_type(GvViewArea *view, GtkType layer_type, gint read_only_ok);
GtkObject* gv_view_area_get_named_layer(GvViewArea *view, const char *name);
GList* gv_view_area_list_layers(GvViewArea *view);
GtkObject *gv_view_area_get_primary_raster(GvViewArea *view);
void gv_view_area_swap_layers(GvViewArea *view, gint layer_a, gint layer_b);
GdkPixmap* gv_view_area_create_thumbnail(GvViewArea *view, GtkObject *layer, gint width, gint height);

GPtrArray *gv_view_area_get_fontnames(GvViewArea *view);
gint gv_view_area_bmfont_load(GvViewArea *view, gchar *name);
GvBMFontInfo *gv_view_area_bmfont_get_info(GvViewArea *view, gint font);
void gv_view_area_bmfont_draw(GvViewArea *view, gint font, gvgeocoord x, gvgeocoord y, gchar *text, int force_simple);
void gv_view_area_set_background_color(GvViewArea *view, GvColor color);
void gv_view_area_get_background_color(GvViewArea *view, GvColor color);

void gv_view_area_set_adjustments (GvViewArea *view, GtkAdjustment *hadj, GtkAdjustment *vadj);

gint gv_view_area_set_projection(GvViewArea *view, const char *projection);
const char *gv_view_area_get_projection(GvViewArea *view);
gint gv_view_area_print_to_file(GvViewArea *view, int width, int height, const char * filename,
				const char * format, int is_rgb);
gint gv_view_area_print_postscript_to_file(GvViewArea *view, int width, int height,
					   float ulx, float uly, float lrx, float lry,
					   int is_rgb, const char * filename);

gint gv_view_area_render_postscript(GvViewArea *view, int width, int height, 
				    float ulx, float uly, float lrx, float lry,
				    int is_rgb,
				    gint (*cb_func)(void *, const char *), void * cb_data);
gint gv_view_area_render_to_func(GvViewArea *view,
				 gint width, gint height,
				 gint (*scanline_handler)( void *, void * ), void *cb_data);
void gv_view_area_page_setup();
gint gv_view_area_print_to_windriver(GvViewArea *view, int width, int height,
				     float ulx, float uly, float lrx, float lry,
				     int is_rgb);
void gv_view_area_zoompan_event(GvViewArea *view, GdkEventButton *event);

int gv_view_area_redraw_timeout(GvViewArea *view);
int gv_view_area_pending_idle_work(GvViewArea *view);

int gv_get_render_counter();

void gv_view_area_set_property(GvViewArea *data, const char *name, 
			       const char *value);
const char *gv_view_area_get_property(GvViewArea *data, const char *name);
GvProperties *gv_view_area_get_properties(GvViewArea *data);
void gv_view_area_queue_cursor_draw(GvViewArea *view, int next_valid, gvgeocoord x, gvgeocoord y);
gint gv_view_area_gl_begin(GvViewArea *view);
void gv_view_area_gl_end(GvViewArea *view);
gint gv_view_area_make_current(GvViewArea *view);
GdkGLContext *gv_view_area_get_gl_context(GvViewArea *view);
gint gv_view_area_begin(GvViewArea *view);
void gv_view_area_swap_buffers(GvViewArea *view);
const char *gv_view_area_format_point_query(GvViewArea *view,
                                            GvProperties *properties,
					    double geo_x, double geo_y);

#endif /* __GV_VIEW_AREA_H__ */



