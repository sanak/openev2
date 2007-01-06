###############################################################################
# $Id: pgucolor.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
#
# Project:  Python Gtk Utility Widgets
# Purpose:  Color-related widgets and utilities.
# Author:   Paul Spencer, pgs@magma.ca
#
###############################################################################
# Copyright (c) 2000, DM Solutions Group Inc. (www.dmsolutions.on.ca)
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

MIN_COLOR=0
MAX_COLOR = 65535


import gtk
from gtk.gdk import *
from gtk.keysyms import *
from gvsignaler import Signaler
import _gv
from gtkmissing import GtkColorWell

def color_string_to_tuple(s):
    from string import replace, split
    s = replace(s, '(', '')
    s = replace(s, ')', '')
    s = replace(s, ',', '')
    r, g, b, a = split(s, None)
    return (float(r), float(g), float(b), float(a))


class ColorSwatch(gtk.DrawingArea, Signaler):
    """
    Class ColorSwatch is a simple widget that
    displays a color.

    The color attribute is an RGBA tuple.  Internally, however,
    a GdkColor object is used that has values in the range 0-MAX_COLOR
    for red, green and blue.  The color display doesn't support
    the alpha channel (yet?)

    Don't use GdkColor(red, green, blue) to allocate colors ... use
    GdkColormap.alloc instead and get a reference to the GdkColormap
    from the widget (self.get_colormap())
    """
    def __init__(self, color=(0,0,0,0)):
        gtk.DrawingArea.__init__(self)
        self.set_size_request(20, 15)
        self.connect('configure-event', self.configure_event)
        self.connect('expose-event', self.expose_event)
        self.connect('realize', self.realize_event)
        self.connect('unrealize', self.unrealize_event)
        self.color = color
        #the color - use GdkColorMap's alloc method to get it
        cm = self.get_colormap()
        self.icolor = cm.alloc_color(int(color[0] * MAX_COLOR), \
                               int(color[1] * MAX_COLOR), \
                               int(color[2] * MAX_COLOR))
        self.publish('color-changed')
        #cached graphics context
        self.gc = None

    def realize_event(self, *args):
        self.gc = self.window.new_gc(foreground=self.icolor)

    def unrealize_event(self, *args):
        #remove references to the gc to prevent leaks?
        self.gc = None

    def configure_event(self, *args):
        #is this required?
        return False

    def expose_event(self, *args):
        #get the window and graphic context
        win = self.window
        w,h = win.get_size()
        win.draw_rectangle(self.get_style().black_gc, False, 0, 0, w-1, h-1)
        win.draw_rectangle(self.gc, True, 1, 1, w-2, h-2)
        return False

    def set_color(self, color=(0,0,0,0)):
        self.color = color
        #the color - use GdkColorMap's alloc method to get it
        cm = self.get_colormap()
        self.icolor = cm.alloc(int(color[0] * MAX_COLOR), int(color[1] * MAX_COLOR),
                               int(color[2] * MAX_COLOR))
        if self.gc is not None:
            self.gc.foreground = self.icolor
        self.queue_draw()
        Signaler.notify(self, 'color-changed')

    def get_color(self):
        return self.color


class ColorButton(gtk.ColorButton):
    """
    Class ColorButton extends gtk.ColorButton
    """

    def __init__(self, color=(0,0,0,0), title='', use_alpha=True, continuous=False, _obj=None, colormap=None):
        if colormap is None:
            raise ValueError, "Must provide a colormap"
        gtk.ColorButton.__init__(self)

        self.colormap = colormap
        self.set_color(color)
        self.set_title(title)
        self.set_use_alpha( use_alpha )
    def get_color(self):
        gtkcolor = gtk.ColorButton.get_color(self)
        trans = lambda x: float(x)/MAX_COLOR
        a = self.get_alpha()
        r,g,b = gtkcolor.red, gtkcolor.green, gtkcolor.blue
        pre = (r,g,b)
        color = tuple([trans(x) for x in pre])
        return color + (a,)
    def set_color(self, color):
        assert isinstance(color,tuple)
        gtkcolor = self.colormap.alloc_color(int(color[0]*MAX_COLOR), 
                int(color[1]*MAX_COLOR), int(color[2]*MAX_COLOR))
        gtk.ColorButton.set_color(self, gtkcolor)
        self.set_alpha(color[-1])
    def get_alpha(self):
        return float(gtk.ColorButton.get_alpha(self))/MAX_COLOR
    def set_alpha(self, alpha):
        gtk.ColorButton.set_alpha(self, int(alpha*MAX_COLOR))
    set_d = set_color
    get_d = get_color


