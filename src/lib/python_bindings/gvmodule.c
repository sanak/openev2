/******************************************************************************
 * $Id: gvmodule.c,v 1.2 2005/04/25 20:41:52 uid1018 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Hand generated python bindings for OpenEV C functions.
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
 * $Log: gvmodule.c,v $
 * Revision 1.2  2005/04/25 20:41:52  uid1018
 * Modified macro to exclude thumbnails on all platforms
 *
 * Revision 1.1.1.1  2005/04/18 16:38:34  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:34  uid1026
 *
 * Imported sources
 *
 * Revision 1.115  2005/01/14 16:52:02  gmwalter
 * Checked in Aude's gv_shapes_add_shape_last function
 * (allows shapes to be added without repeating
 * indices if others have been deleted).
 *
 * Revision 1.114  2005/01/04 18:50:33  gmwalter
 * Checked in Aude's new gvshape function changes.
 *
 * Revision 1.113  2004/09/28 19:45:22  warmerda
 * slight change in PyProxy code to avoid uninit data reference
 *
 * Revision 1.112  2004/08/18 20:55:21  pgs
 * added ability to return columns of data in a python dictionary
 *
 * Revision 1.111  2004/06/23 14:35:17  gmwalter
 * Added support for multi-band complex imagery.
 *
 * Revision 1.110  2004/05/12 10:00:27  dem
 *
 * Fix a bug in _wrap_gv_raster_layer_get_mesh_lod : this wrapper created
 * a python double from a C int and then the mesh_lod was wrong (example 223.454).
 * The wrapper now returns an int.
 * This bug crashed OpenEV when for example we save a project, then reload it :
 * if the wrong mesh is big on a lot of images of the project, this uses a lot of
 * memory and done an allocation error...
 * Note that all projects saved until this fix have wrong mesh_lods.
 *
 * Revision 1.109  2004/02/12 22:36:08  gmwalter
 * Add functions for easily creating a line from three
 * lists of nodes (x,y,z), avoiding python-level for
 * loop.
 *
 * Revision 1.108  2004/02/10 15:48:56  andrey_kiselev
 * Added wrapper for gv_manager_add_dataset() function.
 *
 * Revision 1.107  2004/01/22 20:47:43  andrey_kiselev
 * Added wrappers for gv_raster_layer_nodata_set(), gv_raster_layer_nodata_get()
 * and gv_raster_layer_type_get().
 *
 * Revision 1.106  2003/09/02 17:29:10  warmerda
 * added get_names() method
 *
 * Revision 1.105  2003/08/29 20:52:43  warmerda
 * added to/from xml translation for GvShape
 *
 * Revision 1.104  2003/08/23 04:02:39  warmerda
 * added gv_records_recode
 *
 * Revision 1.103  2003/08/20 20:04:18  warmerda
 * Added collection methods on GvShape
 *
 * Revision 1.102  2003/08/08 18:10:49  warmerda
 * added gv_ciet.c
 *
 * Revision 1.101  2003/08/06 22:23:14  warmerda
 * added progress monitor to gv_records load/save funcs
 *
 * Revision 1.100  2003/08/06 17:17:53  warmerda
 * gv_records_to_dbf() now supports passing in a list of selected items to write.
 * gv_records_get_typed_properties() and gv_records_get_properties() will no
 * longer put NULL properties in the dictionary.
 *
 * Revision 1.99  2003/08/05 15:25:22  warmerda
 * fixed formatting for integer fields when fetching from GvRecords
 *
 * Revision 1.98  2003/07/27 05:00:47  warmerda
 * fleshed out gvrecords support
 *
 * Revision 1.97  2003/05/23 16:18:18  warmerda
 * added GvRecords for CIETMap
 *
 * Revision 1.96  2003/04/08 18:11:40  andrey_kiselev
 * FAdded missed return value in _wrap_gv_symbol_manager_save_vector_symbol()
 *
 * Revision 1.95  2003/04/08 11:58:33  andrey_kiselev
 * Added wrapper for gv_symbol_manager_save_vector_symbol() function.
 *
 * Revision 1.94  2003/04/02 15:46:59  pgs
 * added wrapper for gv_format_point_query
 *
 * Revision 1.93  2003/03/07 22:24:47  warmerda
 * added example MyGDALOperator for Diana
 *
 * Revision 1.92  2003/03/02 17:06:24  warmerda
 * new symbolmanager support
 *
 * Revision 1.91  2003/02/20 19:27:20  gmwalter
 * Updated link tool to include Diana's ghost cursor code, and added functions
 * to allow the cursor and link mechanism to use different gcps
 * than the display for georeferencing.  Updated raster properties
 * dialog for multi-band case.  Added some signals to layerdlg.py and
 * oeattedit.py to make it easier for tools to interact with them.
 * A few random bug fixes.
 *
 * Revision 1.90  2003/01/06 21:39:50  warmerda
 * added gv_shapes_from_ogr_layer
 *
 * Revision 1.89  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.88  2002/09/11 20:40:50  warmerda
 * fixed so that 3d views can be saved
 *
 * Revision 1.87  2002/07/29 21:01:31  warmerda
 * return missing properties as None
 *
 * Revision 1.86  2002/07/24 20:33:22  warmerda
 * added gv_shape_get_property
 *
 * Revision 1.85  2002/07/18 19:43:53  warmerda
 * added gv_shapes_get_typed_properties
 *
 * Revision 1.84  2002/07/18 19:34:40  pgs
 * added wrapper for gv_shapes_to_dbf
 *
 * Revision 1.83  2002/07/16 14:17:06  warmerda
 * added support for getting background color
 *
 * Revision 1.82  2002/03/07 18:31:56  warmerda
 * added preliminary gv_shape_clip_to_rect() implementation
 *
 * Revision 1.81  2002/02/28 18:52:22  gmwalter
 * Added a point-of-interest tool similar to the region-of-interest
 * tool (allows a user to select a temporary point without having to add a
 * new layer).  Added a mechanism to allow some customization of openev
 * via a textfile defining external modules.
 *
 * Revision 1.80  2002/01/18 05:48:13  warmerda
 * added GvShapes.get_extents() method in python
 *
 * Revision 1.79  2001/12/08 04:49:39  warmerda
 * added point in polygon test
 *
 * Revision 1.78  2001/11/28 19:18:30  warmerda
 * Added set_gcps(), and get_gcps() methods on GvRaster, and the
 * geotransform-changed signal generated when the gcps change.
 *
 * Revision 1.77  2001/11/07 15:20:38  warmerda
 * fixed other similar memory leaks
 *
 * Revision 1.76  2001/11/07 14:55:44  warmerda
 * fixed serious memory leak in gv_shape_get_properties() implementation
 *
 * Revision 1.75  2001/10/16 18:52:14  warmerda
 * added autoscale and histogram methods
 *
 * Revision 1.74  2001/09/17 15:31:23  pgs
 * removed extraneous initialization in _wrap_gtk_color_well_get_d
 *
 * Revision 1.73  2001/09/17 03:44:14  pgs
 * removed extra declaration from _wrap_gtk_color_well_get_d
 *
 * Revision 1.72  2001/09/16 04:40:55  warmerda
 * removed extra Py_INCREF call
 *
 * Revision 1.71  2001/09/16 03:29:10  pgs
 * added gtk_color_well_get_d binding.
 *
 * Revision 1.70  2001/09/14 14:22:01  warmerda
 * added GtkColorWell bindings
 *
 * Revision 1.69  2001/08/14 17:03:24  warmerda
 * added standard deviation autoscaling support
 *
 * Revision 1.68  2001/08/08 17:46:52  warmerda
 * added GvShape reference counting support
 *
 * Revision 1.67  2001/07/24 02:59:25  warmerda
 * added force_load method on GvRaster
 *
 * Revision 1.66  2001/07/13 22:13:35  warmerda
 * added function to get height from mesh
 *
 * Revision 1.65  2001/07/09 20:40:48  warmerda
 * added skirt prototype
 *
 * Revision 1.64  2001/04/23 18:51:51  warmerda
 * added set_properties() and __delattr__ to GvShape
 *
 * Revision 1.63  2001/04/22 17:35:25  pgs
 * added get_short_path_name and changed wid_interpolate to a variable exponent for d
 *
 * Revision 1.62  2001/04/02 18:10:46  warmerda
 * expose gv_raster_autoscale() to python
 *
 * Revision 1.61  2001/03/29 14:59:56  warmerda
 * added fill_short flag to control handling of slivers
 *
 * Revision 1.60  2001/03/29 04:49:36  warmerda
 * added gv_view_area_get_volume access
 *
 * Revision 1.59  2001/03/21 04:32:04  warmerda
 * fixed out-of-range __getitem__ on GvShape
 *
 * Revision 1.58  2001/01/30 15:18:01  warmerda
 * added queue_task() access from python
 *
 * Revision 1.57  2000/10/06 16:48:56  warmerda
 * added GvViewArea background color
 *
 * Revision 1.56  2000/09/29 16:09:20  srawlin
 * added Goto function requring fuction to map lat/long to view coordinates
 *
 * Revision 1.55  2000/09/29 04:27:58  warmerda
 * fix type of nodata arguments
 *
 * Revision 1.54  2000/09/29 00:59:46  warmerda
 * take care to return None from GvShapes.__getitem__ on deleted shapes
 *
 * Revision 1.53  2000/09/21 03:01:09  warmerda
 * added gv_data_set_properties
 *
 * Revision 1.52  2000/09/15 15:12:45  warmerda
 * added gv_shape_destroy
 *
 * Revision 1.51  2000/09/15 01:30:02  warmerda
 * added gv_raster_rasterize_shapes cover
 *
 * Revision 1.50  2000/09/12 19:19:33  warmerda
 * added WIDInterpolate
 *
 * Revision 1.49  2000/08/25 20:14:31  warmerda
 * added appcurlayer, and raster layer nodata support
 *
 * Revision 1.48  2000/07/31 21:15:50  srawlin
 * added functions in GvRaster and GvShapes to convert a C change_info struct into a Python tuple
 *
 * Revision 1.47  2000/07/27 20:06:23  warmerda
 * added boundary constraints
 *
 * Revision 1.46  2000/07/25 17:04:49  warmerda
 * Fixed bug with reference leak in gv_view_area_list_layers().
 *
 * Revision 1.45  2000/07/24 14:39:19  warmerda
 * added GvRasterLayer pixel_to_view, view_to_pixel methods
 *
 * Revision 1.44  2000/07/20 19:21:24  warmerda
 * fixed delete_shapes bug for Ahmed
 *
 * Revision 1.43  2000/07/13 19:17:12  warmerda
 * added gv_shape_copy, gv_shapes_replace_shapes
 *
 * Revision 1.41  2000/07/11 20:54:59  srawlin
 * added GvViewArea methods to get and set viewing direction relative to z-plane in 3D
 *
 * Revision 1.40  2000/07/11 18:24:38  warmerda
 * added shape deletion
 *
 * Revision 1.39  2000/07/03 20:59:05  warmerda
 * added some 3d view methods
 *
 * Revision 1.38  2000/06/30 18:05:19  srawlin
 * added ability to set ROI constraints
 *
 * Revision 1.37  2000/06/29 16:13:28  srawlin
 * added ROI Tool creation function
 *
 * Revision 1.36  2000/06/26 15:14:22  warmerda
 * added GvManager dataset support
 *
 * Revision 1.35  2000/06/23 12:57:32  warmerda
 * added GvRasterSource support
 *
 * Revision 1.34  2000/06/20 13:39:06  warmerda
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

#define PyGtk_New pygobject_new
#define PyGtk_Get pygobject_get

#include "gview.h"
#include "gvutils.h"
#include "invdistance.h"
#include "gvrasterize.h"
#include "gtkcolorwell.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "gvtypes.h"         // define GV_USE_DOUBLE_PRECISION_COORD

// SD select float or double
#ifdef GV_USE_DOUBLE_PRECISION_COORD
#
#   define Ccast   "d"
#   define CC      "dd"
#   define CCC     "ddd"
#   define CCCC    "dddd"
#   define CCCCCC  "dddddd"
#
#else
#
#   define Ccast   "f"
#   define CC      "ff"
#   define CCC     "fff"
#   define CCCC    "ffff"
#   define CCCCCC  "ffffff"
#
#endif



GvLayer *gv_build_skirt( GvRasterLayer *, double base_z );

/*
 * This is a function for extracting a raw pointer from a SWIG pointer
 * string.
 */
static
void *SWIG_SimpleGetPtr(char *_c, char *_t)
{
  unsigned long _p;
  if( _c == NULL || _c[0] != '_' )
      return NULL;

  if( _t != NULL && strstr(_c,_t) == NULL )
      return NULL;

  _c++;
  /* Extract hex value from pointer */
  _p = 0;
  while (*_c) {
      if ((*_c >= '0') && (*_c <= '9'))
          _p = (_p << 4) + (*_c - '0');
      else if ((*_c >= 'a') && (*_c <= 'f'))
          _p = (_p << 4) + ((*_c - 'a') + 10);
      else
          break;
      _c++;
  }

  return (void *) _p;
}

static
void SWIG_SimpleMakePtr(char *_c, const void *_ptr, char *type) {
  static char _hex[16] =
  {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
   'a', 'b', 'c', 'd', 'e', 'f'};
  unsigned long _p, _s;
  char _result[20], *_r;    /* Note : a 64-bit hex number = 16 digits */
  _r = _result;
  _p = (unsigned long) _ptr;
  if (_p > 0) {
    while (_p > 0) {
      _s = _p & 0xf;
      *(_r++) = _hex[_s];
      _p = _p >> 4;
    }
    *_r = '_';
    while (_r >= _result)
      *(_c++) = *(_r--);
  } else {
    strcpy (_c, "NULL");
  }
  if (_ptr)
    strcpy (_c, type);
}

/*
** Stuff to support progress reporting callbacks.
*/

typedef struct {
    PyObject *psPyCallback;
    PyObject *psPyCallbackData;
    int nLastReported;
} PyProgressData;

/************************************************************************/
/*                          PyProgressProxy()                           */
/*                                                                      */
/*      Copied from gdal.i                                              */
/************************************************************************/

static int
PyProgressProxy( double dfComplete, const char *pszMessage, void *pData )

