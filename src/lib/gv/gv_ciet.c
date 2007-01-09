/******************************************************************************
 * $Id: gv_ciet.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  CIETmap
 * Purpose:  High performance function for collecting stratified
 *           sample data to accelerate the CIETmap TABLES command.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2003, CIETcanada
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
 * $Log: gv_ciet.c,v $
 * Revision 1.1.1.1  2005/04/18 16:38:34  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:35  uid1026
 *
 * Imported sources
 *
 * Revision 1.11  2004/11/19 15:38:18  zjamesatdm
 * added support for optional else clause to recode.
 *
 * Revision 1.10  2004/09/28 19:43:26  warmerda
 * Avoid extra reference increment on PyDict objects in StratTreeToDict().
 * Was causing a substantial leak as noted in:
 *   https://www.lan.dmsolutions.ca/bugzilla/show_bug.cgi?id=3033
 *
 * Revision 1.9  2004/08/23 21:29:36  pgs
 * fixed memory allocation problem in gv_records_asdict
 *
 * Revision 1.8  2004/08/20 13:32:07  warmerda
 * Removed some unused variables.
 *
 * Revision 1.7  2004/08/18 20:55:21  pgs
 * added ability to return columns of data in a python dictionary
 *
 * Revision 1.6  2004/01/13 19:26:46  pgs
 * fixed SortRecodings algorithim
 *
 * Revision 1.5  2003/08/23 04:02:39  warmerda
 * added gv_records_recode
 *
 * Revision 1.4  2003/08/14 18:31:47  warmerda
 * handle case of no data collected, support just 1 or 2 vars
 *
 * Revision 1.3  2003/08/11 20:06:33  warmerda
 * added progress reporting
 *
 * Revision 1.2  2003/08/11 18:36:49  warmerda
 * added integer type support
 *
 * Revision 1.1  2003/08/08 18:10:16  warmerda
 * New
 *
 */

typedef struct _StratNode {
    int nNodeCount;
    struct _StratNode *pasItemSubNode;
    int  *panItemCount;
} StratNode;

typedef struct {
    int nFieldIndex;
    int nValueCount;
    double *padfValueList;
    PyObject **papyValueList;
    int  *panValueIndex;
    int  *panValueCount;

    int  bLastVar;
    int  bIsInteger;
} StratVarInfo;

/************************************************************************/
/*                         StratVarFindValue()                          */
/*                                                                      */
/*      Try to find the passed in value in the list of values           */
/*      currently associated with the variable.  We keep the values     */
/*      in sorted order so we can do a binary search.  If we find       */
/*      it, just return the index (unsorted that is).                   */
/************************************************************************/

static int StratVarFindValue( StratVarInfo *psVar, double dfValue )

