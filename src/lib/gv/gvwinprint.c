/******************************************************************************
 * $Id: gvwinprint.c,v 1.2 2005/04/21 19:35:45 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  MS Windows Print Support
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
 * $Log: gvwinprint.c,v $
 * Revision 1.2  2005/04/21 19:35:45  uid1026
 * Added unix compilation unit so multi-platform make works
 *
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
 * Revision 1.4  2001/05/01 19:21:51  warmerda
 * include cpl_error.h
 *
 * Revision 1.3  2001/05/01 19:20:19  warmerda
 * reimplement using StretchDIBit(), avoid upsidedown DIB
 *
 * Revision 1.2  2001/04/27 20:50:49  warmerda
 * interim update with everything in one big buffer
 *
 * Revision 1.1  2000/08/07 17:17:12  warmerda
 * New
 *
 */

#include <stdio.h>

#ifdef _WIN32

#include <windows.h>
#include "gvviewarea.h"
#include "cpl_error.h"

static  PRINTDLG prDlg;
static  PAGESETUPDLG psDlg;
static  HGLOBAL  hDevMode = NULL;

void gv_view_area_page_setup()

{
    printf( "gv_view_area_page_setup\n" );
    
    psDlg.Flags = 0;
    psDlg.hwndOwner = NULL;
    psDlg.hDevNames = NULL;
    psDlg.lStructSize = sizeof (PAGESETUPDLG);
    psDlg.hDevMode = hDevMode;
    
    if (!PageSetupDlg (&psDlg))
    {
        if (CommDlgExtendedError ())
            g_message ("PageSetupDlg failed: %d",
                       (int)CommDlgExtendedError ());
        return;
    }
    psDlg.Flags |= PSD_MARGINS;

    if( psDlg.hDevMode != NULL )
        hDevMode = psDlg.hDevMode;
}

typedef struct {
    int            width;
    int            height;
    int            nextScanline;
    unsigned char *bgrRow;
    HDC            hdcMem;
    float          devOffsetX;
    float          devOffsetY;
    float          devScaleX;
    float          devScaleY;
} WinDriverOptions;

static gint windriver_handler( void * cb_data, void * scanline_in )

{
    unsigned char *scanline = (unsigned char *) scanline_in;
    WinDriverOptions *options = (WinDriverOptions *) cb_data;
    int            i, scan_off, line_len;

    line_len = 4 * ((options->width * 3 + 3) / 4);
    scan_off = line_len * (options->height - options->nextScanline - 1);
    /* We need to convert into BGR format */
    for( i = 0; i < options->width; i++ )
    {
        options->bgrRow[i*3+2+scan_off] = scanline[i*3+0];
        options->bgrRow[i*3+1+scan_off] = scanline[i*3+1];
        options->bgrRow[i*3+0+scan_off] = scanline[i*3+2];
    }
    
    options->nextScanline++;
    
    return 0;
}

gint 
gv_view_area_print_to_windriver(GvViewArea *view, int width, int height,
                                float ulx, float uly, float lrx, float lry,
                                int is_rgb )

