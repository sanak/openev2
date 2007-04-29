/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Preferences (and other?) manager.  Singleton object.
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

#ifndef __GV_MANAGER_H__
#define __GV_MANAGER_H__

#include "gvtypes.h"
#include "gvproperties.h"
#include "gvraster.h"

#define GV_TYPE_MANAGER              (gv_manager_get_type ())
#define GV_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GV_TYPE_MANAGER, GvManager))
#define GV_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GV_TYPE_MANAGER, GvManagerClass))
#define GV_IS_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GV_TYPE_MANAGER))
#define GV_IS_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GV_TYPE_MANAGER))

typedef struct _GvManager       GvManager;
typedef struct _GvManagerClass  GvManagerClass;

typedef struct {
    GDALDatasetH  dataset;

    GvRaster     **rasters;
} GvDataset;

typedef struct _idle_task {
    struct _idle_task *next;
    char         *name;
    int          priority;
    GtkFunction  callback;
    void         *task_info;
} GvIdleTask;

struct _GvManager
{
    GObject  object;

    GvProperties preferences;

    GPtrArray *datasets;

    int        busy_flag;    /* TRUE if busy */

    GvIdleTask*idle_tasks; 
};

struct _GvManagerClass
{
    GObjectClass parent_class;

    void (* preferences_changed)(GvManager *man);
    void (* busy_changed)(GvManager *man);
};

GType      gv_manager_get_type   (void);
GvManager* gv_manager_new        (void);
GvManager* gv_get_manager        (void);

void gv_manager_dump(GvManager *manager);
const char* gv_manager_get_preference(GvManager *manager, const char *name);
GvProperties* gv_manager_get_preferences(GvManager *manager);
void gv_manager_set_preference(GvManager *manager, const char *name, const char *value);
GDALDatasetH gv_manager_add_dataset(GvManager *manager, GDALDatasetH dataset);
GDALDatasetH gv_manager_get_dataset(GvManager *manager, const char *filename);
GvRaster *gv_manager_get_dataset_raster(GvManager *manager,
                                        GDALDatasetH dataset, int band);
void       gv_manager_set_busy(GvManager *manager, int busy_flag);
int        gv_manager_get_busy(GvManager *manager);
void       gv_manager_queue_task(GvManager *manager,
                                 const char *task_name, int priority,
                                 GtkFunction callback, void *task_info);
void       gv_manager_dequeue_task(GvManager *manager, GvIdleTask *task);

#endif /* __GV_MANAGER_H__ */
