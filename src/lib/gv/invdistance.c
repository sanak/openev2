/******************************************************************************
 * $Id: invdistance.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  CIETMAP / OpenEV
 * Purpose:  Weighted Inverse Distance Interpolator (point to raster)
 *           This implementation is independent of OpenEV, but does depend
 *           directly on GDAL.
 * Author:   Frank Warmerdam, warmerda@home.com
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
 * $Log: invdistance.c,v $
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
 * Revision 1.3  2001/04/22 17:33:24  pgs
 * changed WIDInterpolate to take variable exponent for d
 *
 * Revision 1.2  2001/02/05 14:45:09  warmerda
 * make padfWeight optional
 *
 * Revision 1.1  2000/09/12 19:17:46  warmerda
 * New
 *
 */

#include "invdistance.h"
#include "cpl_conv.h"

/************************************************************************/
/*                           WIDInterpolate()                           */
/************************************************************************/

/**
 * Weighted Inverse Distance Interpolation
 *
 * This algorithm interpolates the pixel values of a raster (in a GDAL
 * band) using a simple inverse distance interpolator with an additional
 * per point weighting factor.  The algorithm support GDALProcessFunc style
 * progress reporting, and user termination support.
 *
 * @param nPoints the number of input sample points (entries in the padfX,
 * padfY, padfValue and padfWeight arrays).
 *
 * @param padfX the X locations of sample points in raster pixel/line
 * coordinates.
 *
 * @param padfY the Y locations of sample points in raster pixel/line
 * coordinates.
 *
 * @param padfValue the data value for each sample point.
 *
 * @param padfWeight a relative weighting for each sample point.  Use NULL
 * to default to equal weighting (implies all are 1.0).
 *
 * @param hBand the output GDAL band to which results should be written.
 *
 * @param fExponent the exponent to apply to the distance calculate in the
 * weighting formula
 *
 * @param pfnProgress progress function (see GDALDummyProgress() for more
 * information), use NULL if no progress function desired.
 *
 * @param pCBData callback data passed to pfnProgress.
 *
 * @return A CPL error number on failure, or CPLE_None on success.  Note that
 * a user interrupt will result in CPLE_UserInterrupt.
 */

int WIDInterpolate( int nPoints, double *padfX, double *padfY,
                    double *padfValue, double *padfWeight,
                    GDALRasterBandH hBand, double fExponent,
                    GDALProgressFunc pfnProgress, void * pCBData )

{
    int		nXSize, nYSize, nError = CPLE_None, iY, iPoint;
    float       *pafScanline;
    double      *padfDeltaYSquared;

    if( pfnProgress == NULL )
        pfnProgress = GDALDummyProgress;

    nXSize = GDALGetRasterBandXSize( hBand );
    nYSize = GDALGetRasterBandYSize( hBand );

    padfDeltaYSquared = (double *) CPLMalloc(sizeof(double) * nPoints);
    pafScanline = (float *) CPLMalloc(sizeof(float) * nXSize);

    for( iY = 0; iY < nYSize; iY++ )
    {
        int	iX;

        if( !pfnProgress( iY / (double) nYSize, NULL, pCBData ) )
        {
            nError = CPLE_UserInterrupt;
            break;
        }

        /* Precompute DeltaY Squared for point.  It will remain constant
           over the scanline. */
        for( iPoint = 0; iPoint < nPoints; iPoint++ )
        {
            padfDeltaYSquared[iPoint] =
                (padfY[iPoint]-(double)iY) * (padfY[iPoint]-(double)iY);
        }

        for( iX = 0; iX < nXSize; iX++ )
        {
            double	dfNumerator=0.0, dfDenominator = 0.0;
            double      dfX = iX;

            for( iPoint = 0; iPoint < nPoints; iPoint++ )
            {
                double dfDistSquared, dfDeltaX;
                double dfWeight;

                dfDeltaX = (padfX[iPoint] - dfX);

                dfDistSquared = dfDeltaX*dfDeltaX + padfDeltaYSquared[iPoint];

                if (fExponent != 2.0)
                {
					//if the exponent is not 2, use the exponent / 2
					//as the distance is already squared
					dfDistSquared = pow(dfDistSquared, fExponent / 2.0);
				}

                if( padfWeight == NULL )
                    dfWeight = 1.0 / dfDistSquared;
                else
                    dfWeight = padfWeight[iPoint] / dfDistSquared;

                dfDenominator += dfWeight;
                dfNumerator += dfWeight * padfValue[iPoint];
            }

            pafScanline[iX] = dfNumerator / dfDenominator;
        }

        GDALRasterIO( hBand, GF_Write, 0, iY, nXSize, 1,
                      pafScanline, nXSize, 1, GDT_Float32, 0, 0 );
    }

    pfnProgress( 1.0, NULL, pCBData );

    CPLFree( pafScanline );
    CPLFree( padfDeltaYSquared );

    return nError;
}
