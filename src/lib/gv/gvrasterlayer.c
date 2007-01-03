/******************************************************************************
 * $Id: gvrasterlayer.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Raster display layer (managed textures, redraw, etc)
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
 * $Log: gvrasterlayer.c,v $
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
 * Revision 1.86  2004/06/23 14:35:04  gmwalter
 * Added support for multi-band complex imagery.
 *
 * Revision 1.85  2004/02/20 10:40:36  andrey_kiselev
 * Use gv_raster_get_nodata() instead of GDALGetNoDataValue().
 *
 * Revision 1.84  2004/02/18 16:57:41  andrey_kiselev
 * Fixed setting min/max levels in gv_raster_layer_init().
 *
 * Revision 1.83  2003/09/11 20:00:31  gmwalter
 * Add ability to specify a preferred polynomial order for warping a raster,
 * and add "safe mode" (only used if ATLANTIS_BUILD is defined).
 *
 * Revision 1.82  2003/02/20 19:27:16  gmwalter
 * Updated link tool to include Diana's ghost cursor code, and added functions
 * to allow the cursor and link mechanism to use different gcps
 * than the display for georeferencing.  Updated raster properties
 * dialog for multi-band case.  Added some signals to layerdlg.py and
 * oeattedit.py to make it easier for tools to interact with them.
 * A few random bug fixes.
 *
 * Revision 1.81  2003/02/07 20:06:49  andrey_kiselev
 * Memory leaks fixed.
 *
 * Revision 1.80  2003/02/06 08:19:24  warmerda
 * Added support for force_load property on GvRasterLayer
 *
 * Revision 1.79  2003/01/24 16:54:52  warmerda
 * turn some g_warning()s into CPLDebug calls for CIETmap/Paul
 *
 * Revision 1.78  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.77  2002/10/29 05:43:20  warmerda
 * added some debugging logic in texture load code
 *
 * Revision 1.76  2002/04/12 14:40:36  gmwalter
 * Removed the gvmesh rescale function (not needed because of view area
 * rescaling).
 *
 * Revision 1.74  2002/03/20 19:19:00  warmerda
 * add support for exact_render flag on GvViewArea
 *
 * Revision 1.73  2002/03/07 02:31:56  warmerda
 * added default_height to add_height functions
 *
 * Revision 1.72  2001/12/10 18:48:19  warmerda
 * ifdef out disconnect from prototype data in teardown
 *
 * Revision 1.71  2001/11/29 15:53:42  warmerda
 * added autoscale_samples preference
 *
 * Revision 1.70  2001/11/28 19:23:04  warmerda
 * Added logic to keep track if the mesh is dirty (out of date), and to
 * refresh it before a redraw.  It is marked dirty when the prototype data
 * emits a geotransform-changed signal.
 *
 * Revision 1.69  2001/10/25 20:30:46  warmerda
 * added interp_mode preference to control default subpixel interp
 *
 * Revision 1.68  2001/10/17 16:23:51  warmerda
 * added support for composing complex lut and pct
 *
 * Revision 1.67  2001/10/16 18:50:29  warmerda
 * added autoscale and histogram functions
 *
 * Revision 1.66  2001/10/12 17:44:18  warmerda
 * avoid extra redraws when many raster layers displayed
 *
 * Revision 1.65  2001/09/25 20:04:16  warmerda
 * cleanup idle tasks on finalize
 *
 * Revision 1.64  2001/08/22 16:20:38  warmerda
 * use gv_mesh_reset_to_identity for setting to raw
 *
 * Revision 1.63  2001/08/08 02:57:55  warmerda
 * removed unused support for normals
 *
 * Revision 1.62  2001/07/24 21:21:45  warmerda
 * added EV style phase colormap
 *
 * Revision 1.61  2001/07/16 15:20:12  warmerda
 * default to magnitude for complex images, instead of phase/magnitude
 *
 * Revision 1.60  2001/07/03 14:26:05  warmerda
 * added set/get raw ability
 *
 * Revision 1.59  2001/01/30 19:34:29  warmerda
 * make gv_raster_layer_purge_all_textures() public
 *
 * Revision 1.58  2001/01/30 14:32:32  warmerda
 * No longer purges textures on a display-change.  Added
 * gv_raster_layer_purge_all_textures(), to clear textures on in cases where
 * this is necessary.  Now can change alpha with texture_mode_set() without
 * throwing away textures.
 *
 * Revision 1.57  2000/10/06 15:35:33  warmerda
 * set nodata value by default if present on GDALRasterBand
 *
 * Revision 1.56  2000/08/31 20:20:19  warmerda
 * always destroy old textures before resizing them in ...texture_load
 *
 * Revision 1.55  2000/08/25 20:12:47  warmerda
 * added preliminary nodata support
 *
 * Revision 1.54  2000/08/24 17:00:45  srawlin
 * fixed 3D LOD calculation to account for image flip
 *
 * Revision 1.53  2000/08/24 15:48:39  srawlin
 * Added flip in 3D mode tile list calculation
 *
 * Revision 1.52  2000/08/18 21:32:29  warmerda
 * Fixed bug where tiles would pile up on the missing_tex list over multiple
 * calls to the gv_raster_layer_draw() if there were no intermediate calls
 * to the idle handler.
 *
 * Introduced a hack into gv_raster_layer_texture_load() to call glGetError()
 * every now and then to try and work around a flaw in Xi Graphics GL drivers.
 *
 * Revision 1.51  2000/08/09 17:37:58  warmerda
 * disconnect view callback on teardown, clear sources on destroy
 *
 * Revision 1.50  2000/07/25 17:51:07  warmerda
 * change debug to use CPLDebug
 *
 * Revision 1.49  2000/07/25 14:19:49  warmerda
 * dequeue old draw tasks before setting new ones in draw
 *
 * Revision 1.48  2000/07/24 21:25:37  warmerda
 * always set fragment color, even if modulate off
 *
 * Revision 1.47  2000/07/18 15:32:53  warmerda
 * set upper bound on redraw time to 2.0 seconds
 *
 * Revision 1.46  2000/07/18 15:04:50  warmerda
 * tuning of idle handler texture logic
 *
 * Revision 1.45  2000/07/17 19:47:56  warmerda
 * try to wait 3*redraw time
 *
 * Revision 1.44  2000/07/17 19:31:50  warmerda
 * added tentative support for scaling redraw wait to actual redraw time
 *
 * Revision 1.43  2000/07/07 17:54:42  warmerda
 * Modified 3D tile-in-view selection to use corners of tiles, and to compute
 * the view cone based on a windows corners.
 *
 * Revision 1.42  2000/07/03 20:58:31  warmerda
 * eye_pos in georef coordinates now
 *
 * Revision 1.40  2000/06/29 14:38:37  warmerda
 * use GvManager for idle tasks
 *
 * Revision 1.39  2000/06/28 12:09:40  warmerda
 * initial fragment color to all white
 *
 * Revision 1.38  2000/06/27 21:25:41  warmerda
 * rewrote texture caching completely
 *
 * Revision 1.37  2000/06/23 12:56:18  warmerda
 * added multiple GvRasterSource support
 *
 * Revision 1.36  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include "gvrasterlayer.h"
#include "gvrastertypes.h"
#include "gvrasteraverage.h"
#include "gvrasterlut.h"
#include "gvmanager.h"
#include "ogr_srs_api.h"


#if !defined(GL_CLAMP_TO_EDGE) && defined(GL_CLAMP)
#  define GL_CLAMP_TO_EDGE GL_CLAMP
#endif


static void gv_raster_layer_class_init(GvRasterLayerClass *klass);
static void gv_raster_layer_init(GvRasterLayer *layer);
static void gv_raster_layer_draw(GvLayer *layer, GvViewArea *area);
static void gv_raster_layer_extents(GvLayer *layer, GvRect *rect);
static gint gv_raster_layer_texture_load( GvRasterLayer *layer, gint tile_num, gint lod );
static gint gv_raster_layer_idle_handler( gpointer data );
static void gv_raster_layer_setup( GvLayer *layer, GvViewArea *view );
static void gv_raster_layer_teardown( GvLayer *layer, GvViewArea *view );
static void gv_raster_layer_state_changed( GvViewArea *view, GvRasterLayer *layer );
static void gv_raster_layer_finalize( GObject *gobject );
static void gv_raster_layer_destroy( GtkObject *gtk_object );
static gint gv_raster_layer_texture_size( gint format, gint type );
static void gv_raster_layer_gl_disp_set( GvRasterLayer *layer );
static void gv_raster_layer_gl_disp_unset( void );
static gint gv_raster_layer_reproject( GvLayer *layer, 
                                       const char *new_projection );
static int gvrl_to_georef_cb( int pt_count, double *x, double *y, double *z, 
                              void *cb_data );
static int gvrl_to_raw_cb( int pt_count, double *x, double *y, double *z, 
                           void *cb_data );
static int
gv_mesh_transform_with_func(GvMesh *mesh, 
                            int (*trfunc)(int,double*,double*,double*,void *),
                            void *cb_data );
static void gv_raster_layer_cleanup_idle( GvRasterLayer *layer );

struct _GvRasterLayerIdleInfo {
    GvRasterLayer *layer;
    GvViewArea *area;
    int lod;
};

enum {
    GV_RASTER_LAYER_BLEND_OFF = 0,
    GV_RASTER_LAYER_BLEND_FILTER,
    GV_RASTER_LAYER_BLEND_ADD,
    GV_RASTER_LAYER_BLEND_MULTIPLY,
    GV_RASTER_LAYER_BLEND_CUSTOM
};

GtkType
gv_raster_layer_get_type(void)
{
    static GtkType raster_layer_type = 0;

    if (!raster_layer_type)
    {
	static const GtkTypeInfo area_layer_info =
	{
	    "GvRasterLayer",
	    sizeof(GvRasterLayer),
	    sizeof(GvRasterLayerClass),
	    (GtkClassInitFunc) gv_raster_layer_class_init,
	    (GtkObjectInitFunc) gv_raster_layer_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	raster_layer_type = gtk_type_unique(gv_layer_get_type(),
					    &area_layer_info);
    }
    return raster_layer_type;
}

static void
gv_raster_layer_init(GvRasterLayer *layer)
{
    int   i;

    layer->prototype_data = NULL;
    layer->source_count = 0;
    layer->mode = 0;
    layer->mesh = NULL;
    layer->pc_lut = NULL;

    for( i = 0; i < MAX_RASTER_SOURCE; i++ )
    {
        layer->source_list[i].lut = NULL;
        layer->source_list[i].lut_rgba_composed = NULL;
        layer->source_list[i].data = NULL;
        layer->source_list[i].const_value = 0;
        layer->source_list[i].min = 0.0;
        layer->source_list[i].max = 255.0;
        layer->source_list[i].nodata_active = FALSE;
        layer->source_list[i].nodata_real = -1e8;
        layer->source_list[i].nodata_imaginary = 0.0;
    }
}

static void
gv_raster_layer_class_init(GvRasterLayerClass *klass)
{
    GvLayerClass *layer_class;

    layer_class = (GvLayerClass*) klass;
    layer_class->draw = gv_raster_layer_draw;
    layer_class->extents_request = gv_raster_layer_extents;
    layer_class->setup = gv_raster_layer_setup;
    layer_class->teardown = gv_raster_layer_teardown;
    layer_class->reproject = gv_raster_layer_reproject;

    /* ---- Override finalize ---- */
    (G_OBJECT_CLASS(klass))->finalize = gv_raster_layer_finalize;

    ((GtkObjectClass *) klass)->destroy = gv_raster_layer_destroy;

    /* GTK2 PORT...
    ((GtkObjectClass *) klass)->finalize = gv_raster_layer_finalize;
    */


}

