/*
** gluos.h - operating system dependencies for GLU
**
** $Header: /art-cvs/proj/ev2/src/lib/gv/gluos.h,v 1.2 2005/04/21 19:54:30 uid1026 Exp $
*/

#ifndef __evgluos_h__
#define __evgluos_h__

#ifdef __MINGW32__

/* Override Microsoft-specific keywords to build mingw dll */
#ifdef USE_DECLSPEC
#ifdef BUILD_MGW_DLL
#define GLAPI __declspec(dllexport)
#else
#define GLAPI __declspec(dllimport)
#endif
#else
#define GLAPI
#endif /* USE_DECLSPEC */
#define APIENTRY

/* Override Microsoft-specific keywords to build mingw dll 
#ifdef BUILD_MGW_DLL
#define GLAPI __declspec(dllexport)
#else
#define GLAPI __declspec(dllimport)
#endif
*/

#define APIENTRY

/* Add LONG_MAX if limits.h not found */
#include <limits.h>
#ifndef LONG_MAX
#define LONG_MAX 2147483647
#endif

#else

#ifdef _WIN32 

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOIME
#include <windows.h>

/* Disable warnings */
#pragma warning(disable : 4101)
#pragma warning(disable : 4244)
#pragma warning(disable : 4761)

#else

/* Disable Microsoft-specific keywords */
#define GLAPI
#define WINGDIAPI

#endif /* _WIN32 */

#endif /* __MINGW32__ */

#endif /*__evgluos_h__ */