{
    PyProgressData *psInfo = (PyProgressData *) pData;
    PyObject *psArgs, *psResult;
    int      bContinue = TRUE;

    if( psInfo->psPyCallback == NULL || psInfo->psPyCallback == Py_None )
        return TRUE;

    if( psInfo->nLastReported == (int) (100.0 * dfComplete) )
        return TRUE;

    psInfo->nLastReported = (int) 100.0 * dfComplete;

    if( pszMessage == NULL )
        pszMessage = "";

    if( psInfo->psPyCallbackData == NULL )
        psArgs = Py_BuildValue("(dsO)", dfComplete, pszMessage, Py_None );
    else
        psArgs = Py_BuildValue("(dsO)", dfComplete, pszMessage,
                           psInfo->psPyCallbackData );

    psResult = PyEval_CallObject( psInfo->psPyCallback, psArgs);
    Py_XDECREF(psArgs);

    if( psResult == NULL )
    {
        return TRUE;
    }

    if( psResult == Py_None )
    {
    Py_XDECREF(Py_None);
        return TRUE;
    }

    if( !PyArg_Parse( psResult, "i", &bContinue ) )
    {
        PyErr_SetString(PyExc_ValueError, "bad progress return value");
    return FALSE;
    }

    Py_XDECREF(psResult);

    return bContinue;
}

/************************************************************************/
/*                          PyListToXMLTree()                           */
/************************************************************************/

static CPLXMLNode *PyListToXMLTree( PyObject *pyList )

{
    int      nChildCount = 0, iChild, nType;
    CPLXMLNode *psThisNode;
    CPLXMLNode *psChild;
    char       *pszText = NULL;

    nChildCount = PyList_Size(pyList) - 2;
    if( nChildCount < 0 )
    {
        PyErr_SetString(PyExc_TypeError,"Error in input XMLTree." );
    return NULL;
    }

    PyArg_Parse( PyList_GET_ITEM(pyList,0), "i", &nType );
    PyArg_Parse( PyList_GET_ITEM(pyList,1), "s", &pszText );
    psThisNode = CPLCreateXMLNode( NULL, (CPLXMLNodeType) nType, pszText );

    for( iChild = 0; iChild < nChildCount; iChild++ )
    {
        psChild = PyListToXMLTree( PyList_GET_ITEM(pyList,iChild+2) );
        CPLAddXMLChild( psThisNode, psChild );
    }

    return psThisNode;
}

/************************************************************************/
/*                          XMLTreeToPyList()                           */
/************************************************************************/

static PyObject *XMLTreeToPyList( CPLXMLNode *psTree )

{
    PyObject *pyList;
    int      nChildCount = 0, iChild;
    CPLXMLNode *psChild;

    for( psChild = psTree->psChild; 
         psChild != NULL; 
         psChild = psChild->psNext )
        nChildCount++;

    pyList = PyList_New(nChildCount+2);

    PyList_SetItem( pyList, 0, Py_BuildValue( "i", (int) psTree->eType ) );
    PyList_SetItem( pyList, 1, Py_BuildValue( "s", psTree->pszValue ) );

    for( psChild = psTree->psChild, iChild = 2; 
         psChild != NULL; 
         psChild = psChild->psNext, iChild++ )
    {
        PyList_SetItem( pyList, iChild, XMLTreeToPyList( psChild ) );
    }

    return pyList; 
}

/*
 * Functions not handled by the wrapper generator
 */

static PyObject *
_wrap_gv_shape_new(PyObject *self, PyObject *args)
{
    int          type;
    char         swig_ptr[32];
    GvShape *shape;

    if (!PyArg_ParseTuple(args, "i:gv_shape_new",
                          &type) )
    return NULL;

    shape = gv_shape_new(type);
    SWIG_SimpleMakePtr( swig_ptr, shape, "_GvShape" );

    return Py_BuildValue("s",swig_ptr);
}

static PyObject *
_wrap_gv_shape_from_xml(PyObject *self, PyObject *args)
{
    GvShape     *shape;
    CPLXMLNode  *cpl_tree;
    PyObject    *py_tree = NULL;

    if (!PyArg_ParseTuple(args, "O!:gv_shape_from_xml",
                          &PyList_Type, &py_tree ) )
        return NULL;

    cpl_tree = PyListToXMLTree( py_tree );
    
    shape = gv_shape_from_xml_tree( cpl_tree );
    if( shape == NULL )
    {
        PyErr_SetString( PyExc_ValueError, 
                         "XML translation to GvShape filed." );
        return NULL;
    }
    else
    {
        char         swig_ptr[32];
        SWIG_SimpleMakePtr( swig_ptr, shape, "_GvShape" );
        return Py_BuildValue("s",swig_ptr);
    }
}

static PyObject *
_wrap_gv_shape_to_xml(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    CPLXMLNode *psTree;
    PyObject *py_xml = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_to_xml",
                          &swig_shape_ptr))
        return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape == NULL )
        return NULL;

    psTree = gv_shape_to_xml_tree( shape );
    py_xml = XMLTreeToPyList( psTree );
    CPLDestroyXMLNode( psTree );

    return py_xml;
}

static PyObject *
_wrap_gv_shape_destroy(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_destroy",
                          &swig_shape_ptr))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        gv_shape_delete( shape );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_ref(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_ref",
                          &swig_shape_ptr))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        gv_shape_ref( shape );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_unref(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_unref",
                          &swig_shape_ptr))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        gv_shape_unref( shape );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_get_ref(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_get_ref",
                          &swig_shape_ptr))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
    {
        return Py_BuildValue( "i", gv_shape_get_ref( shape ) );
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static PyObject *
_wrap_gv_shape_copy(PyObject *self, PyObject *args)
{
    char  swig_ptr[32];
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL, *copy = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_copy",
                          &swig_shape_ptr))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        copy = gv_shape_copy( shape );

    SWIG_SimpleMakePtr( swig_ptr, copy, "_GvShape" );

    return Py_BuildValue("s",swig_ptr);
}

static PyObject *
_wrap_gv_shape_line_from_nodelists(PyObject *self, PyObject *args)
{
    PyObject *pyxlist=NULL;
    PyObject *pyylist=NULL;
    PyObject *pyzlist=NULL;

    int       node_count, i;
    char         swig_ptr[32];
    GvShape *shape;
    gvgeocoord xnode, ynode, znode;
    int ring=0;

    if (!PyArg_ParseTuple(args, "O!O!O!:gv_shape_line_from_nodelist",
                          &PyList_Type,&pyxlist,
                          &PyList_Type,&pyylist,
                          &PyList_Type,&pyzlist) )
    return NULL;

    node_count=PyList_Size(pyxlist);
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
        gv_shape_add_node(shape,ring,xnode,ynode,znode);
        
    }

    SWIG_SimpleMakePtr( swig_ptr, shape, "_GvShape" );

    return Py_BuildValue("s",swig_ptr);
}


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

    if (!PyArg_ParseTuple(args, "O!O!O!O!:gv_shape_lines_for_vecplot",
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
    last_ok=1;
    shape_count=1;
    last_shape_nodes=0;
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
            shape_ids=g_new(int,shape_count);
            for ( j= 0 ; j < shape_count; j++ )
            {
                *(shape_ids+sizeof(int)) = j;
            }
            gv_shapes_delete_shapes(shapes,shape_count,shape_ids);
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
            shape=gv_shape_new(GVSHAPE_LINE);
            gv_shapes_add_shape(shapes,shape);
            shape_count=shape_count+1;
            last_shape_nodes=0;
            last_ok = 0;
        } 
    }
    if (last_shape_nodes == 0)
    {
        shape_ids=g_new(int,1);
        *shape_ids=shape_count-1;
        gv_shapes_delete_shapes(shapes,1,shape_ids);
        g_free(shape_ids);
    }

    return PyGtk_New((GObject *) shapes);
}



static PyObject *
_wrap_gv_shape_get_property(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    char *key;
    GvShape *shape = NULL;
    const char *value = NULL;

    if (!PyArg_ParseTuple(args, "ss:gv_shape_get_property",
                          &swig_shape_ptr, &key))
        return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        value = gv_properties_get( gv_shape_get_properties( shape ), key );

    if( value != NULL )
        return Py_BuildValue( "s", value );
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static PyObject *
_wrap_gv_shape_get_properties(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    GvProperties *properties = NULL;
    PyObject *psDict = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_get_properties",
                          &swig_shape_ptr))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        properties = gv_shape_get_properties( shape );

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


/************************************************************************/
/*                      gv_shape_get_typed_properties()                 */
/*                                                                      */
/*      This function fetches a dictionary of property values for a     */
/*      given GvShape.  It operates similarly to the normal             */
/*      get_properties() call on the GvShape with the following         */
/*      changes:                                                        */
/*       o The list of fields to extract are passed in.                 */
/*       o The passed in field list includes an indicator of whether    */
/*         the field should be auto-converted to a numeric type.        */
/*                                                                      */
/*      The input argument (beside the shape) is a list of tuples       */
/*      for the field. The tuples consist of the name, and an           */
/*      integer flag indicating (non-zero) if the field should be       */
/*      converted to a numeric value.                                   */
/************************************************************************/

static PyObject *
_wrap_gv_shape_get_typed_properties(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    GvProperties *properties = NULL;
    PyObject *psDict = NULL;
    PyObject *pyFieldList = NULL;
    int      nCount, i;

    if (!PyArg_ParseTuple(args, "sO!:get_typed_properties",
                          &swig_shape_ptr, &PyList_Type, &pyFieldList))
        return NULL;

    if( swig_shape_ptr )
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );

    if( shape != NULL )
        properties = gv_shape_get_properties( shape );

    psDict = PyDict_New();
    if( properties == NULL )
        return psDict;

    nCount = PyList_Size(pyFieldList);
    for( i = 0; i < nCount; i++ )
    {
        char *pszFieldName = NULL;
        int nNumericFlag = 0;
        const char *value;
        PyObject *py_name, *py_value;

        if( !PyArg_Parse( PyList_GET_ITEM(pyFieldList,i), "(si)",
                          &pszFieldName, &nNumericFlag ) )
        {
            PyErr_SetString(PyExc_ValueError,
                            "expecting (name,flag) tuples in list.");
            return NULL;
        }

        value = gv_properties_get(properties,pszFieldName);
        if( value == NULL )
        {
            py_value = Py_None;
            Py_INCREF( Py_None );
        }
        else if( nNumericFlag )
            py_value = Py_BuildValue("f",atof(value));
        else
            py_value = Py_BuildValue("s",value);

        py_name = Py_BuildValue("s",pszFieldName);

        PyDict_SetItem( psDict, py_name, py_value );

        Py_DECREF(py_name);
        Py_DECREF(py_value);
    }

    return psDict;
}

static PyObject *
_wrap_gv_shape_set_property(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL, *name=NULL, *value=NULL;
    GvShape *shape = NULL;
    GvProperties *properties = NULL;

    if (!PyArg_ParseTuple(args, "sss:gv_shape_set_property",
                          &swig_shape_ptr, &name, &value))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
    {
        properties = gv_shape_get_properties( shape );
        gv_properties_set( properties, name, value );
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_set_properties(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    GvProperties *properties = NULL;
    PyObject *psDict = NULL;
    PyObject    *pyKey = NULL, *pyValue = NULL;

    if (!PyArg_ParseTuple(args, "sO!:gv_shape_set_properties",
                          &swig_shape_ptr, &PyDict_Type, &psDict))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
    {
        int i;

        properties = gv_shape_get_properties( shape );

        gv_properties_clear( properties );

        i = 0;
        while( PyDict_Next( psDict, &i, &pyKey, &pyValue ) )
        {
            char            *key = NULL, *value = NULL;

            if( !PyArg_Parse( pyKey, "s", &key )
                || !PyArg_Parse( pyValue, "s", &value ))
                continue;

            gv_properties_set( properties, key, value );

            pyKey = pyValue = NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_get_type(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_get_rings",
                          &swig_shape_ptr))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        return Py_BuildValue("i",gv_shape_type(shape));
    else
        return NULL;
}

static PyObject *
_wrap_gv_shape_get_rings(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_get_rings",
                          &swig_shape_ptr))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        return Py_BuildValue("i",gv_shape_get_rings(shape));
    else
        return NULL;
}

static PyObject *
_wrap_gv_shape_get_nodes(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    int      ring = 0;

    if (!PyArg_ParseTuple(args, "si:gv_shape_get_nodes",
                          &swig_shape_ptr, &ring))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        return Py_BuildValue("i",gv_shape_get_nodes(shape,ring));
    else
        return NULL;
}

static PyObject *
_wrap_gv_shape_add_node(PyObject *self, PyObject *args)
{
    char      *swig_shape_ptr = NULL;
    GvShape   *shape = NULL;
    int        ring = 0;
    gvgeocoord x=0.0, y=0.0, z=0.0;

    if (!PyArg_ParseTuple(args, "s" CCC "i:gv_shape_add_node", &swig_shape_ptr,
                          &x, &y, &z, &ring ))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        return Py_BuildValue("i",gv_shape_add_node(shape,ring, x,y,z));
    else
        return NULL;
}

static PyObject *
_wrap_gv_shape_set_node(PyObject *self, PyObject *args)
{
    char      *swig_shape_ptr = NULL;
    GvShape   *shape = NULL;
    int        ring = 0, node = 0;
    gvgeocoord x=0.0, y=0.0, z=0.0;

    if (!PyArg_ParseTuple(args, "s" CCC "ii:gv_shape_set_node", &swig_shape_ptr,
                          &x, &y, &z, &node, &ring ))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL )
        return Py_BuildValue("i",
                       gv_shape_set_xyz(shape, ring, node, x,y,z));
    else
        return NULL;
}

static PyObject *
_wrap_gv_shape_get_node(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    int      ring = 0, node = 0;

    if (!PyArg_ParseTuple(args, "sii:gv_shape_get_node",
                          &swig_shape_ptr, &node, &ring ))
    return NULL;

    if( swig_shape_ptr )
    {
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if( shape != NULL ){
        return Py_BuildValue("(" CCC ")",
                             gv_shape_get_x(shape,ring,node),
                             gv_shape_get_y(shape,ring,node),
                             gv_shape_get_z(shape,ring,node) );
    }else
        return NULL;
}

static PyObject *
_wrap_gv_shape_point_in_polygon(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    double  x, y;

    if (!PyArg_ParseTuple(args, "sdd:gv_shape_point_in_polygon",
                          &swig_shape_ptr, &x, &y ))
        return NULL;

    if( swig_shape_ptr )
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );

    if( shape != NULL )
        return Py_BuildValue("i", gv_shape_point_in_polygon(shape, x, y ));
    else
        return NULL;
}

static PyObject *
_wrap_gv_shape_distance_from_polygon(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    double  x, y;

    if (!PyArg_ParseTuple(args, "sdd:gv_shape_distance_from_polygon",
                          &swig_shape_ptr, &x, &y ))
        return NULL;

    if( swig_shape_ptr )
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );

    if( shape != NULL )
        return Py_BuildValue("d", gv_shape_distance_from_polygon(shape, x, y ));
    else
        return NULL;
}

