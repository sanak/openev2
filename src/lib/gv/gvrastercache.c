/******************************************************************************
 * $Id: gvrastercache.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  GvRaster tile caching.
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
 * $Log: gvrastercache.c,v $
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
 * Revision 1.12  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "gvrastercache.h"

static gint gv_cache_max = 20 * 1024 * 1024;
static gint gv_cache_used = 0;

static gv_raster_cache_tile *lru_head = NULL;
static gv_raster_cache_tile *lru_tail = NULL;


static void gv_raster_cache_tile_touch( gv_raster_cache_tile * );
static int gv_raster_cache_purge_lru();

gv_raster_cache *
gv_raster_cache_new( gint tiles, gint lod )
{
    gv_raster_cache *cache;
    int i;

    if( ( cache = g_new( gv_raster_cache, 1 ) ) == NULL )
    {
	return NULL;
    }

    cache->max_lod = lod;
    cache->max_tiles = tiles;

    /* Allocate tile pointer arrays, but not the tiles themselves */
    if( (cache->tiles = g_new(gv_raster_cache_tile**,cache->max_lod)) == NULL )
    {
	g_free( cache );
	return NULL;
    }

    for( i = 0; i < cache->max_lod; i++ )
    {
        cache->tiles[i] = g_new0(gv_raster_cache_tile*,cache->max_tiles);
        if( cache->tiles[i] == NULL )
            return NULL;
    }

    return cache;
}

void *gv_raster_cache_get( gv_raster_cache *cache, gint tile, gint lod )
{
    if( tile < cache->max_tiles && lod < cache->max_lod 
        && cache->tiles[lod][tile] )
    {
        gv_raster_cache_tile_touch( cache->tiles[lod][tile] );
        return cache->tiles[lod][tile]->data;
    } else {
        return NULL;
    }
}

int gv_raster_cache_get_max()

{
    return gv_cache_max;
}

void gv_raster_cache_set_max( int new_max )

{
    gv_cache_max = new_max;

    /* free space to ensure we stay below our limit */
    while( gv_cache_used > gv_cache_max )
    {
        if( !gv_raster_cache_purge_lru() )
            break;
    }
}

int gv_raster_cache_get_used()

{
    return gv_cache_used;
}

void
gv_raster_cache_put( gv_raster_cache *cache, gint tile, gint lod, void *data, gint size )
{
    gv_raster_cache_tile *tile_obj;

    if( tile >= cache->max_tiles || lod >= cache->max_lod )
    {
        g_warning( "Illegal tile or lod in gv_raster_cache_put" );
        return;
    }

    /* If there is an existing tile, just blow it away */
    tile_obj = cache->tiles[lod][tile];
    if( tile_obj != NULL )
        gv_raster_cache_del( cache, tile, lod );

    /* free space to ensure we stay below our limit */
    while( gv_cache_used + size > gv_cache_max )
    {
        if( !gv_raster_cache_purge_lru() )
            break;
    }

    /* create a new tile */
    tile_obj = cache->tiles[lod][tile] = g_new0(gv_raster_cache_tile,1);
    tile_obj->tile = tile;
    tile_obj->lod = lod;
    tile_obj->cache = cache;
    tile_obj->data = data;
    tile_obj->size = size;

    /* increment total used space */
    gv_cache_used += size;

    /* put into LRU list */
    gv_raster_cache_tile_touch( tile_obj );
}

void
gv_raster_cache_del( gv_raster_cache *cache, gint tile, gint lod )
{
    gv_raster_cache_tile *tile_obj;

    if( tile >= cache->max_tiles || lod >= cache->max_lod )
        return;

    if( cache->tiles[lod][tile] == NULL )
        return;

    tile_obj = cache->tiles[lod][tile];
    cache->tiles[lod][tile] = NULL;

    /* removed from LRU list */
    if( lru_head == tile_obj )
        lru_head = tile_obj->next;

    if( lru_tail == tile_obj )
        lru_tail = tile_obj->prev;

    if( tile_obj->prev != NULL )
        tile_obj->prev->next = tile_obj->next;
    
    if( tile_obj->next != NULL )
        tile_obj->next->prev = tile_obj->prev;

    /* Free tile data, and tile object itself */
    gv_cache_used -= tile_obj->size;
    g_free( tile_obj->data );
    g_free( tile_obj );
}

/*
 * Touch a cache tile, moving it to the end of the LRU list.
 */
static void gv_raster_cache_tile_touch( gv_raster_cache_tile * tile_obj )
{
    /* don't put the most reduced LOD tiles on LRU list
       ... they are `locked in' */
    if( tile_obj->lod == tile_obj->cache->max_lod-1 )
        return;

    /* Remove from LRU list (if it's there) */
    if( lru_head == tile_obj )
        lru_head = tile_obj->next;

    if( lru_tail == tile_obj )
        lru_tail = tile_obj->prev;

    if( tile_obj->prev != NULL )
        tile_obj->prev->next = tile_obj->next;
    
    if( tile_obj->next != NULL )
        tile_obj->next->prev = tile_obj->prev;

    /* Push on the tail of the LRU list */
    if( lru_tail == NULL )
    {
        lru_head = lru_tail = tile_obj;
        tile_obj->next = NULL;
        tile_obj->prev = NULL;
    }
    else
    {
        lru_tail->next = tile_obj;
        tile_obj->prev = lru_tail;
        tile_obj->next = NULL;
        lru_tail = tile_obj;
    }
}

/*
 * Purge the least recently used tile for a given level of detail. 
 */
static int 
gv_raster_cache_purge_lru()
{
    if( lru_head == NULL )
        return FALSE;

    gv_raster_cache_del( lru_head->cache, lru_head->tile, lru_head->lod );

    return TRUE;
}

/*
 * Flush all tiles in the cache without destroying the cache. 
 */
void 
gv_raster_cache_flush_all( gv_raster_cache *cache )
{ 
    gint i, lod;

    for( lod = 0; lod < cache->max_lod; lod++ )
    {
        for( i = 0; i < cache->max_tiles; i++ )
        {
            gv_raster_cache_del( cache, i, lod );
        }
    }
}

int
gv_raster_cache_get_best_lod( gv_raster_cache *cache, gint tile, gint lod )
{
    int retval, i;

    if( cache->tiles[lod][tile] )
    {
	return lod;
    } else {

	retval = -1;

	for( i = cache->max_lod-1; i >= 0; i-- )
	{
	    if( i == lod )
		continue;

	    if( cache->tiles[i][tile] )
	    {
		if( i > lod )
		{
		    retval = i;
		} else {
		    return i;
		}
	    }
	}

	return retval;
    }
}

void
gv_raster_cache_free( gv_raster_cache * cache )
{
    int   lod;

    if( cache == NULL )
        return;

    gv_raster_cache_flush_all( cache );
    
    for( lod = 0; lod < cache->max_lod; lod++ )
            g_free( cache->tiles[lod] );
    
    g_free( cache->tiles );

    g_free( cache );
}
