/*
 * gv_override.c - Contains utility functions used in the wrapper code (gv.c)
 *
 * These functions are not themselves wrapped!
 *
 * To create a new wrapper function that does not correspond to a c function,
 * add a prototype to gvmodule.h and then override that function with an
 * override../_wrap.. block in gv.override
 *
 * If a c function does exist but did not autowrap, it can be wrapped by
 * just adding an override block in gv.override.  The autowrap code is
 * finicky, you might first try to clean up the prototype (no spaces next
 * to parenthesis, make sure variable names are included).
 */

/* GTK2 Port */
#include <pygobject.h>

#if defined(WIN32) || defined(_WIN32)
#  include <pygtk.h>
#else
#  include <pygtk/pygtk.h>
#endif

#include "gv_override.h"
#include "cpl_conv.h"
#include "cpl_string.h"

/*
 * This is a function for extracting a raw pointer from a SWIG pointer
 * string.
 */
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

void SWIG_SimpleMakePtr(char *_c, const void *_ptr, char *type)
{
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

/************************************************************************/
/*                          PyProgressProxy()                           */
/*                                                                      */
/*      Copied from gdal.i                                              */
/************************************************************************/

int PyProgressProxy(double dfComplete, const char *pszMessage, void *pData)
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

CPLXMLNode *PyListToXMLTree(PyObject *pyList)
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

PyObject *XMLTreeToPyList(CPLXMLNode *psTree)
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

int PyIdleTaskProxy(void *task_info)
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

PyObject *build_py_line(GArray *line)
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

GArray *build_gv_line(PyObject *pylist, int min_len)
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
