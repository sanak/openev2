/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Read/write link to ESRI Shapefiles from GvShapes.
 * Author:   Frank Warmerdam, warmerda@home.com
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
 * $Log: gvshapefile.c,v $
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
 * Revision 1.21  2003/05/27 21:32:40  warmerda
 * added support for reading multi-part arcs as a collection of lines
 *
 * Revision 1.20  2002/09/30 20:16:54  warmerda
 * ensure area rings are forced closed on save
 *
 * Revision 1.19  2002/09/12 15:20:39  warmerda
 * use DBFIsAttributeNULL() call
 *
 * Revision 1.18  2002/08/19 17:12:48  pgs
 * don't set property if source value is NULL, and when writing out a dbf file
 * make sure to write null values.
 *
 * Revision 1.17  2002/08/07 17:12:42  pgs
 * added missing include for CPLDebug
 *
 * Revision 1.16  2002/08/02 20:53:20  warmerda
 * dont produce g_warning() if dbf file missing
 *
 * Revision 1.15  2002/07/24 20:32:30  warmerda
 * minor performance improvements
 *
 * Revision 1.14  2002/07/18 19:33:57  pgs
 * added gv_shapes_to_dbf
 *
 * Revision 1.13  2002/07/15 18:42:10  pgs
 * added _filename to gv_shapes that come from shape files or ogr
 *
 * Revision 1.12  2001/08/08 17:43:33  warmerda
 * avoid leak of SHPObjects when reading
 *
 * Revision 1.11  2001/03/29 15:00:18  warmerda
 * find first _non-NULL_ shape when saving as template
 *
 * Revision 1.10  2001/03/19 21:40:12  pgs
 * use binary mode for dbfopen dummy
 *
 * Revision 1.9  2000/10/29 18:17:06  warmerda
 * add dummy field when saving shapefiles with no fields
 *
 * Revision 1.8  2000/08/25 20:08:18  warmerda
 * added better checking for empty shape layers
 *
 * Revision 1.7  2000/08/08 17:17:39  warmerda
 * fixed return value on save
 *
 * Revision 1.6  2000/08/04 14:17:28  warmerda
 * GvShapes shape ids now persistent
 *
 * Revision 1.5  2000/06/12 20:16:02  warmerda
 * added shapefile write support
 *
 */

#include "shapefil.h"
#include "gvshapes.h"
#include <stdlib.h>
#include "cpl_conv.h"

/************************************************************************/
/*                      gv_shapes_from_shapefile()                      */
/************************************************************************/

GvData *gv_shapes_from_shapefile(const char *filename)

