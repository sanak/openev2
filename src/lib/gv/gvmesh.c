/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Geometric mesh mapping tile s/t coordinates to display x/y/z
 *           coordinates.
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

#include <stdio.h>
#include <math.h>
#include "gvviewarea.h"
#include <GL/gl.h>
#include "gvmesh.h"
#include "gvraster.h"
#include "gvrasterlayer.h"
#include "gvmanager.h"

static void gv_mesh_class_init(GvMeshClass *klass);
static void gv_mesh_init(GvMesh *mesh);
static void gv_mesh_dispose(GObject *gobject);
static void gv_mesh_finalize(GObject *gobject);
static GvDataClass *parent_class = NULL;

#define GV_MESH_RIGHT_X_BIT 1
#define GV_MESH_LEFT_X_BIT  2
#define GV_MESH_TOP_Y_BIT   4
#define GV_MESH_BOT_Y_BIT   8

#define DEG2RAD         0.01745329252
#define RAD2DEG         57.2986885

GType
gv_mesh_get_type(void)
{
    static GType mesh_type = 0;

    if (!mesh_type) {
        static const GTypeInfo mesh_info =
        {
            sizeof(GvMeshClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_mesh_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvMesh),
            0,
            (GInstanceInitFunc) gv_mesh_init,
        };
        mesh_type = g_type_register_static (GV_TYPE_DATA,
                                            "GvMesh",
                                            &mesh_info, 0);
    }

    return mesh_type;
}

static void
gv_mesh_init(GvMesh *mesh)
{

    mesh->vertices = NULL;
    mesh->tex_coords = NULL;

}

static void
gv_mesh_class_init(GvMeshClass *klass)
{
    parent_class = g_type_class_peek_parent (klass);

    /* ---- Override finalize ---- */
    G_OBJECT_CLASS(klass)->dispose = gv_mesh_dispose;
    G_OBJECT_CLASS(klass)->finalize = gv_mesh_finalize;
}

static GArray *
gv_mesh_get_tile_tex_coords( GvMesh *mesh, int tile, int lod )

{
    gint     tiles_across, tiles_down;
    GPtrArray *parray;

    if( lod >= mesh->raster->max_lod )
        lod = mesh->raster->max_lod - 1;
    else if( lod < 0 )
        lod = 0;

    tiles_across = mesh->raster->tiles_across;
    tiles_down = mesh->raster->tiles_down;

    if( tile >= tiles_across * (tiles_down-1) )
    {
        if( (tile % tiles_across) == (tiles_across-1) )
            parray = mesh->tex_coords_bottom_right;
        else
            parray = mesh->tex_coords_bottom;
    }
    else if( (tile % tiles_across) == (tiles_across-1) )
        parray = mesh->tex_coords_right;
    else
        parray = mesh->tex_coords;

    return (GArray *) g_ptr_array_index(parray,lod);
}

gint *
gv_mesh_get_tile_corner_coords( GvMesh *mesh, int tile)
{
    gint tiles_across = mesh->raster->tiles_across;
    /* gint tiles_down   = mesh->raster->tiles_down; */

    gint tile_x = mesh->raster->tile_x;
    gint tile_y = mesh->raster->tile_y;

    gint *coords = (gint *)malloc( 8*sizeof(gint) );
    if(coords == NULL)
    {
        g_error("Out of memory!!!");
        return NULL;
    }

    /* top left corner */
    coords[0] = tile_x * (tile % tiles_across);
    coords[1] = tile_y * (tile / tiles_across);

    /* top right corner */
    coords[2] = coords[0] + tile_x;
    coords[3] = coords[1];

    /* bottom left corner */
    coords[4] = coords[0];
    coords[5] = coords[1] + tile_y;

    /* bottom right corner */
    coords[6] = coords[0] + tile_x;
    coords[7] = coords[1] + tile_y;

    /* Check that aren't past edge of image on far right and bottom */
    if ( coords[2] >= mesh->raster->width)
    {
        coords[2] = mesh->raster->width - 1;
        coords[6] = mesh->raster->width - 1;
    }

    if ( coords[5] >= mesh->raster->height)
    {
        coords[5] = mesh->raster->height - 1;
        coords[7] = mesh->raster->height - 1;
    }

    return coords;

}

static void
gv_mesh_build_tex_coord( GvMesh *mesh, GArray *tex_coords, gint lod,
                         gint ds_x, gint ds_y,
                         gint stride_x, gint stride_y )

