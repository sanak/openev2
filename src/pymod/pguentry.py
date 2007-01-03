import gtk
from gtk.gdk import *

class pguEntry( gtk.Entry ):

    def __init__(self):
        gtk.Entry.__init__(self)
        self.add_events(gtk.gdk.FOCUS_CHANGE_MASK)
        self.connect('focus-out-event', self.cleanup)

    def cleanup(self, *args):
        self.select_region(0, 0)
        self.queue_draw()