class ColorDialog(gtk.Window):
    """used with a ColorButton when it is clicked"""

    def __init__(self, ok_cb = None, cancel_cb = None, cb_data = None):
        gtk.Window.__init__(self)
        self.set_title('Select a Color')
        vbox = gtk.VBox(spacing=3)
        self.add(vbox)
        self.user_ok_cb = ok_cb
        self.user_cancel_cb = cancel_cb
        self.user_cb_data = cb_data

        self.connect('delete-event', self.user_cancel_cb)
        #add the color selection widget
        self.colorsel = gtk.ColorSelection()
        self.colorsel.set_opacity(True)
        vbox.pack_start(self.colorsel)
        #add the ok and cancel buttons
        button_box = gtk.HButtonBox()
        ok_button = gtk.Button("OK")
        ok_button.connect('clicked', self.ok_cb, cb_data)
        cancel_button = gtk.Button("Cancel")
        cancel_button.connect('clicked', self.cancel_cb, cb_data)
        button_box.pack_start(ok_button)
        button_box.pack_start(cancel_button)
        vbox.pack_start(button_box, expand=False)
        vbox.show_all()
        ok_button.set_flags(gtk.CAN_DEFAULT)
        ok_button.grab_default()

    def ok_cb(self, *args):
        if self.user_ok_cb is not None:
            self.user_ok_cb(self.user_cb_data, self)
        self.hide()
        self.destroy()

    def cancel_cb(self, *args):
        if self.user_cancel_cb is not None:
            self.user_cancel_cb(self.user_cb_data, self)
        self.hide()
        self.destroy()

RAMP_GRADIENT = 0
RAMP_DISCRETE = 1

class ColorRamp(gtk.Frame, Signaler):
    """encapsulate the functionality of a color ramp that
    can apply itself in a linearly interpolated number of
    steps between several colors positioned along the ramp

    Colors can be returned from the ramp by calling apply_ramp
    with a callback and the number of colors to calculate.
    """
    def __init__(self):
        """initialize the ramp
        """
        gtk.Frame.__init__(self)
        self.set_shadow_type(gtk.SHADOW_NONE)
        self.colors = []
        self.gradient = ColorGradientSwatch(self)
        self.title = gtk.Label('Ramp')
        fix = gtk.Fixed()
        fix.put(self.gradient, 1, 0)
        fix.put(self.title, 84, 0)
        self.add(fix)
        self.type = RAMP_GRADIENT

    def serialize(self, fname = None):
        """save to a file
        """
        
        result = "%s\n" % self.title.get()
        result = result + "%s\n" % self.type
        for n in self.colors:
            if self.type == RAMP_GRADIENT:
                result = result + "%s %s\n" % (str(n[0]), str(n[1]))
            else:
                result = result + "%s\n" % str(n[1])
                
        if fname is not None:
            f = open(fname, 'w')
            f.write( result )
            f.close()
        else:
            return result      

    def deserialize(self, fname):
        """read from a file"""
        import string
        fp = open(fname, 'r')
        lines = fp.readlines()
        self.title.set_text(lines[0].strip())
        self.type = int(lines[1].strip())
        n_colors = len( lines[2:] )
        for i in range(n_colors):
            line = lines[2+i]
            line = string.replace(line, '(', '')
            line = string.replace(line, ')', '')
            line = string.replace(line, ',', '')
            if self.type == RAMP_GRADIENT:
                pos, r, g, b, a = line.split()
            else:
                r, g, b, a = line.split()
                #assume equally spaced for DISCRETE
                pos = float(i)/float(n_colors - 1)
            self.add_color((float(r), float(g), float(b), float(a)), float(pos))

        fp.close()
        self.queue_draw()

    def add_color(self, color, position):
        """add a color to the ramp at the given position.
        color - an rgba tuple
        position - between 0.0 and 1.0
        """
        for i in range(len(self.colors)):
            if position <= self.colors[i][0]:
                self.colors.insert(i, (position, color))
                break
        else:
            self.colors.append((position, color))

    def apply_ramp(self, color_cb, ncolors):
        """
        return ncolors spread over the ramp by
        calling the color_cb callback with the
        current position (in the range 0 to
        ncolors-1) and color
        """
        if len(self.colors) == 0:
            return

        #insert false entries at 0 and 1 if necessary
        bLow = False
        bHi = False

        if self.colors[0][0] <> 0.0:
            self.add_color(self.colors[0][1], 0.0)
            bLow = True
        if self.colors[len(self.colors) - 1][0] <> 1.0:
            self.add_color(self.colors[len(self.colors)-1][1], 1.0)
            bHi = True
        
        if ncolors > 1:
            for i in range(ncolors):
                if self.type == RAMP_GRADIENT:
                    color_cb(i, self.calculate_color(float(float(i)/float(ncolors - 1))))
                elif self.type == RAMP_DISCRETE:
                    pos = i % len(self.colors)
                    color_cb(i, self.colors[pos][1] )
                    
        else:
            color_cb( 0, self.calculate_color( 0 ) )
            
        #clean up
        if bLow:
            del self.colors[0]
        if bHi:
            del self.colors[len(self.colors)-1]

    def calculate_color(self, pos):
        """calculate the color at the given position.  If a color
        exists at the position, return it. Otherwise get the color
        before and after it and calculate a linear interpolation
        between them.
        """
        for i in range(len(self.colors)-1):
            below = self.colors[i]
            above = self.colors[i+1]
            if below[0] <= pos and above[0] >= pos:
                fr = below[1][0]
                fg = below[1][1]
                fb = below[1][2]
                fa = below[1][3]
                tr = above[1][0]
                tg = above[1][1]
                tb = above[1][2]
                ta = above[1][3]
                delta = (pos - below[0]) / (above[0] - below[0])
                cr = fr + ( tr - fr ) * delta
                cg = fg + ( tg - fg ) * delta
                cb = fb + ( tb - fb ) * delta
                ca = fa + ( ta - fa ) * delta
                return (cr, cg, cb, ca)

    def get_color_list(self, ncolors):
        self.color_list = []
        self.apply_ramp(self.color_list_cb, ncolors)
        return self.color_list

    def color_list_cb(self, num, color):
        self.color_list.insert(num, color)