{
    gint   i, e;
    gint   tile_x = mesh->tile_x;
    gint   tile_y = mesh->tile_y;
    gint   edge_width;

    /*
    ** Edge mesh tiles (bottom, right and bottom/right) that are not a
    ** reduced size, will be computed as having a remainder of zero, and
    ** given a size of 2.  Adjust these.
    */

    if( ds_x == 2 )
        ds_x += mesh->tile_x-GV_TILE_OVERLAP;

    if( ds_y == 2 )
        ds_y += mesh->tile_y-GV_TILE_OVERLAP;

    /* Compute */

    edge_width = (1 << lod) * GV_TILE_OVERLAP/2;

    for( i = 0; i <= tile_y; i += stride_y )
    {
        for( e = 0; e <= tile_x; e += stride_x )
        {
            float s, t;

            s = (float) e / (float) tile_x;
            t = (float) i / (float) tile_y;

            s = (s * (ds_x-edge_width*2) + edge_width) / tile_x;
            t = (t * (ds_y-edge_width*2) + edge_width) / tile_y;

            g_array_append_val( tex_coords, s );
            g_array_append_val( tex_coords, t );
        }
    }
}


GvMesh *
gv_mesh_new_identity( GvRaster *raster, gint detail )
{
    gint begin_x, begin_y;
    gint stride_x, stride_y;
    gint i, e, j, k, mesh_lod;
    gint tiles_down, tiles_across, tile;
    gint tile_x = raster->tile_x;
    gint tile_y = raster->tile_y;
    gint x = raster->width;
    gint y = raster->height;
    GArray *tex_coords;

    GvMesh *mesh = g_object_new (GV_TYPE_MESH, NULL);

    mesh->raster = g_object_ref_sink(raster);

    while( (tile_x >> detail) == 0 || (tile_y >> detail) == 0 )
        detail--;

    stride_x = tile_x >> detail;
    stride_y = tile_y >> detail;

    mesh->detail = detail;
    mesh->tile_x = tile_x;
    mesh->tile_y = tile_y;
    mesh->width = x;
    mesh->height = y;

    tiles_across = raster->tiles_across;
    tiles_down   = raster->tiles_down;

    mesh->max_tiles = tiles_across * tiles_down;
    mesh->vertices = g_array_new( FALSE, FALSE, sizeof( GArray *) );

    /* Set top left coords */

    mesh->corner_coords[0] = 0.0;
    mesh->corner_coords[1] = (float) y;

    /* Set top right coords */

    mesh->corner_coords[2] = (float) x;
    mesh->corner_coords[3] = (float) y;

    /* Set bottom left coords */

    mesh->corner_coords[4] = 0.0;
    mesh->corner_coords[5] = 0.0;

    /* Set bottom right coords */

    mesh->corner_coords[6] = (float) x;
    mesh->corner_coords[7] = 0.0;

    /* prepare meshes for each level of detail */

    mesh->tex_coords = g_ptr_array_new();
    mesh->tex_coords_right = g_ptr_array_new();
    mesh->tex_coords_bottom = g_ptr_array_new();
    mesh->tex_coords_bottom_right = g_ptr_array_new();

    for( mesh_lod = 0; mesh_lod < raster->max_lod; mesh_lod++ )
    {
        /* First we set the full tile s/t coordinate mappings */
        tex_coords = g_array_new( FALSE, FALSE, sizeof( float ) );
        g_ptr_array_add(mesh->tex_coords, tex_coords );
        gv_mesh_build_tex_coord( mesh, tex_coords, mesh_lod,
                                 tile_x, tile_y, stride_x, stride_y );

        /* Generate right hand side partial tile s/t coordinates */
        tex_coords = g_array_new( FALSE, FALSE, sizeof( float ) );;
        g_ptr_array_add(mesh->tex_coords_right, tex_coords );
        gv_mesh_build_tex_coord( mesh, tex_coords, mesh_lod,
                                 (mesh->width % (tile_x-GV_TILE_OVERLAP)) + 2,
                                 tile_y,
                                 stride_x, stride_y );
        /* Generate bottom side partial tile s/t coordinates */
        tex_coords = g_array_new( FALSE, FALSE, sizeof( float ) );;
        g_ptr_array_add(mesh->tex_coords_bottom, tex_coords );
        gv_mesh_build_tex_coord( mesh, tex_coords, mesh_lod,
                                 tile_x,
                                 (mesh->height % (tile_y-GV_TILE_OVERLAP)) + 2,
                                 stride_x, stride_y );

        /* Generate bottom/right partial tile s/t coordinates */
        tex_coords = g_array_new( FALSE, FALSE, sizeof( float ) );;
        g_ptr_array_add(mesh->tex_coords_bottom_right, tex_coords );
        gv_mesh_build_tex_coord( mesh, tex_coords, mesh_lod,
                                 (mesh->width  % (tile_x-GV_TILE_OVERLAP)) + 2,
                                 (mesh->height % (tile_y-GV_TILE_OVERLAP)) + 2,
                                 stride_x, stride_y );
    }

    /* generate output raster coordinates for every tile */
    tile = 0;
    for( i = 0; i < tiles_down; i++ )
    {
        for( e = 0; e < tiles_across; e++ )
        {
            GArray *tile_vertices;
            int     tex_index = 0;

            tex_coords = gv_mesh_get_tile_tex_coords( mesh, tile, 0 );

            begin_x = e * (tile_x-GV_TILE_OVERLAP) - GV_TILE_OVERLAP/2;
            begin_y = i * (tile_y-GV_TILE_OVERLAP) - GV_TILE_OVERLAP/2;

            tile_vertices = g_array_new( FALSE, FALSE, sizeof( float ) );

            for( j = 0; j <= tile_y; j += stride_y )
            {
                for( k = 0; k <= tile_x; k += stride_x )
                {
                    float t_x, t_y, t_z;
                    float s, t;

                    s = g_array_index( tex_coords, float, 2*tex_index );
                    t = g_array_index( tex_coords, float, 2*tex_index+1);
                    tex_index++;

                    t_x = begin_x + s*tile_x;
                    t_y = y - (begin_y + t*tile_y);
                    t_z = 0.0;

                    g_array_append_val( tile_vertices, t_x );
                    g_array_append_val( tile_vertices, t_y );
                    g_array_append_val( tile_vertices, t_z );
                }
            }

            g_array_append_val( mesh->vertices, tile_vertices );
            tile++;
        }
    }

    return mesh;
}

