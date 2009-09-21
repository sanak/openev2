###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  GvRasterLayer Properties Dialog
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

import traceback
import gtk
import gvutils
import pgu
import pgucolorsel
from osgeo import gdal
from osgeo import osr
import gvhtml
from math import log10

from gvconst import *

prop_dialog_list = []

def LaunchRasterPropDialog(layer):
    # Check list to see if dialog exists - make it visible
    for test_dialog in prop_dialog_list:
        if test_dialog.layer == layer:
            test_dialog.update_gui()
            test_dialog.present()
            return test_dialog

    # Create new dialog if one doesn't exist already
    new_dialog = GvRasterPropDialog(layer)
    prop_dialog_list.append(new_dialog)
##    return new_dialog

class GvRasterSource(gtk.Frame):
    def __init__(self, name, layer, src_index):
        gtk.Frame.__init__(self, name)
        self.updating = False
        self.src_index = src_index

        # Eventually the following will have to be more sophisticated.
        if layer is not None:
            self.layer = layer
            self.display_change_id = layer.connect('display-change', self.gui_refresh)

        vbox = gtk.VBox(spacing=3)
        vbox.set_border_width(5)
        self.add(vbox)
        self.updating = True

        # ------ Band Selection -------
        band_list = ['constant']
        ds = self.layer.parent.get_dataset()
        band_count = ds.RasterCount

        for band in range(1, band_count+1) :
            desc = ds.GetRasterBand(band).GetDescription()
            if desc:
                band_desc = '%d: %s' % (band, desc)
            else:
                band_desc = str(band)
            band_list.append(band_desc)

        hbox = gtk.HBox(spacing=5)
        vbox.pack_start(hbox, expand=False)
        hbox.pack_start(pgu.Label('Band:'))
        self.band_combo = pgu.ComboText(band_list, action=self.set_band_cb)
        if ds.RasterCount > 20:
            self.band_combo.set_wrap_width(2)
        hbox.pack_start(self.band_combo, expand=False)

        # ------- Constant Value -----
        self.const_entry = gtk.Entry()
        self.const_entry.set_max_length(8)
        self.const_entry.set_size_request(80, -1)
        self.const_entry.connect('activate', self.const_cb)
        self.const_entry.connect('leave-notify-event', self.const_cb)
        hbox.pack_start(self.const_entry, expand=False)

        # ------ Establish scaling range ------

        smin = layer.min_get(src_index)
        smax = layer.max_get(src_index)
        delta = smax - smin
        smax = smax + delta * 0.25
        smin = smin - delta * 0.25
        datatype = self.layer.parent.get_band().DataType

        if self.layer.get_mode() == RLM_COMPLEX:
            smin = 0.0
        elif datatype == gdal.GDT_Byte:
            smin = 0
            smax = 255
        elif datatype in (gdal.GDT_UInt16, gdal.GDT_UInt32):
            smin = 0

        # Make sure slider still has reasonable step sizes
        # for cases where image has small, floating point
        # values.
        if delta > 10:
            new_inc = 1
        else:
            new_inc = delta/100.0

        # Calculate number of digits for slider. If datatype is integer,
        # this will be 0.  If floating point, this will be set depending 
        # on order of magnitude of delta:
        if datatype in (gdal.GDT_Byte, gdal.GDT_UInt16, gdal.GDT_Int16,
                        gdal.GDT_UInt32, gdal.GDT_Int32):
            sliderDigits = 0
        else:
            sliderDigits = max(0, 2 - int(log10(delta)))

        # ------ Scale Min -------
        hbox = gtk.HBox(spacing=5)
        self.min_hbox = hbox
        vbox.pack_start(hbox)
        hbox.pack_start(gtk.Label('Scale Min:'),expand=False)

        self.min_adjustment = gtk.Adjustment(layer.min_get(src_index),
                                smin, smax, new_inc, new_inc, new_inc)
        self.min_adjustment.connect('value-changed', self.adjustment_cb)
        self.min_slider = gtk.HScale(self.min_adjustment)
        self.min_slider.set_digits(sliderDigits)
        hbox.pack_start(self.min_slider)

        self.min_entry = gtk.Entry()
        self.min_entry.set_max_length(8)
        self.min_entry.set_size_request(80, -1)
        self.min_entry.connect('activate', self.entry_cb)
        self.min_entry.connect('leave-notify-event', self.entry_cb)
        hbox.pack_start(self.min_entry, expand=False)

        # ------ Scale Max -------
        hbox = gtk.HBox(spacing=5)
        self.max_hbox = hbox
        vbox.pack_start(hbox)
        hbox.pack_start(gtk.Label('Scale Max:'),expand=False)
        self.max_adjustment = gtk.Adjustment(layer.max_get(src_index),
                                smin, smax, new_inc, new_inc, new_inc)
        self.max_adjustment.connect('value-changed', self.adjustment_cb)
        self.max_slider = gtk.HScale(self.max_adjustment)
        self.max_slider.set_digits(sliderDigits)
        hbox.pack_start(self.max_slider)

        self.max_entry = gtk.Entry()
        self.max_entry.set_max_length(8)
        self.max_entry.set_size_request(80, -1)
        self.max_entry.connect('activate', self.entry_cb)
        self.max_entry.connect('leave-notify-event', self.entry_cb)
        hbox.pack_start(self.max_entry, expand=False)

        # ------ NODATA -------
        hbox = gtk.HBox(spacing=5)
        self.nodata_hbox = hbox
        vbox.pack_start(hbox)
        hbox.pack_start(gtk.Label('NODATA value:'), expand=False)

        self.nodata_entry = gtk.Entry()
