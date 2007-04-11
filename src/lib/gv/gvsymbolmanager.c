/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  manage file-based symbols
 * Author:   Paul Spencer (pgs@magma.ca)
 *
 ******************************************************************************
 * Copyright (c) 2002, Paul Spencer
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
 * $Log: gvsymbolmanager.c,v $
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
 * Revision 1.12  2004/02/16 17:08:12  warmerda
 * support OPENEVHOME as well as OPENEV_HOME
 *
 * Revision 1.11  2003/11/07 18:27:08  warmerda
 * use dummy sumbol when load fails
 *
 * Revision 1.10  2003/09/02 17:25:08  warmerda
 * Added _has_symbol(), and _get_names() methods.  Use outside GvShape
 * serialize/deserialize code.  Don't store symbols with absolute paths in
 * the hash.
 *
 * Revision 1.9  2003/08/29 20:52:58  warmerda
 * use gv_shape_from_xml_tree
 *
 * Revision 1.8  2003/05/16 17:50:34  warmerda
 * moved debug statement to avoid output everytime a loaded symbol is drawn
 *
 * Revision 1.7  2003/04/09 21:32:48  andrey_kiselev
 * Couple of memory leaks fixed.
 *
 * Revision 1.6  2003/04/09 13:14:24  andrey_kiselev
 * Memory leak fixed.
 *
 * Revision 1.5  2003/04/08 11:56:32  andrey_kiselev
 * Implemented gv_symbol_manager_save_vector_symbol() function.
 *
 * Revision 1.4  2003/04/07 20:09:04  andrey_kiselev
 * Path to simbols directory now could be configured with "symbols_dir"
 * GvManager property.
 *
 * Revision 1.3  2003/04/07 15:52:49  andrey_kiselev
 * gv_symbol_manager_get_symbol() now able to load and inject vector symbols.
 *
 * Revision 1.2  2003/02/28 16:46:45  warmerda
 * added partial support for vector symbols
 *
 * Revision 1.1  2002/11/14 20:10:41  warmerda
 * New
 */

#include "gvsymbolmanager.h"
#include "gvmanager.h"
#include "gdal.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "cpl_minixml.h"
#include <stdio.h>

/* signals (none for now?) */
enum { LAST_SIGNAL };

GByte *     gdal_to_rgba( GDALDatasetH hDS );
gboolean    finalize_symbol(gpointer key, gpointer value, gpointer user_data);

static void gv_symbol_manager_init( GvSymbolManager *manager );
static void gv_symbol_manager_class_init( GvSymbolManagerClass *klass );
static void gv_symbol_manager_finalize( GObject *manager );

static GObjectClass *parent_class = NULL;
/* static void gv_symbol_manager_signals[LAST_SIGNAL] = { 0 }; */

/************************************************************************/
/*                    gv_symbol_manager_class_init()                    */
/************************************************************************/
GType 
gv_symbol_manager_get_type(void)
{
    static GType symbol_manager_type = 0;

    if (!symbol_manager_type) {
        static const GTypeInfo symbol_manager_info =
        {
            sizeof(GvSymbolManagerClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_symbol_manager_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvSymbolManager),
            0,
            (GInstanceInitFunc) gv_symbol_manager_init,
        };
        symbol_manager_type = g_type_register_static (G_TYPE_OBJECT,
                                                    "GvSymbolManager",
                                                    &symbol_manager_info, 0);
        }

    return symbol_manager_type;
}

/************************************************************************/
/*                    gv_symbol_manager_class_init()                    */
/************************************************************************/
static void 
gv_symbol_manager_class_init( GvSymbolManagerClass *klass )
{
    parent_class = g_type_class_peek_parent (klass);

    /* ---- Override finalize ---- */
    G_OBJECT_CLASS(klass)->finalize = gv_symbol_manager_finalize;
}

/************************************************************************/
/*                       gv_symbol_manager_init()                       */
/************************************************************************/
static void 
gv_symbol_manager_init( GvSymbolManager *manager )
{
    manager->symbol_cache = g_hash_table_new( g_str_hash, g_str_equal );

    //GDALAllRegister();
}

