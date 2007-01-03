###############################################################################
# $Id: pgutextarea.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Scrollable text area widget
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

"""
This widget is very simple, it just allows you to append text to it.  It allows
you to scroll around in the text in a very limited fashion.  PyGTK 0.6.6 
doesn't define the events for GDK_SCROLL so it's commented out here.


Also, the width calculations seem to be off so the horizontal page size
calculations don't give a perfect page width.  However, the width always seems 
to be wider than necessary so you can at least see everything.

To use it, simply put it in a container ( the scrollbars are embedded )

Call append_text to append a string to it.  Carriage returns are acceptable

text_area.contents is a list containing one entry per line.

use scroll_to to scroll the window to a particular line - it will attempt to
put the line at the top, taking into account the vertical page size and the
overall size of the buffer.  This means that you will at least see from the
requested line to the end of the buffer if the requeseted line doesn't end
up at the top.

All scroll calculations are done in terms of characters, not pixels.  I tried
this and it gave very nice scrolling but the calculations for optimizing the
expose function were kinda scary, so I dropped.  The end result is that it
looks better if you use a fixed width font.

Freeze/Thaw shouldn't be necessary - I should be checking to see if the added
text is visible and if not, just updating the scrollbars (but even that can
take time)

Wish-list:
remove the scrollbars and use in a GtkScrolledWindow
make scrolling pixel based
mouse scrolling support
accessor functions
documentation ;)
"""

import gtk

