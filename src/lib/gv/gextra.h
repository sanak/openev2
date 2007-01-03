/******************************************************************************
 * $Id: gextra.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Support functions that should have been in Glib.
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
 * $Log: gextra.h,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:26  uid1026
 *
 * Imported sources
 *
 * Revision 1.4  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __G_EXTRA_H__
#define __G_EXTRA_H__

#include <glib.h>

/* Duplicates a memory buffer containing elements of homogeneous type */
#define g_memdup_type(mem,type,count)  \
    ((type*)g_memdup((gconstpointer)mem, (unsigned)sizeof(type) * (count)))

/* Sorts a buffer containing elements of homogeneous type */
#define g_sort_type(mem,type,count) \
    (g_sort((gpointer)mem, count, (gsize)sizeof(type), g_compare_##type))

/* Comparison function for sorting (see qsort man page) */
typedef gint (*GCompareValFunc) (gconstpointer a, gconstpointer b);

/* General in-place sorting (uses qsort) */
void g_sort(gpointer mem, guint nmemb, gsize size, GCompareValFunc compare);

/* gint comparison for g_sort */
gint g_compare_gint(gconstpointer a, gconstpointer b);

void g_ptr_array_insert_fast(GPtrArray *array, guint index, gpointer data);

#define g_list_push(list,data)  g_list_prepend(list, data)
#define g_list_pop(list)        g_list_remove_link(list, list)

double g_get_current_time_as_double();

#endif /* __G_EXTRA_H__ */
