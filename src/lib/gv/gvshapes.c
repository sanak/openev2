/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Vector shape container class.
 * Author:   Frank Warmerdam, warmerda@home.com
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

#include "gextra.h"
#include "gvshapes.h"
#include "gvraster.h"
#include "cpl_error.h"

typedef struct _GvShapesMemento GvShapesMemento;

struct _GvShapesMemento
{
    GvDataMemento base;
    GArray *ids;
    GPtrArray *shapes;
};

static void gv_shapes_class_init(GvShapesClass *klass);
static void gv_shapes_init(GvShapes *points);
static void gv_shapes_get_memento(GvData *points, gpointer info, 
                                  GvDataMemento **memento);
static void gv_shapes_set_memento(GvData *points, GvDataMemento *memento);
static void gv_shapes_del_memento(GvData *points, GvDataMemento *memento);
static void gv_shapes_changed(GvData *points, gpointer data);
static void gv_shapes_finalize(GObject *gobject);

static GvDataClass *parent_class = NULL;

GType
gv_shapes_get_type(void)
{
    static GType shapes_type = 0;

    if (!shapes_type) {
        static const GTypeInfo shapes_info =
        {
            sizeof(GvShapesClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_shapes_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvShapes),
            0,
            (GInstanceInitFunc) gv_shapes_init,
        };
        shapes_type = g_type_register_static (GV_TYPE_DATA,
                                            "GvShapes",
                                            &shapes_info, 0);
        }

    return shapes_type;
}

static void
gv_shapes_init(GvShapes *shapes)
{
    shapes->shapes = g_ptr_array_new();
    shapes->extents_valid = FALSE;
    shapes->actual_num_shapes = 0;
}

static void
gv_shapes_class_init(GvShapesClass *klass)
{
    GvDataClass *data_class = GV_DATA_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    object_class->finalize = gv_shapes_finalize;

    data_class->changed = gv_shapes_changed;
    data_class->get_memento = gv_shapes_get_memento;
    data_class->set_memento = gv_shapes_set_memento;
    data_class->del_memento = gv_shapes_del_memento;
}

GvData *
gv_shapes_new(void)
{
    GvShapes *shapes = g_object_new(GV_TYPE_SHAPES, NULL);

    return GV_DATA(shapes);
}

gint
gv_shapes_add_shape(GvShapes *shapes, GvShape *new_shape)

{
    int  shape_id;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 1, NULL};

    /* Identify where to put it, reuse old holes if available */
    if( shapes->shapes->len != shapes->actual_num_shapes )
    {
        for( shape_id=0; shape_id < shapes->shapes->len; shape_id++ )
        {
            if( g_ptr_array_index(shapes->shapes, shape_id) == NULL )
                break;
        }
    }
    else
    {
        shape_id = shapes->shapes->len;
    }

    /* Make notification of impending change */
    change_info.shape_id = &shape_id;
    gv_data_changing(GV_DATA(shapes), &change_info);

    /* apply update */
    if( shape_id == shapes->shapes->len )
        g_ptr_array_add(shapes->shapes, new_shape );
    else
        g_ptr_array_index(shapes->shapes, shape_id) = new_shape;

    gv_shape_ref( new_shape );
    shapes->actual_num_shapes++;

    /* notify of completed change */
    gv_data_changed(GV_DATA(shapes), &change_info);

    return shape_id;
}

/* same as gv_shape_add_shape but do not fill the holes, always append shape at the end*/
gint
gv_shapes_add_shape_last(GvShapes *shapes, GvShape *new_shape)

{
    int  shape_id;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 1, NULL};


    shape_id = shapes->shapes->len;

    /* Make notification of impending change */
    change_info.shape_id = &shape_id;
    gv_data_changing(GV_DATA(shapes), &change_info);

    /* apply update */
    if( shape_id == shapes->shapes->len )
        g_ptr_array_add(shapes->shapes, new_shape );
    else
        g_ptr_array_index(shapes->shapes, shape_id) = new_shape;

    gv_shape_ref( new_shape );
    shapes->actual_num_shapes++;

    /* notify of completed change */
    gv_data_changed(GV_DATA(shapes), &change_info);

    return shape_id;
}

