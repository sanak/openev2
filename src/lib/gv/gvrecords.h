/******************************************************************************
 * $Id: gvrecords.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  GvRecords declarations.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2003, Frank Warmerdam <warmerdam@pobox.com>
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
 * $Log: gvrecords.h,v $
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
 * Revision 1.5  2003/08/06 22:26:03  warmerda
 * added progress monitor to gv_records load/save funcs
 *
 * Revision 1.4  2003/08/06 17:07:44  warmerda
 * added support to save only selected records in gv_records_to_dbf
 *
 * Revision 1.3  2003/07/27 04:57:28  warmerda
 * complete implementation
 *
 * Revision 1.2  2003/05/23 20:44:25  warmerda
 * fixed header
 *
 * Revision 1.1  2003/05/23 20:43:08  warmerda
 * New
 *
 */

#ifndef __GV_RECORDS_H__
#define __GV_RECORDS_H__

#include "gvdata.h"
#include "gdal.h"

#define GV_TYPE_RECORDS           (gv_records_get_type ())
#define GV_RECORDS(obj)           (GTK_CHECK_CAST ((obj), GV_TYPE_RECORDS, GvRecords))
#define GV_RECORDS_CLASS(klass)   (GTK_CHECK_CLASS_CAST((klass),GV_TYPE_RECORDS, GvRecordsClass))
#define GV_IS_RECORDS(obj)        (GTK_CHECK_TYPE ((obj), GV_TYPE_RECORDS))
#define GV_IS_RECORDS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GV_TYPE_SHAPES))

#define GV_RFT_INTEGER  1
#define GV_RFT_FLOAT    2
#define GV_RFT_STRING   3

#define GV_NULL_MARKER  '\01'

typedef struct _GvRecords      GvRecords;
typedef struct _GvRecordsClass GvRecordsClass;

struct _GvRecordsClass
{
    GvDataClass parent_class;
};

struct _GvRecords
{
    GvData data;

    int nRecordLength;

    int nFieldCount;
    char **papszFieldName;
    int *panFieldOffset;
    int *panFieldWidth;
    int *panFieldType;

    int nRecordCount;

    int nMainDataSize;
    char *pachMainData;

    int nUsedFieldCount;
    int *panUsedFieldList;
};

GtkType gv_records_get_type (void);
GvData* gv_records_new(void);
GvData* gv_records_from_dbf(const char *filename,
                            GDALProgressFunc pfnProgress, void * pCBData);
int gv_records_to_dbf(GvRecords *psRecords, const char *pszFilename,
                      int nSelectionCount, int *panSelectionList,
                      GDALProgressFunc pfnProgress, void *pCBData);
GvData* gv_records_from_rec(const char *filename,
                            GDALProgressFunc pfnProgress, void * pCBData);
gint gv_records_create_records(GvRecords *psRecords, int nNewRecords);
gint gv_records_num_records(GvRecords *psRecords);
gint gv_records_add_field(GvRecords *psRecords, const char *field_name,
                          int field_type, int width, int precision);
const char *gv_records_get_raw_record_data(GvRecords *psRecords, 
                                           int iRecord);
const char *gv_records_get_raw_field_data(GvRecords *psRecords, 
                                          int iRecord, int iField);
void gv_records_set_raw_field_data(GvRecords *psRecords, 
                                   int iRecord, int iField,
                                   const char *pszNewValue);
void gv_records_set_used_properties(GvRecords *psRecords, int nFieldCount,
                                    int *panFieldList);
/*int *gv_records_get_used_properties(GvRecords *, int *pnCount);*/

#endif /* ndef __GV_RECORDS_H__ */






