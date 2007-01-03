/******************************************************************************
 * $Id: gvproperties.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
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
 * $Log: gvproperties.c,v $
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
 * Revision 1.5  2002/09/09 16:22:45  warmerda
 * use int instead of gint to avoid glib.h dependency
 *
 * Revision 1.4  2002/07/24 18:06:26  warmerda
 * reimplement properties using quarks
 *
 * Revision 1.3  2000/09/21 02:55:11  warmerda
 * added gv_properties_clear
 *
 * Revision 1.2  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvtypes.h"
#include "cpl_port.h"
#include "gvproperties.h"

/************************************************************************/
/* ==================================================================== */
/*                             GvProperties                             */
/*                                                                      */
/*      Hash table based implementation.                                */
/*                                                                      */
/*      Each GvProperties is a guint32 pointer.  If it is NULL there    */
/*      are no properties otherwise it points to an array of            */
/*      integers:                                                       */
/*                                                                      */
/*        [0] = Max allocated name/values possible.                     */
/*        [1] = Number of name/value property settings.                 */
/*        [2] = key id of first property name.                          */
/*        [3] = quark id of first property values.                      */
/*        ...                                                           */
/*                                                                      */
/*      The property names are kept in a custom case insensitive        */
/*      symbol table and hash table.  The property values (which        */
/*      must be case sensitive) are implemented using normal GLib       */
/*      quarks.                                                         */
/* ==================================================================== */
/************************************************************************/

#ifdef USE_HASH_BASED_GVPROPERTIES

#include <glib.h>

#define	G_QUARK_BLOCK_SIZE			(512)

static GHashTable   *gvpk_keyid_ht = NULL;
static gchar       **gvpk_keyids = NULL;
static GQuark        gvpk_keyid_seq_id = 0;

#define PROP_KEYID(properties,i) ((*properties)[i*2+2])
#define PROP_VALUEID(properties,i) ((*properties)[i*2+3])
#define PROP_COUNT(properties)   ((*properties)[1])
#define PROP_MAXCOUNT(properties)   ((*properties)[0])

/************************************************************************/
/*                           gvpk_str_equal()                           */
/*                                                                      */
/*      Case insensitive string compare.  Used for the property         */
/*      names hash table.                                               */
/************************************************************************/

static gint gvpk_str_equal (gconstpointer v1, gconstpointer v2)

{
    const gchar *string1 = v1;
    const gchar *string2 = v2;

    return EQUAL(string1, string2);
}

/************************************************************************/
/*                           gvpk_str_hash()                            */
/*                                                                      */
/*      31bit hash (like g_str_hash()) except that it is case           */
/*      insensitive.  All upper case letters are implicitly treated     */
/*      as lower case.                                                  */
/************************************************************************/

static guint gvpk_str_hash (gconstpointer key)
{
    const char *p = key;
    guint h = 0, v;

    for (; *p != '\0'; p++)
    {
        v = tolower(*p);
        h = (h << 5) - h + v;
    }
    
    return h;
}

/************************************************************************/
/*                       gvpk_keyid_from_string()                       */
/************************************************************************/

guint32 gvpk_keyid_from_string( const char *string )

{
    guint32 keyid;

    if (gvpk_keyid_ht)
        keyid = (guint32) g_hash_table_lookup (gvpk_keyid_ht, string);
    else
    {
        gvpk_keyid_ht = g_hash_table_new (gvpk_str_hash, gvpk_str_equal);
        keyid = 0;
    }

    /* Does key does exist yet? */
    if (!keyid)
    {
        /* grow key id table if full */
        if (gvpk_keyid_seq_id % G_QUARK_BLOCK_SIZE == 0)
            gvpk_keyids = g_renew (gchar*, gvpk_keyids, 
                                   gvpk_keyid_seq_id + G_QUARK_BLOCK_SIZE);

        /* add keyid to key id table */
        gvpk_keyids[gvpk_keyid_seq_id] = g_strdup(string);
        gvpk_keyid_seq_id++;

        /* Add key to hash table */
        keyid = gvpk_keyid_seq_id;
        g_hash_table_insert (gvpk_keyid_ht, gvpk_keyids[keyid-1], 
                             GUINT_TO_POINTER (keyid));
    }

    return keyid;        
}


