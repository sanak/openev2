###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Raster classification dialogs
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

import gview
import gvutils
import gtk
from gvsignaler import Signaler
from pgucolor import ColorSwatch, ColorButton, ColorDialog, ColorRamp
from gvclassification import *
from pguentry import pguEntry
import gvogrfsgui
from pgumenu import *
import gdal #for CPLDebug

from string import *

"""gvclassifydlg.py module contains two classes related to raster classification.

GvClassificationDlg is the main gui for modifying raster classifications.
GvReclassifyDlg is a supplementary dialog for changing the number of classes in
a classification scheme.

This module also contains a number of supplementary utilties for working with ramps.


TODO:

 x find out how to modify the background color of a widget and use it in the column
   headers

 x determine when range values are valid/invalid.  When invalid, set bg color of cell
   to red or something.

 x provide more buttons for doin' stuff.
"""

MIN_COLOR=0
LOW_COLOR = 21845
HI_COLOR = 65535
MAX_COLOR = 65535

def set_widget_background(widget, rgba_color):
    style = widget.get_style().copy()
    for i in range(1):
        style.base[i] = gdk_from_rgba(widget, rgba_color)
        style.bg[i] = style.base[i]
    widget.set_style(style)

def gdkcolor(widget, red=0, green=0, blue=0):
    return widget.get_colormap().alloc(int(red), int(green), int(blue))
    #return gtk.GdkColor(red, green, blue, 1)

def gdk_from_rgba(widget, rgba_color):
    r = rgba_color[0] * MAX_COLOR
    g = rgba_color[1] * MAX_COLOR
    b = rgba_color[2] * MAX_COLOR
    return gdkcolor(widget, r, g, b)

def rgba_from_gdk(gdk_color):
    r = float(gdk_color.red) / MAX_COLOR
    g = float(gdk_color.green) / MAX_COLOR
    b = float(gdk_color.blue) / MAX_COLOR
    return (r, g, b, 1.0)

def rgba_tuple(red=0.0, green=0.0, blue=0.0, alpha=1.0):
    return (red, green, blue, alpha)

def load_ramps():
    """reads in all the ramp files in the ramps directory and creates ramps for them"""
    import os
    import os.path
    ramps = []
    ramp_dir = gview.get_preference('ramp_directory')
    if ramp_dir is None:
        ramp_dir = os.path.join(gview.home_dir,'ramps')
    if os.path.isdir(ramp_dir):
        files = os.listdir(ramp_dir)
        for file in files:
            ramp_file = os.path.join(ramp_dir, file)
            if os.path.isfile(ramp_file):
                ramp = ColorRamp()
                try:
                    ramp.deserialize(ramp_file)
                    ramps.append(ramp)
                except:
                    print 'invalid ramp file %s' % ramp_file
    else:
        print 'no default ramp files in ', ramp_dir
    return ramps

def load_ramp_config_file():
    """
    Reads in ramp files specified in the ramp config file
    in the ramps directory and creates ramps for them

    This allows for ordering of the ramps in the config
    file and for specifying separators
    """
    import os
    import os.path
    import string
    ramps = []
    ramp_dir = gview.get_preference('ramp_directory')
    if ramp_dir is None:
        ramp_dir = os.path.join(gview.home_dir,'ramps')
    if os.path.isdir(ramp_dir):
        config_path = os.path.join(ramp_dir, 'ramps.cfg')
        if os.path.isfile(config_path):
            #load config file and parse ramps ...
            config = open(config_path)
            lines = config.readlines()
            for line in lines:
                ramp_file = string.strip(line)
                if ramp_file == '<separator>':
                    ramps.append(gtk.HSeparator())
                else:
                    ramp_file = os.path.join(ramp_dir, ramp_file)
                    if os.path.isfile(ramp_file):
                        ramp = ColorRamp()
                        try:
                            ramp.deserialize(ramp_file)
                            ramps.append(ramp)
                            ramp.show()
                        except:
                            print 'invalid ramp file %s' % ramp_file
                    else:
                        print 'not a file (%s)' % ramp_file
        else:
            print 'no ramps.cfg file, loading ramps directly'
            return load_ramps()
    else:
        print 'no default ramp files in ', ramp_dir
    return ramps

