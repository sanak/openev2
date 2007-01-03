/******************************************************************************
 * $Id: gvtexturecache.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  GvRasterLayer texture caching.
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
 * $Log: gvtexturecache.c,v $
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
 * Revision 1.4  2001/12/13 03:29:17  warmerda
 * avoid purging textures used in this render
 *
 * Revision 1.3  2000/07/18 14:52:06  warmerda
 * added texture dump, touch reset textures
 *
 * Revision 1.2  2000/07/03 12:47:46  warmerda
 * upped default to 16M again
 *
 * Revision 1.1  2000/06/27 21:25:53  warmerda
 * New
 *
 */

#include <stdio.h>
#include <assert.h>
#include "gvrastercache.h"
#include "gvrasterlayer.h"

static gint gv_cache_max = 16 * 1024 * 1024;
static gint gv_cache_used = 0;

static GvRasterLayerTexObj *lru_head = NULL;
static GvRasterLayerTexObj *lru_tail = NULL;

static int gv_purge_texture_lru();

void gv_texture_cache_dump()

{
    GvRasterLayerTexObj *link;

    printf( " -- texture lru dump -- (used=%d of %d)\n",
            gv_cache_used, gv_cache_max );
    for( link = lru_head; link != NULL; link = link->next )
    {
        if( link->next != NULL )
        {
            assert( link->next->prev == link );
        }

        printf( "%-20.20s tex_obj=%d tile=%d lod=%d, size=%d\n",
                gv_data_get_name(GV_DATA(link->layer)),
                link->tex_obj, link->texture, link->lod, link->size );
    }
}

int gv_texture_cache_get_max()

{
    return gv_cache_max;
}

void gv_texture_cache_set_max(int new_max)

{
    gv_cache_max = new_max;

    /* free space to ensure we stay below our limit */
    while( gv_cache_used > gv_cache_max && gv_purge_texture_lru() ) {}
}

int gv_texture_cache_get_used()

{
    return gv_cache_used;
}

void gv_raster_layer_create_texture(GvRasterLayer *layer, int texture,
				    GLuint tex_obj, int lod, int size)

{
    GvRasterLayerTexObj  *tex;

    /* If it already exists, blow it away - this shouldn't happen */
    if( layer->textures[texture] != NULL )
        gv_raster_layer_purge_texture( layer, texture );

    tex = g_new0( GvRasterLayerTexObj, 1 );
    tex->lod = lod;
    tex->size = size;
    tex->tex_obj = tex_obj;
    tex->layer = layer;
    tex->texture = texture;

    /* account for texture memory used */
    gv_cache_used += tex->size;

    /* free space to ensure we stay below our limit */
    while( gv_cache_used > gv_cache_max && gv_purge_texture_lru() ) {}

    /* attach to layer */
    layer->textures[texture] = tex;

    /* put into the LRU list */
    gv_raster_layer_touch_texture( layer, texture );
}

void gv_raster_layer_reset_texture( GvRasterLayer *layer, int texture, 
                                    int lod, int size )

{
    gv_cache_used -= layer->textures[texture]->size;
    
    layer->textures[texture]->size = size;
    layer->textures[texture]->lod = lod;

    gv_cache_used += layer->textures[texture]->size;
    gv_raster_layer_touch_texture( layer, texture );

    /* purge other textures so that will stay under the texture memory limit */
    while(gv_cache_used > gv_cache_max && gv_purge_texture_lru()) {}
}

void gv_raster_layer_purge_texture( GvRasterLayer *layer, int texture )

{
    GvRasterLayerTexObj  *tex = layer->textures[texture];

    if( tex == NULL )
        return;

    gv_cache_used -= tex->size;
    glDeleteTextures( 1, &(tex->tex_obj) );

    if( tex->prev != NULL )
        tex->prev->next = tex->next;

    if( tex->next != NULL )
        tex->next->prev = tex->prev;
    
    if( lru_head == tex )
        lru_head = tex->next;

    if( lru_tail == tex )
        lru_tail = tex->prev;

    g_free( layer->textures[texture] );
    layer->textures[texture] = NULL;
}

void gv_raster_layer_touch_texture( GvRasterLayer *layer, int texture )

{
    GvRasterLayerTexObj  *tex = layer->textures[texture];

    if( tex == NULL )
        return;

    /* Remove from current location in LRU list (if in it) */
    if( tex->prev != NULL )
        tex->prev->next = tex->next;

    if( tex->next != NULL )
        tex->next->prev = tex->prev;
    
    if( lru_head == tex )
        lru_head = tex->next;

    if( lru_tail == tex )
        lru_tail = tex->prev;

    tex->prev = tex->next = NULL;

    /* Add at tail of list */
    if( lru_tail == NULL )
    {
        lru_tail = lru_head = tex;
    }
    else
    {
        assert( lru_head != NULL );
        assert( lru_tail->next == NULL );

        lru_tail->next = tex;
        tex->prev = lru_tail;
        lru_tail = tex;
    }

    /* mark it with the current render count */
    tex->last_render = gv_get_render_counter();
}    

/*
 * Purge the least recently used texture (at the head of LRU list). 
 */
static int gv_purge_texture_lru()

{
    if( lru_head == NULL )
        return FALSE;

    if( lru_head->last_render == gv_get_render_counter() )
    {
        static int bReported = FALSE;

        if( !bReported )
        {
            CPLDebug( "gvrasterlayer", "short circuit purge within render" );
            bReported = TRUE;
        }
        
        return FALSE;
    }
    
    gv_raster_layer_purge_texture( lru_head->layer, 
                                   lru_head->texture );
        
    return TRUE;
}
