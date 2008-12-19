###############################################################################
# $Id$
#
# Project:  OpenEV / CIETmap
# Purpose:  GUI Widgets for displaying and manipulating OGRFS rendering
#           descriptions.
# Author:   Frank Warmerdam, warmerdam@pobox.com
#
# Maintained by Mario Beauchamp (starged@gmail.com) for CIETcanada
#
###############################################################################
# Copyright (c) 2001, Frank Warmerdam <warmerdam@pobox.com>
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
import gtk.keysyms
from gvsignaler import Signaler
import os
if 'CIETMAP_HOME' in os.environ:
    import cview as gview
else:
    import gview

from gvutils import create_pixbuf
import pgu
from pgucolor import ColorButton, ColorControl
from pgufont import FontControl
import gvogrfs

# temporary
def _(s):
    return s

#map display names to ogr symbol names and an icon (for an image menu)
ogrfs_symbols = {
    "cross"             : ('ogr-sym-0', 'sym_cross'),
    "x"                 : ('ogr-sym-1', 'sym_x'),
    "unfilled circle"   : ('ogr-sym-2', 'sym_circle'),
    "filled circle"     : ('ogr-sym-3', 'sym_filled_circle'),
    "unfilled square"   : ('ogr-sym-4', 'sym_square'),
    "filled square"     : ('ogr-sym-5', 'sym_filled_square'),
    "unfilled triangle" : ('ogr-sym-6', 'sym_triangle'),
    "filled triangle"   : ('ogr-sym-7', 'sym_filled_triangle'),
    "unfilled star"     : ('ogr-sym-8', 'sym_star'),
    "filled star"       : ('ogr-sym-9', 'sym_filled_star'),
    "vertical bar"      : ('ogr-sym-10','sym_vertical')
         }

ogrfs_symbol_names = ["cross", "x",
             "unfilled circle", "filled circle",
             "unfilled square", "filled square",
             "unfilled triangle", "filled triangle",
             "unfilled star", "filled star",
             "vertical bar"]

ogrfs_pens = {
    "solid"             : ('"ogr-pen-0"', 'pen_solid'),
    "dash"              : ('"ogr-pen-2"', 'pen_dash'),
    "short dash"        : ('"ogr-pen-3"', 'pen_short_dash'),
    "long dash"         : ('"ogr-pen-4"', 'pen_long_dash'),
    "dot"               : ('"ogr-pen-5"', 'pen_dot'),
    "dash-dot"          : ('"ogr-pen-6"', 'pen_dash_dot'),
    "dash-dot-dot"      : ('"ogr-pen-7"', 'pen_dash_dot_dot'),
    "alternate pixels"  : ('"ogr-pen-8"', 'pen_alternate'),
         }

ogrfs_pen_names = ["solid",
                    "dash",
                    "short dash",
                    "long dash",
                    "dot",
                    "dash-dot",
                    "dash-dot-dot",
                    "alternate pixels"]

class SymbolsCombo(pgu.ComboBox):
    def __init__(self, action=None):
        sym_list = gtk.ListStore(gtk.gdk.Pixbuf, str)
        for sym_name in ogrfs_symbol_names:
            ogr_sym,sym_img = ogrfs_symbols[sym_name]
            pixbuf = create_pixbuf(sym_img)
            sym_list.append( (pixbuf,sym_name) )
        pgu.ComboBox.__init__(self, sym_list, action)

    def get_symbol(self):
        sym_name = self.get_active_text()
        return ogrfs_symbols[sym_name][0]

    def set_symbol(self, sym_name):
        self.set_active(ogrfs_symbol_names.index(sym_name))

    def set_ogr_symbol(self, ogr_sym):
        sym_name = None
        for name,item in ogrfs_symbols.iteritems():
            if ogr_sym == item[0]:
                sym_name = name
                break
        self.set_symbol(sym_name)

class PensCombo(pgu.ComboBox):
    def __init__(self, action=None):
        pen_list = gtk.ListStore(gtk.gdk.Pixbuf, str)
        for pen_name in ogrfs_pen_names:
            ogr_pen,pen_img = ogrfs_pens[pen_name]
            pixbuf = create_pixbuf(pen_img)
            pen_list.append( (pixbuf, pen_name) )
        pgu.ComboBox.__init__(self, pen_list, action)

    def get_pen(self):
        pen_name = self.get_active_text()
        return ogrfs_pens[pen_name][0]

    def set_pen(self, pen):
        # is it the pen name or the ogr id?
        if pen not in ogrfs_pen_names:
            for name,ogr_pen in ogrfs_pens.iteritems():
                if ogr_pen[0][1:-1] == pen:
                    pen_name = name
                    break
        self.set_active(ogrfs_pen_names.index(pen_name))