{
    int  iStart, iEnd, iMiddle;

/* -------------------------------------------------------------------- */
/*      Special case for the very common case of one or two values      */
/*      in the list.                                                    */
/* -------------------------------------------------------------------- */
    if( psVar->nValueCount > 0 && dfValue == psVar->padfValueList[0] )
        return psVar->panValueIndex[0];

    else if( psVar->nValueCount > 1 && dfValue == psVar->padfValueList[1] )
        return psVar->panValueIndex[1];

/* -------------------------------------------------------------------- */
/*      Do a binary search on the list.                                 */
/* -------------------------------------------------------------------- */
    iStart = 0;
    iEnd = psVar->nValueCount - 1;
    while( iEnd >= iStart )
    {
        iMiddle = (iEnd + iStart) / 2;
        if( dfValue < psVar->padfValueList[iMiddle] )
            iEnd = iMiddle - 1;
        else if( dfValue > psVar->padfValueList[iMiddle] )
            iStart = iMiddle + 1;
        else
            return psVar->panValueIndex[iMiddle];
    }

/* -------------------------------------------------------------------- */
/*      We didn't find the value.  Add it to the end of the list.       */
/* -------------------------------------------------------------------- */
    psVar->nValueCount++;

    psVar->padfValueList = (double *) 
        VSIRealloc(psVar->padfValueList, sizeof(double) * psVar->nValueCount );
    psVar->padfValueList[psVar->nValueCount-1] = dfValue;
    
    psVar->panValueIndex = (int *) 
        VSIRealloc(psVar->panValueIndex, sizeof(int) * psVar->nValueCount );
    psVar->panValueIndex[psVar->nValueCount-1] = psVar->nValueCount-1;

    psVar->panValueCount = (int *) 
        VSIRealloc(psVar->panValueCount, sizeof(int) * psVar->nValueCount );
    psVar->panValueCount[psVar->nValueCount-1] = 0;

/* -------------------------------------------------------------------- */
/*      Now sort it into the correct location in order.                 */
/* -------------------------------------------------------------------- */
    for( iStart = psVar->nValueCount - 2; iStart >= 0; iStart-- )
    {
        if( psVar->padfValueList[iStart] > psVar->padfValueList[iStart+1] )
        {
            /* swap values */

            dfValue = psVar->padfValueList[iStart];
            psVar->padfValueList[iStart] = psVar->padfValueList[iStart+1];
            psVar->padfValueList[iStart+1] = dfValue;

            iMiddle = psVar->panValueIndex[iStart];
            psVar->panValueIndex[iStart] = psVar->panValueIndex[iStart+1];
            psVar->panValueIndex[iStart+1] = iMiddle;

            /* note: the counts are in unsorted order */
        }
        else
            break;
    }

    return psVar->nValueCount-1;
}

/************************************************************************/
/*                            StratNodeAdd()                            */
/*                                                                      */
/*      Add the passed value to the counts in this node, and return     */
/*      the subnode associated with the value.  Extent the item list    */
/*      if needed.                                                      */
/************************************************************************/

static StratNode *StratNodeAdd( StratNode *psNode, StratVarInfo *psVar, 
                                double dfValue )

{
    int nItem = StratVarFindValue( psVar, dfValue );

    psVar->panValueCount[nItem]++;

/* -------------------------------------------------------------------- */
/*      Reallocate the list of items larger if we find that the new     */
/*      item is higher than the available item list.                    */
/* -------------------------------------------------------------------- */
    if( nItem >= psNode->nNodeCount )
    {
        if( psVar->bLastVar )
        {
            psNode->panItemCount = (int *) 
                VSIRealloc(psNode->panItemCount,
                           sizeof(int) * psVar->nValueCount);
            memset( psNode->panItemCount + psNode->nNodeCount, 
                    0, sizeof(int)*(psVar->nValueCount-psNode->nNodeCount) );
        }
        else
        {
            psNode->pasItemSubNode = (StratNode *) 
                VSIRealloc(psNode->pasItemSubNode,
                           sizeof(StratNode) * psVar->nValueCount);
            memset( psNode->pasItemSubNode + psNode->nNodeCount, 
                    0, 
                    sizeof(StratNode)*(psVar->nValueCount-psNode->nNodeCount));
        }
        psNode->nNodeCount = psVar->nValueCount;
    }

/* -------------------------------------------------------------------- */
/*      Increment count (for last variable), or return appropriate      */
/*      subnode.                                                        */
/* -------------------------------------------------------------------- */
    if( psVar->bLastVar )
    {
        psNode->panItemCount[nItem]++;
        return NULL;
    }
    else
        return psNode->pasItemSubNode + nItem;
}

/************************************************************************/
/*                          StratTreeToDict()                           */
/************************************************************************/

PyObject *StratTreeToDict( StratNode *psNode, StratVarInfo *pasVarInfos )