class GvClassificationDlg(gtk.Window, Signaler):
    """A dialog for modifying the classification scheme of a GvLayer."""

    def __init__(self, classification):
        """Initialize a GvClassificationDlg on a particular GvLayer"""
        gtk.Window.__init__(self)
        self.set_title('Layer Classification')
        self.set_size_request(-1, 400)
        self.connect('delete-event', self.close)
        self.set_border_width(5)
        self.color_buttons = []
        self.sym_menus = []
        self.scale_spinners = []
        self.view_mgr = None
        self.ranges = []
        self.labels = []
        self.reclassdlg = None
        self.updating = False
        items = load_ramp_config_file()
        self.ramp = None
        if classification is None:
            self.classification = GvClassification()
        elif issubclass(classification.__class__, GvClassification):
            self.classification = classification
        else:
            raise TypeError, 'GvClassificationDlg now requires a \
                              GvClassification instance'
        if self.classification.count <= 0:
            self.ramp = items[0]
            self.classification.prepare_default()
        #d = self.classification.serialize()
        #main vertical box
        vbox = gtk.VBox(spacing=3)

        save_box = gtk.HButtonBox()
        btn_save = gtk.Button('Save ...')
        btn_save.connect('clicked', self.save_cb)
        btn_load = gtk.Button('Load ...')
        btn_load.connect('clicked', self.load_cb)
        save_box.pack_start(btn_load)
        save_box.pack_start(btn_save)

        self.property_list = gtk.combo_box_entry_new_text()
        #try:
        #    import pgucombo
        #    self.property_list = pgucombo.pguCombo()
        #except ImportError:
        #    self.property_list = gtk.Combo()

        self.property_list.child.connect('changed',self.property_select_cb)
        self.update_property_list()

        save_box.pack_start(self.property_list)
        vbox.pack_start(save_box, expand=False)

        #classification frame
        class_frame = gtk.Frame()
        frame_box = gtk.VBox(spacing=3)

        title_box = gtk.HBox()
        title_lbl = gtk.Label('Legend Title: ')
        self.title_txt = gtk.Entry()
        self.title_txt.set_text(self.classification.get_title())
        self.title_txt.connect('changed', self.title_changed_cb)

        title_box.pack_start(title_lbl, expand=False)
        title_box.pack_start(self.title_txt)

        frame_box.pack_start(title_box, expand=False)
        frame_box.set_border_width(5)

        #classification list
        class_box = gtk.ScrolledWindow()
        self.class_list = gtk.List()
        self.class_list.connect( 'select-child', self.list_selected )
        class_box.add_with_viewport(self.class_list)
        frame_box.pack_start(class_box)
        self.reset_classification_list()

        class_frame.add(frame_box)
        vbox.pack_start(class_frame)

        ar_box = gtk.HButtonBox()
        add_btn = gtk.Button('Add class')
        add_btn.connect('clicked', self.add_class_cb)
        classify_btn = gtk.Button('reclassify ...')
        classify_btn.connect('clicked', self.reclassify_cb)
        reset_btn = gtk.Button('Revert')
        reset_btn.connect('clicked', self.reset_cb)
        ar_box.pack_start(add_btn)
        ar_box.pack_start(classify_btn)
        ar_box.pack_start(reset_btn)
        vbox.pack_start(ar_box, expand=False)

        #Color Ramp choices
        ramp_table=gtk.Table(rows=2, columns=2)
        ramp_table.show()
        ramp_lbl = gtk.Label('Color Ramps: ')
        ramp_lbl.show()
        ramp_table.attach(ramp_lbl, 0, 1, 0, 1)
        ramp_opt = gtk.OptionMenu()
        ramp_opt.show()
        self.ramp_menu = gtk.Menu()
        self.ramp_menu.show()
        ramp_item=gtk.MenuItem()
        ramp_item.add(gtk.HSeparator())
        ramp_item.set_sensitive(False)
        self.ramp_menu.append(ramp_item)
        for n in items:
            ramp_item = gtk.MenuItem()
            ramp_item.add(n)
            ramp_item.show_all()
            if issubclass(n.__class__, ColorRamp):
                ramp_item.connect('activate', self.ramp_cb, n)
            else:
                ramp_item.set_sensitive(False)
            self.ramp_menu.append(ramp_item)
        ramp_opt.set_menu(self.ramp_menu)
        ramp_opt.show()
        ramp_opt.set_history(0)
        ramp_table.attach(ramp_opt, 1, 2, 0, 1)
        ramp_table.show_all()
        vbox.pack_start(ramp_table, expand=False)
        #buttons
        button_box = gtk.HButtonBox()
        #button_box.set_layout_default(gtk.BUTTONBOX_START)
        self.ok_button = gtk.Button('OK')
        self.ok_button.connect('clicked', self.ok_cb)
        self.apply_button = gtk.Button('Apply')
        self.apply_button.connect('clicked', self.apply_cb)
        self.cancel_button = gtk.Button('Cancel')
        self.cancel_button.connect('clicked', self.cancel_cb)
        button_box.pack_start(self.ok_button, expand=False)
        button_box.pack_start(self.apply_button, expand=False)
        button_box.pack_start(self.cancel_button, expand=False)
        vbox.pack_start(button_box, expand=False)
        vbox.show_all()
        self.add(vbox)

        #make ok_button a default button ? why isn't it working ?
        self.ok_button.set_flags(gtk.CAN_DEFAULT)
        self.ok_button.grab_default()
        self.publish('classification-changed')

        self.update_property_list()

    def close(self, *args):
        """close and destroy this dialog"""
        self.hide()
        self.destroy()
        if self.reclassdlg is not None:
            self.reclassdlg.destroy()
        return True

    def ok_cb(self, *args):
        """Close the dialog and notify listeners and the raster that the
        classification has changed"""
        #allowing the raster to be rescale screws things up!
        self.classification.update_all_layers(rescale=1)
        Signaler.notify(self, 'classification-changed')
        return self.close()

    def apply_cb(self, *args):
        """apply the current classification"""
        Signaler.notify(self, 'classification-changed')
        self.classification.update_all_layers(rescale=1)

    def cancel_cb(self, *args):
        """close the classification dialog without doing anything
        about the classification"""
        return self.close()

    def list_selected( self, *args ):
        self.class_list.unselect_all()

    def add_class_cb(self, *args):
        """add a single class to the classification.  Add it at the end
        with the same value and color as the upper range value of the
        last class.  If there are no classes, use the entire range.
        """
        #first create the new class
        cls = self.classification
        if len(self.color_buttons) > 0:
            color = self.color_buttons[len(self.color_buttons)-1].get_d()
            print color
            rng = cls.get_range(cls.count-1)
            rng = (rng[1], rng[1])
            symbol = cls.get_symbol( cls.count - 1 )
            scale = cls.get_scale( cls.count - 1 )
        else:
            color = ( 0.0, 0.0, 0.0, 1.0 )
            rng = cls.collect_range()
            #for point layers only
            if cls.layers[0].get_parent()[0].get_shape_type() == gview.GVSHAPE_POINT:
                symbol = '"ogr-sym-0"'
            else:
                symbol = None
            scale = 1.0
        n = cls.set_class(color = color, 
                          range_min = rng[0], 
                          range_max = rng[1],
                          symbol = symbol,
                          symbol_scale = scale)
        self.insert_class( n )                          

    def insert_class( self, class_id ):
        """Create gui elements for the class_id and insert them
        into the gui
        """
        cls = self.classification
        self.color_buttons.insert(class_id, ColorButton(cls.get_color(class_id), 
            colormap=self.get_colormap()))
        self.color_buttons[class_id].connect('color-set', 
                                             self.color_button_cb, class_id)
        symbol = cls.get_symbol( class_id )
        if symbol is not None:
            sym_menu = pguMenuFactory(MENU_FACTORY_OPTION_MENU)
            entries = []
            for i in range(len(gvogrfsgui.ogrfs_symbol_names)):
                sym_name = gvogrfsgui.ogrfs_symbol_names[i]
                sym_img = gvogrfsgui.ogrfs_symbols[sym_name][1]
                a = '<image:' + sym_img + '>'
                entries.append((a, None, self.symbol_change, class_id, 
                                gvogrfsgui.ogrfs_symbols[sym_name][0]))
            sym_menu.add_entries(entries)
            sym_menu.set_size_request(70, 30)
            sym_menu.set_history(int(symbol[8:]))
            self.sym_menus.insert( class_id, sym_menu )

            scale = cls.get_scale( class_id )
            adj = gtk.Adjustment( value=scale, lower=0.0, upper=100.0, 
                                     step_incr=0.11, page_incr=1.0, page_size=1.0 )
            scale_spin = gtk.SpinButton(adj)
            scale_spin.set_editable( True )
            adj.connect( 'value-changed', self.scale_change, class_id )
            self.scale_spinners.insert( class_id, scale_spin )
        else:
            self.sym_menus.insert( class_id, None )
            self.scale_spinners.insert( class_id, None )

        self.ranges.insert(class_id, pguEntry())
        rng = cls.get_range(class_id)
        rng_txt = str( rng[0] )
        if rng[1] != '' and cls.get_type() != CLASSIFY_DISCRETE:
            rng_txt = rng_txt + "-" + str( rng[1] )
        self.ranges[class_id].set_text(rng_txt)
        self.ranges[class_id].connect('changed', self.range_changed_cb, class_id)
        self.labels.insert(class_id, pguEntry())
        self.labels[class_id].set_text(cls.get_name(class_id))
        self.labels[class_id].connect('changed', self.label_changed_cb, class_id)
        self.add_classification_item(self.color_buttons[class_id], 
                                     self.sym_menus[class_id],
                                     self.scale_spinners[class_id],
                                     self.ranges[class_id], 
                                     self.labels[class_id])
        self.class_list.show_all()

    def add_classification_item(self, clr, sym, scl, rng, lbl, 
                                delete_button=True):
        """add a single row to the classification list.  Optionally add a delete 
        button that will delete that row from the classification.
        """
        class_item = gtk.ListItem()
        class_box = gtk.HBox()
        #explicitly size the first 5, let the last one fill the rest of the 
        #space.
        clr.set_size_request(70, -1)
        if sym is not None:
            sym.set_size_request(70, -1)
        if scl is not None:
            scl.set_size_request(70, -1)
        rng.set_size_request(130, -1)
        lbl.set_size_request(130, -1)
        class_box.pack_start(clr, expand=False, fill=False)
        if sym is not None:
            class_box.pack_start(sym, expand=False, fill=False)
        if scl is not None:
            class_box.pack_start(scl, expand=False, fill=False)
        class_box.pack_start(rng, expand=False, fill=False)
        class_box.pack_start(lbl, expand=False, fill=False)
        if delete_button:
            del_btn = gtk.Button('x')
            del_btn.set_size_request( 45, -1 )
            del_btn.connect('clicked', self.delete_item, class_item)
            class_box.pack_start(del_btn, expand=False, fill=False)
        class_box.add( gtk.Label( '' ) )
        class_item.add(class_box)
        class_item.show()
        self.class_list.add(class_item)

    def reset_classification_list(self, *args):
        """Set the contents of class_list to the classification
        scheme in the classification object."""

        #clear existing UI side items.
        self.class_list.clear_items(0, -1)
        del self.color_buttons, self.ranges, self.labels
        self.color_buttons = []
        self.ranges = []
        self.labels = []

        cls = self.classification
        #prepare a default classification if one doesn't exist
        if cls.count == 0:
            cls.prepare_default(5)

        symbol = cls.get_symbol( 0 )
        #setup the column headers
        class_item = gtk.ListItem()
        set_widget_background(class_item, (1.0, 0.0, 0.0))
        class_box = gtk.HBox()
        clr_frm = gtk.Frame()
        clr_frm.add(gtk.Label('Color'))
        clr_frm.set_shadow_type(gtk.SHADOW_OUT)
        if symbol is not None:
            sym_frm = gtk.Frame()
            sym_frm.add( gtk.Label( 'Symbol' ))
            sym_frm.set_shadow_type( gtk.SHADOW_OUT )

            scale_frm = gtk.Frame()
            scale_frm.add( gtk.Label( 'Scale' ))
            scale_frm.set_shadow_type( gtk.SHADOW_OUT )
        else:
            sym_frm = None
            scale_frm = None
        rng_frm = gtk.Frame()
        rng_frm.add(gtk.Label('Range'))
        rng_frm.set_shadow_type(gtk.SHADOW_OUT)
        lbl_frm = gtk.Frame()
        lbl_frm.add(gtk.Label('Label'))
        lbl_frm.set_shadow_type(gtk.SHADOW_OUT)
        self.add_classification_item(clr_frm, sym_frm, scale_frm, rng_frm, 
                                     lbl_frm, False)

        #for each class, create an entry in the list
        for n in range(cls.count):
            self.insert_class( n )

        self.class_list.show_all()

        if self.ramp is not None:
            self.apply_ramp(self.ramp)

    def reclassify_cb(self, *args):
        """show the reclassify dlg"""
        dlg = GvReclassifyDlg(ok_cb = self.reset_dlg_cb, 
                              classify_type = self.classification.get_type())
        self.reclassdlg = dlg
        dlg.show()

    def reset_dlg_cb(self, dlg, *args):
        """reset the classification to the default"""
        self.classification.set_type( dlg.classify_type )
        self.classification.remove_all_classes()
        self.classification.prepare_default(dlg.classes)
        self.reset_classification_list()
        self.reclassdlg = None

    def reset_cb(self,*args):
        """reset the classification to the default"""
        self.classification.remove_all_classes()
        self.classification.prepare_default(5)
        self.reset_classification_list()

    def delete_item(self, btn, item):
        """Remove a class from the classification"""
        n = self.class_list.child_position(item) - 1
        self.classification.remove_class(n)
        self.class_list.remove_items([item])
        del self.color_buttons[n]
        del self.ranges[n]
        del self.labels[n]

    ##def color_button_cb(self, widget, color, num, *args):
    def color_button_cb(self, widget, num):
        """Handle the user changing a color value"""
        self.classification.set_color(num, widget.get_color())

    def symbol_change( self, widget, index, symbol ):
        self.classification.set_symbol( index, symbol )

    def scale_change( self, widget, index ):
        self.classification.set_scale( index, widget.value )

    def range_changed_cb(self, widget, num):
        """Handle the user changing a range value.  This requires validation"""
        print 'range_changed_cb'
        #if self.updating: return
        self.updating = True
        range_txt = strip(widget.get_text()) #remove whitespace
        vals = split(range_txt, '-')
        if range_txt == '':
            #nothing entered
            return
        # lots of hackery here recognise various cases with negatives.
        # Negatives come out of the split as an empty token.
        if len(vals) == 4 and vals[0] == '' and vals[2] == '':
            try:
                low = -float(vals[1])
                hi = -float(vals[3])
            except:
                low = '-' + vals[1]
                hi = '-' + vals[3]
        elif len(vals) == 3 and vals[0] == '':
            try:
                low = -float(vals[1])
                hi = float(vals[2])
            except:
                low = '-' + vals[1]
                hi = vals[2]

        elif len(vals) == 3 and vals[1] == '':
            try:
                low = float(vals[0])
                hi = -float(vals[2])
            except:
                low = vals[0]
                hi = '-' + vals[2]

        elif len(vals) == 2:
            #two vals
            try:
                low = float(vals[0])
                hi = float(vals[1])
            except:
                low = vals[0]
                hi = vals[1]

        elif len(vals) == 1:
            #one val
            try:
                low = float(vals[0])
                hi = float(vals[0])
            except:
                low = vals[0]
                hi = vals[0]
        else:
            #too many values
            return

        try:
            if int(low) == low:
                low_txt = "%.0f" % low
            else:
                low_txt = "%s" % low
        except:
            low_txt = low

        try:
            if int(hi) == hi:
                hi_txt = "%.0f" % hi
            else:
                hi_txt = "%s" % hi
        except:
            hi_txt = hi

        r_low, r_hi = self.classification.get_range( num )
        old_name = self.classification.get_name( num )

        try:
            if int(r_low) == r_low:
                r_low_txt = "%.0f" % r_low
            else:
                r_low_txt = "%s" % r_low
        except:
            r_low_txt = r_low

        try:
            if int(r_hi) == r_hi:
                r_hi_txt = "%.0f" % r_hi
            else:
                r_hi_txt = "%s" % r_hi
        except:
            r_hi_txt = r_hi

        if r_hi_txt == "":
            calc_name = r_low_txt
        else:
            calc_name = "%s - %s" % (r_low_txt, r_hi_txt)

        print 'old rng is ', r_low, r_hi
        print 'new rng is ', low, hi
        print 'name is ', old_name
        print 'calc is ', calc_name
        if calc_name == old_name:
            if hi_txt == "":
                calc_name = low_txt
            else:
                calc_name = "%s - %s" % ( low_txt, hi_txt )
        else:
            calc_name = old_name
        print 'new is ', calc_name            
        self.classification.set_range(num, low, hi)
        self.labels[num].set_text( calc_name )
        self.updating = False

    def label_changed_cb(self, widget, num):
        """Handle the user changing the label."""
        self.classification.set_name(num, widget.get_text())

    def title_changed_cb(self, widget):
        """Handle the user changing the title"""
        self.classification.set_title(widget.get_text())

    def ramp_cb(self, widget, ramp, *args):
        if ramp is not None:
            self.ramp = ramp
            self.apply_ramp(ramp)
        else:
            #TODO: custom ramp creator here.
            pass

    def apply_ramp_cb(self, n, color):
        self.classification.set_color(n, color)
        self.color_buttons[n].set_color(color)

    def apply_ramp(self, ramp, *args):
        ramp.apply_ramp(self.apply_ramp_cb, self.classification.count)

    def save_cb(self, *args):
        import filedlg
        dlg = filedlg.FileDialog(dialog_type=filedlg.FILE_SAVE, filter='Legend Files|*.leg')
        dlg.ok_button.connect('clicked', self.save, dlg)
        dlg.cancel_button.connect('clicked', dlg.hide)
        dlg.show()

    def save(self, widget, dlg, *args):
        import pickle
        import os
        filename = dlg.get_filename()
        path, ext = os.path.splitext(filename)
        if not (ext == ".leg"):
            gvutils.warning("filename extension changed to .leg")
        ext = ".leg"
        filename = path + ext
        dlg.hide()
        if os.path.exists( filename ):
            warning_pix = os.path.join(gview.home_dir, 'pics', 'warning.xpm' )
            win = gvutils._MessageBox( "Do you wish to overwrite the existing file?",
                                       ( 'Yes', 'No', ), warning_pix, modal=True)
            win.set_title( 'File Exists' )
            win.show()
            gtk.main()
            if win.ret == 'No':
                print 'not saving'
                return
        print 'saving'        
        file = open(filename, "w")
        d = self.classification.serialize()
        try:
            pickle.dump(d, file)
        except PicklingError:
            gvutils.error('An error occurred saving the classification:\n' + filename)

    def load_cb(self, *args):
        import filedlg
        dlg = filedlg.FileDialog(dialog_type=filedlg.FILE_OPEN, filter='Legend Files|*.leg')
        dlg.ok_button.connect('clicked', self.load, dlg)
        dlg.cancel_button.connect('clicked', dlg.hide)
        dlg.show()

    def load(self, widget, dlg, *args):
        import pickle
        filename = dlg.get_filename()
        dlg.hide()
        try:
            file = open(filename, "r")
            d = pickle.load(file)
            self.classification.deserialize(d)
            self.ramp = None
            self.reset_classification_list()
            self.title_txt.set_text(self.classification.get_title())
        except:
            gvutils.error('Error opening classification file:\n' + filename)

    def property_select_cb(self, *args):
        if len(self.classification.layers) == 0:
            return

        if self.property_updating:
            return

        layer = self.classification.layers[0]

        if not issubclass(layer.__class__,gview.GvShapesLayer):
            self.property_list.hide()
            return

        new_property = self.property_list.child.get_text()

        self.classification.set_classify_property( layer, new_property )

        self.classification.remove_all_classes()
        self.classification.prepare_default(5)
        self.reset_classification_list()
        self.title_txt.set_text( self.classification.get_title() )
    def update_property_list( self, *args ):
        if len(self.classification.layers) == 0:
            return

        layer = self.classification.layers[0]

        if not issubclass(layer.__class__,gview.GvShapesLayer):
            self.property_list.hide()
            return

        self.property_list.show()

        property = self.classification.get_classify_property( layer )
        schema = layer.get_parent().get_schema()
        fields = []
        for field_def in schema:
            fields.append( field_def[0] )

        self.property_updating = 1
        for f in fields:
            self.property_list.append_text(f)
        #self.property_list.set_popdown_strings( tuple(fields) )
        print property
        if property is not None:
            i = fields.index(property)
            self.property_list.set_active(i)
        self.property_updating = 0