class GvLabelStyle(gtk.VBox, Signaler):
    def __init__(self, spacing=10, text_entry=False, layer=None,
                 label_field=True, enable_offsets=False, interactive=False):
        gtk.VBox.__init__(self, spacing=spacing)

        self.ogrfs_obj = None
        self.layer = layer
        self.shape_obj = None
        self.text_entry = text_entry
        self.enable_offsets = enable_offsets
        self.interactive = interactive
        self.updating = False
        self.old_list = []

        self.set_border_width(10)
        self.create_gui()

##        self.connect('unrealize', self.close)

        self.publish('ogrfs-changed')
        self.publish('apply-text-to-field')

        self.gui_update()
        self.show_all()

        if not label_field:
            self.field_label.hide()
            self.label_field.hide()

    def close(self, *args):
        pass

    def set_ogrfs(self, ogrfs_obj, layer=None, fontlist=None, shape_obj=None):
        if layer is not None:
            self.layer = layer

        if ogrfs_obj is None:
            ogrfs_obj = gvogrfs.OGRFeatureStylePart()
            font = gview.get_preference('default-font','Sans 12')
            if self.enable_offsets:
                dx = self.x_offset.get_value()
                dy = self.y_offset.get_value()
            ogrfs_obj.parse('LABEL(t:"",f:"%s",c:#88FF88)' % font)

        self.ogrfs_obj = ogrfs_obj
        self.shape_obj = shape_obj

        self.gui_update()

    def create_gui(self):
        # for clarity
        shrink = gtk.SHRINK

        tips = gtk.Tooltips()
        table = gtk.Table()
        table.set_row_spacings(3)
        table.set_col_spacings(3)
        self.pack_start(table)

        # collect candidate field names from the schema.
        fnlist = [_("disabled")]

        # Field Name
        self.field_label = pgu.Label(_("Label Field:"))
        table.attach(self.field_label, 0, 1, 0, 1, yoptions=shrink)

        combo = pgu.ComboText(strings=fnlist, action=self.label_change_cb)
        table.attach(combo, 1, 5, 0, 1, yoptions=shrink)
        tips.set_tip(combo, _("Select label field"))
        self.label_field = combo

        # Create Color control.
        txt = _("Label Color:")
        label = pgu.Label(txt)
        table.attach(label, 0, 1, 1, 2, yoptions=shrink)
        
        cc = ColorControl(title=txt[:-1], callback=self.label_change_cb)
        table.attach(cc, 1, 5, 1, 2, yoptions=shrink)
        tips.set_tip(cc, _("Select label color"))
        self.label_color = cc

        # Font
        label = pgu.Label(_("Label Font:"))
        table.attach(label, 0, 1, 2, 3, yoptions=shrink)

        fc = FontControl()
        fc.connect('font-set', self.label_change_cb)
        table.attach(fc, 1, 5, 2, 3, yoptions=shrink)
        tips.set_tip(fc, _("Select label font"))
        self.label_font = fc

        #######################################################################
        # Add Text entry/edit
        if self.text_entry:
            label = pgu.Label(_("Text:"))
            table.attach(label, 0, 1, 3, 4, yoptions=shrink)

            entry = gtk.Entry()
            entry.connect('activate',self.text_change_cb)
            if self.interactive:
                entry.connect('changed', self.text_change_cb)
