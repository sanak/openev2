/******************************************************************************
 * $Id: llrasterize.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  CIETMAP / OpenEV
 * Purpose:  Vector rasterization code (initially GvAreaShapes to GDAL raster).
 * Author:   Frank Warmerdam, warmerda@home.com
 *
 ******************************************************************************
 * Copyright (c) 2000, Frank Warmerdam <warmerda@home.com>
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
 * $Log: llrasterize.c,v $
 * Revision 1.1.1.1  2005/04/18 16:38:34  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:27  uid1026
 *
 * Imported sources
 *
 * Revision 1.9  2004/11/23 06:14:55  warmerda
 * avoid use of rint() - not portable
 *
 * Revision 1.8  2004/11/19 23:59:37  gmwalter
 * Check in Aude's rasterization updates.
 *
 * Revision 1.7  2004/10/20 18:33:55  warmerda
 * Cietmap bug 3233: fix screwup in last scanline under some circumstances.
 *
 * Revision 1.6  2001/08/08 04:26:43  warmerda
 * fixed leak of polyInts array
 *
 * Revision 1.5  2001/03/29 14:59:56  warmerda
 * added fill_short flag to control handling of slivers
 *
 * Revision 1.4  2000/09/27 14:04:31  warmerda
 * removed debugging printf
 *
 * Revision 1.3  2000/09/21 02:54:50  warmerda
 * removed debug stuff
 *
 * Revision 1.2  2000/09/15 01:57:43  warmerda
 * added copyright header
 *
 */

/*
 * NOTE: This code was adapted from the gdImageFilledPolygon() function 
 * in libgd.  
 * 
 * http://www.boutell.com/gd/
 */

#include <stdlib.h>
#include "gvrasterize.h"

static int llCompareInt(const void *a, const void *b)
{
	return (*(const int *)a) - (*(const int *)b);
}

/************************************************************************/
/*                        llImageFilledPolygon()                        */
/*                                                                      */
/*      Perform scanline conversion of the passed multi-ring            */
/*      polygon.  Note the polygon does not need to be explicitly       */
/*      closed.  The scanline function will be called with              */
/*      horizontal scanline chunks which may not be entirely            */
/*      contained within the valid raster area (in the X                */
/*      direction).                                                     */
/************************************************************************/

/* KNOWN BUGS:                                                               */

/*  1) Because the nodes" coordinates of the polygons are casted to integers */
/*  before being passed to the routine, the computation of intersections     */
/*  between raster lines and the polygon"s segments is not accurate and may  */
/*  cause pixels to be considered as inside the shape when they don"t even   */
/*  touch it.                                                                */
/*  2) Pixels falling at the bottom of a shape on an horizontal segment are  */
/*  never taken into account.                                                */
/*  3) When a polygon lies partly outside the extent of the raster file      */
/*  beyond the last line,  and one of its node falls                         */
/*  on the last line, wrong pixels are considered outside.                   */
/*  4) Cones facing down (\./ ) are not filled on the last line.             */
/*  5) Possible core dumps.                                                  */