/************************************************************************/
/*                         gv_symbol_manager_new()                      */
/*                                                                      */
/*      construct a new instance of the symbol manager.  This           */
/*      function is private.  Call gv_get_symbol_manager()              */
/************************************************************************/
GvSymbolManager *gv_symbol_manager_new()
{
    return g_object_new(GV_TYPE_SYMBOL_MANAGER, NULL);
}

/************************************************************************/
/*                         gv_get_symbol_manager()                      */
/*                                                                      */
/*      return a reference to the symbol manager object.  This          */
/*      implements the singleton pattern so that only one symbol        */
/*      manager is created.                                             */
/************************************************************************/
GvSymbolManager *
gv_get_symbol_manager()
{
    static GvSymbolManager *main_symbol_manager = NULL;

    if ( main_symbol_manager == NULL )
    {
        main_symbol_manager = gv_symbol_manager_new();
    }

    return main_symbol_manager;
}

/************************************************************************/
/*                    gv_symbol_manager_has_symbol()                    */
/************************************************************************/

int gv_symbol_manager_has_symbol( GvSymbolManager *manager, 
                                  const char *name )

{
    g_return_val_if_fail( manager != NULL, 0 );

    return g_hash_table_lookup( manager->symbol_cache, name ) != NULL;
}

/************************************************************************/
/*                      gv_symbol_manager_get_symbol()                  */
/*                                                                      */
/*      manager - a gvsymbolmanager instance (use gv_get_symbol_manager)*/
/*      pszFilename - the name of the symbol to read in                 */
/*                                                                      */
/*      This returns a texture 'name' that can be used with             */
/*      glBindTexture if the symbol is successfully loaded and          */
/*      converted to a texture or 0 on failure.  Symbols are cached     */
/*      using the absolute path to the symbol file so that only one     */
/*      copy of the symbol should ever be loaded.  This could be        */
/*      circumvented by having symlinks or .. in the path but in        */
/*      most real cases this won't happen.                              */
/*                                                                      */
/*      The maximum cache size is set in the xxxxxxx property.  When    */
/*      the symbol cache is full, textures are released on a Least      */
/*      Recently Used basis.  The texture can still be accessed         */
/*      because the symbol name will persist in the cache.              */
/************************************************************************/
GvSymbolObj *
gv_symbol_manager_get_symbol(GvSymbolManager *manager, const char *symbol_name)
{
    gchar   *pszOpenEVHome = NULL;
    gchar   *pszSymbolsDir = NULL;
    gchar   *pszAbsolutePath = NULL;
    gchar   *pszPathSeparator = NULL;
    GDALDatasetH hDataset;
    GvSymbolObj *poSymbol;
    CPLXMLNode  *xml_shape = NULL;
    GByte *rgba_buffer;

/* -------------------------------------------------------------------- */
/*      Lookup the symbol in the hash table, and return it if found.    */
/* -------------------------------------------------------------------- */
    poSymbol = g_hash_table_lookup( manager->symbol_cache, symbol_name );
    if( poSymbol != NULL )
        return poSymbol;

/* -------------------------------------------------------------------- */
/*      We didn't already have it, so try to find a file to load it     */
/*      from.                                                           */
/* -------------------------------------------------------------------- */
#ifndef WIN32
    pszPathSeparator = "/";
#else
    pszPathSeparator = "\\";
#endif /* WIN32 */

    /* validate inputs */
    g_return_val_if_fail( manager != NULL, 0 );
    g_return_val_if_fail( symbol_name != NULL, 0 );

    /* get an absolute path */
    if ( !g_path_is_absolute( symbol_name ) )
    {
        /* check configuration option first */
        pszSymbolsDir = g_strdup( gv_manager_get_preference( gv_get_manager(),
                                                             "symbols_dir" ) );

        /* if not configured check $OPENEV_HOME */
        if ( !pszSymbolsDir )
        {
            pszOpenEVHome = (gchar *)g_getenv( "OPENEV_HOME" );
            if( pszOpenEVHome == NULL )
                pszOpenEVHome = (gchar *)g_getenv( "OPENEVHOME" );
            if ( pszOpenEVHome )
                pszSymbolsDir = g_strjoin( pszPathSeparator, pszOpenEVHome,
                                           "symbols", NULL );
        }
        
        /* get current directory as last resort */
        if ( !pszSymbolsDir )
            pszSymbolsDir = g_get_current_dir();

        pszAbsolutePath = g_strjoin( pszPathSeparator, pszSymbolsDir, 
                                     symbol_name, NULL );
        g_free( pszSymbolsDir );
    }
    else
        pszAbsolutePath = g_strdup( symbol_name );

/* -------------------------------------------------------------------- */
/*      pszAbsolutePath contains a newly allocated string that is       */
/*      suitable for using as a key in the hash table.  If a texture    */
/*      is found in the hash table then this string needs to be         */
/*      freed.  If one isn't found then the string is used in the       */
/*      hash table and should be freed when the associated hash         */
/*      table entry is released                                         */
/* -------------------------------------------------------------------- */
    CPLDebug( "OpenEV", 
              "gv_symbol_manager_get_symbol(%s) ... need to load.", 
                  pszAbsolutePath );
    
    /* 
     * validate path by opening with GDAL and looking for an error 
     * Disable CPL error handler to supress error reporting
     */
    
    CPLErrorReset();
    CPLPushErrorHandler( CPLQuietErrorHandler );
    hDataset = GDALOpen( pszAbsolutePath, GA_ReadOnly );
    CPLPopErrorHandler();
    
    if ( hDataset )
    {
        rgba_buffer = gdal_to_rgba( hDataset );
        
        if ( rgba_buffer )
        {
            gv_symbol_manager_inject_raster_symbol( 
                manager, symbol_name,
                GDALGetRasterXSize( hDataset ),
                GDALGetRasterYSize( hDataset ),
                rgba_buffer );
            CPLFree( rgba_buffer );
        }
        
        GDALClose( hDataset );
    }
    /* probably we have vector symbol? */
    else if ( ( xml_shape = CPLParseXMLFile( pszAbsolutePath ) ) )
    {
        GvShape     *shape;
        shape = gv_shape_from_xml_tree( xml_shape );
        
        CPLDestroyXMLNode( xml_shape );
        
        if( shape != NULL )
            gv_symbol_manager_inject_vector_symbol( manager, symbol_name,
                                                    shape );
        else
        {
            CPLDebug( "OpenEV", 
                      "Failed to instantiate GvSahpe from file %s, using simple point.", 
                      pszAbsolutePath );
        
            shape = gv_shape_new( GVSHAPE_POINT );
            gv_symbol_manager_inject_vector_symbol( manager, symbol_name,
                                                    shape );
        }
    }
    else
    {
        GvShape *shape;

        CPLDebug( "OpenEV", "Failed to open file %s, using simple point.", 
                  pszAbsolutePath );
        
        shape = gv_shape_new( GVSHAPE_POINT );
        gv_symbol_manager_inject_vector_symbol( manager, symbol_name,
                                                shape );
    }
    
    /* look up in the hash table again */
    poSymbol = g_hash_table_lookup(manager->symbol_cache, symbol_name );

    g_free( pszAbsolutePath );

    return poSymbol;
}

