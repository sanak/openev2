/******************************************************************************
 * $Id: gvrecords.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  GvRecords implementation (GvShapes like collection of records)
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
 * $Log: gvrecords.c,v $
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
 * Revision 1.2  2003/06/23 15:51:28  warmerda
 * added header
 *
 * Revision 1.1  2003/05/23 20:43:08  warmerda
 * New
 *
 */

#include "gvrecords.h"
#include "cpl_error.h"
#include "cpl_vsi.h"
#include "shapefil.h"

static void gv_records_class_init(GvRecordsClass *klass);
static void gv_records_init(GvRecords *points);
static void gv_records_finalize(GObject *gobject);

GtkType
gv_records_get_type(void)
{
    static GtkType type = 0;

    if (!type)
    {
	static const GtkTypeInfo records_info =
	{
	    "GvRecords",
	    sizeof(GvRecords),
	    sizeof(GvRecordsClass),
	    (GtkClassInitFunc) gv_records_class_init,
	    (GtkObjectInitFunc) gv_records_init,
	    /* reserved_1 */ NULL,
	    /* reserved_2 */ NULL,
	    (GtkClassInitFunc) NULL,
	};

	type = gtk_type_unique(gv_data_get_type(), &records_info);
    }
    return type;
}

static void
gv_records_init(GvRecords *records)
{
    records->nRecordLength = 0;

    records->nFieldCount = 0;
    records->panFieldOffset = NULL;
    records->panFieldWidth = NULL;
    records->panFieldType = NULL;
    
    records->nRecordCount = 0;
    
    records->nMainDataSize = 0;
    records->pachMainData = NULL;
    
    records->nUsedFieldCount = 0;
    records->panUsedFieldList = NULL;
}

static void
gv_records_class_init(GvRecordsClass *klass)
{
    GtkObjectClass *object_class;
    GvDataClass *data_class;

    object_class = (GtkObjectClass*) klass;
    data_class = (GvDataClass*) klass;

    /* ---- Override finalize ---- */
    (G_OBJECT_CLASS(klass))->finalize = gv_records_finalize;

    /* GTK2 PORT...
    object_class->finalize = gv_records_finalize;
    */
}

GvData *
gv_records_new(void)
{
    GvData *data;

    data = GV_DATA(gtk_type_new(gv_records_get_type()));

    return data;
}

/************************************************************************/
/*                     gv_records_create_records()                      */
/*                                                                      */
/*      Creates the requested number of new records, all initialized    */
/*      to NULL values.  Returns the record index of the first new      */
/*      record.                                                         */
/************************************************************************/

gint gv_records_create_records( GvRecords *psRecords, int nNewRecords )

{
    int nFirstRecord = psRecords->nRecordCount;

    if( psRecords->nRecordLength * (psRecords->nRecordCount + nNewRecords) 
        > psRecords->nMainDataSize )
    {
        int nNewMainSize = 
            psRecords->nRecordLength * (psRecords->nRecordCount + nNewRecords)
            + 100000;

        psRecords->pachMainData = 
            g_renew( char, psRecords->pachMainData, nNewMainSize );
        psRecords->nMainDataSize = nNewMainSize;
    }

    psRecords->nRecordCount += nNewRecords;

    memset( (char *) gv_records_get_raw_record_data( psRecords, nFirstRecord ),
            GV_NULL_MARKER, nNewRecords * psRecords->nRecordLength );

    return nFirstRecord;
}

/************************************************************************/
/*                          gv_record_to_dbf()                          */
/************************************************************************/

int gv_records_to_dbf( GvRecords *psRecords, const char *pszFilename,
                       int nSelectionCount, int *panSelectionList,
                       GDALProgressFunc pfnProgress, void * pCBData )

