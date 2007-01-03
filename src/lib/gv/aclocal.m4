
dnl
dnl Major portions of this file taken from the pygtk aclocal.m4 file. 
dnl Other parts derived from libgeotiff, and GDAL. 
dnl

AC_DEFUN(AC_COMPILER_WFLAGS,
[
	# Remove -g from compile flags, we will add via CFG variable if
	# we need it.
	CXXFLAGS=`echo "$CXXFLAGS " | sed "s/-g //"`
	CFLAGS=`echo "$CFLAGS " | sed "s/-g //"`

	# check for GNU compiler, and use -Wall
	if test "$GCC" = "yes"; then
		C_WFLAGS="-Wall"
		AC_DEFINE(USE_GNUCC)
	fi
	if test "$GXX" = "yes"; then
		CXX_WFLAGS="-Wall"
		AC_DEFINE(USE_GNUCC)
	fi
	AC_SUBST(CXX_WFLAGS,$CXX_WFLAGS)
	AC_SUBST(C_WFLAGS,$C_WFLAGS)
])

AC_DEFUN(AC_COMPILER_PIC,
[
	echo 'void f(){}' > conftest.c
	if test -z "`${CC-cc} -fPIC -c conftest.c 2>&1`"; then
	  C_PIC=-fPIC
	else
	  C_PIC=
	fi
	if test -z "`${CXX-g++} -fPIC -c conftest.c 2>&1`"; then
	  CXX_PIC=-fPIC
	else
	  CXX_PIC=
	fi
	rm -f conftest*

	AC_SUBST(CXX_PIC,$CXX_PIC)
	AC_SUBST(C_PIC,$C_PIC)
])

AC_DEFUN(AC_LD_SHARED,
[
  echo 'int main(){ g(); return 0; }' > conftest1.c

  echo 'void g(){}' > conftest2.c
  ${CC} ${C_PIC} -c conftest2.c

  LD_SHARED="/bin/true"
  if test -z "`ld -shared conftest2.o -o libconftest.so 2>&1`" ; then
    if test -z "`${CC} conftest1.c libconftest.so -o conftest1 2>&1`"; then
      LD_LIBRARY_PATH_OLD="$LD_LIBRARY_PATH"
      LD_LIBRARY_PATH="`pwd`"
      export LD_LIBRARY_PATH
      if test -z "`./conftest1 2>&1`" ; then
        echo "checking for ld -shared ... yes"
        LD_SHARED="ld -shared"
      else
        echo "checking for ld -shared ... no"
      fi
      LD_LIBRARY_PATH="$LD_LIBRARY_PATH_OLD"
    else
      echo "checking for ld -shared ... no"
    fi
  else
    echo "checking for ld -shared ... no"
  fi
  rm -f conftest* libconftest* 

  AC_SUBST(LD_SHARED,$LD_SHARED)
])

AC_DEFUN(AM_PATH_PYTHON,
  [AC_CHECK_PROGS(PYTHON, python python2.2 python2.1 python2.0 python1.5,no)
  if test "$PYTHON" != no; then
    AC_MSG_CHECKING([where .py files should go])
changequote(, )dnl
    pythondir=`$PYTHON -c '
import sys
print "%s/lib/python%s/site-packages" % (sys.prefix, sys.version[:3])'`
changequote([, ])dnl
    AC_MSG_RESULT($pythondir)
    AC_MSG_CHECKING([where python extensions should go])
changequote(, )dnl
    pyexecdir=`$PYTHON -c '
import sys
if sys.version[0] > "1" or sys.version[2] > "4":
  print "%s/lib/python%s/site-packages" % (sys.exec_prefix, sys.version[:3])
else:
  print "%s/lib/python%s/sharedmodules" % (sys.exec_prefix, sys.version[:3])'`
changequote([, ])dnl
    AC_MSG_RESULT($pyexecdir)
  else
    # these defaults are version independent ...
    AC_MSG_CHECKING([where .py files should go])
    pythondir='$(prefix)/lib/site-python'
    AC_MSG_RESULT($pythondir)
    AC_MSG_CHECKING([where python extensions should go])
    pyexecdir='$(exec_prefix)/lib/site-python'
    AC_MSG_RESULT($pyexecdir)
  fi
  AC_SUBST(pythondir)
  AC_SUBST(pyexecdir)])