GtkObject *
gv_raster_layer_new(int mode, GvRaster *prototype_data, 
                    GvProperties creation_properties)
{
    GvRasterLayer *layer;
    double nodata_real=-1e8, nodata_imaginary=0.0;
    int nodata_active = FALSE;
    const char *interp_pref;

    if( prototype_data == NULL )
        return NULL;

    layer = GV_RASTER_LAYER(gtk_type_new(gv_raster_layer_get_type()));

    layer->tile_x = prototype_data->tile_x;
    layer->tile_y = prototype_data->tile_y;
    layer->prototype_data = prototype_data;
    layer->pc_lut = NULL;
    layer->pc_lut_composed = NULL;
    layer->pc_lut_rgba_complex = NULL;

    if( mode == GV_RLM_AUTO )
    {
        GDALColorInterp interp;

        interp = GDALGetRasterColorInterpretation( prototype_data->gdal_band );
        if( GDALDataTypeIsComplex( prototype_data->gdal_type ) )
            mode = GV_RLM_COMPLEX;
        else if( interp == GCI_RedBand 
                 || interp == GCI_GreenBand
                 || interp == GCI_BlueBand )
            mode = GV_RLM_RGBA;
	else if ((interp == GCI_HueBand) || (interp == GCI_PaletteIndex)) {
	    mode = GV_RLM_PSCI;
	}
        else
            mode = GV_RLM_SINGLE;
    }

    layer->mode = mode;

    if( gv_properties_get(&creation_properties,"mesh_lod") != NULL)
    {
        layer->mesh = gv_mesh_new_identity( prototype_data, 
                    atoi(gv_properties_get(&creation_properties,"mesh_lod")));
    } else {
        layer->mesh = gv_mesh_new_identity( prototype_data, 0 );
    }
    gtk_object_ref( GTK_OBJECT(layer->mesh) );
    gtk_object_sink( GTK_OBJECT(layer->mesh) );

    layer->mesh_is_raw = TRUE;
    layer->mesh_is_dirty = FALSE;
    gv_mesh_transform_with_func( layer->mesh, gvrl_to_raw_cb, 
                                 prototype_data );

    if( gv_properties_get(&creation_properties,"raw") == NULL )
    {
        gv_mesh_transform_with_func( layer->mesh, gvrl_to_georef_cb, 
                                     prototype_data );

        gv_data_set_projection( GV_DATA(layer), 
                           gv_data_get_projection(GV_DATA(prototype_data)) );

        layer->mesh_is_raw = FALSE;
    }

    memset( layer->source_list + 0, 0, 
            sizeof(GvRasterSource) * MAX_RASTER_SOURCE);

    /* Default GL parameters */
    layer->gl_info.blend_enable = 0;
    layer->gl_info.alpha_test = 0;
    
    layer->gl_info.tex_env_mode = GL_REPLACE;
    layer->gl_info.fragment_color[0] = 1.0;
    layer->gl_info.fragment_color[1] = 1.0;
    layer->gl_info.fragment_color[2] = 1.0;
    layer->gl_info.fragment_color[3] = 1.0;

    layer->gl_info.s_wrap = GL_CLAMP;
    layer->gl_info.t_wrap = GL_CLAMP;

    interp_pref = gv_properties_get(&creation_properties,"interp_mode");
    if( interp_pref == NULL )
        interp_pref = gv_manager_get_preference(gv_get_manager(),
                                                "interp_mode");

#ifdef ATLANTIS_BUILD
    /* Default to nearest for atlantis builds */
    if( interp_pref == NULL || strcmp(interp_pref,"linear") != 0 )
    {
        layer->gl_info.mag_filter = GL_NEAREST;
        layer->gl_info.min_filter = GL_NEAREST;
    }
    else
    {
        layer->gl_info.mag_filter = GL_LINEAR;
        layer->gl_info.min_filter = GL_LINEAR;
    }
#else
    if( interp_pref == NULL || strcmp(interp_pref,"nearest") != 0 )
    {
        layer->gl_info.mag_filter = GL_LINEAR;
        layer->gl_info.min_filter = GL_LINEAR;
    }
    else
    {
        layer->gl_info.mag_filter = GL_NEAREST;
        layer->gl_info.min_filter = GL_NEAREST;
    }
#endif
    /* Setup texture related information */
    layer->tile_list = g_array_new( FALSE, FALSE, sizeof( int ) ) ;
    layer->missing_tex = g_array_new( FALSE, FALSE, sizeof( int ) );
    
    /* Allocate texture structures */

    if( ( layer->textures = g_new0( GvRasterLayerTexObj *, 
                                    prototype_data->max_tiles ) ) == NULL )
    {
	return NULL;
    }

    /* FIXME ... should we always do this? */
    gv_data_set_parent(GV_DATA(layer), GV_DATA(prototype_data));

    /* check for nodata value */
    nodata_active = gv_raster_get_nodata( prototype_data, &nodata_real );

    /* Setup mode dependent information */
    switch( mode )
    {
      case GV_RLM_SINGLE:
        layer->source_count = 1;
        if( GDALGetRasterColorTable( prototype_data->gdal_band ) != NULL )
        {
            gv_raster_layer_set_source( layer, 0, prototype_data, 0, 255.0,
                                        0, NULL, nodata_active, nodata_real, 
                                        nodata_imaginary );
            gv_raster_layer_apply_gdal_color_table( layer, 
                    GDALGetRasterColorTable( prototype_data->gdal_band ) );
        }
        else
            gv_raster_layer_set_source( layer, 0, prototype_data, 
                                    prototype_data->min, prototype_data->max, 
                                    0, NULL, nodata_active, nodata_real, 
                                    nodata_imaginary );
        break;

      case GV_RLM_COMPLEX:
        layer->source_count = 1;
        layer->pc_lut = NULL;
        gv_raster_layer_lut_color_wheel_new_ev(layer, FALSE, TRUE );
        gv_raster_layer_set_source( layer, 0, prototype_data, 
                                    prototype_data->min, 
                                    prototype_data->max, 
                                    0, NULL, nodata_active, nodata_real, 
                                    nodata_imaginary );
        break;

      case GV_RLM_PSCI:
        layer->source_count = 2;
        gv_raster_layer_set_source( layer, 0, prototype_data,
				    prototype_data->min, prototype_data->max,
                                    0, NULL, nodata_active, nodata_real, 
                                    nodata_imaginary );
	gv_raster_layer_apply_gdal_color_table
	    (layer, GDALGetRasterColorTable( prototype_data->gdal_band ) );
        gv_raster_layer_set_source( layer, 1, prototype_data, 
                                    prototype_data->min, prototype_data->max, 
                                    0, NULL, nodata_active, nodata_real, 
                                    nodata_imaginary );

        break;

      case GV_RLM_RGBA:
        layer->source_count = 4;

        layer->pc_lut_rgba_complex = NULL;
        gv_raster_layer_lut_color_wheel_new_ev(layer, FALSE, TRUE );

        gv_raster_layer_set_source( layer, 0, prototype_data, 
                                    prototype_data->min, prototype_data->max, 
                                    0, NULL, nodata_active, nodata_real, 
                                    nodata_imaginary );
        gv_raster_layer_set_source( layer, 1, prototype_data, 
                                    prototype_data->min, prototype_data->max, 
                                    0, NULL, nodata_active, nodata_real, 
                                    nodata_imaginary );
        gv_raster_layer_set_source( layer, 2, prototype_data, 
                                    prototype_data->min, prototype_data->max, 
                                    0, NULL, nodata_active, nodata_real, 
                                    nodata_imaginary );
        gv_raster_layer_set_source( layer, 3, NULL, 0, 255, 
                                    255, NULL, FALSE, 0.0, 0.0 );
        break;

      default:
        g_warning( "unexpected raster layer mode" );
        return NULL;
    }

    return GTK_OBJECT(layer);
}

