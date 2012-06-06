###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Grid to edit shapefile attributes
# Author:   Mario Beauchamp, starged@gmail.com
#
# Originally developed by Paul Spencer for DM Solutions Group as cm_editgrid.py
# which was itself derived from Tool_ShapesGrid.py and then ported to GTK2 for
# CIETmap and then adapted for OpenEV and renamed gvshapesgrid.py by me(!)
#
###############################################################################
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

# temporary
def _(s):
    return s

import pygtk
pygtk.require('2.0')
import gtk
import pgu
from gvutils import error, warning
from gvgrid import Grid, SimpleGrid, DataSource
import gview
from gvsignaler import Signaler

class ShapesSource(DataSource):
    def __init__(self, data, fields):
        DataSource.__init__(self, data, fields)

    def __getitem__(self, n):
        """ Return n'th item from the sorted data. """
        if self.order is not None:
            n = self.order[n]
        elif self.subset is not None:
            n = self.subset[n]

        shp = self.data[n]
        items = [shp.get_property(f) for f in self.fields]

        return items

    def __setitem__(self, ii, values):
        shp = self.data[ii]
        props = {}
        for n,f in enumerate(self.fields):
            props[f] = values[n]

        shp.set_properties(props)

    def sort_on_column(self, col):
        """ Set up data to sort on column. """
        ftype = self.data.get_property('_field_type_%s' % (col+1))
        if ftype == 'integer':
            conv = int
            default = '-9'
        elif ftype == 'float':
            conv = float
            default = '-0.1'
        else:
            conv = str
            default = ''

        if self.subset:
            data = self.subset
        else:
            data = xrange(len(self))

        field = self.fields[col]
        values = [conv(self.data[n].get_property(field, default)) for n in data]
        DataSource.sort(self, values)

class FieldsSource(DataSource):
    def __init__(self, data):
        fields = [  _("Name"),
                    _("Type"),
                    _("Width"),
                    _("Precision")
                    ]
        DataSource.__init__(self, data.get_schema(), fields)

    def __getitem__(self, n):
        """ Return n'th item from the sorted data. """
        if self.order is not None:
            n = self.order[n]
        elif self.subset is not None:
            n = self.subset[n]

        fld = self.data[n]
        items = [f for f in fld]

        return items

    def __setitem__(self, ii, values):
        self.data[ii] = values

class AttributeEditor(gtk.Window, Signaler):
    def __init__(self, app=None):
        gtk.Window.__init__(self)
        self.set_title(_("Edit Vector Attributes"))

        self.app = app
        self.layer = None
        self.shapes = None
        self.viewarea = None
        self.layer_teardown_id = None
        self.selecting = False

        self.init_dialog()
        # popup menu items
        entries = ["Set subset/Selected","Set subset/Unselected",
                    "Set subset/All","New layer/Selected","New layer/Unselected"]

        self.grid.make_menu(entries, self.menu_clicked)

    def init_dialog(self):
        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(10)
        self.add(vbox)

        self.grid = SimpleGrid()
        self.grid.set_size_request(450,398)
        vbox.pack_start(self.grid)

        hbox = gtk.HButtonBox()
        vbox.pack_start(hbox, expand=False)
        self.schema_button = gtk.Button(_("Edit Columns"))
        self.schema_button.connect("clicked", self.edit_schema)
        hbox.pack_start(self.schema_button)

        rbutton = gtk.Button(_("Refresh Columns"))
