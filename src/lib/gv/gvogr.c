/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Read/write link to OGR.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
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
 * $Log: gvogr.c,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:26  uid1026
 *
 * Imported sources
 *
 * Revision 1.5  2004/01/20 16:05:02  warmerda
 * added setting of _ogr_driver_name for S52 viewer
 *
 * Revision 1.4  2003/01/06 21:19:45  warmerda
 * added one-layer access
 *
 * Revision 1.3  2003/01/06 18:21:51  warmerda
 * Added debug on unexpected NULL
 *
 * Revision 1.2  2002/11/04 16:33:45  sduclos
 * Add gard against NULL GeometryRef handle
 *
 * Revision 1.1  2002/09/26 19:24:09  warmerda
 * New
 *
 * Revision 1.8  2002/09/09 17:41:36  warmerda
 * cleanup dataset if iLayer out of range
 *
 * Revision 1.7  2002/07/15 18:42:10  pgs
 * added _filename to gv_shapes that come from shape files or ogr
 *
 * Revision 1.6  2002/05/07 02:51:15  warmerda
 * preliminary support for GVSHAPE_COLLECTION
 *
 * Revision 1.5  2001/11/09 15:20:24  warmerda
 * get ogr style string as _gv_ogrfs
 *
 * Revision 1.4  2000/11/13 20:04:13  warmerda
 * fixed memory leak with geometryless features
 *
 * Revision 1.3  2000/10/24 04:26:18  warmerda
 * easy of use, and bug fixes for ogr link
 *
 * Revision 1.2  2000/07/24 01:30:21  warmerda
 * added read support for multipolygons
 *
 * Revision 1.1  2000/07/03 12:48:22  warmerda
 * New
 *
 */

#include "ogr_api.h"
#include "ogr_srs_api.h"
#include "gvshapes.h"
#include "cpl_error.h"

// helper
int _getGeoPtCount(OGRGeometryH hGeom, int iGeo, OGRGeometryH *hGeomRef )
{
    int vert_count = 0;

    *hGeomRef  = OGR_G_GetGeometryRef( hGeom, iGeo );
    if( NULL != *hGeomRef )
    {
        vert_count = OGR_G_GetPointCount( *hGeomRef );
    }
    else
    {
        /* FIXME: something is wrong in OGR if we get here
         * ie the geometry handle doesn't refer to a geometry!
         */
        CPLDebug( "OpenEV", "gvogr.c:_getGeoPtCount() .. got null geometry!" );
    }

    return vert_count;
}

/************************************************************************/
/*                      ogr_geometry_to_gv_shape()                      */
/************************************************************************/

static GvShape *ogr_geometry_to_gv_shape( OGRGeometryH hGeom )