#        self.nodata_entry.set_max_length(19)
        self.nodata_entry.connect('activate', self.entry_cb)
        self.nodata_entry.connect('leave-notify-event', self.entry_cb)
        hbox.pack_start(self.nodata_entry, expand=False)

        if src_index < 3 and ds.RasterCount > src_index:
            nodata = ds.GetRasterBand(src_index+1).GetNoDataValue()
            if nodata is not None:
                if not isinstance(nodata, complex):
                    nodata = complex(nodata, 0)
                self.layer.nodata_set(src_index, nodata.real, nodata.imag)

        self.updating = False

        self.connect('destroy', self.cleanup)

    def __del__(self):
        print 'Destroying GvRasterSource'

    def cleanup(self, *args):
        self.layer = None

    def gui_refresh(self, *args):
        if self.layer is None or self.updating:
            return

        self.updating = True
        idx = self.src_index

        mode = self.layer.get_mode()
        new_min = self.layer.min_get(idx)
        new_max = self.layer.max_get(idx)
        if mode == RLM_COMPLEX:
            new_min = max(0.0, new_min)

        if new_min < self.min_adjustment.lower:
            self.min_adjustment.value = new_min
            self.min_adjustment.lower = new_min
            self.min_adjustment.changed()                

        if new_max > self.max_adjustment.upper:
            self.max_adjustment.value = new_max
            self.max_adjustment.lower = new_max
            self.max_adjustment.changed()

        self.min_entry.set_text(str(new_min))
        self.max_entry.set_text(str(new_max))

        nodata = self.layer.nodata_get(idx)
        if isinstance(nodata, tuple):
            self.nodata_entry.set_text('%s+%sj' % nodata)
        else:
            self.nodata_entry.set_text(str(nodata))

        self.const_entry.set_text(str(self.layer.get_const_value(idx)))

        raster = self.layer.get_data(idx)
        if raster is None:
            self.const_entry.set_sensitive(True)
            self.min_hbox.set_sensitive(False)
            self.max_hbox.set_sensitive(False)
            self.nodata_hbox.set_sensitive(False)
            self.band_combo.set_active_text('constant')
        else:
            self.const_entry.set_sensitive(False)
            self.max_hbox.set_sensitive(True)
            if mode != RLM_COMPLEX and not band_is_complex(self.layer, idx):
                self.min_hbox.set_sensitive(True)
            else:
                self.min_hbox.set_sensitive(False)
            self.nodata_hbox.set_sensitive(True)

            # Set the band selector.
            band = raster.get_band()
            dataset = raster.get_dataset()
            for iband in range(dataset.RasterCount):
                test_band = dataset.GetRasterBand(iband+1)
                if test_band.this == band.this:
                    self.band_combo.set_active(iband+1)
                    break

        self.updating = False

    def set_band_cb(self, combo):
        if self.updating:
            return

        self.updating = True
        band_number = combo.get_active()
        idx = self.src_index
        if band_number == 0:
            self.layer.set_source(idx, None,
                                  self.layer.min_get(idx),
                                  self.layer.max_get(idx),
                                  self.layer.get_const_value(idx),
                                  self.layer.source_get_lut(idx),
                                  None)
        else:
            dataset = self.layer.parent.get_dataset()
            raster = self.layer.get_dataset_raster(band_number)
            if raster:
                self.layer.set_source(idx, raster,
                                    raster.min, raster.max,
                                    self.layer.get_const_value(idx),
                                    self.layer.source_get_lut(idx),
                          dataset.GetRasterBand(band_number).GetNoDataValue())

        self.enforce_greyscale()

        # enable alpha support if user modifies alpha band.
        if idx == 3:
            self.layer.blend_mode_set(RL_BLEND_FILTER)

        self.updating = False
        self.gui_refresh()

    def adjustment_cb(self, adjustment, *args):
        if self.updating:
            return

        value = adjustment.value
        if value < -1 or value > 1:
            value = int(value*10) / 10.0

        if adjustment == self.min_adjustment:
            self.layer.min_set(self.src_index, value)
        else:
            self.layer.max_set(self.src_index, value)

        self.enforce_greyscale()

    def entry_cb(self, entry, *args):
        if self.updating:
            return

        try:
            value = complex(entry.get_text())
        except:
##            traceback.print_exc()
            return

        if entry == self.min_entry:
            self.min_adjustment.value = value.real
        elif entry == self.max_entry:
            self.max_adjustment.value = value.real
        else:
            self.layer.nodata_set(self.src_index, value.real, value.imag)

        self.enforce_greyscale()

    def const_cb(self,entry,*args):
        if self.updating:
            return

        idx = self.src_index
        const = entry.get_text()
        if const.isdigit():
            self.layer.set_source(idx,
                                  self.layer.get_data(idx),
                                  self.layer.min_get(idx),
                                  self.layer.max_get(idx),
                                  int(const),
                                  self.layer.source_get_lut(idx),
                                  self.layer.nodata_get(idx))

        # enable alpha support if user modifies alpha band.
        if idx == 3:
            self.layer.blend_mode_set(RL_BLEND_FILTER)

        self.enforce_greyscale()

    def set_scaling(self, min_scale=None, max_scale=None):
        raster = self.layer.get_data(self.src_index)
        if min_scale is None:
            min_scale = raster.min
        if max_scale is None:
            max_scale = raster.max
        self.min_adjustment.value = min_scale
        self.max_adjustment.value = max_scale

    def enforce_greyscale(self):
        master_dialog = self.get_toplevel()
        if self.src_index < 3 and master_dialog.greyscale_is_set():
            master_dialog.enforce_greyscale(self.src_index)

class GvRasterPropDialog(gtk.Window):
    def __init__(self, layer):
        gtk.Window.__init__(self)
        self.set_border_width(3)
        if layer is not None:
            self.set_title(layer.get_name()+' Properties')
        else:
            self.set_title('Raster Properties')

        gvhtml.set_help_topic(self, "gvrasterpropdlg.html")
        self.layer = layer
        self.updating = False

        if self.layer is not None:
            self.display_change_id = self.layer.connect('display-change', self.refresh_cb)
            self.teardown_id = layer.connect('teardown', self.close)

        # create the general layer properties dialog
        self.create_notebook()

        self.create_pane1()

        self.updating = True
        self.create_sourcepane()
        self.updating = False

        self.create_openglprop()
        self.create_lutprop()