void
gv_mesh_reset_to_identity( GvMesh *mesh )

{
    int i, e, tile, tiles_across, tiles_down, stride_x, stride_y;
    int begin_x, begin_y;
    gint tile_x = mesh->tile_x;
    gint tile_y = mesh->tile_y;

    stride_x = mesh->tile_x >> mesh->detail;
    stride_y = mesh->tile_y >> mesh->detail;

    tiles_across = mesh->raster->tiles_across;
    tiles_down   = mesh->raster->tiles_down;

    /* generate output raster coordinates for every tile */
    tile = 0;
    for( i = 0; i < tiles_down; i++ )
    {
        for( e = 0; e < tiles_across; e++ )
        {
            GArray *tex_coords;
            GArray *tile_vertices;
            int     tex_index = 0, out_vert = 0, j, k;

            tex_coords = gv_mesh_get_tile_tex_coords( mesh, tile, 0 );

            begin_x = e * (tile_x-GV_TILE_OVERLAP) - GV_TILE_OVERLAP/2;
            begin_y = i * (tile_y-GV_TILE_OVERLAP) - GV_TILE_OVERLAP/2;

            tile_vertices = g_array_index( mesh->vertices, GArray *, tile );

            for( j = 0; j <= tile_y; j += stride_y )
            {
                for( k = 0; k <= tile_x; k += stride_x )
                {
                    float t_x, t_y;
                    float s, t;

                    s = g_array_index( tex_coords, float, 2*tex_index );
                    t = g_array_index( tex_coords, float, 2*tex_index+1);
                    tex_index++;

                    t_x = begin_x + s*tile_x;
                    t_y = begin_y + t*tile_y;

                    g_array_index(tile_vertices, float, out_vert++) = t_x;
                    g_array_index(tile_vertices, float, out_vert++) = t_y;
                    out_vert++;
                }
            }

            tile++;
        }
    }
}


/* Add height values to mesh based on provided hight raster
   - go through mesh->vertices and for every x-y co-ord transform back to image co-ordinates
   - query height raster at x-y point and set mesh->verticies z value to it
 */