void llImageFilledPolygon(int nRasterXSize, int nRasterYSize, 
                          int nPartCount, int *panPartSize, llPoint *p,
                          int bFillShort,
                          llScanlineFunc pfnScanlineFunc, void *pCBData )
{
    int i;
    int y;
    int miny, maxy;
    int x1, y1;
    int x2, y2;
    int ind1, ind2;
    int ints, n, part;
    int *polyInts, polyAllocated;

    if (!nPartCount) {
        return;
    }

    n = 0;
    for( part = 0; part < nPartCount; part++ )
        n += panPartSize[part];

    polyInts = (int *) malloc(sizeof(int) * n);
    polyAllocated = n;

    miny = p[0].y;
    maxy = p[0].y;
    for (i=1; (i < n); i++) {
        if (p[i].y < miny) {
            miny = p[i].y;
        }
        if (p[i].y > maxy) {
            maxy = p[i].y;
        }
    }
    if( miny < 0 )
        miny = 0;
    if( maxy >= nRasterYSize )
        maxy = nRasterYSize-1;

    /* Fix in 1.3: count a vertex only once */
    for (y=miny; (y <= maxy); y++) {
        int	partoffset = 0;

        part = 0;
        ints = 0;

        for (i=0; (i < n); i++) {

            if( i == partoffset + panPartSize[part] ) {
                partoffset += panPartSize[part];
                part++;
            }

            if( i == partoffset ) {
                ind1 = partoffset + panPartSize[part] - 1;
                ind2 = partoffset;
            } else {
                ind1 = i-1;
                ind2 = i;
            }
            
            y1 = p[ind1].y;
            y2 = p[ind2].y;
            if( (y1 < y && y2 < y) || (y1 > y && y2 > y) )
                continue;

            if (y1 < y2) {
                x1 = p[ind1].x;
                x2 = p[ind2].x;
            } else if (y1 > y2) {
                y2 = p[ind1].y;
                y1 = p[ind2].y;
                x2 = p[ind1].x;
                x1 = p[ind2].x;
            } else {
                /* skip horizontal segments */
                continue;
            }

            /* 
            ** the commented out test was intended to ensure the last 
            ** scanline of the polygon gets filled in properly.  Otherwise
            ** it would often be dropped.  But it doesn't always work, and
            ** much worse is that sometimes it results in a bogus extra
            ** transition that screws everything up.
            ** See CIETMap 3233 at DMSG. 
            */
            if( y < y2 /* || y2 == maxy */ )
                polyInts[ints++] = (y-y1) * (x2-x1) / (y2-y1) + x1;
        }

        /* 
         * It would be more efficient to do this inline, to avoid 
         * a function call for each comparison.
         */
        qsort(polyInts, ints, sizeof(int), llCompareInt);

        for (i=0; (i < (ints)); i+=2) {
            if( !bFillShort && polyInts[i] == polyInts[i+1] )
                continue;

            if( polyInts[i] < nRasterXSize && polyInts[i+1] >= 0 )
                pfnScanlineFunc( pCBData, y, polyInts[i], polyInts[i+1] );
        }
    }

    free( polyInts );
}

/************************************************************************/
/*                        llImageFilledPolygon()                        */
/*                                                                      */
/*      Perform scanline conversion of the passed multi-ring            */
/*      polygon.  Note the polygon does not need to be explicitly       */
/*      closed.  The scanline function will be called with              */
/*      horizontal scanline chunks which may not be entirely            */
/*      contained within the valid raster area (in the X                */
/*      direction).                                                     */

/*      NEW: Nodes' coordinate are kept as double  in order             */
/*      to compute accurately the intersections with the lines          */
      
/*         Two methods for determining the border pixels:               */

/*            1) method = 0                                             */  
/*            Inherits algorithm from version above but with several bugs */
/*              fixed except for the cone facing down.                  */
/*               A pixel on which a line intersects a segment of a      */
/*            polygon will always be considered as inside the shape.    */
/*            Note that we only compute intersections with lines that   */
/*            passes through the middle of a pixel (line coord = 0.5,   */
/*            1.5, 2.5, etc.) */
           
/*            2) method = 1:                                            */
/*              A pixel is considered inside a polygon if its center   */
/*            falls inside the polygon. This is somehow more robust unless */
/*            the nodes are placed in the center of the pixels in which */
/*            case, due to numerical inaccuracies, it's hard to predict */
/*            if the pixel will be considered inside or outside the shape.*/

/************************************************************************/

void ImageFilledPolygon0(int nRasterXSize, int nRasterYSize, 
                          int nPartCount, int *panPartSize, dllPoint *p,
                         llScanlineFunc pfnScanlineFunc, void *pCBData);


void ImageFilledPolygon1(int nRasterXSize, int nRasterYSize, 
                         int nPartCount, int *panPartSize, dllPoint *p,
                         llScanlineFunc pfnScanlineFunc, void *pCBData);

void dllImageFilledPolygon(int nRasterXSize, int nRasterYSize, 
                           int nPartCount, int *panPartSize, dllPoint *p,
                           llScanlineFunc pfnScanlineFunc, void *pCBData, 
                           int method)
{
    
    if ( method == 0 )
        ImageFilledPolygon0(nRasterXSize,
                            nRasterYSize,
                            nPartCount, 
                            panPartSize, 
                            p,
                            pfnScanlineFunc, 
                            pCBData);
    
    else if (method == 1)
        ImageFilledPolygon1(nRasterXSize,
                            nRasterYSize,
                            nPartCount, 
                            panPartSize, 
                            p,
                            pfnScanlineFunc, 
                            pCBData);

    return;
    
}