static PyObject *
_wrap_gv_shape_clip_to_rect(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    double  x, y, width, height;

    if (!PyArg_ParseTuple(args, "sdddd:gv_shape_clip_to_rect",
                          &swig_shape_ptr, &x, &y, &width, &height ))
        return NULL;

    if( swig_shape_ptr )
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );

    if( shape != NULL )
    {
        GvRect      rect;
        GvShape *new_shape;

        rect.x = x;
        rect.y = y;
        rect.width = width;
        rect.height = height;

        new_shape = gv_shape_clip_to_rect( shape, &rect );

        if( new_shape == NULL )
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        else
        {
            char swig_ptr[128];

            SWIG_SimpleMakePtr( swig_ptr, new_shape, "_GvShape" );

            return Py_BuildValue("s",swig_ptr);
        }
    }
    else
        return NULL;
}

static PyObject *
_wrap_gv_shape_add_shape(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    char *swig_sub_shape_ptr = NULL;
    GvShape *shape = NULL, *sub_shape = NULL;

    if (!PyArg_ParseTuple(args, "ss:gv_shape_add_shape",
                          &swig_shape_ptr, &swig_sub_shape_ptr ))
        return NULL;

    if( swig_shape_ptr )
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );

    if( swig_sub_shape_ptr )
        sub_shape = SWIG_SimpleGetPtr( swig_sub_shape_ptr, "_GvShape" );

    if( shape != NULL && sub_shape != NULL )
        gv_shape_collection_add_shape( shape, sub_shape );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_get_shape(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;
    int shape_index;

    if (!PyArg_ParseTuple(args, "si:gv_shape_get_shape",
                          &swig_shape_ptr, &shape_index ))
        return NULL;

    if( swig_shape_ptr )
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );

    if( shape == NULL )
    {
        PyErr_SetString(PyExc_ValueError, "no shape passed into get_shape().");
        return NULL;
    }

    shape = gv_shape_collection_get_shape( shape, shape_index );
    if( shape == NULL )
    {
        PyErr_SetString(PyExc_IndexError, "shape index out of range for collection");
        return NULL;
    }
    else
    {
        char swig_ptr[128];
        
        SWIG_SimpleMakePtr( swig_ptr, shape, "_GvShape" );
        
        return Py_BuildValue("s",swig_ptr);
    }
}

static PyObject *
_wrap_gv_shape_collection_get_count(PyObject *self, PyObject *args)
{
    char *swig_shape_ptr = NULL;
    GvShape *shape = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shape_get_shape",
                          &swig_shape_ptr ))
        return NULL;

    if( swig_shape_ptr )
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );

    if( shape == NULL )
    {
        PyErr_SetString(PyExc_ValueError, "no shape passed into get_shape().");
        return NULL;
    }

    return Py_BuildValue( "i", gv_shape_collection_get_count( shape ) );
}

static PyObject *
_wrap_gv_symbol_manager_get_names(PyObject *self, PyObject *args)
{
    PyObject *py_manager, *py_name_list;
    GvSymbolManager *manager;
    char **name_list;
    int i, count;

    if (!PyArg_ParseTuple(args, "O:gv_symbol_manager_get_names", &py_manager )) {
        return NULL;
    }

    manager = GV_SYMBOL_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_SYMBOL_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "manager argument must be a GvSymbolManager");
	return NULL;
    }
    name_list = gv_symbol_manager_get_names(manager);
    
    count = CSLCount( name_list );
    py_name_list = PyList_New( count );
    for( i = 0; i < count; i++ )
        PyList_SetItem( py_name_list, i, Py_BuildValue( "s", name_list[i] ) );
    
    g_free( name_list );

    return py_name_list;
}

static PyObject *
_wrap_gv_symbol_manager_inject_vector_symbol(PyObject *self, PyObject *args)
{
    PyObject *py_manager;
    GvSymbolManager *manager;
    GvShape  *shape = NULL;
    char     *swig_shape_ptr = NULL;
    char     *symbol_name = NULL;

    if (!PyArg_ParseTuple(args, "Oss:gv_symbol_manager_inject_vector_symbol",
                          &py_manager, &symbol_name, &swig_shape_ptr )) {
        return NULL;
    }

    manager = GV_SYMBOL_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_SYMBOL_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "manager argument must be a GvSymbolManager");
	return NULL;
    }

    if( swig_shape_ptr )
        shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );

    if( shape ) {
        gv_symbol_manager_inject_vector_symbol(manager, symbol_name, shape);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_symbol_manager_inject_raster_symbol(PyObject *self, PyObject *args)
{
    PyObject *py_manager;
    GvSymbolManager *manager;
    char     *rgba_string = NULL;
    char     *symbol_name = NULL;
    int      width, height, rgba_len;

    if (!PyArg_ParseTuple(args,
                          "Osiiz#:gv_symbol_manager_inject_raster_symbol",
                          &py_manager, &symbol_name,
                          &width, &height, &rgba_string, &rgba_len )) {
        return NULL;
    }

    manager = GV_SYMBOL_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_SYMBOL_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "manager argument must be a GvSymbolManager");
	return NULL;
    }

    if( width*height*4 > rgba_len )
    {
        PyErr_SetString(PyExc_TypeError,
                        "rgba raster symbol buffer seems to be too small (width*height*4)\nin gv_symbol_manager_inject_raster_symbol()." );
        return NULL;
    }

    gv_symbol_manager_inject_raster_symbol(manager, symbol_name,
					   width, height, rgba_string );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_symbol_manager_get_symbol(PyObject *self, PyObject *args)
{
    PyObject *py_manager;
    GvSymbolManager *manager;
    char     *symbol_name = NULL;
    GvSymbolObj *symbol;

    if (!PyArg_ParseTuple(args,"Os:gv_symbol_manager_get_symbol",
                          &py_manager, &symbol_name ) )
        return NULL;

    manager = GV_SYMBOL_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_SYMBOL_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "manager argument must be a GvSymbolManager");
	return NULL;
    }

    symbol = gv_symbol_manager_get_symbol(manager, symbol_name );

    if( symbol == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if( symbol->type == GV_SYMBOL_VECTOR )
    {
        char      swig_shape_ptr[32];

        SWIG_SimpleMakePtr( swig_shape_ptr, symbol->buffer, "_GvShape" );

        return Py_BuildValue("(is)", symbol->type, swig_shape_ptr );
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

static PyObject *
_wrap_gv_symbol_manager_save_vector_symbol(PyObject *self, PyObject *args)
{
    PyObject *py_manager;
    GvSymbolManager *manager;
    char     *symbol_name = NULL, *new_name = NULL;

    if (!PyArg_ParseTuple(args, "Oss:gv_symbol_manager_save_vector_symbol",
                          &py_manager, &symbol_name, &new_name )) {
        return NULL;
    }

    manager = GV_SYMBOL_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_SYMBOL_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "manager argument must be a GvSymbolManager");
	return NULL;
    }

    if( symbol_name && new_name )
    {
        if (gv_symbol_manager_save_vector_symbol(manager, symbol_name, new_name))
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        else
        {
            PyErr_SetString(PyExc_TypeError,
                "error while saving new symbol in gv_symbol_manager_save_vector_symbol()." );
        return NULL;
        }
    }

    return NULL;
}

