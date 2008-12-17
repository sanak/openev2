/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Base class for raster, vector and layer data containers.
 * Author:   Frank Warmerdam, warmerda@home.com
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

#include "gvdata.h"
#include <stdio.h>

enum
{
    CHANGING,
    CHANGED,
    META_CHANGED,
    DESTROY,
    LAST_SIGNAL
};

static void gv_data_class_init(GvDataClass *klass);
static void gv_data_init(GvData *data);
static void gv_data_parent_changed(GvData *data, gpointer change_info);
static void gv_data_child_changed(GvData *data, GvData *child, gpointer change_info);
static void gv_data_dispose(GObject *gobject);
static void gv_data_finalize(GObject *gobject);

static guint data_signals[LAST_SIGNAL] = { 0 };

static GObjectClass *parent_class = NULL;
static GPtrArray *live_datasets = NULL;

GType
gv_data_get_type(void)
{
    static GType data_type = 0;

    if (!data_type) {
        static const GTypeInfo data_info =
        {
            sizeof(GvDataClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_data_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvData),
            0,
            (GInstanceInitFunc) gv_data_init,
        };
        data_type = g_type_register_static (G_TYPE_OBJECT,
                                          "GvData",
                                          &data_info, 0);
        }

    return data_type;
}

static void
gv_data_class_init(GvDataClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    data_signals[CHANGING] =
      g_signal_new ("changing",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvDataClass, changing),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);
    data_signals[CHANGED] =
      g_signal_new ("changed",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvDataClass, changed),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
                    G_TYPE_POINTER);
    data_signals[META_CHANGED] =
      g_signal_new ("meta-changed",
                    G_TYPE_FROM_CLASS (klass),
                    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                    G_STRUCT_OFFSET (GvDataClass, meta_changed),
                    NULL, NULL,
                    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 0);

    data_signals[DESTROY] =
      g_signal_new ("destroy",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (GvDataClass, destroy),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

    object_class->finalize = gv_data_finalize;
    object_class->dispose = gv_data_dispose;

    klass->changing = NULL;
    klass->changed = NULL;
    klass->meta_changed = NULL;
    klass->child_changed = NULL;
    klass->get_memento = NULL;
    klass->set_memento = NULL;
    klass->del_memento = NULL;
    klass->destroy = NULL;
}

void gv_data_registry_dump()
{
    int         i;

    printf( "gv_data registry dump\n" );
    printf( "=====================\n" );

    if( live_datasets == NULL )
    {
        printf( "no GvDatas created yet\n" );
        return;
    }

    for( i = 0; i < live_datasets->len; i++ )
    {
        GvData  *data = GV_DATA(g_ptr_array_index(live_datasets,i));

        printf( "  %-24s %p/%s: %d references\n", 
                g_type_name (G_TYPE_FROM_INSTANCE (data)),
                data, 
                gv_data_get_name( data ),
                G_OBJECT(data)->ref_count);
    }

    printf( "\n" );
}

static void
gv_data_init(GvData *data)
{
    data->parent = NULL;
    data->name = NULL;
    data->frozen = FALSE;
    data->changed_while_frozen = FALSE;
    data->read_only = FALSE;
    data->projection = NULL;
    data->properties = NULL;

    if( live_datasets == NULL )
        live_datasets = g_ptr_array_new();

    g_ptr_array_add( live_datasets, data );
}

GvData *
gv_data_get_parent(GvData *data)
{
    return data->parent;
}

void
gv_data_set_parent(GvData *data, GvData *parent)
{
    if (data->parent)
    {
        g_signal_handlers_disconnect_matched (data->parent, G_SIGNAL_MATCH_DATA,
                                                0, 0, NULL, NULL, data);
        /* Remove reference to parent */
        g_object_unref(data->parent);
    }

    if (parent)
    {
        /* Each GvData maintains a reference to its parent */
        data->parent = g_object_ref(parent);

        g_signal_connect_swapped(parent, "changed",
                                G_CALLBACK (gv_data_parent_changed),
                                data);
        if (!data->name)
            gv_data_set_name(data, parent->name);
    }
    else
    {
        data->parent = NULL;
    }
}

void
gv_data_set_name(GvData *data, const gchar *name)
{
    if (data->name)
    {
        g_free(data->name);
    }
    if (name)
    {
        data->name = g_strdup(name);
    }
    else
    {
        data->name = NULL;
    }

    /* MB: the following comment is by Vexcel...*/
    /* GTK2 PORT - GTK_OBJECT_DESTOYED supposedly no longer makes sense
       because the destroy signal can be emitted multiple times, and the
       replacement for destruction checking is a signal connection to
       ::destroy.  FIXME - May need to figure out how to check for 
       destroyed if necessary.. */

    /*
    if( !GTK_OBJECT_DESTROYED(data) )
        gv_data_meta_changed(data);
    */

    if (data->name != NULL) {
      gv_data_meta_changed(data);
    }
}

