###############################################################################
# $Id: fft.py 6 2007-01-06 23:30:11Z mariobe $
#
# Project:  OpenEV Python tools
# Purpose:  Tool to calculate forward or inverse Fast Fourier Transform
# Author:   Andrey Kiselev, dron@remotesensing.org
#
###############################################################################
# Copyright (c) 2004, Andrey Kiselev <dron@remotesensing.org>
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
###############################################################################

import gtk
import gview
import gviewapp
import gdalnumeric
import gvutils
import FFT

def layer_is_raster(layer):
    """Return True if layer is raster and False otherwise"""
    try:
        layer.get_nodata(0)
        return True
    except:
        return False

class FFTTool(gviewapp.Tool_GViewApp):

    def __init__(self,app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.init_menu()

    def launch_dialog(self,*args):
	self.win = FFTDialog()
        self.win.show()

    def init_menu(self):
        self.menu_entries.set_entry("Filter/FFT",2,self.launch_dialog)

class FFTDialog(gtk.Window):
    def __init__(self,app=None):

        gtk.Window.__init__(self)
        self.set_title('Fast Fourier Transform')
        self.create_gui()
        self.show()

    def show(self):
        gtk.Window.show_all(self)

    def close(self, *args):
        self.hide()
        self.visibility_flag = 0
        return True

    def create_gui(self):
        box1 = gtk.VBox(spacing = 10)
	box1.set_border_width(10)
        self.add(box1)
        box1.show()

	self.switch_forward = gtk.RadioButton(None, "Forward")
	box1.pack_start(self.switch_forward)
	self.switch_forward.show()
	self.switch_inverse = gtk.RadioButton(self.switch_forward, "Inverse")
	box1.pack_start(self.switch_inverse)
	self.switch_inverse.show()

	separator = gtk.HSeparator()
	box1.pack_start(separator, expand=False)

	self.switch_new_view = gtk.CheckButton("Create new view")
	box1.pack_start(self.switch_new_view)
	self.switch_new_view.show()

	separator = gtk.HSeparator()
	box1.pack_start(separator, expand=False)

	box2 = gtk.HBox(spacing=10)
        box1.pack_start(box2, expand=False)
        box2.show()

        execute_btn = gtk.Button("Ok")
        execute_btn.connect("clicked", self.execute_cb)
	box2.pack_start(execute_btn)

        close_btn = gtk.Button("Cancel")
        close_btn.connect("clicked", self.close)
        box2.pack_start(close_btn)

    def execute_cb( self, *args ):
	layer = gview.app.sel_manager.get_active_layer()
	if not layer_is_raster(layer):
	    gvutils.error("Please select a raster layer.");
	    return
	ds = layer.get_parent().get_dataset()
	data = gdalnumeric.DatasetReadAsArray(ds)

	if self.switch_forward.get_active():
	    data_tr = FFT.fft2d(data)
	else:
	    data_tr = FFT.inverse_fft2d(data)

	array_name = gdalnumeric.GetArrayFilename(data_tr)
	if self.switch_new_view.get_active():
	    gview.app.new_view()
	gview.app.file_open_by_name(array_name)

TOOL_LIST = ['FFTTool']

