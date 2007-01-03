/******************************************************************************
 * $Id: gvmanager.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Preferences (and other?) manager.  Singleton object.
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
 * $Log: gvmanager.c,v $
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
 * Revision 1.14  2004/02/10 15:38:30  andrey_kiselev
 * Added gv_manager_add_dataset() function.
 *
 * Revision 1.13  2001/07/24 02:21:54  warmerda
 * added 8bit phase averaging
 *
 * Revision 1.12  2001/01/30 14:29:33  warmerda
 * added gv_manager_dump
 *
 * Revision 1.11  2000/09/29 20:32:27  warmerda
 * made decimation the default instead of averaging
 *
 * Revision 1.10  2000/09/27 19:17:13  warmerda
 * use prefs for GvRaster sample method
 *
 * Revision 1.9  2000/08/23 14:16:06  warmerda
 * avoid error message opening for update
 *
 * Revision 1.8  2000/08/09 17:36:50  warmerda
 * update lists when GvRaster destroyed
 *
 * Revision 1.7  2000/07/25 14:18:19  warmerda
 * added task dequeuing support
 *
 * Revision 1.6  2000/06/29 19:50:17  warmerda
 * Initialize busy_changed signal handler.
 *
 * Revision 1.5  2000/06/29 14:38:22  warmerda
 * added busy status, and idle task list
 *
 * Revision 1.4  2000/06/26 15:12:00  warmerda
 * Added dataset/gvraster management
 *
 * Revision 1.3  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvmanager.h"
#include <gtk/gtksignal.h>
#include <stdio.h>
#include <string.h>


enum
{
    PREFERENCES_CHANGED,
    BUSY_CHANGED,
    LAST_SIGNAL
};

static void gv_manager_class_init(GvManagerClass *klass);

static guint manager_signals[LAST_SIGNAL] = { 0 };
static void gv_manager_init( GvManager *manager );
static gint gv_manager_idle_handler( gpointer );

GtkType
gv_manager_get_type(void)
{
    static GtkType manager_type = 0;

    if (!manager_type)
    {
	static const GtkTypeInfo manager_info =
	{
	    "GvManager",
	    sizeof(GvManager),
	    sizeof(GvManagerClass),
	    (GtkClassInitFunc) gv_manager_class_init,
	    (GtkObjectInitFunc) gv_manager_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	manager_type = gtk_type_unique(gtk_object_get_type(),
				    &manager_info);
    }
    return manager_type;
}

static void
gv_manager_class_init(GvManagerClass *klass)
{

    manager_signals[PREFERENCES_CHANGED] =
      g_signal_new ("preferences-changed",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvManagerClass, preferences_changed),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 0);
    manager_signals[BUSY_CHANGED] =
      g_signal_new ("busy-changed",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvManagerClass, busy_changed),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 0);

    /* GTK2 PORT...
    GtkObjectClass *object_class;
    object_class = (GtkObjectClass*) klass;
    manager_signals[PREFERENCES_CHANGED] =
	gtk_signal_new ("preferences-changed",
			GTK_RUN_FIRST,
			object_class->type,
			GTK_SIGNAL_OFFSET (GvManagerClass,preferences_changed),
			gtk_marshal_NONE__POINTER,
			GTK_TYPE_NONE, 0 );
    manager_signals[BUSY_CHANGED] =
	gtk_signal_new ("busy-changed",
			GTK_RUN_FIRST,
			object_class->type,
			GTK_SIGNAL_OFFSET (GvManagerClass,busy_changed),
			gtk_marshal_NONE__POINTER,
			GTK_TYPE_NONE, 0 );
    gtk_object_class_add_signals(object_class, manager_signals, LAST_SIGNAL);
    */

    klass->preferences_changed = NULL;
    klass->busy_changed = NULL;
}

static void 
gv_manager_init( GvManager *manager )

