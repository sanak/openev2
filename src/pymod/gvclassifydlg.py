###############################################################################
# $Id$
#
# Project:  CIETmap / OpenEV
# Purpose:  Raster/Vector classification dialogs
# Author:   Paul Spencer, pgs@magma.ca
#
# Maintained by Mario Beauchamp (starged@gmail.com) for CIETcanada
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

# differentiate between CIETMap and OpenEV
import os
if 'CIETMAP_HOME' in os.environ:
    import cview as gview
    from cietutils import set_help_topic
else:
    import gview

from gvutils import error, yesno, create_stock_button
import gtk
import pgu
from pgucolor import ColorButton, ColorRamp
from gvogrfsgui import SymbolsCombo
from gvsignaler import Signaler
from gvclassification import *

# temporary
def _(s):
    return s

# From Paul:
"""gvclassifydlg.py module contains two classes related to raster classification.

GvClassificationDlg is the main gui for modifying raster classifications.
GvReclassifyDlg is a supplementary dialog for changing the number of classes in
a classification scheme.

This module also contains a number of supplementary utilities for working with ramps.


TODO:

 x find out how to modify the background color of a widget and use it in the column
   headers

 x determine when range values are valid/invalid.  When invalid, set bg color of cell
   to red or something.

 x provide more buttons for doin' stuff.
"""

MIN_COLOR = 0
LOW_COLOR = 21845
HI_COLOR = 65535
MAX_COLOR = 65535

def load_ramp(ramp_file):
    """process a ramp file"""
    if os.path.isfile(ramp_file):
        ramp = ColorRamp()
        try:
            ramp.deserialize(ramp_file)
            return ramp
        except:
            print "invalid ramp file %s" % ramp_file

def load_ramps():
    """reads in all the ramp files in the ramps directory and creates ramps for them"""
    ramps = []
    home_dir = gview.home_dir
    ramp_dir = gview.get_preference('ramp_directory')
    if ramp_dir is None:
        ramp_dir = os.path.join(home_dir, 'ramps')
    if os.path.isdir(ramp_dir):
        files = os.listdir(ramp_dir)
        for file in files:
            ramp = load_ramp(os.path.join(ramp_dir, file))
            if ramp:
                ramps.append(ramp)
    return ramps

def load_ramp_config_file():
    """
    Reads in ramp files specified in the ramp config file
    in the ramps directory and creates ramps for them

    This allows for ordering of the ramps in the config
    file and for specifying separators
    """
    ramps = []
    home_dir = gview.home_dir
    ramp_dir = gview.get_preference('ramp_directory')
    if ramp_dir is None:
        ramp_dir = os.path.join(home_dir, 'ramps')
    if os.path.isdir(ramp_dir):
        config_path = os.path.join(ramp_dir, 'ramps.cfg')
        if os.path.isfile(config_path):
            #load config file and parse ramps ...
            config = open(config_path)
            lines = config.readlines()
            for line in lines:
                ramp_file = line.strip()
                ramp = load_ramp(os.path.join(ramp_dir, ramp_file))
                if ramp:
                    ramps.append(ramp)
        else:
            return load_ramps()

    return ramps

