import sys, os
from glob import glob

# copy all of the resource directory to PREFIX/share/openev2
# copy pymod to site-packages/openev2
# copy libs to site-packages
# what to do with tools?
# need to set OPENEV_HOME=PREFIX/share/openev2



# BEFORE importing distutils, remove MANIFEST. distutils doesn't properly
# update it when the contents of directories change.
if os.path.exists('MANIFEST'):
    os.remove('MANIFEST')

if os.name == 'posix':
    os_name = 'posix'
elif os.name in ['nt','dos']:
    os_name = 'windows'
else:
    print 'Unsupported operating system:',os.name
    sys.exit(1)

# Under Windows, 'sdist' is not supported, since it requires lyxport (and
# hence lyx,perl,latex,pdflatex,latex2html,sh,...)
if os_name == 'windows' and sys.argv[1] == 'sdist':
    print 'The sdist command is not available under Windows.  Exiting.'
    sys.exit(1)

from distutils.core import setup, Extension
from distutils.util import change_root, convert_path

# borrowed from pygtk dsextras.py
def getoutput(cmd):
    """Return output (stdout or stderr) of executing cmd in a shell."""
    return getstatusoutput(cmd)[1]

def getstatusoutput(cmd):
    """Return (status, output) of executing cmd in a shell."""
    if sys.platform == 'win32':
        pipe = os.popen(cmd, 'r')
        text = pipe.read()
        sts = pipe.close() or 0
        if text[-1:] == '\n':
            text = text[:-1]
        return sts, text
    else:
        from commands import getstatusoutput
        return getstatusoutput(cmd)

def have_pkgconfig():
    """Checks for the existence of pkg-config"""
    if (sys.platform == 'win32' and
        os.system('pkg-config --version > NUL') == 0):
        return 1
    else:
        if getstatusoutput('pkg-config')[0] == 256:
            return 1

def pkgc_version_check(name, req_version):
    """Check the existence and version number of a package:
    returns 0 if not installed or too old, 1 otherwise."""
    is_installed = not os.system('pkg-config --exists %s' % name)
    if not is_installed:
        return 0

    orig_version = getoutput('pkg-config --modversion %s' % name)
    version = map(int, orig_version.split('.'))
    pkc_version = map(int, req_version.split('.'))

    if version >= pkc_version:
        return 1
        
    return 0

def get_config(names, cmd):
    retval = []
    for name in names:
        output = getoutput(cmd % name)
        for s in output.replace(cmd[-5:-3], '').split():
            if s not in retval:
                retval.append(s)
    return retval

def get_include_dirs(names):
    return get_config(names, 'pkg-config --cflags-only-I %s')

def get_libraries(names):
    return get_config(names, 'pkg-config --libs-only-l %s')

def get_library_dirs(names):
    return get_config(names, 'pkg-config --libs-only-L %s')

# Script to be run by the windows binary installer after the default setup
# routine, to add shortcuts and similar windows-only things.  Windows
# post-install scripts MUST reside in the scripts/ dir, otherwise distutils
# doesn't find them.
if 'bdist_wininst' in sys.argv:
    if len(sys.argv) > 2 and ('sdist' in sys.argv or 'bdist_rpm' in sys.argv):
        print >> sys.stderr,"ERROR: bdist_wininst must be run alone. Exiting."
        sys.exit(1)

html_data = glob('resource/html/*')

# check for pkgconfig first, no point in going further if not present...
if not have_pkgconfig():
    print 'pkgconfig required, exiting...'
    sys.exit(1)

# check for gtkglext
if not pkgc_version_check('gtkglext-1.0', '1.0'):
    print 'gtkglext 1.0 or higher required, exiting'
    sys.exit(1)

# check for GTK version, assuming that its dependencies are present if GTK is
if not pkgc_version_check('gtk+-2.0', '2.6'):
    print 'GTK+ 2.6 or higher required, exiting'
    sys.exit(1)