{
    manager->preferences = NULL;
    manager->datasets = g_ptr_array_new();
    manager->idle_tasks = NULL;
    manager->busy_flag = FALSE;
    GDALAllRegister();
}

GvManager *gv_manager_new()
{
    return GV_MANAGER(gtk_type_new(GV_TYPE_MANAGER));
}

GvManager *gv_get_manager()

{
    static GvManager *main_manager = NULL;

    if( main_manager == NULL )
        main_manager = gv_manager_new();

    return main_manager;
}

void gv_manager_dump( GvManager *manager )
{
    FILE	*fp = stderr;
    int		i;
    GvIdleTask	*task;

    fprintf( fp, "GvManager Status Report\n" );
    fprintf( fp, "=======================\n" );

    fprintf( fp, "\n" );
    fprintf( fp, "Preferences:\n" );
    for( i = 0; i < gv_properties_count( &(manager->preferences) ); i++ )
    {
        fprintf( fp, "  %s=%s\n", 
                 gv_properties_get_name_by_index( &(manager->preferences), i),
                 gv_properties_get_value_by_index( &(manager->preferences),i));
    }
    
    fprintf( fp, "\n" );
    fprintf( fp, "Datasets:\n" );
    for( i = 0; i < manager->datasets->len; i++ )
    {
        GvDataset *ds = (GvDataset *) g_ptr_array_index(manager->datasets,i);
        int band;

        fprintf( fp, "  %s:", GDALGetDescription( ds->dataset ) );
        
        for( band = 0; band < GDALGetRasterCount(ds->dataset); band++ )
        {
            if( ds->rasters[band] != NULL )
                fprintf( fp, "R" );
            else
                fprintf( fp, "_" );
        }
        fprintf( fp, "\n" );
    }
    
    fprintf( fp, "\n" );
    fprintf( fp, "Idle Tasks:\n" );
    
    for( task = manager->idle_tasks; task != NULL; task = task->next )
    {
        fprintf( fp, "  %s: priority=%d, cb=%p, cb_data=%p\n",
                 task->name, task->priority, task->callback, task->task_info );
    }
}

GvProperties *gv_manager_get_preferences(GvManager *manager)

{
    return &(manager->preferences);
}

const char *gv_manager_get_preference(GvManager *manager, const char *name)

{
    return gv_properties_get(&(manager->preferences),name);
}

void gv_manager_set_preference(GvManager *manager, 
                               const char *name, const char *value )

{
    /* don't do anything if it is already set */

    if( gv_properties_get( &(manager->preferences), name ) != NULL 
        && strcmp(gv_properties_get(&(manager->preferences),name),value) == 0 )
        return;

    gv_properties_set( &(manager->preferences), name, value );

    gtk_signal_emit(GTK_OBJECT(manager), 
                    manager_signals[PREFERENCES_CHANGED]);
}

/************************************************************************/
/*                        gv_manager_add_dataset()                      */
/************************************************************************/

/**
 * Adds GDALDatasetH object instance to the list of managed datasets.
 *
 * This method adds given GDALDatasetH object instance to the list of
 * managed datasets. Does nothing if this dataset already listed.
 *
 * @param manager Pointer to GvManager object.
 * @param dataset GDALDatasetH dataset handler.
 *
 * @return Handler for added GDALDatasetH object (the same as supplied in
 * dataset parameter) or NULL if parameters are wrong.
 */

GDALDatasetH gv_manager_add_dataset( GvManager *manager, GDALDatasetH dataset )

