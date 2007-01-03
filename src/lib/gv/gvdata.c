/******************************************************************************
 * $Id: gvdata.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Base class for raster, vector and layer data containers.
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
 * $Log: gvdata.c,v $
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
 * Revision 1.15  2003/02/07 20:06:49  andrey_kiselev
 * Memory leaks fixed.
 *
 * Revision 1.14  2001/08/08 02:57:03  warmerda
 * implemented support for gv_data_registry_dump()
 *
 * Revision 1.13  2001/06/20 14:02:45  warmerda
 * avoid emitting signal in gv_data_set_name if object is destroyed
 *
 * Revision 1.12  2001/05/15 16:22:13  pgs
 * added meta-changed signal for notification of\nchanges to meta data
 *
 * Revision 1.11  2000/08/08 20:09:06  warmerda
 * cleanup properties and parents
 *
 * Revision 1.10  2000/07/21 01:31:11  warmerda
 * added read_only flag for GvData, and utilize for vector layers
 *
 * Revision 1.9  2000/06/20 13:26:54  warmerda
 * added standard headers
 *
 */

#include "gvdata.h"
#include <gtk/gtksignal.h>
#include <stdio.h>

enum
{
    CHANGING,
    CHANGED,
    META_CHANGED,
    LAST_SIGNAL
};

static void gv_data_class_init(GvDataClass *klass);
static void gv_data_init(GvData *data);
static void gv_data_parent_changed(GvData *data, gpointer change_info);
static void gv_data_child_changed(GvData *data, GvData *child, gpointer change_info);
static void gv_data_destroy(GtkObject *object);
static void gv_data_finalize(GObject *gobject);

static guint data_signals[LAST_SIGNAL] = { 0 };

static GPtrArray *live_datasets = NULL;

