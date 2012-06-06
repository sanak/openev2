#!/usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  OpenEV Application Mainline
# Author:   Frank Warmerdam, warmerda@home.com
#
###############################################################################
# Copyright (c) 2000, Atlantis Scientific Inc. (www.atlsci.com)
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
# 
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
###############################################################################

import os
home_dir = os.environ['OPENEV_HOME']
import sys
import gviewapp
import gview
import pygtk
pygtk.require('2.0')
import gtk
import getopt

# Force standard c settings for floating point (. rather than ,)
import locale
locale.setlocale(locale.LC_NUMERIC,'C')

def set_debug():
    from osgeo import gdal
    gdal.SetConfigOption('CPL_DEBUG', 'ON')
    gdal.SetConfigOption('CPL_TIMESTAMP', 'ON')
    gdal.PushErrorHandler('CPLLoggingErrorHandler')

def main():
    # get command line options and args
    # openev -m menufile -i iconfile -t toolfile image1 image2 ......
    options, ifiles = getopt.getopt(sys.argv[1:], 'm:i:t:p:d')

    xml_dir = os.path.join(gview.home_dir, 'xmlconfig')
    if os.path.isdir(xml_dir):
        mfile = 'NewMenuFile.xml'
        if not os.path.isfile(os.path.join(xml_dir, mfile)):
            mfile = None

        ifile = 'NewIconFile.xml'
        if not os.path.isfile(os.path.join(xml_dir, ifile)):
            ifile = None

        pfile = 'DefaultPyshellFile.xml'
        if not os.path.isfile(os.path.join(xml_dir, pfile)):
            pfile = None

    else:
        mfile = None
        ifile = None
        pfile = None

    tfile = None

    for opt in options[0:]:
        if opt[0] == '-m':
            mfile = opt[1]
        elif opt[0] == '-i':
            ifile = opt[1]
        elif opt[0] == '-p':
            pfile = opt[1]
        elif opt[0] == '-t':
            tfile = opt[1]
        elif opt[0] == '-d':
            set_debug()

    gviewapp.add_stock_icons()
    app = gviewapp.GViewApp(toolfile=tfile, menufile=mfile, iconfile=ifile, pyshellfile=pfile)
    gview.app = app
    app.subscribe('quit', gtk.main_quit)
    app.new_view(title=None)
    app.do_auto_imports()

    for item in ifiles:
        app.file_open_by_name(item)

    gtk.main()

if __name__ == '__main__':
    main()

