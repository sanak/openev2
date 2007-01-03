/******************************************************************************
 * $Id: gextra.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
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
 * $Log: gextra.c,v $
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
 * Revision 1.4  2000/06/20 13:26:54  warmerda
 * added standard headers
 *
 */

#include <stdlib.h>
#include <glib.h>
#include "gextra.h"

gint
g_compare_gint(gconstpointer a, gconstpointer b)
{
    return *(gint*)a - *(gint*)b;
}

void
g_sort(gpointer mem, guint nmemb, gsize size, GCompareValFunc compare)
{
    qsort(mem, nmemb, size, compare);
}

void
g_ptr_array_insert_fast(GPtrArray *array, guint index, gpointer data)
{
    g_return_if_fail(array);
    g_return_if_fail(index <= array->len);
    
    if (index == array->len)
    {
	g_ptr_array_add(array, data);
    }
    else
    {
	gpointer moved = g_ptr_array_index(array, index);
	g_ptr_array_add(array, moved);
	g_ptr_array_index(array, index) = data;
    }
}

double g_get_current_time_as_double()

{
    GTimeVal      cur_time;

    g_get_current_time( &cur_time );
    
    return cur_time.tv_sec + cur_time.tv_usec / 1000000.0;
}