{
    SHPHandle   shp_handle;
    DBFHandle   dbf_handle;
    int         shape_count, shape_index, field_count = 0, field_index;
    int         field_type;
    GvShapes    *shapes_data;
    GvProperties *properties;
    char        **field_names = NULL;


/* -------------------------------------------------------------------- */
/*      Open the .shp and .dbf file.                                    */
/* -------------------------------------------------------------------- */
    shp_handle = SHPOpen( filename, "rb" );
    dbf_handle = DBFOpen( filename, "rb" );
    if( dbf_handle == NULL && shp_handle == NULL )
    {
        g_warning( "Invalid shapefile and DBF." );
        return NULL;
    }
    else if( dbf_handle == NULL )
        g_warning( "Unable to open DBF file ... continuing anyways." );
    else
        field_count = DBFGetFieldCount( dbf_handle );


    if ( shp_handle != NULL )
        SHPGetInfo( shp_handle, &shape_count, NULL, NULL, NULL );
    else
    {
        CPLDebug( "OpenEV", "Unable to open shapefile, just using .dbf file.");
        shape_count = DBFGetRecordCount( dbf_handle );
    }

/* -------------------------------------------------------------------- */
/*      Create shapes layer, and assign some metadata about the         */
/*      field definitions.                                              */
/* -------------------------------------------------------------------- */
    shapes_data = GV_SHAPES(gv_shapes_new());
    properties = gv_data_get_properties( GV_DATA(shapes_data) );

    //set the filename property
    gv_properties_set( properties, "_filename", filename );

    field_names = (char **) g_new( char *, field_count+1 );

    for(field_index = 0; field_index < field_count; field_index++ )
    {
        char      prop_value[64], prop_name[64];
        int       field_type, width, precision;

        field_type = DBFGetFieldInfo( dbf_handle, field_index,
                                      prop_value, &width, &precision );

        sprintf( prop_name, "_field_name_%d", field_index+1 );
        gv_properties_set( properties, prop_name, prop_value );

        field_names[field_index] = g_strdup( prop_value );

        sprintf( prop_name, "_field_width_%d", field_index+1 );
        sprintf( prop_value, "%d", width );
        gv_properties_set( properties, prop_name, prop_value );

        if( field_type == FTDouble )
        {
            sprintf( prop_name, "_field_precision_%d", field_index+1 );
            sprintf( prop_value, "%d", precision );
            gv_properties_set( properties, prop_name, prop_value );
        }

        sprintf( prop_name, "_field_type_%d", field_index+1 );
        if( field_type == FTInteger )
            gv_properties_set( properties, prop_name, "integer" );
        else if( field_type == FTDouble )
            gv_properties_set( properties, prop_name, "float" );
        else
            gv_properties_set( properties, prop_name, "string" );
    }

/* -------------------------------------------------------------------- */
/*      Copy all the shapes, and their attributes.                      */
/* -------------------------------------------------------------------- */

    for( shape_index = 0; shape_index < shape_count; shape_index++ )
    {
        SHPObject  *src_shape;
        GvShape    *gv_shape = NULL;

        if ( shp_handle != NULL )
        {
            src_shape = SHPReadObject( shp_handle, shape_index );
            if( src_shape == NULL )
                continue;

            if( src_shape->nSHPType == SHPT_POINT
                || src_shape->nSHPType == SHPT_POINTM
                || src_shape->nSHPType == SHPT_POINTZ )
            {
                gv_shape = gv_shape_new( GVSHAPE_POINT );
                gv_shape_set_xyz( gv_shape, 0, 0,
                                  src_shape->padfX[0],
                                  src_shape->padfY[0],
                                  src_shape->padfZ[0] );
            }

            else if( src_shape->nSHPType == SHPT_ARC
                     || src_shape->nSHPType == SHPT_ARCM
                     || src_shape->nSHPType == SHPT_ARCZ )
            {
                int    node;

                if( src_shape->nParts == 1 )
                {
                    gv_shape = gv_shape_new( GVSHAPE_LINE );
                    for( node = src_shape->nVertices-1; node >= 0; node-- )
                        gv_shape_set_xyz( gv_shape, 0, node,
                                          src_shape->padfX[node],
                                          src_shape->padfY[node],
                                          src_shape->padfZ[node] );
                }
                else
                {
                    int part;
                    
                    gv_shape = gv_shape_new( GVSHAPE_COLLECTION );

                    for( part = 0; part < src_shape->nParts; part++ )
                    {
                        GvShape *line;
                        int     node, node_count, start;
                    
                        start = src_shape->panPartStart[part];
                        
                        if( part == src_shape->nParts-1 )
                            node_count = src_shape->nVertices - start;
                        else
                            node_count = src_shape->panPartStart[part+1] - start;
                        
                        line = gv_shape_new( GVSHAPE_LINE );

                        for( node = node_count - 1; node >= 0; node-- )
                            gv_shape_set_xyz( line, 0, node,
                                          src_shape->padfX[node+start],
                                          src_shape->padfY[node+start],
                                          src_shape->padfZ[node+start] );
                        
                        gv_shape_collection_add_shape( gv_shape, line );
                    }
                }
            }

            else if( src_shape->nSHPType == SHPT_POLYGON
                     || src_shape->nSHPType == SHPT_POLYGONM
                     || src_shape->nSHPType == SHPT_POLYGONZ )
            {
                int    part;

                gv_shape = gv_shape_new( GVSHAPE_AREA );
                for( part = 0; part < src_shape->nParts; part++ )
                {
                    int     node, node_count, start;

                    start = src_shape->panPartStart[part];

                    if( part == src_shape->nParts-1 )
                        node_count = src_shape->nVertices - start;
                    else
                        node_count = src_shape->panPartStart[part+1] - start;

                    for( node = node_count - 1; node >= 0; node-- )
                        gv_shape_set_xyz( gv_shape, part, node,
                                          src_shape->padfX[node+start],
                                          src_shape->padfY[node+start],
                                          src_shape->padfZ[node+start] );
                }
            }

            SHPDestroyObject( src_shape );
        }
        else
            gv_shape = gv_shape_new( GVSHAPE_COLLECTION );

        /* add other types later */

        if( gv_shape != NULL && dbf_handle != NULL )
        {
            properties = gv_shape_get_properties( gv_shape );

            for( field_index = 0; field_index < field_count; field_index++ )
            {
                const char  *field_value;

                if( DBFIsAttributeNULL(dbf_handle,shape_index,field_index) )
                    continue;

                field_value = DBFReadStringAttribute( dbf_handle, shape_index,
                                                      field_index );
                field_type = DBFGetFieldInfo( dbf_handle, field_index,
                                              NULL, NULL, NULL);

                gv_properties_set( properties,
                                   field_names[field_index],
                                   field_value );
            }
        }

        if( gv_shape != NULL )
            gv_shapes_add_shape( shapes_data, gv_shape );
    }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    if( dbf_handle != NULL )
        DBFClose( dbf_handle );
    if( shp_handle != NULL )
        SHPClose( shp_handle );

    for( field_index = 0; field_index < field_count; field_index++ )
        g_free( field_names[field_index] );
    g_free( field_names );

    gv_data_set_name( GV_DATA(shapes_data), filename );

    return GV_DATA(shapes_data);
}

