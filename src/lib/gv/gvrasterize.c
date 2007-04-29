/******************************************************************************
 * $Id$
 *
 * Project:  CIETMAP / OpenEV
 * Purpose:  Vector rasterization code high level API (initially GvAreaShapes
 *           to GDAL raster).
 * Author:   Frank Warmerdam, warmerda@home.com
 * Maintainer: Mario Beauchamp, starged@gmail.com
 *
 ******************************************************************************
 * Copyright (c) 2000, Frank Warmerdam
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

#include "gvrasterize.h"
#include "cpl_conv.h"

typedef struct {
    unsigned char * pabyChunkBuf;
    int nXSize;
    int nYSize;
    GDALDataType eType;
    double dfBurnValue;
} GvRasterizeInfo;

/************************************************************************/
/*                           gvBurnScanline()                           */
/************************************************************************/

void gvBurnScanline( void *pCBData, int nY, int nXStart, int nXEnd )

{
    GvRasterizeInfo *psInfo = (GvRasterizeInfo *) pCBData;

    CPLAssert( nY >= 0 && nY < psInfo->nYSize );
    CPLAssert( nXStart <= nXEnd );
    CPLAssert( nXStart < psInfo->nXSize );

   /*  CPLAssert( nXEnd > 0 ); */
    CPLAssert( nXEnd >= 0 );

    if( nXStart < 0 )
        nXStart = 0;
    if( nXEnd >= psInfo->nXSize )
        nXEnd = psInfo->nXSize - 1;

    if( psInfo->eType == GDT_Byte )
    {
        unsigned char *pabyInsert;
        unsigned char nBurnValue = (unsigned char) psInfo->dfBurnValue;

        pabyInsert = psInfo->pabyChunkBuf + nY * psInfo->nXSize + nXStart;
        memset( pabyInsert, nBurnValue, nXEnd - nXStart + 1 );
    }
    else
    {
        int     nPixels = nXEnd - nXStart + 1;
        float   *pafInsert;

        pafInsert = ((float *) psInfo->pabyChunkBuf) 
            + nY * psInfo->nXSize + nXStart;

        while( nPixels-- > 0 )
            *(pafInsert++) = psInfo->dfBurnValue;
    }
}


/************************************************************************/
/*                       gv_rasterize_one_shape()                       */
/************************************************************************/
static void 
gv_rasterize_one_shape( unsigned char * pabyChunkBuf, int nYOff, int nYSize,
                        GDALDataType eType, GvRaster * raster, 
                        GvShape * shape, double dfBurnValue, int bFillShort )

{
    int   nParts, *panPartSize, nPoints, i;
    llPoint *pasPoints;
    GvRasterizeInfo sInfo;

    sInfo.nXSize = raster->width;
    sInfo.nYSize = nYSize;
    sInfo.pabyChunkBuf = pabyChunkBuf;
    sInfo.eType = eType;
    sInfo.dfBurnValue = dfBurnValue;

/* -------------------------------------------------------------------- */
/*      Prepare parts, and points arrays                                */
/* -------------------------------------------------------------------- */
    nParts = gv_shape_get_rings( shape );
    panPartSize = (int *) CPLMalloc(nParts * sizeof(int));

    nPoints = 0;
    for( i = 0; i < nParts; i++ )
    {
        panPartSize[i] = gv_shape_get_nodes( shape, i );
        nPoints += panPartSize[i];
    }

    pasPoints = (llPoint *) CPLMalloc(sizeof(llPoint) * nPoints);

/* -------------------------------------------------------------------- */
/*      Transform points, taking into account our chunk buffer offset.  */
/* -------------------------------------------------------------------- */
    nPoints = 0;
    for( i = 0; i < nParts; i++ )
    {
        int    node, part_points = 0;

        for( node = 0; node < panPartSize[i]; node++ )
        {
            double      dfX, dfY, dfZ;
            int         nX, nY;

            dfX = gv_shape_get_x( shape, i, node );
            dfY = gv_shape_get_y( shape, i, node );
            dfZ = 0.0;
            gv_raster_georef_to_pixel( raster, &dfX, &dfY, &dfZ );

            nX = (int) dfX;
            nY = ((int) dfY) - nYOff;

            if( node == 0
                || pasPoints[nPoints-1].x != nX 
                || pasPoints[nPoints-1].y != nY )
            {
                pasPoints[nPoints].x = nX;
                pasPoints[nPoints].y = nY;
                nPoints++;
                part_points++;
            }
        }
        panPartSize[i] = part_points;
    }

/* -------------------------------------------------------------------- */
/*      Perform the rasterization.                                      */
/* -------------------------------------------------------------------- */
    llImageFilledPolygon( raster->width, nYSize, 
                          nParts, panPartSize, pasPoints, bFillShort,
                          gvBurnScanline, &sInfo );

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    CPLFree( pasPoints );
    CPLFree( panPartSize );
}


/************************************************************************/
/*                       gv_rasterize_one_shape()                       */
/************************************************************************/
static void 
gv_rasterize_new_one_shape( unsigned char * pabyChunkBuf, int nYOff, int nYSize,
                            GDALDataType eType, GvRaster * raster, 
                            GvShape * shape, double dfBurnValue, int method )