static void gv_raster_layer_destroy( GtkObject *gtk_object )

{
    GvLayerClass *parent_class;
    GvRasterLayer *rlayer = GV_RASTER_LAYER(gtk_object);
    int          isource;

    CPLDebug( "OpenEV", "gv_raster_layer_destroy(%s)\n", 
              gv_data_get_name(GV_DATA(rlayer)) );

    /* clear any "source" references */
    for( isource = 0; isource < rlayer->source_count; isource++ ) {
        gv_raster_layer_set_source( rlayer, isource, NULL, 0, 0, 0, NULL,
                                    FALSE, 0.0, 0.0 );
    }

    /* GTK2 PORT... test and nullify resources */
    if (rlayer->mesh != NULL) {
      gtk_object_unref( GTK_OBJECT(rlayer->mesh) );
      rlayer->mesh = NULL;
    }

    if ( rlayer->textures != NULL) {
      g_free( rlayer->textures );
      rlayer->textures = NULL;
    }

    /* No parent destroy.. */
}

/*
** Clean up any outstanding idle tasks against this raster layer.
*/

static void gv_raster_layer_cleanup_idle( GvRasterLayer *layer )

{
    GvIdleTask *task_list;

    /* Ensure that any existing idle task for this layer is blown away */
    for( task_list = gv_get_manager()->idle_tasks;
         task_list != NULL; task_list = task_list->next ) 
    {
        if( strcmp(task_list->name,"raster-layer-update") == 0 )
        {
            struct _GvRasterLayerIdleInfo *info;

            info = (struct _GvRasterLayerIdleInfo *) task_list->task_info;
	    if (info == NULL) continue;
            if( info->layer == layer )
            {
                g_assert( GV_LAYER(layer)->pending_idle );
                GV_LAYER(layer)->pending_idle = FALSE;

                gv_manager_dequeue_task( gv_get_manager(), task_list );
                g_free( info );
		// task_list->task_info = NULL;
                break;
            }
        }
    }

    g_assert( !GV_LAYER(layer)->pending_idle );
    GV_LAYER(layer)->pending_idle = FALSE;
}

static void gv_raster_layer_finalize( GObject *gobject )

{
    GvLayerClass *parent_class;
    GvRasterLayer *rlayer = GV_RASTER_LAYER(gobject);

    /* GTK2 PORT... Override GObject finalize, test for and set NULLs
       as finalize may be called more than once */

    CPLDebug( "OpenEV", "gv_raster_layer_finalize(%s)\n", 
              gv_data_get_name(GV_DATA(rlayer)) );

    if( rlayer->pc_lut != NULL )
    {
        g_free(rlayer->pc_lut);
        rlayer->pc_lut = NULL;
    }
    if( rlayer->pc_lut_composed != NULL )
    {
        g_free(rlayer->pc_lut_composed);
        rlayer->pc_lut_composed = NULL;
    }

    gv_raster_layer_cleanup_idle( rlayer );

    /* Call parent class function */
    parent_class = gtk_type_class(gv_layer_get_type());
    G_OBJECT_CLASS(parent_class)->finalize(gobject);

    /* Call parent class function
    parent_class = gtk_type_class(gv_layer_get_type());
    GTK_OBJECT_CLASS(parent_class)->finalize(gtk_object);
    */
}

void gv_raster_layer_purge_all_textures( GvRasterLayer *layer )

{
    gint texture; 

    for( texture = 0; 
         texture < layer->prototype_data->max_tiles; 
         texture++ )
    {
        if( layer->textures[texture] )
            gv_raster_layer_purge_texture( layer, texture );
    }
}

static void gv_raster_layer_raster_changed( GvRaster *raster,
                                           void * raw_change_info,
                                           GvRasterLayer *layer )
{
    GvRasterChangeInfo *change_info = (GvRasterChangeInfo *) raw_change_info;
    gint texture; 

    if( change_info != NULL 
        && (change_info->width > 0 && change_info->height > 0) )
    {
        for( texture = 0; 
             texture < layer->prototype_data->max_tiles; 
             texture++ )
        {
            gint  coords[4];

            gv_raster_tile_xy_get( raster, texture, 0, coords );

            if( change_info->x_off < coords[2] 
                && change_info->y_off < coords[3] 
                && change_info->x_off+change_info->width > coords[0] 
                && change_info->y_off+change_info->height > coords[1] )
            {
                if( layer->textures[texture] )
                    gv_raster_layer_purge_texture( layer, texture );
            }
        }
    }
    else
    {
        gv_raster_layer_purge_all_textures( layer );
    }
}
    
static void 
gv_raster_layer_raster_geotransform_changed( GvRaster *raster,
                                             int junk,
                                             GvRasterLayer *layer )
{
    if( layer->mesh_is_raw )
        return;

    if( layer->mesh_is_dirty )
        return;

    layer->mesh_is_dirty = TRUE;
    gv_raster_layer_purge_all_textures( layer );
    gv_view_area_queue_draw( GV_LAYER(layer)->view );
}
    
static void gv_raster_layer_setup( GvLayer *layer, GvViewArea *view )
{
    gtk_signal_connect( GTK_OBJECT(view), "view-state-changed",
			(GtkSignalFunc)gv_raster_layer_state_changed, layer );
    /* FIXME: we need this on all sources. */
    gtk_signal_connect( GTK_OBJECT(GV_RASTER_LAYER(layer)->prototype_data), 
                        "changed",
			(GtkSignalFunc)gv_raster_layer_raster_changed, layer );
    gtk_signal_connect( GTK_OBJECT(GV_RASTER_LAYER(layer)->prototype_data), 
                        "geotransform-changed",
			(GtkSignalFunc)gv_raster_layer_raster_geotransform_changed, layer );
}

static void gv_raster_layer_teardown( GvLayer *layer, GvViewArea *view )
{
    gv_raster_layer_cleanup_idle( GV_RASTER_LAYER(layer) );

    gtk_signal_disconnect_by_data( GTK_OBJECT(view), GTK_OBJECT(layer) );
#ifdef notdef
    gtk_signal_disconnect_by_data( 
        GTK_OBJECT(GV_RASTER_LAYER(layer)->prototype_data), 
        GTK_OBJECT(layer) );
#endif
}

static void gv_raster_layer_state_changed( GvViewArea *view, GvRasterLayer *layer )
{
    if( layer->tile_list )
	g_array_set_size( layer->tile_list, 0 );
}
    