/************************************************************************/
/*                         gv_properties_set()                          */
/*                                                                      */
/*      Set a single name/value property set in the list.               */
/************************************************************************/

void gv_properties_set( GvProperties *properties, 
                        const char * name, const char * value )

{
    guint keyid  = gvpk_keyid_from_string( name );
    GQuark valueq = g_quark_from_string( value );
    int   i;
   
/* -------------------------------------------------------------------- */
/*      Initial allocation of properties.                               */
/* -------------------------------------------------------------------- */
    if( *properties == NULL )
    {
        *properties = g_new( guint32, 14 );
        PROP_MAXCOUNT(properties) = 6;
        PROP_COUNT(properties) = 0;
    }

/* -------------------------------------------------------------------- */
/*      Does the key already exist in the properties list?  If so,      */
/*      just reset the value.                                           */
/* -------------------------------------------------------------------- */
    for( i = 0; i < PROP_COUNT(properties); i++ )
    {
        if( PROP_KEYID(properties,i) == keyid )
        {
            PROP_VALUEID(properties,i) = (guint32) valueq;
            return;
        }
    }

/* -------------------------------------------------------------------- */
/*      We need to add the name/value pair to the list.  Does the       */
/*      allocation of the list need to be grown?                        */
/* -------------------------------------------------------------------- */
    if( PROP_MAXCOUNT(properties) == PROP_COUNT(properties) )
    {
        int new_max = (int)(PROP_MAXCOUNT(properties) * 1.5);
	new_max = (new_max < 6 ? 6 : new_max);
        
        *properties = g_renew( guint32, *properties, new_max * 2 + 2 );
        PROP_MAXCOUNT(properties) = new_max; 
    }

/* -------------------------------------------------------------------- */
/*      Add the new name/value pair.                                    */
/* -------------------------------------------------------------------- */
    i = PROP_COUNT(properties);

    PROP_COUNT(properties)++;

    PROP_KEYID(properties,i) = keyid;
    PROP_VALUEID(properties,i) = (guint32) valueq;
}

/************************************************************************/
/*                         gv_properties_get()                          */
/*                                                                      */
/*      Fetch a single property or return NULL on failure.              */
/************************************************************************/

const char * gv_properties_get( GvProperties *properties, const char * name )

{
    guint32 keyid = gvpk_keyid_from_string( name );
    int i;

    if( *properties == NULL )
        return NULL;

    for( i = 0; i < PROP_COUNT(properties); i++ )
    {
        if( PROP_KEYID(properties,i) == keyid )
            return g_quark_to_string( (GQuark) PROP_VALUEID(properties,i) );
    }

    return NULL;
}

/************************************************************************/
/*                        gv_properties_count()                         */
/*                                                                      */
/*      Return the number of name/value properties in this list.        */
/************************************************************************/

int gv_properties_count( GvProperties *properties )

{
    if( *properties == NULL )
        return 0;
    else
        return PROP_COUNT(properties);
}

/************************************************************************/
/*                  gv_properties_get_name_by_index()                   */
/*                                                                      */
/*      Fetch the propert name of the 'nth' entry in the property list. */
/************************************************************************/

const char * gv_properties_get_name_by_index( GvProperties * properties, 
                                              int prop_index )

{
    guint32 keyid;

    if( *properties == NULL )
        return NULL;

    if( prop_index < 0 || prop_index >= PROP_COUNT(properties) )
        return NULL;

    keyid = PROP_KEYID(properties,prop_index);
    
    g_assert( keyid >= 1 && keyid <= gvpk_keyid_seq_id );

    return gvpk_keyids[keyid-1];
}

/************************************************************************/
/*                  gv_properties_get_value_by_index()                  */
/*                                                                      */
/*      Fetch the value of the 'nth' entry in the property list.        */
/************************************************************************/

const char * gv_properties_get_value_by_index( GvProperties * properties, 
                                               int prop_index )

{
    GQuark value_id;

    if( *properties == NULL )
        return NULL;

    if( prop_index < 0 || prop_index >= PROP_COUNT(properties) )
        return NULL;

    value_id = (GQuark) PROP_VALUEID(properties,prop_index);

    return g_quark_to_string( value_id );
}

/************************************************************************/
/*                        g_properties_remove()                         */
/*                                                                      */
/*      Remove the indicated property, if is it present.                */
/************************************************************************/