{
    int   nParts, *panPartSize, nPoints, i;
    dllPoint *pasPoints;
    GvRasterizeInfo sInfo;

    sInfo.nXSize = raster->width;
    sInfo.nYSize = nYSize;
    sInfo.pabyChunkBuf = pabyChunkBuf;
    sInfo.eType = eType;
    sInfo.dfBurnValue = dfBurnValue;

/* -------------------------------------------------------------------- */
/*      Prepare parts, and points arrays                                */
/* -------------------------------------------------------------------- */
    nParts = gv_shape_get_rings( shape );
    panPartSize = (int *) CPLMalloc(nParts * sizeof(int));

    nPoints = 0;
    for( i = 0; i < nParts; i++ )
    {
        panPartSize[i] = gv_shape_get_nodes( shape, i );
        nPoints += panPartSize[i];
    }

    pasPoints = (dllPoint *) CPLMalloc(sizeof(dllPoint) * nPoints);

/* -------------------------------------------------------------------- */
/*      Transform points, taking into account our chunk buffer offset.  */
/* -------------------------------------------------------------------- */
    nPoints = 0;
    for( i = 0; i < nParts; i++ )
    {
        int    node, part_points = 0;

        for( node = 0; node < panPartSize[i]; node++ )
        {
            double      dfX, dfY, dfZ;


            dfX = gv_shape_get_x( shape, i, node );
            dfY = gv_shape_get_y( shape, i, node );
            dfZ = 0.0;
            gv_raster_georef_to_pixel( raster, &dfX, &dfY, &dfZ );


            if( node == 0
                || pasPoints[nPoints-1].x != dfX 
                || pasPoints[nPoints-1].y != dfY )
            {
                pasPoints[nPoints].x = dfX;
                pasPoints[nPoints].y = dfY - nYOff;
                nPoints++;
                part_points++;
            }
        }
        panPartSize[i] = part_points;
    }

/* -------------------------------------------------------------------- */
/*      Perform the rasterization.                                      */
/* -------------------------------------------------------------------- */
    dllImageFilledPolygon( raster->width, nYSize, 
                           nParts, panPartSize, pasPoints, 
                           gvBurnScanline, &sInfo, method );

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    CPLFree( pasPoints );
    CPLFree( panPartSize );
}


/************************************************************************/
/*                     gv_raster_rasterize_shapes()                     */
/************************************************************************/

int gv_raster_rasterize_shapes( GvRaster *raster, 
                                int shape_count, GvShape **shapes,
                                double dfBurnValue, int bFillShort )

{
    GDALDataType   eType;
    int            nYChunkSize, nScanlineBytes;
    unsigned char *pabyChunkBuf;
    int            iY;
    GvRasterChangeInfo change_info;

/* -------------------------------------------------------------------- */
/*      Establish a chunksize to operate on.  The larger the chunk      */
/*      size the less times we need to make a pass through all the      */
/*      shapes.                                                         */
/* -------------------------------------------------------------------- */
    if( raster->gdal_type == GDT_Byte )
        eType = GDT_Byte;
    else
        eType = GDT_Float32;

    nScanlineBytes = raster->width * (GDALGetDataTypeSize(eType)/8);
    nYChunkSize = 10000000 / nScanlineBytes;
    if( nYChunkSize > raster->height )
        nYChunkSize = raster->height;

    pabyChunkBuf = (unsigned char *) VSIMalloc(nYChunkSize * nScanlineBytes);
    if( pabyChunkBuf == NULL )
    {
        CPLError( CE_Failure, CPLE_OutOfMemory, 
                  "Unable to allocate rasterization buffer." );
        return FALSE;
    }

/* ==================================================================== */
/*      Loop over image in designated chunks.                           */
/* ==================================================================== */
    for( iY = 0; iY < raster->height; iY += nYChunkSize )
    {
        int     nThisYChunkSize;
        int     iShape;

        nThisYChunkSize = nYChunkSize;
        if( nThisYChunkSize + iY > raster->height )
            nThisYChunkSize = raster->height - iY;

        GDALRasterIO( raster->gdal_band, GF_Read, 
                      0, iY, raster->width, nThisYChunkSize, 
                      pabyChunkBuf, raster->width, nThisYChunkSize, eType, 
                      0, 0 );

        for( iShape = 0; iShape < shape_count; iShape++ )
        {

            if (bFillShort < 2 )
                gv_rasterize_one_shape( pabyChunkBuf, iY, nThisYChunkSize,
                                        eType, raster, 
                                        shapes[iShape], dfBurnValue, bFillShort );
            else
                gv_rasterize_new_one_shape( pabyChunkBuf, iY, nThisYChunkSize,
                                            eType, raster, 
                                            shapes[iShape], dfBurnValue, bFillShort - 2 );
        }

        GDALRasterIO( raster->gdal_band, GF_Write, 
                      0, iY, raster->width, nThisYChunkSize, 
                      pabyChunkBuf, raster->width, nThisYChunkSize, eType, 
                      0, 0 );
    }

    VSIFree( pabyChunkBuf );

/* -------------------------------------------------------------------- */
/*      Invalidate the raster to force a reload.                        */
/* -------------------------------------------------------------------- */
    change_info.change_type = GV_CHANGE_REPLACE;
    change_info.x_off = 0;
    change_info.y_off = 0;
    change_info.width = raster->width;
    change_info.height = raster->height;

    gv_data_changed( GV_DATA(raster), &change_info );

    return TRUE;
}