static void
gv_raster_layer_draw( GvLayer *layer, GvViewArea *area )
{
    GvRasterLayer *raster_layer = GV_RASTER_LAYER(layer);
    gint *list = NULL,
	lod=0, i = 0, e;
    struct _GvRasterLayerIdleInfo *idle_info = NULL;
    double   pixel_size, pixel_zoom;
    int      tiles_to_force_load = 0;

    GvMeshTile *mesh = NULL;

    if(!gv_layer_is_visible(layer)) return;

    /* Ensure that any existing idle task for this layer is blown away */
    gv_raster_layer_cleanup_idle( raster_layer );

    g_array_set_size( raster_layer->missing_tex, 0 );

    /* Ensure the mesh is up to date */
    gv_raster_layer_refresh_mesh( raster_layer );

    /* Get the list of tiles to draw for current view */

    if( !raster_layer->tile_list->len )
    {
	raster_layer->tile_list =
            gv_mesh_tilelist_get( raster_layer->mesh, area, raster_layer,
                                  raster_layer->tile_list );
    }
    
    list = (gint *) raster_layer->tile_list->data;

    /* 2D - Mode Decide level of detail (LOD). Note: each tile gets same LOD */
    /*   see below for 3D mode LOD calculation */
    pixel_size = gv_raster_layer_pixel_size(raster_layer);
    pixel_zoom = log(pixel_size) / log(2);

    if ( !area->flag_3d )
    {
        /* 2D Mode */
        lod = (int) (-area->state.zoom - pixel_zoom);

        if( lod < 0 )
            lod = 0;

        if( lod >= raster_layer->prototype_data->max_lod )
            lod = raster_layer->prototype_data->max_lod-1;

    } else {
        /* 3D Case */

        /* Simple distance to Z axis - works pretty well, but not efficent for large images, need reduced LOD in distance */
        /* lod = (int) ((area->state.eye_pos[2]+250)/128.0 - pixel_zoom); */
    }

    /*
    ** Find if we are willing to force load any tiles during rendering in
    ** an attempt to reduce flicker. 
    */
    if( gv_data_get_property( GV_DATA(raster_layer), "force_load" ) != NULL )
        tiles_to_force_load = 
            atoi(gv_data_get_property( GV_DATA(raster_layer), "force_load" ));
#ifdef ATLANTIS_BUILD
    else if( (gv_manager_get_preference(gv_get_manager(),"safe_mode") != NULL) &&
             (strcmp(gv_manager_get_preference(gv_get_manager(),"safe_mode"),"on") == 0))
        tiles_to_force_load = 100;
#endif
    gv_raster_layer_gl_disp_set( raster_layer );

    for( i = 0; list[i] != -1; i++ )
    {
        int	process_when_idle;
        int     skip_render;
        double  tile_dist = 0.0;
        GvRasterLayerTexObj *tex;

        tex = raster_layer->textures[list[i]];

        /* 3D Mode - Calculate individula LODs for each tile based on 
           distance from centre of tile to view position */
        if ( area->flag_3d )
        {
            gint *tile_coords;
            double x_center, y_center, z_center;
            double temp_x, temp_y, temp_z, pixel_ratio;
            int   debug3d = 0;
            
            if( gv_manager_get_preference(gv_get_manager(),"DEBUG3D") != NULL )
                debug3d =
                  atoi(gv_manager_get_preference(gv_get_manager(),"DEBUG3D"));


            /* Get Corners to get LOD*/
            tile_coords = gv_mesh_get_tile_corner_coords( raster_layer->mesh, 
                                                          list[i]);
            x_center = (tile_coords[0]+tile_coords[2])/2.0;
            y_center = (tile_coords[1]+tile_coords[5])/2.0;
            z_center = 0.0;
            gv_raster_layer_pixel_to_view(raster_layer,
                                          &x_center,&y_center,&z_center);

            /* Calculate the optimal LOD for this tile */

            /*  ??? Are these distances correct???  They seem to work. */
            if( debug3d > 1 )
            {
                printf( "--- lod calc ---: tile_center=(%f,%f,%f)\n", 
                        x_center, y_center, z_center );
            }

            /* Must account for image flip - eye_pos is okay, just the tile */
            temp_x = area->state.eye_pos[0] - (x_center  * area->state.flip_x);
            temp_y = area->state.eye_pos[1] - (y_center * area->state.flip_y);
            temp_z = area->state.eye_pos[2] - z_center;
            tile_dist = sqrt(temp_x*temp_x + temp_y*temp_y + temp_z*temp_z);

            /* How many texture pixels are required to cover one screen pixel
             * We are assuming that fov is 90 degrees so that the distance
             * from center of view to top edge of view (shape_y*0.5) is the
             * same as the distance from eye_pos to center of view. 
             */

            pixel_ratio = (ABS(tile_dist) / pixel_size)
            				/ (area->state.shape_y*0.5);
            
            lod = (int) (log(pixel_ratio)/log(2));

            if( debug3d )
                printf( "tile_dist=%f, pixel_size=%f, ratio=%f, lod=%d\n", 
                        tile_dist, pixel_size, pixel_ratio, lod );

            free(tile_coords);

            if( lod < 0 )
                lod = 0;

            if( lod >= raster_layer->prototype_data->max_lod )
                lod = raster_layer->prototype_data->max_lod-1;
        }

        if( (tex == NULL || tex->lod != lod) && area->exact_render )
        {
            gv_raster_layer_texture_load( raster_layer, list[i], lod );
            tex = raster_layer->textures[list[i]];
            CPLAssert( tex->lod == lod );
        }

        if( tex == NULL && tiles_to_force_load > 0 )
        {
            CPLDebug( "OpenEV", "force loading tile for layer %s.", 
                      gv_data_get_name( GV_DATA(raster_layer) ) );

            gv_raster_layer_texture_load( raster_layer, list[i], lod );
            tex = raster_layer->textures[list[i]];
            tiles_to_force_load--;

            // Re-enable textures that are disabled by loader. 
            gv_raster_layer_gl_disp_set( raster_layer );
        }

        /* Get mesh for tile */
        if( tex != NULL )
            mesh = gv_mesh_get( raster_layer->mesh, list[i], tex->lod, 
                                lod, mesh );
        else
            mesh = NULL;

        if( tex == NULL || tex->tex_obj == 0 )
        {
            /* we have no texture at all. Skip rendering, and queue
               idle request to fetch data */
            process_when_idle = TRUE;
            skip_render = TRUE;
        } 
	else if( mesh == NULL )
	{
            /* this should never happen, right? */
            skip_render = TRUE;
            process_when_idle = FALSE;
	}

        else if( tex->lod != lod )
        {
            /* we have a texture, but it's not the optimal lod, render
               it, and queue fetching of the correct lod */

            process_when_idle = TRUE;
            skip_render = FALSE;
        }
        else
        {
            /* general case of having the desired texture available */

            process_when_idle = FALSE;
            skip_render = FALSE;
        }

        if( process_when_idle )
        {
            /* add this to missing texture list */

            g_array_append_val( raster_layer->missing_tex, list[i] );

            if( !idle_info )
            {
                if( (idle_info = g_new(struct _GvRasterLayerIdleInfo,1)) 
                    != NULL )
                {
                    idle_info->layer = raster_layer;
                    idle_info->area = area;
                    idle_info->lod = lod;
                    gv_manager_queue_task( gv_get_manager(), 
                                           "raster-layer-update", 10, 
                                           gv_raster_layer_idle_handler, 
                                           idle_info );
                    GV_LAYER(layer)->pending_idle = TRUE;
                }
            }
        }

        if( skip_render )
            continue;

        /* mark last used time, but only if it is the optimal resolution */
        if( !process_when_idle )
            gv_raster_layer_touch_texture( raster_layer, list[i] );

        /* bind texture to be displayed */
        glBindTexture( GL_TEXTURE_2D, tex->tex_obj );

        /* Draw mesh */
        if( mesh->tex_coords )
        {
            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer( 2, GL_FLOAT, 0, mesh->tex_coords );
        }

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, mesh->vertices );

        for( e = 0; e <= mesh->restarts; e++ )
        {
            glDrawElements( mesh->list_type, mesh->range, GL_UNSIGNED_INT, 
                            &(mesh->indices[e*mesh->range]) );
        }

        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_NORMAL_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	if ( mesh )
	{
	    g_free( mesh );
	    mesh = NULL;
	}
    }

    gv_raster_layer_gl_disp_unset();
}

/* Idle handler */

static gint
gv_raster_layer_idle_handler( gpointer data )
{
    struct _GvRasterLayerIdleInfo *idle_info = (struct _GvRasterLayerIdleInfo *) data;
    GvRasterLayer *layer = idle_info->layer;
    gint i, tile, temp_lod, lod;
    gboolean tile_loaded = FALSE;
    GvViewArea *view = GV_LAYER(layer)->view;

    g_assert( GV_LAYER(layer)->pending_idle );
    GV_LAYER(layer)->pending_idle = FALSE;

    if( !GTK_WIDGET_REALIZED( GTK_WIDGET(view) )
        || !(gv_view_area_make_current(view))
        || layer->missing_tex->len == 0 )
    {
        g_free( idle_info );
        return FALSE;
    }

    tile_loaded = FALSE;
    lod = idle_info->lod;

    /* first scan for any completely missing textures */
    for( i = 0; i < layer->missing_tex->len; i++ )
    {
	tile = g_array_index( layer->missing_tex, int, i );

        /* try to fill in textures where we have no lod at all */
	if( layer->textures[tile] == NULL )
	{
            /* FIXME: Need to check all sources? */
	    if( ( temp_lod = gv_raster_cache_get_best_lod( 
                			layer->prototype_data->cache,
                                        tile, lod ) ) == -1 )
                temp_lod = lod;

            /* force downsamping if higher resolution available, to avoid
             tieing up too much texture memory */
            if( temp_lod < lod )
                temp_lod = lod;

            gv_raster_layer_texture_load( layer, tile, temp_lod );

            g_array_index( layer->missing_tex, int, i ) = -1;

            /* if too long has elapsed, break out to give the user a chance
               to provide input */
            if( gv_view_area_redraw_timeout(view) )
                break;
	}
    }

    /* make another pass processing anything that needs updating. */
    for( i = 0; 
         i < layer->missing_tex->len && !gv_view_area_redraw_timeout(view);
         i++ )
    {
	tile = g_array_index( layer->missing_tex, int, i );
        if( tile < 0 )
            continue;

        gv_raster_layer_texture_load( layer, tile, lod );
    }

    g_array_set_size( layer->missing_tex, 0 );

    /*
     * We only requeue a draw if we ran out of time, or if we know there
     * are no other layers with pending work they would like to do before 
     * a redraw.
     */
    if( gv_view_area_redraw_timeout(view) 
        || !gv_view_area_pending_idle_work(view) )
        gv_view_area_queue_draw( view);

    g_free( idle_info );
    return FALSE;
}