{
    int       i;
    GvDataset *ds;

    if( dataset == NULL || manager == NULL )
        return NULL;

    /*
     * Check for dataset in existing list of open files.  Note that our
     * filename check does not account for different possible names for
     * one dataset.
     */
    for( i = 0; i < manager->datasets->len; i++ )
    {
        ds = (GvDataset *) g_ptr_array_index(manager->datasets, i);

        if( EQUAL(GDALGetDescription(ds->dataset),GDALGetDescription(dataset)) )
        {
            return ds->dataset;
        }
    }

    /* 
     * Add the dataset to the list of managed datasets. 
     */
    GDALReferenceDataset(dataset);
    ds = g_new(GvDataset,1);
    ds->dataset = dataset;
    ds->rasters = g_new0(GvRaster *, GDALGetRasterCount(dataset));
    
    g_ptr_array_add( manager->datasets, ds );

    return dataset;
}

GDALDatasetH gv_manager_get_dataset( GvManager *manager, const char * filename)

{
    int       i;
    GvDataset *ds;
    GDALDatasetH dataset;

    /*
     * Check for dataset in existing list of open files.  Note that our
     * filename check does not account for different possible names for
     * one dataset.
     */
    for( i = 0; i < manager->datasets->len; i++ )
    {
        ds = (GvDataset *) g_ptr_array_index(manager->datasets, i);

        if( EQUAL(GDALGetDescription(ds->dataset),filename) )
        {
            return ds->dataset;
        }
    }

    /*
     * Try to open the dataset, preferably with update access.  We don't
     * want to report update access errors so we supress error reporting
     * temporarily. 
     */
    
    CPLErrorReset();
    CPLPushErrorHandler( CPLQuietErrorHandler );
    dataset = GDALOpen( filename, GA_Update );
    CPLPopErrorHandler();

    if( dataset == NULL )
    {
        dataset = GDALOpen( filename, GA_ReadOnly );
    }

    if( dataset == NULL )
        return NULL;

    /* 
     * Add the dataset to the list of managed datasets. 
     */
    ds = g_new(GvDataset,1);
    ds->dataset = dataset;
    ds->rasters = g_new0(GvRaster *, GDALGetRasterCount(dataset));
    
    g_ptr_array_add( manager->datasets, ds );

    return dataset;
}

static gint gv_manager_raster_destroy_cb( GtkObject * raster_in, 
                                          gpointer cb_data )

{
    GvManager *manager = GV_MANAGER(cb_data);
    GvRaster  *raster = GV_RASTER(raster_in);
    GvDataset *ds = NULL;
    int       i, active_rasters = 0;

    /* 
     * Find in our list.  The dataset must already be "under management".
     */
    for( i = 0; i < manager->datasets->len; i++ )
    {
        ds = (GvDataset *) g_ptr_array_index(manager->datasets, i);

        if( raster->dataset == ds->dataset )
            break;
    }

    if( i == manager->datasets->len )
    {
        g_warning( "gv_manager_raster_destroy_cb(): can't find dataset." );
        return FALSE;
    }

    /*
     * Find our GvRaster.
     */

    for( i = 0; i < GDALGetRasterCount(ds->dataset); i++ )
    {
        if( ds->rasters[i] == raster )
            ds->rasters[i] = NULL;
        else if( ds->rasters[i] != NULL )
            active_rasters++;
    }

    /*
     * We apparently no longer need this GDALDataset.  Dereference it, and
     * remove from the list. 
     */
    if( active_rasters == 0 )
    {
        if( GDALDereferenceDataset( ds->dataset ) < 1 )
            GDALClose( ds->dataset );
        
        g_free( ds->rasters );
        g_free( ds );
        g_ptr_array_remove_fast( manager->datasets, ds );
    }

    return FALSE;
}

GvRaster *gv_manager_get_dataset_raster( GvManager *manager, 
                                         GDALDatasetH dataset, int band )