gtk = ['gtk+-2.0','atk','pango','gdk-2.0','glib-2.0','gtkglext-1.0']

# check for pygtk, assuming it matches GTK
try:
    import pygtk
except ImportError:
    print 'pygtk not found, exiting' 
    sys.exit(1)

# check for GDAL
ret = getstatusoutput('gdalinfo --version')
if ret[0]:
    print 'GDAL not found, exiting'
    sys.exit(1)

gv_root = 'src/lib/gv'
gv_srcs = ['crs.c', 'dbfopen.c', 'gextra.c', 'gvareatool.c', 'gvdata.c',
          'gvlayer.c', 'gvlinetool.c', 'gvmanager.c', 'gvmarshal.c',
          'gvmesh.c', 'gvnodetool.c', 'gvogr.c', 'gv_override.c',
          'gvpointtool.c', 'gvpoitool.c', 'gvpquerylayer.c', 'gvprint.c',
          'gvproperties.c', 'gvraster.c', 'gvrasteraverage.c',
          'gvrastercache.c', 'gvrasterconvert.c', 'gvrasterize.c',
          'gvrasterlayer.c', 'gvrasterlut.c', 'gvrastersource.c',
          'gvrecttool.c', 'gvrenderinfo.c', 'gvroitool.c', 'gvrotatetool.c',
          'gvselecttool.c', 'gvshape.c', 'gvshapefile.c', 'gvshapelayer.c',
          'gvshapes.c', 'gvshapeslayer.c', 'gvskirt.c', 'gvsymbolmanager.c',
          'gvtessshape.c', 'gvtexturecache.c', 'gvtool.c', 'gvtoolbox.c',
          'gvtracktool.c', 'gvundo.c', 'gvutils.c', 'gvviewarea.c',
          'gvviewlink.c', 'gvwinprint.c', 'gvzoompantool.c', 'invdistance.c',
          'llrasterize.c', 'shpopen.c', 'gv_pwrap.c', 'gv-enum-types.c', 'gvmodule.c']

gv_srcs = [os.path.join(gv_root, src) for src in gv_srcs]

includes = [gv_root, 'resource']
# GDAL includes
output = getoutput('gdal-config --cflags')
includes.extend(output.replace('-I', '').split())

# GTK/PyGTK includes
includes.extend(get_include_dirs(gtk))

# find a way to get pygtk include dir, for now use most likely location on linux
includes.append('/usr/include/pygtk-2.0')

lib_dirs = get_library_dirs(gtk)
# GDAL libs dirs
dir,lib = getoutput('gdal-config --libs').split()
if dir[2:] not in lib_dirs:
    lib_dirs.append(dir[2:])

libs = [lib[2:]]
# GTK link libs
libs.extend(get_libraries(gtk))

_gv = Extension('openev._gv', gv_srcs,
                include_dirs=includes,
                define_macros=[('HAVE_OGR',1)],
                library_dirs=lib_dirs,
                libraries=libs
                )

# Call the setup() routine which does most of the work
setup(name             = 'openev',
      version          = '2.1.0',
      description      = '',
      long_description = '',
      author           = 'Vexcel Corporation',
      author_email     = '',
      maintainer       = 'Mario Beauchamp',
      maintainer_email = 'starged@gmail.com',
      url              = 'http://openev.sourceforge.net',
      license          = 'LGPL',
      platforms        = 'UNIX/Linux, Windows',
      keywords         = '',
      ext_modules      = [_gv],
      scripts          = ['resource/scripts/openev2'],
      packages         = ['openev'],
      package_dir      = {'openev':'src/pymod'}, 
      data_files       = [('share/openev/pics', glob('resource/pics/*')),
                          ('share/openev/ramps', glob('resource/ramps/*')),
                          ('share/openev/symbols', glob('resource/symbols/*')),
                          ('share/openev/xmlconfig', glob('resource/xmlconfig/*')),
                          ('share/openev/tools', glob('src/pymod/tools/*.py'))
                          ]
      )