static PyObject *
_wrap_gv_data_get_properties(PyObject *self, PyObject *args)
{
    PyObject *py_data;
    GvData *data;
    GvProperties *properties = NULL;
    PyObject *psDict = NULL;

    if (!PyArg_ParseTuple(args, "O:gv_data_get_properties", &py_data)) {
	return NULL;
    }

    data = GV_DATA(PyGtk_Get(py_data));
    if (!GV_IS_DATA(data)) {
	PyErr_SetString(PyExc_TypeError, "data argument must be a GvData object");
	return NULL;
    }

    properties = gv_data_get_properties(data);

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

static PyObject *
_wrap_gv_data_set_properties(PyObject *self, PyObject *args)
{
    PyObject *py_data;
    GvData *data;
    GvProperties *properties = NULL;
    PyObject *psDict = NULL;
    int      i;
    PyObject    *pyKey = NULL, *pyValue = NULL;

    if (!PyArg_ParseTuple(args, "OO!:gv_data_set_properties",
                          &py_data, &PyDict_Type, &psDict)) {
	return NULL;
    }

    data = GV_DATA(PyGtk_Get(py_data));
    if (!GV_IS_DATA(data)) {
	PyErr_SetString(PyExc_TypeError, "data argument must be a GvData object");
	return NULL;
    }

    properties = gv_data_get_properties(data);

    gv_properties_clear( properties );

    i = 0;
    while( PyDict_Next( psDict, &i, &pyKey, &pyValue ) )
    {
        char            *key = NULL, *value = NULL;

        if( !PyArg_Parse( pyKey, "s", &key )
            || !PyArg_Parse( pyValue, "s", &value ))
            continue;

        gv_properties_set( properties, key, value );

        pyKey = pyValue = NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_data_changed(PyObject *self, PyObject *args)
{
    PyObject *py_data;
    GvData *data;

    if (!PyArg_ParseTuple(args, "O:gv_data_changed", &py_data)) {
	return NULL;
    }

    data = GV_DATA(PyGtk_Get(py_data));
    if (!GV_IS_DATA(data)) {
	PyErr_SetString(PyExc_TypeError, "data argument must be a GvData object");
	return NULL;
    }

    gv_data_changed(data, NULL);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_layer_extents(PyObject *self, PyObject *args)
{
    PyObject *py_layer;
    GvLayer *layer;
    GvRect rect;

    if (!PyArg_ParseTuple(args, "O:gv_layer_extents", &py_layer)) {
	return NULL;
    }

    layer = GV_LAYER(PyGtk_Get(py_layer));
    if (!GV_IS_LAYER(layer)) {
	PyErr_SetString(PyExc_TypeError, "layer argument must be a GvLayer");
	return NULL;
    }

    gv_layer_extents(layer, &rect);

    return Py_BuildValue( "(" CCCC ")", rect.x, rect.y, rect.width, rect.height);
}

static PyObject *
_wrap_gv_layer_display_change(PyObject *self, PyObject *args)
{
    PyObject *py_layer;
    GvLayer *layer;

    if (!PyArg_ParseTuple(args, "O:gv_layer_display_change", &py_layer)) {
	return NULL;
    }

    layer = GV_LAYER(PyGtk_Get(py_layer));
    if (!GV_IS_LAYER(layer)) {
	PyErr_SetString(PyExc_TypeError, "layer argument must be a GvLayer");
	return NULL;
    }

    gv_layer_display_change(layer, NULL );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_area_list_layers(PyObject *self, PyObject *args)
{
    PyObject *py_view, *py_list;
    GvViewArea *view;
    GList *list;

    if (!PyArg_ParseTuple(args, "O:gv_view_area_list_layers", &py_view)) {
        return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }
    
    py_list = PyList_New(0);
    for (list = gv_view_area_list_layers(view); list != NULL; list = list->next)
    {
        PyObject *layer = PyGtk_New(G_OBJECT(list->data));
        PyList_Append(py_list, layer);
        Py_DECREF(layer);
    }
    g_list_free(list);
    return py_list;
}

static PyObject *
_wrap_gv_view_area_get_fontnames(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    GPtrArray *g_list;
    PyObject  *py_list;
    int       i;

    if (!PyArg_ParseTuple(args, "O:gv_view_area_get_fontnames", &py_view)) {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }

    g_list = gv_view_area_get_fontnames(view);

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

static PyObject *
_wrap_gv_view_area_set_background_color(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    GvColor color;

    if (!PyArg_ParseTuple(args, "O(ffff):gv_view_area_set_background_color",
              &py_view, &color[0], &color[1], &color[2], &color[3])) {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }

    gv_view_area_set_background_color(view, color);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_area_get_background_color(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    GvColor color;

    if (!PyArg_ParseTuple(args, "O:gv_view_area_get_background_color",
			  &py_view)) {
        return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }

    gv_view_area_get_background_color(view, color);

    return Py_BuildValue("(ffff)", color[0], color[1], color[2], color[3] );
}

static PyObject *
_wrap_gv_view_area_create_thumbnail(PyObject *self, PyObject *args)
{
#ifndef PENDING_GTK2
    PyErr_SetString(PyExc_RuntimeError, "not supported on this platform");
    return NULL;
#else

    PyObject *py_view;
    GvViewArea *view;
    PyObject *py_layer, *ret;
    GvLayer *layer;
    GdkPixmap *pixmap;
    int width, height;

    if (!PyArg_ParseTuple(args, "OOii:gv_view_area_create_thumbnail",
			  &py_view, &py_layer, &width, &height)) {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }
    layer = GV_LAYER(PyGtk_Get(py_layer));
    if (!GV_IS_LAYER(layer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvLayer");
	return NULL;
    }


    pixmap = gv_view_area_create_thumbnail(view, layer, width, height);
    if (!pixmap) {
        PyErr_SetString(PyExc_RuntimeError, "could not create pixmap");
        return NULL;
    }
    ret = PyGdkWindow_New(pixmap);
    gdk_pixmap_unref(pixmap);
    return ret;
#endif
}

static PyObject *
_wrap_gv_view_area_set_3d_view(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    vec3_t     eye_pos, eye_dir;

    if (!PyArg_ParseTuple(args,  "O(" CCC ")(" CCC "):gv_view_area_set_3d_view",
                          &py_view,
                          eye_pos+0, eye_pos+1, eye_pos+2,
                          eye_dir+0, eye_dir+1, eye_dir+2))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }

    gv_view_area_set_3d_view( view, eye_pos, eye_dir );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_area_set_3d_view_look_at(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    vec3_t     eye_pos;
    gvgeocoord eye_look_at[2];

    if (!PyArg_ParseTuple(args, "O(" CCC ")(" CC "):gv_view_area_set_3d_view_look_at",
                          &py_view,
                          eye_pos+0, eye_pos+1, eye_pos+2,
                          eye_look_at+0, eye_look_at+1))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }

    gv_view_area_set_3d_view_look_at( view, eye_pos, eye_look_at );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_view_area_get_eye_pos(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;

    if (!PyArg_ParseTuple(args, "O:gv_view_area_get_eye_pos", &py_view))
    {
    return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }

    return Py_BuildValue( "(" CCC ")",
                          view->state.eye_pos[0],
                          view->state.eye_pos[1],
                          view->state.eye_pos[2] );
}

static PyObject *
_wrap_gv_view_area_get_eye_dir(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;

    if (!PyArg_ParseTuple(args, "O:gv_view_area_get_eye_dir", &py_view))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }

    return Py_BuildValue( "(" CCC ")",
                          view->state.eye_dir[0],
                          view->state.eye_dir[1],
                          view->state.eye_dir[2] );
}


static PyObject *
_wrap_gv_view_area_get_look_at_pos(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    gvgeocoord  x, y;

    if (!PyArg_ParseTuple(args, "O:gv_view_area_get_look_at_pos", &py_view))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }

    if (!gv_view_area_get_look_at_pos( view, &x, &y))
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return Py_BuildValue("(" CC ")", x, y);

}

static PyObject *
_wrap_gv_view_area_map_location(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    gvgeocoord  x, y;

    if (!PyArg_ParseTuple(args, "O(" CC "):gv_view_area_map_location",
                          &py_view, &x, &y))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }

    gv_view_area_map_location( view, x, y, &x, &y );

    return Py_BuildValue("(" CC ")", x, y );
}

static PyObject *
_wrap_gv_view_area_get_pointer(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    gvgeocoord  x, y;

    if (!PyArg_ParseTuple(args, "O:gv_view_area_get_pointer", &py_view))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }
    gv_view_area_map_pointer( view,
                              view->state.mpos_x, view->state.mpos_y,
                              &x, &y );

    return Py_BuildValue("(" CC ")", x, y );
}

static PyObject *
_wrap_gv_view_area_map_pointer(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    gvgeocoord  x, y;

    if (!PyArg_ParseTuple(args, "O(" CC "):gv_view_area_map_pointer",
			  &py_view, &x, &y))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }
    gv_view_area_map_pointer( view, x, y, &x, &y );

    return Py_BuildValue("(" CC ")", x, y );
}

static PyObject *
_wrap_gv_view_area_inverse_map_pointer(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    gvgeocoord x, y;

    if (!PyArg_ParseTuple(args, "O(" CC "):gv_view_area_inverse_map_pointer",
                          &py_view, &x, &y))
    {
    return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }
    gv_view_area_inverse_map_pointer( view, x, y, &x, &y );

    return Py_BuildValue("(" CC ")", x, y );
}

static PyObject *
_wrap_gv_view_area_get_translation(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;

    if (!PyArg_ParseTuple(args, "O:gv_view_area_get_translation", &py_view))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }
    return Py_BuildValue( "(" CC ")", view->state.tx, view->state.ty );
}

static PyObject *
_wrap_gv_view_area_get_extents(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    gvgeocoord xmin, ymin, xmax, ymax;

    if (!PyArg_ParseTuple(args, "O:gv_view_area_get_extents", &py_view))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }
    gv_view_area_get_extents( view, &xmin, &ymin, &xmax, &ymax );

    return Py_BuildValue("(" CCCC ")", xmin, ymin, xmax, ymax );
}


static PyObject *
_wrap_gv_view_area_get_volume(PyObject *self, PyObject *args)
{
    PyObject *py_view;
    GvViewArea *view;
    double  volume[6];

    if (!PyArg_ParseTuple(args, "O:gv_view_area_get_volume", &py_view))
    {
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea");
	return NULL;
    }
    gv_view_area_get_volume( view, volume );

    return Py_BuildValue("(" CCCCCC ")",
                         volume[0], volume[1],
                         volume[2], volume[3],
                         volume[4], volume[5] );
}


static PyObject *
_wrap_gv_tool_set_boundary(PyObject *self, PyObject *args)
{
    PyObject *py_tool;
    GvRect rect;
    GvTool *tool;

    if (!PyArg_ParseTuple(args, "O(" CCCC "):gv_tool_set_boundary",
                          &py_tool, &rect.x, &rect.y,
                          &rect.width, &rect.height)) {
        return NULL;
    }

    tool = GV_TOOL(PyGtk_Get(py_tool));
    if (!GV_IS_TOOL(tool)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvTool");
	return NULL;
    }

    if (!gv_tool_set_boundary(tool, &rect)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "invalid ROI constraining region specified, width or height <= 0.0");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_roi_tool_get_rect(PyObject *self, PyObject *args)
{
    PyObject *py_tool;
    GvRect rect;
    GvRoiTool *tool;

    if (!PyArg_ParseTuple(args, "O:gv_roi_tool_get_rect", &py_tool)) {
	return NULL;
    }

    tool = GV_ROI_TOOL(PyGtk_Get(py_tool));
    if (!GV_IS_ROI_TOOL(tool)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRoiTool");
	return NULL;
    }

    if (!gv_roi_tool_get_rect(tool, &rect)) {
        PyErr_SetString(PyExc_RuntimeError, "no ROI marked");
        return NULL;
    }

    return Py_BuildValue("(" CCCC ")", rect.x, rect.y, rect.width, rect.height);
}

static PyObject *
_wrap_gv_roi_tool_new_rect(PyObject *self, PyObject *args)
{
    PyObject *py_tool;
    GvRect rect;
    GvRoiTool *tool;

    if (!PyArg_ParseTuple(args, "O(" CCCC "):gv_roi_tool_new_rect", &py_tool,
			  &rect.x, &rect.y, &rect.width, &rect.height)) {
	return NULL;
    }

    tool = GV_ROI_TOOL(PyGtk_Get(py_tool));
    if (!GV_IS_ROI_TOOL(tool)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRoiTool");
	return NULL;
    }

    if (!gv_roi_tool_new_rect(tool, &rect)){
	PyErr_SetString(PyExc_RuntimeError, "invalid ROI specified");
	return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_poi_tool_get_point(PyObject *self, PyObject *args)
{
    PyObject *py_tool;
    GvVertex point;
    GvPoiTool *tool;

    if (!PyArg_ParseTuple(args, "O:gv_poi_tool_get_point", &py_tool)) {
	return NULL;
    }

    tool = GV_POI_TOOL(PyGtk_Get(py_tool));
    if (!GV_IS_POI_TOOL(tool)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvPoiTool");
	return NULL;
    }

    if (!gv_poi_tool_get_point(tool, &point)) {
	PyErr_SetString(PyExc_RuntimeError, "no POI marked");
	return NULL;
    }

    return Py_BuildValue("(" CC ")", point.x, point.y);
}

static PyObject *
_wrap_gv_poi_tool_new_point(PyObject *self, PyObject *args)
{
    PyObject *py_tool;
    GvVertex point;
    GvPoiTool *tool;

    if (!PyArg_ParseTuple(args, "O(" CC "):gv_poi_tool_new_point",
			  &py_tool, &point.x, &point.y)) {
	return NULL;
    }

    tool = GV_POI_TOOL(PyGtk_Get(py_tool));
    if (!GV_IS_POI_TOOL(tool)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvPoiTool");
	return NULL;
    }

    if (!gv_poi_tool_new_point(tool, &point)) {
        PyErr_SetString(PyExc_RuntimeError, "invalid POI specified");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_records_get_typed_properties(PyObject *self, PyObject *args)
{
    PyObject *py_records;
    int       shp_index = -1, iField, ii;
    GvRecords *records;
    const char *pachRecData;
    PyObject *psDict = NULL;

    if (!PyArg_ParseTuple(args, "Oi:gv_records_get_typed_properties",
                          &py_records, &shp_index)) {
        return NULL;
    }

    if( !GV_IS_RECORDS(PyGtk_Get(py_records)) )
        return NULL;

    records = GV_RECORDS(PyGtk_Get(py_records));

    if( shp_index < 0 || shp_index >= records->nRecordCount )
        return NULL;

    psDict = PyDict_New();
    pachRecData = gv_records_get_raw_record_data( records, shp_index );
    for( ii = 0; ii < records->nUsedFieldCount; ii++ )
    {
        PyObject *py_name, *py_value;
        const char *value;

        iField = records->panUsedFieldList[ii];

        value = pachRecData + records->panFieldOffset[iField];

        if( *value == GV_NULL_MARKER )
        {
            py_value = Py_None;
            Py_INCREF( Py_None );
        }
        else if( records->panFieldType[iField] == GV_RFT_INTEGER )
            py_value = Py_BuildValue("i",atoi(value));
        else if( records->panFieldType[iField] == GV_RFT_FLOAT )
            py_value = Py_BuildValue("f",atof(value));
        else
            py_value = Py_BuildValue("s",value);
        
        py_name = Py_BuildValue("s",records->papszFieldName[iField]);

        PyDict_SetItem( psDict, py_name, py_value );

        Py_DECREF(py_name);
        Py_DECREF(py_value);
    }

    return psDict;
}

static PyObject *
_wrap_gv_records_get_properties(PyObject *self, PyObject *args)
{
    PyObject *py_records;
    int       shp_index = -1, iField;
    GvRecords *records;
    const char *pachRecData;
    PyObject *psDict = NULL;

    if (!PyArg_ParseTuple(args, "Oi:gv_records_get_properties",
                          &py_records, &shp_index) )
        return NULL;

    if( !GV_IS_RECORDS(PyGtk_Get(py_records)) )
        return NULL;

    records = GV_RECORDS(PyGtk_Get(py_records));

    if( shp_index < 0 || shp_index >= records->nRecordCount )
        return NULL;

    psDict = PyDict_New();
    pachRecData = gv_records_get_raw_record_data( records, shp_index );
    for( iField = 0; iField < records->nFieldCount; iField++ )
    {
        PyObject *py_name, *py_value;
        const char *value;

        value = pachRecData + records->panFieldOffset[iField];
        if( *value == GV_NULL_MARKER )
            continue;

        py_value = Py_BuildValue("s",value);
        py_name = Py_BuildValue("s",records->papszFieldName[iField]);

        PyDict_SetItem( psDict, py_name, py_value );

        Py_DECREF(py_name);
        Py_DECREF(py_value);
    }

    return psDict;
}

static PyObject *
_wrap_gv_records_set_used_properties(PyObject *self, PyObject *args)
{
    PyObject *py_records;
    PyObject *py_list;
    GvRecords *records;
    int       nFieldCount, *panFieldList = NULL, i;

    if (!PyArg_ParseTuple(args, "OO!:gv_records_set_used_properties",
                          &py_records, &PyList_Type, &py_list) )
        return NULL;

    if( !GV_IS_RECORDS(PyGtk_Get(py_records)) )
        return NULL;

    records = GV_RECORDS(PyGtk_Get(py_records));
    
    nFieldCount = PyList_Size(py_list);
    panFieldList = g_new( int, nFieldCount );

    for( i = 0; i < nFieldCount; i++ )
    {
        if( !PyArg_Parse( PyList_GET_ITEM(py_list,i), "i", panFieldList + i ) )
        {
            PyErr_SetString(PyExc_ValueError,
                            "expecting ints in gv_records_set_used_fields argument");
            return NULL;
        }
    }

    gv_records_set_used_properties( records, nFieldCount, panFieldList );

    g_free( panFieldList );
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_records_to_dbf(PyObject *self, PyObject *args)
{
    PyObject *py_records;
    PyObject *py_list;
    char      *filename = NULL;
    GvRecords *records;
    int       nSelectionCount, *panSelectionList = NULL, i, result;
    PyProgressData sProgressInfo;

    sProgressInfo.psPyCallback = NULL;
    sProgressInfo.psPyCallbackData = NULL;

    if (!PyArg_ParseTuple(args, "OsO!OO:gv_records_to_dbf",
                          &py_records, &filename, 
                          &PyList_Type, &py_list,
                          &(sProgressInfo.psPyCallback),
                          &(sProgressInfo.psPyCallbackData) ) )
        return NULL;

    if( !GV_IS_RECORDS(PyGtk_Get(py_records)) )
        return NULL;

    records = GV_RECORDS(PyGtk_Get(py_records));

    nSelectionCount = PyList_Size(py_list);
    if( nSelectionCount != 0 )
        panSelectionList = g_new( int, nSelectionCount );

    for( i = 0; i < nSelectionCount; i++ )
    {
        if( !PyArg_Parse( PyList_GET_ITEM(py_list,i), "i", panSelectionList + i ) )
        {
            PyErr_SetString(PyExc_ValueError,
                            "expecting ints in gv_records_to_dbf argument");
            return NULL;
        }
    }

    result = gv_records_to_dbf( records, filename, 
                                nSelectionCount, panSelectionList,
                                PyProgressProxy, &sProgressInfo );

    g_free( panSelectionList );
    
    return Py_BuildValue("i", result );
}
    
#include "gv_ciet.c"

static PyObject *
_wrap_gv_shapes_get_shape(PyObject *self, PyObject *args)
{
    PyObject *py_shapes;
    GvShapes *shapes;
    int       shp_index = -1;
    char      swig_shape_ptr[32];
    GvShape   *gv_shape = NULL;

    if (!PyArg_ParseTuple(args, "Oi:gv_shapes_get_shape",
                          &py_shapes, &shp_index) ) {
        return NULL;
    }

    shapes = GV_SHAPES(PyGtk_Get(py_shapes));
    if (!GV_IS_SHAPES(shapes)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvShapes");
	return NULL;
    }

    if( shp_index >= 0 && shp_index < gv_shapes_num_shapes(shapes)) {
        gv_shape = gv_shapes_get_shape(shapes, shp_index);
    }

    if( gv_shape == NULL ) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    SWIG_SimpleMakePtr( swig_shape_ptr, gv_shape, "_GvShape" );

    return Py_BuildValue("s", swig_shape_ptr );
}

static PyObject *
_wrap_gv_shapes_from_ogr_layer(PyObject *self, PyObject *args)
{
    GvData    *data = NULL;
    char      *ogrlayer_in = NULL;
    void      *hLayer;
    PyObject *py_ret = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_shapes_from_ogr_layer",
                          &ogrlayer_in) )
        return NULL;

    hLayer = SWIG_SimpleGetPtr(ogrlayer_in, "OGRLayerH" );
    if( hLayer == NULL )
    {
        PyErr_SetString(PyExc_IOError,
                        "Unable to extract OGRLayerH handle in gv_shapes_from_ogr_layer()");
        return NULL;
    }

    data = gv_shapes_from_ogr_layer( hLayer );
    if( data == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    py_ret = PyGtk_New( G_OBJECT(data) );
    gtk_object_sink( GTK_OBJECT(data) );

    return py_ret;
}

static PyObject *
_wrap_gv_shapes_add_shape(PyObject *self, PyObject *args)
{
    PyObject *py_shapes;
    GvShapes *shapes;
    char     *swig_shape_ptr;
    int       shp_index = -1;
    GvShape   *gv_shape = NULL;

    if (!PyArg_ParseTuple(args, "Os:gv_shapes_add_shape", 
			  &py_shapes, &swig_shape_ptr)) {
	return NULL;
    }

    shapes = GV_SHAPES(PyGtk_Get(py_shapes));
    if (!GV_IS_SHAPES(shapes)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvShapes");
	return NULL;
    }

    if( swig_shape_ptr )
    {
        gv_shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if (gv_shape) {
        shp_index = gv_shapes_add_shape(shapes, gv_shape);
    }

    return Py_BuildValue("i", shp_index );
}

static PyObject *
_wrap_gv_shapes_add_shape_last(PyObject *self, PyObject *args)
{
    PyObject *py_shapes;
    GvShapes *shapes;
    char     *swig_shape_ptr;
    int       shp_index = -1;
    GvShape   *gv_shape = NULL;

    if (!PyArg_ParseTuple(args, "Os:gv_shapes_add_shape_last",
                          &py_shapes, &swig_shape_ptr) ) {
	return NULL;
    }

    shapes = GV_SHAPES(PyGtk_Get(py_shapes));
    if (!GV_IS_SHAPES(shapes)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvShapes");
	return NULL;
    }

    if( swig_shape_ptr ) {
        gv_shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
    }

    if (gv_shape) {
        shp_index = gv_shapes_add_shape_last(shapes, gv_shape);
    }

    return Py_BuildValue("i", shp_index );
}

static PyObject *
_wrap_gv_shapes_delete_shapes(PyObject *self, PyObject *args)
{
    PyObject *py_shapes;
    GvShapes *shapes = NULL;
    int       shape_count, ii;
    PyObject *pylist = NULL;
    int      *shape_ids = NULL;

    if (!PyArg_ParseTuple(args, "OO!:gv_shapes_delete_shapes",
                          &py_shapes, &PyList_Type, &pylist) ) {
	return NULL;
    }

    shapes = GV_SHAPES(PyGtk_Get(py_shapes));
    if (!GV_IS_SHAPES(shapes)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvShapes");
	return NULL;
    }

    shape_count = PyList_Size(pylist);
    shape_ids = g_new(int,shape_count);

    for( ii = 0; ii < shape_count; ii++ )
    {
        if( !PyArg_Parse( PyList_GET_ITEM(pylist,ii), "i", shape_ids + ii ) )
        {
        PyErr_SetString(PyExc_ValueError,
                       "expecting ints in gv_shapes_delete_shapes argument");
        return NULL;
        }
    }

    gv_shapes_delete_shapes( shapes, shape_count, shape_ids );

    g_free( shape_ids );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shapes_replace_shapes(PyObject *self, PyObject *args)
{
    PyObject *py_shapes;
    GvShapes *shapes = NULL;
    int       shape_count, i;
    PyObject *pyindex_list = NULL, *pyshape_list = NULL;
    GvShape **shape_list = NULL;
    int      *shape_ids = NULL;
    int      copy_flag = FALSE;

    if (!PyArg_ParseTuple(args, "OO!O!|d:gv_shapes_replace_shapes",
                          &py_shapes,
                          &PyList_Type, &pyindex_list,
                          &PyList_Type, &pyshape_list,
                          &copy_flag ) ) {
	return NULL;
    }

    shapes = GV_SHAPES(PyGtk_Get(py_shapes));
    if (!GV_IS_SHAPES(shapes)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvShapes");
	return NULL;
    }

    if( PyList_Size(pyindex_list) != PyList_Size(pyshape_list) )
    {
    PyErr_SetString(PyExc_RuntimeError,
          "Size of index & shape lists differ in gv_shapes_replace_shapes().");
    return NULL;
    }

    shape_count = PyList_Size(pyindex_list);
    shape_ids = g_new(int,shape_count);
    shape_list = g_new(GvShape*,shape_count);

    for( i = 0; i < shape_count; i++ )
    {
        char   *shape_swigid = NULL;

         if( !PyArg_Parse( PyList_GET_ITEM(pyindex_list,i), "i", shape_ids+i ))
        {
        PyErr_SetString(PyExc_ValueError,
                       "expecting ints in gv_shapes_replace_shapes argument");
        return NULL;
        }

        if( !PyArg_Parse( PyList_GET_ITEM(pyshape_list,i), "s",&shape_swigid ))
        {
        PyErr_SetString(PyExc_ValueError,
                    "expecting GvShape in gv_shapes_replace_shapes argument");
        return NULL;
        }

        shape_list[i] = SWIG_SimpleGetPtr( shape_swigid, "_GvShape" );
    }


    gv_shapes_replace_shapes( shapes, shape_count, shape_ids, shape_list,
                              copy_flag );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shapes_get_extents(PyObject *self, PyObject *args)
{
    PyObject *py_shapes;
    GvRect rect;
    GvShapes *shapes;

    if (!PyArg_ParseTuple(args, "O:gv_shapes_extents", &py_shapes)) {
        return NULL;
    }

    shapes = GV_SHAPES(PyGtk_Get(py_shapes));
    if (!GV_IS_SHAPES(shapes)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvShapes");
	return NULL;
    }

    gv_shapes_get_extents(shapes, &rect);

    return Py_BuildValue("(" CCCC ")", rect.x, rect.y, rect.width, rect.height);
}

static PyObject *
_wrap_gv_shapes_get_change_info(PyObject *self, PyObject *args)
{
    GvShapeChangeInfo *change_info;
    PyObject *c_change_info;
    PyObject *id_list;
    int i;

    if (!PyArg_ParseTuple(args, "O:gv_shapes_get_change_info", &c_change_info))
    return NULL;

    if (!PyCObject_Check (c_change_info))
        return NULL;

    change_info = (GvShapeChangeInfo *) PyCObject_AsVoidPtr(c_change_info);

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

static PyObject *
_wrap_gv_points_get_point(PyObject *self, PyObject *args)
{
    PyObject *py_points;
    GvPoints *points;
    GvPoint *point;
    int index;

    if (!PyArg_ParseTuple(args, "Oi:gv_points_get_point",
			  &py_points, &index)) {
        return NULL;
    }

    points = GV_POINTS(PyGtk_Get(py_points));
    if (!GV_IS_POINTS(points)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvPoints");
	return NULL;
    }

    if (index < 0 || index >= gv_points_num_points(points))
    {
        PyErr_SetString(PyExc_IndexError, "point index out of range");
        return NULL;
    }
    point = gv_points_get_point(points, index);
    return Py_BuildValue("(" CC ")", point->v.x, point->v.y);
}

static PyObject *
_wrap_gv_points_new_point(PyObject *self, PyObject *args)
{
    PyObject *py_points;
    GvVertex vertex;
    GvPoints *points;

    if (!PyArg_ParseTuple(args, "O(" CC "):gv_points_new_point",
			  &py_points, &vertex.x, &vertex.y)) {
	return NULL;
    }

    points = GV_POINTS(PyGtk_Get(py_points));
    if (!GV_IS_POINTS(points)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvPoints");
	return NULL;
    }

    return PyInt_FromLong(gv_points_new_point(points, &vertex));
}

static PyObject *
build_py_line(GArray *line)
{
    PyObject *pylist;
    GvVertex *v;
    int i;

    pylist = PyList_New(line->len);
    for (i=0; i < line->len; i++)
    {
        PyObject *py_value;

        v = &g_array_index(line, GvVertex, i);
        py_value = Py_BuildValue("(" CC ")", v->x, v->y);
        PyList_SetItem(pylist, i, py_value);
    }
    return pylist;
}

static GArray *
build_gv_line(PyObject *pylist, int min_len)
{
    GArray *line;
    GvVertex *v;
    int  i;

    if (PyList_Size(pylist) < min_len)
    {
    PyErr_SetString(PyExc_ValueError, "line too short");
    return NULL;
    }

    line = g_array_new(FALSE, FALSE, sizeof(GvVertex));
    g_array_set_size(line, PyList_Size(pylist));
    for (i=0; i < line->len; i++)
    {
    v = &g_array_index(line, GvVertex, i);
    if (!PyArg_ParseTuple(PyList_GET_ITEM(pylist, i), CC, &v->x, &v->y))
    {
        PyErr_SetString(PyExc_ValueError, "bad line format");
        g_array_free(line, TRUE);
        return NULL;
    }
    }
    return line;
}

static PyObject *
_wrap_gv_polylines_get_line(PyObject *self, PyObject *args)
{
    PyObject *py_pline;
    GvPolylines *pline;
    int index;

    if (!PyArg_ParseTuple(args, "Oi:gv_polylines_get_line", &py_pline, &index)) {
	return NULL;
    }

    pline = GV_POLYLINES(PyGtk_Get(py_pline));
    if (!GV_IS_POLYLINES(pline)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvPolylines");
	return NULL;
    }

    if (index < 0 || index >= gv_polylines_num_lines(pline))
    {
	PyErr_SetString(PyExc_IndexError, "line index out of range");
	return NULL;
    }
    return build_py_line(gv_polylines_get_line(pline, index));
}

static PyObject *
_wrap_gv_polylines_new_line(PyObject *self, PyObject *args)
{
    PyObject *py_pline, *pylist;
    GvPolylines *pline;
    GArray *line;
    int index;

    if (!PyArg_ParseTuple(args, "OO!:gv_polylines_new_line",
			  &py_pline, &PyList_Type, &pylist)) {
	return NULL;
    }

    pline = GV_POLYLINES(PyGtk_Get(py_pline));
    if (!GV_IS_POLYLINES(pline)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvPolylines");
	return NULL;
    }

    line = build_gv_line(pylist, 2);
    if (!line) return NULL;
    index = gv_polylines_new_line_with_data(pline, 
					    line->len, (GvVertex*)line->data);
    g_array_free(line, TRUE);
    return PyInt_FromLong(index);
}

static PyObject *
_wrap_gv_areas_get_area(PyObject *self, PyObject *args)
{
    PyObject *pyareas, *pyarea;
    GvAreas *areas;
    GvArea *area;
    int index, ring;

    if (!PyArg_ParseTuple(args, "Oi:gv_areas_get_area", &pyareas, &index)) {
	return NULL;
    }

    areas = GV_AREAS(PyGtk_Get(pyareas));
    if (!GV_IS_AREAS(areas)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvAreas");
	return NULL;
    }

    if (index < 0 || index >= gv_areas_num_areas(areas))
    {
	PyErr_SetString(PyExc_IndexError, "area index out of range");
	return NULL;
    }
    area = gv_areas_get_area(areas, index);

    pyarea = PyList_New(gv_areas_num_rings(area));
    for (ring = 0; ring < gv_areas_num_rings(area); ring++)
    {
        PyObject *pyline = build_py_line(gv_areas_get_ring(area, ring));
        PyList_SetItem(pyarea, ring, pyline);
    }
    return pyarea;
}

static PyObject *
_wrap_gv_areas_new_area(PyObject *self, PyObject *args)
{
    PyObject *pyareas, *pyarea;
    GvAreas *areas;
    GvArea *area;
    int index, ring, num_rings;

    if (!PyArg_ParseTuple(args, "OO!:gv_areas_new_area",
			  &pyareas, &PyList_Type, &pyarea)) {
	return NULL;
    }

    areas = GV_AREAS(PyGtk_Get(pyareas));
    if (!GV_IS_AREAS(areas)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvAreas");
	return NULL;
    }

    num_rings = PyList_Size(pyarea);
    if (num_rings < 1)
    {
	PyErr_SetString(PyExc_ValueError, "empty ring list");
	return NULL;
    }

    area = gv_area_new(FALSE);
    for (ring = 0; ring < num_rings; ring++)
    {
	GArray *line = build_gv_line(PyList_GET_ITEM(pyarea, ring), 3);
	if (!line) break;
	g_ptr_array_add(area->rings, line);
    }
    if (PyErr_Occurred())
    {
	gv_area_delete(area);
	return NULL;
    }

    index = gv_areas_new_area_with_data(areas, area);
    gv_area_delete(area);
    return PyInt_FromLong(index);
}

static PyObject *
_wrap_gv_shape_layer_set_color(PyObject *self, PyObject *args)
{
    PyObject *py_layer;
    GvShapeLayer *layer;
    GvColor color;

    if (!PyArg_ParseTuple(args, "O(ffff):gv_shape_layer_set_color",
              &py_layer, &color[0], &color[1], &color[2], &color[3])) {
	return NULL;
    }

    layer = GV_SHAPE_LAYER(PyGtk_Get(py_layer));
    if (!GV_IS_SHAPE_LAYER(layer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvShapeLayer");
	return NULL;
    }

    gv_shape_layer_set_color(layer, color);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_shape_layer_get_selected(PyObject *self, PyObject *args)
{
    PyObject *py_layer;
    GvShapeLayer *layer;
    PyObject  *list;
    GArray    *array;
    int       i;

    if (!PyArg_ParseTuple(args, "O:gv_shape_layer_get_selected", &py_layer)) {
	return NULL;
    }

    layer = GV_SHAPE_LAYER(PyGtk_Get(py_layer));
    if (!GV_IS_SHAPE_LAYER(layer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvShapeLayer");
	return NULL;
    }

    array = g_array_new(FALSE,TRUE,sizeof(gint));
    gv_shape_layer_selected(layer, GV_ALL, (void *) array );

    list = PyList_New(array->len);
    for( i = 0; i < array->len; i++ )
    {
        PyList_SetItem( list, i,
                        Py_BuildValue("i", g_array_index(array,gint,i)) );
    }

    g_array_free( array, TRUE );

    return list;
}

static PyObject *
_wrap_gv_shape_layer_pick_shape( PyObject *self, PyObject *args )
{
    PyObject *py_layer, *py_view;
    GvShapeLayer *layer;
    GvViewArea *view;
    gint shape_id;
    float x, y;

    if (!PyArg_ParseTuple(args, "OOff:gv_shape_layer_pick_shape",
                          &py_layer, &py_view, &x, &y ))
    {
        return NULL;
    }

    layer = GV_SHAPE_LAYER(PyGtk_Get(py_layer));
    if (!GV_IS_SHAPE_LAYER(layer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvShapeLayer");
	return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
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

/******************
 * gv_raster_new
 *
 * This function makes assumptions about the dataio lib and should
 * be modified once the dataio situation is sorted out.
 *
 * Args: filename, format (optional)
 ******************/

static PyObject *
_wrap_gv_raster_new(PyObject *self, PyObject *args, PyObject *keywds)
{
    PyObject * py_ret;
    char *filename = NULL, *dataset_string = NULL;
    GDALDatasetH  dataset;
    static int gdal_initialized = 0;
    GvSampleMethod sm = GvSMAverage;
    int   rband = 1;
    static char *kwlist[] = {"filename", "sample", "real",
                             "dataset", NULL};
    GvRaster *raster;

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "|ziiz", kwlist,
                                     &filename, &sm, &rband,
                                     &dataset_string))
    return NULL;

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
            return NULL;
        }
    }
    else if( filename != NULL )
    {
        dataset = GDALOpen( filename, GA_ReadOnly );

        if (dataset == NULL)
        {
            PyErr_SetString(PyExc_IOError, "failed to open data file");
            return NULL;
        }

        GDALDereferenceDataset( dataset );
    }
    else
    {
        PyErr_SetString(PyExc_IOError,
                        "gv_raster_new: either a filename, or dataset handle"
                        " is required.  Neither provided." );
        return NULL;
    }

    raster = GV_RASTER(gv_raster_new(dataset,rband,sm));
    if( filename != NULL )
        gv_data_set_name(GV_DATA(raster), filename);

    py_ret = PyGtk_New( G_OBJECT(raster) );

    gtk_object_sink( GTK_OBJECT(raster) );

    return py_ret;
}

static PyObject *
_wrap_gv_raster_autoscale(PyObject *self, PyObject *args)

{
    int alg = GvASAAutomatic, assign = 0, success;
    double alg_param = -1.0;
    PyObject *py_raster;
    GvRaster *raster = NULL;
    double   out_min, out_max;

    if (!PyArg_ParseTuple(args, "Oidi:gv_raster_autoscale", 
                          &py_raster, &alg, &alg_param, &assign)) {
        return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    if( assign == 0 )
    {
        success = gv_raster_autoscale(raster, alg, alg_param, 0, NULL,
                                      NULL, NULL);
        if( !success )
        {
            PyErr_SetString(PyExc_RuntimeError,
                            "autoscale() failed, failed to get samples?" );
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
                            "autoscale() failed, failed to get samples?" );
            return NULL;
        }

        return Py_BuildValue("(dd)", out_min, out_max );
    }
}

static PyObject *
_wrap_gv_raster_get_gdal_band(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    char         swig_ptr[32];
    GDALRasterBandH band;

    if (!PyArg_ParseTuple(args, "O:gv_raster_get_gdal_band",
			  py_raster )) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    band = raster->gdal_band;

    if( band == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    SWIG_SimpleMakePtr( swig_ptr, band, "_GDALRasterBandH" );
    return Py_BuildValue("s",swig_ptr);
}

static PyObject *
_wrap_gv_raster_get_gdal_dataset(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    char         swig_ptr[32];
    GDALDatasetH dataset;

    if (!PyArg_ParseTuple(args, "O:gv_raster_get_gdal_dataset",
			  &py_raster )) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    dataset = raster->dataset;

    if( dataset == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    SWIG_SimpleMakePtr( swig_ptr, dataset, "_GDALDatasetH" );
    return Py_BuildValue("s",swig_ptr);
}

static PyObject *
_wrap_gv_raster_force_load(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    int       i;

    if (!PyArg_ParseTuple(args, "O:gv_raster_force_load",
			  &py_raster )) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    for( i = 0; i < raster->max_tiles; i++ )
        gv_raster_tile_get( raster, i, 0 );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_raster_data_changing(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    int      x_off=0, y_off=0, width=0, height=0;
    GvRasterChangeInfo change_info;

    if (!PyArg_ParseTuple(args, "Oiiii:gv_raster_data_changing",
			  &py_raster, &x_off, &y_off, &width, &height )) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    if( height > 0 )
    {
        change_info.change_type = GV_CHANGE_REPLACE;
        change_info.x_off = x_off;
        change_info.y_off = y_off;
        change_info.width = width;
        change_info.height = height;

        gv_data_changing( GV_DATA(raster), &change_info );
    }
    else
    {
        gv_data_changing( GV_DATA(raster), NULL );
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_raster_data_changed(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    int      x_off=0, y_off=0, width=0, height=0;
    GvRasterChangeInfo change_info;

    if (!PyArg_ParseTuple(args, "Oiiii:gv_raster_data_changed",
			  &py_raster, &x_off, &y_off, &width, &height )) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    if( height > 0 )
    {
        change_info.change_type = GV_CHANGE_REPLACE;
        change_info.x_off = x_off;
        change_info.y_off = y_off;
        change_info.width = width;
        change_info.height = height;

        gv_data_changed( GV_DATA(raster), &change_info );
    }
    else
    {
        gv_data_changed( GV_DATA(raster), NULL );
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_raster_get_sample(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    double x, y, real, imaginary;

    if (!PyArg_ParseTuple(args, "Odd:gv_raster_get_sample",
			  py_raster, &x, &y )) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    if( !gv_raster_get_sample(raster, x, y, &real, &imaginary ) )
        return NULL;
    else if( GDALDataTypeIsComplex(raster->gdal_type) )
        return Py_BuildValue( "(ff)", real, imaginary );
    else
        return Py_BuildValue( "f", real );
}

static PyObject *
_wrap_gv_raster_georef_to_pixel(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    double x, y;

    if (!PyArg_ParseTuple(args, "Odd:gv_raster_georef_to_pixel",
			  &py_raster, &x, &y )) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    if( gv_raster_georef_to_pixel(raster, &x, &y, NULL) )
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

static PyObject *
_wrap_gv_raster_pixel_to_georef(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    double x, y;

    if (!PyArg_ParseTuple(args, "Odd:gv_raster_pixel_to_georef",
			  &py_raster, &x, &y )) {
        return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    if( gv_raster_pixel_to_georef(raster, &x, &y, NULL ) )
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

static PyObject *
_wrap_gv_raster_get_gcps(PyObject *self, PyObject *args) {

    PyObject *py_raster;
    GvRaster *raster;
    const GDAL_GCP * pasGCPList;
    PyObject *psList;
    int iGCP;

    if (!PyArg_ParseTuple(args,"O:gv_raster_get_gcps", &py_raster )) {
        return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    pasGCPList = gv_raster_get_gcps( raster );

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

static PyObject *
_wrap_gv_raster_georef_to_pixelCL(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    double x, y;

    if (!PyArg_ParseTuple(args, "Odd:gv_raster_georef_to_pixelCL",
			  &py_raster, &x, &y )) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    if( gv_raster_georef_to_pixelCL(raster, &x, &y, NULL) )
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

static PyObject *
_wrap_gv_raster_pixel_to_georefCL(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    GvRaster *raster;
    double x, y;

    if (!PyArg_ParseTuple(args, "Odd:gv_raster_pixel_to_georefCL",
			  &py_raster, &x, &y )) {
        return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    if( gv_raster_pixel_to_georefCL(raster, &x, &y, NULL ) )
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

static PyObject *
_wrap_gv_raster_get_gcpsCL(PyObject *self, PyObject *args) {

    PyObject *py_raster;
    GvRaster *raster;
    const GDAL_GCP * pasGCPList;
    PyObject *psList;
    int iGCP;

    if (!PyArg_ParseTuple(args,"O:gv_raster_get_gcps", &py_raster )) {
        return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

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

static PyObject *
_wrap_gv_raster_set_gcps(PyObject *self, PyObject *args) {

    PyObject *py_raster;
    GvRaster *raster;
    GDAL_GCP *pasGCPList;
    PyObject *psList;
    int iGCP, nGCPCount, success;

    if (!PyArg_ParseTuple(args,"OO!:gv_raster_set_gcps",
                         &py_raster, &PyList_Type, &psList)) {
        return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

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

static PyObject *
_wrap_gv_raster_set_gcpsCL(PyObject *self, PyObject *args) {

    PyObject *py_raster;
    GvRaster *raster;
    GDAL_GCP *pasGCPList;
    PyObject *psList;
    int iGCP, nGCPCount, success, poly_order;

    if (!PyArg_ParseTuple(args,"OO!i:gv_raster_set_gcpsCL",
			  &py_raster, &PyList_Type, &psList,
			  &poly_order)) {
        return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

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

static PyObject *
_wrap_gv_raster_layer_new(PyObject *self, PyObject *args)
{
    PyObject *py_raster;
    PyObject *py_properties = NULL;
    GvRaster *raster;
    GvRasterLayer *layer;
    GvProperties properties = NULL;
    int      mode = GV_RLM_AUTO;

    if (!PyArg_ParseTuple(args, "O|iO!:gv_raster_layer_new",
			  &py_raster, &mode, &PyList_Type, &py_properties )) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
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
                return NULL;
            }

            gv_properties_set( &properties, name, value );
        }
    }

    layer = GV_RASTER_LAYER(gv_raster_layer_new( mode, raster, properties ));

    gv_properties_destroy( &properties );

    return PyGtk_New((GObject *)layer);
}

static PyObject *
_wrap_gv_raster_layer_autoscale_view(PyObject *self, PyObject *args)

{
    int alg = GvASAAutomatic, isrc = 0, success;
    double alg_param = -1.0;
    PyObject *py_rlayer;
    GvRasterLayer *rlayer = NULL;
    double   out_min, out_max;

    if (!PyArg_ParseTuple(args, "Oidi:gv_raster_layer_autoscale_view",
                          &py_rlayer, &alg, &alg_param, &isrc)) {
        return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));

    success = gv_raster_layer_autoscale_view(rlayer, isrc, alg, alg_param,
                                             &out_min, &out_max );
    if( !success )
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "autoscale() failed, failed to get samples?" );
        return NULL;
    }

    return Py_BuildValue("(dd)", out_min, out_max );
}

static PyObject *
_wrap_gv_raster_layer_get_mesh_lod(PyObject *self, PyObject *args)

{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer = NULL;

    if (!PyArg_ParseTuple(args, "O:gv_raster_layer_get_mesh_lod",
                          &py_rlayer)) {
        return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    return Py_BuildValue("i", rlayer->mesh->detail );
}

static PyObject *
_wrap_gv_raster_layer_histogram_view(PyObject *self, PyObject *args)

{
    PyObject *py_rlayer, *py_list;
    GvRasterLayer *rlayer = NULL;
    double   scale_min, scale_max;
    int      hist_size = 256, isrc = 0, hist_count, i;
    int      *histogram = NULL;

    if (!PyArg_ParseTuple(args, "Oiddi:gv_raster_layer_autoscale_view",
                          &py_rlayer, &isrc,
                          &scale_min, &scale_max, &hist_size)) {
        return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    histogram = g_new(int, hist_size);

    hist_count =
        gv_raster_layer_histogram_view(rlayer, isrc,
                                       scale_min, scale_max, TRUE,
                                       hist_size, histogram );
    if( hist_count == 0 )
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "histogram() failed, failed to get samples?" );
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

static PyObject *
_wrap_gv_raster_layer_pixel_to_view(PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    double x, y;

    if (!PyArg_ParseTuple(args, "Odd:gv_raster_layer_pixel_to_view",
			  &py_rlayer, &x, &y )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    if( gv_raster_layer_pixel_to_view(rlayer, &x, &y, NULL ) )
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

static PyObject *
_wrap_gv_raster_layer_view_to_pixel(PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    double x, y;

    if (!PyArg_ParseTuple(args, "Odd:gv_raster_layer_view_to_pixel",
			  &py_rlayer, &x, &y )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    if( gv_raster_layer_view_to_pixel(rlayer, &x, &y, NULL ) )
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

static PyObject *
_wrap_gv_raster_layer_texture_mode_set(PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    int texture_mode;
    GvColor color;

    if (!PyArg_ParseTuple(args, "Oi(ffff):gv_raster_layer_texture_mode_set",
			  &py_rlayer, &texture_mode,
			  &color[0], &color[1], &color[2], &color[3])) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    return PyInt_FromLong(gv_raster_layer_texture_mode_set(rlayer,
							   texture_mode, color));
}

static PyObject *
_wrap_gv_raster_layer_alpha_get(PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    float alpha_val;
    int alpha_mode;

    if (!PyArg_ParseTuple( args, "O:gv_raster_layer_alpha_get",
			   &py_rlayer )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    if (gv_raster_layer_alpha_get(rlayer, &alpha_mode, &alpha_val ))
    {
	Py_INCREF(Py_None);
	return Py_None;
    } else {
	return Py_BuildValue("(if)", alpha_mode, alpha_val );
    }
}

static PyObject *
_wrap_gv_raster_layer_blend_mode_get( PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    int blend_mode;
    int sfactor;
    int dfactor;

    if (!PyArg_ParseTuple( args, "O:gv_raster_layer_blend_mode_get",
			   &py_rlayer )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    if (gv_raster_layer_blend_mode_get(rlayer, &blend_mode, &sfactor, &dfactor ))
    {
	Py_INCREF(Py_None);
	return Py_None;
    } else {
	return Py_BuildValue("(iii)", blend_mode, sfactor, dfactor );
    }
}

static PyObject *
_wrap_gv_raster_layer_texture_mode_get( PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    int texture_mode;
    GvColor color;
    PyObject *py_color, *py_retval;

    if (!PyArg_ParseTuple( args, "O:gv_raster_layer_texture_mode_get",
			   &py_rlayer )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    if (gv_raster_layer_texture_mode_get(rlayer, &texture_mode, &color ))
    {
	Py_INCREF(Py_None);
	return Py_None;
    } else {
	py_color = Py_BuildValue( "(ffff)", color[0], color[1], color[2], color[3] );
	py_retval = Py_BuildValue( "(iO)", texture_mode, py_color );
	Py_DECREF( py_color );
	return py_retval;
    }
}

static PyObject *
_wrap_gv_raster_layer_lut_put( PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    char *lut;
    int  lut_len, height;

    if (!PyArg_ParseTuple( args, "Oz#:gv_raster_layer_lut_put",
			   &py_rlayer, &lut, &lut_len )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    if( lut != NULL && lut_len != 1024 && lut_len != 1024 * 256 )
    {
     PyErr_SetString(PyExc_TypeError,
       "lut string must be 256x1x4 or 256x256x4 in gv_raster_layer_lut_put");
     return NULL;
    }

    height = lut_len / 1024;
    gv_raster_layer_lut_put(rlayer, lut, height );

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gv_raster_layer_lut_get( PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    PyObject *py_lut, *py_retval;
    char *lut;
    int width, height;
    int rgba_complex=0;

    if (!PyArg_ParseTuple( args, "O|i:gv_raster_layer_lut_get",
                           &py_rlayer, &rgba_complex )) {
        return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    lut = gv_raster_layer_lut_get(rlayer, &width, &height, rgba_complex );
    if (lut == NULL)
    {
        Py_INCREF(Py_None);
        return Py_None;
    } else {
	py_lut = PyString_FromStringAndSize( lut, width * height * 4 );
        if ( py_lut == NULL )
        {
            Py_INCREF(Py_None);
            return Py_None;
        }

        py_retval = Py_BuildValue( "(Oii)", py_lut, width, height );
        Py_DECREF( py_lut );
        return py_retval;
    }
}

static PyObject *
_wrap_gv_raster_layer_source_get_lut( PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    PyObject *py_lut;
    char *lut;
    int isource=0;

    if (!PyArg_ParseTuple( args, "Oi:gv_raster_layer_source_get_lut",
                           &py_rlayer, &isource )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    lut = gv_raster_layer_source_get_lut(rlayer, isource );
    if( lut == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if ( ( py_lut = PyString_FromStringAndSize( lut, 256 ) ) == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return py_lut;
}

static PyObject *
_wrap_gv_raster_layer_nodata_get( PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    int isource = 0;
    gint ret;
    double nodata_real, nodata_imaginary;

    if (!PyArg_ParseTuple( args, "Oi:gv_raster_layer_nodata_get",
                           &py_rlayer, &isource )) {
        return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

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

/* XXX: gv_raster_layer_get_nodata() now DEPRECATED. */
static PyObject *
_wrap_gv_raster_layer_get_nodata( PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    int isource=0;

    if (!PyArg_ParseTuple( args, "Oi:gv_raster_layer_get_nodata",
			   &py_rlayer, &isource )) {
        return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

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

static PyObject *
_wrap_gv_raster_layer_set_source( PyObject *self, PyObject *args)
{
    PyObject *py_rlayer, *py_raster;
    GvRasterLayer *rlayer;
    GvRaster *raster = NULL;
    char *lut = NULL;
    int isource, const_value, ret, lut_len=0, nodata_active=FALSE;
    float min, max;
    PyObject *nodata = NULL;
    float nodata_real=-1e8, nodata_imaginary=0.0;

    if (!PyArg_ParseTuple( args, "OiOffi|z#O:gv_raster_layer_set_source",
                           &py_rlayer, &isource,
                           &py_raster, &min, &max, &const_value,
                           &lut, &lut_len, &nodata )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    if( py_raster == NULL || py_raster == Py_None )
        raster = NULL;
    else
        raster = GV_RASTER(PyGtk_Get(py_raster));

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

    ret = gv_raster_layer_set_source(rlayer,
				     isource, raster,
				     min, max, const_value, lut,
				     nodata_active,
				     nodata_real, nodata_imaginary );

    return Py_BuildValue("i", ret);
}

static PyObject *
_wrap_gv_raster_layer_zoom_get( PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    int mag, min;

    if (!PyArg_ParseTuple( args, "O:gv_raster_layer_zoom_get",
			   &py_rlayer )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    if( gv_raster_layer_zoom_get(rlayer, &mag, &min ) )
    {
	Py_INCREF(Py_None);
	return Py_None;
    } else {
	return Py_BuildValue( "(ii)", min, mag );
    }
}

static PyObject *
_wrap_gv_raster_layer_get_height(PyObject *self, PyObject *args)
{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;
    GvMesh *mesh;
    double x, y, result;
    int    success;

    if (!PyArg_ParseTuple(args, "Odd:gv_raster_layer_get_height",
			  &py_rlayer, &x, &y )) {
	return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    mesh = rlayer->mesh;

    result = gv_mesh_get_height( mesh, x, y, &success );

    if( success )
        return Py_BuildValue("f", result );
    else
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "gv_mesh_get_height() failed." );
        return NULL;
    }
}

static PyObject *
_wrap_gv_raster_get_change_info(PyObject *self, PyObject *args)
{
    GvRasterChangeInfo *change_info;
    PyObject *c_change_info;

    if (!PyArg_ParseTuple(args, "O:gv_raster_get_change_info", &c_change_info))
    return NULL;

    if (!PyCObject_Check (c_change_info))
        return NULL;

    change_info = (GvRasterChangeInfo *) PyCObject_AsVoidPtr(c_change_info);

    return Py_BuildValue( "(iiiii)",
                          change_info->change_type,
                          change_info->x_off,
                          change_info->y_off,
                          change_info->width,
                          change_info->height
                         );
}


static PyObject *
_wrap_gv_manager_get_preferences(PyObject *self, PyObject *args)
{
    PyObject *py_manager;
    GvManager *manager;
    GvProperties *properties = NULL;
    PyObject *psDict = NULL;

    if (!PyArg_ParseTuple(args, "O:gv_manager_get_properties", &py_manager)) {
	return NULL;
    }

    manager = GV_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvManager");
	return NULL;
    }

    properties = gv_manager_get_preferences(manager);

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

static PyObject *
_wrap_gv_manager_add_dataset(PyObject *self, PyObject *args)
{
    PyObject *py_manager;
    GvManager *manager;
    char     *dataset_string=NULL;
    char      swig_ptr[32];
    GDALDatasetH dataset = NULL;

    if (!PyArg_ParseTuple(args, "Os:gv_manager_add_dataset",
                          &py_manager, &dataset_string)) {
        return NULL;
    }

    manager = GV_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvManager");
	return NULL;
    }

    if( manager == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    dataset = (GDALDatasetH) SWIG_SimpleGetPtr(dataset_string, "GDALDatasetH");
    if( dataset == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    dataset = gv_manager_add_dataset(manager, dataset );
    SWIG_SimpleMakePtr( swig_ptr, dataset, "_GDALDatasetH" );
    return Py_BuildValue( "s", swig_ptr );
}

static PyObject *
_wrap_gv_manager_get_dataset(PyObject *self, PyObject *args)
{
    PyObject *py_manager;
    GvManager *manager;
    char     *filename = NULL;
    char      swig_ptr[32];
    GDALDatasetH dataset = NULL;

    if (!PyArg_ParseTuple(args, "Os:gv_manager_get_dataset",
                          &py_manager, &filename)) {
        return NULL;
    }

    manager = GV_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvManager");
	return NULL;
    }

    dataset = gv_manager_get_dataset(manager, filename );

    if( dataset == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else
    {
        SWIG_SimpleMakePtr( swig_ptr, dataset, "_GDALDatasetH" );
        return Py_BuildValue( "s", swig_ptr );
    }
}

static PyObject *
_wrap_gv_manager_get_dataset_raster(PyObject *self, PyObject *args)
{
    PyObject *py_manager;
    GvManager *manager;
    char     *dataset_string=NULL;
    int       band = 0;
    GDALDatasetH dataset = NULL;
    GvRaster *raster = NULL;

    if (!PyArg_ParseTuple(args, "Osi:gv_manager_get_dataset_raster",
                          &py_manager, &dataset_string, &band)) {
        return NULL;
    }

    manager = GV_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvManager");
	return NULL;
    }

    if( manager == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    dataset = (GDALDatasetH) SWIG_SimpleGetPtr(dataset_string, "GDALDatasetH");
    if( dataset == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    raster = gv_manager_get_dataset_raster(manager, dataset, band );

    if( raster == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else
    {
        return PyGtk_New((GObject*)raster);
    }
}

/* -------------------------------------------------------------------- */
/*      Idle Task C callback structure.                                 */
/* -------------------------------------------------------------------- */
typedef struct {
    PyObject *psPyCallback;
    PyObject *psPyCallbackData;
    PyThreadState *psThreadState;
} PyTaskData;

int PyIdleTaskProxy( void *task_info )

{
    PyTaskData *psInfo = (PyTaskData *) task_info;
    PyObject *psArgs, *psResult;
    int      bContinue = TRUE;
    PyThreadState *tstate;

    tstate = PyThreadState_Swap( psInfo->psThreadState );

    psArgs = Py_BuildValue("(O)", psInfo->psPyCallbackData );

    psResult = PyEval_CallObject( psInfo->psPyCallback, psArgs);

    tstate = PyThreadState_Swap( tstate );
    if( tstate == NULL )
    {
        CPLDebug( "OpenEV",
                  "PyIdleTaskProxy: Thread state unexpectedly disappeared.\n"
                  "                  Skipping check for error.\n" );
    }
    else
    {
        tstate = PyThreadState_Swap( tstate );
        if( PyErr_Occurred() )
        {
            PyErr_Print();
            PyErr_Clear();
        }
    }

    Py_XDECREF(psArgs);

    PyThreadState_Swap( tstate );

    if( psResult == NULL
        || psResult == Py_None
        || !PyArg_Parse( psResult, "i", &bContinue )
        || !bContinue )
    {
        bContinue = FALSE;
        Py_XDECREF( psInfo->psPyCallback );
        Py_XDECREF( psInfo->psPyCallbackData );
        g_free( task_info );
    }

    Py_XDECREF(psResult);

    return bContinue;
}

static PyObject *
_wrap_gv_manager_queue_task(PyObject *self, PyObject *args)
{
    PyObject *py_manager;
    GvManager *manager;
    int       priority;
    char      *task_name;
    PyTaskData *psProgressInfo;

    psProgressInfo = g_new(PyTaskData,1);
    psProgressInfo->psPyCallback = NULL;
    psProgressInfo->psPyCallbackData = Py_None;

    if (!PyArg_ParseTuple(args, "OsiO|O:gv_manager_queue_task",
                          &py_manager, &task_name, &priority,
                          &(psProgressInfo->psPyCallback),
                          &(psProgressInfo->psPyCallbackData) ) ) {
	return NULL;
    }

    manager = GV_MANAGER(PyGtk_Get(py_manager));
    if (!GV_IS_MANAGER(manager)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvManager");
	return NULL;
    }

    if( manager == NULL )
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    Py_XINCREF( psProgressInfo->psPyCallback );
    Py_XINCREF( psProgressInfo->psPyCallbackData );

    psProgressInfo->psThreadState = PyThreadState_Get();

    gv_manager_queue_task(manager,
			  task_name, priority,
			  PyIdleTaskProxy,
			  psProgressInfo );

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject *
_wrap_gv_launch_url(PyObject *self, PyObject *args)
{
    char *url = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_launch_url",
                          &url))
    return NULL;

    return Py_BuildValue("i",gv_launch_url(url));
}

static PyObject *
_wrap_gv_rgba_to_rgb(PyObject *self, PyObject *args)
{
    PyObject *rgba_obj = NULL;
    PyObject *rgb_obj = NULL;
    const char *rgba;
    char       *rgb;
    int length, i;

    if (!PyArg_ParseTuple(args, "O!:gv_rgba_to_rgb",
                          &PyString_Type, &rgba_obj))
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

/*
 * Algorithms not very specific to OpenEV.
 */

static PyObject *
_wrap_WIDInterpolate(PyObject *self, PyObject *args)
{
    PyObject *poPyPoints;
    PyProgressData sProgressInfo;
    char *pszSwigBand = NULL;
    int nPoints, i, nErr;
    double *padfXYVW;
    double fExponent;
    GDALRasterBandH hBand;

    sProgressInfo.psPyCallback = NULL;
    sProgressInfo.psPyCallbackData = NULL;

    if (!PyArg_ParseTuple(args, "O!s|dOO:WIDInterpolate",
                          &PyList_Type, &poPyPoints,
                          &pszSwigBand,
                          &fExponent,
                          &(sProgressInfo.psPyCallback),
                          &(sProgressInfo.psPyCallbackData) ) )
    return NULL;

    hBand = (GDALRasterBandH)
        SWIG_SimpleGetPtr(pszSwigBand, "_GDALRasterBandH" );

    if( hBand == NULL )
    {
        PyErr_SetString( PyExc_ValueError,
                         "Couldn't parse GDALRasterBandH argument." );
        return NULL;
    }

    nPoints = PyList_Size(poPyPoints);
    padfXYVW = g_new(double,4*nPoints);
    for( i = 0; i < nPoints; i++ )
    {
        if( !PyArg_Parse( PyList_GET_ITEM(poPyPoints,i), "(dddd)",
                          padfXYVW + i + 0*nPoints,
                          padfXYVW + i + 1*nPoints,
                          padfXYVW + i + 2*nPoints,
                          padfXYVW + i + 3*nPoints ) )
        {
            g_free( padfXYVW );
        PyErr_SetString(PyExc_ValueError,
                            "bad point format (x,y,value,weight)" );
            return NULL;
        }
    }

    nErr = WIDInterpolate( nPoints, padfXYVW, padfXYVW+nPoints,
                           padfXYVW+nPoints*2, padfXYVW+nPoints*3, hBand,
                           fExponent, PyProgressProxy, &sProgressInfo );

    g_free( padfXYVW );

    return Py_BuildValue( "i", nErr );
}

static PyObject *
_wrap_gv_raster_rasterize_shapes(PyObject *self, PyObject *args)
{
    PyObject *py_shapelist;
    PyObject *py_raster;
    GvRaster *raster;
    double   burn_value;
    int      shape_count, i, ret_value, fill_short = 1;
    GvShape  **shape_list;

    if (!PyArg_ParseTuple(args, "OO!di:gv_raster_rasterize_shapes",
			  &py_raster, &PyList_Type, &py_shapelist,
                          &burn_value, &fill_short)) {
	return NULL;
    }

    raster = GV_RASTER(PyGtk_Get(py_raster));
    if (!GV_IS_RASTER(raster)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRaster");
	return NULL;
    }

    shape_count = PyList_Size(py_shapelist);
    shape_list = g_new(GvShape*,shape_count);
    for( i = 0; i < shape_count; i++ )
    {
        GvShape *gv_shape;
        char *swig_shape_ptr;

        if( !PyArg_Parse( PyList_GET_ITEM(py_shapelist,i), "s",
                          &swig_shape_ptr) )
        {
            g_free( shape_list );
            PyErr_SetString( PyExc_ValueError,
                             "bad item in shapelist" );
            return NULL;
        }
        gv_shape = SWIG_SimpleGetPtr( swig_shape_ptr, "_GvShape" );
        if( gv_shape == NULL )
        {
            g_free( shape_list );
            PyErr_SetString( PyExc_ValueError,
                             "bad item in shapelist(2)" );
            return NULL;
        }

        shape_list[i] = gv_shape;
    }

    ret_value = gv_raster_rasterize_shapes( raster, shape_count, shape_list,
                                            burn_value, fill_short );

    return Py_BuildValue( "i", ret_value );
}

/*
 * wrapper function for getting a DOS 8.3 compatible file name on
 * Windows systems.  On other systems, this will return the
 * value passed in.
 */

static PyObject * _wrap_gv_short_path_name(PyObject *self, PyObject *args)
{
    char *lpszLongPath = NULL;
    PyObject * result = NULL;

    if (!PyArg_ParseTuple(args, "s:gv_short_path_name",
                          &lpszLongPath))
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

static PyObject * _wrap_gtk_color_well_get_d( PyObject *self, PyObject *args)
{
    PyObject *py_color_well;
    GtkColorWell *color_well;
    gdouble r, g, b, a;

    if (!PyArg_ParseTuple(args, "O:gtk_color_well_get_d", &py_color_well)) {
        return NULL;
    }

    color_well = GTK_COLOR_WELL(PyGtk_Get(py_color_well));
    if (!GTK_IS_COLOR_WELL(color_well)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GtkColorWell");
	return NULL;
    }

    gtk_color_well_get_d(color_well, &r, &g, &b, &a );

    return Py_BuildValue( "(dddd)", r, g, b, a );
}

static PyObject *_wrap_gv_shapes_to_dbf(PyObject *self, PyObject *args) {
    PyObject *py_data;
    GvData *data;
    char *filename;

    if (!PyArg_ParseTuple(args, "sO:gv_shapes_to_dbf", &filename, &py_data)) {
        return NULL;
    }

    data = GV_DATA(PyGtk_Get(py_data));
    if (!GV_IS_DATA(data)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvData object");
	return NULL;
    }

    return PyInt_FromLong(gv_shapes_to_dbf(filename, data));
}

static PyObject *_wrap_gv_format_point_query( PyObject *self, PyObject *args )
{
    gdouble x, y;
    char *text;
    PyObject *py_view;
    GvViewArea *view = NULL;
    GvManager *manager;

    if (!PyArg_ParseTuple( args, "Odd:gv_format_point_query", &py_view, &x, &y )) {
        return NULL;
    }

    view = GV_VIEW_AREA(PyGtk_Get(py_view));
    if (!GV_IS_VIEW_AREA(view)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvViewArea object");
	return NULL;
    }

    manager = gv_get_manager();

    text = (char *) gv_format_point_query( view, &(manager->preferences), x, y );

    return Py_BuildValue( "s", text );
}


static PyObject *_wrap_gv_test_entry( PyObject *self, PyObject *args )

{
    PyObject *py_rlayer;
    GvRasterLayer *rlayer;

    if (!PyArg_ParseTuple(args, "O:gv_test_entry", &py_rlayer )) {
        return NULL;
    }

    rlayer = GV_RASTER_LAYER(PyGtk_Get(py_rlayer));
    if (!GV_IS_RASTER_LAYER(rlayer)) {
	PyErr_SetString(PyExc_TypeError, "argument must be a GvRasterLayer");
	return NULL;
    }

    return Py_BuildValue( "d", 0.0 );
}

static PyObject *
_wrap_MyGDALOperator(PyObject *self, PyObject *args)
{
    char *pszSwigDS1 = NULL;
    char *pszSwigDS2 = NULL;
    GDALDatasetH hDS1 = NULL, hDS2 = NULL;
    int  nErr = 1;

    if (!PyArg_ParseTuple(args, "ss:MyGDALOperator",
                          &pszSwigDS1, &pszSwigDS2 ) )
        return NULL;

    hDS1 = (GDALDatasetH) SWIG_SimpleGetPtr(pszSwigDS1, "_GDALDatasetH" );
    hDS2 = (GDALDatasetH) SWIG_SimpleGetPtr(pszSwigDS2, "_GDALDatasetH" );

    if( hDS1 != NULL && hDS2 != NULL )
    {
        /* do something with hDS1 and hDS2 */
        printf( "%s -> %s\n",
                GDALGetDescription( hDS1 ),
                GDALGetDescription( hDS2 ) );
        nErr = 0;
    }

    return Py_BuildValue( "i", nErr );
}

static PyObject *
gv_get_type_name(PyObject *self, PyObject *args)
{
    PyObject *check_obj;

    if (!PyArg_ParseTuple( args, "O:gv_get_type_name", &check_obj )) {
	return NULL;
    }

    return Py_BuildValue("s", gtk_type_name(GTK_OBJECT_TYPE(PyGtk_Get(check_obj))));
}

/*
 * Generated wrapper functions
 */

#include "gvmodule_impl.c"

/*
 * Method defs
 */

static PyMethodDef gv_methods[] =
{
    {"gv_test_entry", _wrap_gv_test_entry, 1},
    {"gv_shape_new", _wrap_gv_shape_new, 1},
    {"gv_shape_from_xml", _wrap_gv_shape_from_xml, 1},
    {"gv_shape_to_xml", _wrap_gv_shape_to_xml, 1},
    {"gv_shape_destroy", _wrap_gv_shape_destroy, 1},
    {"gv_shape_ref", _wrap_gv_shape_ref, 1},
    {"gv_shape_unref", _wrap_gv_shape_unref, 1},
    {"gv_shape_get_ref", _wrap_gv_shape_get_ref, 1},
    {"gv_shape_copy", _wrap_gv_shape_copy, 1},
    {"gv_shape_line_from_nodelists",_wrap_gv_shape_line_from_nodelists, 1},
    {"gv_shapes_lines_for_vecplot",_wrap_gv_shapes_lines_for_vecplot, 1},
    {"gv_shape_get_property", _wrap_gv_shape_get_property, 1},
    {"gv_shape_get_properties", _wrap_gv_shape_get_properties, 1},
    {"gv_shape_get_typed_properties", _wrap_gv_shape_get_typed_properties, 1},
    {"gv_shape_set_property", _wrap_gv_shape_set_property, 1},
    {"gv_shape_set_properties", _wrap_gv_shape_set_properties, 1},
    {"gv_shape_get_rings", _wrap_gv_shape_get_rings, 1},
    {"gv_shape_get_nodes", _wrap_gv_shape_get_nodes, 1},
    {"gv_shape_add_node", _wrap_gv_shape_add_node, 1},
    {"gv_shape_set_node", _wrap_gv_shape_set_node, 1},
    {"gv_shape_get_node", _wrap_gv_shape_get_node, 1},
    {"gv_shape_get_type", _wrap_gv_shape_get_type, 1},
    {"gv_shape_point_in_polygon", _wrap_gv_shape_point_in_polygon, 1},
    {"gv_shape_distance_from_polygon", _wrap_gv_shape_distance_from_polygon, 1},
    {"gv_shape_clip_to_rect", _wrap_gv_shape_clip_to_rect, 1},
    {"gv_shape_add_shape", _wrap_gv_shape_add_shape, 1},
    {"gv_shape_get_shape", _wrap_gv_shape_get_shape, 1},
    {"gv_shape_collection_get_count", _wrap_gv_shape_collection_get_count, 1},
    {"gv_symbol_manager_get_names", _wrap_gv_symbol_manager_get_names, 1 },
    {"gv_symbol_manager_inject_vector_symbol",_wrap_gv_symbol_manager_inject_vector_symbol, 1},
    {"gv_symbol_manager_inject_raster_symbol",_wrap_gv_symbol_manager_inject_raster_symbol, 1},
    {"gv_symbol_manager_get_symbol", _wrap_gv_symbol_manager_get_symbol, 1},
    {"gv_symbol_manager_save_vector_symbol", _wrap_gv_symbol_manager_save_vector_symbol, 1},
    {"gv_data_get_properties", _wrap_gv_data_get_properties, 1},
    {"gv_data_set_properties", _wrap_gv_data_set_properties, 1},
    {"gv_data_changed", _wrap_gv_data_changed, 1},
    {"gv_layer_extents", _wrap_gv_layer_extents, 1},
    {"gv_layer_display_change", _wrap_gv_layer_display_change, 1},
    {"gv_view_area_list_layers", _wrap_gv_view_area_list_layers, 1},
    {"gv_view_area_get_translation", _wrap_gv_view_area_get_translation, 1},
    {"gv_view_area_map_location", _wrap_gv_view_area_map_location, 1},
    {"gv_view_area_get_pointer", _wrap_gv_view_area_get_pointer, 1},
    {"gv_view_area_map_pointer", _wrap_gv_view_area_map_pointer, 1},
    {"gv_view_area_get_extents", _wrap_gv_view_area_get_extents, 1 },
    {"gv_view_area_get_volume", _wrap_gv_view_area_get_volume, 1 },
    {"gv_view_area_inverse_map_pointer", _wrap_gv_view_area_inverse_map_pointer, 1},
    {"gv_view_area_get_fontnames", _wrap_gv_view_area_get_fontnames, 1},
    {"gv_view_area_set_background_color", _wrap_gv_view_area_set_background_color, 1},
    {"gv_view_area_get_background_color", _wrap_gv_view_area_get_background_color, 1},
    {"gv_view_area_create_thumbnail", _wrap_gv_view_area_create_thumbnail, 1},
    {"gv_view_area_set_3d_view", _wrap_gv_view_area_set_3d_view, 1 },
    {"gv_view_area_set_3d_view_look_at", _wrap_gv_view_area_set_3d_view_look_at, 1 },
    {"gv_view_area_get_eye_pos", _wrap_gv_view_area_get_eye_pos, 1 },
    {"gv_view_area_get_eye_dir", _wrap_gv_view_area_get_eye_dir, 1 },
    {"gv_view_area_get_look_at_pos", _wrap_gv_view_area_get_look_at_pos, 1 },
    {"gv_roi_tool_get_rect", _wrap_gv_roi_tool_get_rect, 1},
    {"gv_roi_tool_new_rect", _wrap_gv_roi_tool_new_rect, 1},
    {"gv_poi_tool_get_point", _wrap_gv_poi_tool_get_point, 1},
    {"gv_poi_tool_new_point", _wrap_gv_poi_tool_new_point, 1},
    {"gv_tool_set_boundary", _wrap_gv_tool_set_boundary, 1},
    {"gv_records_set_used_properties", _wrap_gv_records_set_used_properties, 1 },
    {"gv_records_get_typed_properties", _wrap_gv_records_get_typed_properties, 1},
    {"gv_records_get_properties", _wrap_gv_records_get_properties, 1},
    {"gv_records_to_dbf", _wrap_gv_records_to_dbf, 1},
    {"gv_records_MultiStratifiedCollect", _wrap_gv_records_MultiStratifiedCollect, 1},
    {"gv_records_recode", _wrap_gv_records_recode, 1},
    {"gv_records_asdict", _wrap_gv_records_asdict, 1},
    {"gv_shapes_from_ogr_layer", _wrap_gv_shapes_from_ogr_layer, 1},
    {"gv_shapes_get_shape", _wrap_gv_shapes_get_shape, 1},
    {"gv_shapes_add_shape", _wrap_gv_shapes_add_shape, 1},
    {"gv_shapes_add_shape_last", _wrap_gv_shapes_add_shape_last, 1},
    {"gv_shapes_delete_shapes", _wrap_gv_shapes_delete_shapes, 1},
    {"gv_shapes_replace_shapes", _wrap_gv_shapes_replace_shapes, 1},
    {"gv_shapes_get_extents", _wrap_gv_shapes_get_extents, 1},
    {"gv_shapes_get_change_info", _wrap_gv_shapes_get_change_info, 1},
    {"gv_points_get_point", _wrap_gv_points_get_point, 1},
    {"gv_points_new_point", _wrap_gv_points_new_point, 1},
    {"gv_polylines_get_line", _wrap_gv_polylines_get_line, 1},
    {"gv_polylines_new_line", _wrap_gv_polylines_new_line, 1},
    {"gv_areas_get_area", _wrap_gv_areas_get_area, 1},
    {"gv_areas_new_area", _wrap_gv_areas_new_area, 1},
    {"gv_shape_layer_set_color", _wrap_gv_shape_layer_set_color, 1},
    {"gv_shape_layer_get_selected", _wrap_gv_shape_layer_get_selected, 1},
    {"gv_shape_layer_pick_shape", _wrap_gv_shape_layer_pick_shape, 1 },
    {"gv_raster_new", (PyCFunction)_wrap_gv_raster_new, METH_VARARGS|METH_KEYWORDS},
    {"gv_raster_autoscale", _wrap_gv_raster_autoscale, 1},
    {"gv_raster_get_gdal_band", _wrap_gv_raster_get_gdal_band, 1},
    {"gv_raster_get_gdal_dataset", _wrap_gv_raster_get_gdal_dataset, 1},
    {"gv_raster_force_load", _wrap_gv_raster_force_load, 1 },
    {"gv_raster_get_sample", _wrap_gv_raster_get_sample, 1},
    {"gv_raster_georef_to_pixel", _wrap_gv_raster_georef_to_pixel, 1},
    {"gv_raster_pixel_to_georef", _wrap_gv_raster_pixel_to_georef, 1},
    {"gv_raster_georef_to_pixelCL", _wrap_gv_raster_georef_to_pixelCL, 1},
    {"gv_raster_pixel_to_georefCL", _wrap_gv_raster_pixel_to_georefCL, 1},
    {"gv_raster_data_changing", _wrap_gv_raster_data_changing, 1},
    {"gv_raster_data_changed", _wrap_gv_raster_data_changed, 1},
    {"gv_raster_get_change_info", _wrap_gv_raster_get_change_info, 1},
    {"gv_raster_get_gcps", _wrap_gv_raster_get_gcps, 1},
    {"gv_raster_set_gcps", _wrap_gv_raster_set_gcps, 1},
    {"gv_raster_get_gcpsCL", _wrap_gv_raster_get_gcpsCL, 1},
    {"gv_raster_set_gcpsCL", _wrap_gv_raster_set_gcpsCL, 1},
    {"gv_raster_layer_new", _wrap_gv_raster_layer_new, 1},
    {"gv_raster_layer_autoscale_view", _wrap_gv_raster_layer_autoscale_view,1},
    {"gv_raster_layer_get_mesh_lod", _wrap_gv_raster_layer_get_mesh_lod,1},
    {"gv_raster_layer_histogram_view", _wrap_gv_raster_layer_histogram_view,1},
    {"gv_raster_layer_view_to_pixel", _wrap_gv_raster_layer_view_to_pixel, 1},
    {"gv_raster_layer_pixel_to_view", _wrap_gv_raster_layer_pixel_to_view, 1},
    {"gv_raster_layer_set_source", _wrap_gv_raster_layer_set_source, 1},
    {"gv_raster_layer_texture_mode_set", _wrap_gv_raster_layer_texture_mode_set, 1},
    {"gv_raster_layer_texture_mode_get", _wrap_gv_raster_layer_texture_mode_get, 1},
    {"gv_raster_layer_alpha_get", _wrap_gv_raster_layer_alpha_get, 1},
    {"gv_raster_layer_blend_mode_get", _wrap_gv_raster_layer_blend_mode_get, 1},
    {"gv_raster_layer_lut_put", _wrap_gv_raster_layer_lut_put, 1},
    {"gv_raster_layer_lut_get", _wrap_gv_raster_layer_lut_get, 1},
    {"gv_raster_layer_nodata_get", _wrap_gv_raster_layer_nodata_get,1},
    {"gv_raster_layer_get_nodata", _wrap_gv_raster_layer_get_nodata,1},
    {"gv_raster_layer_source_get_lut", _wrap_gv_raster_layer_source_get_lut,1},
    {"gv_raster_layer_zoom_get", _wrap_gv_raster_layer_zoom_get, 1},
    {"gv_raster_layer_get_height", _wrap_gv_raster_layer_get_height, 1},
    {"gv_manager_get_preferences", _wrap_gv_manager_get_preferences, 1},
    {"gv_manager_add_dataset", _wrap_gv_manager_add_dataset, 1},
    {"gv_manager_get_dataset", _wrap_gv_manager_get_dataset, 1},
    {"gv_manager_get_dataset_raster", _wrap_gv_manager_get_dataset_raster, 1},
    {"gv_manager_queue_task", _wrap_gv_manager_queue_task, 1},
    {"gv_launch_url", _wrap_gv_launch_url, 1},
    {"gv_rgba_to_rgb", _wrap_gv_rgba_to_rgb, 1},
    {"WIDInterpolate", _wrap_WIDInterpolate, 1},
    {"gv_raster_rasterize_shapes", _wrap_gv_raster_rasterize_shapes, 1},
    {"gv_short_path_name", _wrap_gv_short_path_name, 1},
    {"gtk_color_well_get_d", _wrap_gtk_color_well_get_d, 1},
    {"gv_shapes_to_dbf", _wrap_gv_shapes_to_dbf, 1},
    {"gv_format_point_query", _wrap_gv_format_point_query, 1},
    {"MyGDALOperator", _wrap_MyGDALOperator, 1},
    {"gv_get_type_name", gv_get_type_name, 1},
#include "gvmodule_defs.c"
    {NULL, NULL, 0}
};

/*
 * Module initialization function
 */

void
init_gv(void)
{
    init_pygtk();
    init_pygobject();

    Py_InitModule("_gv", gv_methods);

    if (PyErr_Occurred())
    Py_FatalError("can't initialize module _gv");
}
