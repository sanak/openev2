/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 4 "gv.override"
/* Py_ssize_t availability. */
#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
typedef inquiry lenfunc;
typedef intargfunc ssizeargfunc;
typedef intobjargproc ssizeobjargproc;
typedef intintargfunc ssizessizeargfunc;
typedef intintobjargproc ssizessizeobjargproc;
typedef getreadbufferproc readbufferproc;
typedef getwritebufferproc writebufferproc;
typedef getsegcountproc segcountproc;
typedef getcharbufferproc charbufferproc;
#endif
#include "pygobject.h"
#include "gextra.h"
#include "gvareatool.h"
#include "gvdata.h"
#include "gview.h"
#include "gvlayer.h"
#include "gvlinetool.h"
#include "gvmanager.h"
#include "gvmesh.h"
#include "gvnodetool.h"
#include "gvpointtool.h"
#include "gvpoitool.h"
#include "gvpquerylayer.h"
#include "gvproperties.h"
#include "gvraster.h"
#include "gvrasterize.h"
#include "gvrasterlayer.h"
#include "gvrecttool.h"
#include "gvrenderinfo.h"
#include "gvroitool.h"
#include "gvrotatetool.h"
#include "gvselecttool.h"
#include "gvshapelayer.h"
#include "gvshapes.h"
#include "gvshapeslayer.h"
#include "gvsymbolmanager.h"
#include "gvtool.h"
#include "gvtoolbox.h"
#include "gvutils.h"
#include "gvtracktool.h"
#include "gvviewarea.h"
#include "gvviewlink.h"
#include "gvzoompantool.h"
#include "gv-enum-types.h"
#include "gv_override.h"
#include "invdistance.h"
#include "gvshape_wrap.c"
#include "gvautopan.h"

#ifdef CIET_BUILD
#include "gvrecords_wrap.c"
#endif

#line 67 "gv.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGtkDrawingArea_Type;
#define PyGtkDrawingArea_Type (*_PyGtkDrawingArea_Type)
static PyTypeObject *_PyGtkButton_Type;
#define PyGtkButton_Type (*_PyGtkButton_Type)
static PyTypeObject *_PyGtkObject_Type;
#define PyGtkObject_Type (*_PyGtkObject_Type)
static PyTypeObject *_PyGtkAdjustment_Type;
#define PyGtkAdjustment_Type (*_PyGtkAdjustment_Type)
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)
static PyTypeObject *_PyGtkWidget_Type;
#define PyGtkWidget_Type (*_PyGtkWidget_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject G_GNUC_INTERNAL PyGvData_Type;
PyTypeObject G_GNUC_INTERNAL PyGvLayer_Type;
PyTypeObject G_GNUC_INTERNAL PyGvManager_Type;
PyTypeObject G_GNUC_INTERNAL PyGvMesh_Type;
PyTypeObject G_GNUC_INTERNAL PyGvPqueryLayer_Type;
PyTypeObject G_GNUC_INTERNAL PyGvRaster_Type;
PyTypeObject G_GNUC_INTERNAL PyGvRasterLayer_Type;
PyTypeObject G_GNUC_INTERNAL PyGvShapeLayer_Type;
PyTypeObject G_GNUC_INTERNAL PyGvShapes_Type;
PyTypeObject G_GNUC_INTERNAL PyGvShapesLayer_Type;
PyTypeObject G_GNUC_INTERNAL PyGvSymbolManager_Type;
PyTypeObject G_GNUC_INTERNAL PyGvTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvSelectionTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvRotateTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvRoiTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvRectTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvPointTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvPoiTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvNodeTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvLineTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvAreaTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvToolbox_Type;
PyTypeObject G_GNUC_INTERNAL PyGvTrackTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvViewArea_Type;
PyTypeObject G_GNUC_INTERNAL PyGvViewLink_Type;
PyTypeObject G_GNUC_INTERNAL PyGvZoompanTool_Type;
PyTypeObject G_GNUC_INTERNAL PyGvAutopanTool_Type;

#line 114 "gv.c"



/* ----------- GvData ----------- */

