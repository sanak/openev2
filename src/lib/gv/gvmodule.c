/* -*- Mode: C; c-basic-offset: 4 -*- */

/* include this first, before NO_IMPORT_PYGOBJECT is defined */
#include <pygobject.h>

#if defined(WIN32) || defined(_WIN32)
#  include <pygtk.h>
#else
#  include <pygtk/pygtk.h>
#endif

void gv_register_classes (PyObject *d);

extern PyMethodDef _gv_functions[];

DL_EXPORT(void)
init_gv(void)
{
    PyObject *m, *d;
    char msg[512];
	
    init_pygobject();
    init_pygtk();

    m = Py_InitModule ("_gv", _gv_functions);

    d = PyModule_GetDict (m);
	    if (PyErr_Occurred ()) {
	Py_FatalError ("2nd can't initialise module _gv :(");
    }

    _gv_register_classes (d);

    if (PyErr_Occurred ()) {
        PyErr_Print();
        sprintf(msg, "Cannot initialize module _gv:("); 
	Py_FatalError (msg);
    }
}