/* Load the named texture (tile_num / lod) into a bound texture 
   (via glBindTextures/glTexImage2D) */

static gint
gv_raster_layer_texture_load( GvRasterLayer *layer, gint tile_num, gint lod )
{
    gint size;
    void *buffer;
    int format, type, needs_free;
    static int counter = 0;

    /* give up with an error if we fail to get the cache tile */
    buffer = gv_raster_layer_gltile_get(layer,tile_num,lod, &format, &type,
                                        &needs_free );
    if( buffer == NULL )
        return 1;

    size = gv_raster_layer_texture_size( format, type );
    size *= ( layer->tile_x >> lod ) * ( layer->tile_y >> lod );

    glEnable( GL_TEXTURE_2D );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                     layer->gl_info.mag_filter );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                     layer->gl_info.min_filter );

#ifdef notdef
    if( layer->textures[tile_num] == NULL )
    {
        GLuint tex_obj;

        /* Never been allocated -- here we go */
        glGenTextures( 1, &tex_obj );

        gv_raster_layer_create_texture( layer, tile_num, tex_obj, lod, size );
    }
    else
    {
        gv_raster_layer_reset_texture( layer, tile_num, lod, size );
    }
#else
    {
        GLuint   tex_obj;
        
        if( layer->textures[tile_num] != NULL )
            gv_raster_layer_purge_texture( layer, tile_num );

        /* Never been allocated -- here we go */
        glGenTextures( 1, &tex_obj );

        gv_raster_layer_create_texture( layer, tile_num, tex_obj, lod, size );
    }
#endif
    
    if( layer->textures[tile_num] == NULL )
    {
        CPLDebug( "OpenEV", 
                  "gv_raster_layer_texture_load(): "
                   "unexpectedly missing texture" );
        return 1;
    }

    glBindTexture( GL_TEXTURE_2D, layer->textures[tile_num]->tex_obj );

    /* 
     * We seem to see frequent crashes with Xi Graphics GL drivers if
     * we do too many glTexImage calls without resyncronizing with the
     * server.  The following hack is intended to introduce a round trip
     * with the server "every now and then" while adding minimal wait
     * overhead.
     */
    if( counter++ % 100 == 0 && glGetError() != 0 )
    {
        CPLDebug( "OpenEV", 
                  "Got GL Error in gv_raster_layer_texture_load()" );
    }

    glTexImage2D( GL_TEXTURE_2D, 0, format,
                  layer->tile_x >> lod,
                  layer->tile_y >> lod,
                  0, format, type, buffer );

#ifdef notdef
    if( layer->tile_x >> lod <= 16 )
    {
        int iX, iY, max = layer->tile_x >> lod;

        printf(" format = %d, type = %d\n", format, type );
        for( iY = 0; iY < max; iY++ )
        {
            for( iX = 0; iX < max; iX++ )
                printf( "%02X%02X ", 
                        ((unsigned char *) buffer)[(iX + iY*max)*2],
                        ((unsigned char *) buffer)[(iX + iY*max)*2+1] );
            printf( "\n" );
        }
    }
#endif

    glDisable( GL_TEXTURE_2D );

    if( needs_free )
        g_free( buffer );
        
    return 0;
}

static gint gv_raster_layer_texture_size( gint format, gint type )
{
    gint elem_size;
    gint n_elems;

    switch( type   )
    {
	case GL_UNSIGNED_BYTE:
	    elem_size = sizeof( unsigned char );
	    break;
	default:
	    elem_size = 1;
	    fprintf( stderr, "Unknown type in gv_raster_layer_texture_size\n" );
	    break;
    }

    switch( format )
    {
	case GL_RGBA:
	    n_elems = 4;
	    break;
	case GL_RGB:
	    n_elems = 3;
	    break;
	case GL_LUMINANCE:
	    n_elems = 1;
	    break;
	case GL_LUMINANCE_ALPHA:
	    n_elems = 2;
	    break;
	default:
	    n_elems = 1;
	    fprintf( stderr, "Unknown format in gv_raster_layer_texture_size\n" );
	    break;
    }

    return n_elems * elem_size;
}

static void gv_raster_layer_gl_disp_set( GvRasterLayer *layer )
{
    glEnable( GL_TEXTURE_2D );

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, layer->gl_info.tex_env_mode );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, layer->gl_info.s_wrap );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, layer->gl_info.t_wrap );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, layer->gl_info.mag_filter );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, layer->gl_info.min_filter );

    if( layer->gl_info.tex_env_mode == GL_MODULATE )
        glColor4fv( layer->gl_info.fragment_color );
    else
        glColor4f( 1.0, 1.0, 1.0, 1.0 );

    if( layer->gl_info.blend_enable )
    {
	glEnable( GL_BLEND );

	glBlendFunc( layer->gl_info.blend_src, layer->gl_info.blend_dst );
    }

    if( layer->gl_info.alpha_test )
    {
	glEnable( GL_ALPHA_TEST );

	glAlphaFunc( layer->gl_info.alpha_test_mode,
		     layer->gl_info.alpha_test_val );
    }

}

static void gv_raster_layer_gl_disp_unset( void )
{
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
}

static void
gv_raster_layer_extents(GvLayer *layer, GvRect *rect)
{
    gv_mesh_extents( GV_RASTER_LAYER(layer)->mesh, rect );
}


long
gv_raster_layer_texture_clamp_set( GvRasterLayer *layer, int s_clamp, int t_clamp )
{
    int modes[] = { GL_CLAMP, GL_REPEAT };
    int max_modes = 2;

    g_return_val_if_fail( layer != NULL, 1 );

    if( s_clamp >= max_modes || t_clamp >= max_modes )
	return 1;

    layer->gl_info.s_wrap = modes[s_clamp];
    layer->gl_info.t_wrap = modes[t_clamp];

    gv_raster_layer_purge_all_textures( layer );
    gv_layer_display_change( GV_LAYER(layer), NULL );

    return 0;
}

long
gv_raster_layer_zoom_set( GvRasterLayer *layer, int mag_mode, int min_mode )
{
    int modes[] = { GL_LINEAR, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR };
    int max_mag_modes = 2;
    int max_min_modes = 3;

    if( mag_mode >= max_mag_modes || min_mode >= max_min_modes )
	return 1;

    layer->gl_info.mag_filter = modes[mag_mode];
    layer->gl_info.min_filter = modes[min_mode];

    gv_raster_layer_purge_all_textures( layer );
    gv_layer_display_change( GV_LAYER(layer), NULL );

    return 0;
}

long
gv_raster_layer_zoom_get( GvRasterLayer *layer, int *mag_mode, int *min_mode )
{
    int modes[] = { GL_LINEAR, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR };
    int max_mag_modes = 2;
    int max_min_modes = 3;
    int i;

    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 1 );

    *mag_mode = -1;
    *min_mode = -1;

    for( i = 0; i < max_mag_modes; i++ )
	if( layer->gl_info.mag_filter == modes[i] )
	    *mag_mode = i;

    for( i = 0; i < max_min_modes; i++ )
	if( layer->gl_info.min_filter == modes[i] )
	    *min_mode = i;

    if( *mag_mode == -1 || *min_mode == -1 )
	return 1;

    return 0;
}

long
gv_raster_layer_texture_mode_set( GvRasterLayer *layer, int texture_mode, GvColor color )
{
    int modes[] = { GL_REPLACE, GL_MODULATE };
    int max_modes = 2;

    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 1 );
    g_return_val_if_fail( texture_mode < max_modes, 1 );

    if( modes[texture_mode] != layer->gl_info.tex_env_mode
        || color[0] != layer->gl_info.fragment_color[0]
        || color[1] != layer->gl_info.fragment_color[1]
        || color[2] != layer->gl_info.fragment_color[2]
        || color[3] != layer->gl_info.fragment_color[3] )
    {
        layer->gl_info.tex_env_mode = modes[texture_mode];
        memcpy( layer->gl_info.fragment_color, color, sizeof( GvColor ) );
        gv_layer_display_change( GV_LAYER(layer), NULL );
    }

    return 0;
}

long
gv_raster_layer_texture_mode_get( GvRasterLayer *layer, int *texture_mode, GvColor *color )
{
    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 1 );
    g_return_val_if_fail( texture_mode != NULL, 1 );
    g_return_val_if_fail( color != NULL, 1 );

    switch( layer->gl_info.tex_env_mode )
    {
	case GL_REPLACE:
	    *texture_mode = 0;
	    break;
	case GL_MODULATE:
	    *texture_mode = 1;
	    break;
	default:
	    return 1;
    }

    memcpy( color, layer->gl_info.fragment_color, sizeof( GvColor ) );

    return 0;
}

long
gv_raster_layer_alpha_set( GvRasterLayer *layer, int alpha_mode, float alpha_check_val )
{
    int modes[] = { 0, GL_NEVER, GL_ALWAYS, GL_LESS, GL_LEQUAL, GL_EQUAL, GL_GEQUAL, GL_GREATER, GL_NOTEQUAL };
    int max_modes = 8;

    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( alpha_mode < max_modes, 1 );

    if( alpha_mode )
    {
	layer->gl_info.alpha_test = 1;

	layer->gl_info.alpha_test_mode = modes[alpha_mode];

	layer->gl_info.alpha_test_val = alpha_check_val;
    } else {
	layer->gl_info.alpha_test = 0;
    }

    gv_layer_display_change( GV_LAYER(layer), NULL );

    return 0;
}