/***********************************************************************
Method one
----------
Known bugs:
Cones facing down (\./ ) are not filled on the last line.
************************************************************************/
void ImageFilledPolygon0(int nRasterXSize, int nRasterYSize, 
                          int nPartCount, int *panPartSize, dllPoint *p,
                         llScanlineFunc pfnScanlineFunc, void *pCBData)
{
    int i;
    int y, y1, y2;
    int miny, maxy,minx,maxx;
    double dminy, dmaxy ;
    double dx1, dy1;
    double dx2, dy2;
    double dy;
    double intersect, slope;
    

    int ind1, ind2;
    int ints, n, part;
    int *polyInts, polyAllocated;

  
    int horizontal_x1, horizontal_x2;

    int inter, left, right;
    

    if (!nPartCount) {
        return;
    }

    n = 0;
    for( part = 0; part < nPartCount; part++ )
        n += panPartSize[part];
    
    polyInts = (int *) malloc(sizeof(int) * n);
    polyAllocated = n;
    
    dminy = p[0].y;
    dmaxy = p[0].y;
    for (i=1; (i < n); i++) {

        if (p[i].y < dminy) {
            dminy = p[i].y;
        }
        if (p[i].y > dmaxy) {
            dmaxy = p[i].y;
        }
    }
    miny = (int) dminy;
    maxy = (int) dmaxy;
    

    if( miny < 0 )
        miny = 0;
    if( maxy >= nRasterYSize )
        maxy = nRasterYSize-1;
   
    
    minx = 0;
    maxx = nRasterXSize - 1;
    
    
    /* Fix in 1.3: count a vertex only once */
    for (y=miny; y <= maxy; y++) {
        int	partoffset = 0;

        dy = y +0.5; /* center height of line*/
         

        part = 0;
        ints = 0;

        /*Initialize polyInts, otherwise it can sometimes causes a seg fault */
        for (i=0; (i < n); i++) {
            polyInts[i] = -1;
        }
        

        for (i=0; (i < n); i++) {
        
            
            if( i == partoffset + panPartSize[part] ) {
                partoffset += panPartSize[part];
                part++;
            }

            if( i == partoffset ) {
                ind1 = partoffset + panPartSize[part] - 1;
                ind2 = partoffset;
            } else {
                ind1 = i-1;
                ind2 = i;
            }
	    
            
            dy1 = p[ind1].y;
            dy2 = p[ind2].y;
            

            y1 = (int) dy1;
            y2 = (int) dy2;

            
            if( ( y1 < y && y2 < y) || (y1 > y && y2 > y) )
                continue;


            if (y1 < y2) {
                dx1 = p[ind1].x;
                dx2 = p[ind2].x;
            } else if (y1 > y2) {
                dy2 = p[ind1].y;
                dy1 = p[ind2].y;
                y2 = (int) dy2;
                y1 = (int) dy1;
                dx2 = p[ind1].x;
                dx1 = p[ind2].x;
            } else /*  (y1 == y2) */
	    {
                
                /*AE: DO NOT skip bottom horizontal segments 
		  -Fill them separately- 
		  They are not taken into account twice.*/
		if (p[ind1].x > p[ind2].x)
		{
		    horizontal_x1 = (int) p[ind2].x;
		    horizontal_x2 = (int) p[ind1].x;
		
                    if  ( (horizontal_x1 >  maxx) ||  (horizontal_x2 < minx) )
                        continue;

		    /*fill the horizontal segment (separately from the rest)*/
		    pfnScanlineFunc( pCBData, y, horizontal_x1, horizontal_x2);
		    continue;
		}
		else /*skip top horizontal segments (they are already filled in the regular loop)*/
		    continue;

	    }

           
            if( y < y2 )
            {

                intersect = (dy-dy1) * (dx2-dx1) / (dy2-dy1) + dx1;
                slope = (dx2-dx1) / (dy2-dy1);
                
                /*the intersection must fall in the same x-range of the segment*/
                inter = (int) intersect;
                if (dx1 <= dx2)
                {
                    left = (int)dx1;
                    right = (int)dx2;
                }
                else {
                    left = (int)dx2;
                    right = (int)dx1; 
                }
                
                       
                if (inter < left)
                    inter  = left;
                else if ( inter > right )
                    inter = right;
                
                polyInts[ints++] = inter;

	    }
	}
        


        /*Sort intersections*/
        qsort(polyInts, ints, sizeof(int), llCompareInt);


        for (i=0; (i < (ints)); i+=2) {
            
            if( polyInts[i] <= maxx && polyInts[i+1] >= minx )
            {
                pfnScanlineFunc( pCBData, y, polyInts[i], polyInts[i+1]);
	    }
            
        }
    }

    free( polyInts );
}