static PyObject *
_wrap_gv_data_set_parent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "parent", NULL };
    PyGObject *parent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvData.set_parent", kwlist, &PyGvData_Type, &parent))
        return NULL;
    
    gv_data_set_parent(GV_DATA(self->obj), GV_DATA(parent->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_get_parent(PyGObject *self)
{
    GvData *ret;

    
    ret = gv_data_get_parent(GV_DATA(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_data_set_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvData.set_name", kwlist, &name))
        return NULL;
    
    gv_data_set_name(GV_DATA(self->obj), name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_get_name(PyGObject *self)
{
    const gchar *ret;

    
    ret = gv_data_get_name(GV_DATA(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_destroy(PyGObject *self)
{
    
    gv_data_destroy(GV_DATA(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 874 "gv.override"
static PyObject *
_wrap_gv_data_changing(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x_off", "y_off", "width", "height", NULL };
    GvRaster *raster;
    int      x_off=0, y_off=0, width=0, height=0;
    GvRasterChangeInfo change_info;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|iiii:GvData.changing",
                                    kwlist, &x_off, &y_off, &width, &height ))
        return NULL;

    if (GV_IS_RASTER(self->obj)) {
        raster = GV_RASTER(self->obj);

        if( height > 0 ) {
            change_info.change_type = GV_CHANGE_REPLACE;
            change_info.x_off = x_off;
            change_info.y_off = y_off;
            change_info.width = width;
            change_info.height = height;

            gv_data_changing( GV_DATA(raster), &change_info );
        }
        else
            gv_data_changing( GV_DATA(raster), NULL );
    }
    else {
        PyErr_SetString(PyExc_TypeError, "argument must be a handled GvData type");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 222 "gv.c"


#line 911 "gv.override"
static PyObject *
_wrap_gv_data_changed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x_off", "y_off", "width", "height", NULL };
    GvRaster *raster;
    GvData *data;
    int      x_off=0, y_off=0, width=0, height=0;
    GvRasterChangeInfo change_info;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|iiii:GvData.changed",
                                    kwlist, &x_off, &y_off, &width, &height ))
        return NULL;

    if (GV_IS_RASTER(self->obj)) {
        raster = GV_RASTER(self->obj);

        if( height > 0 ) {
            change_info.change_type = GV_CHANGE_REPLACE;
            change_info.x_off = x_off;
            change_info.y_off = y_off;
            change_info.width = width;
            change_info.height = height;

            gv_data_changed( GV_DATA(raster), &change_info );
        }
        else
            gv_data_changed( GV_DATA(raster), NULL );
    }
    else {
        data = GV_DATA(self->obj);
        if (!GV_IS_DATA(data)) {
            PyErr_SetString(PyExc_TypeError, "data argument must be a GvData object");
            return NULL;
        }
        gv_data_changed( GV_DATA(data), NULL );
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 266 "gv.c"


static PyObject *
_wrap_gv_data_meta_changed(PyGObject *self)
{
    
    gv_data_meta_changed(GV_DATA(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_freeze(PyGObject *self)
{
    
    gv_data_freeze(GV_DATA(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_thaw(PyGObject *self)
{
    
    gv_data_thaw(GV_DATA(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_get_projection(PyGObject *self)
{
    const gchar *ret;

    
    ret = gv_data_get_projection(GV_DATA(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_set_projection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "projection", NULL };
    char *projection;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvData.set_projection", kwlist, &projection))
        return NULL;
    
    gv_data_set_projection(GV_DATA(self->obj), projection);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_set_read_only(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "read_only", NULL };
    int read_only;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvData.set_read_only", kwlist, &read_only))
        return NULL;
    
    gv_data_set_read_only(GV_DATA(self->obj), read_only);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_is_read_only(PyGObject *self)
{
    int ret;

    
    ret = gv_data_is_read_only(GV_DATA(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_data_set_property(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "value", NULL };
    char *name, *value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:GvData.set_property", kwlist, &name, &value))
        return NULL;
    
    gv_data_set_property(GV_DATA(self->obj), name, value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_get_property(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvData.get_property", kwlist, &name))
        return NULL;
    
    ret = gv_data_get_property(GV_DATA(self->obj), name);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 806 "gv.override"
static PyObject *
_wrap_gv_data_get_properties(PyGObject *self)
{
    GvProperties *properties = NULL;
    PyObject *psDict = NULL;

    properties = gv_data_get_properties( GV_DATA(self->obj) );

    psDict = PyDict_New();
    if( properties != NULL )
    {
        int        i, count;

        count = gv_properties_count( properties );
        for( i = 0; i < count; i++ )
        {
            const char *value, *name;
            PyObject *py_name, *py_value;

            value = gv_properties_get_value_by_index(properties,i);
            name = gv_properties_get_name_by_index(properties,i);

            py_name = Py_BuildValue("s",name);
            py_value = Py_BuildValue("s",value);
            PyDict_SetItem( psDict, py_name, py_value );

            Py_DECREF(py_name);
            Py_DECREF(py_value);
        }
    }

    return psDict;
}
#line 421 "gv.c"


#line 841 "gv.override"
static PyObject *
_wrap_gv_data_set_properties(PyGObject *self, PyObject *args)
{
    GvProperties *properties = NULL;
    PyObject *psDict = NULL;
    PyObject    *pyKey = NULL, *pyValue = NULL;
    Py_ssize_t ii;

    if (!PyArg_ParseTuple(args, "O!:GvData.set_properties", &PyDict_Type, &psDict))
        return NULL;

    properties = gv_data_get_properties( GV_DATA(self->obj) );
    gv_properties_clear( properties );

    ii = 0;
    while( PyDict_Next( psDict, &ii, &pyKey, &pyValue ) )
    {
        char *key = NULL, *value = NULL;

        if( !PyArg_Parse( pyKey, "s", &key )
            || !PyArg_Parse( pyValue, "s", &value ))
            continue;

        gv_properties_set( properties, key, value );

        pyKey = pyValue = NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 456 "gv.c"


static const PyMethodDef _PyGvData_methods[] = {
    { "set_parent", (PyCFunction)_wrap_gv_data_set_parent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_parent", (PyCFunction)_wrap_gv_data_get_parent, METH_NOARGS,
      NULL },
    { "set_name", (PyCFunction)_wrap_gv_data_set_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_name", (PyCFunction)_wrap_gv_data_get_name, METH_NOARGS,
      NULL },
    { "destroy", (PyCFunction)_wrap_gv_data_destroy, METH_NOARGS,
      NULL },
    { "changing", (PyCFunction)_wrap_gv_data_changing, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "changed", (PyCFunction)_wrap_gv_data_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "meta_changed", (PyCFunction)_wrap_gv_data_meta_changed, METH_NOARGS,
      NULL },
    { "freeze", (PyCFunction)_wrap_gv_data_freeze, METH_NOARGS,
      NULL },
    { "thaw", (PyCFunction)_wrap_gv_data_thaw, METH_NOARGS,
      NULL },
    { "get_projection", (PyCFunction)_wrap_gv_data_get_projection, METH_NOARGS,
      NULL },
    { "set_projection", (PyCFunction)_wrap_gv_data_set_projection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_read_only", (PyCFunction)_wrap_gv_data_set_read_only, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_read_only", (PyCFunction)_wrap_gv_data_is_read_only, METH_NOARGS,
      NULL },
    { "set_property", (PyCFunction)_wrap_gv_data_set_property, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_property", (PyCFunction)_wrap_gv_data_get_property, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_properties", (PyCFunction)_wrap_gv_data_get_properties, METH_NOARGS,
      NULL },
    { "set_properties", (PyCFunction)_wrap_gv_data_set_properties, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gv_data__get_parent(PyObject *self, void *closure)
{
    GvData *ret;

    ret = GV_DATA(pygobject_get(self))->parent;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_data__get_name(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GV_DATA(pygobject_get(self))->name;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data__get_read_only(PyObject *self, void *closure)
{
    int ret;

    ret = GV_DATA(pygobject_get(self))->read_only;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_data__get_projection(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GV_DATA(pygobject_get(self))->projection;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyGetSetDef gv_data_getsets[] = {
    { "parent", (getter)_wrap_gv_data__get_parent, (setter)0 },
    { "name", (getter)_wrap_gv_data__get_name, (setter)0 },
    { "read_only", (getter)_wrap_gv_data__get_read_only, (setter)0 },
    { "projection", (getter)_wrap_gv_data__get_projection, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGvData_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.Data",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvData_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gv_data_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvLayer ----------- */

static PyObject *
_wrap_gv_layer_setup(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", NULL };
    PyGObject *view;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvLayer.setup", kwlist, &PyGvViewArea_Type, &view))
        return NULL;
    
    gv_layer_setup(GV_LAYER(self->obj), GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_layer_teardown(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", NULL };
    PyGObject *view;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvLayer.teardown", kwlist, &PyGvViewArea_Type, &view))
        return NULL;
    
    gv_layer_teardown(GV_LAYER(self->obj), GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_layer_draw(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", NULL };
    PyGObject *view;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvLayer.draw", kwlist, &PyGvViewArea_Type, &view))
        return NULL;
    
    gv_layer_draw(GV_LAYER(self->obj), GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 953 "gv.override"
static PyObject *
_wrap_gv_layer_extents(PyGObject *self)
{
    GvRect rect;

    gv_layer_extents(GV_LAYER(self->obj), &rect);

    return Py_BuildValue( "(" CCCC ")", rect.x, rect.y, rect.width, rect.height);
}
#line 654 "gv.c"


#line 964 "gv.override"
static PyObject *
_wrap_gv_layer_display_change(PyGObject *self)
{
    gv_layer_display_change(GV_LAYER(self->obj), NULL);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 666 "gv.c"


static PyObject *
_wrap_gv_layer_is_visible(PyGObject *self)
{
    int ret;

    
    ret = gv_layer_is_visible(GV_LAYER(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_layer_set_visible(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "visible", NULL };
    int visible;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvLayer.set_visible", kwlist, &visible))
        return NULL;
    
    gv_layer_set_visible(GV_LAYER(self->obj), visible);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_layer_set_visible_temp(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "visible", NULL };
    int visible, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvLayer.set_visible_temp", kwlist, &visible))
        return NULL;
    
    ret = gv_layer_set_visible_temp(GV_LAYER(self->obj), visible);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_layer_set_presentation(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "presentation", NULL };
    int presentation;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvLayer.set_presentation", kwlist, &presentation))
        return NULL;
    
    gv_layer_set_presentation(GV_LAYER(self->obj), presentation);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_layer_get_view(PyGObject *self)
{
    GvViewArea *ret;

    
    ret = gv_layer_get_view(GV_LAYER(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_layer_reproject(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "projection", NULL };
    char *projection;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvLayer.reproject", kwlist, &projection))
        return NULL;
    
    ret = gv_layer_reproject(GV_LAYER(self->obj), projection);
    
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGvLayer_methods[] = {
    { "setup", (PyCFunction)_wrap_gv_layer_setup, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "teardown", (PyCFunction)_wrap_gv_layer_teardown, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw", (PyCFunction)_wrap_gv_layer_draw, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "extents", (PyCFunction)_wrap_gv_layer_extents, METH_NOARGS,
      NULL },
    { "display_change", (PyCFunction)_wrap_gv_layer_display_change, METH_NOARGS,
      NULL },
    { "is_visible", (PyCFunction)_wrap_gv_layer_is_visible, METH_NOARGS,
      NULL },
    { "set_visible", (PyCFunction)_wrap_gv_layer_set_visible, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_visible_temp", (PyCFunction)_wrap_gv_layer_set_visible_temp, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_presentation", (PyCFunction)_wrap_gv_layer_set_presentation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_view", (PyCFunction)_wrap_gv_layer_get_view, METH_NOARGS,
      NULL },
    { "reproject", (PyCFunction)_wrap_gv_layer_reproject, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvLayer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.Layer",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvLayer_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvManager ----------- */

static int
_wrap_gv_manager_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.Manager.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.Manager object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gv_manager_dump(PyGObject *self)
{
    
    gv_manager_dump(GV_MANAGER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_manager_get_preference(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvManager.get_preference", kwlist, &name))
        return NULL;
    
    ret = gv_manager_get_preference(GV_MANAGER(self->obj), name);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 2978 "gv.override"
static PyObject *
_wrap_gv_manager_get_preferences(PyGObject *self)
{
    GvProperties *properties = NULL;
    PyObject *psDict = NULL;

    properties = gv_manager_get_preferences(GV_MANAGER(self->obj));

    psDict = PyDict_New();
    if( properties != NULL )
    {
        int        i, count;

        count = gv_properties_count( properties );
        for( i = 0; i < count; i++ )
        {
            const char *value, *name;
            PyObject *py_name, *py_value;

            value = gv_properties_get_value_by_index(properties,i);
            name = gv_properties_get_name_by_index(properties,i);

            py_name = Py_BuildValue("s",name);
            py_value = Py_BuildValue("s",value);
            PyDict_SetItem( psDict, py_name, py_value );

            Py_DECREF(py_name);
            Py_DECREF(py_value);
        }
    }

    return psDict;
}
#line 908 "gv.c"


static PyObject *
_wrap_gv_manager_set_preference(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "value", NULL };
    char *name, *value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:GvManager.set_preference", kwlist, &name, &value))
        return NULL;
    
    gv_manager_set_preference(GV_MANAGER(self->obj), name, value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 3013 "gv.override"
static PyObject *
_wrap_gv_manager_add_dataset(PyGObject *self, PyObject *args)
{
    char *dataset_string = NULL;
    char swig_ptr[32];
    GDALDatasetH dataset = NULL;

    if (!PyArg_ParseTuple(args, "s:GvManager.add_dataset", &dataset_string))
        return NULL;

    dataset = (GDALDatasetH) SWIG_SimpleGetPtr(dataset_string, "GDALDatasetH");
    if( dataset == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    dataset = gv_manager_add_dataset(GV_MANAGER(self->obj), dataset);
    SWIG_SimpleMakePtr(swig_ptr, dataset, "_GDALDatasetH");
    return Py_BuildValue("s", swig_ptr);
}
#line 948 "gv.c"


#line 3036 "gv.override"
static PyObject *
_wrap_gv_manager_get_dataset(PyGObject *self, PyObject *args)
{
    char *filename = NULL;
    char swig_ptr[32];
    GDALDatasetH dataset = NULL;

    if (!PyArg_ParseTuple(args, "s:GvManager.get_dataset", &filename))
        return NULL;

    dataset = gv_manager_get_dataset(GV_MANAGER(self->obj), filename);

    if( dataset == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else
    {
        SWIG_SimpleMakePtr(swig_ptr, dataset, "_GDALDatasetH");
        return Py_BuildValue("s", swig_ptr);
    }
}
#line 975 "gv.c"


#line 3061 "gv.override"
static PyObject *
_wrap_gv_manager_get_dataset_raster(PyGObject *self, PyObject *args)
{
    char *dataset_string = NULL;
    int band = 0;
    GDALDatasetH dataset = NULL;
    GvRaster *raster = NULL;

    if (!PyArg_ParseTuple(args, "si:GvManager.get_dataset_raster",
                          &dataset_string, &band))
        return NULL;

    dataset = (GDALDatasetH) SWIG_SimpleGetPtr(dataset_string, "GDALDatasetH");
    if( dataset == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    raster = gv_manager_get_dataset_raster(GV_MANAGER(self->obj), dataset, band);

    return pygobject_new( G_OBJECT(raster) );
}
#line 1002 "gv.c"


static PyObject *
_wrap_gv_manager_set_busy(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "busy_flag", NULL };
    int busy_flag;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvManager.set_busy", kwlist, &busy_flag))
        return NULL;
    
    gv_manager_set_busy(GV_MANAGER(self->obj), busy_flag);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_manager_get_busy(PyGObject *self)
{
    int ret;

    
    ret = gv_manager_get_busy(GV_MANAGER(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 3120 "gv.override"
static PyObject *
_wrap_gv_manager_queue_task(PyGObject *self, PyObject *args)
{
    int       priority;
    char      *task_name;
    PyTaskData *psProgressInfo;

    psProgressInfo = g_new(PyTaskData,1);
    psProgressInfo->psPyCallback = NULL;
    psProgressInfo->psPyCallbackData = Py_None;

    if (!PyArg_ParseTuple(args, "siO|O:GvManager.queue_task",
                          &task_name, &priority,
                          &(psProgressInfo->psPyCallback),
                          &(psProgressInfo->psPyCallbackData)) )
        return NULL;

    Py_XINCREF( psProgressInfo->psPyCallback );
    Py_XINCREF( psProgressInfo->psPyCallbackData );

    psProgressInfo->psThreadState = PyThreadState_Get();

    gv_manager_queue_task(GV_MANAGER(self->obj),
                            task_name, priority,
                            PyIdleTaskProxy,
                            psProgressInfo);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 1062 "gv.c"


#line 3086 "gv.override"
static PyObject *
_wrap_gv_manager_active_rasters(PyGObject *self, PyObject *args)
{
    char *dataset_string = NULL;
    GvDataset *ds = NULL;
    GDALDatasetH dataset = NULL;
    int i, active_rasters = 0;
    
    if (!PyArg_ParseTuple(args, "s:GvManager.active_rasters",
                          &dataset_string))
        return NULL;

    dataset = (GDALDatasetH) SWIG_SimpleGetPtr(dataset_string, "GDALDatasetH");

    if (dataset != NULL) {
        for( i = 0; i < GV_MANAGER(self->obj)->datasets->len; i++ )
        {
            ds = (GvDataset *) g_ptr_array_index(GV_MANAGER(self->obj)->datasets, i);

            if( dataset == ds->dataset ) {
                for( i = 0; i < GDALGetRasterCount(ds->dataset); i++ )
                {
                    if( ds->rasters[i] != NULL )
                        active_rasters++;
                }
                break;
            }
        }
    }

    return PyInt_FromLong(active_rasters);
}
#line 1098 "gv.c"


static const PyMethodDef _PyGvManager_methods[] = {
    { "dump", (PyCFunction)_wrap_gv_manager_dump, METH_NOARGS,
      NULL },
    { "get_preference", (PyCFunction)_wrap_gv_manager_get_preference, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_preferences", (PyCFunction)_wrap_gv_manager_get_preferences, METH_NOARGS,
      NULL },
    { "set_preference", (PyCFunction)_wrap_gv_manager_set_preference, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_dataset", (PyCFunction)_wrap_gv_manager_add_dataset, METH_VARARGS,
      NULL },
    { "get_dataset", (PyCFunction)_wrap_gv_manager_get_dataset, METH_VARARGS,
      NULL },
    { "get_dataset_raster", (PyCFunction)_wrap_gv_manager_get_dataset_raster, METH_VARARGS,
      NULL },
    { "set_busy", (PyCFunction)_wrap_gv_manager_set_busy, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_busy", (PyCFunction)_wrap_gv_manager_get_busy, METH_NOARGS,
      NULL },
    { "queue_task", (PyCFunction)_wrap_gv_manager_queue_task, METH_VARARGS,
      NULL },
    { "active_rasters", (PyCFunction)_wrap_gv_manager_active_rasters, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvManager_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.Manager",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvManager_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_manager_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvMesh ----------- */

PyTypeObject G_GNUC_INTERNAL PyGvMesh_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.Mesh",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvShapeLayer ----------- */

static PyObject *
_wrap_gv_shape_layer_get_first_part_index(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", NULL };
    int shape_id;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.get_first_part_index", kwlist, &shape_id))
        return NULL;
    
    ret = gv_shape_layer_get_first_part_index(GV_SHAPE_LAYER(self->obj), shape_id);
    
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gv_shape_layer_add_part(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", "part_type", NULL };
    int shape_id, part_type;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GvShapeLayer.add_part", kwlist, &shape_id, &part_type))
        return NULL;
    
    ret = gv_shape_layer_add_part(GV_SHAPE_LAYER(self->obj), shape_id, part_type);
    
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gv_shape_layer_create_part(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "part_type", NULL };
    int part_type;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.create_part", kwlist, &part_type))
        return NULL;
    
    ret = gv_shape_layer_create_part(GV_SHAPE_LAYER(self->obj), part_type);
    
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gv_shape_layer_chain_part(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "base_part_index", "new_part_index", NULL };
    int base_part_index, new_part_index;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GvShapeLayer.chain_part", kwlist, &base_part_index, &new_part_index))
        return NULL;
    
    ret = gv_shape_layer_chain_part(GV_SHAPE_LAYER(self->obj), base_part_index, new_part_index);
    
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gv_shape_layer_clear_shape_parts(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", NULL };
    int shape_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.clear_shape_parts", kwlist, &shape_id))
        return NULL;
    
    gv_shape_layer_clear_shape_parts(GV_SHAPE_LAYER(self->obj), shape_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_clear_part(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "part_index", NULL };
    PyObject *py_part_index = NULL;
    guint part_index = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GvShapeLayer.clear_part", kwlist, &py_part_index))
        return NULL;
    if (py_part_index) {
        if (PyLong_Check(py_part_index))
            part_index = PyLong_AsUnsignedLong(py_part_index);
        else if (PyInt_Check(py_part_index))
            part_index = PyInt_AsLong(py_part_index);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'part_index' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    gv_shape_layer_clear_part(GV_SHAPE_LAYER(self->obj), part_index);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_clear_all_renderinfo(PyGObject *self)
{
    
    gv_shape_layer_clear_all_renderinfo(GV_SHAPE_LAYER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_update_renderinfo(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", NULL };
    int shape_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.update_renderinfo", kwlist, &shape_id))
        return NULL;
    
    gv_shape_layer_update_renderinfo(GV_SHAPE_LAYER(self->obj), shape_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_initialize_renderindex(PyGObject *self)
{
    
    gv_shape_layer_initialize_renderindex(GV_SHAPE_LAYER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_select_shape(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", NULL };
    int shape_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.select_shape", kwlist, &shape_id))
        return NULL;
    
    gv_shape_layer_select_shape(GV_SHAPE_LAYER(self->obj), shape_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_deselect_shape(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", NULL };
    int shape_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.deselect_shape", kwlist, &shape_id))
        return NULL;
    
    gv_shape_layer_deselect_shape(GV_SHAPE_LAYER(self->obj), shape_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_clear_selection(PyGObject *self)
{
    
    gv_shape_layer_clear_selection(GV_SHAPE_LAYER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_select_all(PyGObject *self)
{
    
    gv_shape_layer_select_all(GV_SHAPE_LAYER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_draw_selected(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "when", "view", NULL };
    PyObject *py_when = NULL;
    PyGObject *view;
    guint when = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO!:GvShapeLayer.draw_selected", kwlist, &py_when, &PyGvViewArea_Type, &view))
        return NULL;
    if (py_when) {
        if (PyLong_Check(py_when))
            when = PyLong_AsUnsignedLong(py_when);
        else if (PyInt_Check(py_when))
            when = PyInt_AsLong(py_when);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'when' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    gv_shape_layer_draw_selected(GV_SHAPE_LAYER(self->obj), when, GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_is_selected(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", NULL };
    int shape_id, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.is_selected", kwlist, &shape_id))
        return NULL;
    
    ret = gv_shape_layer_is_selected(GV_SHAPE_LAYER(self->obj), shape_id);
    
    return PyInt_FromLong(ret);
}

#line 1635 "gv.override"
static PyObject *
_wrap_gv_shape_layer_get_selected(PyGObject *self, PyObject *args)
{
    GvShapeLayer *layer;
    PyObject  *list;
    GArray    *array;
    int       i;

    layer = GV_SHAPE_LAYER(self->obj);
    if (!GV_IS_SHAPE_LAYER(layer)) {
        PyErr_SetString(PyExc_TypeError, "argument must be a GvShapeLayer");
        return NULL;
    }

    array = g_array_new(FALSE,TRUE,sizeof(gint));
    gv_shape_layer_get_selected(layer, array);

    list = PyList_New(array->len);
    for( i = 0; i < array->len; i++ )
    {
        PyList_SetItem( list, i,
                        Py_BuildValue("i", g_array_index(array,gint,i)) );
    }

    g_array_free( array, TRUE );

    return list;
}
#line 1481 "gv.c"


static PyObject *
_wrap_gv_shape_layer_delete_selected(PyGObject *self)
{
    
    gv_shape_layer_delete_selected(GV_SHAPE_LAYER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_subselect_shape(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", NULL };
    int shape_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.subselect_shape", kwlist, &shape_id))
        return NULL;
    
    gv_shape_layer_subselect_shape(GV_SHAPE_LAYER(self->obj), shape_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_get_subselection(PyGObject *self)
{
    int ret;

    
    ret = gv_shape_layer_get_subselection(GV_SHAPE_LAYER(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_shape_layer_set_scale_dep(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", "dep", NULL };
    int shape_id, dep;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GvShapeLayer.set_scale_dep", kwlist, &shape_id, &dep))
        return NULL;
    
    gv_shape_layer_set_scale_dep(GV_SHAPE_LAYER(self->obj), shape_id, dep);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_get_scale_dep(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", NULL };
    int shape_id, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.get_scale_dep", kwlist, &shape_id))
        return NULL;
    
    ret = gv_shape_layer_get_scale_dep(GV_SHAPE_LAYER(self->obj), shape_id);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_shape_layer_node_motion(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_id", NULL };
    int shape_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.node_motion", kwlist, &shape_id))
        return NULL;
    
    gv_shape_layer_node_motion(GV_SHAPE_LAYER(self->obj), shape_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1665 "gv.override"
static PyObject *
_wrap_gv_shape_layer_pick_shape(PyGObject *self, PyObject *args)
{
    PyGObject *py_view;
    GvShapeLayer *layer;
    GvViewArea *view;
    gint shape_id;
    float x, y;

    if (!PyArg_ParseTuple(args, "O!ff:gv_shape_layer_pick_shape",
                          &PyGvViewArea_Type, &py_view, &x, &y ))
        return NULL;

    layer = GV_SHAPE_LAYER(self->obj);
    if (!GV_IS_SHAPE_LAYER(layer)) {
        PyErr_SetString(PyExc_TypeError, "argument must be a GvShapeLayer");
        return NULL;
    }

    view = GV_VIEW_AREA(py_view->obj);
    if (!GV_IS_VIEW_AREA(view)) {
        PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
        return NULL;
    }

    if (gv_shape_layer_pick_shape(layer, view, x, y, &shape_id )) {
        return PyInt_FromLong( (long)shape_id );
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 1599 "gv.c"


static PyObject *
_wrap_gv_shape_layer_set_num_shapes(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "num_shapes", NULL };
    int num_shapes;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvShapeLayer.set_num_shapes", kwlist, &num_shapes))
        return NULL;
    
    gv_shape_layer_set_num_shapes(GV_SHAPE_LAYER(self->obj), num_shapes);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1612 "gv.override"
static PyObject *
_wrap_gv_shape_layer_set_color(PyGObject *self, PyObject *args)
{
    GvShapeLayer *layer;
    GvColor color;

    if (!PyArg_ParseTuple(args, "(ffff):gv_shape_layer_set_color",
                            &color[0], &color[1], &color[2], &color[3]))
        return NULL;

    layer = GV_SHAPE_LAYER(self->obj);
    if (!GV_IS_SHAPE_LAYER(layer)) {
        PyErr_SetString(PyExc_TypeError, "argument must be a GvShapeLayer");
        return NULL;
    }

    gv_shape_layer_set_color(layer, color);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 1639 "gv.c"


static const PyMethodDef _PyGvShapeLayer_methods[] = {
    { "get_first_part_index", (PyCFunction)_wrap_gv_shape_layer_get_first_part_index, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_part", (PyCFunction)_wrap_gv_shape_layer_add_part, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "create_part", (PyCFunction)_wrap_gv_shape_layer_create_part, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "chain_part", (PyCFunction)_wrap_gv_shape_layer_chain_part, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear_shape_parts", (PyCFunction)_wrap_gv_shape_layer_clear_shape_parts, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear_part", (PyCFunction)_wrap_gv_shape_layer_clear_part, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear_all_renderinfo", (PyCFunction)_wrap_gv_shape_layer_clear_all_renderinfo, METH_NOARGS,
      NULL },
    { "update_renderinfo", (PyCFunction)_wrap_gv_shape_layer_update_renderinfo, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "initialize_renderindex", (PyCFunction)_wrap_gv_shape_layer_initialize_renderindex, METH_NOARGS,
      NULL },
    { "select_shape", (PyCFunction)_wrap_gv_shape_layer_select_shape, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "deselect_shape", (PyCFunction)_wrap_gv_shape_layer_deselect_shape, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear_selection", (PyCFunction)_wrap_gv_shape_layer_clear_selection, METH_NOARGS,
      NULL },
    { "select_all", (PyCFunction)_wrap_gv_shape_layer_select_all, METH_NOARGS,
      NULL },
    { "draw_selected", (PyCFunction)_wrap_gv_shape_layer_draw_selected, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_selected", (PyCFunction)_wrap_gv_shape_layer_is_selected, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_selected", (PyCFunction)_wrap_gv_shape_layer_get_selected, METH_VARARGS,
      NULL },
    { "delete_selected", (PyCFunction)_wrap_gv_shape_layer_delete_selected, METH_NOARGS,
      NULL },
    { "subselect_shape", (PyCFunction)_wrap_gv_shape_layer_subselect_shape, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_subselection", (PyCFunction)_wrap_gv_shape_layer_get_subselection, METH_NOARGS,
      NULL },
    { "set_scale_dep", (PyCFunction)_wrap_gv_shape_layer_set_scale_dep, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_scale_dep", (PyCFunction)_wrap_gv_shape_layer_get_scale_dep, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "node_motion", (PyCFunction)_wrap_gv_shape_layer_node_motion, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pick_shape", (PyCFunction)_wrap_gv_shape_layer_pick_shape, METH_VARARGS,
      NULL },
    { "set_num_shapes", (PyCFunction)_wrap_gv_shape_layer_set_num_shapes, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_color", (PyCFunction)_wrap_gv_shape_layer_set_color, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvShapeLayer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.ShapeLayer",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvShapeLayer_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvShapesLayer ----------- */

#line 1701 "gv.override"
static int
_wrap_gv_shapes_layer_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shapes", NULL };
    PyGObject *py_shapes;
    GvShapes *shapes_obj = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O:_gv.ShapesLayer.__init__",
                                     kwlist, &py_shapes))
        return -1;

    /* ---- Get shapes or NULL ---- */
    if ((PyObject*)py_shapes == Py_None) {
        shapes_obj = g_object_new(GV_TYPE_SHAPES, NULL);
    }
    else if (pygobject_check(py_shapes, &PyGvShapes_Type)) {
        shapes_obj = GV_SHAPES (pygobject_get(py_shapes));
    }
    else {
        PyErr_SetString(PyExc_ValueError, "Incorrect shapes argument type");
        return -1;
    }

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GvShapesLayer object");
        return -1;
    }

    if (shapes_obj != NULL) {
        gv_shapes_layer_set_data(GV_SHAPES_LAYER(self->obj), shapes_obj);
        /* MB: TODO: see if we need to g_object_unref shapes_obj */
    }

    return 0;
}
#line 1782 "gv.c"


static PyObject *
_wrap_gv_shapes_layer_set_data(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", NULL };
    PyGObject *data;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvShapesLayer.set_data", kwlist, &PyGvShapes_Type, &data))
        return NULL;
    
    gv_shapes_layer_set_data(GV_SHAPES_LAYER(self->obj), GV_SHAPES(data->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1741 "gv.override"
static PyObject *
_wrap_gv_shapes_layer_get_symbol_manager(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ok_to_create", NULL };
    int ok_to_create = 0;
    GObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|i:GvShapesLayer.get_symbol_manager",
                                    kwlist, &ok_to_create))
        return NULL;

    ret = gv_shapes_layer_get_symbol_manager(GV_SHAPES_LAYER(self->obj), ok_to_create);

    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}
#line 1817 "gv.c"


static const PyMethodDef _PyGvShapesLayer_methods[] = {
    { "set_data", (PyCFunction)_wrap_gv_shapes_layer_set_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_symbol_manager", (PyCFunction)_wrap_gv_shapes_layer_get_symbol_manager, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvShapesLayer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.ShapesLayer",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvShapesLayer_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_shapes_layer_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvPqueryLayer ----------- */

#line 1759 "gv.override"
static int
_wrap_gv_pquery_layer_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shapes", NULL };
    PyGObject *py_shapes;
    GvShapes *shapes_obj = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O:_gv.PqueryLayer.__init__",
                                     kwlist, &py_shapes))
        return -1;

    /* ---- Get shapes or NULL ---- */
    if ((PyObject*)py_shapes == Py_None) {
        shapes_obj = g_object_new(GV_TYPE_SHAPES, NULL);
        gv_data_set_name( GV_DATA(shapes_obj), "Query Points" );
    }
    else if (PyObject_TypeCheck(py_shapes, &PyGvShapes_Type)) {
        shapes_obj = GV_SHAPES(py_shapes->obj);
    }
    else {
        PyErr_SetString(PyExc_ValueError, "Incorrect shapes argument type");
        return -1;
    }

    /* ---- Create new layer with shapes ---- */
    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GvPqueryLayer object");
        return -1;
    }

    if (shapes_obj != NULL) {
        gv_shapes_layer_set_data(GV_SHAPES_LAYER(self->obj), shapes_obj);
        /* MB: TODO: see if we need to g_object_unref shapes_obj */
    }

    return 0;
}
#line 1916 "gv.c"


PyTypeObject G_GNUC_INTERNAL PyGvPqueryLayer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.PqueryLayer",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_pquery_layer_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvRaster ----------- */

#line 1801 "gv.override"
static int
_wrap_gv_raster_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    PyGObject *py_raster = NULL;
    char *filename = NULL, *dataset_string = NULL;
    GDALDatasetH  dataset;
    static int gdal_initialized = 0;
    GvSampleMethod sm = GvSMAverage;
    int   rband = 1;
    static char *kwlist[] = {"filename", "sample", "real", "_obj",
                             "dataset", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ziiO!z:_gv.Raster.__init__", kwlist,
                                     &filename, &sm, &rband,
                                     &PyGvRaster_Type, &py_raster,
                                     &dataset_string))
        return -1;

    if( !gdal_initialized )
    {
        GDALAllRegister();
        gdal_initialized = 1;
    }

    if( dataset_string != NULL )
    {
        dataset = (GDALDatasetH)
            SWIG_SimpleGetPtr(dataset_string, "GDALDatasetH" );
        if (dataset == NULL)
        {
            PyErr_SetString(PyExc_IOError,
                 "Unable to extract GDALDatasetH handle in gv_raster_new()");
            return -1;
        }
    }
    else if( filename != NULL )
    {
        dataset = GDALOpen( filename, GA_ReadOnly );

        if (dataset == NULL)
        {
            PyErr_SetString(PyExc_IOError, "failed to open data file");
            return -1;
        }

        GDALDereferenceDataset( dataset );
    }
    else if( py_raster != NULL )
    {
        pygobject_constructv(self, 0, NULL);
        // we need to unref here otherwise it will remain a floating GvRaster... 
        g_object_unref(self->obj);
        self->obj = py_raster->obj;
        return 0;
    }
    else
    {
        PyErr_SetString(PyExc_IOError,
                        "_gv.Raster.__init__: either a filename, a dataset handle,"
                        " or a _gv.Raster is required.  Neither provided." );
        return -1;
    }

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create _gv.Raster object");
        return -1;
    }

    gv_raster_read(GV_RASTER(self->obj), dataset, rband, sm);

    return 0;
}
#line 2042 "gv.c"


#line 1943 "gv.override"
static PyObject *
_wrap_gv_raster_flush_cache(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x_off", "y_off", "width", "height", NULL };
    int x_off = 0, y_off = 0, width = 0, height = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|iiii:GvRaster.flush_cache",
                                    kwlist, &x_off, &y_off, &width, &height))
        return NULL;

    gv_raster_flush_cache(GV_RASTER(self->obj), x_off, y_off, width, height);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 2061 "gv.c"


#line 1974 "gv.override"
static PyObject *
_wrap_gv_raster_get_sample(PyGObject *self, PyObject *args)
{
    GvRaster *raster;
    double x, y, real, imaginary;

    if (!PyArg_ParseTuple(args, "dd:GvRaster.get_sample", &x, &y))
        return NULL;

    raster = GV_RASTER(self->obj);

    if( !gv_raster_get_sample(raster, x, y, &real, &imaginary ) )
        return NULL;
    else if( GDALDataTypeIsComplex(raster->gdal_type) )
        return Py_BuildValue( "(ff)", real, imaginary );
    else
        return Py_BuildValue( "f", real );
}
#line 2083 "gv.c"


static PyObject *
_wrap_gv_raster_pixel_size(PyGObject *self)
{
    double ret;

    
    ret = gv_raster_pixel_size(GV_RASTER(self->obj));
    
    return PyFloat_FromDouble(ret);
}

#line 2015 "gv.override"
static PyObject *
_wrap_gv_raster_pixel_to_georef(PyGObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd:GvRaster.pixel_to_georef", &x, &y))
        return NULL;

    if( gv_raster_pixel_to_georef(GV_RASTER(self->obj), &x, &y, NULL ) )
    {
        return Py_BuildValue("(ff)", x, y);
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "georef_to_pixel transformation failed." );
        return NULL;
    }
}
#line 2117 "gv.c"


#line 1994 "gv.override"
static PyObject *
_wrap_gv_raster_georef_to_pixel(PyGObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd:GvRaster.georef_to_pixel", &x, &y ))
        return NULL;

    if( gv_raster_georef_to_pixel(GV_RASTER(self->obj), &x, &y, NULL) )
    {
        return Py_BuildValue( "(ff)", x, y );
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "georef_to_pixel transformation failed." );
        return NULL;
    }
}
#line 2140 "gv.c"


#line 1878 "gv.override"
static PyObject *
_wrap_gv_raster_autoscale(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"alg", "alg_param", "assign", NULL};
    int alg = GvASAAutomatic, assign = 0, success;
    double alg_param = -1.0;
    GvRaster *raster;
    double   out_min, out_max;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|idi:GvRaster.autoscale", 
                                    kwlist, &alg, &alg_param, &assign))
        return NULL;

    raster = GV_RASTER(self->obj);

    if( assign == 0 )
    {
        success = gv_raster_autoscale(raster, alg, alg_param, 0, NULL,
                                      NULL, NULL);
        if( !success )
        {
            PyErr_SetString(PyExc_RuntimeError,
                            "GvRaster.autoscale() failed, failed to get samples?" );
            return NULL;
        }

        return Py_BuildValue("(dd)", raster->min, raster->max);
    }
    else
    {
        success = gv_raster_autoscale(raster, alg, alg_param,
                                      0, NULL,
                                      &out_min, &out_max);

        if( !success )
        {
            PyErr_SetString(PyExc_RuntimeError,
                            "GvRaster.autoscale() failed, failed to get samples?" );
            return NULL;
        }

        return Py_BuildValue("(dd)", out_min, out_max );
    }
}
#line 2188 "gv.c"


#line 2133 "gv.override"
static PyObject *
_wrap_gv_raster_set_gcps(PyGObject *self, PyObject *args)
{
    GvRaster *raster;
    GDAL_GCP *pasGCPList;
    PyObject *psList;
    int iGCP, nGCPCount, success;

    if (!PyArg_ParseTuple(args,"O!:GvRaster.set_gcps", &PyList_Type, &psList))
        return NULL;

    raster = GV_RASTER(self->obj);

    nGCPCount = PyList_Size(psList);
    pasGCPList = (GDAL_GCP *) CPLCalloc(sizeof(GDAL_GCP),nGCPCount);
    GDALInitGCPs( nGCPCount, pasGCPList );

    for( iGCP = 0; iGCP < nGCPCount; iGCP++ )
    {
        char *pszId = NULL, *pszInfo = NULL;

        if( !PyArg_Parse( PyList_GET_ITEM(psList,iGCP), "(ssddddd)",
                      &pszId, &pszInfo,
                      &(pasGCPList[iGCP].dfGCPPixel),
                      &(pasGCPList[iGCP].dfGCPLine),
                      &(pasGCPList[iGCP].dfGCPX),
                      &(pasGCPList[iGCP].dfGCPY),
                      &(pasGCPList[iGCP].dfGCPZ) ) )
        {
            PyErr_SetString(PyExc_ValueError, "improper GCP tuple");
            return NULL;
        }

        CPLFree( pasGCPList[iGCP].pszId );
        pasGCPList[iGCP].pszId = CPLStrdup(pszId);
        CPLFree( pasGCPList[iGCP].pszInfo );
        pasGCPList[iGCP].pszInfo = CPLStrdup(pszInfo);
    }

    success = gv_raster_set_gcps( raster, nGCPCount, pasGCPList );

    GDALDeinitGCPs( nGCPCount, pasGCPList );
    CPLFree( pasGCPList );

    return Py_BuildValue("d", success);
}
#line 2238 "gv.c"


static PyObject *
_wrap_gv_raster_get_gcp_count(PyGObject *self)
{
    int ret;

    
    ret = gv_raster_get_gcp_count(GV_RASTER(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 2102 "gv.override"
static PyObject *
_wrap_gv_raster_get_gcps(PyGObject *self)
{
    GvRaster *raster;
    const GDAL_GCP * pasGCPList;
    PyObject *psList;
    int iGCP;

    raster = GV_RASTER(self->obj);
    pasGCPList = gv_raster_get_gcps(raster);

    psList = PyList_New(raster->gcp_count);
    for( iGCP = 0; pasGCPList != NULL && iGCP < raster->gcp_count; iGCP++)
    {
        PyObject *py_item;

        py_item = Py_BuildValue("(ssddddd)",
                                pasGCPList[iGCP].pszId,
                                pasGCPList[iGCP].pszInfo,
                                pasGCPList[iGCP].dfGCPPixel,
                                pasGCPList[iGCP].dfGCPLine,
                                pasGCPList[iGCP].dfGCPX,
                                pasGCPList[iGCP].dfGCPY,
                                pasGCPList[iGCP].dfGCPZ );
        PyList_SetItem(psList, iGCP, py_item );
    }

    return psList;
}
#line 2282 "gv.c"


#line 2212 "gv.override"
static PyObject *
_wrap_gv_raster_set_gcpsCL(PyGObject *self, PyObject *args)
{
    GvRaster *raster;
    GDAL_GCP *pasGCPList;
    PyObject *psList;
    int iGCP, nGCPCount, success, poly_order;

    if (!PyArg_ParseTuple(args,"O!i:GvRaster.set_gcpsCL",
                            &PyList_Type, &psList,
                            &poly_order))
        return NULL;

    raster = GV_RASTER(self->obj);

    nGCPCount = PyList_Size(psList);
    pasGCPList = (GDAL_GCP *) CPLCalloc(sizeof(GDAL_GCP),nGCPCount);
    GDALInitGCPs( nGCPCount, pasGCPList );

    for( iGCP = 0; iGCP < nGCPCount; iGCP++ )
    {
        char *pszId = NULL, *pszInfo = NULL;

        if( !PyArg_Parse( PyList_GET_ITEM(psList,iGCP), "(ssddddd)",
                      &pszId, &pszInfo,
                      &(pasGCPList[iGCP].dfGCPPixel),
                      &(pasGCPList[iGCP].dfGCPLine),
                      &(pasGCPList[iGCP].dfGCPX),
                      &(pasGCPList[iGCP].dfGCPY),
                      &(pasGCPList[iGCP].dfGCPZ) ) )
        {
            PyErr_SetString(PyExc_ValueError, "improper GCP tuple");
            return NULL;
        }

        CPLFree( pasGCPList[iGCP].pszId );
        pasGCPList[iGCP].pszId = CPLStrdup(pszId);
        CPLFree( pasGCPList[iGCP].pszInfo );
        pasGCPList[iGCP].pszInfo = CPLStrdup(pszInfo);
    }

    success = gv_raster_set_gcpsCL( raster, nGCPCount, pasGCPList,poly_order);

    GDALDeinitGCPs( nGCPCount, pasGCPList );
    CPLFree( pasGCPList );

    return Py_BuildValue("d", success);
}

#line 2335 "gv.c"


static PyObject *
_wrap_gv_raster_get_gcp_countCL(PyGObject *self)
{
    int ret;

    
    ret = gv_raster_get_gcp_countCL(GV_RASTER(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 2181 "gv.override"
static PyObject *
_wrap_gv_raster_get_gcpsCL(PyGObject *self)
{
    GvRaster *raster;
    const GDAL_GCP * pasGCPList;
    PyObject *psList;
    int iGCP;

    raster = GV_RASTER(self->obj);
    pasGCPList = gv_raster_get_gcpsCL( raster );

    psList = PyList_New(raster->gcp_countCL);
    for( iGCP = 0; pasGCPList != NULL && iGCP < raster->gcp_countCL; iGCP++)
    {
        PyObject *py_item;

        py_item = Py_BuildValue("(ssddddd)",
                                pasGCPList[iGCP].pszId,
                                pasGCPList[iGCP].pszInfo,
                                pasGCPList[iGCP].dfGCPPixel,
                                pasGCPList[iGCP].dfGCPLine,
                                pasGCPList[iGCP].dfGCPX,
                                pasGCPList[iGCP].dfGCPY,
                                pasGCPList[iGCP].dfGCPZ );
        PyList_SetItem(psList, iGCP, py_item );
    }

    return psList;
}
#line 2379 "gv.c"


static PyObject *
_wrap_gv_raster_set_poly_order_preference(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "poly_order", NULL };
    int poly_order;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvRaster.set_poly_order_preference", kwlist, &poly_order))
        return NULL;
    
    gv_raster_set_poly_order_preference(GV_RASTER(self->obj), poly_order);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 2277 "gv.override"
static PyObject *
_wrap_gv_raster_get_dataset(PyGObject *self)
{
    char swig_ptr[32];
    GDALDatasetH dataset;

    dataset = GV_RASTER(self->obj)->dataset;
    if( dataset == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    SWIG_SimpleMakePtr( swig_ptr, dataset, "_GDALDatasetH" );
    return Py_BuildValue("s", swig_ptr);
}
#line 2414 "gv.c"


#line 2295 "gv.override"
static PyObject *
_wrap_gv_raster_rasterize_shapes(PyGObject *self, PyObject *args)
{
    PyObject *py_shapelist;
    double   burn_value;
    int      shape_count, i, ret_value, fill_short = 1;
    GvShape  **shape_list;

    if (!PyArg_ParseTuple(args, "O!d|i:GvRaster.rasterize_shapes",
                            &PyList_Type, &py_shapelist,
                            &burn_value, &fill_short))
        return NULL;

    shape_count = PyList_Size(py_shapelist);
    shape_list = g_new(GvShape*,shape_count);
    for (i = 0; i < shape_count; i++)
    {
        PyGvShape *py_shape;

        if (!PyArg_Parse( PyList_GET_ITEM(py_shapelist, i), "O", &py_shape ))
        {
            g_free(shape_list);
            PyErr_SetString(PyExc_ValueError, "bad item in shapelist");
            return NULL;
        }

        shape_list[i] = GV_SHAPE(py_shape);
    }

    ret_value = gv_raster_rasterize_shapes(GV_RASTER(self->obj), shape_count,
                                            shape_list, burn_value, fill_short);

    return Py_BuildValue("i", ret_value);
}
#line 2452 "gv.c"


#line 2036 "gv.override"
static PyObject *
_wrap_gv_raster_cursor_link_georef_to_pixel(PyGObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd:GvRaster.cursor_link_georef_to_pixel", &x, &y))
        return NULL;

    if( gv_raster_georef_to_pixelCL(GV_RASTER(self->obj), &x, &y, NULL) )
    {
        return Py_BuildValue( "(ff)", x, y );
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "georef_to_pixelCL transformation failed." );
        return NULL;
    }
}
#line 2475 "gv.c"


#line 2078 "gv.override"
static PyObject *
_wrap_gv_raster_get_change_info(PyGObject *self, PyObject *args)
{
    GvRasterChangeInfo *change_info;
    PyObject *c_change_info;

    if (!PyArg_ParseTuple(args, "O:GvRaster.get_change_info", &c_change_info))
        return NULL;

    if (!pygobject_check(c_change_info, &PyGPointer_Type))
        return NULL;

    change_info = pyg_pointer_get(c_change_info, GvRasterChangeInfo);

    return Py_BuildValue( "(iiiii)",
                          change_info->change_type,
                          change_info->x_off,
                          change_info->y_off,
                          change_info->width,
                          change_info->height
                         );
}
#line 2501 "gv.c"


#line 2057 "gv.override"
static PyObject *
_wrap_gv_raster_cursor_link_pixel_to_georef(PyGObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd:GvRaster.cursor_link_pixel_to_georef", &x, &y))
        return NULL;

    if( gv_raster_pixel_to_georefCL(GV_RASTER(self->obj), &x, &y, NULL) )
    {
        return Py_BuildValue("(ff)", x, y);
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "georef_to_pixelCL transformation failed." );
        return NULL;
    }
}
#line 2524 "gv.c"


#line 1924 "gv.override"
static PyObject *
_wrap_gv_raster_get_band(PyGObject *self)
{
    char swig_ptr[32];
    GDALRasterBandH band;

    band = GV_RASTER(self->obj)->gdal_band;

    if( band == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    SWIG_SimpleMakePtr( swig_ptr, band, "_GDALRasterBandH" );
    return Py_BuildValue("s", swig_ptr);
}
#line 2545 "gv.c"


#line 2270 "gv.override"
static PyObject *
_wrap_gv_raster_get_max(PyGObject *self)
{
    return PyFloat_FromDouble(GV_RASTER(self->obj)->max);
}
#line 2554 "gv.c"


#line 1960 "gv.override"
static PyObject *
_wrap_gv_raster_force_load(PyGObject *self)
{
    int       i;
    GvRaster *raster = GV_RASTER(self->obj);

    for( i = 0; i < raster->max_tiles; i++ )
        gv_raster_tile_get( raster, i, 0 );

    Py_INCREF(Py_None);
    return Py_None;
}
#line 2570 "gv.c"


#line 2263 "gv.override"
static PyObject *
_wrap_gv_raster_get_min(PyGObject *self)
{
    return PyFloat_FromDouble(GV_RASTER(self->obj)->min);
}
#line 2579 "gv.c"


#line 2331 "gv.override"
static PyObject *
_wrap_gv_raster_wid_interpolate(PyGObject *self, PyObject *args)
{
    PyObject *poPyPoints;
    PyProgressData sProgressInfo;
    int nPoints, i, nErr;
    double *padfXYVW;
    double fExponent = 2.0;
    GDALRasterBandH hBand;

    sProgressInfo.psPyCallback = NULL;
    sProgressInfo.psPyCallbackData = NULL;

    if (!PyArg_ParseTuple(args, "O!|dOO:GvRaster.wid_interpolate",
                          &PyList_Type, &poPyPoints,
                          &fExponent,
                          &(sProgressInfo.psPyCallback),
                          &(sProgressInfo.psPyCallbackData) ) )
        return NULL;

    hBand = GV_RASTER(self->obj)->gdal_band;

    if (hBand == NULL)
    {
        PyErr_SetString(PyExc_ValueError, "Couldn't fetch GDAL band from GvRaster.");
        return NULL;
    }

    nPoints = PyList_Size(poPyPoints);
    padfXYVW = g_new(double,4*nPoints);
    for (i = 0; i < nPoints; i++)
    {
        if (!PyArg_Parse( PyList_GET_ITEM(poPyPoints,i), "(dddd)",
                          padfXYVW + i + 0*nPoints,
                          padfXYVW + i + 1*nPoints,
                          padfXYVW + i + 2*nPoints,
                          padfXYVW + i + 3*nPoints ))
        {
            g_free(padfXYVW);
            PyErr_SetString(PyExc_ValueError, "bad point format (x,y,value,weight)");
            return NULL;
        }
    }

    nErr = WIDInterpolate( nPoints, padfXYVW, padfXYVW+nPoints,
                           padfXYVW+nPoints*2, padfXYVW+nPoints*3, hBand,
                           fExponent, PyProgressProxy, &sProgressInfo );

    g_free(padfXYVW);

    return Py_BuildValue("i", nErr);
}
#line 2635 "gv.c"


static const PyMethodDef _PyGvRaster_methods[] = {
    { "flush_cache", (PyCFunction)_wrap_gv_raster_flush_cache, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_sample", (PyCFunction)_wrap_gv_raster_get_sample, METH_VARARGS,
      NULL },
    { "pixel_size", (PyCFunction)_wrap_gv_raster_pixel_size, METH_NOARGS,
      NULL },
    { "pixel_to_georef", (PyCFunction)_wrap_gv_raster_pixel_to_georef, METH_VARARGS,
      NULL },
    { "georef_to_pixel", (PyCFunction)_wrap_gv_raster_georef_to_pixel, METH_VARARGS,
      NULL },
    { "autoscale", (PyCFunction)_wrap_gv_raster_autoscale, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_gcps", (PyCFunction)_wrap_gv_raster_set_gcps, METH_VARARGS,
      NULL },
    { "get_gcp_count", (PyCFunction)_wrap_gv_raster_get_gcp_count, METH_NOARGS,
      NULL },
    { "get_gcps", (PyCFunction)_wrap_gv_raster_get_gcps, METH_NOARGS,
      NULL },
    { "set_gcpsCL", (PyCFunction)_wrap_gv_raster_set_gcpsCL, METH_VARARGS,
      NULL },
    { "get_gcp_countCL", (PyCFunction)_wrap_gv_raster_get_gcp_countCL, METH_NOARGS,
      NULL },
    { "get_gcpsCL", (PyCFunction)_wrap_gv_raster_get_gcpsCL, METH_NOARGS,
      NULL },
    { "set_poly_order_preference", (PyCFunction)_wrap_gv_raster_set_poly_order_preference, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_dataset", (PyCFunction)_wrap_gv_raster_get_dataset, METH_NOARGS,
      NULL },
    { "rasterize_shapes", (PyCFunction)_wrap_gv_raster_rasterize_shapes, METH_VARARGS,
      NULL },
    { "cursor_link_georef_to_pixel", (PyCFunction)_wrap_gv_raster_cursor_link_georef_to_pixel, METH_VARARGS,
      NULL },
    { "get_change_info", (PyCFunction)_wrap_gv_raster_get_change_info, METH_VARARGS,
      NULL },
    { "cursor_link_pixel_to_georef", (PyCFunction)_wrap_gv_raster_cursor_link_pixel_to_georef, METH_VARARGS,
      NULL },
    { "get_band", (PyCFunction)_wrap_gv_raster_get_band, METH_NOARGS,
      NULL },
    { "get_max", (PyCFunction)_wrap_gv_raster_get_max, METH_NOARGS,
      NULL },
    { "force_load", (PyCFunction)_wrap_gv_raster_force_load, METH_NOARGS,
      NULL },
    { "get_min", (PyCFunction)_wrap_gv_raster_get_min, METH_NOARGS,
      NULL },
    { "wid_interpolate", (PyCFunction)_wrap_gv_raster_wid_interpolate, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gv_raster__get_width(PyObject *self, void *closure)
{
    int ret;

    ret = GV_RASTER(pygobject_get(self))->width;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_raster__get_height(PyObject *self, void *closure)
{
    int ret;

    ret = GV_RASTER(pygobject_get(self))->height;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_raster__get_min(PyObject *self, void *closure)
{
    double ret;

    ret = GV_RASTER(pygobject_get(self))->min;
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gv_raster__get_max(PyObject *self, void *closure)
{
    double ret;

    ret = GV_RASTER(pygobject_get(self))->max;
    return PyFloat_FromDouble(ret);
}

static const PyGetSetDef gv_raster_getsets[] = {
    { "width", (getter)_wrap_gv_raster__get_width, (setter)0 },
    { "height", (getter)_wrap_gv_raster__get_height, (setter)0 },
    { "min", (getter)_wrap_gv_raster__get_min, (setter)0 },
    { "max", (getter)_wrap_gv_raster__get_max, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGvRaster_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.Raster",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvRaster_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gv_raster_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_raster_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvRasterLayer ----------- */

#line 2385 "gv.override"
static int
_wrap_gv_raster_layer_new(PyGObject *self, PyObject *args)
{
    PyObject *py_properties = NULL;
    PyGObject *py_raster;
    GvRaster *raster;
    GvProperties properties = NULL;
    int      mode = GV_RLM_AUTO;

    if (!PyArg_ParseTuple(args, "O!iO!:_gv.RasterLayer.__init__", &PyGvRaster_Type,
                            &py_raster, &mode, &PyList_Type, &py_properties ))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create _gv.RasterLayer object");
        return -1;
    }

    raster = GV_RASTER(py_raster->obj);
    if (!GV_IS_RASTER(raster)) {
        PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
        return -1;
    }

    if( py_properties != NULL )
    {
        int     i;

        for( i = 0; i < PyList_Size(py_properties); i++ )
        {
            char *name, *value;
            PyObject *tuple = PyList_GET_ITEM(py_properties, i);

            if (!PyArg_ParseTuple(tuple, "ss", &name, &value))
            {
                PyErr_SetString(PyExc_ValueError, "properties format");
                return -1;
            }

            gv_properties_set( &properties, name, value );
        }
    }

    gv_raster_layer_read(GV_RASTER_LAYER(self->obj), mode, raster, properties);
    gv_properties_destroy( &properties );

    return 0;
}
#line 2831 "gv.c"


static PyObject *
_wrap_gv_raster_layer_texture_clamp_set(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "s_clamp", "t_clamp", NULL };
    int s_clamp, t_clamp;
    long ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GvRasterLayer.texture_clamp_set", kwlist, &s_clamp, &t_clamp))
        return NULL;
    
    ret = gv_raster_layer_texture_clamp_set(GV_RASTER_LAYER(self->obj), s_clamp, t_clamp);
    
    return PyInt_FromLong(ret);

}

static PyObject *
_wrap_gv_raster_layer_zoom_set(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mag_mode", "min_mode", NULL };
    int mag_mode, min_mode;
    long ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GvRasterLayer.zoom_set", kwlist, &mag_mode, &min_mode))
        return NULL;
    
    ret = gv_raster_layer_zoom_set(GV_RASTER_LAYER(self->obj), mag_mode, min_mode);
    
    return PyInt_FromLong(ret);

}

#line 2805 "gv.override"
static PyObject *
_wrap_gv_raster_layer_zoom_get(PyGObject *self)
{
    int mag, min;

    if( gv_raster_layer_zoom_get(GV_RASTER_LAYER(self->obj), &mag, &min) )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else
    {
        return Py_BuildValue("(ii)", min, mag);
    }
}
#line 2882 "gv.c"


static PyObject *
_wrap_gv_raster_layer_alpha_set(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "alpha_mode", "alpha_check_val", NULL };
    int alpha_mode;
    double alpha_check_val;
    long ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"id:GvRasterLayer.alpha_set", kwlist, &alpha_mode, &alpha_check_val))
        return NULL;
    
    ret = gv_raster_layer_alpha_set(GV_RASTER_LAYER(self->obj), alpha_mode, alpha_check_val);
    
    return PyInt_FromLong(ret);

}

#line 2607 "gv.override"
static PyObject *
_wrap_gv_raster_layer_texture_mode_set(PyGObject *self, PyObject *args)
{
    int texture_mode;
    GvColor color;

    if (!PyArg_ParseTuple(args, "i(ffff):GvRasterLayer.texture_mode_set",
                        &texture_mode, &color[0], &color[1], &color[2], &color[3]))
        return NULL;

    return PyInt_FromLong(gv_raster_layer_texture_mode_set(GV_RASTER_LAYER(self->obj),
                                                            texture_mode, color));
}
#line 2916 "gv.c"


#line 2866 "gv.override"
static PyObject *
_wrap_gv_raster_layer_blend_mode_set(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "blend_mode", "sfactor", "dfactor", NULL };
    int blend_mode, sfactor = 0, dfactor = 0;
    long ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i|ii:GvRasterLayer.blend_mode_set",
                                    kwlist, &blend_mode, &sfactor, &dfactor))
        return NULL;

    ret = gv_raster_layer_blend_mode_set(GV_RASTER_LAYER(self->obj), blend_mode, sfactor, dfactor);

    return PyInt_FromLong(ret);
}
#line 2935 "gv.c"


static PyObject *
_wrap_gv_raster_layer_purge_all_textures(PyGObject *self)
{
    
    gv_raster_layer_purge_all_textures(GV_RASTER_LAYER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_raster_layer_get_mode(PyGObject *self)
{
    int ret;

    
    ret = gv_raster_layer_get_mode(GV_RASTER_LAYER(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 2662 "gv.override"
static PyObject *
_wrap_gv_raster_layer_blend_mode_get(PyGObject *self)
{
    int blend_mode;
    int sfactor;
    int dfactor;

    if (gv_raster_layer_blend_mode_get(GV_RASTER_LAYER(self->obj),
                                        &blend_mode, &sfactor, &dfactor))
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else
    {
        return Py_BuildValue("(iii)", blend_mode, sfactor, dfactor);
    }
}
#line 2978 "gv.c"


#line 2644 "gv.override"
static PyObject *
_wrap_gv_raster_layer_alpha_get(PyGObject *self)
{
    float alpha_val;
    int alpha_mode;

    if (gv_raster_layer_alpha_get(GV_RASTER_LAYER(self->obj), &alpha_mode, &alpha_val))
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else
    {
        return Py_BuildValue("(if)", alpha_mode, alpha_val);
    }
}
#line 2998 "gv.c"


#line 2622 "gv.override"
static PyObject *
_wrap_gv_raster_layer_texture_mode_get(PyGObject *self)
{
    int texture_mode;
    GvColor color;
    PyObject *py_color, *py_retval;

    if (gv_raster_layer_texture_mode_get(GV_RASTER_LAYER(self->obj), &texture_mode, &color))
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else
    {
        py_color = Py_BuildValue( "(ffff)", color[0], color[1], color[2], color[3] );
        py_retval = Py_BuildValue( "(iO)", texture_mode, py_color );
        Py_DECREF( py_color );
        return py_retval;
    }
}
#line 3022 "gv.c"


static PyObject *
_wrap_gv_raster_layer_max_get(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "isource", NULL };
    int isource;
    double ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvRasterLayer.max_get", kwlist, &isource))
        return NULL;
    
    ret = gv_raster_layer_max_get(GV_RASTER_LAYER(self->obj), isource);
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gv_raster_layer_min_get(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "isource", NULL };
    int isource;
    double ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvRasterLayer.min_get", kwlist, &isource))
        return NULL;
    
    ret = gv_raster_layer_min_get(GV_RASTER_LAYER(self->obj), isource);
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gv_raster_layer_get_data(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "isource", NULL };
    int isource;
    GvRaster *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvRasterLayer.get_data", kwlist, &isource))
        return NULL;
    
    ret = gv_raster_layer_get_data(GV_RASTER_LAYER(self->obj), isource);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 2550 "gv.override"
static PyObject *
_wrap_gv_raster_layer_set_source(PyGObject *self, PyObject *args)
{
    PyGObject *py_raster;
    GvRaster *raster = NULL;
    unsigned char *lut = NULL;
    int isource, const_value, ret, lut_len=0, nodata_active=FALSE;
    float min, max;
    PyObject *nodata = NULL;
    float nodata_real=-1e8, nodata_imaginary=0.0;

    if (!PyArg_ParseTuple( args, "iOffi|z#O:GvRasterLayer.set_source",
                           &isource, &py_raster, &min, &max,
                           &const_value, &lut, &lut_len, &nodata ))
        return NULL;

    if( py_raster->obj == NULL || (PyObject *)py_raster == Py_None )
        raster = NULL;
    else if (PyObject_TypeCheck(py_raster, &PyGvRaster_Type))
        raster = GV_RASTER(py_raster->obj);
    else {
        PyErr_SetString(PyExc_TypeError, "2nd argument must be a GvRaster");
        return NULL;
    }

    if( nodata == NULL || nodata == Py_None )
    {
        nodata_real = 0.0;
        nodata_imaginary = 0.0;
        nodata_active = FALSE;
    }
    else if( PyTuple_Check(nodata) )
    {
        if( !PyArg_ParseTuple( nodata, "ff", &nodata_real, &nodata_imaginary) )
            return NULL;

        nodata_active = TRUE;
    }
    else
    {
        if( !PyArg_Parse( nodata, "f", &nodata_real ) )
            return NULL;

        nodata_imaginary = 0.0;
        nodata_active = TRUE;
    }

    ret = gv_raster_layer_set_source(GV_RASTER_LAYER(self->obj),
                                    isource, raster,
                                    min, max, const_value, lut,
                                    nodata_active,
                                    nodata_real, nodata_imaginary);

    return Py_BuildValue("i", ret);
}
#line 3127 "gv.c"


static PyObject *
_wrap_gv_raster_layer_min_set(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "isource", "min", NULL };
    int isource, ret;
    double min;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"id:GvRasterLayer.min_set", kwlist, &isource, &min))
        return NULL;
    
    ret = gv_raster_layer_min_set(GV_RASTER_LAYER(self->obj), isource, min);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_raster_layer_max_set(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "isource", "max", NULL };
    int isource, ret;
    double max;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"id:GvRasterLayer.max_set", kwlist, &isource, &max))
        return NULL;
    
    ret = gv_raster_layer_max_set(GV_RASTER_LAYER(self->obj), isource, max);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_raster_layer_nodata_set(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "isource", "nodata_real", "nodata_imaginary", NULL };
    int isource, ret;
    double nodata_real, nodata_imaginary;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"idd:GvRasterLayer.nodata_set", kwlist, &isource, &nodata_real, &nodata_imaginary))
        return NULL;
    
    ret = gv_raster_layer_nodata_set(GV_RASTER_LAYER(self->obj), isource, nodata_real, nodata_imaginary);
    
    return PyInt_FromLong(ret);
}

#line 2718 "gv.override"
static PyObject *
_wrap_gv_raster_layer_nodata_get(PyGObject *self, PyObject *args)
{
    int isource = 0;
    gint ret;
    double nodata_real, nodata_imaginary;
    GvRasterLayer *rlayer;

    if (!PyArg_ParseTuple( args, "|i:GvRasterLayer.nodata_get", &isource ))
        return NULL;

    rlayer = GV_RASTER_LAYER(self->obj);

    ret = gv_raster_layer_nodata_get( rlayer, isource,
                                      &nodata_real, &nodata_imaginary );
    if( ret )
    {
        if( GDALDataTypeIsComplex(gv_raster_layer_type_get(rlayer, isource)) )
            return Py_BuildValue( "(ff)", nodata_real, nodata_imaginary );
        else
            return Py_BuildValue( "f", nodata_real );
    }
    else
    {
        Py_INCREF( Py_None );
        return Py_None;
    }
}
#line 3204 "gv.c"


#line 2840 "gv.override"
static PyObject *
_wrap_gv_raster_layer_type_get(PyGObject *self, PyObject *args)
{
    int isource;

    if (!PyArg_ParseTuple(args, "i:GvRasterLayer.type_get", &isource))
        return NULL;

    return PyInt_FromLong((long)gv_raster_layer_type_get
              (GV_RASTER_LAYER(self->obj), isource));
}
#line 3219 "gv.c"


#line 2529 "gv.override"
static PyObject *
_wrap_gv_raster_layer_pixel_to_view(PyGObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd:GvRasterLayer.pixel_to_view", &x, &y))
        return NULL;

    if( gv_raster_layer_pixel_to_view(GV_RASTER_LAYER(self->obj), &x, &y, NULL) )
    {
        return Py_BuildValue("(ff)", x, y);
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "pixel_to_view transformation failed." );
        return NULL;
    }
}
#line 3242 "gv.c"


#line 2508 "gv.override"
static PyObject *
_wrap_gv_raster_layer_view_to_pixel(PyGObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd:GvRasterLayer.view_to_pixel", &x, &y))
        return NULL;

    if( gv_raster_layer_view_to_pixel(GV_RASTER_LAYER(self->obj), &x, &y, NULL) )
    {
        return Py_BuildValue("(ff)", x, y);
    }
    else
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "view_to_pixel transformation failed." );
        return NULL;
    }
}
#line 3265 "gv.c"


#line 2438 "gv.override"
static PyObject *
_wrap_gv_raster_layer_autoscale_view(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"alg", "alg_param", "isource", NULL};
    int alg = GvASAAutomatic, isrc = 0, success;
    double alg_param = -1.0;
    double   out_min, out_max;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|idi:GvRasterLayer.autoscale_view",
                                        kwlist, &alg, &alg_param, &isrc))
        return NULL;

    success = gv_raster_layer_autoscale_view(GV_RASTER_LAYER(self->obj), isrc,
                                            alg, alg_param, &out_min, &out_max );
    if( !success )
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "GvRasterLayer.autoscale_view() failed, failed to get samples?" );
        return NULL;
    }

    return Py_BuildValue("(dd)", out_min, out_max);
}
#line 3292 "gv.c"


#line 2470 "gv.override"
static PyObject *
_wrap_gv_raster_layer_histogram_view(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"alg", "alg_param", "assign", NULL};
    PyObject *py_list;
    double   scale_min = 0.0, scale_max = 255.0;
    int      hist_size = 256, isrc = 0, hist_count, i;
    int      *histogram = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|iddi:GvRasterLayer.histogram_view",
                                    kwlist, &isrc, &scale_min, &scale_max, &hist_size))
        return NULL;

    histogram = g_new(int, hist_size);

    hist_count = gv_raster_layer_histogram_view(GV_RASTER_LAYER(self->obj), isrc,
                                                scale_min, scale_max, TRUE,
                                                hist_size, histogram );
    if( hist_count == 0 )
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "GvRasterLayer.histogram_view() failed, failed to get samples?");
        return NULL;
    }

    py_list = PyList_New(0);
    for( i = 0; i < hist_size; i++ )
    {
        PyObject *py_value = Py_BuildValue("i", histogram[i]);
        PyList_Append(py_list, py_value);
        Py_DECREF( py_value );
    }
    g_free( histogram );

    return py_list;
}
#line 3332 "gv.c"


static PyObject *
_wrap_gv_raster_layer_pixel_size(PyGObject *self)
{
    double ret;

    
    ret = gv_raster_layer_pixel_size(GV_RASTER_LAYER(self->obj));
    
    return PyFloat_FromDouble(ret);
}

#line 2925 "gv.override"
static PyObject *
_wrap_gv_raster_layer_add_height(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "height_raster", "default_height", NULL };
    PyGObject *height_raster;
    double default_height = 0.0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!|d:GvRasterLayer.add_height",
                                    kwlist, &PyGvRaster_Type, &height_raster, &default_height))
        return NULL;

    gv_raster_layer_add_height(GV_RASTER_LAYER(self->obj), GV_RASTER(height_raster->obj), default_height);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 3363 "gv.c"


#line 2943 "gv.override"
static PyObject *
_wrap_gv_raster_layer_clamp_height(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bclamp_min", "bclamp_max", "min_height", "max_height", NULL };
    int bclamp_min = 0, bclamp_max = 0;
    double min_height = -30000.0, max_height = 30000.0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|iidd:GvRasterLayer.clamp_height",
                        kwlist, &bclamp_min, &bclamp_max, &min_height, &max_height))
        return NULL;

    gv_raster_layer_clamp_height(GV_RASTER_LAYER(self->obj), bclamp_min, bclamp_max, min_height, max_height);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 3383 "gv.c"


static PyObject *
_wrap_gv_raster_layer_set_raw(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "raw_enable", NULL };
    int raw_enable, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvRasterLayer.set_raw", kwlist, &raw_enable))
        return NULL;
    
    ret = gv_raster_layer_set_raw(GV_RASTER_LAYER(self->obj), raw_enable);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_raster_layer_refresh_mesh(PyGObject *self)
{
    
    gv_raster_layer_refresh_mesh(GV_RASTER_LAYER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_raster_layer_purge_texture(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "texture", NULL };
    int texture;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvRasterLayer.purge_texture", kwlist, &texture))
        return NULL;
    
    gv_raster_layer_purge_texture(GV_RASTER_LAYER(self->obj), texture);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_raster_layer_touch_texture(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "texture", NULL };
    int texture;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvRasterLayer.touch_texture", kwlist, &texture))
        return NULL;
    
    gv_raster_layer_touch_texture(GV_RASTER_LAYER(self->obj), texture);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_raster_layer_reset_texture(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "texture", "lod", "size", NULL };
    int texture, lod, size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iii:GvRasterLayer.reset_texture", kwlist, &texture, &lod, &size))
        return NULL;
    
    gv_raster_layer_reset_texture(GV_RASTER_LAYER(self->obj), texture, lod, size);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 2909 "gv.override"
static PyObject *
_wrap_gv_raster_layer_lut_color_wheel_new_ev(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "set_phase", "set_magnitude", NULL };
    int set_phase = 1, set_magnitude = 1, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ii:GvRasterLayer.lut_color_wheel_new_ev",
                                    kwlist, &set_phase, &set_magnitude))
        return NULL;

    ret = gv_raster_layer_lut_color_wheel_new_ev(GV_RASTER_LAYER(self->obj), set_phase, set_magnitude);

    return PyInt_FromLong(ret);
}
#line 3470 "gv.c"


static PyObject *
_wrap_gv_raster_layer_lut_color_wheel_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "h_type", "h_param", "s_type", "s_param", "v_type", "v_param", NULL };
    int h_type, s_type, v_type, ret;
    double h_param, s_param, v_param;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ididid:GvRasterLayer.lut_color_wheel_new", kwlist, &h_type, &h_param, &s_type, &s_param, &v_type, &v_param))
        return NULL;
    
    ret = gv_raster_layer_lut_color_wheel_new(GV_RASTER_LAYER(self->obj), h_type, h_param, s_type, s_param, v_type, v_param);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_raster_layer_lut_color_wheel_1d_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "s", "v", "offset", NULL };
    int ret;
    double s, v, offset;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddd:GvRasterLayer.lut_color_wheel_1d_new", kwlist, &s, &v, &offset))
        return NULL;
    
    ret = gv_raster_layer_lut_color_wheel_1d_new(GV_RASTER_LAYER(self->obj), s, v, offset);
    
    return PyInt_FromLong(ret);
}

#line 2883 "gv.override"
static PyObject *
_wrap_gv_raster_layer_lut_put(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "lut", NULL };
    unsigned char *lut;
    int  lut_len, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|z#:GvRasterLayer.lut_put",
                                    kwlist, &lut, &lut_len))
        return NULL;

    if( lut != NULL && lut_len != 1024 && lut_len != 1024 * 256 )
    {
        PyErr_SetString(PyExc_TypeError,
            "lut string must be 256x1x4 or 256x256x4 in GvRasterLayer.lut_put");
        return NULL;
    }

    height = lut_len / 1024;
    gv_raster_layer_lut_put(GV_RASTER_LAYER(self->obj), lut, height);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 3528 "gv.c"


#line 2682 "gv.override"
static PyObject *
_wrap_gv_raster_layer_lut_get(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"rgba_complex", NULL};
    PyObject *py_lut, *py_retval;
    char *lut;
    int width, height;
    int rgba_complex = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i:GvRasterLayer.lut_get",
                                        kwlist, &rgba_complex))
        return NULL;

    lut = gv_raster_layer_lut_get(GV_RASTER_LAYER(self->obj),
                                    &width, &height, rgba_complex);
    if (lut == NULL)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else
    {
        py_lut = PyString_FromStringAndSize(lut, width * height * 4);
        if ( py_lut == NULL )
        {
            Py_INCREF(Py_None);
            return Py_None;
        }

        py_retval = Py_BuildValue("(Oii)", py_lut, width, height);
        Py_DECREF(py_lut);
        return py_retval;
    }
}
#line 3566 "gv.c"


static PyObject *
_wrap_gv_raster_layer_lut_compose(PyGObject *self)
{
    int ret;

    
    ret = gv_raster_layer_lut_compose(GV_RASTER_LAYER(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_raster_layer_lut_type_get(PyGObject *self)
{
    long ret;

    
    ret = gv_raster_layer_lut_type_get(GV_RASTER_LAYER(self->obj));
    
    return PyInt_FromLong(ret);

}

#line 2748 "gv.override"
static PyObject *
_wrap_gv_raster_layer_get_nodata(PyGObject *self, PyObject *args)
{
    int isource = 0;
    GvRasterLayer *rlayer;

    if (!PyArg_ParseTuple(args, "|i:GvRasterLayer.get_nodata", &isource))
        return NULL;

    rlayer = GV_RASTER_LAYER(self->obj);

    if( rlayer != NULL && isource >= 0 && isource < rlayer->source_count )
    {
        GvRasterSource *source = rlayer->source_list + isource;

        if( source->nodata_active && source->data != NULL
            && source->data->gdal_type == GDT_CFloat32 )
            return Py_BuildValue( "(ff)", source->nodata_real,
                                  source->nodata_imaginary );
        else
            return Py_BuildValue( "f", source->nodata_real );
    }
    else
    {
        Py_INCREF( Py_None );
        return Py_None;
    }
}
#line 3621 "gv.c"


#line 2463 "gv.override"
static PyObject *
_wrap_gv_raster_layer_get_mesh_lod(PyGObject *self)
{
    return Py_BuildValue("i", GV_RASTER_LAYER(self->obj)->mesh->detail);
}
#line 3630 "gv.c"


#line 2961 "gv.override"
static PyObject *
_wrap_gv_raster_layer_build_skirt(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "base_height", NULL };
    GvRasterLayer *rlayer;
    double base_z = 0.0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|d:GvRasterLayer.build_skirt",
                                    kwlist, &base_z))
        return NULL;

    rlayer = GV_RASTER_LAYER(self->obj);

    return pygobject_new((GObject *)gv_build_skirt(rlayer, base_z));
}
#line 3649 "gv.c"


#line 2822 "gv.override"
static PyObject *
_wrap_gv_raster_layer_get_height(PyGObject *self, PyObject *args)
{
    double x, y, result;
    int success;

    if (!PyArg_ParseTuple(args, "dd:GvRasterLayer.get_height", &x, &y))
        return NULL;

    result = gv_mesh_get_height(GV_RASTER_LAYER(self->obj)->mesh, x, y, &success);
    if (success)
        return Py_BuildValue("f", result);

    PyErr_SetString(PyExc_RuntimeError, "gv_mesh_get_height failed.");
    return NULL;
}
#line 3669 "gv.c"


#line 2853 "gv.override"
static PyObject *
_wrap_gv_raster_layer_get_const_value(PyGObject *self, PyObject *args)
{
    int isource = 0;

    if (!PyArg_ParseTuple(args, "|i:GvRasterLayer.get_const_value", &isource))
        return NULL;

    return PyInt_FromLong(gv_raster_layer_get_const_value
                            (GV_RASTER_LAYER(self->obj), isource));
}
#line 3684 "gv.c"


#line 2778 "gv.override"
static PyObject *
_wrap_gv_raster_layer_get_source_lut(PyGObject *self, PyObject *args)
{
    PyObject *py_lut;
    unsigned char *lut;
    int isource = 0;

    if (!PyArg_ParseTuple(args, "|i:GvRasterLayer.get_source_lut", &isource))
        return NULL;

    lut = gv_raster_layer_source_get_lut(GV_RASTER_LAYER(self->obj), isource);
    if( lut == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if ( ( py_lut = PyString_FromStringAndSize( (char*)lut, 256 ) ) == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return py_lut;
}
#line 3713 "gv.c"


static const PyMethodDef _PyGvRasterLayer_methods[] = {
    { "texture_clamp_set", (PyCFunction)_wrap_gv_raster_layer_texture_clamp_set, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "zoom_set", (PyCFunction)_wrap_gv_raster_layer_zoom_set, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "zoom_get", (PyCFunction)_wrap_gv_raster_layer_zoom_get, METH_NOARGS,
      NULL },
    { "alpha_set", (PyCFunction)_wrap_gv_raster_layer_alpha_set, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "texture_mode_set", (PyCFunction)_wrap_gv_raster_layer_texture_mode_set, METH_VARARGS,
      NULL },
    { "blend_mode_set", (PyCFunction)_wrap_gv_raster_layer_blend_mode_set, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "purge_all_textures", (PyCFunction)_wrap_gv_raster_layer_purge_all_textures, METH_NOARGS,
      NULL },
    { "get_mode", (PyCFunction)_wrap_gv_raster_layer_get_mode, METH_NOARGS,
      NULL },
    { "blend_mode_get", (PyCFunction)_wrap_gv_raster_layer_blend_mode_get, METH_NOARGS,
      NULL },
    { "alpha_get", (PyCFunction)_wrap_gv_raster_layer_alpha_get, METH_NOARGS,
      NULL },
    { "texture_mode_get", (PyCFunction)_wrap_gv_raster_layer_texture_mode_get, METH_NOARGS,
      NULL },
    { "max_get", (PyCFunction)_wrap_gv_raster_layer_max_get, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "min_get", (PyCFunction)_wrap_gv_raster_layer_min_get, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_data", (PyCFunction)_wrap_gv_raster_layer_get_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_source", (PyCFunction)_wrap_gv_raster_layer_set_source, METH_VARARGS,
      NULL },
    { "min_set", (PyCFunction)_wrap_gv_raster_layer_min_set, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "max_set", (PyCFunction)_wrap_gv_raster_layer_max_set, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "nodata_set", (PyCFunction)_wrap_gv_raster_layer_nodata_set, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "nodata_get", (PyCFunction)_wrap_gv_raster_layer_nodata_get, METH_VARARGS,
      NULL },
    { "type_get", (PyCFunction)_wrap_gv_raster_layer_type_get, METH_VARARGS,
      NULL },
    { "pixel_to_view", (PyCFunction)_wrap_gv_raster_layer_pixel_to_view, METH_VARARGS,
      NULL },
    { "view_to_pixel", (PyCFunction)_wrap_gv_raster_layer_view_to_pixel, METH_VARARGS,
      NULL },
    { "autoscale_view", (PyCFunction)_wrap_gv_raster_layer_autoscale_view, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "histogram_view", (PyCFunction)_wrap_gv_raster_layer_histogram_view, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixel_size", (PyCFunction)_wrap_gv_raster_layer_pixel_size, METH_NOARGS,
      NULL },
    { "add_height", (PyCFunction)_wrap_gv_raster_layer_add_height, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clamp_height", (PyCFunction)_wrap_gv_raster_layer_clamp_height, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_raw", (PyCFunction)_wrap_gv_raster_layer_set_raw, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "refresh_mesh", (PyCFunction)_wrap_gv_raster_layer_refresh_mesh, METH_NOARGS,
      NULL },
    { "purge_texture", (PyCFunction)_wrap_gv_raster_layer_purge_texture, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "touch_texture", (PyCFunction)_wrap_gv_raster_layer_touch_texture, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "reset_texture", (PyCFunction)_wrap_gv_raster_layer_reset_texture, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lut_color_wheel_new_ev", (PyCFunction)_wrap_gv_raster_layer_lut_color_wheel_new_ev, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lut_color_wheel_new", (PyCFunction)_wrap_gv_raster_layer_lut_color_wheel_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lut_color_wheel_1d_new", (PyCFunction)_wrap_gv_raster_layer_lut_color_wheel_1d_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lut_put", (PyCFunction)_wrap_gv_raster_layer_lut_put, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lut_get", (PyCFunction)_wrap_gv_raster_layer_lut_get, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lut_compose", (PyCFunction)_wrap_gv_raster_layer_lut_compose, METH_NOARGS,
      NULL },
    { "lut_type_get", (PyCFunction)_wrap_gv_raster_layer_lut_type_get, METH_NOARGS,
      NULL },
    { "get_nodata", (PyCFunction)_wrap_gv_raster_layer_get_nodata, METH_VARARGS,
      NULL },
    { "get_mesh_lod", (PyCFunction)_wrap_gv_raster_layer_get_mesh_lod, METH_NOARGS,
      NULL },
    { "build_skirt", (PyCFunction)_wrap_gv_raster_layer_build_skirt, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_height", (PyCFunction)_wrap_gv_raster_layer_get_height, METH_NOARGS,
      NULL },
    { "get_const_value", (PyCFunction)_wrap_gv_raster_layer_get_const_value, METH_VARARGS,
      NULL },
    { "get_source_lut", (PyCFunction)_wrap_gv_raster_layer_get_source_lut, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvRasterLayer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.RasterLayer",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvRasterLayer_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_raster_layer_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvShapes ----------- */

#line 1211 "gv.override"
static int
_wrap_gv_shapes_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    PyGObject *py_shapes = NULL;
    static char* kwlist[] = { "shapefilename", "ogrlayer", "_obj", NULL };
    char *filename = NULL;
    GvData    *data = NULL;
    char      *ogrlayer_in = NULL;
    void      *hLayer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|zzO!:_gv.Shapes.__init__",
                                     kwlist, &filename, &ogrlayer_in,
                                     &PyGvShapes_Type, &py_shapes
                                     ))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create _gv.Shapes object");
        return -1;
    }

    if (filename != NULL) {
        gv_shapes_read_from_file(filename, GV_SHAPES (self->obj));
    }
    else if( ogrlayer_in != NULL ) {
        // we need to unref here otherwise it will remain a floating GvShapes... 
        g_object_unref(self->obj);
        hLayer = SWIG_SimpleGetPtr(ogrlayer_in, "OGRLayerH" );
        if( hLayer == NULL )
        {
            PyErr_SetString(PyExc_IOError,
                            "Unable to extract OGRLayerH handle in _gv.Shapes.__init__");
            return -1;
        }

        data = gv_shapes_from_ogr_layer( hLayer );
        if( data == NULL )
        {
            return -1;
        }
        self->obj = G_OBJECT (data);
    }
    else if( py_shapes != NULL )
    {
        // we need to unref here otherwise it will remain a floating GvShapes... 
        g_object_unref(self->obj);
        self->obj = py_shapes->obj;
    }

    return 0;
}
#line 3912 "gv.c"


#line 1594 "gv.override"
static PyObject *
_wrap_gv_shapes_add_height(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "raster", "offset", "default_height", NULL };
    PyGObject *raster;
    double offset = 0.0, default_height = 0.0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|dd:GvShapes.add_height",
                                    kwlist, &PyGvData_Type, &raster, &offset, &default_height))
        return NULL;

    gv_shapes_add_height(GV_SHAPES(self->obj), GV_DATA(raster->obj), offset, default_height);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 3932 "gv.c"


#line 1553 "gv.override"
static PyObject *
_wrap_gv_shapes_get_extents(PyGObject *self)
{
    GvRect rect;

    gv_shapes_get_extents(GV_SHAPES(self->obj), &rect);

    return Py_BuildValue("(" CCCC ")", rect.x, rect.y, rect.width, rect.height);
}
#line 3945 "gv.c"


#line 1498 "gv.override"
static PyObject *
_wrap_gv_shapes_replace_shapes(PyGObject *self, PyObject *args)
{
    int       shape_count, i;
    PyObject *pyindex_list = NULL, *pyshape_list = NULL;
    GvShape **shape_list = NULL;
    int      *shape_ids = NULL;
    int      copy_flag = FALSE;

    if (!PyArg_ParseTuple(args, "O!O!|d:GvShapes.replace_shapes",
                          &PyList_Type, &pyindex_list,
                          &PyList_Type, &pyshape_list,
                          &copy_flag ) )
        return NULL;

    if( PyList_Size(pyindex_list) != PyList_Size(pyshape_list) )
    {
        PyErr_SetString(PyExc_RuntimeError,
              "Size of index & shape lists differ in GvShapes.replace_shapes().");
        return NULL;
    }

    shape_count = PyList_Size(pyindex_list);
    shape_ids = g_new(int,shape_count);
    shape_list = g_new(GvShape*,shape_count);

    for( i = 0; i < shape_count; i++ )
    {
        PyGvShape *py_shape = NULL;

        if( !PyArg_Parse( PyList_GET_ITEM(pyindex_list,i), "i", shape_ids+i ))
        {
            PyErr_SetString(PyExc_ValueError,
                           "expecting ints in GvShapes.replace_shapes argument");
            return NULL;
        }

        if( !PyArg_Parse( PyList_GET_ITEM(pyshape_list,i), "O", &py_shape ))
        {
            PyErr_SetString(PyExc_ValueError,
                        "expecting GvShape in GvShapes.replace_shapes argument");
            return NULL;
        }

        shape_list[i] = GV_SHAPE(py_shape);
    }

    gv_shapes_replace_shapes( GV_SHAPES(self->obj), shape_count,
                            shape_ids, shape_list, copy_flag );

    Py_INCREF(Py_None);
    return Py_None;
}
#line 4002 "gv.c"


#line 1466 "gv.override"
static PyObject *
_wrap_gv_shapes_delete_shapes(PyGObject *self, PyObject *args)
{
    int       shape_count, ii;
    PyObject *pylist = NULL;
    int      *shape_ids = NULL;

    if (!PyArg_ParseTuple(args, "O!:GvShapes.delete_shapes", &PyList_Type, &pylist))
        return NULL;

    shape_count = PyList_Size(pylist);
    shape_ids = g_new(int, shape_count);

    for( ii = 0; ii < shape_count; ii++ )
    {
        if( !PyArg_Parse( PyList_GET_ITEM(pylist,ii), "i", shape_ids + ii ) )
        {
            PyErr_SetString(PyExc_ValueError,
                           "expecting ints in GvShapes.delete_shapes argument");
            return NULL;
        }
    }

    gv_shapes_delete_shapes(GV_SHAPES(self->obj), shape_count, shape_ids);

    g_free(shape_ids);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 4036 "gv.c"


#line 2078 "gv.override"
static PyObject *
_wrap_gv_shapes_get_change_info(PyGObject *self, PyObject *args)
{
    GvShapeChangeInfo *change_info;
    PyObject *c_change_info;
    PyObject *id_list;
    int i;

    if (!PyArg_ParseTuple(args, "O:GvShapes.get_change_info", &c_change_info))
        return NULL;

    if (!pygobject_check(c_change_info, &PyGPointer_Type))
        return NULL;

    change_info = pyg_pointer_get(c_change_info, GvShapeChangeInfo);

    id_list = PyTuple_New(change_info->num_shapes);

    for(i=0; i<change_info->num_shapes; i++)
    {
        PyTuple_SetItem(id_list, i, PyInt_FromLong((long)change_info->shape_id[i]));
    }

    return Py_BuildValue( "(iiO)",
                         change_info->change_type,
                         change_info->num_shapes,
                         id_list );
}
#line 4068 "gv.c"


#line 1410 "gv.override"
static PyObject *
_wrap_gv_shapes_get_shape(PyGObject *self, PyObject* args)
{
    int shp_index = -1;
    GvShape* shape = NULL;

    if (!PyArg_ParseTuple(args, "i:GvShapes.get_shape", &shp_index))
        return NULL;

    if (shp_index < 0 || shp_index >= gv_shapes_num_shapes(GV_SHAPES(self->obj))) {
        PyErr_SetString(PyExc_IndexError, "shape index out of range");
        return NULL;
    }

    shape = gv_shapes_get_shape(GV_SHAPES(self->obj), shp_index);
    if (shape)
        return pygv_shape_from_shape(shape);
    else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 4094 "gv.c"


#line 1450 "gv.override"
static PyObject *
_wrap_gv_shapes_append_last(PyGObject *self, PyObject *args)
{
    PyObject *py_shape;
    int       shp_index = -1;

    if (!PyArg_ParseTuple(args, "O!:GvShapes.append_last", &PyGvShape_Type, &py_shape))
        return NULL;

    if (GV_SHAPE(py_shape))
        shp_index = gv_shapes_add_shape_last(GV_SHAPES(self->obj), GV_SHAPE(py_shape));

    return Py_BuildValue("i", shp_index);
}
#line 4112 "gv.c"


#line 1381 "gv.override"
static PyObject *
_wrap_gv_shapes_save_to_dbf(PyGObject *self, PyObject *args)
{
    char *filename;

    if (!PyArg_ParseTuple(args, "s:GvShapes.save_to_dbf", &filename))
        return NULL;

    return PyInt_FromLong(gv_shapes_to_dbf(filename, GV_DATA(self->obj)));
}
#line 4126 "gv.c"


#line 1393 "gv.override"
static PyObject *
_wrap_gv_shapes_save_to(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", "shp_type", NULL };
    char *filename;
    int shp_type = 0, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|i:GvShapes.save_to",
                                    kwlist, &filename, &shp_type))
        return NULL;

    ret = gv_shapes_to_shapefile(filename, GV_DATA(self->obj), shp_type);

    return PyInt_FromLong(ret);
}
#line 4145 "gv.c"


#line 617 "gv.override"
static PyObject *
_wrap_gv_shapes_num_shapes(PyGObject *self)
{
    return PyInt_FromLong(gv_shapes_num_shapes(GV_SHAPES(self->obj)));
}
#line 4154 "gv.c"


#line 1434 "gv.override"
static PyObject *
_wrap_gv_shapes_append(PyGObject *self, PyObject *args)
{
    PyObject *py_shape;
    int       shp_index = -1;

    if (!PyArg_ParseTuple(args, "O!:GvShapes.append", &PyGvShape_Type, &py_shape))
        return NULL;

    if (GV_SHAPE(py_shape))
        shp_index = gv_shapes_add_shape(GV_SHAPES(self->obj), GV_SHAPE(py_shape));

    return Py_BuildValue("i", shp_index);
}
#line 4172 "gv.c"


#line 1341 "gv.override"
static PyObject *
_wrap_gv_shapes_get_fid(PyGObject *self, PyObject *args)
{
    GvProperties *properties = NULL;
    char *field = NULL;
    int field_index, fid = 0;

    if (!PyArg_ParseTuple(args, "s:GvShapes.get_fid", &field))
        return NULL;

    properties = gv_data_get_properties(GV_DATA(self->obj));
    if (properties == NULL) {
        PyErr_SetString(PyExc_ValueError, "no properties in GvShapes.get_fid()");
        return NULL;
    }

    for (field_index = 0; TRUE; field_index++)
    {
        char name[64];
        const char *prop_value;

        sprintf(name, "_field_name_%d", field_index+1);
        prop_value = gv_properties_get(properties, name);
        if (prop_value == NULL)
            break;
        if ( g_strcasecmp(prop_value, field) == 0 ) {
            fid = field_index + 1;
            break;
        }
    }

    if (fid != 0)
        return Py_BuildValue("i", fid);
    else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 4214 "gv.c"


static const PyMethodDef _PyGvShapes_methods[] = {
    { "add_height", (PyCFunction)_wrap_gv_shapes_add_height, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_extents", (PyCFunction)_wrap_gv_shapes_get_extents, METH_NOARGS,
      NULL },
    { "replace_shapes", (PyCFunction)_wrap_gv_shapes_replace_shapes, METH_VARARGS,
      NULL },
    { "delete_shapes", (PyCFunction)_wrap_gv_shapes_delete_shapes, METH_VARARGS,
      NULL },
    { "get_change_info", (PyCFunction)_wrap_gv_shapes_get_change_info, METH_VARARGS,
      NULL },
    { "get_shape", (PyCFunction)_wrap_gv_shapes_get_shape, METH_VARARGS,
      NULL },
    { "append_last", (PyCFunction)_wrap_gv_shapes_append_last, METH_VARARGS,
      NULL },
    { "save_to_dbf", (PyCFunction)_wrap_gv_shapes_save_to_dbf, METH_VARARGS,
      NULL },
    { "save_to", (PyCFunction)_wrap_gv_shapes_save_to, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "num_shapes", (PyCFunction)_wrap_gv_shapes_num_shapes, METH_NOARGS,
      NULL },
    { "append", (PyCFunction)_wrap_gv_shapes_append, METH_VARARGS,
      NULL },
    { "get_fid", (PyCFunction)_wrap_gv_shapes_get_fid, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 1267 "gv.override"
static int
_wrap_gv_shapes_sq_length(PyGObject *self)
{
    return gv_shapes_num_shapes(GV_SHAPES(self->obj));
}

static PyObject *
_wrap_gv_shapes_sq_item(PyGObject *self, int shp_index)
{
    GvShape *shape;

    if (shp_index < 0 || shp_index >= gv_shapes_num_shapes(GV_SHAPES(self->obj))) {
        PyErr_SetString(PyExc_IndexError, "shape index out of range");
        return NULL;
    }

    shape = gv_shapes_get_shape(GV_SHAPES(self->obj), shp_index);

    if (shape)
        return pygv_shape_from_shape(shape);
    else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static int
_wrap_gv_shapes_sq_ass_item(PyGObject *self, int shp_index, PyObject *shape)
{
    GvShape **shape_list = NULL;
    int      *shape_id = NULL;

    if (shp_index < 0 || shp_index >= gv_shapes_num_shapes(GV_SHAPES(self->obj))) {
        PyErr_SetString(PyExc_IndexError, "shape index out of range");
        return -1;
    }

    if (shape == NULL) {
        shape_id = g_new(int, 1);
        shape_id[0] = shp_index;
        gv_shapes_delete_shapes(GV_SHAPES(self->obj), 1, shape_id);
        g_free(shape_id);
        return 0;
    }

    if (! PyObject_TypeCheck(shape, &PyGvShape_Type)) {
        PyErr_SetString(PyExc_TypeError, "arg 2 not a GvShape object");
        return -1;
    }

    shape_id = g_new(int, 1);
    shape_list = g_new(GvShape*, 1);
    shape_list[0] = GV_SHAPE(shape);

    gv_shapes_replace_shapes(GV_SHAPES(self->obj), 1, shape_id, shape_list, FALSE);

    g_free(shape_id);
    g_free(shape_list);

    return 0;
}

static PySequenceMethods _wrap_gv_shapes_tp_as_sequence = {
    (lenfunc)_wrap_gv_shapes_sq_length,
    0,
    0,
    (ssizeargfunc)_wrap_gv_shapes_sq_item,
    0,
    (ssizeobjargproc)_wrap_gv_shapes_sq_ass_item,
    0,
    0,
};
#line 4318 "gv.c"


PyTypeObject G_GNUC_INTERNAL PyGvShapes_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.Shapes",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)&_wrap_gv_shapes_tp_as_sequence, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvShapes_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_shapes_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvSymbolManager ----------- */

static int
_wrap_gv_symbol_manager_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.SymbolManager.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.SymbolManager object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gv_symbol_manager_has_symbol(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvSymbolManager.has_symbol", kwlist, &name))
        return NULL;
    
    ret = gv_symbol_manager_has_symbol(GV_SYMBOL_MANAGER(self->obj), name);
    
    return PyInt_FromLong(ret);
}

#line 695 "gv.override"
static PyObject *
_wrap_gv_symbol_manager_get_symbol(PyGObject *self, PyObject *args)
{
    GvSymbolManager *manager;
    char     *symbol_name = NULL;
    GvSymbolObj *symbol;

    if (!PyArg_ParseTuple(args,"s:GvSymbolManager.get_symbol", &symbol_name))
        return NULL;

    manager = GV_SYMBOL_MANAGER(self->obj);
    symbol = gv_symbol_manager_get_symbol(manager, symbol_name);

    if (symbol == NULL)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if (symbol->type == GV_SYMBOL_VECTOR)
    {
        PyObject *py_shape = pygv_shape_from_shape((GvShape*)symbol->buffer);
        return Py_BuildValue("(iO)", symbol->type, py_shape);
    }
    else
    {
        PyObject *py_rgba_buffer;
        PyObject *py_result;

        py_rgba_buffer =
            PyString_FromStringAndSize( symbol->buffer,
                                        symbol->width * symbol->height * 4 );

        py_result = Py_BuildValue("(iiiO)",
                                  symbol->type, symbol->width, symbol->height,
                                  py_rgba_buffer );
        Py_DECREF( py_rgba_buffer );

        return py_result;
    }
}
#line 4447 "gv.c"


#line 666 "gv.override"
static PyObject *
_wrap_gv_symbol_manager_inject_raster_symbol(PyGObject *self, PyObject *args)
{
    GvSymbolManager *manager;
    char     *rgba_string = NULL;
    char     *symbol_name = NULL;
    int      width, height, rgba_len;

    if (!PyArg_ParseTuple(args, "siiz#:GvSymbolManager.inject_raster_symbol",
                          &symbol_name, &width, &height, &rgba_string, &rgba_len ))
        return NULL;

    manager = GV_SYMBOL_MANAGER(self->obj);

    if( width*height*4 > rgba_len )
    {
        PyErr_SetString(PyExc_TypeError,
                        "rgba raster symbol buffer seems to be too small (width*height*4)\nin GvSymbolManager.inject_raster_symbol()." );
        return NULL;
    }

    gv_symbol_manager_inject_raster_symbol(manager, symbol_name,
                                            width, height, rgba_string );

    Py_INCREF(Py_None);
    return Py_None;
}
#line 4478 "gv.c"


#line 646 "gv.override"
static PyObject *
_wrap_gv_symbol_manager_inject_vector_symbol(PyGObject *self, PyObject *args)
{
    PyObject *py_shape;
    GvSymbolManager *manager;
    char     *symbol_name = NULL;

    if (!PyArg_ParseTuple(args, "sO!:GvSymbolManager.inject_vector_symbol",
                          &symbol_name, &PyGvShape_Type, &py_shape))
        return NULL;

    manager = GV_SYMBOL_MANAGER(self->obj);

    gv_symbol_manager_inject_vector_symbol(manager, symbol_name, GV_SHAPE(py_shape));

    Py_INCREF(Py_None);
    return Py_None;
}
#line 4500 "gv.c"


static PyObject *
_wrap_gv_symbol_manager_eject_symbol(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "symbol_name", NULL };
    char *symbol_name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvSymbolManager.eject_symbol", kwlist, &symbol_name))
        return NULL;
    
    ret = gv_symbol_manager_eject_symbol(GV_SYMBOL_MANAGER(self->obj), symbol_name);
    
    return PyInt_FromLong(ret);
}

#line 738 "gv.override"
static PyObject *
_wrap_gv_symbol_manager_save_vector_symbol(PyGObject *self, PyObject *args)
{
    char *symbol_name = NULL, *new_name = NULL;

    if (!PyArg_ParseTuple(args, "ss:GvSymbolManager.save_vector_symbol",
                          &symbol_name, &new_name))
        return NULL;

    if( symbol_name && new_name )
    {
        if (gv_symbol_manager_save_vector_symbol(GV_SYMBOL_MANAGER(self->obj),
                                                symbol_name, new_name))
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        else
        {
            PyErr_SetString(PyExc_TypeError,
                "error while saving new symbol in GvSymbolManager.save_vector_symbol()." );
            return NULL;
        }
    }

    return NULL;
}
#line 4546 "gv.c"


#line 624 "gv.override"
static PyObject *
_wrap_gv_symbol_manager_get_names(PyGObject *self)
{
    PyObject *py_name_list;
    GvSymbolManager *manager;
    char **name_list;
    int i, count;

    manager = GV_SYMBOL_MANAGER(self->obj);
    name_list = gv_symbol_manager_get_names(manager);

    count = CSLCount( name_list );
    py_name_list = PyList_New( count );
    for( i = 0; i < count; i++ )
        PyList_SetItem( py_name_list, i, Py_BuildValue( "s", name_list[i] ) );

    g_free( name_list );

    return py_name_list;
}
#line 4570 "gv.c"


#line 767 "gv.override"
static PyObject *
_wrap_gv_symbol_manager_serialize(PyGObject *self)
{
    /* not working yet... */
    GvSymbolManager *manager;
    GvSymbolObj *symbol;
    CPLXMLNode *tree, *root;
    PyObject *py_xml = NULL;
    char **names;
    int i, count;

    if (self->obj == NULL)
        return NULL;

    manager = GV_SYMBOL_MANAGER(self->obj);
    names = gv_symbol_manager_get_names(manager);
    count = CSLCount(names);

    root = CPLCreateXMLNode(NULL, CXT_Element, "GvSymbolManager");
    for (i = 0; i < count; i++)
    {
        symbol = gv_symbol_manager_get_symbol(manager, names[i]);
        if (symbol->type == GV_SYMBOL_VECTOR)
        {
            tree = CPLCreateXMLNode(root, CXT_Element, "GvVectorSymbol");
            CPLCreateXMLNode( CPLCreateXMLNode(tree, CXT_Attribute, "name"),
                            CXT_Text, names[i]);
            CPLAddXMLChild(tree, gv_shape_to_xml_tree( (GvShape*)symbol->buffer ));
            CPLAddXMLChild(root, tree);
        }
    }

    py_xml = XMLTreeToPyList(root);
    CPLDestroyXMLNode(root);

    return py_xml;
}
#line 4611 "gv.c"


static const PyMethodDef _PyGvSymbolManager_methods[] = {
    { "has_symbol", (PyCFunction)_wrap_gv_symbol_manager_has_symbol, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_symbol", (PyCFunction)_wrap_gv_symbol_manager_get_symbol, METH_VARARGS,
      NULL },
    { "inject_raster_symbol", (PyCFunction)_wrap_gv_symbol_manager_inject_raster_symbol, METH_VARARGS,
      NULL },
    { "inject_vector_symbol", (PyCFunction)_wrap_gv_symbol_manager_inject_vector_symbol, METH_VARARGS,
      NULL },
    { "eject_symbol", (PyCFunction)_wrap_gv_symbol_manager_eject_symbol, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "save_vector_symbol", (PyCFunction)_wrap_gv_symbol_manager_save_vector_symbol, METH_VARARGS,
      NULL },
    { "get_names", (PyCFunction)_wrap_gv_symbol_manager_get_names, METH_NOARGS,
      NULL },
    { "serialize", (PyCFunction)_wrap_gv_symbol_manager_serialize, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvSymbolManager_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.SymbolManager",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvSymbolManager_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_symbol_manager_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvTool ----------- */

static PyObject *
_wrap_gv_tool_activate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", NULL };
    PyGObject *view;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvTool.activate", kwlist, &PyGvViewArea_Type, &view))
        return NULL;
    
    gv_tool_activate(GV_TOOL(self->obj), GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_tool_deactivate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", NULL };
    PyGObject *view;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvTool.deactivate", kwlist, &PyGvViewArea_Type, &view))
        return NULL;
    
    gv_tool_deactivate(GV_TOOL(self->obj), GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_tool_get_view(PyGObject *self)
{
    GvViewArea *ret;

    
    ret = gv_tool_get_view(GV_TOOL(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 1191 "gv.override"
static PyObject *
_wrap_gv_tool_set_boundary(PyGObject *self, PyObject *args)
{
    GvRect rect;

    if (!PyArg_ParseTuple(args, "(" CCCC "):gv_tool_set_boundary",
                          &rect.x, &rect.y, &rect.width, &rect.height))
        return NULL;

    if (!gv_tool_set_boundary(GV_TOOL(self->obj), &rect)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "invalid ROI constraining region specified, width or height <= 0.0");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 4744 "gv.c"


static PyObject *
_wrap_gv_tool_set_cursor(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cursor_type", NULL };
    int cursor_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvTool.set_cursor", kwlist, &cursor_type))
        return NULL;
    
    gv_tool_set_cursor(GV_TOOL(self->obj), cursor_type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvTool_methods[] = {
    { "activate", (PyCFunction)_wrap_gv_tool_activate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "deactivate", (PyCFunction)_wrap_gv_tool_deactivate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_view", (PyCFunction)_wrap_gv_tool_get_view, METH_NOARGS,
      NULL },
    { "set_boundary", (PyCFunction)_wrap_gv_tool_set_boundary, METH_VARARGS,
      NULL },
    { "set_cursor", (PyCFunction)_wrap_gv_tool_set_cursor, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gv_tool__get_view(PyObject *self, void *closure)
{
    GvViewArea *ret;

    ret = GV_TOOL(pygobject_get(self))->view;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyGetSetDef gv_tool_getsets[] = {
    { "view", (getter)_wrap_gv_tool__get_view, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGvTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.Tool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gv_tool_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvSelectionTool ----------- */

static int
_wrap_gv_selection_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.SelectionTool.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.SelectionTool object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gv_selection_tool_set_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", NULL };
    PyGObject *layer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvSelectionTool.set_layer", kwlist, &PyGvShapeLayer_Type, &layer))
        return NULL;
    
    gv_selection_tool_set_layer(GV_SELECTION_TOOL(self->obj), GV_SHAPE_LAYER(layer->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvSelectionTool_methods[] = {
    { "set_layer", (PyCFunction)_wrap_gv_selection_tool_set_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gv_selection_tool__get_layer(PyObject *self, void *closure)
{
    GvShapeLayer *ret;

    ret = GV_SELECTION_TOOL(pygobject_get(self))->layer;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyGetSetDef gv_selection_tool_getsets[] = {
    { "layer", (getter)_wrap_gv_selection_tool__get_layer, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGvSelectionTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.SelectionTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvSelectionTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gv_selection_tool_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_selection_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvRotateTool ----------- */

#line 1049 "gv.override"
static int
_wrap_gv_rotate_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { "layer", NULL };
    char *name = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s:_gv.RotateTool.__init__",
                                     kwlist, &name))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError,  "could not create _gv.RotateTool object");
        return -1;
    }

    if (name != NULL)
        gv_rotate_tool_set_named_layer(GV_ROTATE_TOOL(self->obj), name);

    return 0;
}
#line 4967 "gv.c"


static PyObject *
_wrap_gv_rotate_tool_set_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", NULL };
    PyGObject *layer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvRotateTool.set_layer", kwlist, &PyGvShapeLayer_Type, &layer))
        return NULL;
    
    gv_rotate_tool_set_layer(GV_ROTATE_TOOL(self->obj), GV_SHAPE_LAYER(layer->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_rotate_tool_set_named_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvRotateTool.set_named_layer", kwlist, &name))
        return NULL;
    
    gv_rotate_tool_set_named_layer(GV_ROTATE_TOOL(self->obj), name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvRotateTool_methods[] = {
    { "set_layer", (PyCFunction)_wrap_gv_rotate_tool_set_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_named_layer", (PyCFunction)_wrap_gv_rotate_tool_set_named_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvRotateTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.RotateTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvRotateTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_rotate_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvRoiTool ----------- */

#line 1099 "gv.override"
static int
_wrap_gv_roi_tool_new(PyGObject *self, PyObject *args)
{
    GvRect rect;

    if (!PyArg_ParseTuple(args, "(" CCCC "):_gv.RoiTool.__init__",
                          &rect.x, &rect.y, &rect.width, &rect.height))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError,  "could not create _gv.RoiTool object");
        return -1;
    }

    /* not working yet... */
    /* if (!gv_tool_set_boundary(GV_TOOL(self->obj), &rect))
        PyErr_SetString(PyExc_RuntimeError,
                    "invalid ROI constraining region specified, not set"); */

    return 0;
}
#line 5080 "gv.c"


#line 1125 "gv.override"
static PyObject *
_wrap_gv_roi_tool_get_rect(PyGObject *self)
{
    GvRect rect;

    if (!gv_roi_tool_get_rect(GV_ROI_TOOL(self->obj), &rect)) {
        PyErr_SetString(PyExc_RuntimeError, "no ROI marked");
        return NULL;
    }

    return Py_BuildValue("(" CCCC ")", rect.x, rect.y, rect.width, rect.height);
}
#line 5096 "gv.c"


#line 1139 "gv.override"
static PyObject *
_wrap_gv_roi_tool_new_rect(PyGObject *self, PyObject *args)
{
    GvRect rect;

    if (!PyArg_ParseTuple(args, "(" CCCC "):GvRoiTool.new_rect",
                            &rect.x, &rect.y, &rect.width, &rect.height))
        return NULL;

    if (!gv_roi_tool_new_rect(GV_ROI_TOOL(self->obj), &rect)){
        PyErr_SetString(PyExc_RuntimeError, "invalid ROI specified");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 5117 "gv.c"


static const PyMethodDef _PyGvRoiTool_methods[] = {
    { "get_rect", (PyCFunction)_wrap_gv_roi_tool_get_rect, METH_NOARGS,
      NULL },
    { "new_rect", (PyCFunction)_wrap_gv_roi_tool_new_rect, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvRoiTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.RoiTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvRoiTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_roi_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvRectTool ----------- */

#line 1024 "gv.override"
static int
_wrap_gv_rect_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { "layer", NULL };
    char *name = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s:_gv.RectTool.__init__",
                                     kwlist, &name))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError,  "could not create _gv.RectTool object");
        return -1;
    }

    if (name != NULL)
        gv_rect_tool_set_named_layer(GV_RECT_TOOL(self->obj), name);

    return 0;
}
#line 5199 "gv.c"


static PyObject *
_wrap_gv_rect_tool_set_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", NULL };
    PyGObject *layer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvRectTool.set_layer", kwlist, &PyGvShapeLayer_Type, &layer))
        return NULL;
    
    gv_rect_tool_set_layer(GV_RECT_TOOL(self->obj), GV_SHAPE_LAYER(layer->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_rect_tool_set_named_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvRectTool.set_named_layer", kwlist, &name))
        return NULL;
    
    gv_rect_tool_set_named_layer(GV_RECT_TOOL(self->obj), name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvRectTool_methods[] = {
    { "set_layer", (PyCFunction)_wrap_gv_rect_tool_set_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_named_layer", (PyCFunction)_wrap_gv_rect_tool_set_named_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvRectTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.RectTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvRectTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_rect_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvPointTool ----------- */

#line 974 "gv.override"
static int
_wrap_gv_point_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { "layer", NULL };
    char *name = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s:_gv.PointTool.__init__",
                                     kwlist, &name))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError,  "could not create _gv.PointTool object");
        return -1;
    }

    if (name != NULL)
        gv_point_tool_set_named_layer(GV_POINT_TOOL(self->obj), name);

    return 0;
}
#line 5311 "gv.c"


static PyObject *
_wrap_gv_point_tool_set_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", NULL };
    PyGObject *layer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvPointTool.set_layer", kwlist, &PyGvShapeLayer_Type, &layer))
        return NULL;
    
    gv_point_tool_set_layer(GV_POINT_TOOL(self->obj), GV_SHAPE_LAYER(layer->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_point_tool_set_named_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvPointTool.set_named_layer", kwlist, &name))
        return NULL;
    
    gv_point_tool_set_named_layer(GV_POINT_TOOL(self->obj), name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvPointTool_methods[] = {
    { "set_layer", (PyCFunction)_wrap_gv_point_tool_set_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_named_layer", (PyCFunction)_wrap_gv_point_tool_set_named_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvPointTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.PointTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvPointTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_point_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvPoiTool ----------- */

static int
_wrap_gv_poi_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.PoiTool.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.PoiTool object");
        return -1;
    }
    return 0;
}

#line 1158 "gv.override"
static PyObject *
_wrap_gv_poi_tool_get_point(PyGObject *self)
{
    GvVertex point;

    if (!gv_poi_tool_get_point(GV_POI_TOOL(self->obj), &point)) {
        PyErr_SetString(PyExc_RuntimeError, "no POI marked");
        return NULL;
    }

    return Py_BuildValue("(" CC ")", point.x, point.y);
}
#line 5434 "gv.c"


#line 1172 "gv.override"
static PyObject *
_wrap_gv_poi_tool_new_point(PyGObject *self, PyObject *args)
{
    GvVertex point;

    if (!PyArg_ParseTuple(args, "(" CC "):GvPoiTool.new_point",
                            &point.x, &point.y))
        return NULL;

    if (!gv_poi_tool_new_point(GV_POI_TOOL(self->obj), &point)) {
        PyErr_SetString(PyExc_RuntimeError, "invalid POI specified");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 5455 "gv.c"


static const PyMethodDef _PyGvPoiTool_methods[] = {
    { "get_point", (PyCFunction)_wrap_gv_poi_tool_get_point, METH_NOARGS,
      NULL },
    { "new_point", (PyCFunction)_wrap_gv_poi_tool_new_point, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvPoiTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.PoiTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvPoiTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_poi_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvNodeTool ----------- */

static int
_wrap_gv_node_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.NodeTool.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.NodeTool object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gv_node_tool_set_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", NULL };
    PyGObject *layer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvNodeTool.set_layer", kwlist, &PyGvShapeLayer_Type, &layer))
        return NULL;
    
    gv_node_tool_set_layer(GV_NODE_TOOL(self->obj), GV_SHAPE_LAYER(layer->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvNodeTool_methods[] = {
    { "set_layer", (PyCFunction)_wrap_gv_node_tool_set_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvNodeTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.NodeTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvNodeTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_node_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvLineTool ----------- */

#line 999 "gv.override"
static int
_wrap_gv_line_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { "layer", NULL };
    char *name = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s:_gv.LineTool.__init__",
                                     kwlist, &name))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError,  "could not create _gv.LineTool object");
        return -1;
    }

    if (name != NULL)
        gv_line_tool_set_named_layer(GV_LINE_TOOL(self->obj), name);

    return 0;
}
#line 5627 "gv.c"


static PyObject *
_wrap_gv_line_tool_set_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", NULL };
    PyGObject *layer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvLineTool.set_layer", kwlist, &PyGvShapeLayer_Type, &layer))
        return NULL;
    
    gv_line_tool_set_layer(GV_LINE_TOOL(self->obj), GV_SHAPE_LAYER(layer->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_line_tool_set_named_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvLineTool.set_named_layer", kwlist, &name))
        return NULL;
    
    gv_line_tool_set_named_layer(GV_LINE_TOOL(self->obj), name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvLineTool_methods[] = {
    { "set_layer", (PyCFunction)_wrap_gv_line_tool_set_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_named_layer", (PyCFunction)_wrap_gv_line_tool_set_named_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvLineTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.LineTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvLineTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_line_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvAreaTool ----------- */

#line 1074 "gv.override"
static int
_wrap_gv_area_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { "layer", NULL };
    char *name = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s:_gv.AreaTool.__init__",
                                     kwlist, &name))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError,  "could not create _gv.AreaTool object");
        return -1;
    }

    if (name != NULL)
        gv_area_tool_set_named_layer(GV_AREA_TOOL(self->obj), name);

    return 0;
}
#line 5739 "gv.c"


static PyObject *
_wrap_gv_area_tool_set_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", NULL };
    PyGObject *layer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvAreaTool.set_layer", kwlist, &PyGvShapeLayer_Type, &layer))
        return NULL;
    
    gv_area_tool_set_layer(GV_AREA_TOOL(self->obj), GV_SHAPE_LAYER(layer->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_area_tool_set_named_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvAreaTool.set_named_layer", kwlist, &name))
        return NULL;
    
    gv_area_tool_set_named_layer(GV_AREA_TOOL(self->obj), name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvAreaTool_methods[] = {
    { "set_layer", (PyCFunction)_wrap_gv_area_tool_set_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_named_layer", (PyCFunction)_wrap_gv_area_tool_set_named_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gv_area_tool__get_layer(PyObject *self, void *closure)
{
    GvShapeLayer *ret;

    ret = GV_AREA_TOOL(pygobject_get(self))->layer;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_area_tool__get_named_layer(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GV_AREA_TOOL(pygobject_get(self))->named_layer;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyGetSetDef gv_area_tool_getsets[] = {
    { "layer", (getter)_wrap_gv_area_tool__get_layer, (setter)0 },
    { "named_layer", (getter)_wrap_gv_area_tool__get_named_layer, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGvAreaTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.AreaTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvAreaTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gv_area_tool_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_area_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvToolbox ----------- */

static int
_wrap_gv_toolbox_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.Toolbox.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.Toolbox object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gv_toolbox_add_tool(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "tool", NULL };
    char *name;
    PyGObject *tool;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO!:GvToolbox.add_tool", kwlist, &name, &PyGvTool_Type, &tool))
        return NULL;
    
    gv_toolbox_add_tool(GV_TOOLBOX(self->obj), name, GV_TOOL(tool->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_toolbox_activate_tool(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tool_name", NULL };
    char *tool_name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvToolbox.activate_tool", kwlist, &tool_name))
        return NULL;
    
    gv_toolbox_activate_tool(GV_TOOLBOX(self->obj), tool_name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvToolbox_methods[] = {
    { "add_tool", (PyCFunction)_wrap_gv_toolbox_add_tool, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "activate_tool", (PyCFunction)_wrap_gv_toolbox_activate_tool, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gv_toolbox__get_active_tool(PyObject *self, void *closure)
{
    GvTool *ret;

    ret = GV_TOOLBOX(pygobject_get(self))->active_tool;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyGetSetDef gv_toolbox_getsets[] = {
    { "active_tool", (getter)_wrap_gv_toolbox__get_active_tool, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGvToolbox_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.Toolbox",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvToolbox_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gv_toolbox_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_toolbox_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvTrackTool ----------- */

#line 3395 "gv.override"
static int
_wrap_gv_track_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "label", NULL };
    PyGObject *label;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:_gv.TrackTool.__init__",
                                    kwlist, &PyGtkObject_Type, &label))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GvTrackTool object");
        return -1;
    }

    GV_TRACK_TOOL(self->obj)->label = GTK_OBJECT(label->obj);
    return 0;
}
#line 6000 "gv.c"


PyTypeObject G_GNUC_INTERNAL PyGvTrackTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.TrackTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_track_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvViewArea ----------- */

static int
_wrap_gv_view_area_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.ViewArea.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.ViewArea object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gv_view_area_set_mode(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flag_3d", NULL };
    int flag_3d;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvViewArea.set_mode", kwlist, &flag_3d))
        return NULL;
    
    gv_view_area_set_mode(GV_VIEW_AREA(self->obj), flag_3d);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_area_get_mode(PyGObject *self)
{
    int ret;

    
    ret = gv_view_area_get_mode(GV_VIEW_AREA(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 271 "gv.override"
static PyObject *
_wrap_gv_view_area_height_scale(PyGObject *self, PyObject *args)
{
    double scale;

    if (!PyArg_ParseTuple(args, "d:GvViewArea.height_scale", &scale))
        return NULL;

    gv_view_area_height_scale(GV_VIEW_AREA(self->obj), scale);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6112 "gv.c"


#line 286 "gv.override"
static PyObject *
_wrap_gv_view_area_get_height_scale(PyGObject *self)
{
    return PyFloat_FromDouble(gv_view_area_get_height_scale(GV_VIEW_AREA(self->obj)));
}
#line 6121 "gv.c"


#line 475 "gv.override"
static PyObject *
_wrap_gv_view_area_set_3d_view(PyGObject *self, PyObject *args)
{
    vec3_t     eye_pos, eye_dir;

    if (!PyArg_ParseTuple(args,  "(" CCC ")(" CCC "):GvViewArea.set_3d_view",
                          eye_pos+0, eye_pos+1, eye_pos+2,
                          eye_dir+0, eye_dir+1, eye_dir+2))
        return NULL;

    gv_view_area_set_3d_view( GV_VIEW_AREA(self->obj), eye_pos, eye_dir );

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6140 "gv.c"


#line 492 "gv.override"
static PyObject *
_wrap_gv_view_area_set_3d_view_look_at(PyGObject *self, PyObject *args)
{
    vec3_t     eye_pos;
    gvgeocoord eye_look_at[2];

    if (!PyArg_ParseTuple(args, "(" CCC ")(" CC "):GvViewArea.set_3d_view_look_at",
                          eye_pos+0, eye_pos+1, eye_pos+2,
                          eye_look_at+0, eye_look_at+1))
        return NULL;

    gv_view_area_set_3d_view_look_at( GV_VIEW_AREA(self->obj), eye_pos, eye_look_at );

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6160 "gv.c"


#line 534 "gv.override"
static PyObject *
_wrap_gv_view_area_get_look_at_pos(PyGObject *self)
{
    gvgeocoord  x, y;

    if (!gv_view_area_get_look_at_pos( GV_VIEW_AREA(self->obj), &x, &y))
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return Py_BuildValue("(" CC ")", x, y);
}
#line 6177 "gv.c"


static PyObject *
_wrap_gv_view_area_set_raw(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ref_layer", "raw_enable", NULL };
    PyGObject *ref_layer;
    int raw_enable, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GvViewArea.set_raw", kwlist, &PyGObject_Type, &ref_layer, &raw_enable))
        return NULL;
    
    ret = gv_view_area_set_raw(GV_VIEW_AREA(self->obj), G_OBJECT(ref_layer->obj), raw_enable);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_view_area_get_raw(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ref_layer", NULL };
    PyGObject *ref_layer;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvViewArea.get_raw", kwlist, &PyGObject_Type, &ref_layer))
        return NULL;
    
    ret = gv_view_area_get_raw(GV_VIEW_AREA(self->obj), G_OBJECT(ref_layer->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_view_area_queue_draw(PyGObject *self)
{
    
    gv_view_area_queue_draw(GV_VIEW_AREA(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 189 "gv.override"
static PyObject *
_wrap_gv_view_area_zoom(PyGObject *self, PyObject *args)
{
    double zoom;

    if (!PyArg_ParseTuple(args, "d:GvViewArea.zoom", &zoom))
        return NULL;

    gv_view_area_zoom(GV_VIEW_AREA(self->obj), zoom);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6234 "gv.c"


#line 204 "gv.override"
static PyObject *
_wrap_gv_view_area_get_zoom(PyGObject *self)
{
    return PyFloat_FromDouble(gv_view_area_get_zoom(GV_VIEW_AREA(self->obj)));
}
#line 6243 "gv.c"


#line 211 "gv.override"
static PyObject *
_wrap_gv_view_area_rotate(PyGObject *self, PyObject *args)
{
    double angle;

    if (!PyArg_ParseTuple(args, "d:GvViewArea.rotate", &angle))
        return NULL;

    gv_view_area_rotate(GV_VIEW_AREA(self->obj), angle);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6260 "gv.c"


#line 226 "gv.override"
static PyObject *
_wrap_gv_view_area_translate(PyGObject *self, PyObject *args)
{
    double dx, dy;

    if (!PyArg_ParseTuple(args, "dd:GvViewArea.translate", &dx, &dy))
        return NULL;

    gv_view_area_translate(GV_VIEW_AREA(self->obj), dx, dy);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6277 "gv.c"


#line 241 "gv.override"
static PyObject *
_wrap_gv_view_area_set_translation(PyGObject *self, PyObject *args)
{
    double x, y;

    if (!PyArg_ParseTuple(args, "dd:GvViewArea.set_translation", &x, &y))
        return NULL;

    gv_view_area_set_translation(GV_VIEW_AREA(self->obj), x, y);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6294 "gv.c"


static PyObject *
_wrap_gv_view_area_get_flip_x(PyGObject *self)
{
    int ret;

    
    ret = gv_view_area_get_flip_x(GV_VIEW_AREA(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_view_area_get_flip_y(PyGObject *self)
{
    int ret;

    
    ret = gv_view_area_get_flip_y(GV_VIEW_AREA(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_view_area_set_flip_xy(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flip_x", "flip_y", NULL };
    int flip_x, flip_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GvViewArea.set_flip_xy", kwlist, &flip_x, &flip_y))
        return NULL;
    
    gv_view_area_set_flip_xy(GV_VIEW_AREA(self->obj), flip_x, flip_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_area_fit_all_layers(PyGObject *self)
{
    
    gv_view_area_fit_all_layers(GV_VIEW_AREA(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 256 "gv.override"
static PyObject *
_wrap_gv_view_area_fit_extents(PyGObject *self, PyObject *args)
{
    double llx, llyy, width, height;

    if (!PyArg_ParseTuple(args, "dddd:GvViewArea.fit_extents", &llx, &llyy, &width, &height))
        return NULL;

    gv_view_area_fit_extents(GV_VIEW_AREA(self->obj), llx, llyy, width, height);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6358 "gv.c"


#line 382 "gv.override"
static PyObject *
_wrap_gv_view_area_get_extents(PyGObject *self)
{
    gvgeocoord xmin, ymin, xmax, ymax;

    gv_view_area_get_extents( GV_VIEW_AREA(self->obj), &xmin, &ymin, &xmax, &ymax );

    return Py_BuildValue("(" CCCC ")", xmin, ymin, xmax, ymax );
}
#line 6371 "gv.c"


#line 393 "gv.override"
static PyObject *
_wrap_gv_view_area_get_volume(PyGObject *self)
{
    double  volume[6];

    gv_view_area_get_volume( GV_VIEW_AREA(self->obj), volume );

    return Py_BuildValue("(" CCCCCC ")",
                         volume[0], volume[1],
                         volume[2], volume[3],
                         volume[4], volume[5] );
}

#line 6388 "gv.c"


#line 339 "gv.override"
static PyObject *
_wrap_gv_view_area_map_location(PyGObject *self, PyObject *args)
{
    gvgeocoord  x, y;

    if (!PyArg_ParseTuple(args, "(" CC "):GvViewArea.map_location", &x, &y))
        return NULL;

    gv_view_area_map_location( GV_VIEW_AREA(self->obj), x, y, &x, &y );

    return Py_BuildValue("(" CC ")", x, y );
}
#line 6404 "gv.c"


static PyObject *
_wrap_gv_view_area_copy_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "copy", NULL };
    PyGObject *copy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvViewArea.copy_state", kwlist, &PyGvViewArea_Type, &copy))
        return NULL;
    
    gv_view_area_copy_state(GV_VIEW_AREA(self->obj), GV_VIEW_AREA(copy->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 368 "gv.override"
static PyObject *
_wrap_gv_view_area_map_pointer(PyGObject *self, PyObject *args)
{
    gvgeocoord  x, y;

    if (!PyArg_ParseTuple(args, "(" CC "):GvViewArea.map_pointer", &x, &y))
        return NULL;

    gv_view_area_map_pointer(  GV_VIEW_AREA(self->obj), x, y, &x, &y );

    return Py_BuildValue("(" CC ")", x, y );
}
#line 6435 "gv.c"


#line 408 "gv.override"
static PyObject *
_wrap_gv_view_area_inverse_map_pointer(PyGObject *self, PyObject *args)
{
    gvgeocoord x, y;

    if (!PyArg_ParseTuple(args, "(" CC "):GvViewArea.inverse_map_pointer", &x, &y))
        return NULL;

    gv_view_area_inverse_map_pointer( GV_VIEW_AREA(self->obj), x, y, &x, &y );

    return Py_BuildValue("(" CC ")", x, y );
}
#line 6451 "gv.c"


static PyObject *
_wrap_gv_view_area_add_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", NULL };
    PyGObject *layer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvViewArea.add_layer", kwlist, &PyGObject_Type, &layer))
        return NULL;
    
    gv_view_area_add_layer(GV_VIEW_AREA(self->obj), G_OBJECT(layer->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 293 "gv.override"
static PyObject *
_wrap_gv_view_area_remove_layer(PyGObject *self, PyObject *args)
{
    PyGObject *layer;

    if (!PyArg_ParseTuple(args, "O!:GvViewArea.remove_layer", &PyGObject_Type, &layer))
        return NULL;

    gv_view_area_remove_layer(GV_VIEW_AREA(self->obj), G_OBJECT(layer->obj));

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6483 "gv.c"


static PyObject *
_wrap_gv_view_area_active_layer(PyGObject *self)
{
    GObject *ret;

    
    ret = gv_view_area_active_layer(GV_VIEW_AREA(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_view_area_set_active_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", NULL };
    PyGObject *layer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvViewArea.set_active_layer", kwlist, &PyGObject_Type, &layer))
        return NULL;
    
    gv_view_area_set_active_layer(GV_VIEW_AREA(self->obj), G_OBJECT(layer->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_area_get_layer_of_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer_type", "read_only_ok", NULL };
    PyObject *py_layer_type = NULL;
    int read_only_ok;
    GType layer_type;
    GObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oi:GvViewArea.get_layer_of_type", kwlist, &py_layer_type, &read_only_ok))
        return NULL;
    if ((layer_type = pyg_type_from_object(py_layer_type)) == 0)
        return NULL;
    
    ret = gv_view_area_get_layer_of_type(GV_VIEW_AREA(self->obj), layer_type, read_only_ok);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_view_area_get_named_layer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    GObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvViewArea.get_named_layer", kwlist, &name))
        return NULL;
    
    ret = gv_view_area_get_named_layer(GV_VIEW_AREA(self->obj), name);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 308 "gv.override"
static PyObject *
_wrap_gv_view_area_list_layers(PyGObject *self)
{
    PyObject *py_list;
    GvViewArea *view;
    GList *list;

    view = GV_VIEW_AREA(self->obj);

    py_list = PyList_New(0);
    for (list = gv_view_area_list_layers(view); list != NULL; list = list->next)
    {
        PyObject *layer = pygobject_new(G_OBJECT(list->data));
        PyList_Append(py_list, layer);
        Py_DECREF(layer);
    }

    g_list_free(list);
    return py_list;
}
#line 6570 "gv.c"


static PyObject *
_wrap_gv_view_area_get_primary_raster(PyGObject *self)
{
    GObject *ret;

    
    ret = gv_view_area_get_primary_raster(GV_VIEW_AREA(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_view_area_swap_layers(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer_a", "layer_b", NULL };
    int layer_a, layer_b;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GvViewArea.swap_layers", kwlist, &layer_a, &layer_b))
        return NULL;
    
    gv_view_area_swap_layers(GV_VIEW_AREA(self->obj), layer_a, layer_b);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 422 "gv.override"
static PyObject *
_wrap_gv_view_area_get_fontnames(PyGObject *self)
{
    GPtrArray *g_list;
    PyObject  *py_list;
    int       i;

    g_list = gv_view_area_get_fontnames( GV_VIEW_AREA(self->obj) );

    py_list = PyList_New(0);
    for( i = 0; i < g_list->len; i++ )
    {
        const char  *item = (const char *) g_ptr_array_index(g_list,i);
        PyObject    *py_item;

        py_item = Py_BuildValue( "s", item );
        PyList_Append( py_list, py_item );
        Py_DECREF( py_item );
    }

    g_ptr_array_free( g_list, FALSE );

    return py_list;
}
#line 6625 "gv.c"


#line 448 "gv.override"
static PyObject *
_wrap_gv_view_area_set_background_color(PyGObject *self, PyObject *args)
{
    GvColor color;

    if (!PyArg_ParseTuple(args, "(ffff):GvViewArea.set_background_color",
                            &color[0], &color[1], &color[2], &color[3]))
        return NULL;

    gv_view_area_set_background_color(GV_VIEW_AREA(self->obj), color);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 6643 "gv.c"


#line 464 "gv.override"
static PyObject *
_wrap_gv_view_area_get_background_color(PyGObject *self)
{
    GvColor color;

    gv_view_area_get_background_color(GV_VIEW_AREA(self->obj), color);

    return Py_BuildValue("(ffff)", color[0], color[1], color[2], color[3] );
}
#line 6656 "gv.c"


static PyObject *
_wrap_gv_view_area_set_projection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "projection", NULL };
    char *projection;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvViewArea.set_projection", kwlist, &projection))
        return NULL;
    
    ret = gv_view_area_set_projection(GV_VIEW_AREA(self->obj), projection);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_view_area_get_projection(PyGObject *self)
{
    const gchar *ret;

    
    ret = gv_view_area_get_projection(GV_VIEW_AREA(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_area_print_to_file(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "width", "height", "filename", "format", "is_rgb", NULL };
    int width, height, is_rgb, ret;
    char *filename, *format;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iissi:GvViewArea.print_to_file", kwlist, &width, &height, &filename, &format, &is_rgb))
        return NULL;
    
    ret = gv_view_area_print_to_file(GV_VIEW_AREA(self->obj), width, height, filename, format, is_rgb);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_view_area_print_postscript_to_file(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "width", "height", "ulx", "uly", "lrx", "lry", "is_rgb", "filename", NULL };
    int width, height, is_rgb, ret;
    double ulx, uly, lrx, lry;
    char *filename;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiddddis:GvViewArea.print_postscript_to_file", kwlist, &width, &height, &ulx, &uly, &lrx, &lry, &is_rgb, &filename))
        return NULL;
    
    ret = gv_view_area_print_postscript_to_file(GV_VIEW_AREA(self->obj), width, height, ulx, uly, lrx, lry, is_rgb, filename);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_view_area_print_to_windriver(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "width", "height", "ulx", "uly", "lrx", "lry", "is_rgb", NULL };
    int width, height, is_rgb, ret;
    double ulx, uly, lrx, lry;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiddddi:GvViewArea.print_to_windriver", kwlist, &width, &height, &ulx, &uly, &lrx, &lry, &is_rgb))
        return NULL;
    
    ret = gv_view_area_print_to_windriver(GV_VIEW_AREA(self->obj), width, height, ulx, uly, lrx, lry, is_rgb);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_view_area_set_property(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "value", NULL };
    char *name, *value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:GvViewArea.set_property", kwlist, &name, &value))
        return NULL;
    
    gv_view_area_set_property(GV_VIEW_AREA(self->obj), name, value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_area_get_property(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvViewArea.get_property", kwlist, &name))
        return NULL;
    
    ret = gv_view_area_get_property(GV_VIEW_AREA(self->obj), name);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 549 "gv.override"
static PyObject *
_wrap_gv_view_area_format_point_query(PyGObject *self, PyObject *args)
{
    gdouble x, y;
    char *text;
    GvManager *manager;

    if (!PyArg_ParseTuple(args, "dd:GvViewArea.format_point_query", &x, &y))
        return NULL;

    manager = gv_get_manager();

    text = (char *) gv_view_area_format_point_query(GV_VIEW_AREA(self->obj),
                                                    &(manager->preferences), x, y);

    return Py_BuildValue("s", text);
}
#line 6785 "gv.c"


#line 3516 "gv.override"
static PyObject *
_wrap_gv_view_area_set_rotation(PyGObject *self, PyObject *args)
{
    double angle;

    if (!PyArg_ParseTuple(args, "d:GvViewArea.rotate", &angle))
        Py_RETURN_FALSE;

    gv_view_area_set_rotation(GV_VIEW_AREA(self->obj), angle);

	Py_RETURN_TRUE;
}
#line 6801 "gv.c"


#line 510 "gv.override"
static PyObject *
_wrap_gv_view_area_get_eye_pos(PyGObject *self)
{
    GvViewArea *view = GV_VIEW_AREA(self->obj);

    return Py_BuildValue( "(" CCC ")",
                          view->state.eye_pos[0],
                          view->state.eye_pos[1],
                          view->state.eye_pos[2] );
}
#line 6815 "gv.c"


#line 2822 "gv.override"
static PyObject *
_wrap_gv_view_area_get_height(PyGObject *self)
{
    return PyInt_FromLong(gv_view_area_get_height(GV_VIEW_AREA(self->obj)));
}
#line 6824 "gv.c"


#line 3507 "gv.override"
static PyObject *
_wrap_gv_view_area_get_rotation(PyGObject *self)
{
    GvViewArea *view = GV_VIEW_AREA(self->obj);

    return Py_BuildValue( "d", view->state.rot);
}
#line 6835 "gv.c"


#line 330 "gv.override"
static PyObject *
_wrap_gv_view_area_get_translation(PyGObject *self)
{
    GvViewArea *view = GV_VIEW_AREA(self->obj);

    return Py_BuildValue( "(" CC ")", view->state.tx, view->state.ty );
}
#line 6846 "gv.c"


#line 175 "gv.override"
static PyObject *
_wrap_gv_view_area_get_width(PyGObject *self)
{
    return PyInt_FromLong(gv_view_area_get_width(GV_VIEW_AREA(self->obj)));
}
#line 6855 "gv.c"


#line 522 "gv.override"
static PyObject *
_wrap_gv_view_area_get_eye_dir(PyGObject *self)
{
    GvViewArea *view = GV_VIEW_AREA(self->obj);

    return Py_BuildValue( "(" CCC ")",
                          view->state.eye_dir[0],
                          view->state.eye_dir[1],
                          view->state.eye_dir[2] );
}
#line 6869 "gv.c"


static const PyMethodDef _PyGvViewArea_methods[] = {
    { "set_mode", (PyCFunction)_wrap_gv_view_area_set_mode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_mode", (PyCFunction)_wrap_gv_view_area_get_mode, METH_NOARGS,
      NULL },
    { "height_scale", (PyCFunction)_wrap_gv_view_area_height_scale, METH_VARARGS,
      NULL },
    { "get_height_scale", (PyCFunction)_wrap_gv_view_area_get_height_scale, METH_NOARGS,
      NULL },
    { "set_3d_view", (PyCFunction)_wrap_gv_view_area_set_3d_view, METH_VARARGS,
      NULL },
    { "set_3d_view_look_at", (PyCFunction)_wrap_gv_view_area_set_3d_view_look_at, METH_VARARGS,
      NULL },
    { "get_look_at_pos", (PyCFunction)_wrap_gv_view_area_get_look_at_pos, METH_NOARGS,
      NULL },
    { "set_raw", (PyCFunction)_wrap_gv_view_area_set_raw, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_raw", (PyCFunction)_wrap_gv_view_area_get_raw, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "queue_draw", (PyCFunction)_wrap_gv_view_area_queue_draw, METH_NOARGS,
      NULL },
    { "zoom", (PyCFunction)_wrap_gv_view_area_zoom, METH_VARARGS,
      NULL },
    { "get_zoom", (PyCFunction)_wrap_gv_view_area_get_zoom, METH_VARARGS,
      NULL },
    { "rotate", (PyCFunction)_wrap_gv_view_area_rotate, METH_VARARGS,
      NULL },
    { "translate", (PyCFunction)_wrap_gv_view_area_translate, METH_VARARGS,
      NULL },
    { "set_translation", (PyCFunction)_wrap_gv_view_area_set_translation, METH_VARARGS,
      NULL },
    { "get_flip_x", (PyCFunction)_wrap_gv_view_area_get_flip_x, METH_NOARGS,
      NULL },
    { "get_flip_y", (PyCFunction)_wrap_gv_view_area_get_flip_y, METH_NOARGS,
      NULL },
    { "set_flip_xy", (PyCFunction)_wrap_gv_view_area_set_flip_xy, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "fit_all_layers", (PyCFunction)_wrap_gv_view_area_fit_all_layers, METH_NOARGS,
      NULL },
    { "fit_extents", (PyCFunction)_wrap_gv_view_area_fit_extents, METH_VARARGS,
      NULL },
    { "get_extents", (PyCFunction)_wrap_gv_view_area_get_extents, METH_NOARGS,
      NULL },
    { "get_volume", (PyCFunction)_wrap_gv_view_area_get_volume, METH_NOARGS,
      NULL },
    { "map_location", (PyCFunction)_wrap_gv_view_area_map_location, METH_VARARGS,
      NULL },
    { "copy_state", (PyCFunction)_wrap_gv_view_area_copy_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "map_pointer", (PyCFunction)_wrap_gv_view_area_map_pointer, METH_VARARGS,
      NULL },
    { "inverse_map_pointer", (PyCFunction)_wrap_gv_view_area_inverse_map_pointer, METH_VARARGS,
      NULL },
    { "add_layer", (PyCFunction)_wrap_gv_view_area_add_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_layer", (PyCFunction)_wrap_gv_view_area_remove_layer, METH_VARARGS,
      NULL },
    { "active_layer", (PyCFunction)_wrap_gv_view_area_active_layer, METH_NOARGS,
      NULL },
    { "set_active_layer", (PyCFunction)_wrap_gv_view_area_set_active_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_layer_of_type", (PyCFunction)_wrap_gv_view_area_get_layer_of_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_named_layer", (PyCFunction)_wrap_gv_view_area_get_named_layer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "list_layers", (PyCFunction)_wrap_gv_view_area_list_layers, METH_NOARGS,
      NULL },
    { "get_primary_raster", (PyCFunction)_wrap_gv_view_area_get_primary_raster, METH_NOARGS,
      NULL },
    { "swap_layers", (PyCFunction)_wrap_gv_view_area_swap_layers, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_fontnames", (PyCFunction)_wrap_gv_view_area_get_fontnames, METH_NOARGS,
      NULL },
    { "set_background_color", (PyCFunction)_wrap_gv_view_area_set_background_color, METH_VARARGS,
      NULL },
    { "get_background_color", (PyCFunction)_wrap_gv_view_area_get_background_color, METH_NOARGS,
      NULL },
    { "set_projection", (PyCFunction)_wrap_gv_view_area_set_projection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_projection", (PyCFunction)_wrap_gv_view_area_get_projection, METH_NOARGS,
      NULL },
    { "print_to_file", (PyCFunction)_wrap_gv_view_area_print_to_file, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "print_postscript_to_file", (PyCFunction)_wrap_gv_view_area_print_postscript_to_file, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "print_to_windriver", (PyCFunction)_wrap_gv_view_area_print_to_windriver, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_property", (PyCFunction)_wrap_gv_view_area_set_property, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_property", (PyCFunction)_wrap_gv_view_area_get_property, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "format_point_query", (PyCFunction)_wrap_gv_view_area_format_point_query, METH_VARARGS,
      NULL },
    { "set_rotation", (PyCFunction)_wrap_gv_view_area_set_rotation, METH_VARARGS,
      NULL },
    { "get_eye_pos", (PyCFunction)_wrap_gv_view_area_get_eye_pos, METH_NOARGS,
      NULL },
    { "get_height", (PyCFunction)_wrap_gv_view_area_get_height, METH_NOARGS,
      NULL },
    { "get_rotation", (PyCFunction)_wrap_gv_view_area_get_rotation, METH_NOARGS,
      NULL },
    { "get_translation", (PyCFunction)_wrap_gv_view_area_get_translation, METH_NOARGS,
      NULL },
    { "get_width", (PyCFunction)_wrap_gv_view_area_get_width, METH_NOARGS,
      NULL },
    { "get_eye_dir", (PyCFunction)_wrap_gv_view_area_get_eye_dir, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gv_view_area__get_projection(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GV_VIEW_AREA(pygobject_get(self))->projection;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyGetSetDef gv_view_area_getsets[] = {
    { "projection", (getter)_wrap_gv_view_area__get_projection, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGvViewArea_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.ViewArea",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvViewArea_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gv_view_area_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_view_area_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvViewLink ----------- */

static int
_wrap_gv_view_link_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.ViewLink.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.ViewLink object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gv_view_link_register_view(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", NULL };
    PyGObject *view;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvViewLink.register_view", kwlist, &PyGvViewArea_Type, &view))
        return NULL;
    
    gv_view_link_register_view(GV_VIEW_LINK(self->obj), GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_link_remove_view(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", NULL };
    PyGObject *view;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvViewLink.remove_view", kwlist, &PyGvViewArea_Type, &view))
        return NULL;
    
    gv_view_link_remove_view(GV_VIEW_LINK(self->obj), GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_link_enable(PyGObject *self)
{
    
    gv_view_link_enable(GV_VIEW_LINK(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_link_disable(PyGObject *self)
{
    
    gv_view_link_disable(GV_VIEW_LINK(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_link_set_cursor_mode(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cursor_mode", NULL };
    int cursor_mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvViewLink.set_cursor_mode", kwlist, &cursor_mode))
        return NULL;
    
    gv_view_link_set_cursor_mode(GV_VIEW_LINK(self->obj), cursor_mode);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGvViewLink_methods[] = {
    { "register_view", (PyCFunction)_wrap_gv_view_link_register_view, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_view", (PyCFunction)_wrap_gv_view_link_remove_view, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "enable", (PyCFunction)_wrap_gv_view_link_enable, METH_NOARGS,
      NULL },
    { "disable", (PyCFunction)_wrap_gv_view_link_disable, METH_NOARGS,
      NULL },
    { "set_cursor_mode", (PyCFunction)_wrap_gv_view_link_set_cursor_mode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvViewLink_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.ViewLink",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvViewLink_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_view_link_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvZoompanTool ----------- */

static int
_wrap_gv_zoompan_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.ZoompanTool.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.ZoompanTool object");
        return -1;
    }
    return 0;
}

PyTypeObject G_GNUC_INTERNAL PyGvZoompanTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.ZoompanTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_zoompan_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GvAutopanTool ----------- */

static int
_wrap_gv_autopan_tool_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":_gv.AutopanTool.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create _gv.AutopanTool object");
        return -1;
    }
    return 0;
}

#line 3418 "gv.override"
static PyObject *
_wrap_gv_autopan_tool_new_rect(PyGObject *self, PyObject *args)
{
	GvRect rect;

	if (!PyArg_ParseTuple(args, "(dddd):_gv.AutopanTool.set_extents", &rect.x, &rect.y, &rect.width, &rect.height))
	        return NULL;

    if (!gv_autopan_tool_new_rect( GV_AUTOPAN_TOOL(self->obj), &rect))
    {
        Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}
#line 7301 "gv.c"


#line 3435 "gv.override"
static PyObject *
_wrap_gv_autopan_tool_get_rect(PyGObject *self)
{
	GvRect rect;

    if (!gv_autopan_tool_get_rect( GV_AUTOPAN_TOOL(self->obj), &rect))
    {
        Py_RETURN_NONE;
    }

    return Py_BuildValue("(" CCCC ")", rect.x, rect.y, rect.width, rect.height);
}
#line 7317 "gv.c"


static PyObject *
_wrap_gv_autopan_tool_play(PyGObject *self)
{
    int ret;

    
    ret = gv_autopan_tool_play(GV_AUTOPAN_TOOL(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_pause(PyGObject *self)
{
    int ret;

    
    ret = gv_autopan_tool_pause(GV_AUTOPAN_TOOL(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_stop(PyGObject *self)
{
    int ret;

    
    ret = gv_autopan_tool_stop(GV_AUTOPAN_TOOL(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_set_speed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "speed", NULL };
    double speed, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:GvAutopanTool.set_speed", kwlist, &speed))
        return NULL;
    
    ret = gv_autopan_tool_set_speed(GV_AUTOPAN_TOOL(self->obj), speed);
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gv_autopan_tool_get_speed(PyGObject *self)
{
    double ret;

    
    ret = gv_autopan_tool_get_speed(GV_AUTOPAN_TOOL(self->obj));
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gv_autopan_tool_set_overlap(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "overlap", NULL };
    int ret;
    double overlap;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:GvAutopanTool.set_overlap", kwlist, &overlap))
        return NULL;
    
    ret = gv_autopan_tool_set_overlap(GV_AUTOPAN_TOOL(self->obj), overlap);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_get_overlap(PyGObject *self)
{
    double ret;

    
    ret = gv_autopan_tool_get_overlap(GV_AUTOPAN_TOOL(self->obj));
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gv_autopan_tool_set_block_x_size(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "block_x_size", "mode", NULL };
    int mode, ret;
    double block_x_size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"di:GvAutopanTool.set_block_x_size", kwlist, &block_x_size, &mode))
        return NULL;
    
    ret = gv_autopan_tool_set_block_x_size(GV_AUTOPAN_TOOL(self->obj), block_x_size, mode);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_set_x_resolution(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "resolution", NULL };
    int ret;
    double resolution;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:GvAutopanTool.set_x_resolution", kwlist, &resolution))
        return NULL;
    
    ret = gv_autopan_tool_set_x_resolution(GV_AUTOPAN_TOOL(self->obj), resolution);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_set_standard_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "path_type", NULL };
    int path_type, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GvAutopanTool.set_standard_path", kwlist, &path_type))
        return NULL;
    
    ret = gv_autopan_tool_set_standard_path(GV_AUTOPAN_TOOL(self->obj), path_type);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_set_lines_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "lines", NULL };
    PyGObject *lines;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvAutopanTool.set_lines_path", kwlist, &PyGvShapes_Type, &lines))
        return NULL;
    
    ret = gv_autopan_tool_set_lines_path(GV_AUTOPAN_TOOL(self->obj), GV_SHAPES(lines->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_set_location(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "z", NULL };
    int ret;
    double x, y, z;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddd:GvAutopanTool.set_location", kwlist, &x, &y, &z))
        return NULL;
    
    ret = gv_autopan_tool_set_location(GV_AUTOPAN_TOOL(self->obj), x, y, z);
    
    return PyInt_FromLong(ret);
}

#line 3449 "gv.override"
static PyObject *
_wrap_gv_autopan_tool_get_location(PyGObject *self)
{
    gvgeocoord  x, y, z;

    if (!gv_autopan_tool_get_location( GV_AUTOPAN_TOOL(self->obj), &x, &y, &z))
    {
    	Py_RETURN_NONE;
    }

    return Py_BuildValue("(" CCC ")", x, y, z);
}
#line 7491 "gv.c"


#line 3463 "gv.override"
static PyObject *
_wrap_gv_autopan_tool_get_state(PyGObject *self)
{
	gint play_flag, path_type, block_size_mode, num_views;
    gvgeocoord  block_x_size, x_resolution;

    gv_autopan_tool_get_state( GV_AUTOPAN_TOOL(self->obj), &play_flag, &path_type, &block_size_mode, &block_x_size, &x_resolution, &num_views);

    return Py_BuildValue("(iiiddi)", play_flag, path_type, block_size_mode, block_x_size, x_resolution, num_views);
}
#line 7505 "gv.c"


static PyObject *
_wrap_gv_autopan_tool_clear_trail(PyGObject *self)
{
    
    gv_autopan_tool_clear_trail(GV_AUTOPAN_TOOL(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_autopan_tool_set_trail_color(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", "red", "green", "blue", "alpha", NULL };
    PyGObject *view;
    double red, green, blue, alpha;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!dddd:GvAutopanTool.set_trail_color", kwlist, &PyGvViewArea_Type, &view, &red, &green, &blue, &alpha))
        return NULL;
    
    ret = gv_autopan_tool_set_trail_color(GV_AUTOPAN_TOOL(self->obj), GV_VIEW_AREA(view->obj), red, green, blue, alpha);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_set_trail_mode(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", "trail_mode", NULL };
    PyGObject *view;
    int trail_mode, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GvAutopanTool.set_trail_mode", kwlist, &PyGvViewArea_Type, &view, &trail_mode))
        return NULL;
    
    ret = gv_autopan_tool_set_trail_mode(GV_AUTOPAN_TOOL(self->obj), GV_VIEW_AREA(view->obj), trail_mode);
    
    return PyInt_FromLong(ret);
}

#line 3475 "gv.override"
static PyObject *
_wrap_gv_autopan_tool_set_trail_parameters(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "overview_extents", "overview_width_pixels", NULL };
    GvRect overview_extents;
    int overview_width_pixels;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "(dddd)i:_gv.AutopanTool.set_trail_parameters",
                                    kwlist, &overview_extents.x, &overview_extents.y, &overview_extents.width, &overview_extents.height, &overview_width_pixels))
        return NULL;

    if (!gv_autopan_tool_set_trail_parameters( GV_AUTOPAN_TOOL(self->obj), &overview_extents, overview_width_pixels))
    {
        Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}
#line 7568 "gv.c"


#line 3495 "gv.override"
static PyObject *
_wrap_gv_autopan_tool_get_trail_parameters(PyGObject *self)
{
	GvRect overview_extents;
	int overview_width_pixels, num_trail_tiles;

    gv_autopan_tool_get_trail_parameters( GV_AUTOPAN_TOOL(self->obj), &overview_extents, &overview_width_pixels, &num_trail_tiles);

    return Py_BuildValue("(dddd)ii", overview_extents.x, overview_extents.y, overview_extents.width, overview_extents.height, overview_width_pixels, num_trail_tiles);
}
#line 7582 "gv.c"


static PyObject *
_wrap_gv_autopan_tool_save_trail_tiles(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "basename", NULL };
    char *basename;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GvAutopanTool.save_trail_tiles", kwlist, &basename))
        return NULL;
    
    ret = gv_autopan_tool_save_trail_tiles(GV_AUTOPAN_TOOL(self->obj), basename);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_load_trail_tiles(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "basename", "num_trail_tiles", NULL };
    char *basename;
    int num_trail_tiles, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:GvAutopanTool.load_trail_tiles", kwlist, &basename, &num_trail_tiles))
        return NULL;
    
    ret = gv_autopan_tool_load_trail_tiles(GV_AUTOPAN_TOOL(self->obj), basename, num_trail_tiles);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_register_view(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", "can_resize", "can_reposition", "trail_mode", NULL };
    PyGObject *view;
    int can_resize, can_reposition, trail_mode, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iii:GvAutopanTool.register_view", kwlist, &PyGvViewArea_Type, &view, &can_resize, &can_reposition, &trail_mode))
        return NULL;
    
    ret = gv_autopan_tool_register_view(GV_AUTOPAN_TOOL(self->obj), GV_VIEW_AREA(view->obj), can_resize, can_reposition, trail_mode);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_autopan_tool_remove_view(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "view", NULL };
    PyGObject *view;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GvAutopanTool.remove_view", kwlist, &PyGvViewArea_Type, &view))
        return NULL;
    
    ret = gv_autopan_tool_remove_view(GV_AUTOPAN_TOOL(self->obj), GV_VIEW_AREA(view->obj));
    
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGvAutopanTool_methods[] = {
    { "set_extents", (PyCFunction)_wrap_gv_autopan_tool_new_rect, METH_VARARGS,
      NULL },
    { "get_extents", (PyCFunction)_wrap_gv_autopan_tool_get_rect, METH_NOARGS,
      NULL },
    { "play", (PyCFunction)_wrap_gv_autopan_tool_play, METH_NOARGS,
      NULL },
    { "pause", (PyCFunction)_wrap_gv_autopan_tool_pause, METH_NOARGS,
      NULL },
    { "stop", (PyCFunction)_wrap_gv_autopan_tool_stop, METH_NOARGS,
      NULL },
    { "set_speed", (PyCFunction)_wrap_gv_autopan_tool_set_speed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_speed", (PyCFunction)_wrap_gv_autopan_tool_get_speed, METH_NOARGS,
      NULL },
    { "set_overlap", (PyCFunction)_wrap_gv_autopan_tool_set_overlap, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_overlap", (PyCFunction)_wrap_gv_autopan_tool_get_overlap, METH_NOARGS,
      NULL },
    { "set_block_x_size", (PyCFunction)_wrap_gv_autopan_tool_set_block_x_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_x_resolution", (PyCFunction)_wrap_gv_autopan_tool_set_x_resolution, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_standard_path", (PyCFunction)_wrap_gv_autopan_tool_set_standard_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_lines_path", (PyCFunction)_wrap_gv_autopan_tool_set_lines_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_location", (PyCFunction)_wrap_gv_autopan_tool_set_location, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_location", (PyCFunction)_wrap_gv_autopan_tool_get_location, METH_NOARGS,
      NULL },
    { "get_state", (PyCFunction)_wrap_gv_autopan_tool_get_state, METH_NOARGS,
      NULL },
    { "clear_trail", (PyCFunction)_wrap_gv_autopan_tool_clear_trail, METH_NOARGS,
      NULL },
    { "set_trail_color", (PyCFunction)_wrap_gv_autopan_tool_set_trail_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_trail_mode", (PyCFunction)_wrap_gv_autopan_tool_set_trail_mode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_trail_parameters", (PyCFunction)_wrap_gv_autopan_tool_set_trail_parameters, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_trail_parameters", (PyCFunction)_wrap_gv_autopan_tool_get_trail_parameters, METH_NOARGS,
      NULL },
    { "save_trail_tiles", (PyCFunction)_wrap_gv_autopan_tool_save_trail_tiles, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "load_trail_tiles", (PyCFunction)_wrap_gv_autopan_tool_load_trail_tiles, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "register_view", (PyCFunction)_wrap_gv_autopan_tool_register_view, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_view", (PyCFunction)_wrap_gv_autopan_tool_remove_view, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGvAutopanTool_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "_gv.AutopanTool",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGvAutopanTool_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gv_autopan_tool_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

static PyObject *
_wrap_gv_data_registry_dump(PyObject *self)
{
    
    gv_data_registry_dump();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_get_manager(PyObject *self)
{
    GvManager *ret;

    
    ret = gv_get_manager();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_raster_cache_get_max(PyObject *self)
{
    int ret;

    
    ret = gv_raster_cache_get_max();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_raster_cache_set_max(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "max", NULL };
    int max;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gv_raster_cache_set_max", kwlist, &max))
        return NULL;
    
    gv_raster_cache_set_max(max);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_raster_cache_get_used(PyObject *self)
{
    int ret;

    
    ret = gv_raster_cache_get_used();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_texture_cache_set_max(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "max", NULL };
    int max;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gv_texture_cache_set_max", kwlist, &max))
        return NULL;
    
    gv_texture_cache_set_max(max);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_texture_cache_get_max(PyObject *self)
{
    int ret;

    
    ret = gv_texture_cache_get_max();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_texture_cache_get_used(PyObject *self)
{
    int ret;

    
    ret = gv_texture_cache_get_used();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_texture_cache_dump(PyObject *self)
{
    
    gv_texture_cache_dump();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shapes_from_shapefile(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", NULL };
    char *filename;
    GvData *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gv_shapes_from_shapefile", kwlist, &filename))
        return NULL;
    
    ret = gv_shapes_from_shapefile(filename);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_shapes_from_ogr(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", "iLayer", NULL };
    char *filename;
    int iLayer;
    GvData *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:gv_shapes_from_ogr", kwlist, &filename, &iLayer))
        return NULL;
    
    ret = gv_shapes_from_ogr(filename, iLayer);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_shapes_layer_draw_selected(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", "view", NULL };
    PyGObject *layer, *view;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:gv_shapes_layer_draw_selected", kwlist, &PyGvShapeLayer_Type, &layer, &PyGvViewArea_Type, &view))
        return NULL;
    
    gv_shapes_layer_draw_selected(GV_SHAPE_LAYER(layer->obj), GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shapes_layer_draw(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layer", "view", NULL };
    PyGObject *layer, *view;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:gv_shapes_layer_draw", kwlist, &PyGvLayer_Type, &layer, &PyGvViewArea_Type, &view))
        return NULL;
    
    gv_shapes_layer_draw(GV_LAYER(layer->obj), GV_VIEW_AREA(view->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_get_symbol_manager(PyObject *self)
{
    GvSymbolManager *ret;

    
    ret = gv_get_symbol_manager();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gv_undo_register_data(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", NULL };
    PyGObject *data;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gv_undo_register_data", kwlist, &PyGvData_Type, &data))
        return NULL;
    
    gv_undo_register_data(GV_DATA(data->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_undo_open(PyObject *self)
{
    
    gv_undo_open();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_undo_close(PyObject *self)
{
    
    gv_undo_close();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_undo_pop(PyObject *self)
{
    
    gv_undo_pop();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_undo_clear(PyObject *self)
{
    
    gv_undo_clear();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_undo_enable(PyObject *self)
{
    
    gv_undo_enable();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_undo_disable(PyObject *self)
{
    
    gv_undo_disable();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_undo_can_undo(PyObject *self)
{
    int ret;

    
    ret = gv_undo_can_undo();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_undo_start_group(PyObject *self)
{
    int ret;

    
    ret = gv_undo_start_group();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gv_undo_end_group(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "group", NULL };
    int group;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gv_undo_end_group", kwlist, &group))
        return NULL;
    
    gv_undo_end_group(group);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_make_latlong_srs(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "projected_srs", NULL };
    char *projected_srs;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gv_make_latlong_srs", kwlist, &projected_srs))
        return NULL;
    
    ret = gv_make_latlong_srs(projected_srs);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

#line 3212 "gv.override"
static PyObject *
_wrap_gv_launch_url(PyObject *self, PyObject *args)
{
    char *url = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_launch_url", &url))
        return NULL;

    return Py_BuildValue("i",gv_launch_url(url));
}
#line 8073 "gv.c"


#line 3224 "gv.override"
static PyObject *
_wrap_gv_short_path_name(PyObject *self, PyObject *args)
{
    char *lpszLongPath = NULL;
    PyObject * result = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_short_path_name", &lpszLongPath))
        return NULL;

    result = Py_BuildValue("s",gv_short_path_name(lpszLongPath));

    //invalid path in lpszLongPath results in zero length string
    if (PyString_Size(result) == 0)
    {
        PyErr_Format(PyExc_OSError, "path (%s) does not exist.", lpszLongPath);
        return NULL;
    }

    return result;
}
#line 8097 "gv.c"


static PyObject *
_wrap_gv_shape_get_count(PyObject *self)
{
    int ret;

    
    ret = gv_shape_get_count();
    
    return PyInt_FromLong(ret);
}

#line 3152 "gv.override"
static PyObject *
_wrap_gv_rgba_to_rgb(PyObject *self, PyObject *args)
{
    PyObject *rgba_obj = NULL;
    PyObject *rgb_obj = NULL;
    const char *rgba;
    char       *rgb;
    int length, i;

    if (!PyArg_ParseTuple(args, "O!:gv_rgba_to_rgb", &PyString_Type, &rgba_obj))
        return NULL;

    length = PyString_Size( rgba_obj ) / 4;
    rgba = PyString_AS_STRING( rgba_obj );

    rgb = (char *) malloc(length*3);

    for( i = 0; i < length; i++ )
    {
        rgb[i*3  ] = rgba[i*4  ];
        rgb[i*3+1] = rgba[i*4+1];
        rgb[i*3+2] = rgba[i*4+2];
    }

    rgb_obj = PyString_FromStringAndSize( rgb, length * 3 );

    free( rgb );

    return rgb_obj;
}
#line 8142 "gv.c"


#line 3303 "gv.override"
static PyObject *
_wrap_gv_shapes_lines_for_vecplot(PyObject *self, PyObject *args)
{
    PyObject *pyxlist=NULL;
    PyObject *pyylist=NULL;
    PyObject *pyzlist=NULL;
    PyObject *pyoklist=NULL;

    int       node_count, i, j, last_ok, shape_count,last_shape_nodes, oknode;
    GvShape *shape;
    GvShapes *shapes;
    gvgeocoord xnode, ynode, znode;
    int ring=0;
    int *shape_ids=NULL;

    if (!PyArg_ParseTuple(args, "O!O!O!O!:gv_shapes_lines_for_vecplot",
                          &PyList_Type,&pyxlist,
                          &PyList_Type,&pyylist,
                          &PyList_Type,&pyzlist,
                          &PyList_Type,&pyoklist) )
    return NULL;

    node_count=PyList_Size(pyxlist);
    if (node_count < 1)
    {
        PyErr_SetString(PyExc_ValueError,
              "require at least one node in list for gv_shapes_lines_for_vecplot");
        return NULL;
    }
    if ((node_count != PyList_Size(pyylist)) ||
        (node_count != PyList_Size(pyzlist)) ||
        (node_count != PyList_Size(pyoklist)))
    {
        PyErr_SetString(PyExc_ValueError,
              "x, y, z, ok lists must have identical lengths for gv_shapes_lines_for_vecplot");
        return NULL;
    }
    shapes = (GvShapes *) gv_shapes_new();
    shape = gv_shape_new(GVSHAPE_LINE);
    gv_shapes_add_shape(shapes,shape);
    last_ok = 1;
    shape_count = 1;
    last_shape_nodes = 0;
    for( i = 0; i < node_count; i++ )
    {
        if(( !PyArg_Parse( PyList_GET_ITEM(pyxlist,i), 
              Ccast ":gv_shapes_lines_for_vecplot" , &xnode ) ) ||
           ( !PyArg_Parse( PyList_GET_ITEM(pyylist,i), 
              Ccast ":gv_shapes_lines_for_vecplot" , &ynode ) ) ||
           ( !PyArg_Parse( PyList_GET_ITEM(pyzlist,i), 
              Ccast ":gv_shapes_lines_for_vecplot" , &znode ) ) ||
           ( !PyArg_Parse( PyList_GET_ITEM(pyoklist,i), 
              "i:gv_shapes_lines_for_vecplot" , &oknode ) ))
        {
            PyErr_SetString(PyExc_ValueError,
          "expecting floats for nodes, ints for ok in gv_shapes_lines_for_vecplot arguments");
            shape_ids = g_new(int, shape_count);
            for ( j= 0 ; j < shape_count; j++ )
            {
                *(shape_ids+sizeof(int)) = j;
            }
            gv_shapes_delete_shapes(shapes, shape_count, shape_ids);
            g_free(shape_ids);
            return NULL;
        }
        if (oknode == 1)
        {
            gv_shape_add_node(shape,ring,xnode,ynode,znode);
            last_ok = 1;
            last_shape_nodes=last_shape_nodes+1;
        }
        else if (last_ok == 1)
        {
            shape = gv_shape_new(GVSHAPE_LINE);
            gv_shapes_add_shape(shapes, shape);
            shape_count = shape_count+1;
            last_shape_nodes = 0;
            last_ok = 0;
        } 
    }
    if (last_shape_nodes == 0)
    {
        shape_ids = g_new(int,1);
        *shape_ids = shape_count-1;
        gv_shapes_delete_shapes(shapes, 1, shape_ids);
        g_free(shape_ids);
    }

    return pygobject_new((GObject *) shapes);
}
#line 8236 "gv.c"


#line 610 "gv.override"
static PyObject *
_wrap_gv_have_ogr_support(PyObject *self)
{
    return PyInt_FromLong(gv_have_ogr_support());
}
#line 8245 "gv.c"


#line 3246 "gv.override"
static PyObject *
_wrap_gv_shape_line_from_nodelists(PyObject *self, PyObject *args)
{
    PyObject *pyxlist = NULL;
    PyObject *pyylist = NULL;
    PyObject *pyzlist = NULL;

    int       node_count, i;
    GvShape *shape;
    gvgeocoord xnode, ynode, znode;
    int ring = 0;

    if (!PyArg_ParseTuple(args, "O!O!O!:gv_shape_line_from_nodelist",
                          &PyList_Type,&pyxlist,
                          &PyList_Type,&pyylist,
                          &PyList_Type,&pyzlist) )
    return NULL;

    node_count = PyList_Size(pyxlist);
    if (node_count < 1)
    {
        PyErr_SetString(PyExc_ValueError,
              "require at least one node in list for gv_shape_line_from_nodelist");
        return NULL;
    }
    if ((node_count != PyList_Size(pyylist)) ||
        (node_count != PyList_Size(pyzlist)))
    {
        PyErr_SetString(PyExc_ValueError,
              "x, y, and z node lists must have identical lengths for gv_shape_line_from_nodelist");
        return NULL;
    }

    shape = gv_shape_new(GVSHAPE_LINE);

    for( i = 0; i < node_count; i++ )
    {
        if ( ( !PyArg_Parse( PyList_GET_ITEM(pyxlist,i), 
                Ccast ":gv_shape_line_from_nodelist" , &xnode ) ) ||
             ( !PyArg_Parse( PyList_GET_ITEM(pyylist,i), 
                Ccast ":gv_shape_line_from_nodelist" , &ynode ) ) ||
             ( !PyArg_Parse( PyList_GET_ITEM(pyzlist,i), 
                Ccast ":gv_shape_line_from_nodelist" , &znode ) ))
        {
            PyErr_SetString(PyExc_ValueError,
                    "expecting floats in gv_shape_line_from_nodelist arguments");
            gv_shape_delete(shape);
            return NULL;
        }
        gv_shape_add_node(shape, ring, xnode, ynode, znode);
    }

    return pygv_shape_from_shape(shape);
}

#line 8304 "gv.c"


const PyMethodDef _gv_functions[] = {
    { "gv_data_registry_dump", (PyCFunction)_wrap_gv_data_registry_dump, METH_NOARGS,
      NULL },
    { "gv_get_manager", (PyCFunction)_wrap_gv_get_manager, METH_NOARGS,
      NULL },
    { "gv_raster_cache_get_max", (PyCFunction)_wrap_gv_raster_cache_get_max, METH_NOARGS,
      NULL },
    { "gv_raster_cache_set_max", (PyCFunction)_wrap_gv_raster_cache_set_max, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gv_raster_cache_get_used", (PyCFunction)_wrap_gv_raster_cache_get_used, METH_NOARGS,
      NULL },
    { "gv_texture_cache_set_max", (PyCFunction)_wrap_gv_texture_cache_set_max, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gv_texture_cache_get_max", (PyCFunction)_wrap_gv_texture_cache_get_max, METH_NOARGS,
      NULL },
    { "gv_texture_cache_get_used", (PyCFunction)_wrap_gv_texture_cache_get_used, METH_NOARGS,
      NULL },
    { "gv_texture_cache_dump", (PyCFunction)_wrap_gv_texture_cache_dump, METH_NOARGS,
      NULL },
    { "gv_shapes_from_shapefile", (PyCFunction)_wrap_gv_shapes_from_shapefile, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gv_shapes_from_ogr", (PyCFunction)_wrap_gv_shapes_from_ogr, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gv_shapes_layer_draw_selected", (PyCFunction)_wrap_gv_shapes_layer_draw_selected, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gv_shapes_layer_draw", (PyCFunction)_wrap_gv_shapes_layer_draw, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gv_get_symbol_manager", (PyCFunction)_wrap_gv_get_symbol_manager, METH_NOARGS,
      NULL },
    { "gv_undo_register_data", (PyCFunction)_wrap_gv_undo_register_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gv_undo_open", (PyCFunction)_wrap_gv_undo_open, METH_NOARGS,
      NULL },
    { "gv_undo_close", (PyCFunction)_wrap_gv_undo_close, METH_NOARGS,
      NULL },
    { "gv_undo_pop", (PyCFunction)_wrap_gv_undo_pop, METH_NOARGS,
      NULL },
    { "gv_undo_clear", (PyCFunction)_wrap_gv_undo_clear, METH_NOARGS,
      NULL },
    { "gv_undo_enable", (PyCFunction)_wrap_gv_undo_enable, METH_NOARGS,
      NULL },
    { "gv_undo_disable", (PyCFunction)_wrap_gv_undo_disable, METH_NOARGS,
      NULL },
    { "gv_undo_can_undo", (PyCFunction)_wrap_gv_undo_can_undo, METH_NOARGS,
      NULL },
    { "gv_undo_start_group", (PyCFunction)_wrap_gv_undo_start_group, METH_NOARGS,
      NULL },
    { "gv_undo_end_group", (PyCFunction)_wrap_gv_undo_end_group, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gv_make_latlong_srs", (PyCFunction)_wrap_gv_make_latlong_srs, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gv_launch_url", (PyCFunction)_wrap_gv_launch_url, METH_VARARGS,
      NULL },
    { "gv_short_path_name", (PyCFunction)_wrap_gv_short_path_name, METH_VARARGS,
      NULL },
    { "gv_shape_get_count", (PyCFunction)_wrap_gv_shape_get_count, METH_NOARGS,
      NULL },
    { "gv_rgba_to_rgb", (PyCFunction)_wrap_gv_rgba_to_rgb, METH_VARARGS,
      NULL },
    { "gv_shapes_lines_for_vecplot", (PyCFunction)_wrap_gv_shapes_lines_for_vecplot, METH_VARARGS,
      NULL },
    { "gv_have_ogr_support", (PyCFunction)_wrap_gv_have_ogr_support, METH_NOARGS,
      NULL },
    { "gv_shape_line_from_nodelists", (PyCFunction)_wrap_gv_shape_line_from_nodelists, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
_gv_add_constants(PyObject *module, const gchar *strip_prefix)
{
#ifdef VERSION
    PyModule_AddStringConstant(module, "__version__", VERSION);
#endif
  pyg_enum_add(module, "SampleMethod", strip_prefix, GV_TYPE_SAMPLE_METHOD);
  pyg_enum_add(module, "AutoScaleAlg", strip_prefix, GV_TYPE_AUTO_SCALE_ALG);
  pyg_enum_add(module, "RasterLayerMode", strip_prefix, GV_TYPE_RASTER_LAYER_MODE);
  pyg_enum_add(module, "RasterLutEnhancement", strip_prefix, GV_TYPE_RASTER_LUT_ENHANCEMENT);
  pyg_enum_add(module, "LinkCursorMode", strip_prefix, GV_TYPE_LINK_CURSOR_MODE);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
_gv_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gobject")) != NULL) {
        _PyGObject_Type = (PyTypeObject *)PyObject_GetAttrString(module, "GObject");
        if (_PyGObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name GObject from gobject");
            return ;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gobject");
        return ;
    }
    if ((module = PyImport_ImportModule("gtk")) != NULL) {
        _PyGtkDrawingArea_Type = (PyTypeObject *)PyObject_GetAttrString(module, "DrawingArea");
        if (_PyGtkDrawingArea_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name DrawingArea from gtk");
            return ;
        }
        _PyGtkButton_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Button");
        if (_PyGtkButton_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Button from gtk");
            return ;
        }
        _PyGtkObject_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Object");
        if (_PyGtkObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Object from gtk");
            return ;
        }
        _PyGtkAdjustment_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Adjustment");
        if (_PyGtkAdjustment_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Adjustment from gtk");
            return ;
        }
        _PyGtkWidget_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Widget");
        if (_PyGtkWidget_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Widget from gtk");
            return ;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gtk");
        return ;
    }


#line 8450 "gv.c"
    pygobject_register_class(d, "GvData", GV_TYPE_DATA, &PyGvData_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_DATA);
    pygobject_register_class(d, "GvLayer", GV_TYPE_LAYER, &PyGvLayer_Type, Py_BuildValue("(O)", &PyGvData_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_LAYER);
    pygobject_register_class(d, "GvManager", GV_TYPE_MANAGER, &PyGvManager_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_MANAGER);
    pygobject_register_class(d, "GvMesh", GV_TYPE_MESH, &PyGvMesh_Type, Py_BuildValue("(O)", &PyGvData_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_MESH);
    pygobject_register_class(d, "GvRaster", GV_TYPE_RASTER, &PyGvRaster_Type, Py_BuildValue("(O)", &PyGvData_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_RASTER);
    pygobject_register_class(d, "GvRasterLayer", GV_TYPE_RASTER_LAYER, &PyGvRasterLayer_Type, Py_BuildValue("(O)", &PyGvLayer_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_RASTER_LAYER);
    pygobject_register_class(d, "GvShapeLayer", GV_TYPE_SHAPE_LAYER, &PyGvShapeLayer_Type, Py_BuildValue("(O)", &PyGvLayer_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_SHAPE_LAYER);
    pygobject_register_class(d, "GvShapes", GV_TYPE_SHAPES, &PyGvShapes_Type, Py_BuildValue("(O)", &PyGvData_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_SHAPES);
    pygobject_register_class(d, "GvShapesLayer", GV_TYPE_SHAPES_LAYER, &PyGvShapesLayer_Type, Py_BuildValue("(O)", &PyGvShapeLayer_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_SHAPES_LAYER);
    pygobject_register_class(d, "GvPqueryLayer", GV_TYPE_PQUERY_LAYER, &PyGvPqueryLayer_Type, Py_BuildValue("(O)", &PyGvShapesLayer_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_PQUERY_LAYER);
    pygobject_register_class(d, "GvSymbolManager", GV_TYPE_SYMBOL_MANAGER, &PyGvSymbolManager_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_SYMBOL_MANAGER);
    pygobject_register_class(d, "GvTool", GV_TYPE_TOOL, &PyGvTool_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_TOOL);
    pygobject_register_class(d, "GvSelectionTool", GV_TYPE_SELECTION_TOOL, &PyGvSelectionTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_SELECTION_TOOL);
    pygobject_register_class(d, "GvRotateTool", GV_TYPE_ROTATE_TOOL, &PyGvRotateTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_ROTATE_TOOL);
    pygobject_register_class(d, "GvRoiTool", GV_TYPE_ROI_TOOL, &PyGvRoiTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_ROI_TOOL);
    pygobject_register_class(d, "GvRectTool", GV_TYPE_RECT_TOOL, &PyGvRectTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_RECT_TOOL);
    pygobject_register_class(d, "GvPointTool", GV_TYPE_POINT_TOOL, &PyGvPointTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_POINT_TOOL);
    pygobject_register_class(d, "GvPoiTool", GV_TYPE_POI_TOOL, &PyGvPoiTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_POI_TOOL);
    pygobject_register_class(d, "GvNodeTool", GV_TYPE_NODE_TOOL, &PyGvNodeTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_NODE_TOOL);
    pygobject_register_class(d, "GvLineTool", GV_TYPE_LINE_TOOL, &PyGvLineTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_LINE_TOOL);
    pygobject_register_class(d, "GvAreaTool", GV_TYPE_AREA_TOOL, &PyGvAreaTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_AREA_TOOL);
    pygobject_register_class(d, "GvToolbox", GV_TYPE_TOOLBOX, &PyGvToolbox_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_TOOLBOX);
    pygobject_register_class(d, "GvTrackTool", GV_TYPE_TRACK_TOOL, &PyGvTrackTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_TRACK_TOOL);
    pygobject_register_class(d, "GvViewArea", GV_TYPE_VIEW_AREA, &PyGvViewArea_Type, Py_BuildValue("(O)", &PyGtkDrawingArea_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_VIEW_AREA);
    pygobject_register_class(d, "GvViewLink", GV_TYPE_VIEW_LINK, &PyGvViewLink_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_VIEW_LINK);
    pygobject_register_class(d, "GvZoompanTool", GV_TYPE_ZOOMPAN_TOOL, &PyGvZoompanTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_ZOOMPAN_TOOL);
    pygobject_register_class(d, "GvAutopanTool", GV_TYPE_AUTOPAN_TOOL, &PyGvAutopanTool_Type, Py_BuildValue("(O)", &PyGvTool_Type));
    pyg_set_object_has_new_constructor(GV_TYPE_AUTOPAN_TOOL);
}
