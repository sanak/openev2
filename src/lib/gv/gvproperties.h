/******************************************************************************
 * $Id: gvproperties.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Generic string properties list.
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
 * $Log: gvproperties.h,v $
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
 * Revision 1.6  2002/11/04 21:42:06  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.5  2002/09/09 16:22:45  warmerda
 * use int instead of gint to avoid glib.h dependency
 *
 * Revision 1.4  2002/07/24 18:06:26  warmerda
 * reimplement properties using quarks
 *
 * Revision 1.3  2000/09/21 02:55:11  warmerda
 * added gv_properties_clear
 *
 * Revision 1.2  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_PROPERTIES_H__
#define __GV_PROPERTIES_H__

#define USE_HASH_BASED_GVPROPERTIES

#ifdef USE_HASH_BASED_GVPROPERTIES
#include <glib.h>
  typedef guint32 *GvProperties;
#else
  typedef char **GvProperties;
#endif

void gv_properties_set(GvProperties *properties, const char *name, const char *value);
const char * gv_properties_get(GvProperties *properties, const char *name);
int  gv_properties_count(GvProperties *properties);
const char *gv_properties_get_name_by_index(GvProperties *properties, int prop_index);
const char *gv_properties_get_value_by_index(GvProperties *properties, int prop_index);
void gv_properties_remove(GvProperties *properties, const char *key);
void gv_properties_init(GvProperties *properties);
void gv_properties_copy(GvProperties *source, GvProperties *target);
void gv_properties_destroy(GvProperties *properties);
void gv_properties_clear(GvProperties *properties);

#endif /*__GV_PROPERTIES_H__ */