{
    DOCINFO	docInfo;
    double	devResX, devResY;
    HBITMAP	hBitmap;
    HANDLE	oldBm;
    BITMAPV4HEADER bmHeader;
    HGLOBAL	hDevNames;
    WinDriverOptions options;
    gint        errcode;
  
    prDlg.hwndOwner = NULL;
    prDlg.hDevNames = NULL;
    prDlg.Flags |=
        PD_RETURNDC | PD_DISABLEPRINTTOFILE | PD_HIDEPRINTTOFILE
        | PD_NOSELECTION;
    prDlg.nMinPage = prDlg.nMaxPage = 0;
    prDlg.nCopies = 1;
    prDlg.lStructSize = sizeof (PRINTDLG);
    prDlg.hDevMode = hDevMode;

    if (!PrintDlg (&prDlg))
    {
        if (CommDlgExtendedError ())
            g_message ("PrintDlg failed: %d",
                       (int)CommDlgExtendedError ());
        return 1;
    }

    if( hDevMode == NULL )
        hDevMode = prDlg.hDevMode;
    
    hDevNames = prDlg.hDevNames;

    if( prDlg.hDevMode != NULL )
        hDevMode = prDlg.hDevMode;

    if (!(GetDeviceCaps(prDlg.hDC, RASTERCAPS) & RC_BITBLT)) 
    {
        g_message ("Printer doesn't support bitmaps");
        return 1;
    }

    /* Start print job. */
    docInfo.cbSize = sizeof (DOCINFO);
    docInfo.lpszDocName = "OpenEV Print";
    docInfo.lpszOutput = NULL;
    docInfo.lpszDatatype = NULL;
    docInfo.fwType = 0;

    if (StartDoc (prDlg.hDC, &docInfo) == SP_ERROR)
        return 1;

    /* Prepare printer to accept a page. */
    if (StartPage (prDlg.hDC) <= 0)
    {
        g_message ("StartPage failed");
        AbortDoc (prDlg.hDC);
        return 1;
    }
    
    options.hdcMem = CreateCompatibleDC (prDlg.hDC);

    memset( &bmHeader, 0, sizeof(bmHeader) );
    bmHeader.bV4Size = sizeof (BITMAPV4HEADER);
    bmHeader.bV4Width = width;
    bmHeader.bV4Height = height;
    bmHeader.bV4Planes = 1;
    bmHeader.bV4BitCount = 24;
    bmHeader.bV4V4Compression = BI_RGB;
    bmHeader.bV4SizeImage = 0;
    bmHeader.bV4XPelsPerMeter = 0;
    bmHeader.bV4YPelsPerMeter = 0;
    bmHeader.bV4ClrUsed = 0;
    bmHeader.bV4ClrImportant = 0;
    bmHeader.bV4CSType = 0;

    hBitmap = CreateDIBSection (options.hdcMem,
                                (BITMAPINFO *) &bmHeader,
                                DIB_RGB_COLORS,
                                (PVOID *) &(options.bgrRow),
                                NULL,
                                0);
    if (hBitmap == NULL)
    {
        g_message ("CreateDIBSection failed");
        AbortDoc (prDlg.hDC);
        return 1;
    }

    options.nextScanline = 0;
    options.width = width;
    options.height = height;
    
    devResX = GetDeviceCaps(prDlg.hDC, LOGPIXELSX); /* in dpi */
    devResY = GetDeviceCaps(prDlg.hDC, LOGPIXELSY); /* in dpi */
    
    CPLDebug( "gvwinprint", "devResX = %f, devResY = %f\n",
            devResX, devResY );

    CPLDebug( "gvwinprint", "Size=%dmm x %dmm\n",
            GetDeviceCaps( prDlg.hDC, HORZSIZE ),
            GetDeviceCaps( prDlg.hDC, VERTSIZE ) );
    
    CPLDebug( "gvwinprint", "Resolution=%dp x %dp\n",
            GetDeviceCaps( prDlg.hDC, HORZRES ),
            GetDeviceCaps( prDlg.hDC, VERTRES ) );

    options.devOffsetX = 0;
    options.devOffsetY = 0;

    if( GetDeviceCaps( prDlg.hDC, HORZRES ) / (float) width
        < GetDeviceCaps( prDlg.hDC, VERTRES ) / (float) height )
    {
        options.devScaleX =
            GetDeviceCaps( prDlg.hDC, HORZRES ) / (float) width;
        options.devScaleY =
            GetDeviceCaps( prDlg.hDC, HORZRES ) / (float) width;
    }
    else
    {
        options.devScaleX =
            GetDeviceCaps( prDlg.hDC, VERTRES ) / (float) height;
        options.devScaleY =
            GetDeviceCaps( prDlg.hDC, VERTRES ) / (float) height;
    }
    
    oldBm = SelectObject (options.hdcMem, hBitmap);
	  
    errcode = gv_view_area_render_to_func( view, width, height, 
                                           windriver_handler, &options );
    
    if( StretchDIBits( prDlg.hDC,
                       (int) options.devOffsetX,
                       (int) options.devOffsetY,
                       (int) (width * options.devScaleX),
                       (int) (height * options.devScaleY),
                       0, 0, width, height,
                       (PVOID *) options.bgrRow,
                       (BITMAPINFO *) &bmHeader,
                       DIB_RGB_COLORS,
                       SRCCOPY ) == 0 )
    {
        CPLDebug( "gvwinprint", "StretchDIBits() error.\n" );
        errcode = 1;
    }
    
    SelectObject (options.hdcMem, oldBm);
    DeleteObject (hBitmap);
    
    if (EndPage (prDlg.hDC) <= 0)
    {
        g_message ("EndPage failed");
        EndDoc (prDlg.hDC);
        return 1;
    }

    EndDoc (prDlg.hDC);

    DeleteDC( prDlg.hDC );
    
    return errcode;
}

#else

void dummy_page_setup()

{
    printf( "Don't need this file for non-windows builds.\n" );
}

#endif    