/************************************************************************/
/*                      gv_shapes_read_from_file()                      */
/************************************************************************/

void gv_shapes_read_from_file(const char *filename, GvShapes *shapes_data)

{
    SHPHandle   shp_handle;
    DBFHandle   dbf_handle;
    int         shape_count, shape_index, field_count = 0, field_index;
    int         field_type;
    GvProperties *properties;
    char        **field_names = NULL;


/* -------------------------------------------------------------------- */
/*      Open the .shp and .dbf file.                                    */
/* -------------------------------------------------------------------- */
    shp_handle = SHPOpen( filename, "rb" );
    dbf_handle = DBFOpen( filename, "rb" );
    if( dbf_handle == NULL && shp_handle == NULL )
    {
        g_warning( "Invalid shapefile and DBF." );
        return;
    }
    else if( dbf_handle == NULL )
        g_warning( "Unable to open DBF file ... continuing anyways." );
    else
        field_count = DBFGetFieldCount( dbf_handle );


    if ( shp_handle != NULL )
        SHPGetInfo( shp_handle, &shape_count, NULL, NULL, NULL );
    else
    {
        CPLDebug( "OpenEV", "Unable to open shapefile, just using .dbf file.");
        shape_count = DBFGetRecordCount( dbf_handle );
    }

/* -------------------------------------------------------------------- */
/*      Create shapes layer, and assign some metadata about the         */
/*      field definitions.                                              */
/* -------------------------------------------------------------------- */
    properties = gv_data_get_properties( GV_DATA(shapes_data) );

    //set the filename property
    gv_properties_set( properties, "_filename", filename );

    field_names = (char **) g_new( char *, field_count+1 );

    for(field_index = 0; field_index < field_count; field_index++ )
    {
        char      prop_value[64], prop_name[64];
        int       field_type, width, precision;

        field_type = DBFGetFieldInfo( dbf_handle, field_index,
                                      prop_value, &width, &precision );

        sprintf( prop_name, "_field_name_%d", field_index+1 );
        gv_properties_set( properties, prop_name, prop_value );

        field_names[field_index] = g_strdup( prop_value );

        sprintf( prop_name, "_field_width_%d", field_index+1 );
        sprintf( prop_value, "%d", width );
        gv_properties_set( properties, prop_name, prop_value );

        if( field_type == FTDouble )
        {
            sprintf( prop_name, "_field_precision_%d", field_index+1 );
            sprintf( prop_value, "%d", precision );
            gv_properties_set( properties, prop_name, prop_value );
        }

        sprintf( prop_name, "_field_type_%d", field_index+1 );
        if( field_type == FTInteger )
            gv_properties_set( properties, prop_name, "integer" );
        else if( field_type == FTDouble )
            gv_properties_set( properties, prop_name, "float" );
        else
            gv_properties_set( properties, prop_name, "string" );
    }

/* -------------------------------------------------------------------- */
/*      Copy all the shapes, and their attributes.                      */
/* -------------------------------------------------------------------- */

    for( shape_index = 0; shape_index < shape_count; shape_index++ )
    {
        SHPObject  *src_shape;
        GvShape    *gv_shape = NULL;

        if ( shp_handle != NULL )
        {
            src_shape = SHPReadObject( shp_handle, shape_index );
            if( src_shape == NULL )
                continue;

            if( src_shape->nSHPType == SHPT_POINT
                || src_shape->nSHPType == SHPT_POINTM
                || src_shape->nSHPType == SHPT_POINTZ )
            {
                gv_shape = gv_shape_new( GVSHAPE_POINT );
                gv_shape_set_xyz( gv_shape, 0, 0,
                                  src_shape->padfX[0],
                                  src_shape->padfY[0],
                                  src_shape->padfZ[0] );
            }

            else if( src_shape->nSHPType == SHPT_ARC
                     || src_shape->nSHPType == SHPT_ARCM
                     || src_shape->nSHPType == SHPT_ARCZ )
            {
                int    node;

                if( src_shape->nParts == 1 )
                {
                    gv_shape = gv_shape_new( GVSHAPE_LINE );
                    for( node = src_shape->nVertices-1; node >= 0; node-- )
                        gv_shape_set_xyz( gv_shape, 0, node,
                                          src_shape->padfX[node],
                                          src_shape->padfY[node],
                                          src_shape->padfZ[node] );
                }
                else
                {
                    int part;
                    
                    gv_shape = gv_shape_new( GVSHAPE_COLLECTION );

                    for( part = 0; part < src_shape->nParts; part++ )
                    {
                        GvShape *line;
                        int     node, node_count, start;
                    
                        start = src_shape->panPartStart[part];
                        
                        if( part == src_shape->nParts-1 )
                            node_count = src_shape->nVertices - start;
                        else
                            node_count = src_shape->panPartStart[part+1] - start;
                        
                        line = gv_shape_new( GVSHAPE_LINE );

                        for( node = node_count - 1; node >= 0; node-- )
                            gv_shape_set_xyz( line, 0, node,
                                          src_shape->padfX[node+start],
                                          src_shape->padfY[node+start],
                                          src_shape->padfZ[node+start] );
                        
                        gv_shape_collection_add_shape( gv_shape, line );
                    }
                }
            }

            else if( src_shape->nSHPType == SHPT_POLYGON
                     || src_shape->nSHPType == SHPT_POLYGONM
                     || src_shape->nSHPType == SHPT_POLYGONZ )
            {
                int    part;

                gv_shape = gv_shape_new( GVSHAPE_AREA );
                for( part = 0; part < src_shape->nParts; part++ )
                {
                    int     node, node_count, start;

                    start = src_shape->panPartStart[part];

                    if( part == src_shape->nParts-1 )
                        node_count = src_shape->nVertices - start;
                    else
                        node_count = src_shape->panPartStart[part+1] - start;

                    for( node = node_count - 1; node >= 0; node-- )
                        gv_shape_set_xyz( gv_shape, part, node,
                                          src_shape->padfX[node+start],
                                          src_shape->padfY[node+start],
                                          src_shape->padfZ[node+start] );
                }
            }

            SHPDestroyObject( src_shape );
        }
        else
            gv_shape = gv_shape_new( GVSHAPE_COLLECTION );

        /* add other types later */

        if( gv_shape != NULL && dbf_handle != NULL )
        {
            properties = gv_shape_get_properties( gv_shape );

            for( field_index = 0; field_index < field_count; field_index++ )
            {
                const char  *field_value;

                if( DBFIsAttributeNULL(dbf_handle,shape_index,field_index) )
                    continue;

                field_value = DBFReadStringAttribute( dbf_handle, shape_index,
                                                      field_index );
                field_type = DBFGetFieldInfo( dbf_handle, field_index,
                                              NULL, NULL, NULL);

                gv_properties_set( properties,
                                   field_names[field_index],
                                   field_value );
            }
        }

        if( gv_shape != NULL )
            gv_shapes_add_shape( shapes_data, gv_shape );
    }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    if( dbf_handle != NULL )
        DBFClose( dbf_handle );
    if( shp_handle != NULL )
        SHPClose( shp_handle );

    for( field_index = 0; field_index < field_count; field_index++ )
        g_free( field_names[field_index] );
    g_free( field_names );

    gv_data_set_name( GV_DATA(shapes_data), filename );
}