class GvClassificationDlg(gtk.Window, Signaler):
    """A dialog for modifying the classification scheme of a GvLayer."""
    def __init__(self, cls, cwd=None):
        """Initialize a GvClassificationDlg on a particular GvLayer"""
        gtk.Window.__init__(self)
        self.set_title(_("Layer Classification"))
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
        self.ramps = items

        if cwd:
            self.cwd = cwd
        else:
            self.cwd = os.getcwd()

        if cls is None:
            self.cls = GvClassification()
        elif isinstance(cls, GvClassification):
            self.cls = cls
        else:
            raise TypeError, "GvClassificationDlg now requires a GvClassification instance"
        self.classification = self.cls
        if self.cls.count <= 0:
            self.ramp = items[0]
            self.cls.prepare_default()

        #main vertical box
        vbox = gtk.VBox(spacing=5)
        self.add(vbox)

        if isinstance(cls.layers[0], gview.GvShapesLayer):
            self.property_list = pgu.ComboText(action=self.property_select_cb)
            self.property_list.set_size_request(125, -1)
            self.update_property_list()
            ncols = 3
        else:
            self.property_list = None
            ncols = 2

        hbox = gtk.HBox(homogeneous=True, spacing=30)
        vbox.pack_start(hbox, expand=False)

        button = gtk.Button(stock=gtk.STOCK_OPEN)
        button.connect('clicked', self.load_cb)
        hbox.pack_start(button, expand=False)
        button = gtk.Button(stock=gtk.STOCK_SAVE)
        button.connect('clicked', self.save_cb)
        hbox.pack_start(button, expand=False)

        if self.property_list:
            hbox.pack_start(self.property_list, expand=False)

        #classification frame
        class_frame = gtk.Frame()
        vbox.pack_start(class_frame)
        frame_box = gtk.VBox(spacing=3)
        frame_box.set_border_width(5)
        class_frame.add(frame_box)

        hbox = gtk.HBox(spacing=5)
        frame_box.pack_start(hbox, expand=False)
        label = pgu.Label(_("Legend Title:"))
        hbox.pack_start(label, expand=False)
        self.title_txt = gtk.Entry()
        self.title_txt.set_text(self.cls.get_title())
        self.title_txt.connect('changed', self.title_changed_cb)
        hbox.pack_start(self.title_txt)

        #classification list
        class_box = gtk.ScrolledWindow()
        # let the viewport determine the sizes
        class_box.set_size_request(436, 148)
        class_box.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.class_list = gtk.VBox()
        class_box.add_with_viewport(self.class_list)
        frame_box.pack_start(class_box)
        self.reset_cls_list()

        hbox = gtk.HBox(homogeneous=True, spacing=30)
        frame_box.pack_start(hbox, expand=False)

        # Classification buttons
        button = gtk.Button(_("Add class"))
        button.connect('clicked', self.add_class_cb)
        hbox.pack_start(button, expand=False)
        button = gtk.Button(_("Reclassify"))
        button.connect('clicked', self.reclassify_cb)
        hbox.pack_start(button, expand=False)
        button = gtk.Button(_("Revert"))
        button.connect('clicked', self.reset_cb)
        hbox.pack_start(button, expand=False)

        #Color Ramp choices
        hbox = gtk.HBox(spacing=5)
        frame_box.pack_start(hbox, expand=False)
        label = pgu.Label(_("Color Ramps:"))
        hbox.pack_start(label, expand=False)

        lst = gtk.ListStore(gtk.gdk.Pixbuf,str)
        for ramp in items:
            lst.append((ramp.gradient.get_pixbuf(), ramp.title))
        combo = pgu.ComboBox(model=lst)
        combo.set_active(0)
        combo.connect(cb=self.ramp_cb)
        hbox.pack_end(combo, expand=False)

        # buttons
        hbox = gtk.HBox(homogeneous=True, spacing=30)
        vbox.pack_start(hbox, expand=False)
        self.ok_button = gtk.Button(stock=gtk.STOCK_OK)
        self.ok_button.connect('clicked', self.ok_cb)
        hbox.pack_start(self.ok_button, expand=False)
        self.apply_button = gtk.Button(_("Apply"))
        self.apply_button.connect('clicked', self.apply_cb)
        hbox.pack_start(self.apply_button, expand=False)
        self.cancel_button = gtk.Button(stock=gtk.STOCK_CANCEL)
        self.cancel_button.connect('clicked', self.cancel_cb)
        hbox.pack_start(self.cancel_button, expand=False)

        self.show_all()

        #make ok_button a default button
        self.ok_button.set_flags(gtk.CAN_DEFAULT)
        self.ok_button.grab_default()
        self.publish('classification-changed')
        # sets help for CIETMap
        if 'CIETMAP_HOME' in os.environ:
            set_help_topic(self, _('cm-help-classification'))

    def close(self, *args):
        """close and destroy this dialog"""
        self.hide()
        self.destroy()
        if self.reclassdlg:
            self.reclassdlg.destroy()
        return True

    def ok_cb(self, *args):
        """Close the dialog and notify listeners and the raster that the
        classification has changed"""
        #allowing the raster to be rescale screws things up!
        self.cls.update_all_layers(rescale=1)
        self.notif('classification-changed')
        return self.close()

    def apply_cb(self, *args):
        """apply the current classification"""
        self.cls.update_all_layers(rescale=1)
        self.notif('classification-changed')

    def cancel_cb(self, *args):
        """close the classification dialog without doing anything
        about the classification"""