/************************************************************************/
/*                            finalize_symbol()                         */
/*                                                                      */
/*      finalize a symbol object's hash table entry by freeing          */
/*      allocated memory.  It is (relatively) safe to free the key      */
/*      since we are sure we allocated the memory for it when           */
/*      inserting into the hashtable.                                   */
/************************************************************************/

gboolean
finalize_symbol( gpointer key, gpointer value, gpointer user_data )
{
    GvSymbolObj *poSymbol = (GvSymbolObj *) value;

    g_free( key );

    if (poSymbol->type == GV_SYMBOL_RASTER)
        g_free( poSymbol->buffer );
    else if( poSymbol->type == GV_SYMBOL_VECTOR )
        gv_shape_unref( (GvShape *) poSymbol->buffer );

    g_free( poSymbol );

    return TRUE;
}

/************************************************************************/
/*                       gv_symbol_manager_finalize()                   */
/*                                                                      */
/*      clean up the symbol manager storage and free associated memory. */
/************************************************************************/
static void
gv_symbol_manager_finalize( GObject *gobject)
{
    GvSymbolManager *manager = GV_SYMBOL_MANAGER(gobject);

    CPLDebug( "OpenEV", "gv_symbol_manager_finalize(%p)", gobject );

    if (manager->symbol_cache != NULL) {
      g_hash_table_foreach_remove(manager->symbol_cache, finalize_symbol, NULL);
      g_hash_table_destroy( manager->symbol_cache );
      manager->symbol_cache = NULL;
    }

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}

