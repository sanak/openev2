/******************************************************************************
 * $Id: gv_ciet.h,v 1.12 2005/07/27 18:31:50 warmerda Exp $
 *
 * Project:  CIETmap
 * Purpose:  gv_ciet declarations (to include in gv.override)
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
 */

#ifndef __GV_CIET_H__
#define __GV_CIET_H__

#include <Python.h>               

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

typedef struct {
    double dfMin;
    double dfMax;
    double dfReplacement;
    int    bReplaceNULL;
} GvRecodeEntry;

StratNode *StratNodeAdd(StratNode *psNode, StratVarInfo *psVar, double dfValue);
PyObject *StratTreeToDict(StratNode *psNode, StratVarInfo *pasVarInfos);
void SortRecodings(int nCount, GvRecodeEntry *pasRecodings);
GvRecodeEntry *FindRecoding(double dfValue, int nCount, GvRecodeEntry *pasRecodings);

#endif /* ndef __GV_CIET_H__ */
