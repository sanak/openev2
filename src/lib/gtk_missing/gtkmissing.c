/******************************************************************************
 * $Id: gtkmissing.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Hand generated python bindings for some Gtk functionality
 *           not addressed by pygtk.
 * Author:   OpenEV Team
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
 * $Log: gtkmissing.c,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:33  uid1026
 *
 * Imported sources
 *
 * Revision 1.8  2004/09/30 21:12:07  warmerda
 * added explicit ref() and unref() though we shouldn't generally need them
 *
 * Revision 1.7  2002/07/09 03:34:10  warmerda
 * added window get/set position calls
 *
 * Revision 1.6  2001/07/03 23:25:51  warmerda
 * added gtk_object_sink
 *
 * Revision 1.5  2000/06/23 13:00:41  warmerda
 * added a few ref count related debugging functions
 *
 * Revision 1.4  2000/06/20 13:39:06  warmerda
 * added standard headers
 *
 */

/* GTK2 Port */
#include <pygobject.h>

#if defined(WIN32) || defined(_WIN32)
#  include <pygtk.h>
#else
#  include <pygtk/pygtk.h>
#endif

#define PyGtk_Type PyGPointer_Type
#define PyGtk_New pygobject_new
#define PyGtk_Get pygobject_get
#define PyGtk_Object PyGObject

/*
 * Functions missing in the python gtk bindings (as of pygtk-0.6.3)
 */

static PyObject *
_wrap_gtk_window_move(PyObject *self, PyObject *args)
{
    PyGtk_Object *t;
    int x, y;
    GdkWindow *window;
    
    if (!PyArg_ParseTuple(args, "O!ii:gtk_window_move",
			  &PyGtk_Type, &t, &x, &y))
	return NULL;

    window = GTK_WIDGET(PyGtk_Get(t))->window;

    gdk_window_move( window, x, y );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef gtkmissing_methods[] =
{
    {"gtk_window_move", _wrap_gtk_window_move, 1},
    {NULL, NULL, 0}
};

void
init_gtkmissing(void)
{
    init_pygtk();
    
    Py_InitModule("_gtkmissing", gtkmissing_methods);
    
    if (PyErr_Occurred())
	Py_FatalError("can't initialize module _gtkmissing");
}
