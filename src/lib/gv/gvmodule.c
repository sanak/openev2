/* -*- Mode: C; c-basic-offset: 4 -*- */
/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Gv python module
 * Author:   OpenEV Team
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

/* include this first, before NO_IMPORT_PYGOBJECT is defined */
#include <pygobject.h>
#ifdef CIET_BUILD
#include "gvrecords.h"
PyTypeObject G_GNUC_INTERNAL PyGvRecords_Type;
PyTypeObject G_GNUC_INTERNAL PyGvData_Type;
#endif

#if defined(WIN32) || defined(_WIN32)
#  include <pygtk.h>
#else
#  include <pygtk/pygtk.h>
#endif

PyTypeObject G_GNUC_INTERNAL PyGvShape_Type;

void gv_register_classes (PyObject *d);

extern PyMethodDef _gv_functions[];

void
init_pygv_shape_type(PyObject *m)
{
    PyGvShape_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyGvShape_Type) < 0)
        return;
    PyModule_AddObject(m, "Shape", (PyObject *)&PyGvShape_Type);
}

DL_EXPORT(void)
init_gv(void)
{
    PyObject *m, *d;
    char msg[512];

    init_pygobject();
    init_pygtk();

    m = Py_InitModule ("_gv", _gv_functions);
    d = PyModule_GetDict (m);
    if (PyErr_Occurred ())
        Py_FatalError ("2nd can't initialise module _gv :(");

    _gv_register_classes (d);
    init_pygv_shape_type(m);

#ifdef CIET_BUILD
    pygobject_register_class(d, "GvRecords", GV_TYPE_RECORDS,
                             &PyGvRecords_Type, Py_BuildValue("(O)", &PyGvData_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_RECORDS);
#endif

    if (PyErr_Occurred ()) {
        PyErr_Print();
        sprintf(msg, "Cannot initialize module _gv:("); 
        Py_FatalError (msg);
    }
}