GtkType
gv_data_get_type(void)
{
    static GtkType data_type = 0;

    if (!data_type)
    {
	static const GtkTypeInfo data_info =
	{
	    "GvData",
	    sizeof(GvData),
	    sizeof(GvDataClass),
	    (GtkClassInitFunc) gv_data_class_init,
	    (GtkObjectInitFunc) gv_data_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	data_type = gtk_type_unique(gtk_object_get_type(), &data_info);
    }
    return data_type;
}

static void
gv_data_class_init(GvDataClass *klass)
{

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

    /* ---- Override finalize ---- */
    (G_OBJECT_CLASS(klass))->finalize = gv_data_finalize;

    ((GtkObjectClass *) klass)->destroy = gv_data_destroy;

    /* GTK2 PORT...
    GtkObjectClass *object_class;
    object_class = (GtkObjectClass*) klass;
    data_signals[CHANGING] =
	gtk_signal_new ("changing",
			GTK_RUN_FIRST,
			object_class->type,
			GTK_SIGNAL_OFFSET (GvDataClass, changing),
			gtk_marshal_NONE__POINTER,
			GTK_TYPE_NONE, 1,
			GTK_TYPE_POINTER);
    data_signals[CHANGED] =
	gtk_signal_new ("changed",
			GTK_RUN_FIRST,
			object_class->type,
			GTK_SIGNAL_OFFSET (GvDataClass, changed),
			gtk_marshal_NONE__POINTER,
			GTK_TYPE_NONE, 1,
			GTK_TYPE_POINTER);
	data_signals[META_CHANGED] =
	gtk_signal_new ("meta-changed",
			GTK_RUN_FIRST,
			object_class->type,
			GTK_SIGNAL_OFFSET (GvDataClass, meta_changed),
			gtk_marshal_NONE__POINTER,
			GTK_TYPE_NONE, 0);
    gtk_object_class_add_signals(object_class, data_signals, LAST_SIGNAL);
    object_class->destroy = gv_data_destroy;
    object_class->finalize = gv_data_finalize;
    */

    klass->changing = NULL;
    klass->changed = NULL;
    klass->meta_changed = NULL;
    klass->child_changed = NULL;
    klass->get_memento = NULL;
    klass->set_memento = NULL;
    klass->del_memento = NULL;
}

void gv_data_registry_dump()

{
    int		i;

    printf( "gv_data registry dump\n" );
    printf( "=====================\n" );

    if( live_datasets == NULL )
    {
        printf( "no GvDatas created yet\n" );
        return;
    }

    for( i = 0; i < live_datasets->len; i++ )
    {
        GvData	*data = GV_DATA(g_ptr_array_index(live_datasets,i));

        printf( "  %-24s %p/%s\n", 
                gtk_type_name(GTK_OBJECT_TYPE( GTK_OBJECT(data) )),
                data, 
                gv_data_get_name( data ) );
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
	/* Remove reference to parent */
	gtk_signal_disconnect_by_data(GTK_OBJECT(data->parent),
				      GTK_OBJECT(data));
	gtk_object_unref(GTK_OBJECT(data->parent));
    }

    data->parent = parent;

    if (parent)
    {
	/* Each GvData maintains a reference to its parent */
	gtk_object_ref(GTK_OBJECT(parent));
	gtk_object_sink(GTK_OBJECT(parent));

	gtk_signal_connect_object(GTK_OBJECT(parent), "changed",
				  GTK_SIGNAL_FUNC(gv_data_parent_changed),
				  GTK_OBJECT(data));
	if (!data->name)
	{
	    gv_data_set_name(data, parent->name);
	}
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
    return gv_properties_get( &(data->properties), name );
}

void
gv_data_set_property(GvData *data, const char *name, const char *value)
{
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
	    gtk_signal_emit(GTK_OBJECT(data), data_signals[CHANGING],
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
	    gtk_signal_emit(GTK_OBJECT(data), data_signals[CHANGED],
			    change_info);
	}
    }
}

void
gv_data_meta_changed(GvData *data)
{
	//do we need to any tests at this point?

	gtk_signal_emit(GTK_OBJECT(data), data_signals[META_CHANGED]);
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

    GvDataClass *klass = GV_DATA_CLASS(GTK_OBJECT_GET_CLASS(data));

    /* GTK2 PORT...
    GvDataClass *klass = GV_DATA_CLASS(((GtkObject*)data)->klass);
    */
    GvDataMemento *memento = NULL;

    if (klass->get_memento)
    {
	klass->get_memento(data, change_info, &memento);
    }
    return memento;
}

void
gv_data_set_memento(GvData *data, GvDataMemento *memento)
{

    GvDataClass *klass = GV_DATA_CLASS(GTK_OBJECT_GET_CLASS(data));

    /* GTK2 PORT...
    GvDataClass *klass = GV_DATA_CLASS(((GtkObject*)data)->klass);
    */
    g_return_if_fail(memento);

    if (klass->set_memento)
    {
	klass->set_memento(data, memento);
    }
}

void
gv_data_del_memento(GvData *data, GvDataMemento *memento)
{

    GvDataClass *klass = GV_DATA_CLASS(GTK_OBJECT_GET_CLASS(data));

    /* GTK2 PORT...
    GvDataClass *klass = GV_DATA_CLASS(((GtkObject*)data)->klass);
    */
    g_return_if_fail(memento);

    if (klass->del_memento)
    {
	klass->del_memento(data, memento);
    }
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
	gtk_signal_emit(GTK_OBJECT(data), data_signals[CHANGED],
			change_info);
    }
}

static void
gv_data_child_changed(GvData *data, GvData *child, gpointer change_info)
{

    GvDataClass *klass = GV_DATA_CLASS(GTK_OBJECT_GET_CLASS(data));

    /* GTK2 PORT...
    GvDataClass *klass = GV_DATA_CLASS(((GtkObject*)data)->klass);
    */
    if (klass->child_changed)
    {
	klass->child_changed(data, child, change_info);
    }

    gv_data_changed(data, change_info);
}

static void
gv_data_destroy(GtkObject *object)
{
    GtkObjectClass *parent_class;

    /* Remove reference to parent */
    gv_data_set_parent(GV_DATA(object), NULL);

    parent_class = gtk_type_class(gtk_object_get_type());
    GTK_OBJECT_CLASS(parent_class)->destroy(object);
}

static void
gv_data_finalize(GObject *gobject)
{
    GtkObjectClass *parent_class;

    /* GTK2 PORT... Override GObject finalize, test for and set NULLs
       as finalize may be called more than once. */

    gv_data_set_name(GV_DATA(gobject), NULL);
    gv_data_set_projection(GV_DATA(gobject), NULL);
    gv_properties_destroy(&(GV_DATA(gobject)->properties));
    gv_data_set_parent(GV_DATA(gobject), NULL);

    /* Call parent class finalize */
    parent_class = gtk_type_class(gtk_object_get_type());
    G_OBJECT_CLASS(parent_class)->finalize(gobject);

    /*
    GTK_OBJECT_CLASS(parent_class)->finalize(object);
    */

    g_ptr_array_remove( live_datasets, gobject );
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