{
    GvShape *gv_shape = NULL;
    OGRwkbGeometryType eType;

    if( hGeom == NULL )
    {
        // Use collection - which can be empty - to represent a geometryless
        // feature.

        return gv_shape_new( GVSHAPE_COLLECTION );
    }

    eType = wkbFlatten(OGR_G_GetGeometryType(hGeom));

    if( eType == wkbPoint )
    {
        gv_shape = gv_shape_new( GVSHAPE_POINT );
        gv_shape_set_xyz( gv_shape, 0, 0,
                          OGR_G_GetX( hGeom, 0 ),
                          OGR_G_GetY( hGeom, 0 ),
                          OGR_G_GetZ( hGeom, 0 ) );
    }

    else if( eType == wkbLineString )
    {
        int    node, count = OGR_G_GetPointCount(hGeom);

        gv_shape = gv_shape_new( GVSHAPE_LINE );
        for( node = count-1; node >= 0; node-- )
            gv_shape_set_xyz( gv_shape, 0, node,
                              OGR_G_GetX( hGeom, node ),
                              OGR_G_GetY( hGeom, node ),
                              OGR_G_GetZ( hGeom, node ) );
    }

    else if( eType == wkbPolygon )
    {
        OGRGeometryH hRing;
        int        iRing = 0, nRingCount = OGR_G_GetGeometryCount( hGeom );

        gv_shape = gv_shape_new( GVSHAPE_AREA );

        for( iRing = 0; iRing < nRingCount; iRing++ )
        {
            int node;
            int vert_count = _getGeoPtCount( hGeom, iRing, &hRing );

            //hRing      = OGR_G_GetGeometryRef( hGeom, iRing );
            //vert_count = OGR_G_GetPointCount(hRing);

            for( node = vert_count - 1; node >= 0; node-- )
                gv_shape_set_xyz( gv_shape, iRing, node,
                                  OGR_G_GetX( hRing, node ),
                                  OGR_G_GetY( hRing, node ),
                                  OGR_G_GetZ( hRing, node ) );
        }
    }

    else if( eType == wkbMultiPolygon )
    {
        int iPoly, nShapeRing = 0;

        gv_shape = gv_shape_new( GVSHAPE_AREA );

        for( iPoly = 0; iPoly < OGR_G_GetGeometryCount( hGeom ); iPoly++ )
        {
            //OGRGeometryH hPoly, hRing;
            OGRGeometryH hPoly;
            int         iRing, nRingCount;

            hPoly      = OGR_G_GetGeometryRef( hGeom, iPoly );
            nRingCount = OGR_G_GetGeometryCount( hPoly );

            for( iRing = 0; iRing < nRingCount; iRing++ )
            {
                OGRGeometryH hRing;
                int vert_count = _getGeoPtCount( hPoly, iRing, &hRing );
                int node;

                //hRing      = OGR_G_GetGeometryRef( hPoly, iRing );
                //vert_count = OGR_G_GetPointCount(hRing);

                for( node = vert_count - 1; node >= 0; node-- )
                    gv_shape_set_xyz( gv_shape, nShapeRing, node,
                                     OGR_G_GetX( hRing, node ),
                                     OGR_G_GetY( hRing, node ),
                                     OGR_G_GetZ( hRing, node ) );
                nShapeRing++;
            }
        }
    }

    else if( eType == wkbGeometryCollection
             || eType == wkbMultiLineString
             || eType == wkbMultiPoint )
    {
        int         iGeom, nGeomCount;

        nGeomCount = OGR_G_GetGeometryCount( hGeom );

        gv_shape = gv_shape_new( GVSHAPE_COLLECTION );

        for( iGeom = 0; iGeom < nGeomCount; iGeom++ )
        {
            OGRGeometryH hSubGeom = OGR_G_GetGeometryRef( hGeom, iGeom );
            GvShape *sub_shape;

            sub_shape = ogr_geometry_to_gv_shape( hSubGeom );
            gv_shape_collection_add_shape( gv_shape, sub_shape );
        }
    }

    return gv_shape;
}

/************************************************************************/
/*                      gv_shapes_from_ogr_layer()                      */
/************************************************************************/

GvData *gv_shapes_from_ogr_layer(OGRLayerH hLayer)

