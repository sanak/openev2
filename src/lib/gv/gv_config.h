/* Define if you don't have vprintf but do have _doprnt.  */
#undef HAVE_DOPRNT

/* Define if you have the vprintf function.  */
#undef HAVE_VPRINTF

/* Define if you have the ANSI C header files.  */
#undef STDC_HEADERS

/* Define if you have the <fcntl.h> header file.  */
#undef HAVE_FCNTL_H

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H

#undef HAVE_LIBDL 

#undef HAVE_DLFCN_H
#undef WORDS_BIGENDIAN

#define HAVE_BROKEN_GL_POINTS 1

#undef SHOW_TESS_LINES

/* Necessary for some unknown reason...  */
#ifdef _WIN32
#include <windows.h>
#endif