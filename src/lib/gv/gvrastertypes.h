/******************************************************************************
 * $Id: gvrastertypes.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Raster data types.
 * Author:   OpenEV Team
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
 * $Log: gvrastertypes.h,v $
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
 * Revision 1.2  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef _GV_RASTER_TYPES_H_
#define _GV_RASTER_TYPES_H_

enum {
    GV_RASTER_BYTE_RGBA = 0,
    GV_RASTER_BYTE_RGB,
    GV_RASTER_BYTE_REAL,
    GV_RASTER_BYTE_COMPLEX,
    GV_RASTER_SHORT_RGBA,
    GV_RASTER_SHORT_RGB,
    GV_RASTER_SHORT_REAL,
    GV_RASTER_SHORT_COMPLEX,
    GV_RASTER_INT_RGBA,
    GV_RASTER_INT_RGB,
    GV_RASTER_INT_REAL,
    GV_RASTER_INT_COMPLEX,
    GV_RASTER_FLOAT_RGBA,
    GV_RASTER_FLOAT_RGB,
    GV_RASTER_FLOAT_REAL,
    GV_RASTER_FLOAT_COMPLEX
};

#endif