void
gv_mesh_add_height( GvMesh *mesh, GvRaster *raster,
                    double default_height )
{
    int     i, j, success, iteration, max_iteration;
    float   z_float;
    double  x, y, z, imaginary;
    double  nodata_value;

    success = gv_raster_get_nodata( raster, &nodata_value );
    if( !success )
        nodata_value = -1e8;

    /* number of tiles */
    for( i=0; i < mesh->vertices->len; i++)
    {
        GArray *tile_vertices;

        tile_vertices = g_array_index( mesh->vertices, GArray *, i);

        /* Vertices in tile */
        for( j=0; j < (tile_vertices->len/3); j++)
        {
            /* get xy in image space */
            x = (float) g_array_index( tile_vertices, float, 3*j);
            y = (float) g_array_index( tile_vertices, float, 3*j+1);
            z = (float) g_array_index( tile_vertices, float, 3*j+2);

            /*            printf("x y z %f %f %f", x, y, z); */

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

            /* Check if mesh xy values outside of height raster - leave as 0 */
            if( x >= 0.0 && x < raster->width
                && y >= 0.0 && y < raster->height )
            {
                if (!gv_raster_get_sample(raster, x, y, &z, &imaginary))
                {
                    fprintf(stderr,
                            "ERROR raster_get_sample failed for (x y z) %f %f\n",
                            x, y);
                    z_float = (float)nodata_value;
                }
                else
                {
                    z_float = z;
                }
            }
            else
                z_float = (float)nodata_value;

            g_array_index( tile_vertices, float, 3*j+2) = z_float;
        }

        mesh->vertices = g_array_remove_index(mesh->vertices, i);
        g_array_insert_val(mesh->vertices, i, tile_vertices);
    }

    /*
     * Make another pass, trying to fill in mesh elevations for vertices
     * where we got nodata, or fell off the available data.
     *
     * We run this twice to propagate out values an extra step.
     */

    max_iteration = 3;
    for( iteration = 0; iteration < max_iteration; iteration++ )
    {
        /* number of tiles */
        for( i=0; i < mesh->vertices->len; i++)
        {
            GArray *tile_vertices;
            int j, k;
            int mesh_xsize, mesh_ysize;
            float       *data;

            tile_vertices = g_array_index( mesh->vertices, GArray *, i);
            data = (float *) tile_vertices->data;

            mesh_xsize = (1 << mesh->detail) + 1;
            mesh_ysize = (1 << mesh->detail) + 1;

            g_assert( mesh_xsize * mesh_ysize * 3 == tile_vertices->len );

            for( j = 0; j < mesh_ysize; j++ )
            {
                for( k = 0; k < mesh_xsize; k++ )
                {
                    float       sum = 0;
                    int count = 0;
                    int t_i;

                    t_i = j * mesh_xsize + k;

                    /* skip entries with an OK value */
                    if( data[3*t_i+2] != (float)nodata_value )
                        continue;

                    /* check mesh entry to left */
                    if( k > 0 && data[3*(t_i-1)+2] != (float)nodata_value )
                    {
                        sum += data[3*(t_i-1)+2];
                        count++;
                    }

                    /* check mesh entry to right */
                    if( k+1 < mesh_xsize && data[3*(t_i+1)+2] != (float)nodata_value )
                    {
                        sum += data[3*(t_i+1)+2];
                        count++;
                    }

                    /* check mesh entry above */
                    if( j > 0
                        && data[3*(t_i-mesh_xsize)+2] != (float)nodata_value )
                    {
                        sum += data[3*(t_i-mesh_xsize)+2];
                        count++;
                    }

                    /* check mesh entry below */
                    if( j+1 < mesh_ysize
                        && data[3*(t_i+mesh_xsize)+2] != (float)nodata_value )
                    {
                        sum += data[3*(t_i+mesh_xsize)+2];
                        count++;
                    }

                    /* if we got hits, average them and assign */
                    if( count > 0 )
                    {
                        data[3*t_i+2] = sum / count;
                    }
                    else if( iteration == max_iteration-1 )
                    {
                        data[3*t_i+2] = default_height;
                    }
                }
            }
        }
    }
}

/*
 * Set upper/lower bounds for the height (eg. if a DEM
 * defaults to -32767 for patches of missing data within
 * the image, gv_mesh_add_height will treat it as valid data.
 * This function takes a pass through the vertices
 * and clamps if necessary).  This will also get rid of the
 * effects of a very low nodata_value (when the DEM doesn't
 * cover the full area of the raster being draped atop it).
 */