/*************************************************************************
Method 2:
=========
No known bug
*************************************************************************/

void ImageFilledPolygon1(int nRasterXSize, int nRasterYSize, 
                         int nPartCount, int *panPartSize, dllPoint *p,
                         llScanlineFunc pfnScanlineFunc, void *pCBData)
{
    int i;
    int y;
    int miny, maxy,minx,maxx;
    double dminy, dmaxy;
    double dx1, dy1;
    double dx2, dy2;
    double dy;
    double intersect;
    

    int ind1, ind2;
    int ints, n, part;
    int *polyInts, polyAllocated;

  
    int horizontal_x1, horizontal_x2;


    if (!nPartCount) {
        return;
    }

    n = 0;
    for( part = 0; part < nPartCount; part++ )
        n += panPartSize[part];
    
    polyInts = (int *) malloc(sizeof(int) * n);
    polyAllocated = n;
    
    dminy = p[0].y;
    dmaxy = p[0].y;
    for (i=1; (i < n); i++) {

        if (p[i].y < dminy) {
            dminy = p[i].y;
        }
        if (p[i].y > dmaxy) {
            dmaxy = p[i].y;
        }
    }
    miny = (int) dminy;
    maxy = (int) dmaxy;
    
    
    if( miny < 0 )
        miny = 0;
    if( maxy >= nRasterYSize )
        maxy = nRasterYSize-1;
   
    
    minx = 0;
    maxx = nRasterXSize - 1;

    /* Fix in 1.3: count a vertex only once */
    for (y=miny; y <= maxy; y++) {
        int	partoffset = 0;

        dy = y +0.5; /* center height of line*/
         

        part = 0;
        ints = 0;


        /*Initialize polyInts, otherwise it can sometimes causes a seg fault */
        for (i=0; (i < n); i++) {
            polyInts[i] = -1;
        }


        for (i=0; (i < n); i++) {
        
            
            if( i == partoffset + panPartSize[part] ) {
                partoffset += panPartSize[part];
                part++;
            }

            if( i == partoffset ) {
                ind1 = partoffset + panPartSize[part] - 1;
                ind2 = partoffset;
            } else {
                ind1 = i-1;
                ind2 = i;
            }
	    

            dy1 = p[ind1].y;
            dy2 = p[ind2].y;
            

            if( (dy1 < dy && dy2 < dy) || (dy1 > dy && dy2 > dy) )
                continue;

            


            if (dy1 < dy2) {
                dx1 = p[ind1].x;
                dx2 = p[ind2].x;
            } else if (dy1 > dy2) {
                dy2 = p[ind1].y;
                dy1 = p[ind2].y;
                dx2 = p[ind1].x;
                dx1 = p[ind2].x;
            } else /* if (fabs(dy1-dy2)< 1.e-6) */
	    {
                
                /*AE: DO NOT skip bottom horizontal segments 
		  -Fill them separately- 
		  They are not taken into account twice.*/
		if (p[ind1].x > p[ind2].x)
		{
		    horizontal_x1 = (int) floor(p[ind2].x+0.5);
		    horizontal_x2 = (int) floor(p[ind1].x+0.5);
		
                    if  ( (horizontal_x1 >  maxx) ||  (horizontal_x2 < minx) )
                        continue;


		    /*fill the horizontal segment (separately from the rest)*/
		    pfnScanlineFunc( pCBData, y, horizontal_x1, horizontal_x2 - 1 );
		    continue;
		}
		else /*skip top horizontal segments (they are already filled in the regular loop)*/
		    continue;

	    }

           

            if(( dy < dy2 ) && (dy >= dy1))
            {
                
                intersect = (dy-dy1) * (dx2-dx1) / (dy2-dy1) + dx1;

		polyInts[ints++] = (int) floor(intersect+0.5);
	    }
	}



        /* 
         * It would be more efficient to do this inline, to avoid 
         * a function call for each comparison.
         */
        qsort(polyInts, ints, sizeof(int), llCompareInt);


        for (i=0; (i < (ints)); i+=2) {

            if( polyInts[i] <= maxx && polyInts[i+1] >= minx )
            {
                pfnScanlineFunc( pCBData, y, polyInts[i], polyInts[i+1] - 1 );
                
	    }
            
        }
    }


    free( polyInts );
}