long
gv_raster_layer_alpha_get( GvRasterLayer *layer, int *alpha_mode, float *alpha_check_val )
{
    g_return_val_if_fail( layer != NULL, 1 );
    g_return_val_if_fail( GV_IS_RASTER_LAYER( layer ), 1 );
    g_return_val_if_fail( alpha_mode != NULL, 1 );
    g_return_val_if_fail( alpha_check_val != NULL, 1 );

    if( layer->gl_info.alpha_test )
    {
	switch( layer->gl_info.alpha_test_mode )
	{
	    case GL_NEVER:
		*alpha_mode = 1;
		break;
	    case GL_ALWAYS:
		*alpha_mode = 2;
		break;
	    case GL_LESS:
		*alpha_mode = 3;
		break;
	    case GL_LEQUAL:
		*alpha_mode = 4;
		break;
	    case GL_EQUAL:
		*alpha_mode = 5;
		break;
	    case GL_GEQUAL:
		*alpha_mode = 6;
		break;
	    case GL_GREATER:
		*alpha_mode = 7;
		break;
	    case GL_NOTEQUAL:
		*alpha_mode = 8;
		break;
	    default:
		return 1;
	}
    } else {
	*alpha_mode = 0;
    }

    *alpha_check_val = layer->gl_info.alpha_test_val;

    return 0;
}

long
gv_raster_layer_blend_mode_set( GvRasterLayer *layer, int blend_mode, int sfactor, int dfactor )
{
    int factors[] = { GL_ZERO, GL_ONE, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };
    int max_factors = 7; /* This must match the entries in the factors[] list */
    int max_modes = 5;
    int new_blend_src, new_blend_dst, new_blend_enable;

    g_return_val_if_fail( blend_mode < max_modes, 1 );

    g_return_val_if_fail( ( blend_mode != GV_RASTER_LAYER_BLEND_MODE_CUSTOM ) 
                 || ( sfactor < max_factors && dfactor < max_factors ), 1 );

    switch( blend_mode )
    {
      case GV_RASTER_LAYER_BLEND_MODE_FILTER:
        new_blend_src = GL_SRC_ALPHA;
        new_blend_dst = GL_ONE_MINUS_SRC_ALPHA;
        new_blend_enable = 1;
        break;
      case GV_RASTER_LAYER_BLEND_MODE_MULTIPLY:
        new_blend_src = GL_DST_COLOR;
        new_blend_dst = GL_ZERO;
        new_blend_enable = 1;
        break;
      case GV_RASTER_LAYER_BLEND_MODE_ADD:
        new_blend_src = GL_ONE;
        new_blend_dst = GL_ONE;
        new_blend_enable = 1;
        break;
      case GV_RASTER_LAYER_BLEND_MODE_CUSTOM:
        new_blend_src = factors[sfactor];
        new_blend_dst = factors[dfactor];
        new_blend_enable = 1;
        break;
      default:
        new_blend_enable = 0;
        new_blend_src = layer->gl_info.blend_src;
        new_blend_dst = layer->gl_info.blend_dst;
        break;
    }

    if( layer->gl_info.blend_enable != new_blend_enable
        || layer->gl_info.blend_src != new_blend_src
        || layer->gl_info.blend_dst != new_blend_dst )
    {
        layer->gl_info.blend_src = new_blend_src;
        layer->gl_info.blend_dst = new_blend_dst;
        layer->gl_info.blend_enable = new_blend_enable;
        gv_layer_display_change( GV_LAYER(layer), NULL );
    }

    return 0;
}

long
gv_raster_layer_blend_mode_get( GvRasterLayer *layer, int *blend_mode, int *sfactor, int *dfactor )
{
    int factors[] = { GL_ZERO, GL_ONE, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };
    int max_factors = 7;
    int i;

    if( layer->gl_info.blend_enable )
    {
	*blend_mode = 4;

	for( i = 0; i < max_factors; i++ )
	{
	    if( factors[i] == layer->gl_info.blend_src )
	    {
		*sfactor = i;
		break;
	    }
      
	    if( factors[i] == layer->gl_info.blend_dst )
	    {
		*dfactor = i;
	    }
	}
    } else {
	*blend_mode = 0;
	*sfactor = 0;
	*dfactor = 0;
    }
    return 0;
}

int gv_raster_layer_mode_get( GvRasterLayer *layer )

{
    return layer->mode;
}

		
/* the following function should move to gvmesh.h/c when Paul is done with
   them. */

static int
gv_mesh_transform_with_func(GvMesh *mesh, 
                            int (*trfunc)(int,double*,double*,double*,void *),
                            void *cb_data )
{
    int    tile;

    for( tile = 0; tile < mesh->max_tiles; tile++ )
    {
        GArray *verts;
        float  *xyz_verts;
        int    xyz_offset;

	verts = g_array_index( mesh->vertices, GArray *, tile );
        
        xyz_verts = (float *) verts->data;
        for( xyz_offset = 0; xyz_offset < verts->len; xyz_offset += 3 )
        {
            double    x_out, y_out, z_out;

            x_out = xyz_verts[0];
            y_out = xyz_verts[1];
            z_out = xyz_verts[2];

            if( !trfunc( 1, &x_out, &y_out, &z_out, cb_data ) )
                return FALSE;

            xyz_verts[0] = x_out;
            xyz_verts[1] = y_out;
            xyz_verts[2] = z_out;

            xyz_verts += 3;
        }
    }

    return TRUE;
}

static int gvrl_reproject_cb( int pt_count, double *x, double *y, double *z, 
                              void *cb_data )

{
    OGRCoordinateTransformationH *ct = (OGRCoordinateTransformationH) cb_data;

    return OCTTransform( ct, pt_count, x, y, z );
}

static gint 
gv_raster_layer_reproject( GvLayer *layer, 
                           const char * new_projection )

{
    int success = TRUE;
    OGRSpatialReferenceH   hSRSNew = NULL, hSRSOld = NULL;
    OGRCoordinateTransformationH hTransform = NULL;

    /* 
     * Try and establish if we can, or need to do reprojection.
     */
    if( gv_data_get_projection(GV_DATA(layer)) == NULL )
        return FALSE;

    if( EQUAL(gv_data_get_projection(GV_DATA(layer)),"PIXEL") )
        return FALSE;

    hSRSNew = OSRNewSpatialReference( new_projection );
    if( hSRSNew == NULL )
        return FALSE;

    hSRSOld = OSRNewSpatialReference(gv_data_get_projection(GV_DATA(layer)));
    if( hSRSOld == NULL )
    {
        OSRDestroySpatialReference( hSRSNew );
        return FALSE;
    }

    if( OSRIsSame( hSRSOld, hSRSNew ) )
    {
        OSRDestroySpatialReference( hSRSOld );
        OSRDestroySpatialReference( hSRSNew ); 
       
        return TRUE;
    }

    /*
     * Establish transformation.
     */
    
    hTransform = OCTNewCoordinateTransformation( hSRSOld, hSRSNew );
    if( hTransform == NULL )
    {
        OSRDestroySpatialReference( hSRSOld );
        OSRDestroySpatialReference( hSRSNew ); 
        
        return FALSE;
    }

    /*
     * Transform all the mesh points.
     */

    success = gv_mesh_transform_with_func( GV_RASTER_LAYER(layer)->mesh, 
                                           gvrl_reproject_cb,
                                           (void *) hTransform );

    OCTDestroyCoordinateTransformation( hTransform );
    OSRDestroySpatialReference( hSRSOld );
    OSRDestroySpatialReference( hSRSNew ); 

    if( success )
        gv_data_set_projection( GV_DATA(layer), new_projection );

    return success;
}

static int 
gvrl_to_georef_cb( int pt_count, double *x, double *y, double *z, 
                   void *cb_data )

{
    GvRaster *raster = GV_RASTER(cb_data);
    int      i, success = TRUE;

    for( i = 0; i < pt_count; i++ )
    {
        success |= gv_raster_pixel_to_georef( raster, x+i, y+i, z+i );
    }

    return success;
}

static int 
gvrl_to_raw_cb( int pt_count, double *x, double *y, double *z, 
                void *cb_data )

{
    GvRaster *raster = GV_RASTER(cb_data);
    int      i, success = TRUE;

    for( i = 0; i < pt_count; i++ )
    {
        y[i] = raster->height - y[i];
    }

    return success;
}

void gv_raster_layer_refresh_mesh( GvRasterLayer *layer )

{
    GvRaster	*prototype_data = layer->prototype_data;

    if( !layer->mesh_is_dirty )
        return;

    /* Should we actually reset the mesh to raw? */
    if( layer->mesh_is_raw )
        return;

    gv_mesh_reset_to_identity( layer->mesh );
    
    gv_mesh_transform_with_func( layer->mesh, gvrl_to_georef_cb, 
                                 prototype_data );
    layer->mesh_is_dirty = FALSE;
}

int gv_raster_layer_set_raw( GvRasterLayer *layer, int raw_enable )