void
gv_mesh_clamp_height( GvMesh *mesh, int bclamp_min, int bclamp_max,
                    double min_height, double max_height )
{

    int i;

    /*
     * Clamp minimum z value to min_height if bclamp_min = 1
     * Clamp maximum z value to max_height if bclamp_max = 1
     */

    /* number of tiles */
    for( i=0; i < mesh->vertices->len; i++)
    {
        GArray *tile_vertices;
        int     j, k;
        int     mesh_xsize, mesh_ysize;
        float   *data;

        tile_vertices = g_array_index( mesh->vertices, GArray *, i);
        data = (float *) tile_vertices->data;

        mesh_xsize = (1 << mesh->detail) + 1;
        mesh_ysize = (1 << mesh->detail) + 1;

        g_assert( mesh_xsize * mesh_ysize * 3 == tile_vertices->len );


        for( j = 0; j < mesh_ysize; j++ )
        {
            for( k = 0; k < mesh_xsize; k++ )
            {
                int     t_i;

                t_i = j * mesh_xsize + k;

                if (( bclamp_min == 1 ) && ( data[3*t_i+2] < min_height ))
                    data[3*t_i+2] = min_height;

                if (( bclamp_max == 1 ) && ( data[3*t_i+2] > max_height ))
                    data[3*t_i+2] = max_height;

            }
        }
    }
}


/*
 * This function assumes the mesh is the result of gv_mesh_new_identity, and
 * maps a raw raster space into an equivelent OpenGL model space.  Now
 * transform the model space outputs to be georeferenced coordinates instead
 * of pixel/line identity values.
 *
 * The "geotransform" is the usual six word affine transform.
 */

void
gv_mesh_set_transform( GvMesh *mesh, gint xsize, gint ysize,
                       double *geotransform )
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
            float     x_out, y_out;

            x_out = geotransform[0]
                + xyz_verts[0] * geotransform[1]
                + (ysize - xyz_verts[1]) * geotransform[2];
            y_out = geotransform[3]
                + xyz_verts[0] * geotransform[4]
                + (ysize - xyz_verts[1]) * geotransform[5];

            xyz_verts[0] = x_out;
            xyz_verts[1] = y_out;

            xyz_verts += 3;
        }
    }
}



GvMeshTile *
gv_mesh_get( GvMesh *mesh, gint tile, gint raster_lod, gint detail,
             GvMeshTile *tile_info )
{
    gint i, e, step;
    gint dimensions = (1 << mesh->detail) + 1;
    GArray *verts;
    static GArray *g_indices = NULL;

    if( tile_info == NULL )
    {
        if( ( tile_info = g_new( GvMeshTile, 1 ) ) == NULL )
        {
            return NULL;
        }
    }

    if( detail < raster_lod )
        detail = raster_lod;

    if( tile < mesh->max_tiles )
    {
        if( mesh->tex_coords )
        {
            GArray  *tex_coords;

            tex_coords = gv_mesh_get_tile_tex_coords( mesh, tile, detail );
            tile_info->tex_coords = (float *) tex_coords->data;
        }
        else
            tile_info->tex_coords = NULL;

        verts = g_array_index( mesh->vertices, GArray *, tile );

        if( verts == NULL )
        {
            fprintf( stderr, "Missing vertices information\n" );
        }

        tile_info->vertices = (float *)verts->data;

        /* Figure out the spacing for the index */

        /*  What does this do ??? */
        if( detail >= mesh->detail )
        {
            /* Use mesh spacing */
            step = 1;
            detail = mesh->detail;
        } else {
            /* Use raster spacing */
            step = 1 << (mesh->detail - detail);
        }

        /* Note: overwrites above calculations */
        step = 1;
        detail = mesh->detail;


        /* Now we build the indices and get ready to return the tile */

        tile_info->restarts = 0;
        tile_info->list_type = GL_TRIANGLE_STRIP;

        if( g_indices )
        {
            g_array_set_size( g_indices, 0 );
        } else {
            g_indices = g_array_new( FALSE, FALSE, sizeof( int ) );
        }

        for( i = 0; i < dimensions-1; i += step )
        {
            for( e = 0; e < dimensions; e += step )
            {
                gint val;

                val = i * dimensions + e;

                g_array_append_val( g_indices, val );

                val = (i+step) * dimensions + e;

                g_array_append_val( g_indices, val );
            }
            tile_info->restarts++;
        }

        switch( tile_info->list_type )
        {
            case 0:
                tile_info->range = dimensions*4;
                break;
            case GL_TRIANGLE_STRIP:
                /* Only this case works right now */
                tile_info->range = ( (1 << detail ) + 1 ) * 2;
                break;
            case 2:
                tile_info->range = 2+dimensions;
                break;
            case 3:
                tile_info->range = dimensions*2;
                break;
        }

        tile_info->restarts--;
        tile_info->indices = (gint *) g_indices->data;
    } else {
        return NULL;
    }
    return tile_info;

}