##            else:
##                entry.connect('focus-out-event',self.text_change_cb)
            table.attach(entry, 1, 5, 3, 4)
            self.text_entry = entry
        else:
            self.text_entry = None

        if self.enable_offsets:
            #Label X offset
            label = pgu.Label(_("Label Offsets:"))
            table.attach(label, 0, 1, 4, 5, yoptions=shrink)
            label = pgu.Label("X:")
            table.attach(label, 1, 2, 4, 5, yoptions=shrink)

            spin_adjust = gtk.Adjustment(value=0.0, lower=-20.0, upper=20.0, step_incr=1.0)
            spin = gtk.SpinButton(spin_adjust)
            spin.set_editable(True)
            spin.set_digits(1)
            spin.set_size_request(45, -1)
            spin.connect('changed', self.label_change_cb)
            table.attach(spin, 2, 3, 4, 5, yoptions=shrink)
            self.x_offset = spin

            #Label Y offset
            label = pgu.Label("Y:")
            table.attach(label, 3, 4, 4, 5, yoptions=shrink)

            spin_adjust = gtk.Adjustment(value=0.0, lower=-20.0, upper=20.0, step_incr=1.0)
            spin = gtk.SpinButton(spin_adjust)
            spin.set_editable(True)
            spin.set_digits(1)
            spin.set_size_request(45, -1)
            spin.connect('changed', self.label_change_cb)
            table.attach(spin, 4, 5, 4, 5, yoptions=shrink)
            self.y_offset = spin

    def gui_update(self, *args):
        self.updating = True

        # Update the field list.
        fnlist = [_("disabled")]
        if self.layer:
            schema = self.layer.get_parent().get_schema()
            for item in schema:
                fnlist.append(item[0])

        if fnlist != self.old_list:
            self.label_field.set_popdown_strings(fnlist)
            self.old_list = fnlist

        self.label_field.set_active(-1)

        if self.ogrfs_obj:
            font = self.ogrfs_obj.parms['f'].value
            if not font:
                font = gview.get_preference('default-font')
            self.label_font.set_font_name(font)

            tparm = self.ogrfs_obj.parms['t']
            if tparm.role == 'field_name':
                self.label_field.set_active_text(tparm.value)
                if self.shape_obj:
                    text_value = self.shape_obj.get_property(tparm.value,'')
                else:
                    text_value = ''
            else:
                self.label_field.set_active_text("disabled")
                text_value = tparm.value

            color = self.ogrfs_obj.get_color((0.5, 1.0, 0.5, 1.0))
            self.label_color.set_color(color)

            if self.text_entry:
                self.text_entry.set_text(text_value)
                #self.text_entry.set_sensitive(tparm.role != 'field_name')

            if self.enable_offsets:
                dx = float(self.ogrfs_obj.get_parm('dx',0))
                self.x_offset.set_value(dx)
                dy = float(self.ogrfs_obj.get_parm('dy',0))
                self.y_offset.set_value(-dy)

        else:
            self.label_field.set_active_text("disabled")
            self.label_color.set_color( (0.5, 1.0, 0.5, 1.0) )
            if self.enable_offsets:
                self.x_offset.set_value(0.0)
                self.y_offset.set_value(0.0)

        self.updating = False

    ###########################################################################
    # Handle updates to the raw text.  Normally we just turn this over to
    # the generic label_change_cb(), but if we have field indirection in
    # operation, then we instead emit a signal offering the text to the
    # application to apply to the shape.

    def text_change_cb(self, *args):
        field_name = self.label_field.get_active_text()
        if not field_name or field_name == "disabled":
            self.label_change_cb()
            return

        #this shouldn't happen :)
        val = ''
        if self.text_entry:
            val = self.text_entry.get_text()

        self.notif('apply-text-to-field', field_name, val)
        self.gui_update()

    # Handle updates to the label font.
    def label_change_cb(self, *args):
        if self.layer is None or self.updating:
            return

        font = self.label_font.get_font_name()
        field_name = self.label_field.get_active_text()
        text_value = ''
        if self.text_entry:
            text_value = self.text_entry.get_text()

        gv_color = self.label_color.current_color
        color = gvogrfs.gv_to_ogr_color(gv_color)

        x_off = ''
        y_off = ''

        if self.enable_offsets:
            #handle user editing of the values in the x and y offsets.
            #something odd happens when listening to the 'changed' signal
            #of the spin box, the text value is difference from the float
            #value, even if the values should match.  This hack forces the
            #spin buttons to update the value and exits because this
            #will be called again right away
            x = self.x_offset.get_value()
            y = self.y_offset.get_value()
            sx = self.x_offset.get_text()
            sy = self.y_offset.get_text()
            try:
                if float(sx) != x:
                    self.x_offset.set_value(float(sx))
                    return
                if float(sy) != y:
                    self.y_offset.set_value(float(sy))
                    return
            except:
                return
            if 0.0 != x:
                x_off = 'dx:%s,' % x

            if 0.0 != y:
                y_off = 'dy:%s,' % -y

        if not field_name or field_name == "disabled":
            ogrfs = 'LABEL(%s%st:\"%s\",f:"%s",c:%s)' % (x_off, y_off, text_value, font, color)
        else:
            ogrfs = 'LABEL(%s%st:{%s},f:"%s",c:%s)' % (x_off, y_off, field_name, font, color)

        if ogrfs:
            self.ogrfs_obj = gvogrfs.OGRFeatureStylePart()
            self.ogrfs_obj.parse(ogrfs)
        else:
            self.ogrfs_obj = None

        self.gui_update()

        self.notif('ogrfs-changed')

    def set_sensitive(self, sensitive):
        self.text_entry.set_sensitive(sensitive)

    # Handle updates to the label font.
    def text_input(self, keyval):
        if self.text_entry and keyval > 31 and keyval < 256:
            new_string = self.text_entry.get_text()
            new_string += chr(keyval)
            self.text_entry.set_text(new_string)

        elif keyval == gtk.keysyms.Return:
            self.label_change_cb()

        # add support for delete, etc, later.