/************************************************************************/
/*                   gv_symbol_manager_eject_symbol()                   */
/*                                                                      */
/*      Remove and deallocate one (named) symbol from the symbol        */
/*      manager cache.                                                  */
/************************************************************************/

int gv_symbol_manager_eject_symbol( GvSymbolManager *manager, 
                                    const char *symbol_name )

{
    CPLDebug( "OpenEV", 
              "gv_symbol_manager_eject_symbol() not yet impelemented." );
    return FALSE;
}

/************************************************************************/
/*               gv_symbol_manager_inject_raster_symbol()               */
/*                                                                      */
/*      Create a GvSymbolObject corresponding to the pass size and      */
/*      raster info, and inject it into the symbol manager with the     */
/*      indicated name.                                                 */
/************************************************************************/

void gv_symbol_manager_inject_raster_symbol( GvSymbolManager *manager, 
                                             const char *symbol_name, 
                                             int width, int height, 
                                             void *rgba_buffer )

{
    GvSymbolObj *symbol;

    /* allocate a new symbol object */
    symbol = g_new0( GvSymbolObj, 1 );

    symbol->type = GV_SYMBOL_RASTER;
    
    symbol->width = width;
    symbol->height = height;
    
    /* initialize the foreground and background colors */
    symbol->foreground[0] = 0.0;
    symbol->foreground[1] = 0.0;
    symbol->foreground[2] = 0.0;
    symbol->foreground[3] = 1.0;
    symbol->background[0] = 1.0;
    symbol->background[1] = 1.0;
    symbol->background[2] = 1.0;
    symbol->background[3] = 0.0;

    /* copy the raster buffer. */
    symbol->buffer = (GByte *) CPLMalloc(width*height*4);
    memcpy( symbol->buffer, rgba_buffer, width*height*4 );

    /* insert new symbol into symbol manager */
    g_hash_table_insert( manager->symbol_cache, g_strdup(symbol_name), symbol);

    CPLDebug("OpenEV", "inject_raster_symbol(%s, %dx%d)", 
             symbol_name, width, height );
}

/************************************************************************/
/*               gv_symbol_manager_inject_vector_symbol()               */
/*                                                                      */
/*      Create a GvSymbolObject corresponding to the passed             */
/*      GvShape.  An internal copy of the shape is made.                */
/************************************************************************/

void gv_symbol_manager_inject_vector_symbol( GvSymbolManager *manager, 
                                             const char *symbol_name, 
                                             GvShape *shape )

{
    GvSymbolObj *symbol;

    /* allocate a new symbol object */
    symbol = g_new0( GvSymbolObj, 1 );

    symbol->type = GV_SYMBOL_VECTOR;
    
    /* take a reference to the shape */
    symbol->buffer = shape;
    gv_shape_ref( shape );
    
    /* insert new symbol into symbol manager */
    g_hash_table_insert( manager->symbol_cache, g_strdup(symbol_name), symbol );

    CPLDebug("OpenEV", "inject_vector_symbol(%s)", symbol_name );
}

