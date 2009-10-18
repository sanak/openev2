###############################################################################
# $Id$
#
# Project:  Python Gtk Utility Widgets
# Purpose:  Color-related widgets and utilities.
# Author:   Paul Spencer, pgs@magma.ca
#
# Maintained by Mario Beauchamp (starged@gmail.com) for CIETcanada
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

MIN_COLOR = 0
MAX_COLOR = 65535


import gtk
from gvsignaler import Signaler

# temporary
def _(s):
    return s

def color_string_to_tuple(s):
    """Convert a string to a color tuple"""
    if isinstance(s, tuple): # CIETmap requirement
        return s
    if s.startswith('('): # assume this is '(r,g,b,a)' (or '(r g b a)')
        s = s[1:-1]
    sep = None
    if ',' in s: # in case this is a space separated variant
        sep = ','
    rgba = s.split(sep)
    return tuple([float(c) for c in rgba])

def color_tuple_to_gdk(color):
    """Convert a gv color tuple to a gtk.gdk.Color"""
    r,g,b,a = color
    return gtk.gdk.Color(int(r*MAX_COLOR), int(g*MAX_COLOR), int(b*MAX_COLOR))

def gdk_to_gv_color(gdk_color, alpha):
    """Convert a gtk.gdk.Color and alpha to a gv color tuple"""
    maxc = float(MAX_COLOR)
    return (gdk_color.red/maxc, gdk_color.green/maxc, gdk_color.blue/maxc, alpha/maxc)

