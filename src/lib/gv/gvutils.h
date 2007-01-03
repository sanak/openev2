/******************************************************************************
 * $Id: gvutils.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
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
 * $Log: gvutils.h,v $
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
 * Revision 1.5  2001/04/22 17:32:25  pgs
 * added get_short_path_name
 *
 * Revision 1.4  2000/08/25 20:08:44  warmerda
 * added phase/magnitude reporting
 *
 * Revision 1.3  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_UTILS_H__
#define __GV_UTILS_H__

#include "gvtypes.h"
#include "gvproperties.h"
#include "gvviewarea.h"

void gv_set_color_from_string(GvColor color, const char * string,
                              float def_red, float def_green,
                              float def_blue, float def_alpha);

const char *gv_format_point_query(GvViewArea *view,
                                   GvProperties *properties,
                                   double geo_x, double geo_y);

int gv_reproject_points(const char *source_srs,
			const char *destination_srs,
			int count, double *x, double *y, double *z);
gchar *gv_make_latlong_srs(const char *projected_srs);

void gv_complex_to_phase_mag(float real, float imaginary,
			     float *phase, float *magnitude);

int gv_launch_url(const char *url);

char *gv_short_path_name(const char *lpszLongPath);

#endif /* __GV_UTILS_H__ */