class ColorGradientSwatch(ColorSwatch):
    """
    Class ColorGradientSwatch extends ColorSwatch to n colors
    and draws itself as a gradient between the various colors.

    This class is intended primarily to provide a GUI element for
    ColorRamps
    """
    def __init__(self, ramp):
        ColorSwatch.__init__(self)
        self.set_size_request(80, 15)
        self.ramp = ramp

    def expose_event(self, *args):
        #get the window and graphic context
        win = self.window
        self.width, self.height = win.get_size()
        self.cm = self.get_colormap()
        if self.ramp.type == RAMP_GRADIENT:
            colors = self.ramp.get_color_list(self.width)
        else:
            colors = self.ramp.get_color_list( len(self.ramp.colors) - 1 )
        bar_width = self.width / len(colors)
        i = 0
        for color in colors:
            self.gc.foreground = self.cm.alloc_color(int(color[0] * MAX_COLOR),
                                               int(color[1] * MAX_COLOR),
                                               int(color[2] * MAX_COLOR))
            win.draw_rectangle(self.gc, True, i, 0, bar_width, self.height)
            i = i + bar_width
        win.draw_rectangle(self.get_style().black_gc, False, 0, 0, self.width-1, self.height-1)
        return False


def test_cb(num, color):
    print num, ' - ', color

def color_set( widget ):
    print 'color set to ', widget.get_color()

if __name__ == '__main__':
    a = ColorRamp()
    a.add_color((0.0,1.0,0.0,1.0), 0.0)
    a.add_color((1.0,1.0,0.0,1.0), 0.50)
    a.add_color((1.0,0.0,0.0,1.0), 1.0)
    print a.colors
    a.apply_ramp(test_cb, 5)
    print a.colors
    a.serialize('c:\\test_ramp')
    b = ColorRamp()
    b.deserialize('c:\\test_ramp')
    print b.colors

    c = ColorButton((1.0, 0.0, 0.0, 1.0))
    c.connect('color-set', color_set)

    dlg = gtk.Dialog()
    btn = gtk.Button('OK')
    btn.connect('clicked', gtk.main_quit)

    dlg.vbox.pack_start(a)
    dlg.vbox.pack_start(b)
    dlg.vbox.pack_start(c)
    dlg.action_area.pack_start(btn)
    dlg.show_all()
    dlg.show()

    gtk.main()


    
