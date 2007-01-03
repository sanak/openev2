###############################################################################
# $Id: imgproctemplate.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Template Image Processing "tool".  Seach for CHANGEME.
# Author:   Frank Warmerdam, warmerdam@pobox.com
#
###############################################################################
# Copyright (c) 2002, Frank Warmerdam <warmerdam@pobox.com>
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

import gview
import string           
import gvutils
import os
import sys
import gviewapp		
import gdalnumeric

class ImageProcTool(gviewapp.Tool_GViewApp):
    
    def __init__(self,app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.init_menu()

    def launch_dialog(self,*args):
        self.win = ImageProcDialog()
        self.win.update_gui()
        self.win.show()

    def init_menu(self):
        self.menu_entries.set_entry("Tools/Image Processing",2,
                                    self.launch_dialog)

class ImageProcDialog(gtk.Window):

    def __init__(self,app=None):
        gtk.Window.__init__(self)

        self.set_title('Image Processing')

        self.create_gui()
        self.show()

    def show(self):
        gtk.Window.show_all(self)

    def close(self, *args):
        self.hide()
        self.visibility_flag = 0
        return gtk.TRUE

    def create_gui(self):
        box1 = gtk.VBox()
        self.add(box1)
        box1.show()

        text_view = gtk.TextView(gtk.TextBuffer())
        text_view.set_editable(gtk.FALSE)
        text_view.set_size_request(400,150)
        text_view.set_wrap_mode(gtk.WRAP_NONE)
        text_view.show()
        self.text_view = text_view
        box1.pack_start(text_view, expand=gtk.TRUE)

        box2 = gtk.HBox()
        box1.pack_start(box2, expand=gtk.FALSE)
        box2.show()

        self.execute_btn = gtk.Button("Execute")
        self.execute_btn.connect("clicked", self.execute_cb)
        box2.pack_start(self.execute_btn)
        
        self.close_btn = gtk.Button("Close")
        self.close_btn.connect("clicked", self.close)
        box2.pack_start(self.close_btn)

    def update_gui(self,*args):
        pass

    ##########################################################################
    # Return the currently selected raster layer.
    #
    # Reports an error and returns None if there isn't a suitable raster
    # layer selected
    #
    def get_cur_rlayer(self):

        rlayer = gview.app.sel_manager.get_active_layer()

        if rlayer is None:
            gvutils.error( 'Please select a raster layer.' )
            return None

        # This will only work for a real GvRasterLayer
        try:
            nd = rlayer.get_nodata(0)
            return rlayer
        
        except:
            gvutils.error( 'Please select a raster layer.' )
            return None

    ##########################################################################
    # Load indicated raster layer into a memory array.
    #
    # Note that all bands are loaded, not just the ones being displayed in
    # in the view. 

    def load_layer(self, rl, xoff=0, yoff=0, xsize=None, ysize=None ):
        filename = rl.get_parent().get_dataset().GetDescription()
        target_data = gdalnumeric.LoadFile(filename,xoff,yoff,xsize,ysize)
        return target_data;

    ##########################################################################
    # Save image array into existing raster file.
    #

    def save_layer(self, rl, rd, xoff=0, yoff=0 ):
        ds = rl.get_parent().get_dataset()

        if len(rd.shape) == 3:
            for iBand in range(len(rd.shape)):
                gdal_band = ds.GetRasterBand(iBand+1)
                rd_band = rd[iBand]

                gdal_band.WriteArray( rd_band, xoff, yoff )
        else:
                gdal_band = ds.GetRasterBand(1)
                gdal_band.WriteArray( rd, xoff, yoff )

    ##########################################################################
    # Main algorithm execution callback.
    
    def execute_cb( self, *args ):

        print 'Entering execute_cb()'

        try:
            # Determine the currently selected raster layer.
            rl = self.get_cur_rlayer()
            if rl is None:
                print 'didnt get rlayer'
                return

            # Load it into memory.
            rd = self.load_layer( rl )
            if rd is None:
                print 'didnt load layer'
                return 

            # Apply algorithm to raster data. *** CHANGEME ***
            rd = rd * 0.5

            # Save results back into source raster layer.
            self.save_layer( rl, rd )
            rl.refresh()
            
        except:
            print 'Trapped Error'
            sys.excepthook(sys.exc_info()[0],
                           sys.exc_info()[1],
                           sys.exc_info()[2])

        print 'Exiting execute_cb()'

TOOL_LIST = ['ImageProcTool']

