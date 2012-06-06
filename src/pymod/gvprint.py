###############################################################################
# $Id$
#
# Project:  OpenEV / CIETmap
# Purpose:  Print Dialog
# Author:   Frank Warmerdam, warmerda@home.com
#
# Maintained by Mario Beauchamp (starged@gmail.com) for CIETcanada
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

# differentiate between CIETMap and OpenEV
import os
if 'CIETMAP_HOME' in os.environ:
    import cview as gview
    from cietutils import set_help_topic
else:
    import gview
    from gvhtml import set_help_topic

from gvutils import error
import pygtk
pygtk.require('2.0')
import gtk
import pgu
from osgeo import gdal

# temporary
def _(s):
    return s

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
        self.set_title(_("Print"))
        self.connect('delete-event',self.close)
        self.view = view
        self.command = gview.get_preference('print_command', 'lpr')

        cgroup = gtk.VBox(spacing=6)
        cgroup.set_border_width(5)
        self.add(cgroup)
        frame = gtk.Frame()
        cgroup.pack_start(frame, expand=False)

        table = gtk.Table()
        table.set_row_spacings(6)
        table.set_border_width(5)
        frame.add(table)

        # Setup Driver Option Menu
        driver_label = pgu.Label(_("Driver:"))
        table.attach(driver_label, 0,1,0,1, yoptions=gtk.SHRINK)
        if os.name == 'nt':
            drivers = ("PostScript", "TIFF", "PNG", "Windows Print Driver", "GIF")
        else:
            drivers = ("PostScript", "TIFF", "PNG", "GIF")
        self.driver = pgu.ComboText(strings=drivers, action=self.update_cb)
        table.attach(self.driver, 1,2,0,1, yoptions=gtk.SHRINK)

        # Setup Device Option Menu
        device_label = pgu.Label(_("Device:"))
        table.attach(device_label, 0,1,1,2, yoptions=gtk.SHRINK)
        self.device = pgu.ComboText(strings=(_("File"), _("Spool to Printer")), action=self.device_cb)
        table.attach(self.device, 1,2,1,2, yoptions=gtk.SHRINK)

        # Setup File/Command entry.
        self.file_label = pgu.Label(_("File:"))
        table.attach(self.file_label, 0,1,2,3, yoptions=gtk.SHRINK)
        self.file = gtk.Entry()
        table.attach(self.file, 1,2,2,3, yoptions=gtk.SHRINK)

        # Setup Output Type
        self.output_label = pgu.Label(_("Output type:"))
        table.attach(self.output_label, 0,1,3,4, yoptions=gtk.SHRINK)
        self.output = pgu.ComboText(strings=(_("Greyscale"), _("Color")))
        table.attach(self.output, 1,2,3,4, yoptions=gtk.SHRINK)

        # Setup Paper Type
        self.paper_label = pgu.Label(_("Paper:"))
        table.attach(self.paper_label, 0,1,4,5, yoptions=gtk.SHRINK)
        sizes = []
        for entry in paper_sizes:
            sizes.append(entry[0])
        self.paper = pgu.ComboText(strings=sizes, action=self.update_cb)
        table.attach(self.paper, 1,2,4,5, yoptions=gtk.SHRINK)

        # Setup Scale slider
        self.scale_label = pgu.Label(_("Scale:"))
        table.attach(self.scale_label, 0,1,5,6, yoptions=gtk.SHRINK)
        self.scale_adjustment = gtk.Adjustment(1, 0, 1.25, 0.05, 0.05, 0.05)
        self.scale_slider = gtk.HScale(self.scale_adjustment)
        table.attach(self.scale_slider, 1,2,5,6, yoptions=gtk.SHRINK)

        # Setup Resolution spinner
        resolution_label = pgu.Label(_("Resolution:"))
        table.attach(resolution_label, 0,1,6,7, yoptions=gtk.SHRINK)
        self.resolution_adjustment = gtk.Adjustment(1, 0, 10, 0.1, 0.1, 0.1)
        self.resolution_spinner = gtk.SpinButton(self.resolution_adjustment, climb_rate=0.1, digits=1)
        self.resolution_spinner.connect('changed', self.resolution_cb)
        table.attach(self.resolution_spinner, 1,2,6,7, yoptions=gtk.SHRINK)

        # Setup Size entries
        size_label = pgu.Label(_("Image size:"))
        table.attach(size_label,0,1,7,8, yoptions=gtk.SHRINK)
        size_box = gtk.HBox(spacing=5)

        self.xsize_entry = gtk.Entry()
        self.xsize_entry.connect('activate', self.resolution_cb)
        self.xsize_entry.connect('leave-notify-event', self.resolution_cb)
        size_box.pack_start(self.xsize_entry)
        size_box.pack_start(gtk.Label("x"))

        self.ysize_entry = gtk.Entry()
        self.ysize_entry.connect('activate', self.resolution_cb)
        self.ysize_entry.connect('leave-notify-event', self.resolution_cb)
        size_box.pack_start(self.ysize_entry)
        table.attach(size_box, 1,2,7,8, yoptions=gtk.SHRINK)

        # Add Print, and Close button(s)
        btn_box = gtk.HBox(homogeneous=True, spacing=20)

        but = gtk.Button(stock=gtk.STOCK_PRINT)
        but.connect('clicked',self.print_cb)
        btn_box.pack_start(but, expand=False)

        but = gtk.Button(stock=gtk.STOCK_CLOSE)
        but.connect('clicked',self.close)
        btn_box.pack_start(but, expand=False)

        cgroup.pack_end(btn_box, expand=False)

        # Initialize values.
        # lazy
        prefs = gview.get_preference
        pref = prefs('print_driver', -1)
        self.driver.set_active(int(pref))
        if os.name == 'nt':
            self.driver.set_active(DR_WINPRINT)

        pref = prefs('print_device', -1)
        self.device.set_active(int(pref))

        if self.device.get_active() == DV_FILE:
            self.set_default_filename()
        else:
            self.file.set_text(self.command)

        pref = prefs('print_paper', -1)
        self.paper.set_active(int(pref))

        pref = prefs('print_output', -1)
        self.output.set_active(int(pref))

        pref = prefs('print_resolution')
        if pref:
            resolution = float(pref)
            self.resolution_adjustment.set_value(resolution)
            width = int(self.view.get_width() * resolution + 0.5)
            height = int(self.view.get_height() * resolution + 0.5)
            self.xsize_entry.set_text(str(width))
            self.ysize_entry.set_text(str(height))

        self.set_paper_size()
        self.scale_adjustment.set_value(1.0)

        if 'CIETMAP_HOME' in os.environ:
            topic = "Pages/Printing.htm"
            self.filename = 'cietmap.ps'
        else:
            topic = "gvprint.html"
            self.filename = 'openev.ps'
        set_help_topic(self, topic)
        # Show
        self.show_all()
        self.update_cb()

    def resolution_cb(self, entry, *args):
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
        if self.device.get_active() == DV_FILE:
            self.command = self.file.get_text()
            self.set_default_filename()
        else:
            self.file.set_text(self.command)
        self.update_cb(args)

    def set_default_filename(self):
        driver = self.driver.get_active()
        if 'CIETMAP_HOME' in os.environ:
            basename = "cietmap"
        else:
            basename = "openev"
        if driver == DR_TIFF:
            self.file.set_text("%s.tif" % basename)
        elif driver == DR_PNG:
            self.file.set_text("%s.png" % basename)
        elif driver == DR_GIF:
            self.file.set_text("%s.gif" % basename)
        else:
            self.file.set_text("%s.ps" % basename)

    def set_paper_size(self):
        # Setup paper size.
        self.paper_x = 8.5
        self.paper_y = 11
        try:
            entry = paper_sizes[self.paper.get_active()]
            self.paper_x = entry[1]
            self.paper_y = entry[2]
        except:
            pass

    def update_cb(self, *args):
        driver = self.driver.get_active()

        # Set FILE/PRINTER Device based on driver.
        if driver in (DR_TIFF, DR_PNG, DR_GIF):
            self.device.set_active(DV_FILE)
        if driver == DR_WINPRINT:
            self.device.set_active(DV_PRINTER)
        if driver == DR_POSTSCRIPT and os.name == 'nt':
            self.device.set_active(DV_FILE)

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

        if self.device.get_active() == DV_PRINTER:
            self.file_label.set_text(_("Command:"))
        else:
            self.file_label.set_text(_("File:"))

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

        self.resize_children()

    def print_cb(self, *args):
        rez = self.resolution_adjustment.value
        if rez >= 0.99 and rez <= 1.01:
            width = self.view.get_width()
            height = self.view.get_height()
        else:
            width = self.view.get_width() * rez
            height = self.view.get_height() * rez
            width = int(width+0.5)
            height = int(height+0.5)

        if width/self.paper_x > height/self.paper_y:
            pixels_per_inch = width / (self.paper_x*0.9)
        else:
            pixels_per_inch = height / (self.paper_y*0.9)

        pixels_per_inch = pixels_per_inch * rez
        ulx = (self.paper_x - width/pixels_per_inch)/2.0
        uly = (self.paper_y - height/pixels_per_inch)/2.0
        lrx = self.paper_x - ulx
        lry = self.paper_y - uly

        try:
            os.unlink(self.file.get_text())
        except:
            pass

        err = 0
        driver = self.driver.get_active()
        output = self.output.get_active()
        filename = self.file.get_text()
        if driver == DR_POSTSCRIPT:
            if self.device.get_active() == DV_PRINTER:
                filename = '|' + filename
            err = self.view.print_postscript_to_file(width, height,
                                                    ulx, uly, lrx, lry,
                                                    output, filename)
        elif driver == DR_TIFF:
            err = self.view.print_to_file(width, height, filename, 'GTiff', output)
        elif driver == DR_PNG:
            err = self.view.print_to_file(width, height, '_temp.tif', 'GTiff', output)
            if not err:
                gdal.GetDriverByName('PNG').CreateCopy(filename, gdal.Open('_temp.tif'), True)
            os.unlink('_temp.tif')
        elif driver == DR_WINPRINT:
            self.view.print_to_windriver(width, height, ulx, uly, lrx, lry, output)
        elif driver == DR_GIF:
            err = self.view.print_to_file(width, height, '_temp.tif', 'GTiff', output)
            if not err:
                if output == 1:
                    gdal.RGBFile2PCTFile('_temp.tif', '_temp2.tif')
                    os.unlink('_temp.tif')
                    os.rename('_temp2.tif', '_temp.tif')

                gdal.GetDriverByName('GIF').CreateCopy(filename, gdal.Open('_temp.tif'), True)
            os.unlink('_temp.tif')

        if err:
            error(_("The request to print appears to have failed."))

        self.close()

    def close(self, *args):
        # lazy
        pref = gview.set_preference
        if self.device.get_active() == DV_PRINTER:
            pref('print_command', self.file.get_text())
        pref('print_driver', str(self.driver.get_active()))
        pref('print_device', str(self.device.get_active()))
        pref('print_paper', str(self.paper.get_active()))
        pref('print_output', str(self.output.get_active()))
        pref('print_resolution', str(self.resolution_adjustment.value))
        pref('print_scale', str(self.scale_adjustment.value))

        self.hide()
        self.destroy()

        return True