{
    GvRaster	*prototype_data = layer->prototype_data;

    if( !raw_enable == !layer->mesh_is_raw )
        return TRUE;

    if( !raw_enable )
    {
        gv_mesh_transform_with_func( layer->mesh, gvrl_to_georef_cb, 
                                     prototype_data );

        gv_data_set_projection( GV_DATA(layer), 
                           gv_data_get_projection(GV_DATA(prototype_data)) );

        layer->mesh_is_raw = FALSE;
    }
    else 
    {
        /* we can't rawify ungeoreferenced layers */
        if( gv_data_get_projection( GV_DATA(layer) ) != NULL
            && EQUAL(gv_data_get_projection( GV_DATA(layer) ),"PIXEL") )
            return FALSE;

        gv_mesh_reset_to_identity( layer->mesh );

        gv_data_set_projection( GV_DATA(layer), NULL );

        layer->mesh_is_raw = TRUE;
    }

    return TRUE;
}

gint gv_raster_layer_pixel_to_view(GvRasterLayer *layer, 
                                   double *x, double *y, double *z )

{
    int      success = TRUE;

    if( !layer->mesh_is_raw )
    {
        success = gv_raster_pixel_to_georef( layer->prototype_data, x, y, z );
    }

    if( success
        && gv_data_get_projection(GV_DATA(layer->prototype_data)) != NULL
        && gv_data_get_projection(GV_DATA(layer)) != NULL
        && !EQUAL(gv_data_get_projection(GV_DATA(layer->prototype_data)),
                  gv_data_get_projection(GV_DATA(layer))) )
    {
        g_warning( "gv_raster_pixel_to_view doesn't reproject yet." );
    }

    return success;
}

gint gv_raster_layer_view_to_pixel(GvRasterLayer *layer, 
                                   double *x, double *y, double *z )

{
    int      success = TRUE;

    if( success
        && gv_data_get_projection(GV_DATA(layer->prototype_data)) != NULL
        && gv_data_get_projection(GV_DATA(layer)) != NULL
        && !EQUAL(gv_data_get_projection(GV_DATA(layer->prototype_data)),
                  gv_data_get_projection(GV_DATA(layer))) )
    {
        g_warning( "gv_raster_pixel_to_view doesn't reproject yet." );
    }

    if( success && !layer->mesh_is_raw )
    {
        success = gv_raster_georef_to_pixel( layer->prototype_data, x, y, z );
    }

    return success;
}

double gv_raster_layer_pixel_size( GvRasterLayer *raster )

{
    double	x1, y1, x2, y2;

    x1 = raster->prototype_data->width / 2; 
    y1 = raster->prototype_data->height / 2;

    x2 = x1 + 1.0;
    y2 = y1 + 1.0;
    
    gv_raster_layer_pixel_to_view( raster, &x1, &y1, NULL );
    gv_raster_layer_pixel_to_view( raster, &x2, &y2, NULL );

    return (ABS(x2-x1) + ABS(y2-y1)) * 0.5;
}

int gv_raster_layer_get_mode( GvRasterLayer *layer )

{
    return layer->mode;
}

void gv_raster_layer_add_height( GvRasterLayer *layer, GvRaster *height_raster,
                                 double default_height )
{
    gv_mesh_add_height(layer->mesh, height_raster, default_height );
}

void gv_raster_layer_clamp_height( GvRasterLayer *layer, int bclamp_min, int bclamp_max,
                                 double min_height, double max_height )
{
    gv_mesh_clamp_height(layer->mesh, bclamp_min, bclamp_max, min_height, max_height );
}

#define GV_MESH_RIGHT_X_BIT 1
#define GV_MESH_LEFT_X_BIT  2
#define GV_MESH_TOP_Y_BIT   4
#define GV_MESH_BOT_Y_BIT   8

#define DEG2RAD         0.01745329252
#define RAD2DEG         57.2986885

static GArray *
gv_mesh_tilelist_get_2d( GvMesh *mesh, GvViewArea *view, 
                         GvRasterLayer *rlayer, GArray *tilelist )
{
    float cos_ang, sin_ang;
    float term1, term2, term3, term4, term5, term6;
    gint i, e, j, dimensions, mask;
    float x, y; /* Just to make it compile */
    GArray *verts;

    cos_ang = cos( view->state.rot * DEG2RAD );
    sin_ang = sin( view->state.rot * DEG2RAD );

    term1 = (2/view->state.shape_x) *
        view->state.linear_zoom *
        view->state.flip_x *
        cos_ang;

    term2 = (2/view->state.shape_x) *
        -view->state.linear_zoom *
        view->state.flip_y *
        sin_ang;

    term3 = (2/view->state.shape_x) *
        view->state.linear_zoom *
        ( ( view->state.flip_x *
            view->state.tx *
            cos_ang ) -
          ( view->state.flip_y *
            view->state.ty *
            sin_ang ) );

    term4 = (2/view->state.shape_y) *
        view->state.linear_zoom *
        view->state.flip_x *
        sin_ang;

    term5 = (2/view->state.shape_y) *
        view->state.linear_zoom *
        view->state.flip_y *
        cos_ang;

    term6 = (2/view->state.shape_y) *
        view->state.linear_zoom *
        ( ( view->state.flip_x *
            view->state.tx *
            sin_ang ) +
          ( view->state.flip_y *
            view->state.ty *
            cos_ang ) );

    dimensions = ( 1 << mesh->detail );

    for( i = 0; i < mesh->max_tiles; i++ )
    {
        if( ( verts = g_array_index( mesh->vertices, GArray *, i ) ) != NULL )
        {
            mask = 0xFF;

            for( e = 0; e <= (dimensions+1); e += (dimensions+1) )
            {
                for( j = 0; j <= dimensions; j += dimensions )
                {
                    gint mask_point = 0;
                    gfloat old_x, old_y;

                    old_x = g_array_index( verts, float, 3*(e*dimensions+j) );
                    old_y = g_array_index( verts, float, 3*(e*dimensions+j)+1 );

                    x = ( term1*old_x ) + ( term2*old_y ) + term3;
                    y = ( term4*old_x ) + ( term5*old_y ) + term6;

                    if( x > 1.0 )
                        mask_point |= GV_MESH_RIGHT_X_BIT;

                    if( x < -1.0 )
                        mask_point |= GV_MESH_LEFT_X_BIT;

                    if( y > 1.0 )
                        mask_point |= GV_MESH_TOP_Y_BIT;

                    if( y < -1.0 )
                        mask_point |= GV_MESH_BOT_Y_BIT;

                    mask &= mask_point;

                }
            }

            if( !mask )
            {
                g_array_append_val( tilelist, i );
            }

        }
    }

    return tilelist;
}

static GArray *
gv_mesh_tilelist_get_3d( GvMesh *mesh, GvViewArea *view, 
                         GvRasterLayer *rlayer, GArray *tilelist )
{
    float tile_vect[3];  /* xyz */
    float tile_magnitude, eye_dir_magnitude, cos_angle;
    float eye_pos[3], eye_dir[3];
    float diag, cos_diag;
    int   debug3d = 0;
    int   tiles_across = rlayer->prototype_data->tiles_across;
    int   tiles_down = rlayer->prototype_data->tiles_down;
    int   *cornerInside, tile_i, tile_j;

    if( gv_manager_get_preference(gv_get_manager(),"DEBUG3D") != NULL )
        debug3d = atoi(gv_manager_get_preference(gv_get_manager(),"DEBUG3D"));

    eye_pos[0] = view->state.eye_pos[0];
    eye_pos[1] = view->state.eye_pos[1];
    eye_pos[2] = view->state.eye_pos[2];

    eye_dir[0] = view->state.eye_dir[0];
    eye_dir[1] = view->state.eye_dir[1];
    eye_dir[2] = view->state.eye_dir[2];

    eye_dir_magnitude = sqrt( (eye_dir[0]) * (eye_dir[0]) + 
                              (eye_dir[1]) * (eye_dir[1]) + 
                              (eye_dir[2]) * (eye_dir[2]));
    /*
     * Compute the diagonal of the window to the y axis, and from this
     * the cos() of the angle between the eye direction and the vector to
     * the corner of the window. 
     */
    diag = sqrt(1.0 + (view->state.shape_x*view->state.shape_x)
                /(view->state.shape_y*view->state.shape_y));

    cos_diag = 1.0 / sqrt(1+diag*diag);
    
    if( debug3d )
    {
        printf( "diagonal angle = %.1f\n", 
                acos(cos_diag) * RAD2DEG );
    }    
    
    /* Allocate array of flags for each tile corner. */
    cornerInside = (int *) g_new(int,(tiles_across+1)*(tiles_down+1));

    /*
     * Loop over each tile corner, and work out if it is inside the cone
     * or not. 
     */ 

    for( tile_j = 0; tile_j < tiles_down+1; tile_j++ )
    {
        int     tile_y;

        tile_y = tile_j * rlayer->prototype_data->tile_y;
        if( tile_y > rlayer->prototype_data->height )
            tile_y = rlayer->prototype_data->height;

        for( tile_i = 0; tile_i < tiles_across+1; tile_i++ )
        {
            int     tile_x;
            double  geo_x, geo_y, geo_z;

            tile_x = tile_i * rlayer->prototype_data->tile_x;
            if( tile_x > rlayer->prototype_data->width )
                tile_x = rlayer->prototype_data->width;

            geo_x = tile_x * view->state.flip_x;
            geo_y = tile_y * view->state.flip_y;
            geo_z = 0.0;
                
            gv_raster_layer_pixel_to_view(rlayer, &geo_x, &geo_y, &geo_z );
            
            tile_vect[0] = geo_x - eye_pos[0];
            tile_vect[1] = geo_y - eye_pos[1];
            tile_vect[2] = geo_z - eye_pos[2];

            tile_magnitude = sqrt(tile_vect[0]*tile_vect[0] +
                                  tile_vect[1]*tile_vect[1] +
                                  tile_vect[2]*tile_vect[2]);

            /* Dot product of tile vector with eye_direction to find 
               cos of angle*/
            cos_angle = (tile_vect[0] * (eye_dir[0]) +
                         tile_vect[1] * (eye_dir[1]) +
                         tile_vect[2] * (eye_dir[2]) ) 
                / (tile_magnitude * eye_dir_magnitude);

            cornerInside[tile_j * (tiles_across+1) + tile_i] =
                (cos_angle >= cos_diag);

            if( debug3d > 1 )
            {
                printf("tile (%d,%d) z %f  angle=%f ", 
                       tile_i, tile_j, 
                       eye_pos[2], RAD2DEG * acos(cos_angle));

                if( cornerInside[tile_j*(tiles_across+1)+tile_i] )
                {
                    printf("\n");
                } else {
                    printf("not\n");
                }
            }
            if( debug3d > 2)
            {
                printf("tile(%f,%f,%f) tile_mag %f eye_mag %f\n", 
                       tile_vect[0], tile_vect[1], tile_vect[2], 
                       tile_magnitude, eye_dir_magnitude);
            }
        }
    }

    /* 
     * Scan through all the files, and for each one establish whether
     * any of the corners are in the view cone.  If so, add the tile
     * to the list of files to be drawn.
     */
    for( tile_j = 0; tile_j < tiles_down; tile_j++ )
    {
        for( tile_i = 0; tile_i < tiles_across; tile_i++ )
        {
            int     tile = tile_i + tile_j * tiles_across;
            int     corner = tile_i + tile_j * (tiles_across+1);

            if( cornerInside[corner]
                || cornerInside[corner+1]
                || cornerInside[corner+tiles_across+1]
                || cornerInside[corner+tiles_across+2] )
            {
                g_array_append_val( tilelist, tile );  
            }
        }
    }

    g_free( cornerInside );

    return tilelist;
}

