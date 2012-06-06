###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Methods for displaying HTML documentation.
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

import gview
import os
import os.path
import _gv
import pygtk
pygtk.require('2.0')
import gtk
from gtk.gdk import *
from gtk.keysyms import *
import gview
import gvutils

def GetBrowseCommand():
    if gview.get_preference('html_browser'):
        return gview.get_preference('html_browser')

    # On NT we don't try to find executables, so that we will default
    # to using gv_launch_url().
    if os.name == "nt":
        return ''

    exe_names = ['netscape', 'mozilla', 'mosaic', 'gnome-help-browser' ]
    for name in exe_names:
        full_path = gvutils.FindExecutable(name)
        if (full_path is not None) and (full_path != ''):
            return full_path

    return ''

def SetBrowseCommand(command):
    global html_browse_command
    html_browse_command = command
    gview.set_preference('html_browser', command)


def LaunchHTML( page_name ):
    """Display indicated HTML page.

    If the passed name is not an absolute path name, nor has an http: prefix,
    it will be massaged to point into the GView html help tree."""

    if not os.path.isabs(page_name) and page_name[:5] != 'http:':
        page_name = os.path.abspath( \
            os.path.join(gview.home_dir,'html',page_name) )

    global html_browse_command

    if page_name[:5] != 'http:':
        page_name = 'file://'+page_name

    if html_browse_command == '':
        html_browse_command = GetBrowseCommand()

    if os.name == "nt" and html_browse_command == '':
        try:
            import webbrowser
            webbrowser.open(page_name)
        except:    
            _gv.gv_launch_url( page_name )

        return

    if html_browse_command == '' or html_browse_command is None:
        try:
            import webbrowser
            webbrowser.open(page_name)
        except:
            gvutils.warning( 'Unable to display HTML online help, browser not configured.' )
        return

    if "%s" in html_browse_command:
        full_command = html_browse_command.replace("%s",page_name) + " &"
    else:
        full_command = html_browse_command + ' ' + page_name + ' &'

    os.system( full_command )

def f1_help_cb( item, event, topic, *args ):
    if (event.keyval == gtk.keysyms.F1) and not (event.state & gtk.gdk.CONTROL_MASK):
        LaunchHTML( topic )

def set_help_topic( object, topic ):
    """Set a help topic for a widget.

    The help topic (an html file) is launched if the user hits F1 over the
    widget.

    topic -- topic name, such as 'edittools.html', suitable for use
    with the LaunchHTML() function."""

    object.connect( "key_press_event", f1_help_cb, topic )

html_browse_command = ''



