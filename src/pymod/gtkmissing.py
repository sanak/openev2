import gtk; _gtk = gtk; del gtk
import _gv
import pgu
from gvobject import GvObject

# TEMP - GTK2 PORT
#import _gtkmissing

###############################################################################
class GtkColorWell(_gv.ColorWell, GvObject):
    def __init__(self, title="", _obj=None):
        if (_obj == None):
            _gv.ColorWell.__init__(self, title)
            _obj = self
        GvObject.__init__(self, _obj)

    def get_color( self ):
        return self.get_d()

    def set_color( self, color ):
        self.set_d( color[0], color[1], color[2], color[3] )

###############################################################################
#
#def toolbar_append_element(self, type, widget, text, tooltip, tp,
#                           icon, callback, *extra):
#    if widget: widget = widget._o
#    if icon: icon = icon._o
#    return _gtk._obj2inst(_gtkmissing.gtk_toolbar_append_element(
#        self._o, type, widget, text, tooltip, tp, icon, callback, extra))
#
#_gtk.Toolbar.append_element = toolbar_append_element
#
###############################################################################

def gtk_window_get_position( self ):
    return _gtkmissing.gtk_window_get_position( self._o )

def gtk_window_move( self, x, y ):
    return _gtkmissing.gtk_window_move( self._o, x, y )

#_gtk.Window.get_position = gtk_window_get_position
#_gtk.Window.window_move = gtk_window_move
#
#del toolbar_append_element