/************************************************************************/
/*                       gv_shapes_to_shapefile()                       */
/************************************************************************/

int gv_shapes_to_shapefile(const char *filename, GvData *shapes_data,
                           int shp_type )

{
    GvShapes       *shapes = GV_SHAPES(shapes_data);
    GvShape        *shape;
    DBFHandle      dbf_handle;
    SHPHandle      shp_handle;
    int            field_index, field_count=0, shape_count, shape_index;
    GvProperties   *properties;
    int            max_vertices = 0, fid = -1;
    double         *x=NULL, *y=NULL, *z=NULL;

/* -------------------------------------------------------------------- */
/*      What sort of shapefile should we write?                         */
/* -------------------------------------------------------------------- */
    if( shp_type == SHPT_NULL )
    {
        int i = 0;

        shape = NULL;
        while( i < gv_shapes_num_shapes(shapes) && shape == NULL )
        {
            shape = gv_shapes_get_shape(shapes, i);
            i++;
        }

        if( shape == NULL )
        {
            g_warning("no shapes in layer to save ... unable to default type");
            return FALSE;
        }

        if( gv_shape_type(shape) == GVSHAPE_POINT )
            shp_type = SHPT_POINT;
        else if( gv_shape_type(shape) == GVSHAPE_LINE )
            shp_type = SHPT_ARC;
        else
            shp_type = SHPT_POLYGON;
    }

/* -------------------------------------------------------------------- */
/*      Try to create the named file(s).                                */
/* -------------------------------------------------------------------- */
    dbf_handle = DBFCreate( filename );
    if( dbf_handle == NULL )
    {
        g_warning( "Failed to create DBF file." );
        return FALSE;
    }

    shp_handle = SHPCreate( filename, shp_type );
    if( shp_handle == NULL )
    {
        g_warning( "Failed to create SHP file." );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Create the fields on the DBF file, if any.                      */
/* -------------------------------------------------------------------- */
    properties = gv_data_get_properties( shapes_data );
    for( field_index = 0; TRUE; field_index++ )
    {
        int  width, precision = 0, field_type;
        char prop_name[64];
        const char *prop_value;


        sprintf( prop_name, "_field_width_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value == NULL )
            break;
        width = atoi(prop_value);

        sprintf( prop_name, "_field_precision_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value != NULL )
            precision = atoi(prop_value);

        sprintf( prop_name, "_field_type_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value == NULL )
            prop_value = "string";

        if( g_strcasecmp(prop_value,"integer") == 0 )
            field_type = FTInteger;
        else if( g_strcasecmp(prop_value,"float") == 0 )
            field_type = FTDouble;
        else
            field_type = FTString;

        sprintf( prop_name, "_field_name_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value == NULL )
            break;

        DBFAddField( dbf_handle, prop_value, field_type, width, precision );
        field_count++;
    }

/* -------------------------------------------------------------------- */
/*      Add a dummy field if there are none.                            */
/* -------------------------------------------------------------------- */
    if( field_count == 0 )
    {
        DBFAddField( dbf_handle, "FID", FTInteger, 10, 0 );
        fid = 0;
    }

/* -------------------------------------------------------------------- */
/*      Start writing shapes, ignoring any that don't match our         */
/*      desired type.                                                   */
/* -------------------------------------------------------------------- */
    shape_count = gv_shapes_num_shapes(shapes);
    for( shape_index = 0; shape_index < shape_count; shape_index++ )
    {
        SHPObject   *shp_object;
        int         vertex_count, ring, shp_id;
        int         *ring_start = NULL, ring_count;

        shape = gv_shapes_get_shape( shapes, shape_index );

        if( shape == NULL )
            continue;

        /* skip shapes not matching desired type */
        if( (shp_type == SHPT_POINT && gv_shape_type(shape) != GVSHAPE_POINT)
         || (shp_type == SHPT_ARC && gv_shape_type(shape) != GVSHAPE_LINE)
         || (shp_type == SHPT_POLYGON && gv_shape_type(shape) != GVSHAPE_AREA))
            continue;

        /* count all the vertices */
        vertex_count = 0;
        ring_count = gv_shape_get_rings(shape);
        for( ring = 0; ring < ring_count; ring++ )
            vertex_count += gv_shape_get_nodes( shape, ring );

        if( max_vertices < vertex_count+1 )
        {
            if( x != NULL )
            {
                free(x);
                free(y);
                free(z);
            }

            max_vertices = vertex_count * 2 + 50;
            x = (double *) malloc(sizeof(double) * max_vertices);
            y = (double *) malloc(sizeof(double) * max_vertices);
            z = (double *) malloc(sizeof(double) * max_vertices);
        }

        vertex_count = 0;
        for( ring = 0; ring < ring_count; ring++ )
        {
            int   node, node_count;

            node_count = gv_shape_get_nodes( shape, ring );
            for( node = 0; node < node_count; node++ )
            {
                x[vertex_count] = gv_shape_get_x(shape,ring,node);
                y[vertex_count] = gv_shape_get_y(shape,ring,node);
                z[vertex_count] = gv_shape_get_z(shape,ring,node);
                vertex_count++;
            }
            
            /* force closing of unclosed area rings. */
            if( gv_shape_type(shape) == GVSHAPE_AREA 
                && (gv_shape_get_x(shape,ring,0) 
                       != gv_shape_get_x(shape,ring,node_count-1) 
                    || gv_shape_get_y(shape,ring,0) 
                       != gv_shape_get_y(shape,ring,node_count-1) ) )
            {
                x[vertex_count] = gv_shape_get_x(shape,ring,0);
                y[vertex_count] = gv_shape_get_y(shape,ring,0);
                z[vertex_count] = gv_shape_get_z(shape,ring,0);
                vertex_count++;
            }

            if( ring_start != NULL )
                ring_start[ring] = vertex_count;
        }

        if( gv_shape_get_rings( shape ) > 1 )
        {
            vertex_count = 0;
            ring_start = (int *) malloc(sizeof(int) * ring_count);

            for( ring = 0; ring < ring_count; ring++ )
            {
                ring_start[ring] = vertex_count;
                vertex_count += gv_shape_get_nodes( shape, ring );
            }
        }

        if( ring_count > 1 )
            shp_object = SHPCreateObject( shp_type, -1, ring_count, ring_start,
                                          NULL, vertex_count, x, y, z, NULL );
        else
            shp_object = SHPCreateObject( shp_type, -1, 0, NULL,
                                          NULL, vertex_count, x, y, z, NULL );
        if( ring_start != NULL )
        {
            free( ring_start );
            ring_start = NULL;
        }

        shp_id = SHPWriteObject( shp_handle, -1, shp_object );
        SHPDestroyObject( shp_object );

        /* Write the attributes of this shape that match the DBF schema */
        for( field_index = 0; field_index < field_count; field_index++ )
        {
            char field_name[32];
            const char * field_value;
            int  field_type;

            /* FIXME: This will fail for truncated field names! */
            field_type = DBFGetFieldInfo( dbf_handle, field_index,
                                          field_name, NULL, NULL);

            field_value = gv_properties_get( gv_shape_get_properties(shape),
                                             field_name);
            if( field_value == NULL )
                field_value = "";

            if( field_type == FTDouble )
                DBFWriteDoubleAttribute( dbf_handle, shp_id, field_index,
                                         atof(field_value) );
            else if( field_type == FTInteger )
                DBFWriteIntegerAttribute( dbf_handle, shp_id, field_index,
                                          atoi(field_value) );
            else
                DBFWriteStringAttribute( dbf_handle, shp_id, field_index,
                                         field_value );
        }

        if( fid != -1 )
        {
            DBFWriteIntegerAttribute( dbf_handle, shp_id, fid, shp_id );
        }
    }

    if( x != NULL )
    {
        free( x );
        free( y );
        free( z );
    }

    SHPClose( shp_handle );
    DBFClose( dbf_handle );

    return TRUE;
}

/************************************************************************/
/*                          gv_shapes_to_dbf()                          */
/************************************************************************/

int gv_shapes_to_dbf(const char *filename, GvData *shapes_data )

{
    GvShapes       *shapes = GV_SHAPES(shapes_data);
    GvShape        *shape;
    DBFHandle      dbf_handle;
    int            field_index, field_count=0, shape_count, shape_index;
    GvProperties   *properties;

/* -------------------------------------------------------------------- */
/*      Try to create the named file(s).                                */
/* -------------------------------------------------------------------- */
    dbf_handle = DBFCreate( filename );
    if( dbf_handle == NULL )
    {
        g_warning( "Failed to create DBF file." );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Create the fields on the DBF file, if any.                      */
/* -------------------------------------------------------------------- */
    properties = gv_data_get_properties( shapes_data );
    for( field_index = 0; TRUE; field_index++ )
    {
        int  width, precision = 0, field_type;
        char prop_name[64];
        const char *prop_value;

        sprintf( prop_name, "_field_width_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value == NULL )
            break;

        width = atoi(prop_value);

        sprintf( prop_name, "_field_precision_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value != NULL )
            precision = atoi(prop_value);

        sprintf( prop_name, "_field_type_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value == NULL )
            prop_value = "string";

        if( g_strcasecmp(prop_value,"integer") == 0 )
            field_type = FTInteger;
        else if( g_strcasecmp(prop_value,"float") == 0 )
            field_type = FTDouble;
        else
            field_type = FTString;

        sprintf( prop_name, "_field_name_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value == NULL )
            break;

        DBFAddField( dbf_handle, prop_value, field_type, width, precision );
        field_count++;
    }

/* -------------------------------------------------------------------- */
/*      Add a dummy field if there are none.                            */
/* -------------------------------------------------------------------- */
    if( field_count == 0 )
    {
        g_warning( "No attributes to save in DBF file." );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Start writing shapes, ignoring any that don't match our         */
/*      desired type.                                                   */
/* -------------------------------------------------------------------- */
    shape_count = gv_shapes_num_shapes(shapes);
    for( shape_index = 0; shape_index < shape_count; shape_index++ )
    {
        shape = gv_shapes_get_shape( shapes, shape_index );

        if( shape == NULL )
            continue;

        /* Write the attributes of this shape that match the DBF schema */
        for( field_index = 0; field_index < field_count; field_index++ )
        {
            char field_name[32];
            const char * field_value;
            int  field_type;

            /* FIXME: This will fail for truncated field names! */
            field_type = DBFGetFieldInfo( dbf_handle, field_index,
                                          field_name, NULL, NULL);

            field_value = gv_properties_get( gv_shape_get_properties(shape),
                                             field_name);
            if( field_value == NULL )
            {
                DBFWriteNULLAttribute( dbf_handle, shape_index, field_index );
            }
            else
            {
                if( field_type == FTDouble )
                    DBFWriteDoubleAttribute( dbf_handle, shape_index, field_index,
                                             atof(field_value) );
                else if( field_type == FTInteger )
                    DBFWriteIntegerAttribute( dbf_handle, shape_index, field_index,
                                              atoi(field_value) );
                else
                    DBFWriteStringAttribute( dbf_handle, shape_index, field_index,
                                             field_value );
            }
        }

    }

    DBFClose( dbf_handle );

    return TRUE;
}