##        self.reset_cb()
##        self.cls.remove_all_classes()
##        self.notify('classification-changed')
        return self.close()

    def add_class_cb(self, *args):
        """add a single class to the classification.  Add it at the end
        with the same value and color as the upper range value of the
        last class.  If there are no classes, use the entire range.
        """
        #first create the new class
        cls = self.cls
        if self.color_buttons:
            color = self.color_buttons[-1].get_color()
            rng = cls.get_range(cls.count - 1)
            rng = (rng[1], rng[1])
            symbol = cls.get_symbol(cls.count - 1)
            scale = cls.get_scale(cls.count - 1)
        else:
            color = (0.0, 0.0, 0.0, 1.0)
            layer = cls.layers[0]
            rng = cls.collect_range(layer)
            #for point layers only
            if layer.parent[0].get_shape_type() == gview.GVSHAPE_POINT:
                symbol = '"ogr-sym-0"'
            else:
                symbol = None
            scale = 1.0

        n = cls.set_class(color=color, range_min=rng[0], range_max=rng[1], symbol=symbol, symbol_scale=scale)
        self.insert_class(n)                          

    def insert_class(self, class_id):
        """Create gui elements for the class_id and insert them
        into the gui
        """
        cls = self.cls
        color = cls.get_color(class_id)
        title = "Select Class %s Color" % cls.get_name(class_id)
        self.color_buttons.insert(class_id, ColorButton(color, title))
        self.color_buttons[class_id].connect('color-set', self.color_button_cb, class_id)
        symbol = cls.get_symbol(class_id)
        if symbol:
            combo = SymbolsCombo()
            combo.set_ogr_symbol(symbol)
            combo.connect('changed', self.symbol_change, class_id)
            # do not show the symbol names
            combo.clear_attributes(combo.txt)
            self.sym_menus.insert(class_id, combo)

            scale = cls.get_scale(class_id)
            adj = gtk.Adjustment(value=scale, lower=0.0, upper=100.0, 
                                     step_incr=1.0, page_incr=5.0, page_size=5.0)
            scale_spin = gtk.SpinButton(adj)
            scale_spin.set_editable(True)
            adj.connect('value-changed', self.scale_change, class_id)
            self.scale_spinners.insert(class_id, scale_spin)
        else:
            self.sym_menus.insert(class_id, None)
            self.scale_spinners.insert(class_id, None)

        self.ranges.insert(class_id, pgu.Entry())
        rng = cls.get_range(class_id)
        rng_txt = str( rng[0] )
        if rng[1] and cls.get_type() != CLASSIFY_DISCRETE:
            rng_txt += ('-%s' % rng[1])
        self.ranges[class_id].set_text(rng_txt)
        self.ranges[class_id].connect('changed', self.range_changed_cb, class_id)
        self.labels.insert(class_id, pgu.Entry())
        self.labels[class_id].set_text(cls.get_name(class_id))
        self.labels[class_id].connect('changed', self.label_changed_cb, class_id)
        self.add_cls_item(self.color_buttons[class_id], 
                                     self.sym_menus[class_id],
                                     self.scale_spinners[class_id],
                                     self.ranges[class_id], 
                                     self.labels[class_id])
        self.class_list.show_all()

    def add_cls_item(self, clr, sym, scl, rng, lbl, delete_button=True):
        """add a single row to the classification list.  Optionally add a delete 
        button that will delete that row from the classification.
        """
        class_box = gtk.HBox()
        #explicitly size the first 5, let the last one fill the rest of the 
        #space.
        h = 24
        clr.set_size_request(48, h)
        rng.set_size_request(130, h)
        lbl.set_size_request(130, h)
        class_box.pack_start(clr, expand=False)
        if sym:
            sym.set_size_request(50, h)
            class_box.pack_start(sym, expand=False, fill=False)
        if scl:
            scl.set_size_request(50, h)
            class_box.pack_start(scl, expand=False, fill=False)
        class_box.pack_start(rng, expand=False)
        class_box.pack_start(lbl, expand=False)
        if delete_button:
            del_btn = create_stock_button(gtk.STOCK_DELETE, self.delete_item, class_box)
            del_btn.set_size_request(h, h)
            class_box.pack_start(del_btn, expand=False)
        class_box.pack_start( gtk.Label() )
        self.class_list.pack_start(class_box, expand=False)

    def reset_cls_list(self, *args):
        """Set the contents of class_list to the classification
        scheme in the classification object."""

        #clear existing UI side items.
        for item in self.class_list.get_children():
            self.class_list.remove(item)
            item.unrealize()
            item.destroy()
        del self.color_buttons, self.ranges, self.labels
        self.color_buttons = []
        self.ranges = []
        self.labels = []

        cls = self.cls
        #prepare a default classification if one doesn't exist
        if cls.count == 0:
            cls.prepare_default(5)

        symbol = cls.get_symbol(0)
        #setup the column headers
        class_box = gtk.HBox()
        clr_frm = gtk.Frame()
        clr_frm.set_shadow_type(gtk.SHADOW_OUT)
        clr_frm.add( gtk.Label(_("Color")) )
        if symbol:
            sym_frm = gtk.Frame()
            sym_frm.set_shadow_type(gtk.SHADOW_OUT)
            sym_frm.add( gtk.Label(_("Symbol")) )

            scale_frm = gtk.Frame()
            scale_frm.set_shadow_type(gtk.SHADOW_OUT)
            scale_frm.add( gtk.Label(_("Scale")) )
        else:
            sym_frm = None
            scale_frm = None
        rng_frm = gtk.Frame()
        rng_frm.set_shadow_type(gtk.SHADOW_OUT)
        rng_frm.add( gtk.Label(_("Range")) )
        lbl_frm = gtk.Frame()
        lbl_frm.set_shadow_type(gtk.SHADOW_OUT)
        lbl_frm.add( gtk.Label(_("Label")) )
        self.add_cls_item(clr_frm, sym_frm, scale_frm, rng_frm, lbl_frm, False)

        #for each class, create an entry in the list
        for n in range(cls.count):
            self.insert_class(n)

        self.class_list.show_all()

        if self.ramp:
            self.apply_ramp(self.ramp)

    def reclassify_cb(self, *args):
        """show the reclassify dlg"""
        dlg = GvReclassifyDlg(ok_cb=self.reset_dlg_cb, classify_type=self.cls.get_type())
        self.reclassdlg = dlg
        dlg.show()

    def reset_dlg_cb(self, dlg, *args):
        """reset the classification to the default"""
        self.cls.set_type( dlg.classify_type )
        self.cls.remove_all_classes()
        self.cls.prepare_default(dlg.classes)
        self.reset_cls_list()
        self.reclassdlg = None

    def reset_cb(self, *args):
        """reset the classification to the default"""
        self.cls.remove_all_classes()
        self.cls.prepare_default(5)
        self.reset_cls_list()

    def delete_item(self, btn, item):
        """Remove a class from the classification"""
        n = self.class_list.get_children().index(item) - 1
        self.cls.remove_class(n)
        self.class_list.remove(item)
        del self.color_buttons[n]
        del self.ranges[n]
        del self.labels[n]

    def color_button_cb(self, widget, num, *args):
        """Handle the user changing a color value"""
        self.cls.set_color(num, widget.get_color())

    def symbol_change(self, combo, index):
        symbol = combo.get_symbol()
        self.cls.set_symbol(index, symbol)

    def scale_change(self, widget, index):
        self.cls.set_scale(index, widget.value)

    def range_changed_cb(self, widget, num):
        """Handle the user changing a range value.  This requires validation"""
        #if self.updating: return
        self.updating = True
        range_txt = widget.get_text().strip() #remove whitespace
        vals = range_txt.split('-')
        if not range_txt:
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
                low_txt = '%.0f' % low
            else:
                low_txt = '%s' % low
        except:
            low_txt = low

        try:
            if int(hi) == hi:
                hi_txt = '%.0f' % hi
            else:
                hi_txt = '%s' % hi
        except:
            hi_txt = hi

        r_low, r_hi = self.cls.get_range( num )
        old_name = self.cls.get_name( num )

        try:
            if int(r_low) == r_low:
                r_low_txt = '%.0f' % r_low
            else:
                r_low_txt = '%s' % r_low
        except:
            r_low_txt = r_low

        try:
            if int(r_hi) == r_hi:
                r_hi_txt = '%.0f' % r_hi
            else:
                r_hi_txt = '%s' % r_hi
        except:
            r_hi_txt = r_hi

        if r_hi_txt == '':
            calc_name = r_low_txt
        else:
            calc_name = '%s - %s' % (r_low_txt, r_hi_txt)

