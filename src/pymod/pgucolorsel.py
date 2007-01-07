###############################################################################
# $Id$
#
# Project:  Python Gtk Utility Widgets
# Purpose:  Embeddable color selector.
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

import gtk
import pgu
import pgucolor

class ColorControl(gtk.HBox):
    """Embeddable Color Selector Control

    The pgucolorsel.ColorControl widget is intended to provide a simple
    color selection widget that can be embedded in dialogs and that will
    launch a fill gtk.ColorSelectionDialog if the user doesn't select one of
    the list of predefined colors.

    The application supplies a single callback which is involved when the
    selected color changes.  All colors are handled as RGBA tuples, with the
    values being in the range of 0-1 (rather than 0-255). 

    Arguments:

    title -- the title used for the color selection dialog, if displayed.

    callback -- the application function to call when the color changes.  It
    should take a color tuple (RGBA) and a callback data object as arguments. 

    cb_data -- callback data to pass to callback. 

    Example:

        self.mod_color = pgucolorsel.ColorControl('Modulation Color',
                                                  self.color_cb,None)
        self.mod_color.set_color(tcolor)

    def color_cb( self, color, cb_data ):
        print color[0], color[1], color[2], color[3]
    """

    color_list = { "Red": (1, 0, 0, 1),
                   "Green": (0, 1, 0, 1),
                   "Blue": (0, 0, 1, 1),
                   "White": (1, 1, 1, 1),
                   "Black": (0, 0, 0, 1),
                   "Transparent": (0, 0, 0, 0),
                   "Custom": (0, 0, 0, 0)
                    }

    def __init__(self, title, callback=None, cb_data=None):
        gtk.HBox.__init__(self, spacing=2)

        self.callback = callback
        self.cb_data = cb_data
        self.current_color = (1,0,0,1)

        but = pgucolor.ColorButton(color=self.current_color, title=title)
        but.connect('color-set', self.set_color_cb)
        self.pack_start(but, expand=False)
        self.cbut = but

        combo = pgu.ComboText(strings=self.color_list.keys(), action=self.set_color_cb)
        self.pack_end(combo)
        self.colorsCB = combo
        self.show_all()
        self.updating = False

    def set_color(self, new_color):
        if len(new_color) == 3:
            new_color = (new_color[0],new_color[1],new_color[2],1.0)

        if new_color[0] > 1 or new_color[1] > 1 \
           or new_color[2] > 1 or new_color[3] > 1:
            new_color = (new_color[0] / 255.0,
                         new_color[1] / 255.0,
                         new_color[2] / 255.0,
                         new_color[3] / 255.0)

        if new_color == self.current_color:
            return

        self.current_color = new_color
        self.colorsCB.set_active_text("Custom")
        self.cbut.set_color(new_color)

    def set_color_from_string(self, new_color):
        if new_color is None:
            return

        red, green, blue, alpha = new_color.split(' ')
        self.set_color((float(red), float(green), float(blue), float(alpha)))

    def invoke_callback(self):
        if self.callback == None:
            return
        self.callback(self.current_color, self.cb_data)

    def set_color_cb(self, widget):
        if self.updating:
            return

        if widget == self.cbut:
            color = self.cbut.get_color()
            self.updating = True
            self.colorsCB.set_active_text("Custom")
            self.updating = False
            self.color_list["Custom"] = self.current_color = color
            self.invoke_callback()
        else:
            color_name = widget.get_active_text()
            color = self.color_list[color_name]
            self.current_color = color
            self.updating = True
            self.cbut.set_color(color)
            self.updating = False
            self.invoke_callback()

pgu.gtk_register('ColorControl',ColorControl)