void gv_mesh_extents( GvMesh *mesh, GvRect *rect )
{
    float x[4], y[4];
    int dimensions;
    int tile;
    GArray *verts;

    if( mesh )
    {
        gint   tiles_per_row;

        /* First we get the top left tile */

        if( ( verts = g_array_index( mesh->vertices, GArray *, 0 ) ) == NULL )
        {
            return;
        }

        dimensions = ( 1 << mesh->detail ) + 1;

        x[0] = g_array_index( verts, float, 0 );
        y[0] = g_array_index( verts, float, 1 );

        /* The we do the top right tile */

        tiles_per_row = mesh->raster->tiles_across;
        tile = tiles_per_row - 1;

        if( ( verts = g_array_index( mesh->vertices, GArray *, tile ) ) == NULL )
        {
            return;
        }

        x[1] = g_array_index( verts, float, 3*(dimensions-1) );
        y[1] = g_array_index( verts, float, 3*(dimensions-1)+1 );

        /* The we do the bottom left tile */

        tile = mesh->raster->tiles_across * (mesh->raster->tiles_down-1);

        if( ( verts = g_array_index( mesh->vertices, GArray *, tile ) ) == NULL )
        {
            return;
        }

        x[2] = g_array_index( verts, float, 3*(dimensions*(dimensions-1)) );
        y[2] = g_array_index( verts, float, 3*(dimensions*(dimensions-1))+1);

        /* The we do the last tile */

        if( ( verts = g_array_index( mesh->vertices, GArray *, mesh->max_tiles-1 ) ) == NULL )
        {
            return;
        }

        x[3] = g_array_index( verts, float, 3*(dimensions*dimensions-1) );
        y[3] = g_array_index( verts, float, 3*(dimensions*dimensions-1)+1 );

    }

    rect->x = MIN( MIN( MIN( x[0], x[1] ), x[2] ), x[3] );
    rect->y = MIN( MIN( MIN( y[0], y[1] ), y[2] ), y[3] );

    rect->width = MAX( MAX( MAX( x[0], x[1] ), x[2] ), x[3] ) - rect->x;
    rect->height = MAX( MAX( MAX( y[0], y[1] ), y[2] ), y[3] ) - rect->y;

    return;
}

/************************************************************************/
/*                         gv_mesh_get_height()                         */
/*                                                                      */
/*      Fetch the mesh height at the indicated location.                */
/************************************************************************/

float gv_mesh_get_height( GvMesh *mesh,
                          double x, double y, int *success )

