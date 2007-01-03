###############################################################################
# $Id: pgucolorsel.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
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
from gtk import TRUE, FALSE
from string import *
import pgu
from pgucolourswatch import ColourSwatch


class ColorControl(gtk.HBox):

    """Embeddable Color Selector Control

    The pgucolorsel.ColorControl widget is intended to provide a simple
    color selection widget that can be embedded in dialogs and that will
    launch a fill gtk.ColorSelectionDialog if the user doesn't select one of
    the list of predefined colors.

    At this time the ColorControl's main widgetry is just an option menu, but
    it is expected in the future to also include some sort of color swatch
    showing the currently selected color graphically.

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

    color_list = [ ('Red',         (  1,   0,   0,   1)),
                   ('Green',       (  0,   1,   0,   1)),
                   ('Blue',        (  0,   0,   1,   1)),
                   ('White',       (  1,   1,   1,   1)),
                   ('Black',       (  0,   0,   0,   1)),
                   ('Transparent', (  0,   0,   0,   0))]

    def __init__(self, title, callback = None, cb_data = None):
        gtk.HBox.__init__(self)

        self.callback = callback
        self.cb_data = cb_data
        self.current_color = (1,0,0,1)
        self.mixer = None
        self.title = title

        self.swatch = ColourSwatch(self.current_color)

        self.om = gtk.OptionMenu()

        menu = gtk.Menu()
        self.om.set_menu(menu)

        item = None
        for cinfo in self.color_list:
            name, value = cinfo
            item = gtk.RadioMenuItem(item,name)
            item.connect('activate', self.set_color_cb, value)
            item.show()
            menu.append(item)
        
        item = gtk.RadioMenuItem(item,'Custom')
        item.connect('activate', self.launch_mixer_cb)
        item.show()
        menu.append(item)
        
        item = gtk.RadioMenuItem(item,'Mixer')
        item.connect('activate', self.launch_mixer_cb)
        item.show()
        menu.append(item)

        self.om.set_history(0)
        
        self.pack_start(self.swatch, expand=FALSE)
        self.pack_start(self.om)

    def launch_mixer_cb(self,item):
        if not item.get_active():
            return

        cm = self.get_colormap()
        icolor = cm.alloc_color(int(self.current_color[0] * 65535),
                                int(self.current_color[1] * 65535),
                                int(self.current_color[2] * 65535))

        if self.mixer is not None and self.mixer.window is not None:
            self.mixer.colorsel.set_current_color(icolor)
            self.mixer.colorsel.set_current_alpha(self.current_color[3] * 65535)
            self.mixer.window.raise_()
            return

        self.mixer = gtk.ColorSelectionDialog(self.title)
        self.mixer.connect('delete-event',self.mixer_delete)
        self.mixer.colorsel.set_has_opacity_control(True)
        self.mixer.colorsel.set_current_alpha(self.current_color[3] * 65535)
        self.mixer.colorsel.set_current_color(icolor)
        self.mixer.ok_button.connect('clicked',self.mixer_ok_cb )
        self.mixer.ok_button.set_label('Apply')
        self.mixer.cancel_button.connect('clicked',self.mixer_cancel )
        self.mixer.cancel_button.set_label('Close')
        self.mixer.help_button.hide()
        self.mixer.show()
        self.update_om()

    def mixer_delete(self, widget, args):
        self.mixer = None
        
    def mixer_cancel(self, args):
        self.mixer.hide()
        self.mixer.destroy()
        self.mixer = None
        
    def mixer_ok_cb(self, args):
        rgb = self.mixer.colorsel.get_current_color()
        alpha = self.mixer.colorsel.get_current_alpha()
        self.set_color((rgb.red / 65535.0, rgb.green / 65535.0,
                        rgb.blue / 65535.0, alpha / 65535.0))
        
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
        self.swatch.set_colour(self.current_color)
        self.invoke_callback()
        self.update_om()

    def update_om(self):
        for ci_index in range(len(self.color_list)):
            name, value = self.color_list[ci_index]
            if value == self.current_color:
                self.om.set_history(ci_index)
                return

        # Mark as custom
        self.om.set_history(len(self.color_list))

    def set_color_from_string(self, new_color):
        if new_color is None:
            return
        
        red, green, blue, alpha = split(new_color,' ')
        self.set_color((atof(red), atof(green), atof(blue), atof(alpha)))
        
    def invoke_callback(self):
        if self.callback == None:
            return
        self.callback( self.current_color, self.cb_data )

    def set_color_cb(self, item, color):
        if item.get_active():
            self.set_color( color )

pgu.gtk_register('ColorControl',ColorControl)