void
gv_shapes_delete_shapes(GvShapes *shapes, gint num_shapes, gint *id_list)
{
    GvShapeChangeInfo change_info = {GV_CHANGE_DELETE, 0, NULL};
    GvShape  *shape;
    gint     i;

    change_info.num_shapes = num_shapes;
    change_info.shape_id = id_list;

    gv_data_changing(GV_DATA(shapes), &change_info);

    for( i = 0; i < num_shapes; i++ )
    {
        if( id_list[i] < 0 || id_list[i] >= shapes->shapes->len )
            shape = NULL;
        else
            shape = g_ptr_array_index(shapes->shapes, id_list[i]);

        if( shape != NULL )
        {
            g_ptr_array_index(shapes->shapes,id_list[i]) = NULL;
            gv_shape_unref(shape);
            shapes->actual_num_shapes--;
        }
    }

    /* Boil NULLs off the end of the list */
    while( shapes->shapes->len > 0 
           && g_ptr_array_index(shapes->shapes, 
                                shapes->shapes->len-1) == NULL )
    {
        g_ptr_array_remove_index_fast( shapes->shapes, 
                                       shapes->shapes->len-1 );
    }

    gv_data_changed(GV_DATA(shapes), &change_info);
}

void
gv_shapes_translate_shapes(GvShapes *shapes, gint num_shapes, gint *id_list,
                           gvgeocoord dx, gvgeocoord dy)
{
    GvShape *shape;
    gint i;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};

    change_info.num_shapes = num_shapes;
    change_info.shape_id = id_list;

    gv_data_changing(GV_DATA(shapes), &change_info);

    for (i=0; i < num_shapes; ++i)
    {
        int    ring;

        shape = gv_shapes_get_shape(shapes,id_list[i]);
        if( shape == NULL )
            continue;

        for( ring = gv_shape_get_rings(shape)-1; ring >= 0; ring-- )
        {
            int    node;

            for( node = gv_shape_get_nodes(shape,ring)-1; 
                 node >= 0; node-- )
            {
                gv_shape_set_xyz( shape, ring, node, 
                                  gv_shape_get_x(shape, ring, node) + dx,
                                  gv_shape_get_y(shape, ring, node) + dy,
                                  gv_shape_get_z(shape, ring, node) );
            }
        }
    }
    gv_data_changed(GV_DATA(shapes), &change_info);
}

void
gv_shapes_get_extents(GvShapes *shapes, GvRect *rect)
{
    if (!shapes->extents_valid)
    {
        gint i, num_shapes, valid_shapes = 0;
        GvVertex vmax, vmin;

        vmin.x = vmin.y = GV_MAXFLOAT;
        vmax.x = vmax.y = -GV_MAXFLOAT;

        num_shapes = gv_shapes_num_shapes(shapes);
        for (i=0; i < num_shapes; ++i)
        {
            GvRect   rect;
            GvShape *shape = gv_shapes_get_shape(shapes,i);

            if( shape == NULL )
                continue;

            gv_shape_get_extents( shape, &rect );

            if( rect.x != 0 || rect.y != 0 
                || rect.width != 0 || rect.height != 0 )
            {
                valid_shapes++;
                vmin.x = MIN(vmin.x,rect.x);
                vmax.x = MAX(vmax.x,rect.x+rect.width);
                vmin.y = MIN(vmin.y,rect.y);
                vmax.y = MAX(vmax.y,rect.y+rect.height);
            }
        }

        if (valid_shapes == 0)
        {
            shapes->extents.x = 0;
            shapes->extents.y = 0;
            shapes->extents.width = 0;
            shapes->extents.height = 0;
        }
        else
        {
            shapes->extents.x = vmin.x;
            shapes->extents.y = vmin.y;
            shapes->extents.width = vmax.x - vmin.x;
            shapes->extents.height = vmax.y - vmin.y;
        }
        shapes->extents_valid = TRUE;
    }

    *rect = shapes->extents;
}

void
gv_shapes_replace_shapes(GvShapes *shapes, gint num_shapes, gint *shape_id,
                         GvShape **shps, int make_copy)
{
    gint i;
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};

    change_info.num_shapes = num_shapes;
    change_info.shape_id = shape_id;

    gv_data_changing(GV_DATA(shapes), &change_info);

    for (i=0; i < num_shapes; ++i)
    {
        GvShape *shape;

        if( shape_id[i] < 0 || shape_id[i] >= shapes->shapes->len )
            continue;
        else if( gv_shapes_get_shape(shapes, shape_id[i]) != NULL )
            gv_shape_unref( gv_shapes_get_shape(shapes, shape_id[i]) );
        else
            g_warning( "Missing shape in gv_shapes_replace_shapes()" );

        if( make_copy )
            shape = gv_shape_copy(shps[i]);
        else
            shape = shps[i];

        gv_shape_ref( shape );
        g_ptr_array_index(shapes->shapes, shape_id[i]) = shape;
    }

    gv_data_changed(GV_DATA(shapes), &change_info);
}