const gchar *
gv_data_get_name(GvData *data)
{
    return (const gchar *)data->name;
}

const char *
gv_data_get_property(GvData *data, const char *name)
{
    /* MB: could we use gobject.get_property()? */
    return gv_properties_get( &(data->properties), name );
}

void
gv_data_set_property(GvData *data, const char *name, const char *value)
{
    /* MB: could we use gobject.set_property()? */
    gv_properties_set( &(data->properties), name, value );
}

GvProperties *
gv_data_get_properties(GvData *data)
{
    return &(data->properties);
}

void
gv_data_changing(GvData *data, gpointer change_info)
{
    if (!data->frozen)
    {
        if (data->parent)
        {
            gv_data_changing(data->parent, change_info);
        }
        else
        {
            g_signal_emit(data, data_signals[CHANGING], 0,
                            change_info);
        }
    }
}

void
gv_data_changed(GvData *data, gpointer change_info)
{
    if (data->frozen)
    {
        data->changed_while_frozen = TRUE;
    }
    else
    {
        if (data->parent)
        {
            gv_data_child_changed(data->parent, data, change_info);
        }
        else
        {
            g_signal_emit(data, data_signals[CHANGED], 0,
                            change_info);
        }
    }
}

void
gv_data_meta_changed(GvData *data)
{
        //do we need to any tests at this point?

        g_signal_emit(data, data_signals[META_CHANGED], 0);
}

void
gv_data_freeze(GvData *data)
{
    data->frozen = TRUE;
    data->changed_while_frozen = FALSE;
}

void
gv_data_thaw(GvData *data)
{
    if (data->frozen)
    {
        data->frozen = FALSE;
        if (data->changed_while_frozen)
        {
            gv_data_changed(data, NULL);
        }
    }
}

GvDataMemento *
gv_data_get_memento(GvData *data, gpointer change_info)
{
    GvDataClass *klass = GV_DATA_CLASS(G_OBJECT_GET_CLASS(data));
    GvDataMemento *memento = NULL;

    if (klass->get_memento)
        klass->get_memento(data, change_info, &memento);

    return memento;
}

void
gv_data_set_memento(GvData *data, GvDataMemento *memento)
{
    GvDataClass *klass = GV_DATA_CLASS(G_OBJECT_GET_CLASS(data));

    g_return_if_fail(memento);

    if (klass->set_memento)
        klass->set_memento(data, memento);
}

void
gv_data_del_memento(GvData *data, GvDataMemento *memento)
{
    GvDataClass *klass = GV_DATA_CLASS(G_OBJECT_GET_CLASS(data));

    g_return_if_fail(memento);

    if (klass->del_memento)
        klass->del_memento(data, memento);
}

static void
gv_data_parent_changed(GvData *data, gpointer change_info)
{
    if (data->frozen)
    {
        data->changed_while_frozen = TRUE;
    }
    else
    {
        g_signal_emit(data, data_signals[CHANGED], 0,
                        change_info);
    }
}

static void
gv_data_child_changed(GvData *data, GvData *child, gpointer change_info)
{
    GvDataClass *klass = GV_DATA_CLASS(G_OBJECT_GET_CLASS(data));

    if (klass->child_changed)
        klass->child_changed(data, child, change_info);

    gv_data_changed(data, change_info);
}

static void
gv_data_dispose(GObject *gobject)
{
    /* MB: not sure what should go in finalize and vice-versa */
    /* Remove reference to parent */
    g_signal_emit(gobject, data_signals[DESTROY], 0);
    gv_data_set_parent(GV_DATA(gobject), NULL);

    G_OBJECT_CLASS (parent_class)->dispose (gobject);
}

void
gv_data_destroy(GvData *data)
{
    g_signal_emit(data, data_signals[DESTROY], 0);
}

static void
gv_data_finalize(GObject *gobject)
{
    /* MB: not sure what should go in dispose and vice-versa */
    gv_data_set_name(GV_DATA(gobject), NULL);
    gv_data_set_projection(GV_DATA(gobject), NULL);
    gv_properties_destroy(&(GV_DATA(gobject)->properties));

    g_ptr_array_remove( live_datasets, gobject );

    /* Call parent class finalize */
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}

const char *
gv_data_get_projection(GvData *data)
{
    return data->projection;
}

void
gv_data_set_projection(GvData *data, const char *projection)

{
    if( data->projection != NULL )
    {
        g_free( data->projection );
        data->projection = NULL;
    }

    if( projection != NULL )
        data->projection = g_strdup(projection);
}

gint
gv_data_is_read_only(GvData *data)
{
    return data->read_only;
}

void
gv_data_set_read_only(GvData *data, int read_only)
{
    data->read_only = read_only;
}
