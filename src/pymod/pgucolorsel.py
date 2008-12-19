###############################################################################
# $Id$
#
# Project:  Python Gtk Utility Widgets
# Purpose:  DEPRECATED: now only subclassing pgucolor.ColorControl
# Author:   Mario Beauchamp (starged@gmail.com)
#
###############################################################################
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

from pgucolor import ColorControl as cc

class ColorControl(cc):
    def __init__(self, title, callback=None, cb_data=None):
        cc.__init__(self, title, callback, cb_data)

    def set_color_from_string(self, new_color):
        if new_color is None:
            return
        
        red, green, blue, alpha = new_color.split()
        self.set_color((float(red), float(green), float(blue), float(alpha)))
