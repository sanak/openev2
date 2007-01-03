/******************************************************************************
 * $Id: gvprint.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  GvViewArea printing support.
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
 * $Log: gvprint.c,v $
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
 * Revision 1.8  2001/02/15 16:36:51  warmerda
 * fixed serious bug with print_handler - now deinitialized when done
 *
 * Revision 1.7  2000/08/07 18:42:48  warmerda
 * added windows printing stubs
 *
 * Revision 1.6  2000/08/03 18:20:41  warmerda
 * implemented print scaling and paper sizes properly
 *
 * Revision 1.5  2000/07/20 03:21:26  warmerda
 * added is_rgb for print_to_file()
 *
 * Revision 1.4  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvviewarea.h"
#include "gdal.h"
#include "cpl_conv.h"

typedef struct 
{
    gint     width;
    gint     (*cb_func)(void *, const char *);
    void     *cb_data;
    gint     is_rgb; /* otherwise reduce to greyscale */
    char    *text_buf;
} gvPostScriptOptions;

static gint print_handler( void * cb_data, void * scanline_in )

{
    unsigned char *scanline = (unsigned char *) scanline_in;
    static GDALDatasetH  working_ds = NULL;
    static int next_scanline = 0;
    static GDALRasterBandH red_band, green_band, blue_band;
    gint   width;

    if( cb_data == NULL && scanline_in == NULL )
    {
        next_scanline = 0;
        working_ds = NULL;
        return -1;
    }

    if( working_ds != cb_data )
    {
        working_ds = (GDALDatasetH) cb_data;
        next_scanline = 0;

        red_band = GDALGetRasterBand( working_ds, 1 );
        if( GDALGetRasterCount( working_ds ) >= 3 )
        {
            green_band = GDALGetRasterBand( working_ds, 2 );
            blue_band = GDALGetRasterBand( working_ds, 3 );
        }
        else
        {
            green_band = blue_band = NULL;
        }

        if( red_band == NULL )
            return -1;
    }

    width = GDALGetRasterXSize( working_ds );

    if( green_band == NULL )
    {
        GByte	*grey;
        int     i, value;

        grey = g_new( GByte, width );
        
        for( i = 0; i < width; i++ )
        {
            value = *(scanline++);
            value += *(scanline++);
            value += *(scanline++);
            value = (value+1) / 3;
            grey[i] = value;
        }
        
        GDALRasterIO( red_band, GF_Write, 0, next_scanline, width, 1, 
                      grey, width, 1, GDT_Byte, 1, 0 );
        g_free( grey );
    }
    else
    {
        GDALRasterIO( red_band, GF_Write, 0, next_scanline, width, 1, 
                      scanline+0, width, 1, GDT_Byte, 3, 0 );
        GDALRasterIO( green_band, GF_Write, 0, next_scanline, width, 1, 
                      scanline+1, width, 1, GDT_Byte, 3, 0 );
        GDALRasterIO( blue_band, GF_Write, 0, next_scanline, width, 1, 
                      scanline+2, width, 1, GDT_Byte, 3, 0 );
    }
    
    next_scanline++;

    return 0;
}

gint gv_view_area_print_to_file(GvViewArea *view, int width, int height, const char * filename, const char * format, int is_rgb)
{
    GDALDriverH   driver;
    GDALDatasetH  dataset;
    gint          errcode;

    driver = GDALGetDriverByName( format );
    if( driver == NULL )
        return -1;

    if( is_rgb )
        dataset = GDALCreate( driver, filename, width, height, 3, GDT_Byte, 
                              NULL );
    else
        dataset = GDALCreate( driver, filename, width, height, 1, GDT_Byte, 
                              NULL );

    if( dataset == NULL )
        return -1;

    errcode = gv_view_area_render_to_func( view, width, height, 
                                           print_handler, dataset );
    GDALClose( dataset );

    print_handler( NULL, NULL );
    
    return errcode;
}

static gint postscript_handler( void * cb_data, void * scanline_in )

{
    unsigned char *scanline = (unsigned char *) scanline_in;
    gvPostScriptOptions *options = (gvPostScriptOptions *) cb_data;
    int i;
    
    if( options->is_rgb )
    {
        for( i = 0; i < options->width; i++ )
            sprintf( options->text_buf + i*2, "%02x", 
                     scanline[i*3] );

        for( i = 0; i < options->width; i++ )
            sprintf( options->text_buf + i*2 + options->width*2, "%02x", 
                     scanline[i*3+1] );

        for( i = 0; i < options->width; i++ )
            sprintf( options->text_buf + i*2 + options->width*4, "%02x", 
                     scanline[i*3+2] );

        options->text_buf[options->width*6] = '\0';
    }
    else
    {
        for( i = 0; i < options->width; i++ )
        {
            int value;
            
            value = *(scanline++);
            value += *(scanline++);
            value += *(scanline++);
            value = (value+1) / 3;

            sprintf( options->text_buf + i*2, "%02x", value );
        }
        options->text_buf[options->width*2] = '\0';
    }

    strcat( options->text_buf, "\n" );

    

    return options->cb_func( options->cb_data, options->text_buf );
}

gint
gv_view_area_render_postscript(GvViewArea *view, int width, int height, 
                               float ulx, float uly, float lrx, float lry,
                               int is_rgb,
                               gint (*cb_func)(void *, const char *), 
                               void * cb_data )