{
    int         field_count = 0, field_index;
    GvShapes    *shapes_data;
    GvProperties *properties;
    OGRFeatureDefnH  hDefn;
    OGRFeatureH  hFeature;

    hDefn = OGR_L_GetLayerDefn( hLayer );
    field_count = OGR_FD_GetFieldCount( hDefn );

/* -------------------------------------------------------------------- */
/*      Create shapes layer, and assign some metadata about the         */
/*      field definitions.                                              */
/* -------------------------------------------------------------------- */
    shapes_data = GV_SHAPES(gv_shapes_new());
    gv_data_set_name( GV_DATA(shapes_data), OGR_FD_GetName(hDefn) );

    properties = gv_data_get_properties( GV_DATA(shapes_data) );

    for(field_index = 0; field_index < field_count; field_index++ )
    {
        OGRFieldDefnH hField = OGR_FD_GetFieldDefn( hDefn, field_index );
        OGRFieldType eFldType;
        char      prop_value[64], prop_name[64];

        sprintf( prop_name, "_field_name_%d", field_index+1 );
        gv_properties_set( properties, prop_name,
                           OGR_Fld_GetNameRef( hField ) );

        sprintf( prop_name, "_field_width_%d", field_index+1 );
        sprintf( prop_value, "%d", OGR_Fld_GetWidth( hField ) );
        gv_properties_set( properties, prop_name, prop_value );

        eFldType = OGR_Fld_GetType( hField );
        if( eFldType == OFTReal )
        {
            sprintf( prop_name, "_field_precision_%d", field_index+1 );
            sprintf( prop_value, "%d", OGR_Fld_GetPrecision(hField) );
            gv_properties_set( properties, prop_name, prop_value );
        }

        sprintf( prop_name, "_field_type_%d", field_index+1 );
        if( eFldType == OFTInteger )
            gv_properties_set( properties, prop_name, "integer" );
        else if( eFldType == OFTReal )
            gv_properties_set( properties, prop_name, "float" );
        else
            gv_properties_set( properties, prop_name, "string" );
    }

/* -------------------------------------------------------------------- */
/*      Copy all the shapes, and their attributes.                      */
/* -------------------------------------------------------------------- */
    OGR_L_ResetReading( hLayer );
    while( (hFeature = OGR_L_GetNextFeature(hLayer)) != NULL )
    {
        GvShape    *gv_shape = NULL;

        gv_shape = ogr_geometry_to_gv_shape( OGR_F_GetGeometryRef(hFeature) );

        if( gv_shape != NULL )
        {
            properties = gv_shape_get_properties( gv_shape );

            for( field_index = 0; field_index < field_count; field_index++ )
            {
                if( OGR_F_IsFieldSet( hFeature, field_index ) )
                {
                    gv_properties_set( properties,
                           OGR_Fld_GetNameRef(
                               OGR_F_GetFieldDefnRef(hFeature,field_index) ),
                           OGR_F_GetFieldAsString( hFeature, field_index ) );
                }
            }

            if( OGR_F_GetStyleString(hFeature) != NULL )
                gv_properties_set( properties, "_gv_ogrfs",
                                   OGR_F_GetStyleString( hFeature ) );
        }

        if( gv_shape != NULL )
            gv_shapes_add_shape( shapes_data, gv_shape );

        OGR_F_Destroy( hFeature );
    }

    return GV_DATA(shapes_data);
}

/************************************************************************/
/*                         gv_shapes_from_ogr()                         */
/************************************************************************/

GvData *gv_shapes_from_ogr(const char *filename, int iLayer)

{
    OGRLayerH        hLayer;
    OGRDataSourceH hDS;
    GvData *psData;
    OGRSFDriverH     hDriver;
    OGRSpatialReferenceH hSR;   //spatial reference for this layer
    char* projection_wkt;

/* -------------------------------------------------------------------- */
/*      Open the OGR dataset.                                           */
/* -------------------------------------------------------------------- */
    OGRRegisterAll();

    hDS = OGROpen( filename, FALSE, &hDriver );
    if( hDS == NULL )
        return NULL;

    if( iLayer < 0 || iLayer >= OGR_DS_GetLayerCount(hDS) )
    {
        OGR_DS_Destroy( hDS );
        return NULL;
    }

    hLayer =  OGR_DS_GetLayer( hDS, iLayer );

/* -------------------------------------------------------------------- */
/*      Actually read the shapes.                                       */
/* -------------------------------------------------------------------- */
    psData = gv_shapes_from_ogr_layer( hLayer );

    if( psData != NULL )
    {
        gv_data_set_property( psData, "_filename", filename );
        gv_data_set_property( psData, "_ogr_driver_name",
                              OGR_Dr_GetName(hDriver) );

        //save the ogr data source.  Cleanup is now done in finalize.
        GV_SHAPES(psData)->hOGRds = hDS;

        //extract the wkt for setting the data projection
        hSR = OGR_L_GetSpatialRef(hLayer);
        if (hSR != NULL)
        {
            OSRExportToWkt(hSR, &projection_wkt);
            gv_data_set_projection(psData, projection_wkt);
        }
    }

    return psData;
}

