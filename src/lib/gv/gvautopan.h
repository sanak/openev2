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
 * $Log: gvautopan.h,v $
 * Revision 1.1.2.6  2005/09/12 15:32:13  gmwalter
 * Update autopan tool for line paths.
 *
 * Revision 1.1.2.5  2005/01/29 04:01:11  gmwalter
 * Initial support for autopan trail.
 *
 * Revision 1.1.2.4  2005/01/20 15:20:08  gmwalter
 * Add ability to change standard path.
 *
 * Revision 1.1.2.3  2005/01/05 21:22:23  gmwalter
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

#ifndef __GV_AUTOPAN_TOOL_H__
#define __GV_AUTOPAN_TOOL_H__

#include "gvtypes.h"
#include "gvtool.h"
#include "gvrasterlayer.h"
#include "gvshapes.h"
#include <GL/gl.h>
#include <GL/glu.h>

#define GV_TYPE_AUTOPAN_TOOL            (gv_autopan_tool_get_type ())
#define GV_AUTOPAN_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_AUTOPAN_TOOL, GvAutopanTool))
#define GV_AUTOPAN_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_AUTOPAN_TOOL, GvAutopanToolClass))
#define GV_IS_AUTOPAN_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_AUTOPAN_TOOL))
#define GV_IS_AUTOPAN_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_AUTOPAN_TOOL))

typedef struct _GvAutopanTool       GvAutopanTool;
typedef struct _GvAutopanToolClass  GvAutopanToolClass;
typedef struct _GvAutopanViewItem      GvAutopanViewItem;
typedef struct _GvAutopanTrailTile      GvAutopanTrailTile;



enum
{
    TL_R_D_L_D = 0, /* Start top left, go right, down, left, down, right... */
    BL_R_U_L_U,     /* Start bottom left, go right, up, left, up, right... */
    TR_L_D_R_D,
    BR_L_U_R_U,
    TL_R_D_R_D,    /* Start top left, go right, jump down and to left edge, go right... */
    /* TL_D_R_D_R,     Start top left, go down, jump right and to top edge, go down... */
    STANDARD_PATHS
};

#define LINES_PATH -1

struct _GvAutopanTool
{
    GvTool tool;

    guint quit_handler_id;

    /* play_flag: 0- not playing, 1- playing, 2- paused */
    gint play_flag;

    /* Route to follow in panning: -1 for non-standard, 0...STANDARD_PATHS-1
     * for preset path coverage. non-standard is not implemented yet.
     *
     * standard paths cover the tool's current extents completely in a
     * regular fashion.
     *
     * non-standard will allow the user to provide a set of points that
     * define a path to follow: the locations will be interpolated from
     * this path (the number of locations between points will depend on
     * the current speed settings).
     */

    gint path_type;

    GvShapes *nonstandard_path_shapes;

    /* Track changes that user makes to window size and zoom level,
     * and reset accordingly.
     */

    gvgeocoord win_width, win_height, win_zoom;

    /* Default settings:
     * - pan_region: extents (can't just use tool boundaries
     *   because locations have to be recalculated when extents are
     *   reset).  Only used for standard paths.
     * - overlap (perpendicular to path direction, as a fraction
     *   of the extents in that direction)
     * - speed (panning speed- appropriate values will be machine
     *   dependent- set negative to go in reverse direction)
     */

    GvRect pan_region;
    gvgeocoord overlap; /* only used for standard paths */
    gvgeocoord speed;


    /* Block size mode: user has different options in how block
     * size is set.
     *
     * block_size_mode = 0:
     *     - user sets block_x_size to a float between 0 and 1,
     *       corresponding to the block size as a fraction of the
     *       total x extents (block_y_size will be determined by
     *       block_x_size and the window's aspect ratio).  resolution
     *       is ignored in this mode.
     *
     * block_size_mode = 1:
     *     - same as mode 0, except that block_x_size is in view
     *       coordinates rather than a fraction of total x extents
     *       to be panned.
     *
     * block_size_mode = 2:
     *     - block size is set so that the size of pixels in the
     *       view will be constant at "resolution".
     */

    gint block_size_mode;

    gvgeocoord block_x_size;

    gvgeocoord resolution;

    gint current_index;

    /* list of sequential translates for panning */
    GArray *centers;
    gint num_centers;

    /* trail info */
    /* Predicted region that trail will cover: can go outside; just  */
    /* used in resolution calculations.                              */
    /* Overview_width_pixels should correspond to maximum resoluiont */
    /* required by all overview windows.                             */
    GvRect trail_overview_region;
    gint trail_overview_width_pixels;

    gint trail_tile_pixels;
    gint trail_tile_lines;

    /* Next four parameters are calculated based on overview info */
    gvgeocoord trail_x0;
    gvgeocoord trail_y0;
    gvgeocoord trail_tile_xsize;
    gvgeocoord trail_tile_ysize;

