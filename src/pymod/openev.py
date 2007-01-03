#!/usr/bin/env python
###############################################################################
# $Id: openev.py,v 1.1.1.1 2005/04/18 16:38:35 uid1026 Exp $
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

import gviewapp
import gview
import gtk
import sys
import os
import getopt

# Force standard c settings for floating point (. rather than ,)
import locale
locale.setlocale(locale.LC_NUMERIC,'C')

#
# Activate derived band pixel functions used by application
#
try:
    import _vxdisplay
    _vxdisplay.RegisterDerivedBandPixelFunctions();
except:
    print "Ignoring vxdisplay functions"

def main():
    # get command line options and args
    # openev -m menufile -i iconfile -t toolfile image1 image2 ......
    (options, ifiles) = getopt.getopt(sys.argv[1:], 'm:i:t:p:')

    if os.path.isdir(os.path.join(gview.home_dir, 'xmlconfig')):
        mfile = 'DefaultMenuFile.xml'
        if not os.path.isfile(os.path.join(gview.home_dir, 'xmlconfig',mfile)):
            mfile = None

        ifile = 'DefaultIconFile.xml'
        if not os.path.isfile(os.path.join(gview.home_dir, 'xmlconfig',ifile)):
            ifile = None

        pfile = 'DefaultPyshellFile.xml'
        if not os.path.isfile(os.path.join(gview.home_dir, 'xmlconfig',pfile)):
            pfile = None

    else:
        mfile=None
        ifile=None
        pfile=None
        
    tfile = None

    for opt in options[0:]:
        if opt[0] == '-m':
            mfile=opt[1]
        elif opt[0] == '-i':
            ifile=opt[1]
        elif opt[0] == '-p':
            pfile=opt[1]
        elif opt[0] == '-t':
            tfile=opt[1]

    app = gviewapp.GViewApp(toolfile=tfile,menufile=mfile,iconfile=ifile,pyshellfile=pfile)
    gview.app = app
    app.subscribe('quit',gtk.main_quit)
    app.show_layerdlg()
    app.new_view(title=None)
    app.do_auto_imports()

    for item in ifiles:
        app.file_open_by_name(item)

    gtk.main()


if __name__ == '__main__':
    main()
    
