
#ifndef __GV_OVERRIDE_H__
#define __GV_OVERRIDE_H__

#include "cpl_conv.h"
#include "cpl_string.h"
#include "cpl_minixml.h"
#include "gvtypes.h"

/* >>>> Structure defs <<<< */

/*
 * gv_override.h - Define utility functions used in the wrapper code (gv.c)
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

/*
** Stuff to support progress reporting callbacks.
*/

typedef struct {
    PyObject *psPyCallback;
    PyObject *psPyCallbackData;
    int nLastReported;
} PyProgressData;

/* -------------------------------------------------------------------- */
/*      Idle Task C callback structure.                                 */
/* -------------------------------------------------------------------- */
typedef struct {
    PyObject *psPyCallback;
    PyObject *psPyCallbackData;
    PyThreadState *psThreadState;
} PyTaskData;

/* >>>> Macros <<<< */

#define PyGtk_New pygobject_new
#define PyGtk_Get pygobject_get

/* >>>> Function Prototypes <<<< */

void *SWIG_SimpleGetPtr(char *_c, char *_t);
void SWIG_SimpleMakePtr(char *_c, const void *_ptr, char *type);
int CPL_STDCALL PyProgressProxy(double dfComplete, const char *pszMessage, void *pData);
CPLXMLNode *PyListToXMLTree(PyObject *pyList);
PyObject *XMLTreeToPyList(CPLXMLNode *psTree);
int PyIdleTaskProxy(void *task_info);
PyObject *build_py_line(GArray *line);
GArray *build_gv_line(PyObject *pylist, int min_len);

#endif /* __GV_OVERRIDE_H__ */