##        rbutton.connect("clicked", self.refresh_columns)
        hbox.pack_start(rbutton, expand=False)
        rbutton.set_sensitive(False)

        hbox = gtk.HButtonBox()
        vbox.pack_start(hbox, expand=False)
        button = gtk.Button(stock=gtk.STOCK_CLOSE)
        button.connect("clicked", self.close)
        hbox.pack_start(button, expand=False)

        # Trap window close event
        self.connect('delete-event', self.close)

        self.app.sel_manager.subscribe('active-layer-changed', self.layer_update)
        self.grid.subscribe('selected-changed', self.selected_cb)
        self.show_all()
        # Only do actions when grid is visible
        self.active = False

    def show_cb(self, *args):
        # Activate the view that the grid was launched from
        # (confusing for user if launching the grid from
        # one view shows the vectors for another)
        for view in self.app.view_manager.view_list:
            if view == args[0]:
                self.app.view_manager.set_active_view(view)

        self.active = True
        self.show_all()
        self.present()
        self.layer_update()

    def close(self, *args):
        self.active = False
        self.layer_teardown_cb()
        self.hide()
        return True

    def layer_update(self, *args):
        # Disconnect from the old layer
        if not self.active:
            return

        self.layer_teardown_cb()

        layer = self.app.sel_manager.get_active_layer()
        if layer is None or isinstance(layer, gview.GvRasterLayer):
            return

        try:
            self.layer = layer
            self.viewarea = self.app.sel_manager.get_active_view()
            self.shapes = self.layer.get_parent()
            self.set_source(self.shapes)
            self.layer_teardown_id = self.layer.connect('teardown', self.layer_teardown_cb)
            self.app.sel_manager.subscribe('selection-changed', self.selection_update)
            self.selection_update()
        except:
            import sys, traceback
            last_type, last_value, exc_traceback = sys.exc_info()
            exp = traceback.format_exception(last_type, last_value, exc_traceback)
            for line in exp:
                print line

#            error(_('cm-editgrid-err-load'))

    def selection_update(self, *args):
        if self.selecting:
            return
        self.grid.set_selected(self.layer.get_selected())

    def selected_cb(self, *args):
        self.selecting = True
        self.layer.clear_selection()
        for selected in self.grid.get_selected():
            self.layer.select_shape(selected)
        self.layer.display_change()
        self.selecting = False

    def layer_teardown_cb(self, *args):
        self.grid.clear()
        if self.layer_teardown_id is not None:
            self.layer.disconnect(self.layer_teardown_id)
            self.layer_teardown_id = None
            self.app.sel_manager.unsubscribe('selection-changed', self.selection_update)
        self.viewarea = None
        self.layer = None
        self.shapes = None

    def set_source(self, source, titles=None, hidden=None):
        #trap setting to None or an invalid shapes object       
        if not source:
            return

        if not titles:
            titles = source.get_fieldnames()
        data = ShapesSource(source, titles)
        schema = source.get_schema()
        parms = self.grid.calculate_widths(titles, [f[2] for f in schema])
        types = [str]*len(titles)
        self.grid.set_data_source(data, titles, types, parms)

    def edit_schema(self, *args):
        """ Edit the shapes schema. """
        if self.shapes is None:
            return

        sch = SchemaDialog(self.shapes, self)

    def menu_clicked(self, text, row):
        rows = self.grid.get_selected()
        if not rows:
            return

        if "Unselected" in text:
            rows = filter(lambda i: i not in rows, range(len(self.shapes)))

        if text == "Set subset/All":
            self.grid.set_subset(None)
        elif "Set subset" in text:
            self.grid.set_subset(rows)
        elif "New layer" in text:
            name = text.split('/')[1]
            self.new_layer_selected(rows, name)

    def new_layer_selected(self, rows, name):
        """ Create new layer of only selected shapes """
        if not rows:
            warning('No rows selected- ignoring!')
            return

        newshps = gview.GvShapes(name=name)
        src = self.shapes
        if src is None:
            warning('No source layer found- ignoring!')
            return
        
        newshps.copy_fields(src.get_schema())

        for idx in rows:
            if src[idx] is not None:
                newshps.append(src[idx].copy())

        gview.undo_register(newshps)
        clayer = gview.GvShapesLayer(newshps)

        cview = self.app.new_view()
        cview.viewarea.add_layer(clayer)
        cview.viewarea.set_active_layer(clayer)