{
    DBFHandle      dbf_handle;
    int            field_index, field_count=0, shape_index, i;
    GvProperties   *properties;

    if( pfnProgress == NULL )
        pfnProgress = GDALDummyProgress;

    if( !pfnProgress( 0.0, "", pCBData ) )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Try to create the named file(s).                                */
/* -------------------------------------------------------------------- */
    dbf_handle = DBFCreate( pszFilename );
    if( dbf_handle == NULL )
    {
        g_warning( "Failed to create DBF file." );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Create the fields on the DBF file, if any.                      */
/* -------------------------------------------------------------------- */
    properties = gv_data_get_properties( GV_DATA(psRecords) );
    for( field_index = 0; TRUE; field_index++ )
    {
        int  width, precision = 0, field_type;
        char prop_name[64];
        const char *prop_value;

        sprintf( prop_name, "_field_width_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value == NULL )
            break;

        width = atoi(prop_value);

        sprintf( prop_name, "_field_precision_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value != NULL )
            precision = atoi(prop_value);

        sprintf( prop_name, "_field_type_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value == NULL )
            prop_value = "string";

        if( g_strcasecmp(prop_value,"integer") == 0 )
            field_type = FTInteger;
        else if( g_strcasecmp(prop_value,"float") == 0 )
            field_type = FTDouble;
        else
            field_type = FTString;

        sprintf( prop_name, "_field_name_%d", field_index+1 );
        prop_value = gv_properties_get( properties, prop_name );
        if( prop_value == NULL )
            break;

        DBFAddField( dbf_handle, prop_value, field_type, width, precision );
        field_count++;
    }

/* -------------------------------------------------------------------- */
/*      Add a dummy field if there are none.                            */
/* -------------------------------------------------------------------- */
    if( field_count == 0 )
    {
        g_warning( "No attributes to save in DBF file." );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Start writing shapes, ignoring any that don't match our         */
/*      desired type.                                                   */
/* -------------------------------------------------------------------- */
    if( panSelectionList == NULL )
        nSelectionCount = gv_records_num_records(psRecords);

    for( i = 0; i < nSelectionCount; i++ )
    {
        if( panSelectionList != NULL )
            shape_index = panSelectionList[i];
        else
            shape_index = i;

        /* Write the attributes of this shape that match the DBF schema */
        for( field_index = 0; field_index < field_count; field_index++ )
        {
            char field_name[32];
            const char * field_value;
            int  field_type;

            /* FIXME: This will fail for truncated field names! */
            field_type = DBFGetFieldInfo( dbf_handle, field_index,
                                          field_name, NULL, NULL);

            field_value = 
                gv_records_get_raw_field_data( psRecords, shape_index,
                                               field_index);

            if( field_value == NULL )
            {
                DBFWriteNULLAttribute( dbf_handle, i, field_index );
            }
            else
            {
                if( field_type == FTDouble )
                    DBFWriteDoubleAttribute( dbf_handle, i, field_index,
                                             atof(field_value) );
                else if( field_type == FTInteger )
                    DBFWriteIntegerAttribute( dbf_handle, i, field_index,
                                              atoi(field_value) );
                else
                    DBFWriteStringAttribute( dbf_handle, i, field_index,
                                             field_value );
            }
        }

        if( !pfnProgress( (i+1) / (double) nSelectionCount, "", pCBData ) )
        {
            DBFClose( dbf_handle );
            VSIUnlink( pszFilename );
            return FALSE;
        }
    }

    DBFClose( dbf_handle );

    return TRUE;
}

/************************************************************************/
/*                        gv_records_from_dbf()                         */
/************************************************************************/

GvData* gv_records_from_dbf( const char *filename,
                             GDALProgressFunc pfnProgress, void * pCBData )

{
    DBFHandle   dbf_handle;
    int         shape_count, shape_index, field_count = 0, field_index;
    GvRecords   *shapes_data;
    GvProperties *properties;
    int         cancelled = FALSE;

    if( pfnProgress == NULL )
        pfnProgress = GDALDummyProgress;

    if( !pfnProgress( 0.0, "", pCBData ) )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Open the .shp and .dbf file.                                    */
/* -------------------------------------------------------------------- */
    dbf_handle = DBFOpen( filename, "rb" );
    if( dbf_handle == NULL )
    {
        g_warning( "Invalid DBF." );
        return NULL;
    }
    else
        field_count = DBFGetFieldCount( dbf_handle );

    shape_count = DBFGetRecordCount( dbf_handle );

/* -------------------------------------------------------------------- */
/*      Create shapes layer, and assign some metadata about the         */
/*      field definitions.                                              */
/* -------------------------------------------------------------------- */
    shapes_data = GV_RECORDS(gv_records_new());
    properties = gv_data_get_properties( GV_DATA(shapes_data) );

    //set the filename property
    gv_properties_set( properties, "_filename", filename );

    for(field_index = 0; field_index < field_count; field_index++ )
    {
        char      prop_value[64];
        int       field_type, width, precision;
        int       rfld_type;

        field_type = DBFGetFieldInfo( dbf_handle, field_index,
                                      prop_value, &width, &precision );

        if( field_type == FTInteger )
            rfld_type = GV_RFT_INTEGER;
        else if( field_type == FTDouble )
            rfld_type = GV_RFT_FLOAT;
        else 
            rfld_type = GV_RFT_STRING;

        gv_records_add_field( shapes_data, prop_value, rfld_type,
                              width, precision );
    }

/* -------------------------------------------------------------------- */
/*      Copy all the shapes, and their attributes.                      */
/* -------------------------------------------------------------------- */
    gv_records_create_records( shapes_data, shape_count );

    for( shape_index = 0; 
         shape_index < shape_count && !cancelled; 
         shape_index++ )
    {
        for( field_index = 0; field_index < field_count; field_index++ )
        {
            const char  *field_value;
            
            if( DBFIsAttributeNULL(dbf_handle,shape_index,field_index) )
                field_value = NULL;
            else
                field_value = DBFReadStringAttribute( dbf_handle, shape_index,
                                                      field_index );

            gv_records_set_raw_field_data( shapes_data, shape_index, 
                                           field_index, field_value );
        }

        cancelled = 
            !pfnProgress( (shape_index+1) / (double) shape_count, 
                          "", pCBData );
    }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    if( dbf_handle != NULL )
        DBFClose( dbf_handle );

    if( cancelled )
    {
        gtk_object_unref( GTK_OBJECT(shapes_data) );
        return NULL;
    }
    else
    {
        gv_data_set_name( GV_DATA(shapes_data), filename );
        
        return GV_DATA(shapes_data);
    }
}

CPL_C_START
int CPL_DLL RECGetFieldCount( FILE *fp);
int CPL_DLL RECGetFieldDefinition( FILE *fp, char *pszFieldName, int *pnType, 
                                   int *pnWidth, int *pnPrecision );
int CPL_DLL RECReadRecord( FILE *fp, char *pszRecBuf, int nRecordLength  );
const char CPL_DLL *RECGetField( const char *pszSrc, int nStart, int nWidth );
CPL_C_END

/************************************************************************/
/*                        gv_records_from_rec()                         */
/************************************************************************/

GvData* gv_records_from_rec( const char *filename,
                             GDALProgressFunc pfnProgress, void * pCBData )

{
    int         record_index, field_count = 0, field_index;
    int         *foffset, *fwidth, cancelled = FALSE;
    GvRecords   *records_data;
    GvProperties *properties;
    FILE        *fpRec;
    long        first_record_offset, all_records_length;
    int         record_length = 0, raw_field_count, approx_record_count;
    char        *raw_record;

    if( pfnProgress == NULL )
        pfnProgress = GDALDummyProgress;

    if( !pfnProgress( 0.0, "", pCBData ) )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Open the .rec file.                                             */
/* -------------------------------------------------------------------- */
    fpRec = VSIFOpen( filename, "rb" );
    if( fpRec == NULL )
    {
        g_warning( "Unable to open requested .rec file." );
        return NULL;
    }

    raw_field_count = RECGetFieldCount( fpRec );
    if( raw_field_count < 1 )
    {
        VSIFClose( fpRec );
        g_warning( "Unable to get field count from .rec, corrupt?" );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Create shapes layer, and assign some metadata about the         */
/*      field definitions.                                              */
/* -------------------------------------------------------------------- */
    foffset = (int *) g_new(int,raw_field_count);
    fwidth = (int *) g_new(int,raw_field_count);

    records_data = GV_RECORDS(gv_records_new());
    properties = gv_data_get_properties( GV_DATA(records_data) );

    //set the filename property
    gv_properties_set( properties, "_filename", filename );

    for(field_index = 0, field_count = 0, record_length = 0; 
        field_index < raw_field_count; field_index++ )
    {
        char      fieldname[128];
        int       field_type, width, precision;
        int       rfld_type;

        if( !RECGetFieldDefinition( fpRec, fieldname, &field_type, 
                                    &width, &precision ) )
        {
            g_warning( "corrupt field definition line." );
            return NULL;
        }

        if( width == 0 )
            continue;

        if( field_type == 0 /* OFTInteger */ )
            rfld_type = GV_RFT_INTEGER;
        else if( field_type == 2 /* OFTReal */ )
            rfld_type = GV_RFT_FLOAT;
        else 
            rfld_type = GV_RFT_STRING;

        gv_records_add_field( records_data, fieldname, rfld_type,
                              width, precision );

        fwidth[field_count] = width;
        foffset[field_count++] = record_length;
        record_length += width;
    }

/* -------------------------------------------------------------------- */
/*      Compute the approximate number of records in the file.          */
/* -------------------------------------------------------------------- */
    first_record_offset = VSIFTell( fpRec );
    VSIFSeek( fpRec, 0, SEEK_END );

    all_records_length = VSIFTell( fpRec ) - first_record_offset;
    VSIFSeek( fpRec, first_record_offset, SEEK_SET );

    approx_record_count = all_records_length / record_length;

/* -------------------------------------------------------------------- */
/*      Copy all the shapes, and their attributes.                      */
/* -------------------------------------------------------------------- */
    gv_records_create_records( records_data, approx_record_count );
    raw_record = g_new( char, record_length + 10 );

    for( record_index = 0; 
         record_index < approx_record_count && !cancelled; 
         record_index++ )
    {
        if( !RECReadRecord( fpRec, raw_record, record_length ) )
        {
            records_data->nRecordCount = record_index;
            break;
        }

        for( field_index = 0; field_index < field_count; field_index++ )
        {
            const char  *field_value = RECGetField( raw_record, 
                                                    foffset[field_index]+1, 
                                                    fwidth[field_index] );
            
            if( field_value != NULL && *field_value == '\0' )
                field_value = NULL;
            
            gv_records_set_raw_field_data( records_data, record_index, 
                                           field_index, field_value );
        }


        cancelled = 
            !pfnProgress( (record_index+1) / (double) approx_record_count, 
                          "", pCBData );
    }

    if( !cancelled )
        cancelled = !pfnProgress( 1.0, "", pCBData );

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    g_free( foffset );
    g_free( fwidth );
    g_free( raw_record );

    VSIFClose( fpRec );

    if( !cancelled )
    {
        gv_data_set_name( GV_DATA(records_data), filename );
        return GV_DATA(records_data);
    }
    else
    {
        gtk_object_unref( GTK_OBJECT(records_data) );
        return NULL;
    }
}

/************************************************************************/
/*                        gv_records_finalize()                         */
/************************************************************************/
static void
gv_records_finalize(GObject *gobject)
{
    GvDataClass *parent_class;
    GvRecords *records = GV_RECORDS(gobject);

    CPLDebug( "OpenEV", "gv_records_finalize(%s/%p)", 
              gv_data_get_name( GV_DATA(gobject) ), gobject );

    if( records->pachMainData != NULL) {
        g_free( records->pachMainData );
	records->pachMainData = NULL;
    }

    if( records->papszFieldName != NULL )
    {
        int i;
        for( i = 0; i < records->nFieldCount; i++ )
            g_free( records->papszFieldName[i] );
        g_free( records->papszFieldName );

        g_free( records->panFieldOffset );
        g_free( records->panFieldWidth );
        g_free( records->panFieldType );

	records->papszFieldName = NULL;
    }

    /* Call parent class function */
    parent_class = gtk_type_class(gv_data_get_type());
    G_OBJECT_CLASS(parent_class)->finalize(gobject);         
}

/************************************************************************/
/*                   gv_records_get_raw_record_data()                   */
/************************************************************************/

const char *gv_records_get_raw_record_data( GvRecords *psRecords, 
                                            int iRecord )

{
    if( iRecord < 0 || iRecord >= psRecords->nRecordCount )
        return NULL;
    else
        return psRecords->pachMainData + psRecords->nRecordLength * iRecord;
}

/************************************************************************/
/*                   gv_records_get_raw_field_data()                    */
/************************************************************************/

const char *gv_records_get_raw_field_data( GvRecords *psRecords, 
                                           int iRecord, int iField )

{
    const char *pszResult;

    if( iRecord < 0 || iRecord >= psRecords->nRecordCount 
        || iField < 0 || iField >= psRecords->nFieldCount )
        return NULL;

    pszResult = psRecords->pachMainData + psRecords->nRecordLength * iRecord
        + psRecords->panFieldOffset[iField];
    
    if( *pszResult == GV_NULL_MARKER )
        return NULL;
    else
        return pszResult;
}

/************************************************************************/
/*                   gv_records_set_raw_field_data()                    */
/************************************************************************/

void gv_records_set_raw_field_data( GvRecords *psRecords, 
                                    int iRecord, int iField,
                                    const char *pszNewValue )

{
    char *pszTarget;
    
    if( iRecord < 0 || iRecord >= psRecords->nRecordCount 
        || iField < 0 || iField >= psRecords->nFieldCount )
        return;

    pszTarget = psRecords->pachMainData + psRecords->nRecordLength * iRecord
        + psRecords->panFieldOffset[iField];
    
    if( pszNewValue != NULL )
    {
        strncpy( pszTarget, pszNewValue, psRecords->panFieldWidth[iField] );
        pszTarget[psRecords->panFieldWidth[iField]] = '\0';
    }
    else
    {
        pszTarget[0] = GV_NULL_MARKER;
        pszTarget[1] = '\0';
    }
}

/************************************************************************/
/*                       gv_records_num_records()                       */
/************************************************************************/

int gv_records_num_records( GvRecords *psRecords )

{
    return psRecords->nRecordCount;
}

/************************************************************************/
/*                        gv_records_add_field()                        */
/************************************************************************/

gint gv_records_add_field( GvRecords *psRecords, const char *field_name,
                           int field_type, int width, int precision )

{
    char szPropName[256], szPropValue[256];
    GvProperties *properties;

    int iField = psRecords->nFieldCount;

    (void) precision;

/* -------------------------------------------------------------------- */
/*      Grow the field list array.                                      */
/* -------------------------------------------------------------------- */
    psRecords->nFieldCount++;

    psRecords->papszFieldName = 
        g_renew( char *, psRecords->papszFieldName, psRecords->nFieldCount );
    psRecords->panFieldOffset = 
        g_renew( int, psRecords->panFieldOffset, psRecords->nFieldCount );
    psRecords->panFieldWidth = 
        g_renew( int, psRecords->panFieldWidth, psRecords->nFieldCount );
    psRecords->panFieldType = 
        g_renew( int, psRecords->panFieldType, psRecords->nFieldCount );

/* -------------------------------------------------------------------- */
/*      Add the new values.                                             */
/* -------------------------------------------------------------------- */
    psRecords->papszFieldName[iField] = g_strdup( field_name );
    psRecords->panFieldWidth[iField] = width;
    psRecords->panFieldType[iField] = field_type;
    psRecords->panFieldOffset[iField] = psRecords->nRecordLength;

/* -------------------------------------------------------------------- */
/*      Increase the record size.                                       */
/* -------------------------------------------------------------------- */
    psRecords->nRecordLength += width + 1;

/* -------------------------------------------------------------------- */
/*      Add the new field to the layer properties for easy access       */
/*      from python.                                                    */
/* -------------------------------------------------------------------- */
    properties = gv_data_get_properties( GV_DATA(psRecords) );

    sprintf( szPropName, "_field_name_%d", iField+1 );
    gv_properties_set( properties, szPropName, field_name );

    sprintf( szPropName, "_field_width_%d", iField+1 );
    sprintf( szPropValue, "%d", width );
    gv_properties_set( properties, szPropName, szPropValue );

    sprintf( szPropName, "_field_precision_%d", iField+1 );
    sprintf( szPropValue, "%d", precision );
    gv_properties_set( properties, szPropName, szPropValue );

    if( field_type == GV_RFT_FLOAT )
    {
        sprintf( szPropName, "_field_precision_%d", iField+1 );
        sprintf( szPropValue, "%d", precision );
        gv_properties_set( properties, szPropName, szPropValue );
    }

    sprintf( szPropName, "_field_type_%d", iField+1 );
    if( field_type == GV_RFT_INTEGER )
        gv_properties_set( properties, szPropName, "integer" );
    else if( field_type == GV_RFT_FLOAT )
        gv_properties_set( properties, szPropName, "float" );
    else
        gv_properties_set( properties, szPropName, "string" );

/* -------------------------------------------------------------------- */
/*      If data already exists we need to do a big reshuffle.           */
/* -------------------------------------------------------------------- */
    if( psRecords->nRecordCount != 0 )
    {
        int iRec;
        int old_rec_length = psRecords->nRecordLength - width - 1;
        
        if( psRecords->nMainDataSize < 
            psRecords->nRecordCount * psRecords->nRecordLength )
        {
            psRecords->nMainDataSize = 
                psRecords->nRecordCount * psRecords->nRecordLength;
            psRecords->pachMainData = 
                g_renew( char, psRecords->pachMainData, 
                         psRecords->nMainDataSize );
        }

        for( iRec = psRecords->nRecordCount-1; iRec >= 0; iRec-- )
        {
            char *pszNew = psRecords->pachMainData 
                + iRec * psRecords->nRecordLength + old_rec_length;

            memmove( psRecords->pachMainData + iRec * psRecords->nRecordLength,
                     psRecords->pachMainData + iRec * old_rec_length, 
                     old_rec_length );
            memset( pszNew, 0, psRecords->nRecordLength - old_rec_length );
            pszNew[0] = GV_NULL_MARKER;
        }
    }

    return iField;
}

/************************************************************************/
/*                   gv_records_set_used_properties()                   */
/************************************************************************/

void gv_records_set_used_properties(GvRecords *psRecords, int nFieldCount,
                                    int *panFieldList )

{
    if( psRecords->nUsedFieldCount != 0 )
        g_free( psRecords->panUsedFieldList );

    psRecords->panUsedFieldList = g_new( int, nFieldCount );
    memcpy( psRecords->panUsedFieldList, panFieldList, 
            sizeof(int) * nFieldCount );
    psRecords->nUsedFieldCount = nFieldCount;
}

