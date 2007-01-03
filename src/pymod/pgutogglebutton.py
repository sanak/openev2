#!/usr/bin/env python
###############################################################################
# $Id: pgutogglebutton.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
#
# Project:  OpenEV Python GTK Utility classes
# Purpose:  Embeddable, configurable toggle widget
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

import gtk
from gtk import TRUE, FALSE
import gview
import os.path

class pguToggleButton(gtk.ToggleButton):
    """
    a widget for displaying toggled state (on/off).
    """

    def __init__(self, pix_on = "ck_on_l.xpm", pix_off = "ck_off_l.xpm"):
        """
        """
        gtk.ToggleButton.__init__( self )
        
        filename = os.path.join(gview.home_dir, 'pics', pix_on)

        pix, mask = gtk.gdk.pixmap_colormap_create_from_xpm(None,
            gtk.gdk.colormap_get_system(), None, filename)
        self.pix_on = gtk.Image()
        self.pix_on.set_from_pixmap(pix, mask)
        self.pix_on.show()

        # OLD...
        #pix, mask = gtk.create_pixmap_from_xpm(self, None, filename)
        #self.pix_on = gtk.Pixmap( pix, mask )
        #self.pix_on.show()
        
        filename = os.path.join(gview.home_dir, 'pics', pix_off)

        pix, mask = gtk.gdk.pixmap_colormap_create_from_xpm(None,
            gtk.gdk.colormap_get_system(), None, filename)
        self.pix_off = gtk.Image()
        self.pix_off.set_from_pixmap(pix, mask)
        self.pix_off.show()

        # OLD...
        #pix, mask = gtk.create_pixmap_from_xpm(self, None, filename)
        #self.pix_off = gtk.Pixmap( pix, mask )
        #self.pix_off.show()
        
        self.add( self.pix_off )
        
        self.active_pix = self.pix_off
        
        self.set_size_request(*pix.get_size())
       
        self.connect( 'toggled', self.expose )
        self.connect( 'expose-event', self.expose )
        self.show()
        
    def expose( self, *args ):
        
        if not self.flags() & gtk.REALIZED:
            return
            
        
        if self.get_active():
            active_pix = self.pix_on
        else:
            active_pix = self.pix_off
        if active_pix != self.active_pix:
            self.remove( self.active_pix )
            self.active_pix = active_pix
            self.add( self.active_pix )
        
if __name__ == "__main__":
    dlg = gtk.Dialog()
    filename = os.path.join(gview.home_dir, 'pics')
    print 'pixs from ', filename
    tb = pguToggleButton()
    dlg.vbox.pack_start( tb )
    
    btn = gtk.Button( "OK" )
    btn.connect( 'clicked', gtk.main_quit )
    dlg.action_area.pack_start( btn )
    dlg.connect( 'delete-event', gtk.main_quit )
    dlg.show_all()
    gtk.main()
    