##        print 'old rng is ', r_low, r_hi
##        print 'new rng is ', low, hi
##        print 'name is ', old_name
##        print 'calc is ', calc_name
        if calc_name == old_name:
            if hi_txt == '':
                calc_name = low_txt
            else:
                calc_name = '%s - %s' % ( low_txt, hi_txt )
        else:
            calc_name = old_name
##        print 'new is ', calc_name            
        self.cls.set_range(num, low, hi)
        self.labels[num].set_text( calc_name )
        self.updating = False

    def label_changed_cb(self, widget, num):
        """Handle the user changing the label."""
        self.cls.set_name(num, widget.get_text())

    def title_changed_cb(self, widget):
        """Handle the user changing the title"""
        self.cls.set_title(widget.get_text())

    def ramp_cb(self, combo):
        active = combo.get_active()
        if active > -1:
            self.ramp = self.ramps[active]
            self.apply_ramp(self.ramp)
        else:
            #TODO: custom ramp creator here.
            pass

    def apply_ramp_cb(self, n, color):
        self.cls.set_color(n, color)
        self.color_buttons[n].set_color(color)

    def apply_ramp(self, ramp, *args):
        ramp.apply_ramp(self.apply_ramp_cb, self.cls.count)

    def save_cb(self, *args):
        if 'CIETMAP_HOME' in os.environ:
            filedlg = cmfiledlg
        from filedlg import file_save
        file_save(_("Save Legend"), self.cwd, filter=['leg'], cb=self.save)

    def save(self, filename, cwd):
        import pickle
        self.cwd = cwd

        path, ext = os.path.splitext(filename)
        if ext != '.leg':
            ext = '.leg'
        filename = path + ext

        if os.path.exists(filename):
            ret = yesno(title=_("File exists"), text=_("Do you wish to overwrite the existing file?"))
            if ret == 'No':
                return

        file = open(filename, 'w')
        d = self.cls.serialize()
        try:
            pickle.dump(d, file)
        except PicklingError:
            msg = _("An error occurred saving the classification:")
            error('%s\n%s' % (msg, filename))

    def load_cb(self, *args):
        if 'CIETMAP_HOME' in os.environ:
            filedlg = cmfiledlg
        from filedlg import file_open
        file_open(_("Load Legend"), self.cwd, filter=['leg'], cb=self.load)

    def load(self, filename, cwd):
        import pickle
        self.cwd = cwd

        try:
            file = open(filename, 'r')
            d = pickle.load(file)
            self.cls.deserialize(d)
            self.ramp = None
            self.reset_cls_list()
            self.title_txt.set_text(self.cls.get_title())
        except:
            msg = _("Error opening classification file:")
            error('%s\n%s' % (msg, filename))

    def property_select_cb(self, *args):
        if not self.cls.layers or self.property_updating:
            return

        layer = self.cls.layers[0]
        new_property = self.property_list.get_text()
        self.cls.set_classify_property(layer, new_property)

        self.cls.remove_all_classes()
        self.cls.prepare_default(5)
        self.reset_cls_list()
        self.title_txt.set_text(self.cls.get_title())

    def update_property_list(self, *args):
        if not self.cls.layers:
            return

        layer = self.cls.layers[0]

        property = self.cls.get_classify_property(layer)
        fields = layer.get_parent().get_fieldnames()

        self.property_updating = True
        self.property_list.set_popdown_strings(fields)
        if property:
            self.property_list.set_active_text(property)
        self.property_updating = False