{
    int         tile, tile_in_x, tile_in_y, vert_off;
    int         mesh_pnts_per_line, mesh_pnts_per_column, vert_x, vert_y;
    int         factor;
    double      stride_x, stride_y;
    GvRaster    *raster = mesh->raster;
    double      pl_x, pl_y;
    GArray      *tile_vertices;
    float       x_vert[4], y_vert[4], z_vert[4], u, v;
    int         tile_width, tile_height;

    if( success != NULL )
        *success = FALSE;

/* -------------------------------------------------------------------- */
/*      What pixel/line location are we looking for.  Provide a         */
/*      little bit of fudging if necessary to avoid stuff on the        */
/*      edge falling out of bounds.                                     */
/* -------------------------------------------------------------------- */

    pl_x = x;
    pl_y = y;

    gv_raster_georef_to_pixel( raster, &pl_x, &pl_y, NULL );

    if( pl_x < 0.0 && pl_x > -0.01 )
        pl_x += 0.01;
    if( pl_y < 0.0 && pl_y > -0.01 )
        pl_y += 0.01;
    if( pl_x >= raster->width && pl_x < raster->width + 0.01 )
        pl_x -= 0.01;
    if( pl_y >= raster->height && pl_y < raster->height + 0.01 )
        pl_y -= 0.01;

    if( pl_x < 0 || pl_x >= raster->width
        || pl_y < 0 || pl_y >= raster->height )
    {
        CPLDebug( "OpenEV",
                  "Didn't get a value (1) for location Geo:(%g,%g) PL:(%g,%g) Size:(%d,%d)",
                  x, y, pl_x, pl_y, raster->width, raster->height );
        return 0.0;
    }

/* -------------------------------------------------------------------- */
/*      What tile does this fall in.                                    */
/* -------------------------------------------------------------------- */
    tile_in_x = ((int) pl_x) / (raster->tile_x-GV_TILE_OVERLAP);
    tile_in_y = ((int) pl_y) / (raster->tile_y-GV_TILE_OVERLAP);

    tile = tile_in_x + tile_in_y * raster->tiles_across;

/* -------------------------------------------------------------------- */
/*      Where does our request fall within the tile.                    */
/* -------------------------------------------------------------------- */
    factor = 1 << mesh->detail;

    mesh_pnts_per_line = factor + 1;
    mesh_pnts_per_column = factor + 1;

    if( tile_in_x == raster->tiles_across - 1 )
        tile_width = mesh->width
            - (raster->tile_x - GV_TILE_OVERLAP) * tile_in_x;
    else
        tile_width = mesh->tile_x;

    if( tile_in_y == raster->tiles_down - 1 )
        tile_height = mesh->height
            - (raster->tile_y - GV_TILE_OVERLAP) * tile_in_y;
    else
        tile_height = mesh->tile_y;

    stride_x = tile_width / (float) factor;
    stride_y = tile_height / (float) factor;

    pl_x -= tile_in_x * (raster->tile_x - GV_TILE_OVERLAP);
    pl_y -= tile_in_y * (raster->tile_y - GV_TILE_OVERLAP);

    vert_x = (int) (pl_x / stride_x);
    vert_y = (int) (pl_y / stride_y);

    vert_off = vert_x + vert_y * mesh_pnts_per_line;

/* -------------------------------------------------------------------- */
/*      Find the neighbouring vertices.                                 */
/* -------------------------------------------------------------------- */
    g_assert( tile >= 0 && tile < mesh->vertices->len );
    tile_vertices = g_array_index( mesh->vertices, GArray *, tile );

    if( vert_off < 0 || vert_off*3 >= tile_vertices->len )
    {
        CPLDebug( "OpenEV",
                  "Didn't get a value for location (%g,%g), vert_off=%d\n",
                  x, y, vert_off );
        return 0.0;
    }

    x_vert[0] = g_array_index( tile_vertices, float, 3*vert_off   );
    y_vert[0] = g_array_index( tile_vertices, float, 3*vert_off+1 );
    z_vert[0] = g_array_index( tile_vertices, float, 3*vert_off+2 );

    if( vert_x+1 < mesh_pnts_per_line )
    {
        x_vert[1] = g_array_index( tile_vertices, float, 3*(vert_off+1)   );
        y_vert[1] = g_array_index( tile_vertices, float, 3*(vert_off+1)+1 );
        z_vert[1] = g_array_index( tile_vertices, float, 3*(vert_off+1)+2 );
    }
    else
    {
        x_vert[1] = x_vert[0];
        y_vert[1] = y_vert[0];
        z_vert[1] = z_vert[0];
    }

    if( vert_y+1 < mesh_pnts_per_column )
    {
        int     off = vert_off + mesh_pnts_per_line;

        x_vert[2] = g_array_index( tile_vertices, float, 3*off     );
        y_vert[2] = g_array_index( tile_vertices, float, 3*off + 1 );
        z_vert[2] = g_array_index( tile_vertices, float, 3*off + 2 );
    }
    else
    {
        x_vert[2] = x_vert[0];
        y_vert[2] = y_vert[0];
        z_vert[2] = z_vert[0];
    }

    if( vert_y+1 < mesh_pnts_per_column && vert_x+1 < mesh_pnts_per_line )
    {
        int     off = vert_off + mesh_pnts_per_line + 1;

        x_vert[3] = g_array_index( tile_vertices, float, 3*off     );
        y_vert[3] = g_array_index( tile_vertices, float, 3*off + 1 );
        z_vert[3] = g_array_index( tile_vertices, float, 3*off + 2 );
    }
    else if( vert_y+1 < mesh_pnts_per_column )
    {
        x_vert[3] = x_vert[2];
        y_vert[3] = y_vert[2];
        z_vert[3] = z_vert[2];
    }
    else
    {
        x_vert[3] = x_vert[1];
        y_vert[3] = y_vert[1];
        z_vert[3] = z_vert[1];
    }

/* -------------------------------------------------------------------- */
/*      Interpolate the value within this grid square.                  */
/* -------------------------------------------------------------------- */
    u = (pl_x - vert_x * stride_x) / (float) stride_x;
    v = (pl_y - vert_y * stride_y) / (float) stride_y;

    if( u < -0.0001 || u > 1.0001 || v < -0.0001 || v > 1.0001 )
        g_warning( "illegal u or v in gv_mesh_get_height()" );

#ifdef notdef
    /* Note: this will produce false positives for "on the edges" conditions*/
    if( x < MIN(x_vert[0],x_vert[1])
        || x > MAX(x_vert[0],x_vert[1])
        || y < MIN(y_vert[0],y_vert[2])
        || y > MAX(y_vert[0],y_vert[2]) )
    {
        CPLDebug( "OpenEV",
                  "x/y outside of quadrant in gv_mesh_get_height(), uv=%g,%g\n"
                  "  x,y=%g,%g   quad=(%g,%g),(%g,%g),(%g,%g),(%g,%g)",
                  u, v,
                  x, y,
                  x_vert[0], y_vert[0],
                  x_vert[1], y_vert[1],
                  x_vert[2], y_vert[2],
                  x_vert[3], y_vert[3] );
    }
#endif

    if( success != NULL )
        *success = TRUE;

    return z_vert[0] * (1.0-u) * (1.0-v)
        + z_vert[1] * (u) * (1.0-v)
        + z_vert[2] * (1.0-u) * (v)
        + z_vert[3] * (u) * (v);
}