void gv_properties_remove( GvProperties *properties, const char * key )

{
    guint32 keyid = gvpk_keyid_from_string( key );
    int i;

    if( *properties == NULL )
        return;

    for( i = 0; i < PROP_COUNT(properties); i++ )
    {
        if( PROP_KEYID(properties,i) == keyid )
        {
            int last_prop = PROP_COUNT(properties)-1;

            PROP_KEYID(properties,i) = PROP_KEYID(properties,last_prop);
            PROP_VALUEID(properties,i) = PROP_VALUEID(properties,last_prop);
            PROP_COUNT(properties)--;
            return;
        }
    }
}

/************************************************************************/
/*                         gv_properties_init()                         */
/*                                                                      */
/*      Initialize a GvProperties value.                                */
/************************************************************************/

void gv_properties_init( GvProperties *properties )

{
    *properties = NULL;
}

/************************************************************************/
/*                       gv_properties_destroy()                        */
/*                                                                      */
/*      Wipe whole properties list, recovering all allocation.          */
/************************************************************************/
void gv_properties_destroy( GvProperties *properties )

{
    if( *properties != NULL )
    {
        g_free( *properties );
        *properties = NULL;
    }
}

/************************************************************************/
/*                        gv_properties_clear()                         */
/*                                                                      */
/*      Wipe all properties.                                            */
/************************************************************************/

void gv_properties_clear( GvProperties *properties )

{
    gv_properties_destroy( properties );
}

/************************************************************************/
/*                         gv_properties_copy()                         */
/*                                                                      */
/*      Make an efficient copy of a properties list.  It is assumed     */
/*      that the target GvProperties has not even been initialized.     */
/************************************************************************/

void gv_properties_copy( GvProperties *source, GvProperties *target )

{
    if( *source == NULL )
    {
        *target = NULL;
        return;
    }

    *target = g_new( guint32, PROP_COUNT(source) * 2 + 2 );
    memcpy( *target, *source, sizeof(guint32) * (PROP_COUNT(source)*2 + 2));
    PROP_MAXCOUNT(target) = PROP_COUNT(target);
}

#endif /* notdef USE_QUARK_BASED_GVPROPERTIES */

/************************************************************************/
/* ==================================================================== */
/*                             GvProperties                             */
/*                                                                      */
/*      CPL StringList based implementation.                            */
/* ==================================================================== */
/************************************************************************/

#ifndef USE_HASH_BASED_GVPROPERTIES
#include "cpl_string.h"

void gv_properties_set( GvProperties *properties, 
                        const char * name, const char * value )

{
    *properties = CSLSetNameValue( *properties, name, value );
}

const char * gv_properties_get( GvProperties *properties, const char * name )

{
    return CSLFetchNameValue( *properties, name );
}

int gv_properties_count( GvProperties *properties )

{
    return CSLCount( *properties );
}

const char * gv_properties_get_name_by_index( GvProperties * properties, 
                                              int prop_index )

{
    static char *last_property = NULL;

    if( last_property != NULL )
        CPLFree( last_property );

    last_property = NULL;
    CPLParseNameValue( (*properties)[prop_index], &last_property );

    return last_property;
}

const char * gv_properties_get_value_by_index( GvProperties * properties, 
                                               int prop_index )

{
    return CPLParseNameValue( (*properties)[prop_index], NULL );
}

void gv_properties_remove( GvProperties *properties, const char * key )

{
    int   prop_index = CSLFindString( *properties, key );
    char  **targets;

    if( prop_index >= 0 )
    {
        *properties = CSLRemoveStrings( *properties, prop_index, 1, 
                                        &targets );
        CSLDestroy( targets );
    }
}

void gv_properties_init( GvProperties *properties )

{
    *properties = NULL;
}

void gv_properties_destroy( GvProperties *properties )

{
    if (*properties != NULL) {
      CSLDestroy( *properties );
      *properties = NULL;
    }
}

void gv_properties_clear( GvProperties *properties )

{
    gv_properties_destroy( properties );
}

void gv_properties_copy( GvProperties *source, GvProperties *target )

{
    *target = CSLDuplicate( *source );
}
#endif /* notdef USE_HASH_BASED_GVPROPERTIES */