    GArray *trail;
    gint num_trail_tiles;
    gint trail_mode; /* Number of views showing a trail */
    GLuint *trail_block; /* Used to selectively replace parts of a texture */



    /* optional secondary views in which to draw a box
       indicating current location.
       view- gvviewarea
       can_resize- whether or not the current panning
                        resolution can be reset by dragging
                        a corner of the box in the view.
       can_relocate- whether or not the current position
                        can be reset by selecting a boundary
                        of the box and dragging the box in
                        the view.
       trail_mode- whether or not the view should display a
                   trail where user has already panned.
     */
    GArray *view_items;

    gint num_views;

};


struct _GvAutopanTrailTile
{
    gint xindex;
    gint yindex;
    gint pixels;
    gint lines;

    gvgeocoord x0;
    gvgeocoord y0;
    gvgeocoord xf;
    gvgeocoord yf;

    unsigned char *mask;
};

struct _GvAutopanViewItem
{
    GvViewArea *view;
    GvAutopanTool *tool;

    gint can_resize;
    gint can_reposition;
    gint trail_mode;

    gint banding;
    gint translating;
    gint play_flag;
    GvColor trail_color;
    GArray *trail_textures;

    /* Store connections for later disconnect */
    gint press_id;
    gint release_id;
    gint motion_id;
};

struct _GvAutopanToolClass
{
    GvToolClass parent_class;

    void (* zoomextents_changed)(GvAutopanTool *tool);
    void (* zoomextents_changing)(GvAutopanTool *tool);
};


GType gv_autopan_tool_get_type(void);
GvTool* gv_autopan_tool_new(void);

gint gv_autopan_tool_play(GvAutopanTool *tool);
gint gv_autopan_tool_pause(GvAutopanTool *tool);
gint gv_autopan_tool_stop(GvAutopanTool *tool);

double gv_autopan_tool_set_speed(GvAutopanTool *tool, gvgeocoord speed);
double gv_autopan_tool_get_speed(GvAutopanTool *tool);

gint gv_autopan_tool_new_rect(GvAutopanTool *tool, GvRect *rect);
gint gv_autopan_tool_get_rect(GvAutopanTool *tool, GvRect *rect);

gint gv_autopan_tool_set_location(GvAutopanTool *tool, gvgeocoord x,
                                  gvgeocoord y, gvgeocoord z);
gint gv_autopan_tool_get_location(GvAutopanTool *tool, gvgeocoord *x,
                                  gvgeocoord *y, gvgeocoord *z);

gint gv_autopan_tool_set_overlap(GvAutopanTool *tool, gvgeocoord overlap);
double gv_autopan_tool_get_overlap(GvAutopanTool *tool);

gint gv_autopan_tool_set_block_x_size(GvAutopanTool *tool,
                                      gvgeocoord block_x_size,
                                      gint mode);
gint gv_autopan_tool_set_x_resolution(GvAutopanTool *tool,
                                    gvgeocoord resolution);

gint gv_autopan_tool_set_standard_path(GvAutopanTool *tool, gint path_type);

void gv_autopan_tool_get_state(GvAutopanTool *tool,
                               gint *play_flag,
                               gint *path_type,
                               gint *block_size_mode,
                               gvgeocoord *block_x_size,
                               gvgeocoord *x_resolution,
                               gint *num_views);

void gv_autopan_tool_clear_trail(GvAutopanTool *tool);

gint gv_autopan_tool_set_trail_color(GvAutopanTool *tool, GvViewArea *view,
                                     float red, float green, float blue,
                                     float alpha);

gint gv_autopan_tool_set_trail_mode( GvAutopanTool *tool, GvViewArea *view,
                                     gint trail_mode);

gint gv_autopan_tool_register_view(GvAutopanTool *tool, GvViewArea *view,
                                   gint can_resize, gint can_reposition,
                                   gint trail_mode);

gint gv_autopan_tool_remove_view(GvAutopanTool *tool, GvViewArea *view);

gint gv_autopan_tool_set_lines_path(GvAutopanTool *tool, GvShapes *lines);

gint gv_autopan_tool_set_trail_parameters(GvAutopanTool *tool,
                                          GvRect *overview_extents,
                                          int overview_width_pixels);

void gv_autopan_tool_get_trail_parameters(GvAutopanTool *tool,
                                          GvRect *overview_extents,
                                          int *overview_width_pixels,
                                          int *num_trail_tiles);

gint gv_autopan_tool_save_trail_tiles(GvAutopanTool *tool,
                                      const char *basename);

gint gv_autopan_tool_load_trail_tiles(GvAutopanTool *tool,
                                      const char *basename,
                                      int num_trail_tiles);



#endif /* __GV_AUTOPAN_TOOL_H__ */
