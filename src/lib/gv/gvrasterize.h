/******************************************************************************
 * $Id: gvrasterize.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  CIETMAP / OpenEV
 * Purpose:  Vector rasterization code (initially GvAreaShapes to GDAL raster).
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
 * $Log: gvrasterize.h,v $
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
 * Revision 1.3  2004/11/19 23:59:37  gmwalter
 * Check in Aude's rasterization updates.
 *
 * Revision 1.2  2001/03/29 14:59:56  warmerda
 * added fill_short flag to control handling of slivers
 *
 * Revision 1.1  2000/09/15 01:29:04  warmerda
 * New
 *
 */

#ifndef __GVRASTERIZE_H__
#define __GVRASTERIZE_H__

#include "gdal.h"
#include "gvraster.h"
#include "gvshapes.h"

/* -------------------------------------------------------------------- */
/*      Low level rasterizer API.                                       */
/* -------------------------------------------------------------------- */
typedef struct {
    int x, y;
} llPoint;

typedef struct {
    double x, y;
} dllPoint;


typedef void (*llScanlineFunc)(void *pCBData, int nY, int nXStart, int nXEnd);


void llImageFilledPolygon(int nRasterXSize, int nRasterYSize, 
                       int nPartCount, int * panPartSize, llPoint * pasPoints,
                       int bFillShort,
                       llScanlineFunc pfnScanlineFunc, void * pCBData);
                                            
void dllImageFilledPolygon(int nRasterXSize, int nRasterYSize, 
                          int nPartCount, int *panPartSize, dllPoint *p,
                          llScanlineFunc pfnScanlineFunc, void *pCBData, 
                          int method);


/* -------------------------------------------------------------------- */
/*      High level API - GvShapes burned into GDAL raster.              */
/* -------------------------------------------------------------------- */
int gv_raster_rasterize_shapes(GvRaster *raster, 
                                int shape_count, GvShape **shapes,
                                double dfBurnValue, int bFillShort);

#endif /* ndef __GVRASTERIZE_H__ */
