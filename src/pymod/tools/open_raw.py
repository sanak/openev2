###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Interactive tool to open raw image files.
# Author:   Andrey Kiselev, dron@remotesensing.org
#
###############################################################################
# Copyright (c) 2004, American Museum of Natural History. All rights reserved.
# This software is based upon work supported by NASA under award
# number NAG5-12333
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
# TO DO:
#
# 1) Allow output of more header types by adding a header-only
#    creation option for flat binary raster + header type images?
#
# 2) Add advanced option to window the input file:  The current
#   implementation doesn't make full use of vrt flexibility- the
#   user is forced to load the whole image.  The
#   LineOffset, PixelOffset, and ImageOffset parameters could
#   also be used to load a sub image rather than the whole thing.
#   The user would have to specify the existing parameters
#   (which describe the whole file) plus line/pixel offsets and
#   sizes for the subwindow.

import gtk

from stat import *
import os
import os.path
import sys
import math
import Numeric

import gdal
import gdalnumeric
import gview
import gviewapp
import gvutils
import string
import vrtutils
import pgufilesel

class OpenRaw(gviewapp.Tool_GViewApp):

    def __init__(self,app=None):
	self.wins = {}
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.init_menu()

    def launch_dialog(self,*args):
        win=OpenRawDialog(self.app)
        win.show()

    def init_menu(self):
        self.menu_entries.set_entry("File/Open Raw",2,self.launch_dialog)

