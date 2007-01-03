/******************************************************************************
 * $Id: gvutils.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Various utility functions.
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
 * $Log: gvutils.c,v $
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
 * Revision 1.22  2004/06/23 14:35:05  gmwalter
 * Added support for multi-band complex imagery.
 *
 * Revision 1.21  2004/04/21 15:13:58  andrey_kiselev
 * Fix comparisons with NODATA value.
 *
 * Revision 1.20  2004/04/02 17:01:02  gmwalter
 * Updated nodata support for complex and
 * rgb data.
 *
 * Revision 1.19  2004/01/22 20:03:22  andrey_kiselev
 * gv_format_point_query() returns "[NODATA]" label if value marked as NODATA.
 *
 * Revision 1.18  2001/08/08 17:43:22  warmerda
 * avoid warning
 *
 * Revision 1.17  2001/04/22 17:32:25  pgs
 * added get_short_path_name
 *
 * Revision 1.16  2000/10/10 17:38:41  srawlin
 * changed lat/long decimal format to report position to 7 decimal digits
 *
 * Revision 1.15  2000/09/26 16:55:35  srawlin
 * changed lat/long decimal format to report N/S and E/W instead of +/-
 *
 * Revision 1.14  2000/09/26 16:32:41  srawlin
 * added degree_mode to set Lat/Long display to be either in the form dd:mm:ss (dms) or dd.dd (decimal)
 *
 * Revision 1.13  2000/08/25 20:08:44  warmerda
 * added phase/magnitude reporting
 *
 * Revision 1.12  2000/08/24 19:49:27  warmerda
 * don't operate o null strings
 *
 * Revision 1.11  2000/08/24 03:37:52  warmerda
 * added PIXEL as a coordinate system
 *
 * Revision 1.10  2000/08/24 03:26:01  warmerda
 * fixed +- sign for complex raster values
 *
 * Revision 1.9  2000/06/26 15:12:17  warmerda
 * don't crash if gvraster is null
 *
 * Revision 1.8  2000/06/23 12:56:52  warmerda
 * added multiple GvRasterSource support
 *
 * Revision 1.7  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include <stdio.h>
#include "gvutils.h"
#include "gvrasterlayer.h"
#include "ogr_srs_api.h"
#include "cpl_conv.h"

void gv_set_color_from_string( GvColor color, const char * string,
                               float def_red, float def_green,
                               float def_blue, float def_alpha )

{
    if( string != NULL )
    {
        def_alpha = 1.0;
        sscanf( string, "%f %f %f %f",
                &def_red, &def_green, &def_blue, &def_alpha );
    }

    if( def_red > 1.0 || def_green > 1.0 || def_blue > 1.0 || def_alpha > 1.0 )
    {
        def_red /= 255.0;
        def_green /= 255.0;
        def_blue /= 255.0;
        def_alpha /= 255.0;
    }
    color[0] = def_red;
    color[1] = def_green;
    color[2] = def_blue;
    color[3] = def_alpha;
}

void gv_complex_to_phase_mag( float real, float imaginary,
                              float *phase, float *magnitude )

{
    *magnitude = sqrt(real*real+imaginary*imaginary);
    *phase = atan2(imaginary, real);
}

gchar *gv_make_latlong_srs( const char * projected_srs )

{
    OGRSpatialReferenceH  hSRSProjected, hSRSLatLong;
    char                  *osr_wkt, *glib_wkt;

    if( projected_srs == NULL )
        return NULL;

    hSRSProjected = OSRNewSpatialReference( projected_srs );
    if( hSRSProjected == NULL )
        return FALSE;

    if( OSRIsGeographic( hSRSProjected ) )
    {
        OSRDestroySpatialReference( hSRSProjected );
        return g_strdup( projected_srs );
    }

    hSRSLatLong = OSRCloneGeogCS( hSRSProjected );
    OSRDestroySpatialReference( hSRSProjected );
    if( hSRSLatLong == NULL )
        return NULL;

    osr_wkt = NULL;
    glib_wkt = NULL;
    OSRExportToWkt( hSRSLatLong, &osr_wkt );
    if( osr_wkt != NULL )
    {
        glib_wkt = g_strdup( osr_wkt );
        CPLFree( osr_wkt );
    }

    if( hSRSLatLong != NULL )
        OSRDestroySpatialReference( hSRSLatLong );

    return glib_wkt;
}

int gv_reproject_points( const char *source_srs,
                         const char *destination_srs,
                         int count, double *x, double *y, double *z )

{
    int success = TRUE;
    OGRSpatialReferenceH   hSRSNew = NULL, hSRSOld = NULL;
    OGRCoordinateTransformationH hTransform = NULL;

    if( source_srs == NULL || destination_srs == NULL )
        return FALSE;

    /*
     * Try and establish if we can, or need to do reprojection.
     */
    hSRSNew = OSRNewSpatialReference( destination_srs );
    if( hSRSNew == NULL )
        return FALSE;

    hSRSOld = OSRNewSpatialReference(source_srs);
    if( hSRSOld == NULL )
    {
        OSRDestroySpatialReference( hSRSNew );
        return FALSE;
    }

    if( OSRIsSame( hSRSOld, hSRSNew ) )
    {
        OSRDestroySpatialReference( hSRSOld );
        OSRDestroySpatialReference( hSRSNew );

        return TRUE;
    }

    /*
     * Establish transformation.
     */

    hTransform = OCTNewCoordinateTransformation( hSRSOld, hSRSNew );
    if( hTransform == NULL )
    {
        OSRDestroySpatialReference( hSRSOld );
        OSRDestroySpatialReference( hSRSNew );

        return FALSE;
    }

    /*
     * Transform all the mesh points.
     */

    success = OCTTransform( hTransform, count, x, y, z );

    OCTDestroyCoordinateTransformation( hTransform );
    OSRDestroySpatialReference( hSRSOld );
    OSRDestroySpatialReference( hSRSNew );

    return success;
}

#ifndef WIN32
int gv_launch_url( const char * url )

{
    return 0;
}

char * gv_short_path_name( const char * lpszLongPath)
{
	return (char *) lpszLongPath;
}

#else
#include "windows.h"
int gv_launch_url( const char * url )

{
    HINSTANCE   hInstance;

    hInstance = ShellExecute( NULL, "open", url, "", "", SW_SHOWNORMAL );

    return ((int) hInstance) > 32;
}

char * gv_short_path_name( const char * lpszLongPath)
{
	int ret;
	char * lpszShortPath;

	lpszShortPath = (char *) malloc(MAX_PATH * sizeof(char));
	lpszShortPath[0] = 0;

	ret = GetShortPathName(lpszLongPath, lpszShortPath, MAX_PATH);

	#ifdef DEBUG
		printf("gv_short_path_name: returned %d for (%s)\n", ret, lpszShortPath);
	#endif

	if (ret > 0)
		return lpszShortPath;
	else
	{
		free(lpszShortPath);
		return "";
	}
}
#endif