dnl AM_CHECK_PYMOD(MODNAME [,SYMBOL [,ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND]]])
dnl Check if a module containing a given symbol is visible to python.
AC_DEFUN(AM_CHECK_PYMOD,
[AC_REQUIRE([AM_PATH_PYTHON])
py_mod_var=`echo $1['_']$2 | sed 'y%./+-%__p_%'`
AC_MSG_CHECKING(for ifelse([$2],[],,[$2 in ])python module $1)
AC_CACHE_VAL(py_cv_mod_$py_mod_var, [
ifelse([$2],[], [prog="
import sys
try:
	import $1
except ImportError:
	sys.exit(1)
except:
	sys.exit(0)
sys.exit(0)"], [prog="
import $1
$1.$2"])
if $PYTHON -c "$prog" 1>&AC_FD_CC 2>&AC_FD_CC
  then
    eval "py_cv_mod_$py_mod_var=yes"
  else
    eval "py_cv_mod_$py_mod_var=no"
  fi
])
py_val=`eval "echo \`echo '$py_cv_mod_'$py_mod_var\`"`
if test "x$py_val" != xno; then
  AC_MSG_RESULT(yes)
  ifelse([$3], [],, [$3
])dnl
else
  AC_MSG_RESULT(no)
  ifelse([$4], [],, [$4
])dnl
fi
])

# AM_PATH_GTKGL([ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND]])
AC_DEFUN(AM_PATH_GTKGL,
[
AC_REQUIRE([AM_PATH_GTK])
AC_PROVIDE([AM_PATH_GTKGL])

AC_ARG_WITH(gl-prefix,    [  --with-gl-prefix=PFX   Prefix where OpenGL or Mesa is installed],
 gl_prefix="$withval",
 gl_prefix="")

AC_ARG_WITH(gtkgl-prefix, [  --with-gtkgl-prefix=PFX Prefix where GtkGLArea is installed],
 gtkgl_prefix="$withval",
 gtkgl_prefix="")



if test x$gl_prefix != x ; then
 GL_CFLAGS="-I$gl_prefix/include"
 GL_LDOPTS="-L$gl_prefix/lib"
else
 GL_CFLAGS=""
 GL_LDOPTS=""
fi

saved_LIBS="$LIBS"
saved_CFLAGS="$CFLAGS"

# test for plain OpenGL
AC_MSG_CHECKING([GL])
LIBS="$GTK_LIBS $GL_LDOPTS -lGLU -lGL $saved_LIBS"
AC_TRY_LINK( ,[ char glBegin(); glBegin(); ], have_GL=yes, have_GL=no)
AC_MSG_RESULT($have_GL)

if test x$have_GL = xyes; then

 GL_LIBS="-lGLU -lGL"

else

 # test for Mesa
 AC_MSG_CHECKING([Mesa])
 LIBS="$saved_LIBS $GTK_LIBS $GL_LDOPTS -lMesaGLU -lMesaGL"
 AC_TRY_LINK( ,[ char glBegin(); glBegin(); ], have_Mesa=yes, have_Mesa=no)
 AC_MSG_RESULT($have_Mesa)

 if test x$have_Mesa = xyes; then

  GL_LIBS="-lMesaGLU -lMesaGL"

 else

  # test for Mesa with threads
  AC_MSG_CHECKING([Mesa with pthreads])
  LIBS="$GTK_LIBS $GL_LDOPTS -lMesaGL -lMesaGLU -lpthread $saved_LIBS"
  AC_TRY_LINK( ,[ char glBegin(); glBegin(); ], have_Mesa_pthread=yes, have_Mesa_pthread=no)
  AC_MSG_RESULT($have_Mesa_pthread)

  if test x$have_Mesa_pthread = xyes; then
    
    GL_LIBS="-lMesaGLU -lMesaGL -lpthread"

  else

   #all failed
   LIBS="$saved_LIBS"
   CFLAGS="$saved_CFLAGS"
   GTKGL_LIBS=""
   GTKGL_CFLAGS=""
   ifelse([$2], , :, [$2])

  fi
 fi
fi


if test x$gtkgl_prefix != x; then
 GTKGL_CFLAGS="-I$gtkgl_prefix/include"
 GTKGL_LDOPTS="-L$gtkgl_prefix/lib"
else
 GTKGL_CFLAGS=""
 GTKGL_LDOPTS=""
fi

AC_MSG_CHECKING([GtkGLArea])
LIBS="$GTK_LIBS $GL_LDOPTS -lgtkgl $GL_LIBS $GTKGL_LDOPTS $saved_LIBS"
AC_TRY_LINK( ,[ char gtk_gl_area_new(); gtk_gl_area_new(); ], have_gtkgl=yes, have_gtkgl=no)
AC_MSG_RESULT($have_gtkgl)

if test x$have_gtkgl = xyes; then

 LIBS="$saved_LIBS"
 CFLAGS="$saved_CFLAGS"
 GTKGL_CFLAGS="$GTKGL_CFLAGS $GL_CFLAGS"
 GTKGL_LIBS="$GTKGL_LDOPTS -lgtkgl $GL_LDOPTS $GL_LIBS"
 ifelse([$1], , :, [$1])

else

 LIBS="$saved_LIBS"
 CFLAGS="$saved_CFLAGS"
 GTKGL_LIBS=""
 GTKGL_CFLAGS=""
 ifelse([$2], , :, [$2])

fi

AC_SUBST(GTKGL_CFLAGS)
AC_SUBST(GTKGL_LIBS)

])

# Configure paths for GTK+
# Owen Taylor     97-11-3

dnl AM_PATH_GTK([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GTK, and define GTK_CFLAGS and GTK_LIBS
dnl
AC_DEFUN(AM_PATH_GTK,
[dnl 
dnl Get the cflags and libraries from the gtk-config script
dnl
AC_ARG_WITH(gtk-prefix,[  --with-gtk-prefix=PFX   Prefix where GTK is installed (optional)],
            gtk_config_prefix="$withval", gtk_config_prefix="")
AC_ARG_WITH(gtk-exec-prefix,[  --with-gtk-exec-prefix=PFX Exec prefix where GTK is installed (optional)],
            gtk_config_exec_prefix="$withval", gtk_config_exec_prefix="")
AC_ARG_ENABLE(gtktest, [  --disable-gtktest       Do not try to compile and run a test GTK program],
		    , enable_gtktest=yes)

  for module in . $4
  do
      case "$module" in
         gthread) 
             gtk_config_args="$gtk_config_args gthread"
         ;;
      esac
  done

  if test x$gtk_config_exec_prefix != x ; then
     gtk_config_args="$gtk_config_args --exec-prefix=$gtk_config_exec_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_exec_prefix/bin/gtk-config
     fi
  fi
  if test x$gtk_config_prefix != x ; then
     gtk_config_args="$gtk_config_args --prefix=$gtk_config_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_prefix/bin/gtk-config
     fi
  fi

  AC_PATH_PROG(GTK_CONFIG, gtk-config, no)
  min_gtk_version=ifelse([$1], ,0.99.7,$1)
  AC_MSG_CHECKING(for GTK - version >= $min_gtk_version)
  no_gtk=""
  if test "$GTK_CONFIG" = "no" ; then
    no_gtk=yes
  else
    GTK_CFLAGS=`$GTK_CONFIG $gtk_config_args --cflags`
    GTK_LIBS=`$GTK_CONFIG $gtk_config_args --libs`
    gtk_config_major_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gtk_config_minor_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gtk_config_micro_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gtktest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GTK_CFLAGS"
      LIBS="$GTK_LIBS $LIBS"
dnl
dnl Now check if the installed GTK is sufficiently new. (Also sanity
dnl checks the results of gtk-config to some extent
dnl
      rm -f conf.gtktest
      AC_TRY_RUN([
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gtktest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gtk_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gtk_version");
     exit(1);
   }

  if ((gtk_major_version != $gtk_config_major_version) ||
      (gtk_minor_version != $gtk_config_minor_version) ||
      (gtk_micro_version != $gtk_config_micro_version))
    {
      printf("\n*** 'gtk-config --version' returned %d.%d.%d, but GTK+ (%d.%d.%d)\n", 
             $gtk_config_major_version, $gtk_config_minor_version, $gtk_config_micro_version,
             gtk_major_version, gtk_minor_version, gtk_micro_version);
      printf ("*** was found! If gtk-config was correct, then it is best\n");
      printf ("*** to remove the old version of GTK+. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If gtk-config was wrong, set the environment variable GTK_CONFIG\n");
      printf("*** to point to the correct copy of gtk-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
#if defined (GTK_MAJOR_VERSION) && defined (GTK_MINOR_VERSION) && defined (GTK_MICRO_VERSION)
  else if ((gtk_major_version != GTK_MAJOR_VERSION) ||
	   (gtk_minor_version != GTK_MINOR_VERSION) ||
           (gtk_micro_version != GTK_MICRO_VERSION))
    {
      printf("*** GTK+ header files (version %d.%d.%d) do not match\n",
	     GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     gtk_major_version, gtk_minor_version, gtk_micro_version);
    }
#endif /* defined (GTK_MAJOR_VERSION) ... */
  else
    {
      if ((gtk_major_version > major) ||
        ((gtk_major_version == major) && (gtk_minor_version > minor)) ||
        ((gtk_major_version == major) && (gtk_minor_version == minor) && (gtk_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GTK+ (%d.%d.%d) was found.\n",
               gtk_major_version, gtk_minor_version, gtk_micro_version);
        printf("*** You need a version of GTK+ newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GTK+ is always available from ftp://ftp.gtk.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the gtk-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GTK+, but you can also set the GTK_CONFIG environment to point to the\n");
        printf("*** correct copy of gtk-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_gtk=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gtk" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GTK_CONFIG" = "no" ; then
       echo "*** The gtk-config script installed by GTK could not be found"
       echo "*** If GTK was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GTK_CONFIG environment variable to the"
       echo "*** full path to gtk-config."
     else
       if test -f conf.gtktest ; then
        :
       else
          echo "*** Could not run GTK test program, checking why..."
          CFLAGS="$CFLAGS $GTK_CFLAGS"
          LIBS="$LIBS $GTK_LIBS"
          AC_TRY_LINK([
#include <gtk/gtk.h>
#include <stdio.h>
],      [ return ((gtk_major_version) || (gtk_minor_version) || (gtk_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GTK or finding the wrong"
          echo "*** version of GTK. If it is not finding GTK, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the GTK package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps gtk gtk-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GTK was incorrectly installed"
          echo "*** or that you have moved GTK since it was installed. In the latter case, you"
          echo "*** may want to edit the gtk-config script: $GTK_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GTK_CFLAGS=""
     GTK_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
  rm -f conf.gtktest
])


# serial 1

dnl finds information needed for compilation of shared library style python
dnl extensions.  AM_PATH_PYTHON should be called before hand.
dnl NFW: Modified from original to avoid overridding CC, SO and OPT
AC_DEFUN(AM_INIT_PYEXEC_MOD,
  [AC_REQUIRE([AM_PATH_PYTHON])
  AC_MSG_CHECKING([for python headers])
  AC_CACHE_VAL(am_cv_python_includes,
    [changequote(,)dnl
    am_cv_python_includes="`$PYTHON -c '
import sys
includepy = \"%s/include/python%s\" % (sys.prefix, sys.version[:3])
if sys.version[0] > \"1\" or sys.version[2] > \"4\":
  libpl = \"%s/include/python%s\" % (sys.exec_prefix, sys.version[:3])
else:
  libpl = \"%s/lib/python%s/config\" % (sys.exec_prefix, sys.version[:3])
print \"-I%s -I%s\" % (includepy, libpl)'`"
    changequote([, ])])
  PYTHON_INCLUDES="$am_cv_python_includes"
  AC_MSG_RESULT(found)
  AC_SUBST(PYTHON_INCLUDES)

  AC_MSG_CHECKING([definitions from Python makefile])
changequote(, )dnl
  py_makefile="`$PYTHON -c '
import sys
print \"%s/lib/python%s/config/Makefile\"%(sys.exec_prefix, sys.version[:3])'`"
changequote([, ])dnl

  if test ! -f "$py_makefile"; then
      AC_MSG_ERROR([*** Couldnt find the python config makefile.  Maybe you are
*** missing the development portion of the python installation])
  fi
  eval `sed -n \
-e "s/^CC=[ 	]*\(.*\)/am_cv_python_CC='\1'/p" \
-e "s/^OPT=[ 	]*\(.*\)/am_cv_python_OPT='\1'/p" \
-e "s/^CCSHARED=[ 	]*\(.*\)/am_cv_python_CCSHARED='\1'/p" \
-e "s/^LDSHARED=[ 	]*\(.*\)/am_cv_python_LDSHARED='\1'/p" \
-e "s/^SO=[ 	]*\(.*\)/am_cv_python_SO='\1'/p" \
    $py_makefile`

  AC_MSG_RESULT(done)
  PYTHON_CC="$am_cv_python_CC"
  PYTHON_OPT="$am_cv_python_OPT"
  PYTHON_SO="$am_cv_python_SO"
  PYTHON_CFLAGS="$am_cv_python_CCSHARED \$(OPT)"
  PYTHON_LINK="$am_cv_python_LDSHARED -o \[$]@"

  AC_SUBST(PYTHON_CC)dnl
  AC_SUBST(PYTHON_OPT)dnl
  AC_SUBST(PYTHON_SO)dnl
  AC_SUBST(PYTHON_CFLAGS)dnl
  AC_SUBST(PYTHON_LINK)])