{
    gvPostScriptOptions   options;
    int errcode;
    char line[128];

    /* write prolog */

    cb_func( cb_data, "%!PS-Adobe-3.0 EPSF-3.0\n" );
    cb_func( cb_data, "%%Creator: gview\n" );
    cb_func( cb_data, "%%Title: gview_print\n" );
    cb_func( cb_data, "%%CreationDate: Thu Apr  6 20:11:10 2000\n" );
    cb_func( cb_data, "%%DocumentData: Clean7Bit\n" );
    cb_func( cb_data, "%%Origin: 0 0\n" );
#ifdef notdef
    sprintf( line, "%%%%BoundingBox: 0 0 %d %d\n", width, height );
    cb_func( cb_data, line );
#endif
    cb_func( cb_data, "%%LanguageLevel: 1\n" );
    cb_func( cb_data, "%%Pages: 1\n" );
    cb_func( cb_data, "%%EndComments\n" );
    cb_func( cb_data, "%%BeginSetup\n" );
    cb_func( cb_data, "%%EndSetup\n" );
    cb_func( cb_data, "%%Page: 1 1\n" );
    cb_func( cb_data, "gsave\n" );
    cb_func( cb_data, "100 dict begin\n" );

    sprintf( line, "%f %f translate\n", ulx*72.0, uly*72.0 );
    cb_func( cb_data, line );

    sprintf( line, "%f %f scale\n", (lrx - ulx)*72.0, (lry - uly)*72.0 );
    cb_func( cb_data, line );

    if( is_rgb )
    {
        sprintf( line, 
                 "%%ImageData: %d %d 8 3 0 %d 2 \"true 3 colorimage\"\n", 
                 width, height, width );
        cb_func( cb_data, line );

        sprintf( line, "/line0 %d string def\n", width );
        cb_func( cb_data, line );
        
        sprintf( line, "/line1 %d string def\n", width );
        cb_func( cb_data, line );
        
        sprintf( line, "/line2 %d string def\n", width );
        cb_func( cb_data, line );
    }
    else
    {
        sprintf( line, 
                 "%%ImageData: %d %d 8 1 0 %d 2 \"image\"\n", 
                 width, height, width );
        cb_func( cb_data, line );

        sprintf( line, "/scanLine %d string def\n", width );
        cb_func( cb_data, line );
    }


    sprintf( line, "%d %d 8\n", width, height );
    cb_func( cb_data, line );

    sprintf( line, "[%d 0 0 %d 0 %d]\n", width, -height, height );
    cb_func( cb_data, line );

    if( is_rgb )
    {
        cb_func( cb_data, "{currentfile line0 readhexstring pop}bind\n" );
        cb_func( cb_data, "{currentfile line1 readhexstring pop}bind\n" );
        cb_func( cb_data, "{currentfile line2 readhexstring pop}bind\n" );
        cb_func( cb_data, "true 3 colorimage\n" );
    }
    else
    {
        cb_func( cb_data, "{currentfile scanLine readhexstring pop}bind\n" );
        cb_func( cb_data, "image\n" );
    }

    /* now prepare and write image data */
    options.cb_func = cb_func;
    options.cb_data = cb_data;
    options.width = width;
    options.is_rgb = is_rgb;
    options.text_buf = g_malloc(width * 6 + 3);
    if( options.text_buf == NULL )
        return -1;

    errcode = gv_view_area_render_to_func( view, width, height, 
                                           postscript_handler, &options );
    
    g_free( options.text_buf );

    /* write postlog */

    if( errcode == 0 )
    {
        cb_func( cb_data, "end\n" );
        cb_func( cb_data, "grestore\n" );
        cb_func( cb_data, "showpage\n" );
        cb_func( cb_data, "%%Trailer\n" );
        cb_func( cb_data, "%%Pages: 1\n" );
        cb_func( cb_data, "%%EOF\n" );
    }

    return errcode;
}

static gint
postscript_to_file_handler( void *cb_data, const char * text )

{
    FILE *fp = (FILE *) cb_data;

    if( fputs(text, fp) < 0 )
        return -1;
    else
        return 0;
}

#ifndef _WIN32
static gint 
gv_view_area_print_postscript_to_pipe(GvViewArea *view, 
                                      int width, int height,
                                      float ulx, float uly, 
                                      float lrx, float lry,
                                      int is_rgb, const char * filename )

{
    FILE  *fp;
    int   errcode;

    fp = popen( filename, "w" );
    if( fp == NULL )
        return -1;
    
    errcode = 
        gv_view_area_render_postscript(view, width, height, ulx, uly, lrx, lry,
                                       is_rgb,postscript_to_file_handler, fp );

    pclose(fp);

    return errcode;
}
#endif
                                      
gint 
gv_view_area_print_postscript_to_file(GvViewArea *view, 
				      int width, int height, 
                                      float ulx, float uly, 
                                      float lrx, float lry,
                                      int is_rgb,
                                      const char * filename)

{
    FILE  *fp;
    int   errcode;

#ifndef _WIN32
    if(filename[0] == '|')
        return gv_view_area_print_postscript_to_pipe(view, width, height, 
                                                     ulx, uly, lrx, lry,
                                                     is_rgb, filename+1 );
#endif

    fp = fopen( filename, "wt" );
    if( fp == NULL )
        return -1;
    
    errcode = 
        gv_view_area_render_postscript(view, width, height, ulx, uly, lrx, lry,
                                       is_rgb,postscript_to_file_handler, fp );

    fclose(fp);

    return errcode;
}
             
#ifndef WIN32                         
void gv_view_area_page_setup()
{
}

gint 
gv_view_area_print_to_windriver(GvViewArea *view, int width, int height,
                                float ulx, float uly, float lrx, float lry,
                                int is_rgb )

{
    return 1;
}
#endif