/************************************************************************/
/*               gv_symbol_manager_save_vector_symbol()                 */
/*                                                                      */
/*      Save an existing and loaded vector simbol under new name.       */
/************************************************************************/

int gv_symbol_manager_save_vector_symbol( GvSymbolManager *manager, 
                                          const char *symbol_name, 
                                          const char *new_name )

{
    GvSymbolObj *symbol;

    /* allocate a new symbol object */
    symbol = g_hash_table_lookup( manager->symbol_cache, symbol_name );

    CPLDebug("OpenEV", "save_vector_symbol(%s->%s)", symbol_name, new_name );

    if ( symbol && symbol->type == GV_SYMBOL_VECTOR )
    {
        GvShape     *shape;
        CPLXMLNode  *xml_shape;

        shape = (GvShape *)symbol->buffer;
        xml_shape = gv_shape_to_xml_tree( shape );

        if ( CPLSerializeXMLTreeToFile( xml_shape, new_name ) )
        {
            CPLDestroyXMLNode( xml_shape );
            return TRUE;
        }
    }

    return FALSE;

}

/************************************************************************/
/*                             gdal_to_rgba()                           */
/*                                                                      */
/*      convert a GDAL dataset into an RGBA buffer.  Provided by        */
/*      Frank Warmerdam :)                                              */
/************************************************************************/
GByte *
gdal_to_rgba( GDALDatasetH hDS )
{
    int  nXSize, nYSize;
    GByte *pabyRGBABuf = NULL;

    /* validation of input parameters */
    g_return_val_if_fail( hDS != NULL, NULL );

/* -------------------------------------------------------------------- */
/*      Allocate RGBA Raster buffer.                                    */
/* -------------------------------------------------------------------- */

    nXSize = GDALGetRasterXSize( hDS );
    nYSize = GDALGetRasterYSize( hDS );
    CPLDebug( "OpenEV", "creating buffer of (%d,%d)", nXSize, nYSize );
    pabyRGBABuf = (GByte *) CPLMalloc( nXSize * nYSize * 4 );

/* -------------------------------------------------------------------- */
/*      Handle case where source is already presumed to be RGBA.        */
/* -------------------------------------------------------------------- */
    if( GDALGetRasterCount(hDS) == 4 )
    {
        GDALRasterIO( GDALGetRasterBand( hDS, 1 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+0, nXSize, nYSize, GDT_Byte, 
                      4, nXSize * 4 );
                      
        GDALRasterIO( GDALGetRasterBand( hDS, 2 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+1, nXSize, nYSize, GDT_Byte, 4, 
                      nXSize * 4 );
                      
        GDALRasterIO( GDALGetRasterBand( hDS, 3 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+2, nXSize, nYSize, GDT_Byte, 4, 
                      nXSize * 4 );
                      
        GDALRasterIO( GDALGetRasterBand( hDS, 4 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+3, nXSize, nYSize, GDT_Byte, 4, 
                      nXSize * 4 );
    }
/* -------------------------------------------------------------------- */
/*      Source is RGB.  Set Alpha to 255.                               */
/* -------------------------------------------------------------------- */
    else if( GDALGetRasterCount(hDS) == 3 )
    {
        memset( pabyRGBABuf, 255, 4 * nXSize * nYSize );
        
        GDALRasterIO( GDALGetRasterBand( hDS, 1 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+0, nXSize, nYSize, GDT_Byte, 
                      4, nXSize * 4 );
                      
        GDALRasterIO( GDALGetRasterBand( hDS, 2 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+1, nXSize, nYSize, GDT_Byte, 4, 
                      nXSize * 4 );
                      
        GDALRasterIO( GDALGetRasterBand( hDS, 3 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+2, nXSize, nYSize, GDT_Byte, 4, 
                      nXSize * 4 );
    }
/* -------------------------------------------------------------------- */
/*      Source is pseudocolored.  Load and then convert to RGBA.        */
/* -------------------------------------------------------------------- */
    else if( GDALGetRasterCount(hDS) == 1 
             && GDALGetRasterColorTable( GDALGetRasterBand( hDS, 1 )) != NULL )
    {
        int        i;
        GDALColorTableH hTable;
        GByte      abyPCT[1024];

        /* Load color table, and produce 256 entry table to RGBA. */
        hTable = GDALGetRasterColorTable( GDALGetRasterBand( hDS, 1 ) );

        for( i = 0; i < MIN(256,GDALGetColorEntryCount( hTable )); i++ )
        {
            GDALColorEntry sEntry;

            GDALGetColorEntryAsRGB( hTable, i, &sEntry );
            abyPCT[i*4+0] = sEntry.c1;
            abyPCT[i*4+1] = sEntry.c2;
            abyPCT[i*4+2] = sEntry.c3;
            abyPCT[i*4+3] = sEntry.c4;
        }

        /* Fill in any missing colors with greyscale. */
        for( i = GDALGetColorEntryCount( hTable ); i < 256; i++ )
        {
            abyPCT[i*4+0] = i;
            abyPCT[i*4+1] = i;
            abyPCT[i*4+2] = i;
            abyPCT[i*4+3] = 255;
        }

        /* Read indexed raster */
        GDALRasterIO( GDALGetRasterBand( hDS, 1 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+0, nXSize, nYSize, GDT_Byte, 
                      4, nXSize * 4 );

        /* Convert to RGBA using palette. */
        for( i = nXSize * nYSize - 1; i >= 0; i-- )
        {
            memcpy( pabyRGBABuf + i*4, 
                    abyPCT + pabyRGBABuf[i*4]*4, 
                    4 );
        }
    }
/* -------------------------------------------------------------------- */
/*      Source band is greyscale.  Load it into Red, Green and Blue.    */
/* -------------------------------------------------------------------- */
    else if( GDALGetRasterCount(hDS) == 1 )
    {
        memset( pabyRGBABuf, 255, 4 * nXSize * nYSize );
        
        GDALRasterIO( GDALGetRasterBand( hDS, 1 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+0, nXSize, nYSize, GDT_Byte, 
                      4, nXSize * 4 );
        GDALRasterIO( GDALGetRasterBand( hDS, 1 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+1, nXSize, nYSize, GDT_Byte, 
                      4, nXSize * 4 );
        GDALRasterIO( GDALGetRasterBand( hDS, 1 ), GF_Read, 
                      0, 0, nXSize, nYSize, 
                      pabyRGBABuf+2, nXSize, nYSize, GDT_Byte, 
                      4, nXSize * 4 );
    }

    return pabyRGBABuf;
}

/************************************************************************/
/*                        gv_sm_name_collector()                        */
/*                                                                      */
/*      Callback function used by gv_symbol_manager_get_names().        */
/************************************************************************/

typedef struct {
    char **name_list;
    int  count_so_far;
    int  count_max;
} name_collector_info;

static void gv_sm_name_collector( gpointer key, gpointer value, 
                                  gpointer user_data )

{
    name_collector_info *nci = (name_collector_info *) user_data;

    nci->name_list[nci->count_so_far++] = (char*) key;
}

/************************************************************************/
/*                    gv_symbol_manager_get_names()                     */
/*                                                                      */
/*      Fetch a list of all symbol names on this symbol manager.        */
/*      The list should be freed with g_free() when no longer           */
/*      needed.  The strings it contains are internal.                  */
/************************************************************************/

char **gv_symbol_manager_get_names( GvSymbolManager *manager )

{
    name_collector_info nci;

    nci.count_so_far = 0;
    nci.count_max = g_hash_table_size( manager->symbol_cache );
    nci.name_list = g_new( char *, nci.count_max+1 );
    nci.name_list[nci.count_max] = NULL;
    
    g_hash_table_foreach( manager->symbol_cache, gv_sm_name_collector, 
                          &nci );

    return nci.name_list;
}

