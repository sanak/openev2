import gtk

class FontTest(gtk.Window):
    """ A quick test of the font selection button and dialog.
        I was having problems with one out of ten fonts in
        the dialog, where the entire app crashes most ungracefully
        with a pango error:
        (fonttest.py): GLib-GObject-CRITICAL **: file gobject.c: line 1561 (g_object_ref): assertion `G_IS_OBJECT (object)' failed
        ** (fonttest.py): CRITICAL **: file pango-engine.c: line 68 (_pango_engine_shape_shape): assertion `PANGO_IS_FONT (font)' failed
    """
    def __init__(self):
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        
        vbox = gtk.VBox()
        btn = gtk.FontButton()
        vbox.add(btn)
        self.set_border_width(30)
        self.set_position(gtk.WIN_POS_CENTER)
        self.add(vbox)
        self.show_all()
          
        # When the window is given the "delete_event" signal (this is given
        # by the window manager, usually by the "close" option, or on the
        # titlebar), we ask it to call the delete_event () function
        # as defined above. The data passed to the callback
        # function is NULL and is ignored in the callback function.
        self.connect("delete_event", self.delete_event)
    
        # Here we connect the "destroy" event to a signal handler.  
        # This event occurs when we call gtk_widget_destroy() on the window,
        # or if we return FALSE in the "delete_event" callback.
        self.connect("destroy", self.destroy)
      
    # Another callback
    def destroy(self, widget, data=None):
        gtk.Window.destroy(self)
        gtk.main_quit()
      
    def delete_event(self, widget, event, data=None):
        # If you return FALSE in the "delete_event" signal handler,
        # GTK will emit the "destroy" signal. Returning TRUE means
        # you don't want the window to be destroyed.
        # This is useful for popping up 'are you sure you want to quit?'
        # type dialogs.
        #
        # Change FALSE to TRUE and the main window will not be destroyed
        # with a "delete_event".
        return gtk.FALSE
      
def main():
    pt = FontTest()
    gtk.main()

if __name__ == '__main__':
    pt = FontTest()
    gtk.main()
    
  