static void
gv_shapes_insert_shapes(GvShapes *shapes, gint num_shapes, gint *shape_ids,
                        GvShape **shps)
{
    gint i;
    GvShapeChangeInfo change_info = {GV_CHANGE_ADD, 0, NULL};

    change_info.num_shapes = num_shapes;
    change_info.shape_id = shape_ids;

    gv_data_changing(GV_DATA(shapes), &change_info);

    for (i=0; i < num_shapes; ++i)
    {
        int id = shape_ids[i];

        if( id >= shapes->shapes->len )
        {
            int  old_length = shapes->shapes->len;

            g_ptr_array_set_size( shapes->shapes, id+1 );
            while( old_length < id )
            {
                g_ptr_array_index(shapes->shapes, old_length) = NULL;
                old_length++;
            }

            gv_shape_ref( shps[i] );
            g_ptr_array_index( shapes->shapes, id ) = shps[i];

            shapes->actual_num_shapes++;
        }
        else if( g_ptr_array_index( shapes->shapes, id ) == NULL )
        {
            gv_shape_ref( shps[i] );
            g_ptr_array_index( shapes->shapes, id ) = shps[i];
            shapes->actual_num_shapes++;
        }
        else
        {
            g_warning( "gv_shapes_insert_shapes(): target shape_id not NULL!");
        }
    }

    gv_data_changed(GV_DATA(shapes), &change_info);
}

static void
gv_shapes_get_memento(GvData *gv_data, gpointer data,
                      GvDataMemento **memento)
{
    GvShapes    *shapes = GV_SHAPES(gv_data);
    GvShapesMemento *mem;
    GvShapeChangeInfo *info = (GvShapeChangeInfo *) data;
    int i;

    mem = g_new(GvShapesMemento, 1);
    mem->base.data = GV_DATA(shapes);
    mem->base.type = info->change_type;

    mem->ids = g_array_new(FALSE, FALSE, sizeof(gint));
    g_array_append_vals(mem->ids, info->shape_id, info->num_shapes);

    /* Grab in ascending order */
    if (info->num_shapes > 1)
    {
        g_sort_type(mem->ids->data, gint, mem->ids->len);
    }

    if (info->change_type == GV_CHANGE_ADD)
    {
        mem->shapes = NULL;
    }
    else
    {
        mem->shapes = g_ptr_array_new();
        for (i=0; i < info->num_shapes; ++i)
        {
            GvShape    *shape = gv_shapes_get_shape(shapes,info->shape_id[i]);

            shape = gv_shape_copy( shape );
            gv_shape_ref( shape );
            g_ptr_array_add(mem->shapes, shape );
        }
    }

    *memento = (GvDataMemento*)mem;
}

static void
gv_shapes_set_memento(GvData *gv_data, GvDataMemento *data_memento)
{
    GvShapes    *shapes = GV_SHAPES(gv_data);
    GvShapesMemento *memento = (GvShapesMemento *) data_memento;

    switch (memento->base.type)
    {
        case GV_CHANGE_ADD:
            gv_shapes_delete_shapes(shapes, memento->ids->len,
                                    (gint*)memento->ids->data);
            break;

        case GV_CHANGE_REPLACE:
            gv_shapes_replace_shapes(shapes, memento->ids->len,
                                     (gint*)memento->ids->data,
                                     (GvShape **)memento->shapes->pdata,
                                     TRUE);
            break;

        case GV_CHANGE_DELETE:
            gv_shapes_insert_shapes(shapes, memento->ids->len,
                                     (gint*)memento->ids->data,
                                     (GvShape **)memento->shapes->pdata);
            break;
    }

    gv_shapes_del_memento((GvData *) shapes, (GvDataMemento *) memento);
}

static void
gv_shapes_del_memento(GvData *gv_data, GvDataMemento *data_memento)
{
    GvShapesMemento *memento = (GvShapesMemento *) data_memento;

    if (memento->shapes)
    {
        int  i;

        for (i=0; i < memento->shapes->len; ++i)
        {
            gv_shape_unref(g_ptr_array_index(memento->shapes, i));
        }
        g_ptr_array_free(memento->shapes, TRUE);

    }
    g_array_free(memento->ids, TRUE);
    g_free(memento);
}

static void
gv_shapes_changed(GvData *gv_data, gpointer data)
{
    GvShapes    *shapes = GV_SHAPES(gv_data);

    shapes->extents_valid = FALSE;
}

static void
gv_shapes_finalize(GObject *gobject)
{
    GvShapes *shapes = GV_SHAPES(gobject);
    int          i;

    if (shapes->shapes != NULL) {
      for( i = 0; i < gv_shapes_num_shapes(shapes); i++ )
        {
          if( gv_shapes_get_shape(shapes, i) != NULL )
            gv_shape_unref( gv_shapes_get_shape(shapes, i) );
        }

      g_ptr_array_free(shapes->shapes,TRUE);
      shapes->shapes = NULL;
    }

    /* Call parent class finalize */
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}