class SchemaDialog(gtk.Window):
    ftypes = ('string','integer','float')
    def __init__(self, shapes, shapesgridtool=None):
        gtk.Window.__init__(self)
        self.set_title(_("Schema"))
        self.ftype = 'string'
        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(5)
        self.add(vbox)

        self.grid = SimpleGrid()
        self.grid.set_size_request(257,200)
        self.grid.sortable = False
        self.shapes = shapes
        self.shapesgridtool = shapesgridtool
        vbox.pack_start(self.grid)

        # New field
        frame = gtk.Frame(_("Add Field"))
        vbox.pack_start(frame, expand=False)

        table = gtk.Table(rows=5, columns=3)
        table.set_row_spacings(5)
        table.set_col_spacings(5)
        table.set_border_width(10)
        frame.add(table)

        table.attach(gtk.Label(_("Name")),0,1,0,1)
        self.new_field_name_entry = gtk.Entry(10)
        self.new_field_name_entry.set_text('')
        self.new_field_name_entry.set_editable(True)
        table.attach(self.new_field_name_entry,1,2,0,1)

        table.attach(gtk.Label(_("Type")),0,1,2,3)
        self.new_field_width_entry = gtk.Entry(2)
        self.new_field_width_entry.set_text('20')
        self.new_field_width_entry.set_editable(True)
        table.attach(self.new_field_width_entry,1,2,2,3)

        table.attach(gtk.Label(_("Width")),0,1,3,4)
        self.new_field_precision_entry = gtk.Entry(2)
        self.new_field_precision_entry.set_text('0')
        self.new_field_precision_entry.set_editable(False)
        self.new_field_precision_entry.set_sensitive(False)
        table.attach(self.new_field_precision_entry,1,2,3,4)

        table.attach(gtk.Label(_("Precision")),0,1,1,2)
        self.new_field_type_menu = pgu.ComboText(strings=self.ftypes, action=self.new_field_precision_cb)
        self.new_field_type_menu.set_size_request(-1,24)
        table.attach(self.new_field_type_menu,1,2,1,2)

        button = gtk.Button(stock=gtk.STOCK_ADD)
        table.attach(button,0,2,4,5)
        button.connect("clicked", self.add_field)

        # Ability to delete fields?
        self.fill_grid()
##        self.grid.resize_to_default()
        hbox = gtk.HButtonBox()
        vbox.pack_end(hbox, expand=False)
        button = gtk.Button(stock=gtk.STOCK_CLOSE)
        button.connect("clicked", self.close)
        hbox.pack_start(button)
        self.show_all()

    def fill_grid(self):
        """ Get the schema and fill the grid """
        data = FieldsSource(self.shapes)
        types = [str]*4
        self.grid.set_data_source(data, data.fields, types)
        # name and type are not editable
        for col in self.grid.get_columns()[:2]:
            rend = col.get_cell_renderers()[0]
            rend.set_property('editable', False)
        self.grid.model.connect('row-changed', self.changed_field)

    def add_field(self, *args):
        """ Add field """
        sch = self.shapes.get_schema()
        name = self.new_field_name_entry.get_text()

        for item in sch:
            if item[0].lower() == name.lower():
                error(_("cm_editgrid56811109949513") % name)
                return

        ftype = self.ftype

        try:
            fwidth = int(self.new_field_width_entry.get_text())
        except:
            error(_("cm_editgrid56821109949513"))
            return

        if ftype == 'float':
            try:
                fprec = int(self.new_field_precision_entry.get_text())
            except:
                error(_("cm_editgrid56831109949513"))
                return
        else:
            fprec = 0

        self.shapes.add_field(name.upper(), ftype, fwidth, fprec)
        self.fill_grid()
        if self.shapesgridtool is not None:
            self.shapesgridtool.grid.add_column(name.upper(), ftype, fwidth)

    def changed_field(self, lst, path, iter):
        """ User changed a field """
        idx = path[0]
        n,t,w,p = lst[idx]
        self.shapes.set_property('_field_width_%s' % (idx+1), w)
        if lst.get_value(iter,1) == 'float':
            self.shapes.set_property('_field_precision_%s' % (idx+1), p)
        if self.shapesgridtool is not None:
            self.shapesgridtool.layer_update()

    def new_field_precision_cb(self, widget, *args):
        self.ftype = self.ftypes[widget.get_active()]
        if self.ftype == 'float':
            # precision is only relevant for float
            self.new_field_precision_entry.set_editable(True)
            self.new_field_precision_entry.set_sensitive(True)
        else:
            self.new_field_precision_entry.set_text('0')
            self.new_field_precision_entry.set_editable(False)
            self.new_field_precision_entry.set_sensitive(False)

    def close(self, *args):
        self.hide()
##        self.shapesgridtool.set_keep_above(True)
        self.destroy()