class ColorSwatch(gtk.DrawingArea, Signaler):
    """
    Class ColorSwatch is a simple widget that
    displays a color.

    The color attribute is an RGBA tuple.  Internally, however,
    a GdkColor object is used that has values in the range 0-MAX_COLOR
    for red, green and blue.  The color display doesn't support
    the alpha channel (yet?)

    Don't use gtk.gdk.Color(red, green, blue) to allocate colors ... use
    gtk.gdk.Colormap.alloc_color instead and get a reference to the Colormap
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
        self.icolor = cm.alloc_color(color_tuple_to_gdk(color))
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
        w,h = self.size_request()
        win.draw_rectangle(self.get_style().black_gc, False, 0, 0, w-1, h-1)
        win.draw_rectangle(self.gc, True, 1, 1, w-2, h-2)
        return False

    def set_color(self, color=(0,0,0,0)):
        self.color = color
        #the color - use GdkColorMap's alloc method to get it
        cm = self.get_colormap()
        self.icolor = cm.alloc_color(color_tuple_to_gdk(color))
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

    def __init__(self, color=(0,0,0,0), title="Select a Color", use_alpha=True,
                    continuous=False, _obj=None, colormap=None):
        if colormap is None:
            pass # colormap no longer required
        gtk.ColorButton.__init__(self)

        self.set_color(color)
        self.set_title(title)
        self.set_use_alpha(use_alpha)

    def get_color(self):
        """overriden to return color tuple"""
        gdkcolor = gtk.ColorButton.get_color(self)
        alpha = self.get_alpha()
        return gdk_to_gv_color(gdkcolor, alpha)

    def set_color(self, color):
        """overriden to set a color tuple"""
        if isinstance(color, str): # CIETmap requirement
            color = color_string_to_tuple(color)

        gtk.ColorButton.set_color(self, color_tuple_to_gdk(color))
        self.set_alpha(int(color[3]*MAX_COLOR))

    set_d = set_color
    get_d = get_color

RAMP_GRADIENT = 0
RAMP_DISCRETE = 1

class ColorRamp(object, Signaler):
    """encapsulate the functionality of a color ramp that
    can apply itself in a linearly interpolated number of
    steps between several colors positioned along the ramp

    Colors can be returned from the ramp by calling apply_ramp
    with a callback and the number of colors to calculate.
    """
    def __init__(self):
        """initialize the ramp
        """
        self.colors = []
        self.gradient = ColorGradientSwatch(self)
        self.title = "Ramp"
        self.type = RAMP_GRADIENT

    def serialize(self, fname=None):
        """save to a file
        """
        title = self.title
        result = '\n'.join( [title, self.type] )
        for n in self.colors:
            if self.type == RAMP_GRADIENT:
                result += '%s %s\n' % (n[0], n[1])
            else:
                result += '%s\n' % n[1]

        if fname:
            f = open(fname, 'w')
            f.write(result)
            f.close()
        else:
            return result      

    def deserialize(self, fname):
        """read from a file"""
        fp = open(fname, 'r')
        lines = fp.readlines()
        self.title = lines.pop(0).strip()
        self.type = int(lines.pop(0))
        n_colors = len(lines)
        for n,line in enumerate(lines):
            line = line.replace('(', '')
            line = line.replace(')', '')
            line = line.replace(',', '')
            if self.type == RAMP_GRADIENT:
                pos, r, g, b, a = line.split()
            else:
                r, g, b, a = line.split()
                #assume equally spaced for DISCRETE
                pos = float(n)/float(n_colors - 1)
            self.add_color((float(r), float(g), float(b), float(a)), float(pos))

        fp.close()

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
        if not self.colors:
            return

        #insert false entries at 0 and 1 if necessary
        bLow = False
        bHi = False

        if self.colors[0][0] != 0.0:
            self.add_color(self.colors[0][1], 0.0)
            bLow = True
        if self.colors[-1][0] != 1.0:
            self.add_color(self.colors[-1][1], 1.0)
            bHi = True
        
        if ncolors > 1:
            for i in range(ncolors):
                if self.type == RAMP_GRADIENT:
                    color_cb(i, self.calculate_color(float(float(i)/float(ncolors - 1))))
                elif self.type == RAMP_DISCRETE:
                    pos = i % len(self.colors)
                    color_cb(i, self.colors[pos][1])
                    
        else:
            color_cb(0, self.calculate_color(0))
            
        #clean up
        if bLow:
            del self.colors[0]
        if bHi:
            del self.colors[-1]

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
                cr = fr + (tr - fr) * delta
                cg = fg + (tg - fg) * delta
                cb = fb + (tb - fb) * delta
                ca = fa + (ta - fa) * delta
                return (cr, cg, cb, ca)

    def get_color_list(self, ncolors):
        self.color_list = []
        self.apply_ramp(self.color_list_cb, ncolors)
        return self.color_list

    def color_list_cb(self, num, color):
        self.color_list.insert(num, color)

    def save_to_png(self):
        pixbuf = self.gradient.get_pixbuf()
        pixbuf.save(self.title.get_text()+'.png','png')

class ColorGradientSwatch(gtk.gdk.Pixmap):
    """
    Class ColorGradientSwatch uses a Pixmap and draws itself
    as a gradient between the various colors.

    This class is intended primarily to provide a GUI element for
    ColorRamps
    """
    def __init__(self, ramp):
        gtk.gdk.Pixmap.__init__(self, None, 80, 15, 24)
        self.ramp = ramp
        self.gc = self.new_gc()
        self.cm = gtk.gdk.colormap_get_system()

    def draw(self):
        width, height = self.get_size()
        if self.ramp.type == RAMP_GRADIENT:
            colors = self.ramp.get_color_list(width)
        else:
            colors = self.ramp.get_color_list(len(self.ramp.colors) - 1)

        bar_width = width / len(colors)
        i = 0
        for color in colors:
            self.gc.foreground = self.cm.alloc_color(color_tuple_to_gdk(color))
            self.draw_rectangle(self.gc, True, i, 0, bar_width, height)
            i += bar_width
        self.gc.foreground = self.cm.alloc_color(gtk.gdk.Color(0,0,0,0))
        self.draw_rectangle(self.gc, False, 0, 0, width-1, height-1)

    def get_pixbuf(self):
        self.draw()
        pixbuf = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB, False, 8, 80, 15)
        return pixbuf.get_from_drawable(self, self.cm, 0,0,0,0,-1,-1)

# previously in pgucolorsel.py
class ColorControl(gtk.HBox):
    def __init__(self, title, callback=None, cb_data=None):
        from pgu import ComboText
        gtk.HBox.__init__(self)
        self.color_list = { _("Red"): (1, 0, 0, 1),
                       _("Green"): (0, 1, 0, 1),
                       _("Blue"): (0, 0, 1, 1),
                       _("White"): (1, 1, 1, 1),
                       _("Black"): (0, 0, 0, 1),
                       _("Transparent"): (0, 0, 0, 0),
                       _("Custom"): (0, 0, 0, 1)
                        }

        self.callback = callback
        self.cb_data = cb_data
        self.current_color = (1,0,0,1)

        but = ColorButton(color=self.current_color, title=title)
        but.connect('color-set', self.set_color_cb)
        self.pack_start(but)
        self.cbut = but

        combo = ComboText(strings=self.color_list.keys(), action=self.set_color_cb)
        self.pack_end(combo)
        self.colorsCB = combo
        self.show_all()
        self.updating = False

    def set_color_cb(self, widget):
        if self.updating:
            return

        if widget == self.cbut:
            color = self.cbut.get_color()
            self.updating = True
            self.colorsCB.set_active_text("Custom")
            self.updating = False
            self.color_list['Custom'] = self.current_color = color
            self.invoke_callback()
        else:
            color_name = widget.get_active_text()
            color = self.color_list[color_name]
            self.current_color = color
            self.updating = True
            self.cbut.set_color(color)
            self.updating = False
            self.invoke_callback()

    def set_color(self, new_color):
        if isinstance(new_color, str):
            new_color = color_string_to_tuple(new_color)
        if new_color == self.current_color:
            return

        self.current_color = new_color
        self.updating = True
        self.colorsCB.set_active_text("Custom")
        self.updating = False
        self.cbut.set_color(new_color)

    def invoke_callback(self):
        if self.callback is None:
            return
        self.callback(self.current_color, self.cb_data)

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