#ifndef HAVE_OGR
GvData *gv_shapes_from_ogr(const char *filename, int iLayer)
{
    CPLDebug( "OpenEV", 
              "gv_shapes_from_ogr(%s) called, but OGR not configured",
              filename );
    return NULL;
}

GvData *gv_shapes_from_ogr_layer(void *ogr_layer)
{
    CPLDebug( "OpenEV", "gv_shapes_from_ogr_layer() called, "
              "but OGR not configured" );
    return NULL;
}
#endif

void
gv_shapes_add_height(GvShapes *shapes, GvData *raster_data, double offset,
                     double default_height)
{
    int success, i, num_shapes;
    double x, y, z, last_z, imaginary, nodata_value;
    GvRaster *raster = GV_RASTER(raster_data);
    GvShapeChangeInfo change_info = {GV_CHANGE_REPLACE, 0, NULL};
    int    *id_list;

    /*
     * Notify of impending change.
     */
    num_shapes = gv_shapes_num_shapes(shapes);
    id_list = g_new( int, num_shapes );

    change_info.num_shapes = 0;
    change_info.shape_id = id_list;

    for (i=0; i < num_shapes; i++)
    {
        if( gv_shapes_get_shape(shapes,i) != NULL )
            id_list[change_info.num_shapes++] = i;
    }

    gv_data_changing(GV_DATA(shapes), &change_info);

    /*
     * Establish the "nodata" value.
     */
    success = gv_raster_get_nodata( raster, &nodata_value );
    if( !success )
        nodata_value = -1e8;

    /*
     * Loop over all shapes, applying height. 
     */
    for (i=0; i < num_shapes; i++)
    {
        GvShape *shape = gv_shapes_get_shape(shapes,i);
        int     ring, ring_count = gv_shape_get_rings( shape );

        if( shape == NULL )
            continue;

        last_z = default_height;

        for( ring = 0; ring < ring_count; ring++ )
        {
            int node, node_count = gv_shape_get_nodes( shape, ring );

            for( node = 0; node < node_count; node++ )
            {
                double  x_orig, y_orig;

                /* get xy in image space */
                x_orig = x = gv_shape_get_x( shape, ring, node );
                y_orig = y = gv_shape_get_y( shape, ring, node );
                z = 0.0;

                if (!gv_raster_georef_to_pixel(raster, &x, &y, &z))
                {
                    fprintf(stderr, "ERROR raster_georef_to_pixel failed!!!\n");
                    break;
                }

                if( x > -1.0 && x < 0.0 )
                    x = 0.0;
                if( y > -1.0 && y < 0.0 )
                    y = 0.0;
                if( x >= raster->width && x < raster->width+1 )
                    x = raster->width - 0.01;
                if( y >= raster->height && y < raster->height+1 )
                    y = raster->height - 0.01;

                /* Check if mesh xy values outside of height raster 
                   - leave as 0 */
                if( x >= 0.0 && x < raster->width
                    && y >= 0.0 && y < raster->height )
                {
                    if (!gv_raster_get_sample(raster, x, y, &z, &imaginary))
                    {
                        fprintf(stderr, 
                                "ERROR raster_get_sample failed for (x y z) %f %f\n", 
                                x, y);
                    }
                    else
                    {
                        if( z == nodata_value && x > 0 )
                            gv_raster_get_sample(raster, x-1, y, &z, 
                                                 &imaginary);

                        if( z == nodata_value && x < raster->width-1 )
                            gv_raster_get_sample(raster, x+1, y, &z, 
                                                 &imaginary);

                        if( z == nodata_value && y > 0 )
                            gv_raster_get_sample(raster, x, y-1, &z, 
                                                 &imaginary);

                        if( z == nodata_value && y < raster->height-1 )
                            gv_raster_get_sample(raster, x, y+1, &z, 
                                                 &imaginary);

                        if( z == nodata_value && x > 1 )
                            gv_raster_get_sample(raster, x-2, y, &z, 
                                                 &imaginary);

                        if( z == nodata_value && x < raster->width-2 )
                            gv_raster_get_sample(raster, x+2, y, &z, 
                                                 &imaginary);

                        if( z == nodata_value && y > 1 )
                            gv_raster_get_sample(raster, x, y-2, &z, 
                                                 &imaginary);

                        if( z == nodata_value && y < raster->height-2 )
                            gv_raster_get_sample(raster, x, y+2, &z, 
                                                 &imaginary);

                        if( z == nodata_value )
                            z = last_z;
                        else
                            z += offset;

                        last_z = z;
                    }
                }

                gv_shape_set_xyz( shape, ring, node, x_orig, y_orig, z );
            }
        }
    }

    /* notify of completed change */
    gv_data_changed(GV_DATA(shapes), &change_info);

    g_free( id_list);
}