class GvReclassifyDlg(gtk.Window):
    """This dialog displays a re-classification dialog that allows
    the user to specify the number of classes, and the type of classification
    """
    def __init__(self, ok_cb=None, cancel_cb=None, cb_data=None, 
                 classify_type=CLASSIFY_EQUAL_INTERVAL):
        gtk.Window.__init__(self)
        self.set_title('Classification')
        self.user_ok_cb = ok_cb
        self.user_cancel_cb = cancel_cb
        self.user_cb_data = cb_data
        self.classify_type = classify_type
        self.set_border_width(6)
        #main vertical box
        vbox = gtk.VBox(spacing=6)
        type_box = gtk.HBox(spacing=6)
        type_box.pack_start(gtk.Label('Type:'), expand=False)
        opt_menu = gtk.OptionMenu()
        type_menu = gtk.Menu()

        #using classification_types dictionary from gvclassification
        for i in range(len(classification_types)):
            for type in classification_types.iteritems():
                if type[1] == i:
                    item = gtk.MenuItem( type[0] )
                    item.connect( 'activate', self.type_menu_cb, classification_types[type[0]] )
                    type_menu.append( item )

        opt_menu.set_menu(type_menu)
        opt_menu.set_history( classify_type )
        opt_menu.resize_children()
        type_box.pack_start(opt_menu)
        vbox.pack_start(type_box, expand=False)
        #Number of classes
        classes_box = gtk.HBox(spacing=6)
        classes_box.pack_start(gtk.Label('Number of classes:'))
        adj = gtk.Adjustment(5, 2, 80, 1, 5, 5)
        self.spinner = gtk.SpinButton(adj)
        self.spinner.set_snap_to_ticks(True)
        self.spinner.set_digits(0)
        classes_box.pack_start(self.spinner)
        vbox.pack_start(classes_box, expand=False)
        #add the ok and cancel buttons
        button_box = gtk.HButtonBox()
        ok_button = gtk.Button("OK")
        ok_button.connect('clicked', self.ok_cb, cb_data)
        cancel_button = gtk.Button("Cancel")
        cancel_button.connect('clicked', self.cancel_cb, cb_data)
        button_box.pack_start(ok_button)
        button_box.pack_start(cancel_button)
        vbox.pack_start(button_box, expand=False)
        vbox.show_all()
        self.add(vbox)
        ok_button.set_flags(gtk.CAN_DEFAULT)
        ok_button.grab_default()

    def type_menu_cb(self, menu_item, classify_type):
        self.classify_type = classify_type

    def ok_cb(self, *args):
        self.classes = self.spinner.get_value_as_int()
        if self.user_ok_cb is not None:
            self.user_ok_cb(self, self.user_cb_data)
        self.hide()
        self.destroy()

    def cancel_cb(self, *args):
        if self.user_cancel_cb is not None:
            self.user_cancel_cb(self.user_cb_data, self)
        self.hide()
        self.destroy()

if __name__ == '__main__':
    gview.set_preference('ramp_directory', '../../resource/ramps')
    ##shapes = gview.GvShapes( shapefilename="c:/projects/dmsolutions/ciet/ciet_data/wcsite.shp" )
    shapes = gview.GvShapes( shapefilename="/home/jcollins/vexcel/data_tmp/ships.shp")
    layer = gview.GvShapesLayer( shapes=shapes )
    cls = GvClassification( layer )
    dlg = GvClassificationDlg( cls )
    dlg.apply_button.connect('clicked', cls.dump)
    dlg.apply_button.connect('clicked', gtk.main_quit)

    dlg.connect('delete-event', gtk.main_quit)
    dlg.show()
    gtk.main()