static void
gv_mesh_dispose(GObject *gobject)
{
    GvMesh    *mesh = GV_MESH(gobject);


    if (mesh->raster != NULL)
    {
        g_object_unref(mesh->raster);
        mesh->raster = NULL;
    }

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->dispose(gobject);

}

/************************************************************************/
/*                          gv_mesh_finalize()                          */
/*                                                                      */
/*      Final destruction of GvMesh.                                    */
/************************************************************************/

static void
gv_mesh_finalize(GObject *gobject)
{
    GvMesh    *mesh = GV_MESH(gobject);
    int        ii;
    GPtrArray  *tex_coords;

    mesh->raster = NULL;

    /* free all vertices */
    if (mesh->vertices != NULL) {
      for( ii = 0; ii < mesh->vertices->len; ii++ )
        g_array_free( g_array_index( mesh->vertices, GArray *,ii), TRUE );
      g_array_free( mesh->vertices, TRUE );
      mesh->vertices = NULL;
    }

    /* free texture coordinates */
    if (mesh->tex_coords != NULL) {
      tex_coords = mesh->tex_coords;
      for( ii = 0; ii < tex_coords->len; ii++ )
        g_array_free( (GArray *) g_ptr_array_index(tex_coords,ii), TRUE );
      mesh->tex_coords = NULL;
    }
    if (mesh->tex_coords_right != NULL) {
      tex_coords = mesh->tex_coords_right;
      for( ii = 0; ii < tex_coords->len; ii++ )
        g_array_free( (GArray *) g_ptr_array_index(tex_coords,ii), TRUE );
      mesh->tex_coords_right = NULL;
    }
    if (mesh->tex_coords_bottom_right != NULL) {
      tex_coords = mesh->tex_coords_bottom_right;
      for( ii = 0; ii < tex_coords->len; ii++ )
        g_array_free( (GArray *) g_ptr_array_index(tex_coords,ii), TRUE );
      mesh->tex_coords_bottom_right = NULL;
    }
    if (mesh->tex_coords_bottom != NULL) {
      tex_coords = mesh->tex_coords_bottom;
      for( ii = 0; ii < tex_coords->len; ii++ )
        g_array_free( (GArray *) g_ptr_array_index(tex_coords,ii), TRUE );
      mesh->tex_coords_bottom = NULL;
    }

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}
