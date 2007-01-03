license = ''

import sys, os
from glob import glob
isfile = os.path.isfile
from types import StringType

# copy all of the resource directory to PREFIX/share/openev2
# copy pymod to site-packages/openev2
# copy libs to site-packages
# what to do with tools?
# need to set OPENEV_HOME=PREFIX/share/openev2



# BEFORE importing distutils, remove MANIFEST. distutils doesn't properly
# update it when the contents of directories change.
if os.path.exists('MANIFEST'): os.remove('MANIFEST')


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

# Make certain that the CURRENTARCH variable is set
current_arch = os.environ.get('CURRENTARCH')
if current_arch is None:
    print 'Must set the CURRENTARCH variable'
    sys.exit(1)


from distutils.core import setup, Extension
from distutils.command.build_ext import build_ext
from distutils.util import change_root, convert_path
from distutils.dir_util import mkpath
from distutils.errors import DistutilsFileError

# Script to be run by the windows binary installer after the default setup
# routine, to add shortcuts and similar windows-only things.  Windows
# post-install scripts MUST reside in the scripts/ dir, otherwise distutils
# doesn't find them.
if 'bdist_wininst' in sys.argv:
    if len(sys.argv) > 2 and ('sdist' in sys.argv or 'bdist_rpm' in sys.argv):
        print >> sys.stderr,"ERROR: bdist_wininst must be run alone. Exiting."
        sys.exit(1)

import pdb
class no_build_ext(build_ext):
    def build_extensions(self):
        # just copy the files already created to 
        # build/lib.<arch>

        ##pdb.set_trace()
        path = os.path.join('output/lib/lib.' + current_arch)
        for ext in self.extensions:
            fullname = self.get_ext_filename(ext.name)
            src = convert_path(os.path.join(path, fullname))
            dest = self.build_lib
            self.copy_file(src, dest)

html_data = glob('resource/html/*')
html_data.remove('resource/html/developer_info')

# Call the setup() routine which does most of the work
setup(name             = 'openev',
      version          = '2.0.0',
      description      = '',
      long_description = '',
      author           = 'Vexcel Corporation',
      author_email     = '',
      url              = '',
      license          = license,
      platforms        = '',
      keywords         = '',
      cmdclass         = {'build_ext':no_build_ext},
      ext_modules      = [Extension('_gv', [])],
      scripts          = ['resource/scripts/openev'],
      packages         = ['openev'],
      package_dir      = {'openev':'src/pymod'},
      data_files       = [('share/openev/data', glob('resource/data/*')),
                          ('share/openev/html', html_data),
                          ('share/openev/html/developer_info', glob('resource/html/developer_info/*')),
                          ('share/openev/pics', glob('resource/pics/*')),
                          ('share/openev/ramps', glob('resource/ramps/*')),
                          ('share/openev/scripts', glob('resource/scripts/*')),
                          ('share/openev/symbols', glob('resource/symbols/*')),
                          ('share/openev/xmlconfig', glob('resource/xmlconfig/*')),
                          ('share/openev/tools', glob('src/pymod/tools/*')),
     ] 
      )