{
    int       i;
    GvDataset *ds = NULL;
    GvSampleMethod sm;
    const char *sm_pref;

    if( band < 1 || band > GDALGetRasterCount(dataset) )
        return NULL;

    /* 
     * Find in our list.  The dataset must already be "under management".
     */
    for( i = 0; i < manager->datasets->len; i++ )
    {
        ds = (GvDataset *) g_ptr_array_index(manager->datasets, i);

        if( dataset == ds->dataset )
            break;
    }

    if( i == manager->datasets->len )
    {
        g_warning( "gv_manager_get_dataset_raster called with unmanaged dataset" );
        return NULL;
    }

    /*
     * Figure out the sample method to use from the preferences.
     */
    sm_pref = gv_manager_get_preference( manager, "default_raster_sample");
    if( sm_pref != NULL && EQUAL(sm_pref,"average") )
        sm = GvSMAverage;
    else if( sm_pref != NULL && EQUAL(sm_pref,"average_8bit_phase") )
        sm = GvSMAverage8bitPhase;
    else
        sm = GvSMSample;

    /* 
     * Create a new GvRaster if it doesn't already exist.
     */

    if( ds->rasters[band-1] == NULL )
    {
        GDALRasterBandH gdal_band;

        gdal_band = GDALGetRasterBand( ds->dataset, band );
        if( GDALGetRasterColorInterpretation(gdal_band) == GCI_PaletteIndex )
            sm = GvSMSample;

        ds->rasters[band-1] = GV_RASTER(gv_raster_new( ds->dataset, band, sm));
        gtk_signal_connect(
            GTK_OBJECT(ds->rasters[band-1]), "destroy", 
            GTK_SIGNAL_FUNC(gv_manager_raster_destroy_cb),
            GTK_OBJECT(manager));
    }

    return ds->rasters[band-1];
}

void gv_manager_set_busy( GvManager *manager, int busy_flag )

{
    if( !manager->busy_flag == !busy_flag )
        return;

    if( !manager->busy_flag )
        gtk_idle_add( gv_manager_idle_handler, NULL );

    manager->busy_flag = busy_flag;

    gtk_signal_emit(GTK_OBJECT(manager), 
                    manager_signals[BUSY_CHANGED]);
}

int gv_manager_get_busy( GvManager *manager )

{
    return manager->busy_flag;
}

static gint gv_manager_idle_handler( gpointer cb_data )

{
    GvManager *manager = gv_get_manager();
    GvIdleTask *task;

    if( manager->idle_tasks == NULL )
    {
        gv_manager_set_busy( manager, FALSE );
        return FALSE;
    }

    /* remove task from list for now */
    task = manager->idle_tasks;
    manager->idle_tasks = task->next;
    task->next = NULL;

    if( task->callback( task->task_info ) )
        gv_manager_queue_task( manager, task->name, task->priority, 
                               task->callback, task->task_info );

    g_free( task->name );
    g_free( task );

    return TRUE;
}

void gv_manager_queue_task( GvManager *manager, const char *task_name, 
                            int priority, GtkFunction callback, 
                            void *task_info)

{
    GvIdleTask *task = g_new0( GvIdleTask, 1 );

    task->name = g_strdup( task_name );
    task->priority = priority;
    task->callback = callback;
    task->task_info = task_info;

    if( manager->idle_tasks == NULL 
        || manager->idle_tasks->priority > task->priority )
    {
        task->next = manager->idle_tasks;
        manager->idle_tasks = task;
    }
    else
    {
        GvIdleTask *link;

        for( link = manager->idle_tasks; link != NULL; link = link->next ) 
        {
            if( link->next == NULL )
            {
                link->next = task;
                break;
            }
            else if( link->next->priority > task->priority )
            {
                task->next = link->next;
                link->next = task;
                break;
            }
        }
    }

    gv_manager_set_busy( manager, TRUE );
}

void gv_manager_dequeue_task( GvManager *manager, GvIdleTask *task )

{
    GvIdleTask **prev_ptr = &(manager->idle_tasks);

    while( *prev_ptr != NULL )
    {
        if( *prev_ptr == task )
        {
            *prev_ptr = task->next;
            
            g_free( task->name );
            g_free( task );
        }
        else
            prev_ptr = &((*prev_ptr)->next);
    }
}