class GvSymbolStyle(gtk.VBox, Signaler):
    """
    A generic embeddable widget for controlling the SYMBOL ogr feature style
    on an arbitrary object (layer or shape).
    """
    def __init__(self, spacing=10, ogrfs_obj=None, layer=None):
        """
        Initialize the widget optionally setting the ogr object for which this
        represents the SYMBOL feature style.
        """
        gtk.VBox.__init__(self, spacing=spacing)
        self.set_border_width(10)
        self.create_gui()

        self.updating = False
        self.publish('ogrfs-changed')
        self.set_ogrfs(ogrfs_obj, layer)

    def create_gui(self):
        """
        create the widgets for this symbol
        """
        # for clarity
        shrink = gtk.SHRINK

        tips = gtk.Tooltips()
        table = gtk.Table()
        table.set_row_spacings(3)
        table.set_col_spacings(3)
        self.pack_start(table)

        # Point symbol
        label = pgu.Label(_("Symbol:"))
        table.attach(label, 0, 1, 0, 1, yoptions=shrink)

        combo = SymbolsCombo(action=self.symbol_change)
        combo.set_size_request(150, 30)
        table.attach(combo, 1, 4, 0, 1, yoptions=shrink)
        tips.set_tip(combo, _("Select point symbol"))
        self.symbol_type = combo

        #symbol color
        label = pgu.Label(_("Color:"))
        table.attach(label, 0, 1, 1, 2, yoptions=shrink)

        cb = ColorButton(color=(0.5, 1.0, 0.5, 1.0))
        cb.connect('color-set', self.color_change)
        table.attach(cb, 1, 2, 1, 2, xoptions=shrink, yoptions=shrink)
        tips.set_tip(cb, _("Click to change symbol color"))
        self.symbol_color = cb

        # Point size
        label = pgu.Label(_("Scale:"))
        table.attach(label, 2, 3, 1, 2, yoptions=shrink)

        spin_adjust = gtk.Adjustment(value=1.0, lower=0.2, upper=4.0, step_incr=0.1)
        spin = gtk.SpinButton(spin_adjust)
        spin.set_editable(True)
        spin.set_digits(1)
        spin.set_size_request(45, -1)
        spin.connect('changed', self.scale_change)
        table.attach(spin, 3, 4, 1, 2, xoptions=shrink, yoptions=shrink)
        tips.set_tip(spin, _("Set point size"))
        self.symbol_size = spin

    def gui_update(self, *args):
        """
        refresh the screen
        """
        #get the current values
        if self.updating:
            return

        self.updating = True

        sym = int(self.ogrfs_obj.parms['id'].value[8:9])
        color = gvogrfs.ogr_to_gv_color(self.ogrfs_obj.parms['c'].value)
        scale = float(self.ogrfs_obj.get_parm('s','1.0'))

        #update the widgets
        self.symbol_type.set_active(sym)
        self.symbol_color.set_color(color)
        self.symbol_size.set_value(scale)

        self.updating = False

    def set_ogrfs(self, ogrfs_obj, layer=None):
        """
        set the ogr feature specification from the shape object passed,
        or from the layer if the shape has none, or provide a default
        """
        if ogrfs_obj is None:
            ogrfs_obj = gvogrfs.OGRFeatureStylePart()
            ogrfs_obj.parse('SYMBOL(id:"ogr-sym-0",c:#88FF88)')

        self.ogrfs_obj = ogrfs_obj
        self.layer = layer
        self.gui_update()

    def color_change(self, widget, *args):
        gv_color = widget.get_color()
        parm = gvogrfs.OGRFeatureStyleParam()
        parm.parse('c:%s' % gvogrfs.gv_to_ogr_color(gv_color))
        self.ogrfs_obj.set_parm(parm)
        self.notif('ogrfs-changed')

    def scale_change(self, spin):
        """
        change the scale ... be careful as it may not exist beforehand
        """
        parm = gvogrfs.OGRFeatureStyleParam()
        parm.parse('s:%s' % spin.get_value())
        self.ogrfs_obj.set_parm(parm)
        self.notif('ogrfs-changed')

    def symbol_change(self, combo, *args):
        """
        change the symbol
        """
        parm = gvogrfs.OGRFeatureStyleParam()
        ogr_sym = combo.get_symbol()
        parm.parse('id:%s' % ogr_sym)
        self.ogrfs_obj.set_parm(parm)
        self.notif('ogrfs-changed')