#        self.create_projprop()
        self.create_imageinfo()

        self.show_all()
        self.update_gui()

        for source in self.sources:
            source.gui_refresh()

    def __del__(self):
        print 'disconnect:', self.display_change_id
        self.layer.disconnect(self.display_change_id)

    def create_notebook(self):
        self.notebook = gtk.Notebook()
        self.add( self.notebook )
        self.connect('delete-event', self.close)

    def create_lutprop(self):

        self.lut_pane = gtk.VBox(spacing=10)
        self.lut_pane.set_border_width(10)
        self.notebook.append_page( self.lut_pane, gtk.Label('LUT'))

        self.lut_pixbuf = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB, True,
            8, 256, 256)
        self.lut_pixbuf.fill(0x00000000)
        self.lut_image = gtk.Image()
        self.lut_image.set_from_pixbuf(self.lut_pixbuf)
        #self.lut_preview = gtk.Preview()
        #self.lut_preview.size(256,32)

        self.lut_pane.pack_start(self.lut_image)

        self.complex_lut_om = \
            gvutils.GvOptionMenu(('Magnitude', 'Phase',
                                  'Magnitude & Phase', 'Real','Imaginary'),
                                 self.complex_lut_cb)
        self.lut_pane.pack_start(self.complex_lut_om, expand=False)

    def create_sourcepane(self):
        self.sources = []
        self.source_pane = gtk.VBox(spacing=5)
        self.source_pane.set_border_width(10)
        self.notebook.append_page(self.source_pane, gtk.Label('Raster Source'))

        if self.layer.get_mode() == RLM_RGBA:
            for iSource,channel in enumerate(['Red','Green','Blue','Alpha']):
                source = GvRasterSource(channel, self.layer, iSource)
                self.source_pane.pack_start(source, expand=False)
                self.sources.append(source)

            self.grey_toggle = gtk.CheckButton(label='Greyscale Lock')
            self.grey_toggle.connect('toggled', self.greyscale_cb)
            self.source_pane.pack_start(self.grey_toggle, expand=False)
            self.grey_toggle.set_active(self.greyscale_is_set())

            scaleHBox = gtk.HBox(spacing=10)
            self.scale_toggle = gtk.CheckButton(label="Scale Lock")
            self.scale_min_entry = gtk.Entry()
            self.scale_max_entry = gtk.Entry()
            self.scale_min_entry.connect("activate", self.activateLockEntry_cb)
            self.scale_max_entry.connect("activate", self.activateLockEntry_cb)
            self.scale_min_entry.set_size_request(50, -1)
            self.scale_max_entry.set_size_request(50, -1)

            scaleHBox.pack_start(self.scale_toggle) 
            scaleHBox.pack_start(self.scale_min_entry)
            scaleHBox.pack_start(self.scale_max_entry)

            self.scale_toggle.connect("toggled", self.scalelock_cb)
            self.source_pane.pack_start(scaleHBox, expand=False)
            self.scale_toggle.set_active(self.scalelock_is_set())
        else:
            source = GvRasterSource('Raster', self.layer, 0)
            self.source_pane.pack_start(source, expand=False)
            self.sources.append(source)

    def create_pane1(self):
        # Setup General Properties Tab
        self.pane1 = gtk.VBox(spacing=10)
        self.pane1.set_border_width(10)
        self.notebook.append_page( self.pane1, gtk.Label('General'))

        # Setup layer name entry box.
        box = gtk.HBox(spacing=5)
        self.pane1.pack_start(box, expand=False)
        label = gtk.Label('Layer:' )
        box.pack_start(label,expand=False)
        self.layer_name = gtk.Entry()
        self.layer_name.connect('changed', self.name_cb)
        box.pack_start(self.layer_name)

        # Setup Visibility radio buttons.
        vis_box = gtk.HBox(spacing=5)
        self.pane1.pack_start(vis_box, expand=False)
        vis_box.pack_start(gtk.Label('Visibility:'),expand=False)
        self.vis_yes = gtk.RadioButton(label='yes')
        self.vis_yes.connect('toggled', self.visibility_cb)
        vis_box.pack_start(self.vis_yes,expand=False)
        self.vis_no = gtk.RadioButton(label='no',group=self.vis_yes)
        self.vis_no.connect('toggled', self.visibility_cb)
        vis_box.pack_start(self.vis_no,expand=False)

        # Setup Editability radio buttons.
        edit_box = gtk.HBox(spacing=5)
        self.pane1.pack_start(edit_box, expand=False)
        edit_box.pack_start(gtk.Label('Editable:'),expand=False)
        self.edit_yes = gtk.RadioButton(label='yes')
        self.edit_yes.connect('toggled', self.edit_cb)
        edit_box.pack_start(self.edit_yes,expand=False)
        self.edit_no = gtk.RadioButton(label='no',group=self.edit_yes)
        self.edit_no.connect('toggled', self.edit_cb)
        edit_box.pack_start(self.edit_no,expand=False)

    def create_openglprop(self):
        oglpane = gtk.VBox(spacing=10)
        oglpane.set_border_width(10)
        self.notebook.append_page(oglpane, gtk.Label('Draw Style'))

        # Create Modulation Color
        box = gtk.HBox(spacing=5)
        oglpane.pack_start(box, expand=False)
        box.pack_start(gtk.Label('Modulation Color:'),expand=False)
        self.mod_color = pgucolorsel.ColorControl('Modulation Color',
                                                  self.color_cb,None)
        box.pack_start(self.mod_color)

        # Create Interpolation Control
        box = gtk.HBox(spacing=5)
        oglpane.pack_start(box, expand=False)
        box.pack_start(gtk.Label('Subpixel Interpolation:'),expand=False)
        self.interp_om = gvutils.GvOptionMenu(('Linear','Off (Nearest)'), \
                                              self.set_interp_cb)
        box.pack_start(self.interp_om,expand=False)

    def create_projparms(self):
        """Create projection parameters controls"""
        
        self.parm_dict = {}
        if self.proj_table is not None:
            self.proj_table.destroy()
            num = len(self.proj_parms[self.proj_index])
            if num == 0:
                return
        self.proj_table = gtk.Table(2, num)
        self.proj_table.set_border_width(5)
        self.proj_table.set_row_spacings(5)
        self.proj_table.set_col_spacings(5)
        self.proj_table.show()
        row = 0
        for i in self.proj_parms[self.proj_index]:
            parm_label = gtk.Label(i[1])
            parm_label.set_alignment(0, 0.5)
            self.proj_table.attach(parm_label, 0, 1, row, row + 1)
            parm_label.show()
            parm_value = self.sr.GetProjParm(i[0])
            if parm_value is None:
                parm_value = str(i[3])
            parm_entry = gtk.Entry()
            parm_entry.set_text(str(parm_value))
            self.parm_dict[i[0]] = parm_value
            parm_entry.set_editable(True)
            parm_entry.connect('changed', self.parm_entry_cb, i[0])
            self.proj_table.attach(parm_entry, 1, 2, row, row + 1)
            parm_entry.show()
            row += 1
        
        self.proj_vbox.pack_end(self.proj_table, expand=False)

    def create_projprop(self):
        projpane = gtk.VBox(spacing=10)
        projpane.set_border_width(10)
        self.notebook.append_page(projpane, gtk.Label('Coordinate System'))

        self.projprop_vbox = gtk.VBox(spacing=5)

        # Projection frame
        proj_frame = gtk.Frame('Projection')
        proj_frame.show()
        projpane.pack_start(proj_frame, expand=False)
        self.proj_vbox = gtk.VBox(spacing=5)

        # Fetch projection record
        self.proj_full = ''
        proj_name = ''
        projection = self.layer.get_projection()

        self.sr = None
        if projection is not None and len(projection) > 0:
            self.sr = osr.SpatialReference()
            if self.sr.ImportFromWkt( projection ) == 0:
                self.proj_full = self.sr.ExportToPrettyWkt(1)
        if self.proj_full is None:
            self.proj_full = ''
        proj_name = self.sr.GetAttrValue("PROJECTION")
        if proj_name is None:
            proj_name = ''

        # Create projection switch
        proj_hbox = gtk.HBox(spacing=5)
        proj_hbox.pack_start(gtk.Label('Projection Name:'), \
        expand=False, padding=5)
        proj_methods = osr.GetProjectionMethods()
        self.projs = map(lambda x: x.__getitem__(0), proj_methods)
        self.projs.insert(0, '')
        proj_names = map(lambda x: x.__getitem__(1), proj_methods)
        proj_names.insert(0, 'None')
        self.proj_parms = map(lambda x: x.__getitem__(2), proj_methods)
        self.proj_parms.insert(0, [])
        self.proj_index = self.projs.index(proj_name)

        self.proj_table = None
        self.proj_om = gvutils.GvOptionMenu(proj_names, self.set_proj_cb)
        self.create_projparms()
        self.proj_om.set_history(self.proj_index)
        proj_hbox.pack_start(self.proj_om, padding=5)
        self.proj_vbox.pack_start(proj_hbox, expand=False)

        proj_frame.add(self.proj_vbox)

        # Datum frame
        datum_frame = gtk.Frame('Datum')
        datum_frame.show()
        projpane.pack_start(datum_frame, expand=False)
        datum_hbox = gtk.HBox(spacing=5)
        datum_hbox.pack_start(gtk.Label('Datum Name:'), expand=False, padding=5)

        try:
            self.datum_name = self.sr.GetAttrValue("DATUM")
        except:
            traceback.print_exc()
            self.datum_name = None

        self.datum_names = {None:"None", osr.SRS_DN_NAD27:"NAD27", \
        osr.SRS_DN_NAD83:"NAD83", osr.SRS_DN_WGS72:"WGS72", \
        osr.SRS_DN_WGS84:"WGS84"}
        try:
            self.datum_index = self.datum_names.keys().index(self.datum_name)
        except ValueError:
            self.datum_index = self.datum_names.keys().index(None)
        self.datum_om = gvutils.GvOptionMenu(self.datum_names.values(), \
            self.set_datum_cb)
        self.datum_om.set_history(self.datum_index)
        datum_hbox.pack_start(self.datum_om, expand=False, padding=5)

        datum_frame.add(datum_hbox)

        # Units frame
        units_frame = gtk.Frame('Units')
        #units_frame.show()
        #projpane.pack_start(units_frame, expand=False)
        units_hbox = gtk.HBox(spacing=5)
        units_hbox.pack_start(gtk.Label('Units:'), expand=False, padding=5)

        units_frame.add(units_hbox)

        # WKT frame
        proj_text_frame = gtk.Frame('Well Known Text')
        proj_text_frame.show()
        projpane.pack_end(proj_text_frame, expand=True)

        self.proj_text_buff = gtk.TextBuffer()
        self.proj_text_buff.set_text(self.proj_full)
        self.proj_text_view = gtk.TextView(self.proj_text_buff)
        self.proj_text_view.set_wrap_mode(gtk.WRAP_CHAR)
        self.proj_text_view.set_editable(False)
        self.proj_text_view.show()

        # GTK2 Port...
        #self.proj_text = gtk.Text()
        #self.proj_text.set_line_wrap(True)
        #self.proj_text.set_word_wrap(False)
        #self.proj_text.set_editable(False)
        #self.proj_text.show()
        #self.proj_text.insert_defaults(self.proj_full)

        proj_scrollwin = gtk.ScrolledWindow()
        proj_scrollwin.set_size_request(0, 300)
        proj_scrollwin.add(self.proj_text_view)
        proj_text_frame.add(proj_scrollwin)

    def create_imageinfo(self):
        iipane = gtk.VBox(spacing=10)
        iipane.set_border_width(10)
        self.notebook.append_page( iipane, gtk.Label('Image Info') )

        self.ii_text_buff = gtk.TextBuffer()
        self.ii_text_view = gtk.TextView(self.ii_text_buff)
        self.ii_text_view.set_wrap_mode(gtk.WRAP_NONE)
        self.ii_text_view.set_editable(False)
        self.ii_text_view.show()

        # GTK2 Port...
        #self.ii_text = gtk.Text()
        #self.ii_text.set_line_wrap(False)
        #self.ii_text.set_word_wrap(False)
        #self.ii_text.set_editable(False)
        #self.ii_text.show()

        self.ii_scrollwin = gtk.ScrolledWindow()
        self.ii_scrollwin.add( self.ii_text_view)
        iipane.pack_start(self.ii_scrollwin,expand=True)

        # Now create and assign the text contents.
        gdal_ds = self.layer.parent.get_dataset()

        text = ''

        text = text + 'Filename: ' + gdal_ds.GetDescription() + '\n'

        text = text + 'Size: ' + str(gdal_ds.RasterXSize) + 'P x '
        text = text +            str(gdal_ds.RasterYSize) + 'L x '
        text = text +            str(gdal_ds.RasterCount) + 'Bands\n'

        driver = gdal_ds.GetDriver()
        text = text + 'Driver: ' + driver.LongName + '\n'

        transform = gdal_ds.GetGeoTransform()
        if transform[2] == 0.0 and transform[4] == 0.0:
            text = text + 'Origin: ' + str(transform[0])               \
                               + ' ' + str(transform[3]) + '\n'
            text = text + 'Pixel Size: ' + str(transform[1])           \
                                 + ' x ' + str(transform[5]) + '\n'

        projection = gdal_ds.GetProjection()
        if ((projection is None) or (len(projection) == 0)):
            projection = gdal_ds.GetGCPProjection()

        if projection is None:
            projection=""

        if len(projection) > 0:
            sr = osr.SpatialReference()
            if sr.ImportFromWkt( projection ) == 0:
                projection = sr.ExportToPrettyWkt(1)

        text = text + 'Projection:\n'
        text = text + projection + '\n'

        metadata = gdal_ds.GetMetadata()
        if len(metadata) > 0:
            text = text + 'Metadata:\n'
            for item in metadata.items():
                text = text + '    '+item[0]+': '+item[1]+'\n'

        for band_index in range(gdal_ds.RasterCount):
            band = gdal_ds.GetRasterBand(band_index+1)

            text = text + 'Band %2d: Type=' % (band_index+1)
            text = text + gdal.GetDataTypeName(band.DataType)
            if len(band.GetDescription()) > 0:
                text = text + ' - ' + band.GetDescription()
            text = text + '\n'

        #self.ii_text.insert_defaults(text)
        self.ii_text_buff.set_text(text)

    # Initialize GUI state from underlying object state.
    def update_gui(self):
        if self.layer is None or self.updating == True:
            return

        self.updating = True

        # Layer name.
        self.layer_name.set_text( self.layer.get_name() )

        # Visibility radio buttons
        self.vis_yes.set_active( self.layer.is_visible() )
        self.vis_no.set_active( not self.layer.is_visible() )

        # Editability radio buttons
        self.edit_yes.set_active( not self.layer.is_read_only() )
        self.edit_no.set_active( self.layer.is_read_only() )

        # modulation color
        tflag, tcolor = self.layer.texture_mode_get()
        if tflag == 0:
            tcolor = (1,1,1,1)
        self.mod_color.set_color(tcolor)

        # Interpolation Mode
        zmin, zmax = self.layer.zoom_get()
        if zmax == RL_FILTER_BILINEAR:
            self.interp_om.set_history(0)
        else:
            self.interp_om.set_history(1)

        self.updating = False

        # LUT
        if self.layer.get_mode() != RLM_RGBA:
            lut_tuple = self.layer.lut_get()
        else:
            if self.greyscale_is_set() and band_is_complex(self.layer,0):
                # Only show complex lut frame when bands are locked,
                # because that's the only time it makes sense
                # to apply non-magnitude luts.
                lut_tuple = self.layer.lut_get(rgba_complex=1)
                # Don't let the user modulate alpha by a band, because
                # results won't make sense unless the LUT is magnitude
                # (when alpha is not constant, it uses the red component
                # of the lookup table for the modulating band- this will
                # give the expected behaviour if the LUT is magnitude,
                # but will give non-sensible results if the LUT
                # is phase). 
                self.sources[3].hide()
            else:
                lut_tuple = self.layer.lut_get()
                self.sources[3].show()

        if lut_tuple is None:
            self.lut_pane.hide()
        elif lut_tuple[2] == 1:
            self.lut_pixbuf = gtk.gdk.pixbuf_new_from_data(lut_tuple[0],
                gtk.gdk.COLORSPACE_RGB, True, 8, 256, 32, 1024)
            self.lut_image.set_from_pixbuf(self.lut_pixbuf)
            self.lut_pane.show()
        else:
            self.lut_pixbuf = gtk.gdk.pixbuf_new_from_data(lut_tuple[0],
                gtk.gdk.COLORSPACE_RGB, True, 8, 256, 256, 1024)
            self.lut_image.set_from_pixbuf(self.lut_pixbuf)
            self.lut_pane.show()
            self.complex_lut_om.show()

    def name_cb(self, *args):
        if self.layer_name.get_text() != self.layer.get_name():
            self.layer.set_name( self.layer_name.get_text() )

    # Visibility changing
    def visibility_cb(self, widget):
        self.layer.set_visible( self.vis_yes.get_active())

    # Readonly changing
    def edit_cb(self, widget):
        self.layer.set_read_only( self.edit_no.get_active() )

    def scalelock_is_set(self): 
        return self.layer.get_property('_scale_lock') == 'locked'

    def activateLockEntry_cb(self, *args):
        if not self.scale_toggle.get_active():
            self.scale_toggle.set_active(True)
        else:
            self.scalelock_cb(self.scale_toggle)

    def scalelock_cb(self, toggle):
        locked = toggle.get_active()
        self.layer.set_property('_scale_lock', ('unlocked','locked')[locked])
        min_scale, max_scale = None, None

        if locked:
            min_text, max_text = self.scale_min_entry.get_text(), self.scale_max_entry.get_text()
            if min_text and max_text:
                min_scale, max_scale = float(min_text), float(max_text)
            else: 
                min_scale, max_scale = self.layer.min_get(0), self.layer.max_get(0)
                self.scale_min_entry.set_text(str(min_scale))
                self.scale_max_entry.set_text(str(max_scale))

            self.layer.set_property('_scale_limits', '%f %f' % (min_scale, max_scale))

        for source in self.sources[:-1]:
            source.set_scaling(min_scale, max_scale)

    def greyscale_is_set(self):
        return self.layer.get_property('_greyscale_lock') == 'locked'

    def greyscale_cb(self, toggle):
        locked = toggle.get_active()
        self.layer.set_property('_greyscale_lock', ('unlocked','locked')[locked])

        if locked:
            if self.layer.get_mode() == RLM_RGBA:
                self.complex_lut_om.set_history(0)
                self.layer.complex_lut('magnitude')

            self.enforce_greyscale(0)
        else:
            # MB: we should do something here but I am not sure what...
            pass

        # show/hide lut as necessary    
        self.update_gui()

    def enforce_greyscale(self, isrc):
        # MB: this needs to be redone...
        if isrc != 0:
            self.layer.set_source(0, self.layer.get_data(isrc),
                                  self.layer.min_get(isrc),
                                  self.layer.max_get(isrc),
                                  self.layer.get_const_value(isrc),
                                  self.layer.get_source_lut(isrc),
                                  self.layer.nodata_get(isrc))

        if isrc != 1:
            self.layer.set_source(1, self.layer.get_data(isrc),
                                  self.layer.min_get(isrc),
                                  self.layer.max_get(isrc),
                                  self.layer.get_const_value(isrc),
                                  self.layer.get_source_lut(isrc),
                                  self.layer.nodata_get(isrc))

        if isrc != 2:
            self.layer.set_source(2, self.layer.get_data(isrc),
                                  self.layer.min_get(isrc),
                                  self.layer.max_get(isrc),
                                  self.layer.get_const_value(isrc),
                                  self.layer.get_source_lut(isrc),
                                  self.layer.nodata_get(isrc))

        # In multi-band complex case, reset alpha band to a constant
        if band_is_complex(self.layer,0):
            self.layer.set_source(3, None,
                                  self.layer.min_get(3),
                                  self.layer.max_get(3),
                                  self.layer.get_const_value(3),
                                  self.layer.get_source_lut(3),
                                  None)

    def complex_lut_cb(self, *args):
        # Magnitude
        if self.complex_lut_om.get_history() == 0:
            method = 'magnitude'

        # Phase
        elif self.complex_lut_om.get_history() == 1:
            method = 'phase'

        # Magnitude and Phase
        elif self.complex_lut_om.get_history() == 2:
            method = 'magphase'

        # Real
        elif self.complex_lut_om.get_history() == 3:
            method = 'real'

        # Imaginary
        elif self.complex_lut_om.get_history() == 4:
            method = 'imaginary'

        self.layer.complex_lut( method )

        self.update_gui()

    # Set modulation color
    def color_cb(self, color, type):
        #if color[0] == 1.0 and color[1] == 1.0 \
        #   and color[2] == 1.0 and color[3] == 1.0:
        #    pass
        #else:
        if self.updating:
           return
        if color[3] != 1.0:
            self.layer.blend_mode_set(RL_BLEND_FILTER)
            self.layer.texture_mode_set(1, color)

    def set_interp_cb(self,*args):
        if self.interp_om.get_history() == 0:
            self.layer.zoom_set(RL_FILTER_BILINEAR,RL_FILTER_BILINEAR)
        else:
            self.layer.zoom_set(RL_FILTER_NEAREST,RL_FILTER_NEAREST)

    def update_proj_text(self):
        """Update text control showing projection information"""
        self.proj_full = self.sr.ExportToPrettyWkt( simplify = 1 )
        if self.proj_full is None:
            self.proj_full = ''

        self.proj_text_buff.set_text(self.proj_full)

    def parm_entry_cb(self, entry, parm):
        """Set projection parameters"""
        self.parm_dict[parm] = float(entry.get_text())
        self.sr.SetNormProjParm(parm, self.parm_dict[parm])

        self.update_proj_text()

    def set_proj_cb(self,*args):
        """Set projection"""
        if self.proj_index != self.proj_om.get_history():
            self.proj_index = self.proj_om.get_history()
            self.sr = osr.SpatialReference()
            self.sr.SetWellKnownGeogCS(self.datum_names[self.datum_name])
            self.sr.SetProjection(self.projs[self.proj_index])
            for i in self.proj_parms[self.proj_index]:
                try:
                    self.sr.SetNormProjParm(i[0], self.parm_dict[i[0]])
                except KeyError:
                    self.sr.SetNormProjParm(i[0], i[3])
            self.layer.set_projection(self.sr.ExportToWkt())
            self.create_projparms()

	    self.update_proj_text()

    def set_datum_cb(self, *args):
        """Set datum"""
        if self.datum_index != self.datum_om.get_history():
            self.datum_index = self.datum_om.get_history()
            self.datum_name = self.datum_names.keys()[self.datum_index]
            self.sr.SetWellKnownGeogCS(self.datum_names[self.datum_name])
            self.update_proj_text()

    # Dialog closed, remove references to python object
    def close(self, *args):
        prop_dialog_list.remove(self)
        self.layer.disconnect(self.display_change_id)
        self.layer.disconnect(self.teardown_id)

        self.sources = None
        self.layer = None
        self.destroy()

    # Force GUI Refresh
    def refresh_cb(self, widget, *args):
        self.update_gui()

def band_is_complex(layer,src_index):
    """ Returns 1 if src_index'th band of layer is present
        and complex; 0 otherwise.
    """

    try:
        srctype = layer.get_data(src_index).get_band().DataType
        if srctype in (gdal.GDT_CInt16, gdal.GDT_CInt32, gdal.GDT_CFloat32,
                        gdal.GDT_CFloat64):
            return 1

        return 0
    except:
        traceback.print_exc()
        return 0

if __name__ == '__main__':
    dialog = GvRasterPropDialog(None)
    dialog.connect('delete-event', gtk.main_quit)

    gtk.main()
