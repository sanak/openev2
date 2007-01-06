###############################################################################
# $Id: pgucolourswatch.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
#
# Project:  Python Gtk Utility Widgets
# Purpose:  Embeddable color swatch.
# Author:   Paul Spencer, pgs@magma.ca
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
from gvsignaler import Signaler

class ColourSwatch(gtk.DrawingArea, Signaler):
    '''
    Class ColourSwatch is a simple widget that
    displays a colour.
    
    The colour attribute is an RGBA tuple.  Internally, however,
    a GdkColour object is used that has values in the range 0-65535
    for red, green and blue.      
    '''
    def __init__(self, colour=(0,0,0,0)):
        gtk.DrawingArea.__init__(self)
        self.size(30, 15)
        self.connect('configure-event', self.configure_event)
        self.connect('expose-event', self.expose_event)
        self.connect('realize', self.realize_event)
        self.connect('unrealize', self.unrealize_event)
        self.colour = colour
        #the color - use GdkColourMap's alloc method to get it
        cm = self.get_colormap()
        self.icolour = cm.alloc(int(colour[0] * 65535),
                                int(colour[1] * 65535),
                                int(colour[2] * 65535))
        self.publish('colour-changed')
        #cached graphics context
        self.gc = None
        
    def realize_event(self, *args):
        self.gc = self.window.new_gc(foreground=self.icolour)

    def unrealize_event(self, *args):
        #don't know the correct way to destroy a gc yet
        #self.gc.destroy()
        pass
                
    def configure_event(self, *args):
        #is this required?
        return False

    def expose_event(self, *args):
        #get the window and graphic context 
        win = self.window
        width, height = win.get_size()
        win.draw_rectangle(self.get_style().black_gc, False,
            2, 2, width-2, height-2)
        win.draw_rectangle(self.gc, True,
            3, 3, width-3, height-3)
        return False
        
    def set_colour(self, colour=(0,0,0,0)):
        self.colour = colour
        #the color - use GdkColourMap's alloc method to get it
        cm = self.get_colormap()
        self.icolour = cm.alloc_color(int(colour[0] * 65535),
                                      int(colour[1] * 65535),
                                      int(colour[2] * 65535))
        if self.gc is not None:
                self.gc.foreground = self.icolour
        Signaler.notify(self, 'colour-changed')
        if self.flags() & gtk.REALIZED:
            self.expose_event()