class GvReclassifyDlg(gtk.Window):
    """This dialog displays a re-classification dialog that allows
    the user to specify the number of classes, and the type of classification
    """
    def __init__(self, ok_cb=None, cancel_cb=None, cb_data=None, 
                 classify_type=CLASSIFY_EQUAL_INTERVAL):
        gtk.Window.__init__(self)
        self.set_title(_("Classification"))
        self.user_ok_cb = ok_cb
        self.user_cancel_cb = cancel_cb
        self.user_cb_data = cb_data
        self.classify_type = classify_type
        self.set_border_width(6)
        #main vertical box
        vbox = gtk.VBox(spacing=6)
        self.add(vbox)

        hbox = gtk.HBox(spacing=6)
        vbox.pack_start(hbox, expand=False)
        hbox.pack_start(gtk.Label(_("Type:")), expand=False)

        #using classification_types dictionary from gvclassification
        opt_list = gtk.ListStore(str, int)
        for item in classification_types.iteritems():
            opt_list.append(item)

        combo = pgu.ComboText(model=opt_list)
        combo.set_active_text(opt_list[classify_type][0])
        combo.connect(cb=self.type_menu_cb)
        hbox.pack_start(combo)

        #Number of classes
        hbox = gtk.HBox(spacing=6)
        self.class_box = hbox
        vbox.pack_start(hbox, expand=False)
        hbox.pack_start(gtk.Label(_("Number of classes:")))

        adj = gtk.Adjustment(5, 2, 80, 1, 5, 5)
        self.spinner = gtk.SpinButton(adj)
        self.spinner.set_snap_to_ticks(True)
        self.spinner.set_digits(0)
        hbox.pack_start(self.spinner)
        hbox.set_sensitive( (classify_type not in (CLASSIFY_DISCRETE,CLASSIFY_NORM_SD)) )

        #add the ok and cancel buttons
        button_box = gtk.HButtonBox()
        vbox.pack_start(button_box, expand=False)
        ok_button = gtk.Button(stock=gtk.STOCK_OK)
        ok_button.connect('clicked', self.ok_cb, cb_data)
        button_box.pack_start(ok_button)
        cancel_button = gtk.Button(stock=gtk.STOCK_CANCEL)
        cancel_button.connect('clicked', self.cancel_cb, cb_data)
        button_box.pack_start(cancel_button)

        vbox.show_all()
        ok_button.set_flags(gtk.CAN_DEFAULT)
        ok_button.grab_default()

    def type_menu_cb(self, combo):
        model = combo.get_model()
        cls_type = model[combo.get_active()][1]
        self.classify_type = cls_type
        self.class_box.set_sensitive( (cls_type not in (CLASSIFY_DISCRETE,CLASSIFY_NORM_SD)) )

    def ok_cb(self, *args):
        self.classes = self.spinner.get_value_as_int()
        if self.user_ok_cb:
            self.user_ok_cb(self, self.user_cb_data)
        self.hide()
        self.destroy()

    def cancel_cb(self, *args):
        if self.user_cancel_cb:
            self.user_cancel_cb(self.user_cb_data, self)
        self.hide()
        self.destroy()