GArray *
gv_mesh_tilelist_get( GvMesh *mesh, GvViewArea *view, 
                      GvRasterLayer *rlayer, GArray *tilelist )
{
    gint i;

    if( !view->flag_3d)
        gv_mesh_tilelist_get_2d( mesh, view, rlayer, tilelist );
    else
        gv_mesh_tilelist_get_3d( mesh, view, rlayer, tilelist );
        
    i = -1;
    g_array_append_val( tilelist, i );

    return tilelist;
}

#define GROW_RANGE(x,xoff,xsize) { \
  if( x < 0 ) x = 0; \
  if( x < xoff ) { xsize += xoff - x; xoff = x; } \
  else if( x > xoff+xsize ) { xsize = x - xoff; } }

static void
gv_raster_layer_view_extents( GvRasterLayer *rlayer, 
                              int *xoff, int *yoff, int *xsize, int *ysize )

{
    GvRaster *raster;
    GvViewArea   *view;

    raster = rlayer->prototype_data;
    view = GV_LAYER(rlayer)->view;
    
    /*
    ** Compute a bounding rectangle.  In 3D we will just use all tiles
    ** identified in the tile list, but for 2D we try to restrict things
    ** more closely.
    */
    if( view->flag_3d )
    {
        *xoff = *yoff = 0;
        *xsize = raster->width;
        *ysize = raster->height;
    }
    else
    {
        gvgeocoord x, y;
        double  dx, dy;
        
        *xsize = 1;
        *ysize = 1;

        /* upper left */
        gv_view_area_map_pointer( view, 0.0, 0.0, &x, &y );
        dx = x; dy = y;
        if( gv_raster_layer_view_to_pixel( rlayer, &dx, &dy, NULL ) )
        {
            *xoff = MAX(0,MIN(raster->width-1,(int) dx));
            *yoff = MAX(0,MIN(raster->height-1,(int) dy));
        }
        else
        {
            *xoff = 0;
            *yoff = 0;
        }

        /* upper right */
        gv_view_area_map_pointer( view, view->state.shape_x, 0.0, 
                                  &x, &y );
        dx = x; dy = y;
        if( gv_raster_layer_view_to_pixel( rlayer, &dx, &dy, NULL ) )
        {
            GROW_RANGE(((int) dx), *xoff, *xsize );
            GROW_RANGE(((int) dy), *yoff, *ysize );
        }

        /* lower right */
        gv_view_area_map_pointer( view, 
                                  view->state.shape_x, 
                                  view->state.shape_y, 
                                  &x, &y );
        dx = x; dy = y;
        if( gv_raster_layer_view_to_pixel( rlayer, &dx, &dy, NULL ) )
        {
            GROW_RANGE(((int) dx), *xoff, *xsize );
            GROW_RANGE(((int) dy), *yoff, *ysize );
        }

        /* lower left */
        gv_view_area_map_pointer( view, 0.0, view->state.shape_y, 
                                       &x, &y );
        dx = x; dy = y;
        if( gv_raster_layer_view_to_pixel( rlayer, &dx, &dy, NULL ) )
        {
            GROW_RANGE(((int) dx), *xoff, *xsize );
            GROW_RANGE(((int) dy), *yoff, *ysize );
        }

        if( *xoff + *xsize > raster->width )
            *xsize = raster->width - *xoff;
        if( *yoff + *ysize > raster->height )
            *ysize = raster->height - *yoff;
    }
}

gint 
gv_raster_layer_autoscale_view( GvRasterLayer *rlayer, int isrc,
                                GvAutoScaleAlg alg, double alg_param, 
                                double *out_min, double *out_max )

{
    int	sample_count, ret_val;
    float *samples;
    GvRaster *raster;
    GvViewArea   *view;
    GArray  *tile_list;
    int xoff, yoff, xsize, ysize;

    if( isrc < 0 || isrc >= rlayer->source_count )
        return FALSE;

    raster = rlayer->source_list[isrc].data;
    if( raster == NULL )
        return FALSE;

    view = GV_LAYER(rlayer)->view;
    
    /*
    ** Build the tile list for the view.  Even works in 3D!
    */
    tile_list = g_array_new( FALSE, FALSE, sizeof(int) );

    tile_list = gv_mesh_tilelist_get( rlayer->mesh, view, rlayer, tile_list );

    /*
    ** Compute a bounding rectangle.  In 3D we will just use all tiles
    ** identified in the tile list, but for 2D we try to restrict things
    ** more closely.
    */
    gv_raster_layer_view_extents( rlayer, &xoff, &yoff, &xsize, &ysize );

    /*
    ** Collect random samples to computing scaling min/max.
    */
    if( gv_manager_get_preference(gv_get_manager(),"autoscale_samples") )
    {
        sample_count = atoi(
            gv_manager_get_preference(gv_get_manager(),"autoscale_samples"));
        sample_count = MAX(10,sample_count);
    }
    else
        sample_count = 10000;

    samples = (float *) g_new(float,sample_count);

    sample_count = 
        gv_raster_collect_random_sample(
            raster, 10000, samples, 
            xoff, yoff, xsize, ysize, tile_list );

    /*
    ** Perform the autoscaling.
    */
    ret_val = 
        gv_raster_autoscale( raster, alg, alg_param, sample_count, samples, 
                             out_min, out_max );

    g_free( samples );

    g_array_free( tile_list, TRUE );

    return ret_val;
}

gint gv_raster_layer_histogram_view( GvRasterLayer *rlayer, int isrc,
                                     double scale_min, double scale_max,
                                     int include_out_of_range,
                                     int bucket_count, int *histogram )

{
    GvRaster *raster;
    GvViewArea   *view;
    GArray  *tile_list;
    int xoff, yoff, xsize, ysize;

    if( isrc < 0 || isrc >= rlayer->source_count )
        return 0;

    raster = rlayer->source_list[isrc].data;
    if( raster == NULL )
        return 0;

    view = GV_LAYER(rlayer)->view;
    
    /*
    ** Build the tile list for the view.  Even works in 3D!
    */
    tile_list = g_array_new( FALSE, FALSE, sizeof(int) );

    tile_list = gv_mesh_tilelist_get( rlayer->mesh, view, rlayer, tile_list );

    /*
    ** Compute a bounding rectangle.  In 3D we will just use all tiles
    ** identified in the tile list, but for 2D we try to restrict things
    ** more closely.
    */
    gv_raster_layer_view_extents( rlayer, &xoff, &yoff, &xsize, &ysize );

    /*
    ** Perform the histogram.
    */
    
    return gv_raster_collect_histogram( raster, scale_min, scale_max, 
                                        bucket_count, histogram, 
                                        include_out_of_range,
                                        xoff, yoff, xsize, ysize, tile_list );
}