class OpenRawDialog(gtk.Window):

    def __init__(self,app=None):
	self.wins = {}
        gtk.Window.__init__(self)
        self.set_title('Open Raw Image File')
        self.create_gui()
        self.show()
        self.app=app

    def show(self):
        gtk.Window.show_all(self)

    def close(self, *args):
        self.destroy()

    def create_gui(self):
        box1 = gtk.VBox(spacing = 10)
	box1.set_border_width(10)
        self.add(box1)
        self.tips=gtk.Tooltips()
        box1.show()

	# File open controls
	frame1 = gtk.Frame('Select raw image file')
	frame1.show()
        box1.pack_start(frame1, expand=False)
	box2 = gtk.HBox(spacing = 5)
	box2.set_border_width(5)
        box2.show()

	open_btn = gtk.Button('Open...')
	open_btn.connect("clicked", self.open_cb)
	box2.pack_start(open_btn)
	self.open_entry = gtk.Entry()
	self.open_entry.set_editable(True)
	self.open_entry.set_text('')
	box2.pack_start(self.open_entry)
	frame1.add(box2)

	# Image geometry controls
	frame2 = gtk.Frame('Set image geometry')
	frame2.show()
        box1.pack_start(frame2, expand=False)
	tbl = gtk.Table(4, 5)
	tbl.set_border_width(5)
	tbl.set_row_spacings(5)
	tbl.set_col_spacings(5)
        tbl.show()

	width_label = gtk.Label('Image width:')
	width_label.set_alignment(0, 0.5)
	tbl.attach(width_label, 0, 1, 0, 1)
	self.width_entry = gtk.Entry()
	self.width_entry.set_text('0')
	self.width_entry.set_editable(True)
	tbl.attach(self.width_entry, 1, 2, 0, 1)

	height_label = gtk.Label('Image height:')
	height_label.set_alignment(0, 0.5)
	tbl.attach(height_label, 0, 1, 1, 2)
	self.height_entry = gtk.Entry()
	self.height_entry.set_text('0')
	self.height_entry.set_editable(True)
	tbl.attach(self.height_entry, 1, 2, 1, 2)

	bands_label = gtk.Label('Number of bands:')
	bands_label.set_alignment(0, 0.5)
	tbl.attach(bands_label, 0, 1, 2, 3)
	self.bands_entry = gtk.Entry()
	self.bands_entry.set_text('1')
	self.bands_entry.set_editable(True)
	tbl.attach(self.bands_entry, 1, 2, 2, 3)

	header_label = gtk.Label('Image header size:')
	header_label.set_alignment(0, 0.5)
	tbl.attach(header_label, 0, 1, 3, 4)
	self.header_entry = gtk.Entry()
	self.header_entry.set_text('0')
	self.header_entry.set_editable(True)
	tbl.attach(self.header_entry, 1, 2, 3, 4)

	guess_btn = gtk.Button("Guess image geometry")
        guess_btn.connect("clicked", self.guess_cb)
	tbl.attach(guess_btn, 0, 2, 4, 5)

	size_label = gtk.Label('File size in bytes:')
	size_label.set_alignment(0, 0.5)
	tbl.attach(size_label, 2, 3, 0, 1)
	self.bytes_label = gtk.Label('')
	self.bytes_label.set_alignment(0, 0.5)
	tbl.attach(self.bytes_label, 3, 4, 0, 1)

	type_label = gtk.Label('Image data type:')
	type_label.set_alignment(0, 0.5)
	tbl.attach(type_label, 2, 3, 1, 2)
	self.type_list = ['Byte', 'UInt16', 'Int16', 'UInt32','Int32',
                          'Float32','Float64','CInt16','CInt32',
                          'CFloat32','CFloat64']
	self.type_menu = gvutils.GvOptionMenu(self.type_list)
	tbl.attach(self.type_menu, 3, 4, 1, 2)

	swap_label = gtk.Label('Byte Order:')
	swap_label.set_alignment(0, 0.5)
	tbl.attach(swap_label, 2, 3, 2, 3)
	swap_list = ['LSB (Swapped)', 'MSB (Unswapped)']
	self.swap_menu = gvutils.GvOptionMenu(swap_list)
	tbl.attach(self.swap_menu, 3, 4, 2, 3)

	interleave_label = gtk.Label('Type of interleaving:')
	interleave_label.set_alignment(0, 0.5)
	tbl.attach(interleave_label, 2, 3, 3, 4)
	self.interleave_list = ['Pixel', 'Band', 'Line']
	self.interleave_menu = gvutils.GvOptionMenu(self.interleave_list)
	tbl.attach(self.interleave_menu, 3, 4, 3, 4)

	frame2.add(tbl)

	# Output header format.  A possible route for future
        # improvement would be adding a creation option to CreateCopy for the
        # simple-flat-binary-raster-plus-header formats that specifies
        # header-only output and avoids copying all the associated binary data.
        # We might also want to add a GDAL_DMD-type metadata item to the driver
        # so that the tool can pick up which formats support header-only output
        # (sort of like the datatypes and creation options are currently set as
        # metadata items in the drivers).

        sbox = gtk.HBox(spacing=10)
        label=gtk.Label('Output Header Format:')
        label.set_alignment(0,0.5)
        label.show()
        sbox.pack_start(label)

        self.format_list = ['VRT','PAux']
	self.format_menu = gvutils.GvOptionMenu(self.format_list)
	self.format_menu.show()
	sbox.pack_start(self.format_menu)
        box1.pack_start(sbox)
        sbox.show()

	# Ok/Cancel buttons
	separator = gtk.HSeparator()
	box1.pack_start(separator, expand=False)

	box3 = gtk.HBox(spacing=10)
        box1.pack_start(box3, expand=False)

        current_btn = gtk.Button("Current View")
        current_btn.connect("clicked", self.import_cb,'Current')
	box3.pack_start(current_btn)

        new_btn = gtk.Button("New View")
        new_btn.connect("clicked", self.import_cb,'New')
	box3.pack_start(new_btn)

        save_btn = gtk.Button("Save")
        save_btn.connect("clicked", self.import_cb,'Save')
	box3.pack_start(save_btn)

        close_btn = gtk.Button("Close")
        close_btn.connect("clicked", self.close)
        box3.pack_start(close_btn)

        self.tips.set_tip(close_btn,
                          'Exit the Open Raw tool')
        self.tips.set_tip(save_btn,
                          'Create dataset and save header in selected format')
        self.tips.set_tip(new_btn,
                          'Create dataset and display in a new view')
        self.tips.set_tip(current_btn,
                          'Create dataset and display in current view')
        box3.show()

    def open_cb(self, *args):
        if gview.get_preference('save_recent_directory') == 'on':
	    recent_dir = gview.get_preference('recent_directory')
	else:
	    recent_dir = None

        filename=pgufilesel.GetFileName(title="Open raw image file",
                                        default_filename=recent_dir)
        if filename is None:
            return
        self.open_entry.set_text(filename)
        self.bytes_label.set_text(str(os.stat(filename)[ST_SIZE]))

    def import_cb(self, *args):
	# Check if the parameters valid
	filename = self.open_entry.get_text()
	if filename is '':
	    gvutils.error('You should select a raw file to load!')
	    return

	if not os.path.isfile(filename):
	    gvutils.error('Unable to load '+ filename)
	    return

        if args[1] == 'Save':
            self.create_header(filename)
        else:
            lines=self.create_vrt_lines(filename)
            vrtds=gdal.OpenShared(lines)
            if args[1] == 'New':
                self.app.new_view()
            self.app.open_gdal_dataset(vrtds)

    def create_vrt_lines(self,filename):
	image_offset = long(self.header_entry.get_text())
	width = long(self.width_entry.get_text())
	height = long(self.height_entry.get_text())
	bands = long(self.bands_entry.get_text())
	interleaving = self.interleave_list[self.interleave_menu.get_history()]
        dtype = self.type_list[self.type_menu.get_history()]
	gdaltype = gdal.GetDataTypeByName(dtype)
	datasize = gdal.GetDataTypeSize(gdaltype) / 8
        byteorder = ['LSB','MSB'][self.swap_menu.get_history()]

        vrtdsc = vrtutils.VRTDatasetConstructor(width,height)

        if interleaving == 'Pixel':
            pixoff = datasize*bands
            lineoff = pixoff*width
            for idx in range(bands):
                imoff = image_offset + idx*datasize
                vrtdsc.AddRawBand(filename, dtype, byteorder,
                                 imoff, pixoff, lineoff)

        elif interleaving == 'Line':
            pixoff=datasize
            lineoff=datasize*width*bands
            for idx in range(bands):
                imoff = image_offset + idx*lineoff
                vrtdsc.AddRawBand(filename, dtype, byteorder,
                                 imoff, pixoff, lineoff)

        else:
            pixoff=datasize
            lineoff=datasize*width
            for idx in range(bands):
                imoff = image_offset + datasize*width*height*idx
                vrtdsc.AddRawBand(filename, dtype, byteorder,
                                 imoff, pixoff, lineoff)

        return vrtdsc.GetVRTLines()


    def create_header(self,filename):
        fmt=self.format_list[self.format_menu.get_history()]
        dtype=self.type_list[self.type_menu.get_history()]
        dr=gdal.GetDriverByName(fmt)
        tlist=string.split(dr.GetMetadata()["DMD_CREATIONDATATYPES"])
        if dtype not in tlist:
            gvutils.error(fmt+' format does not support '+dtype+' data type!')
            return

        if fmt == 'PAux':
            self.create_paux_header(filename,dtype)
        else:
            fname,ext = os.path.splitext(filename)
            vrtname = fname+'.vrt'

            fname=pgufilesel.GetFileName(title="Select VRT Save Name",
                                         default_filename=vrtname)
            if fname is None:
                return
            if os.path.exists(fname):
	        resp = GtkExtra.message_box('Confirmation', \
		        fname + ' exists. Overwrite?', ('Yes','No'))
                if resp == 'No':
                    return
            lines=self.create_vrt_lines(filename)
            fh=open(fname,'w')
            fh.writelines(lines)
            fh.close()

    def create_paux_header(self,filename,datatype):    
	(path, ext) = os.path.splitext(filename)
	auxname = path + ".aux"
	if os.path.isfile(auxname):
	    resp = GtkExtra.message_box('Confirmation', \
		    auxname + ' exists. Overwrite?', ('Yes','No'))
            if resp == 'No':
                return

	# Take the image parameters
	header = long(self.header_entry.get_text())
	width = long(self.width_entry.get_text())
	height = long(self.height_entry.get_text())
	bands = long(self.bands_entry.get_text())
        aux_type_dict={'Byte':'8U','Int16':'16S','UInt16':'16U',
                       'Float32':'32R'}
	aux_type_list = ['8U', '16S', '16U', '32R']
	type = aux_type_dict[datatype]
	gdaltype = gdal.GetDataTypeByName(datatype)
	interleaving = self.interleave_list[self.interleave_menu.get_history()]
	datasize = gdal.GetDataTypeSize(gdaltype) / 8

	# Calculate offsets
	pixel_offset = []
	line_offset = []
	image_offset = []
	if interleaving is 'Pixel':
	    for i in range(bands):
		pixel_offset.append(datasize * bands)
		line_offset.append(datasize * width * bands)
		image_offset.append(header + datasize * i)
	elif interleaving is 'Band':
	    for i in range(bands):
		pixel_offset.append(datasize)
		line_offset.append(datasize * width)
		image_offset.append(header + datasize * width * height * i)
	elif interleaving is 'Line':
	    for i in range(bands):
		pixel_offset.append(datasize)
		line_offset.append(datasize * width * bands)
		image_offset.append(header + datasize * width * i)
	else:
	    raise 'Unsupported interleaving type!'

	aux_swap_list = ['Swapped', 'Unswapped']
	swap = aux_swap_list[self.swap_menu.get_history()]

	# Write out the auxilary file
	aux = open(auxname, "wt")
	aux.write("AuxilaryTarget: " +  os.path.basename(filename) + '\n')
	aux.write("RawDefinition: " + str(width) + ' ' \
		+ str(height) + ' ' + str(bands) + '\n')
	for i in range(bands):
	    aux.write("ChanDefinition-" + str(i + 1) + ': ' + type + ' ' \
		    + str(image_offset[i]) + ' ' + str(pixel_offset[i]) \
		    + ' ' + str(line_offset[i]) + ' ' + swap + '\n')
	aux.close()
	aux = None

    def guess_cb(self, *args):
	"""Guess image geometry parameters."""

	def correlation(array1, array2):
	    """Calculate correlation coefficient of two arrays."""
	    n_elems = float(array1.shape[0])
	    M1 = Numeric.add.reduce(array1)
	    M2 = Numeric.add.reduce(array2)
	    D1 = Numeric.add.reduce(array1 * array1) - M1 * M1 / n_elems
	    D2 = Numeric.add.reduce(array2 * array2) - M2 * M2 / n_elems
            K = (Numeric.add.reduce(array1 * array2) - M1 * M2 / n_elems) / math.sqrt(D1 * D2)

	    return K

	header = long(self.header_entry.get_text())
	width = long(self.width_entry.get_text())
	height = long(self.height_entry.get_text())
	bands = long(self.bands_entry.get_text())
	gdaltype = \
	    gdal.GetDataTypeByName(self.type_list[self.type_menu.get_history()])
	numtype = gdalnumeric.GDALTypeCodeToNumericTypeCode(gdaltype)
	depth = gdal.GetDataTypeSize(gdaltype) / 8

	filename = self.open_entry.get_text()
        if os.path.isfile(filename) == 0:
            gvutils.error('Input file '+filename+' does not exist!')
            return

	filesize = os.stat(filename)[ST_SIZE]
	if filesize < header:
	    gvutils.error('Specified header size larger then file size!')
            return
	imagesize = (filesize - header) / bands / depth

	if width != 0 and height == 0:
	    height = imagesize / width
	elif width == 0 and height != 0:
	    width = imagesize / height
	else:
	    rawfile = open(filename, 'rb')
	    longt = 40.0	# maximum possible height/width ratio
	    cor_coef = 0.0
	    w = long(math.sqrt(imagesize / longt))
	    w_max = long(math.sqrt(imagesize * longt))
	    if (self.swap_menu.get_history() == 0 \
		and sys.byteorder == 'little') or \
	       (self.swap_menu.get_history() == 1 and sys.byteorder == 'big'):
		swap = 0
	    else:
		swap = 1
	    while w < w_max:
		if imagesize % w == 0:
		    scanlinesize = w * depth
		    h = imagesize / w
		    rawfile.seek(header + h / 2 * scanlinesize)
		    buf1 = rawfile.read(scanlinesize)
		    buf2 = rawfile.read(scanlinesize)
		    a1 = Numeric.fromstring(buf1, numtype)
		    a2 = Numeric.fromstring(buf2, numtype)
		    if swap:
			a1.byteswapped()
			a2.byteswapped()

		    try:
  		        tmp = correlation(a1.astype(Numeric.Float32), a2.astype(Numeric.Float32))
                    except:
                        # catch 0 division errors
                        gvutils.error('Unable to guess image geometry!')
                        return

		    if tmp > cor_coef:
			cor_coef = tmp
			width = w
			height = h
		w += 1

	self.width_entry.set_text(str(width))
	self.height_entry.set_text(str(height))

TOOL_LIST = ['OpenRaw']

