###############################################################################
# $Id: gvprint.py,v 1.1.1.1 2005/04/18 16:38:35 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Print Dialog
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

from gvconst import *
import gview
import gvutils
import gdal
import os
import gvhtml

paper_sizes = ( ("US Letter",       8.500, 11.000 ),
                ("US Legal",        8.500, 14.000 ),
                ("A4",              8.268, 11.693 ),
                ("B5",              7.205, 10.118 ),
                ("A3",             11.693, 16.535 ) )

DR_POSTSCRIPT = 0
DR_TIFF = 1
DR_PNG = 2
DR_WINPRINT = 3
DR_GIF = 4

DV_FILE = 0
DV_PRINTER = 1

class GvPrintDialog(gtk.Window):

    def __init__(self, view):
        gtk.Window.__init__(self)
        self.set_title('Print')
        self.connect('delete-event',self.close)
        self.view = view

        gvhtml.set_help_topic( self, "gvprint.html" );

        self.command = gview.get_preference('print_command')
        if self.command is None:
            self.command = 'lpr'
            
        self.filename = 'openev.ps'

        cgroup = gtk.VBox(spacing=6)
        cgroup.set_border_width(10)
        self.add( cgroup )

        table = gtk.Table()
        table.n_columns = 2
        table.n_rows = 4
        cgroup.add(table)

        # Setup Driver Option Menu
	driver_label = gtk.Label('Driver:')
	driver_label.set_alignment(0, 0.5)
        table.attach(driver_label,0,1,0,1)
        if os.name == "nt":
            self.driver = gvutils.GvOptionMenu( ('PostScript', 'TIFF', 'PNG',
                                                 'Windows Print Driver',
                                                 'GIF' ),
                                                self.update_cb )
        else:
            self.driver = gvutils.GvOptionMenu( ('PostScript', 'TIFF', 'PNG',
                                                 '', 'GIF' ),
                                                self.update_cb )
        table.attach(self.driver,1,2,0,1)

        # Setup Device Option Menu
	device_label = gtk.Label('Device:')
	device_label.set_alignment(0, 0.5)
        table.attach(device_label,0,1,1,2)
        self.device = gvutils.GvOptionMenu( ('File', 'Spool to Printer'),
                                            self.device_cb )
        table.attach(self.device,1,2,1,2)

        # Setup File/Command entry.
        self.file_label = gtk.Label('File:')
	self.file_label.set_alignment(0, 0.5)
        table.attach(self.file_label,0,1,2,3)
        self.file = gtk.Entry()
        self.file.set_max_length(40)
        table.attach(self.file,1,2,2,3)

        # Setup Output Type
        self.output_label = gtk.Label('Output Type:')
	self.output_label.set_alignment(0, 0.5)
        table.attach(self.output_label,0,1,3,4)
        self.output = gvutils.GvOptionMenu( ('Greyscale', 'Color' ), None )
        table.attach(self.output,1,2,3,4)

        # Setup Paper Type
        self.paper_label = gtk.Label('Paper:')
	self.paper_label.set_alignment(0, 0.5)
        table.attach(self.paper_label,0,1,4,5)
        sizes = []
        for entry in paper_sizes:
            sizes.append( entry[0] )
        self.paper = gvutils.GvOptionMenu( sizes, self.update_cb )
        table.attach(self.paper,1,2,4,5)

        # Setup Scale slider
        self.scale_label = gtk.Label('Scale:')
	self.scale_label.set_alignment(0, 0.5)
        table.attach(self.scale_label,0,1,5,6)
        self.scale_adjustment = gtk.Adjustment(1, 0, 1.25, 0.05, 0.05, 0.05)
        self.scale_slider = gtk.HScale(self.scale_adjustment)
        table.attach(self.scale_slider,1,2,5,6)

        # Setup Resolution spinner
	resolution_label = gtk.Label('Resolution:')
	resolution_label.set_alignment(0, 0.5)
        table.attach(resolution_label,0,1,6,7)
        self.resolution_adjustment = gtk.Adjustment(1, 0, 10, 0.1, 0.1, 0.1)
	self.resolution_spinner = \
	    gtk.SpinButton(self.resolution_adjustment,climb_rate=0.1,digits=1)
	self.resolution_spinner.connect("changed", self.resolution_cb)
	table.attach(self.resolution_spinner,1,2,6,7)

        # Setup Size entries
	size_label = gtk.Label('Image size:')
	size_label.set_alignment(0, 0.5)
        table.attach(size_label,0,1,7,8)
	size_box = gtk.HBox(spacing=5)
	self.xsize_entry = gtk.Entry()
	self.xsize_entry.connect('activate', self.resolution_cb)
	self.xsize_entry.connect('leave-notify-event', self.resolution_cb)
	size_box.pack_start(self.xsize_entry)
	size_box.pack_start(gtk.Label('x'))
	self.ysize_entry = gtk.Entry()
	self.ysize_entry.connect('activate', self.resolution_cb)
	self.ysize_entry.connect('leave-notify-event', self.resolution_cb)
	size_box.pack_start(self.ysize_entry)
        table.attach(size_box,1,2,7,8)

        # Add Print, and Close button(s)
	btn_box = gtk.HBox(spacing=10)

        but = gtk.Button('Print')
        but.connect('clicked',self.print_cb)
	btn_box.pack_start(but)

        but = gtk.Button('Close')
        but.connect('clicked',self.close)
	btn_box.pack_start(but)

        table.attach(btn_box,0,2,8,9)

        # Initialize values.
        if gview.get_preference('print_driver') is not None:
            self.driver.set_history(int(gview.get_preference('print_driver')))
        elif os.name == 'nt':
            self.driver.set_history(DR_WINPRINT)

        if gview.get_preference('print_device') is not None:
            self.device.set_history(int(gview.get_preference('print_device')))

        if self.device.get_history() == 0:
            self.set_default_filename()
        else:
            self.file.set_text( self.command )

        if gview.get_preference('print_paper') is not None:
            self.paper.set_history(int(gview.get_preference('print_paper')))

        if gview.get_preference('print_output') is not None:
            self.output.set_history(int(gview.get_preference('print_output')))

        if gview.get_preference('print_resolution') is not None:
	    resolution = float(gview.get_preference('print_resolution'))
            self.resolution_adjustment.set_value(resolution)
	    width = int(self.view.get_width() * resolution + 0.5)
	    height = int(self.view.get_height() * resolution + 0.5)
	    self.xsize_entry.set_text(str(width))
	    self.ysize_entry.set_text(str(height))

        self.set_paper_size()
        self.scale_adjustment.set_value(1.0)

        # Show
        table.set_row_spacings(6)
        table.show_all()
        self.update_cb()
        cgroup.show()
        self.show()

    def resolution_cb(self,entry,*args):
        try:
            value = float(entry.get_text())
	except:
	    return

	if entry == self.resolution_spinner:
	    resolution = self.resolution_adjustment.value
	    width = int(self.view.get_width() * resolution + 0.5)
	    self.xsize_entry.set_text(str(width))
	    height = int(self.view.get_height() * resolution + 0.5)
	    self.ysize_entry.set_text(str(height))
	elif entry == self.xsize_entry:
	    resolution = value / self.view.get_width()
	    height = int(self.view.get_height() * resolution + 0.5)
	    self.ysize_entry.set_text(str(height))
	elif entry == self.ysize_entry:
	    resolution = value / self.view.get_height()
	    width = int(self.view.get_width() * resolution + 0.5)
	    self.xsize_entry.set_text(str(width))
	self.resolution_adjustment.set_value(resolution)
	

    def device_cb(self, *args):
        if self.device.get_history() == 0:
            self.command = self.file.get_text()
            self.set_default_filename()
        else:
            self.file.set_text(self.command)
        self.update_cb( args )

    def set_default_filename(self):
        if self.driver.get_history() == DR_TIFF:
            self.file.set_text('openev.tif')
        elif self.driver.get_history() == DR_PNG:
            self.file.set_text('openev.png')
        elif self.driver.get_history() == DR_GIF:
            self.file.set_text('openev.gif')
        else:
            self.file.set_text('openev.ps')

    def set_paper_size(self):
        # Setup paper size.
        self.paper_x = 8.5
        self.paper_y = 11
        try:
            entry = paper_sizes[self.paper.get_history()]
            self.paper_x = entry[1]
            self.paper_y = entry[2]
        except:
            pass

    def update_cb(self, *args):
        
        driver = self.driver.get_history()

        # Set FILE/PRINTER Device based on driver.
        if driver == DR_TIFF or driver == DR_PNG or driver == DR_GIF:
            self.device.set_history(DV_FILE)
        if driver == DR_WINPRINT:
            self.device.set_history(DV_PRINTER)
        if driver == DR_POSTSCRIPT and os.name == 'nt':
            self.device.set_history(DV_FILE)

        self.set_paper_size()

        # Hide the file/command tool for WINDRIVER
        if driver == DR_WINPRINT:
            self.file_label.hide()
            self.file.hide()
            self.output_label.hide()
            self.output.hide()
        else:
            self.file_label.show()
            self.file.show()
            self.output_label.show()
            self.output.show()

        if self.device.get_history() == DV_PRINTER:
            self.file_label.set_text('Command:')
        else:
            self.file_label.set_text('File:')

        # Make Positioning controls visible only for PostScript
        if driver == DR_POSTSCRIPT:
            self.scale_label.show()
            self.scale_slider.show()
            self.paper_label.show()
            self.paper.show()
        else:
            self.scale_label.hide()
            self.scale_slider.hide()
            self.paper_label.hide()
            self.paper.hide()
                    
    def print_cb(self, *args):
        if self.resolution_adjustment.value >= 0.99 \
           and self.resolution_adjustment.value <= 1.01:
            width = self.view.get_width()
            height = self.view.get_height()
        else:
            width = self.view.get_width() * self.resolution_adjustment.value
            height = self.view.get_height() * self.resolution_adjustment.value
            width=int(width+0.5)
            height=int(height+0.5)

        if width / self.paper_x > height / self.paper_y:
            pixels_per_inch = width / (self.paper_x*0.9)
        else:
            pixels_per_inch = height / (self.paper_y*0.9)

        pixels_per_inch = pixels_per_inch * self.scale_adjustment.value
        ulx = (self.paper_x - width/pixels_per_inch)/2.0
        uly = (self.paper_y - height/pixels_per_inch)/2.0
        lrx = self.paper_x - ulx
        lry = self.paper_y - uly
        
        try:
            os.unlink( self.file.get_text() )
        except:
            pass

        err = 0            
        if self.driver.get_history() == DR_POSTSCRIPT:
            filename = self.file.get_text()
            if self.device.get_history() == 1:
                filename = '|' + filename
                
            err = self.view.print_postscript_to_file(width,height,
                                               ulx,uly,lrx,lry,
                                               self.output.get_history(),
                                               filename )
        elif self.driver.get_history() == DR_TIFF:
            err = self.view.print_to_file(width,height,self.file.get_text(),
                                    'GTiff',self.output.get_history())
        elif self.driver.get_history() == DR_PNG:
            err = self.view.print_to_file(width,height,'_temp.tif','GTiff',
                                          self.output.get_history())
            if err == 0:
                gdal.GetDriverByName('PNG').CreateCopy(self.file.get_text(),
                                                   gdal.Open('_temp.tif'),TRUE)
            os.unlink( '_temp.tif' )
        elif self.driver.get_history() == DR_WINPRINT:
            self.view.print_to_windriver( width, height, ulx, uly, lrx, lry,
                                          self.output.get_history() )
        elif self.driver.get_history() == DR_GIF:
            err = self.view.print_to_file(width,height,'_temp.tif','GTiff',
                                    self.output.get_history())
            if err == 0:
                if self.output.get_history() == 1:
                    gdal.RGBFile2PCTFile( '_temp.tif', '_temp2.tif' )
                    os.unlink('_temp.tif')
                    os.rename('_temp2.tif','_temp.tif')
                
                gdal.GetDriverByName('GIF').CreateCopy(self.file.get_text(),
                                                  gdal.Open('_temp.tif'),TRUE)
            os.unlink( '_temp.tif' )

        if err != 0:
            gvutils.error('The request to print appears to have failed.')

        self.close()
            
    def close(self, *args):
        if self.device.get_history() == 1:
            gview.set_preference('print_command',self.file.get_text())
        gview.set_preference('print_driver', str(self.driver.get_history()))
        gview.set_preference('print_device', str(self.device.get_history()))
        gview.set_preference('print_paper', str(self.paper.get_history()))
        gview.set_preference('print_output', str(self.output.get_history()))
        gview.set_preference('print_resolution',
                             str(self.resolution_adjustment.value))
        gview.set_preference('print_scale',
                             str(self.scale_adjustment.value))
            
        self.destroy()
        
        return TRUE
        

if __name__ == '__main__':
    dialog = GvPrintDialog(None)

    dialog.connect('delete-event', gtk.main_quit)

    gtk.main()