class pguTextArea( gtk.Table ):
    
    def __init__(self):
        gtk.Table.__init__( self, rows=2, columns=2 )
        
        self.hadj = gtk.Adjustment()
        self.vadj = gtk.Adjustment()
        
        self._hscroll = gtk.HScrollbar( adj = self.hadj )
        self._vscroll = gtk.VScrollbar( adj = self.vadj )
        self._area = gtk.DrawingArea()
        #set events for scrolling (not defined in GDK
        #self._area.set_events(1 << 21)
        self.contents = []
        self.max_width = 0
        self.max_length = 0
        self.height = 0
        self.line_height = 0
        self.start_line = 0
        self.start_col = 0
        self.freeze_count = 0
        self.updating = gtk.FALSE
        
        frm = gtk.Frame()
        frm.set_shadow_type( gtk.SHADOW_ETCHED_OUT )
        frm.add(self._area)
        self.attach( frm, 0, 1, 0, 1 )
        self.attach( self._vscroll, 1, 2, 0, 1, xoptions=gtk.SHRINK)
        self.attach( self._hscroll, 0, 1, 1, 2, yoptions=gtk.SHRINK )
                                                
        self.show_all()
        
        self._area.connect( 'expose-event', self.expose )
        self.connect( 'configure-event', self.configure )
        self.hadj.connect( 'value-changed', self.changed )
        self.vadj.connect( 'value-changed', self.changed )
        self._area.connect( "scroll-event", self.event )
        self.connect( 'style-set', self.expose )
        
    def freeze( self ):
        """Freeze the widget - no further updates until thawed
        """
        self.freeze_count = self.freeze_count + 1
        return
        
    def thaw( self ):
        """Thaw the widget - this may not cause an update unless
        the freeze count reaches 0
        """
        self.freeze_count = max( self.freeze_count - 1, 0 )
        if self.freeze_count == 0:
            self.calc_adjustments()
            self.expose()
        
    """
    this would handle scroll events but it seems to get into an endless
    loop sometimes????
    def event( self, widget, event ):
        #event type for scrolling not defined in GDK
        #and pygtk0.6.6 doesn't include a scroll event
        try:
            if event.type == 31:
                dir = 1
                if event.direction == 0:
                    dir = -1
                newval = max( self.vadj.value + dir, self.vadj.lower )
                self.vadj.value = min( newval, self.vadj.upper - self.vadj.page_size)
                self.vadj.value_changed()
        except:
            pass
    """        
    def changed( self, widget ):
        """Track changes to the scrollbars and record the 
        """
        self.start_line = int(self.vadj.value)
        self.start_col = int(self.hadj.value)
        self.expose()

    def append_text( self, text ):
        """Append a block of text to the widget
        
        Record new max width/length for scrollbar calculations
        """
        if len(text) > 0 and text[-1] == '\n':
            text = text[:-1]
        font = self.get_style().font
        rows = text.split( "\n" )
        for row in rows:
            idx = self.contents.append( row )
            self.max_width = max( font.measure(row), self.max_width )
            self.max_length = max( len(row), self.max_length )
            #record line height only once,
            #use ascent only, height (ascent+descent) includes extra whitespace
            if self.line_height == 0:
                self.line_height = int( font.extents(row)[3] )
        
        if self.freeze_count == 0:
            self.calc_adjustments()

    def calc_adjustments( self ):
        """Recalculate the adjustment settings
        """
        if self.freeze_count > 0:
            return
            
        self.updating = gtk.TRUE
        geom = self._area.get_allocation()
        #horizontal min/max are 0 and max line length - page size
        hpos = self.hadj.value
        if self.max_length == 0:
            hpage = 1
        else:
            hpage = int(geom[2]/(self.max_width/self.max_length)/2)
        hstep = int( hpage / 4 )
        self.hadj.set_all( hpos, 0, self.max_length - hpage, 1, hstep, hpage )
        self.hadj.changed()

        vpos = self.vadj.value
        if self.line_height == 0:
            vpage = 1
        else:
            vpage = int(geom[3]/(self.line_height))
        vstep = int( vpage / 4 )
        vmax = max( 1, len(self.contents) )
        self.vadj.set_all( vpos, 0, vmax, 1, vstep, vpage )
        self.vadj.changed()
        self.updating = gtk.FALSE
        
    def expose( self, *args ):
        """Draw the widget
        """
        if self.freeze_count > 0:
            return
            
        if not (self.flags() & gtk.REALIZED):
            return
        if not (self.flags() & gtk.MAPPED):
            return
            
        geom = self._area.get_allocation()
        pix = gtk.create_pixmap( self._area.window, geom[2], geom[3] )    
        style = self.get_style()
        gtk.draw_rectangle(pix, style.white_gc, gtk.TRUE, 0, 0, 
                                  geom[2], geom[3])
        gtk.draw_rectangle(pix, style.black_gc, gtk.FALSE, 0, 0, 
                                  geom[2], geom[3])
        font = self.get_style().font
        
        v_offset = 0
        for line in self.contents[self.start_line:]:
            v_offset = v_offset + self.line_height
            gtk.draw_string( pix, font, style.black_gc, 3, 
                                    v_offset, line[self.start_col:] )
            if v_offset > geom[3]:
                break
        
        self._area.draw_pixmap(style.white_gc, pix, 0, 0, 0, 0, geom[2]-1, geom[3]-1 )

        return gtk.FALSE
           
    def configure( self, widget, event, *args ):
        """Track changes in width, height
        """
        self.resize_children()
        self.calc_adjustments()
        self.expose()
        
    def scroll_to( self, line ):
        """Scroll the window to the requested line number
        
        Attempt to put the line at the top.  If there are less lines
        after the requested line than will fit the page, then it will
        move to a lower line number to display the whole last page
        """
        line = min( line, len(self.contents))
        line = min( line, len(self.contents) - self.vadj.page_size)
        if line != int(self.vadj.value):
            self.vadj.set_value( line )
        if int(self.hadj.value) != 0:
            self.hadj.set_value( 0 )
            
    def page_up(self):
        self.scroll_to( self.vadj.value - self.vadj.page_size )
        
    def page_down(self):
        self.scroll_to( self.vadj.value + self.vadj.page_size )
    

class TestText( gtk.Window ):

    def __init__(self):
        gtk.Window.__init__( self )
        self.set_title("Test TextArea")
        
        vbox = gtk.VBox()
        self.text = pguTextArea()
        self.entry = gtk.Entry()
        self.button = gtk.Button( "insert" )
        self.button.connect( "clicked", self.insert_text )
        
        vbox.pack_start( self.text )
        vbox.pack_start( self.entry, expand=gtk.FALSE )
        vbox.pack_start( self.button, expand=gtk.FALSE )
        self.add(vbox)
        self.show_all()
        
    def insert_text( self, *args ):
        self.text.append_text( self.entry.get_text() )
        self.entry.set_text( "" )
        
        
if __name__ == "__main__":
    win = TestText()
    for i in range(3):
        win.text.append_text( "This is a\ntest of some\nlonger inserts and should work really well" )
    win.connect( 'delete-event', gtk.main_quit )
    gtk.main()
    
