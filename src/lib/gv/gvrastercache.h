/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  GvRaster tile caching.
 * Author:   OpenEV Team
 * Maintainer: Mario Beauchamp, starged@gmail.com
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
 */

#ifndef __GV_RASTER_CACHE_H__
#define __GV_RASTER_CACHE_H__

#include <glib.h>

typedef struct _gv_raster_cache gv_raster_cache;
typedef struct _gv_raster_cache_tile gv_raster_cache_tile;

struct _gv_raster_cache_tile {
    gv_raster_cache_tile *prev;
    gv_raster_cache_tile *next;
    gv_raster_cache      *cache;

    int  tile;
    int  lod;
    int  lru;
    int  size;
    void *data;
};

struct _gv_raster_cache {
    int max_lod;
    int max_tiles;

    gv_raster_cache_tile ***tiles;  /* double dimensioned as [lod][tile] */
};

gv_raster_cache *gv_raster_cache_new(gint max_tiles, gint max_lod);

void gv_raster_cache_free(gv_raster_cache *cache);

void *gv_raster_cache_get(gv_raster_cache *cache, gint tile, gint lod);

void gv_raster_cache_put(gv_raster_cache *cache, gint tile, gint lod, void *data, gint size);

int gv_raster_cache_get_best_lod(gv_raster_cache *cache, gint tile, gint lod);

void gv_raster_cache_flush_all(gv_raster_cache *cache);
void gv_raster_cache_del(gv_raster_cache *cache, gint tile, gint lod);
int gv_raster_cache_get_max();
void gv_raster_cache_set_max(int max);
int gv_raster_cache_get_used();

#endif