{
    PyObject *psDict = NULL;
    int      i, bHaveItems = FALSE;

/* -------------------------------------------------------------------- */
/*      Do we have any children?  If not, we don't need to do           */
/*      anything.                                                       */
/* -------------------------------------------------------------------- */
    for( i = 0; i < psNode->nNodeCount && !bHaveItems; i++ )
    {
        if( pasVarInfos[0].bLastVar  )
            bHaveItems |= (psNode->panItemCount[i] != 0);
        else 
            bHaveItems |= (psNode->pasItemSubNode[i].nNodeCount != 0);
    }

/* -------------------------------------------------------------------- */
/*      Create a dictionary into which to stick subdictionaries, or     */
/*      counts.                                                         */
/* -------------------------------------------------------------------- */
    if( bHaveItems )
    {
        psDict = PyDict_New();
    
        for( i = 0; i < psNode->nNodeCount; i++ )
        {
            PyObject *pyValue = NULL; 

            if( pasVarInfos[0].bLastVar  )
            {
                if( psNode->panItemCount[i] != 0 )
                    pyValue = Py_BuildValue( "i", psNode->panItemCount[i] );
            }
            else 
            {
                if( psNode->pasItemSubNode[i].nNodeCount != 0 )
                {
                    pyValue = StratTreeToDict( psNode->pasItemSubNode + i, 
                                               pasVarInfos + 1 );
                }
            }

            if( pyValue )
            {
                PyDict_SetItem( psDict, pasVarInfos[0].papyValueList[i], 
                                pyValue );
                Py_DECREF( pyValue );
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Cleanup resources associated witht his node.                    */
/* -------------------------------------------------------------------- */
    CPLFree( psNode->panItemCount );
    CPLFree( psNode->pasItemSubNode );

    return psDict;
}

/************************************************************************/
/*                  GvRecords.MultiStratifiedCollect()                  */
/************************************************************************/

static PyObject *
_wrap_gv_records_MultiStratifiedCollect(PyObject *self, PyObject *args)
{
    PyObject *py_records, *pyResultTree;
    PyObject *py_data_selection, *py_strat_vars;
    PyObject *v1_keys=Py_None, *v2_keys=Py_None, *result;
    char     *var1 = NULL, *var2 = NULL;
    GvRecords *records;
    int       nSelectionCount, *panSelectionList = NULL, i;
    int       nVarCount, iVar, bCancelled = FALSE;
    StratVarInfo *pasVarInfo;
    StratNode *psRoot = NULL;
    double    *padfRecValues;
    PyProgressData sProgressInfo;

    sProgressInfo.psPyCallback = NULL;
    sProgressInfo.psPyCallbackData = NULL;

    if (!PyArg_ParseTuple(args, "OO!ssO!OO:gv_records_MultiStratifiedCollect",
                          &py_records,
                          &PyList_Type, &py_data_selection,
                          &var1, &var2, 
                          &PyList_Type, &py_strat_vars,
                          &(sProgressInfo.psPyCallback),
                          &(sProgressInfo.psPyCallbackData) ) )
        return NULL;

    if( !GV_IS_RECORDS(PyGtk_Get(py_records)) )
        return NULL;

    records = GV_RECORDS(PyGtk_Get(py_records));

    /* 
    ** Initialize progress.
    */
    if( !PyProgressProxy( 0.0, "", &sProgressInfo ) )
        return FALSE;

    /*
    ** Build the variable list.  Var2 is manditory, var1 is optional. 
    */
    nVarCount = PyList_Size( py_strat_vars ) + 1;
    if( var1 != NULL && strlen(var1) > 0 )
        nVarCount++;

    pasVarInfo = (StratVarInfo *) VSICalloc(sizeof(StratVarInfo),nVarCount);
    padfRecValues = (double *) VSICalloc(sizeof(double),nVarCount);

    for( iVar = 0; iVar < nVarCount; iVar++ )
    {
        const char *pszVarName = NULL;
        int iField;

        if( iVar < nVarCount-2 )
        {
            if( !PyArg_Parse( PyList_GET_ITEM(py_strat_vars,iVar), "s",
                              &pszVarName ) )
            {
                PyErr_SetString(PyExc_ValueError,
                                "expecting variable name in strat_vars.");
                return NULL;
            }
        }
        else if( iVar == nVarCount - 2 )
        {
            pszVarName = var1;
        }
        else if( iVar == nVarCount - 1 )
        {
            pszVarName = var2;
            pasVarInfo[iVar].bLastVar = TRUE;
        }

        for( iField = 0; iField < records->nFieldCount; iField++ )
        {
            if( EQUAL(pszVarName,records->papszFieldName[iField]) )
            {
                pasVarInfo[iVar].nFieldIndex = iField;
                pasVarInfo[iVar].bIsInteger = 
                    records->panFieldType[iField] == GV_RFT_INTEGER;
                break;
            }
        }

        if( iField == records->nFieldCount )
        {
            PyErr_SetString(PyExc_ValueError,
                            "unrecognised field name.");
            return NULL;
        }
    }

    /*
    ** Build up the "selection" list.
    */
    nSelectionCount = PyList_Size( py_data_selection );
    panSelectionList = (int *) CPLMalloc(sizeof(int) * nSelectionCount );

    for( i = 0; i < nSelectionCount; i++ )
    {
        if( !PyArg_Parse( PyList_GET_ITEM(py_data_selection,i), "i",
                          panSelectionList + i ) )
        {
            PyErr_SetString(PyExc_ValueError,
                            "problem extracting selection item.");
            return NULL;
        }
    }

    /*
    ** Allocate a root node for the data collection "tree".
    */

    psRoot = (StratNode *) CPLCalloc(sizeof(StratNode),1);

    /*
    ** Process all records.
    */
    
    for( i = 0; i < nSelectionCount && !bCancelled; i++ )
    {
        int rec_index = panSelectionList[i];
        StratNode *psNode = psRoot;

        for( iVar = 0; iVar < nVarCount; iVar++ )
        {
            const char *pszRawFieldValue 
                = gv_records_get_raw_field_data( records, rec_index, 
                                                 pasVarInfo[iVar].nFieldIndex);
            if( pszRawFieldValue == NULL )
                break;
            
            if( pasVarInfo[iVar].bIsInteger )
                padfRecValues[iVar] = atoi(pszRawFieldValue);
            else
                padfRecValues[iVar] = atof(pszRawFieldValue);
        }

        if( i % 100 == 0 )
        {
            if( !PyProgressProxy( i / (double) nSelectionCount, 
                                  "", &sProgressInfo ) )
                bCancelled = TRUE;
        }

        /* skip records with nulls in a used variable. */
        if( iVar < nVarCount )
            continue;

        for( iVar = 0; iVar < nVarCount; iVar++ )
        {
            psNode = StratNodeAdd( psNode, pasVarInfo + iVar, 
                                   padfRecValues[iVar] );
        }
    }

    CPLFree( padfRecValues );
    CPLFree( panSelectionList );

    if( !bCancelled )
        PyProgressProxy( 1.0, "", &sProgressInfo );

    /*
    ** Unsort the values list in the StratVarInfo's so that our indexes can
    ** be used easily to get back to the variable values. 
    */
    for( iVar = 0; iVar < nVarCount; iVar++ )
    {
        StratVarInfo  *psVar = pasVarInfo + iVar;
        int           iValue;
        double        *padfNewValueList;

        padfNewValueList = (double *) 
            CPLMalloc(sizeof(double) * psVar->nValueCount);
        
        for( iValue = 0; iValue < psVar->nValueCount; iValue++ )
        {
            padfNewValueList[psVar->panValueIndex[iValue]] = 
                psVar->padfValueList[iValue];
        }
        CPLFree( psVar->padfValueList );
        psVar->padfValueList = padfNewValueList;

        CPLFree( psVar->panValueIndex );
        psVar->panValueIndex = NULL;
    }

    /*
    ** Create Python string reresentations of the values for use in the
    ** final results dictionary.
    */
    for( iVar = 0; iVar < nVarCount; iVar++ )
    {
        StratVarInfo *psVar = pasVarInfo + iVar;
        int           iValue;

        psVar->papyValueList = (PyObject **) 
            CPLMalloc(sizeof(PyObject *) * psVar->nValueCount);

        for( iValue = 0; iValue < psVar->nValueCount; iValue++ )
        {
            if( psVar->bIsInteger )
            {
                psVar->papyValueList[iValue] = 
                    Py_BuildValue( "i", (int) psVar->padfValueList[iValue] );
            }
            else
            {
                psVar->papyValueList[iValue] = 
                    Py_BuildValue( "d", psVar->padfValueList[iValue] );
            }
        }
    }

    /*
    ** Marshall the results into a tree of dictionaries.  Wipe tree as
    ** part of the process.
    */
    
    pyResultTree = StratTreeToDict( psRoot, pasVarInfo );
    if( pyResultTree == NULL )
    {
        pyResultTree = Py_None;
        Py_INCREF( Py_None );
    }

    /*
    ** Create v1/v2_keys ... a dictionary with one entry for each value of 
    ** var1/var2 that occurs in the data.
    */

    for( iVar = nVarCount-1; iVar >= nVarCount-2 && iVar >= 0; iVar-- )
    {
        int iValue;
        PyObject *key_dict = PyDict_New();
        
        for( iValue = 0; iValue < pasVarInfo[iVar].nValueCount; iValue++ )
        {
            PyDict_SetItem( key_dict, pasVarInfo[iVar].papyValueList[iValue], 
                            Py_None );
        }

        if( iVar == nVarCount-1 )
            v2_keys = key_dict;
        else
            v1_keys = key_dict;
    }
    
    /*
    ** Cleanup working structures. 
    */

    /*
    ** Prepare final result tuple. 
    */

    result = Py_BuildValue( "(OOO)", v1_keys, v2_keys, pyResultTree );

    if( v1_keys != Py_None )
    {
        Py_DECREF( v1_keys );
    }
    Py_DECREF( v2_keys );
    Py_DECREF( pyResultTree );

    if( bCancelled )
    {
        PyErr_SetString(PyExc_ValueError,
                        "User called processing.");
        Py_DECREF( result );
        return NULL;
    }
    else
    {
        return result;
    }
}

typedef struct {
    double dfMin;
    double dfMax;
    double dfReplacement;
    int    bReplaceNULL;
} GvRecodeEntry;

/************************************************************************/
/*                           SortRecodings()                            */
/*                                                                      */
/*      Simple bubble sort of the recode table entries.                 */
/************************************************************************/

void SortRecodings( int nCount, GvRecodeEntry *pasRecodings )

{
    int  i, j;

    for( i = 0; i < nCount-1; i++ )
    {
        for( j = 0; j < nCount-1; j++ )
        {
            if( pasRecodings[j].dfMin > pasRecodings[j+1].dfMin )
            {
                GvRecodeEntry sTempEntry;

                memcpy( &sTempEntry, pasRecodings+j, sizeof(sTempEntry) );
                memcpy( pasRecodings+j, pasRecodings+j+1, sizeof(sTempEntry) );
                memcpy( pasRecodings+j+1, &sTempEntry, sizeof(sTempEntry) );
            }
        }
    }
}

/************************************************************************/
/*                            FindRecoding()                            */
/************************************************************************/

GvRecodeEntry *FindRecoding( double dfValue, 
                             int nCount, GvRecodeEntry *pasRecodings )

{
    int  iStart, iEnd, iMiddle;

    iStart = 0;
    iEnd = nCount - 1;
    
    //TODO: dump out here???

    while( iEnd >= iStart )
    {
        iMiddle = (iStart+iEnd)/2;

        if( pasRecodings[iMiddle].dfMin > dfValue )
            iEnd = iMiddle - 1;
        else if( pasRecodings[iMiddle].dfMax < dfValue )
            iStart = iMiddle+1;
        else if( pasRecodings[iMiddle].dfMin <= dfValue
                 && pasRecodings[iMiddle].dfMax >= dfValue )
            return pasRecodings + iMiddle;
        else
            return NULL;
    }

    return NULL;
}

/************************************************************************/
/*                         gv_records_recode()                          */
/************************************************************************/

static PyObject *
_wrap_gv_records_recode(PyObject *self, PyObject *args)
{
    PyObject *py_records, *py_single_mapping, *py_range_mapping;
    GvRecords *records;
    char      *pszSrcVarName = NULL, *pszDstVarName = NULL;
    int       bCancelled = FALSE, i, nRecordCount;
    int       nRangeRecodeCount = 0, nSingleRecodeCount = 0;
    int       iSrcField, iDstField, nIgnored=0, nAltered=0, nUnchanged=0;
    PyProgressData sProgressInfo;
    GvRecodeEntry *pasSingleRecodings;
    GvRecodeEntry *pasRangeRecodings;
    PyObject    *pyKey = NULL, *pyValue = NULL;
    int       bHaveNULLReplacement = FALSE;
    GvRecodeEntry sNULLEntry;
    int       bElseClause = FALSE;
    GvRecodeEntry sElseEntry;
    
    sProgressInfo.psPyCallback = NULL;
    sProgressInfo.psPyCallbackData = NULL;

    if (!PyArg_ParseTuple(args, "OO!O!ssOO:gv_records_recode",
                          &py_records,
                          &PyDict_Type, &py_single_mapping,
                          &PyList_Type, &py_range_mapping,
                          &pszSrcVarName, &pszDstVarName, 
                          &(sProgressInfo.psPyCallback),
                          &(sProgressInfo.psPyCallbackData) ) )
        return NULL;

    if( !GV_IS_RECORDS(PyGtk_Get(py_records)) )
        return NULL;

    records = GV_RECORDS(PyGtk_Get(py_records));

    /*
    ** Identify the source and destination field indexes.
    */
    iSrcField = -1;
    iDstField = -1;
    for( i = 0; i < records->nFieldCount; i++ )
    {
        if( EQUAL(pszSrcVarName,records->papszFieldName[i]) )
            iSrcField = i;
        if( EQUAL(pszDstVarName,records->papszFieldName[i]) )
            iDstField = i;
    }

    if( iSrcField == -1 || iDstField == -1 )
    {
        PyErr_SetString( PyExc_ValueError, "RECODE variable not recognised." );
        return NULL;
    }

    cos( 0.0 );

    /*
    ** Collect the single value mappings.
    */
    pasSingleRecodings = 
        g_new( GvRecodeEntry, PyDict_Size(py_single_mapping) );

    nSingleRecodeCount = 0;
    i = 0;
    while( PyDict_Next( py_single_mapping, &i, &pyKey, &pyValue ) )
    {
        char *pszKey = NULL,  *pszValue = NULL;

        if( pyValue == Py_None )
        {
            pasSingleRecodings[nSingleRecodeCount].dfReplacement = 0.0;
            pasSingleRecodings[nSingleRecodeCount].bReplaceNULL = TRUE;
        }
        else if( !PyArg_Parse( pyValue, "s", &pszValue ) )
        {
            PyErr_SetString(PyExc_TypeError, 
                            "Single value is not numeric." );
            return NULL;
        }
        else
        {
            pasSingleRecodings[nSingleRecodeCount].dfReplacement =
                atof( pszValue );
            pasSingleRecodings[nSingleRecodeCount].bReplaceNULL = FALSE;
        }

        if( pyKey == Py_None )
        {
            bHaveNULLReplacement = TRUE;
            memcpy( &sNULLEntry, 
                    pasSingleRecodings + nSingleRecodeCount, 
                    sizeof(sNULLEntry) );
        }
        else if( !PyArg_Parse( pyKey, "s", &pszKey ) )
        {
            PyErr_SetString(PyExc_TypeError, 
                            "Single value is not numeric." );
            return NULL;
        }
        else if( EQUAL(pszKey,"None") || EQUAL(pszKey,"NULL") )
        {
            bHaveNULLReplacement = TRUE;
            memcpy( &sNULLEntry, 
                    pasSingleRecodings + nSingleRecodeCount, 
                    sizeof(sNULLEntry) );
        }
        else if( EQUAL(pszKey,"ELSE")  )
        {
            /*NULL is a special case*/
            if( pyValue != Py_None )
            {
                sElseEntry.dfReplacement = atof( pszValue );
            }
            else
            {
                bHaveNULLReplacement = TRUE;
            }
            
            /*any value should be caught by else*/
            sElseEntry.dfMin = 0 - HUGE_VAL;
            sElseEntry.dfMax = HUGE_VAL;
            memcpy( &sElseEntry, 
                    pasSingleRecodings + nSingleRecodeCount, 
                    sizeof(sElseEntry) );
            bElseClause = TRUE;
        }
        else
        {
            pasSingleRecodings[nSingleRecodeCount].dfMin = atof(pszKey);
            pasSingleRecodings[nSingleRecodeCount].dfMax = atof(pszKey);
            nSingleRecodeCount++;
        }
    }

    SortRecodings( nSingleRecodeCount, pasSingleRecodings );
    
    /*
    ** Collect range mappings. 
    */

    nRangeRecodeCount = PyList_Size(py_range_mapping);
    pasRangeRecodings = g_new( GvRecodeEntry, nRangeRecodeCount );

    for( i = 0; i < nRangeRecodeCount; i++ )
    {
        char *pszReplacement=NULL;

        if( !PyArg_Parse( PyList_GetItem( py_range_mapping, i ), "(ddz)", 
                          &(pasRangeRecodings[i].dfMin),
                          &(pasRangeRecodings[i].dfMax),
                          &pszReplacement ) )
        {
            PyErr_SetString(PyExc_TypeError, 
                            "Range value is not (min,max,result) tuple." );
            return NULL;
        }

        if( pszReplacement == NULL )
        {
            pasRangeRecodings[i].dfReplacement = 0.0;
            pasRangeRecodings[i].bReplaceNULL = TRUE;
        }
        else
        {
            pasRangeRecodings[i].dfReplacement = atof(pszReplacement);
            pasRangeRecodings[i].bReplaceNULL = FALSE;
        }
    }

    SortRecodings( nRangeRecodeCount, pasRangeRecodings );

    /* 
    ** Initialize progress.
    */
    if( !PyProgressProxy( 0.0, "", &sProgressInfo ) )
        return FALSE;

    /*
    ** Process all records (not just selected ones!). 
    */
    nRecordCount = gv_records_num_records( records );
    for( i = 0; i < nRecordCount && !bCancelled; i++ )
    {
        double dfValue;
        GvRecodeEntry *psEntry;
        const char *pszRawFieldValue = 
            gv_records_get_raw_field_data( records, i, iSrcField );

        if( pszRawFieldValue == NULL )
        {
            if( bHaveNULLReplacement )
                psEntry = &sNULLEntry;
            else
                psEntry = NULL;
        }
        else
        {
            dfValue = atof(pszRawFieldValue);
        
            psEntry = FindRecoding( dfValue, 
                                    nSingleRecodeCount, pasSingleRecodings );
            if( psEntry == NULL)
                psEntry = FindRecoding( dfValue, 
                                        nRangeRecodeCount, pasRangeRecodings );
            
            /*
             if nothing else was applied, use the ELSE value after adding
             the correct mapping
            */
            if (psEntry == NULL && bElseClause)
            {
                psEntry = &sElseEntry;
            }
        }

        
        if( psEntry == NULL )
            nUnchanged++;
        else if( psEntry->bReplaceNULL )
        {
            gv_records_set_raw_field_data( records, i, iDstField, NULL );
            nIgnored++;
        }
        else
        {
            char szReplaceText[200];

            sprintf( szReplaceText, "%g", psEntry->dfReplacement );
            gv_records_set_raw_field_data( records, i, iDstField, 
                                           szReplaceText);
            nAltered++;
        }
        
        if( i % 100 == 0 )
        {
            if( !PyProgressProxy( i / (double) nRecordCount,
                                  "", &sProgressInfo ) )
                bCancelled = TRUE;
        }
    }

    if( !bCancelled )
        PyProgressProxy( 1.0, "", &sProgressInfo );

    g_free( pasSingleRecodings );
    g_free( pasRangeRecodings );

    return Py_BuildValue( "(iii)", nUnchanged, nAltered, nIgnored );
}



/************************************************************************/
/*                         gv_records_aslist()                          */
/************************************************************************/

static PyObject *
_wrap_gv_records_asdict(PyObject *self, PyObject *args)
{
    PyObject *py_records, *py_fields;
    GvRecords *records;
    int       i, j, nRecordCount, nFields, *panFields;
    PyObject *py_dict = NULL, *py_list = NULL;
    char ** paszFieldNames;
    
    if (!PyArg_ParseTuple(args, "OO!:gv_records_asdict",
                          &py_records,
                          &PyList_Type, &py_fields ) )
        return NULL;

    if( !GV_IS_RECORDS(PyGtk_Get(py_records)) )
        return NULL;

    records = GV_RECORDS(PyGtk_Get(py_records));


    nFields = PyList_Size(py_fields);
    paszFieldNames = (char **) malloc( nFields * sizeof( char * ) );
    panFields = (int *)malloc( nFields * sizeof( int ) );
    for( i = 0; i < nFields; i++ )
    {
        paszFieldNames[i] = (char*)malloc(20 * sizeof( char ));
        PyArg_Parse( PyList_GET_ITEM(py_fields,i), "s", &paszFieldNames[i] );
        panFields[i] = -1;
        for( j = 0; j < records->nFieldCount; j++ )
        {
            if( EQUAL(paszFieldNames[i],records->papszFieldName[j]) )
                panFields[i] = j;
        }
        if( panFields[i] == -1)
        {
            PyErr_SetString( PyExc_ValueError, "ASDICT variable not recognised." );
            return NULL;
        }
    }
    
    nRecordCount = gv_records_num_records( records );
    py_dict = PyDict_New();
    for( i = 0; i < nFields; i++ )
    {
        PyDict_SetItem( py_dict, PyString_FromString(paszFieldNames[i]), PyList_New( nRecordCount ) );
    }
    
    /*
     ** Process all records (not just selected ones!). 
     */
    
    for( i = 0; i < nRecordCount; i++ )
    {
        for( j = 0; j < nFields; j++ )
        {
            const char *pszRawFieldValue = 
                gv_records_get_raw_field_data( records, i, panFields[j] );

            py_list = PyDict_GetItem( py_dict, PyString_FromString(paszFieldNames[j]) );
            
            if( pszRawFieldValue == NULL )
            {
                PyList_SetItem( py_list, i, Py_None );
            }
            else
            {
                switch( records->panFieldType[panFields[j]] )
                {
                    case GV_RFT_INTEGER:
                        PyList_SetItem( py_list, i, Py_BuildValue( "i", atoi(pszRawFieldValue) ));
                        break;
                    case GV_RFT_FLOAT:
                        PyList_SetItem( py_list, i, Py_BuildValue( "d", atof(pszRawFieldValue) ));
                        break;
                    default:
                        PyList_SetItem( py_list, i, Py_BuildValue( "s", pszRawFieldValue));
                }
            }
            //PyDict_SetItem( py_dict, PyString_FromString(paszFieldNames[j]), py_list );
        }

    }
    
    free( panFields );
    for(i=0;i<nFields;i++)
        free( paszFieldNames[i] );
    free( paszFieldNames );
    
    return py_dict;
}
