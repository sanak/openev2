###############################################################################
# $Id$
#
# Project:  CIETmap/OpenEV
# Purpose:  General purpose grid widget.
# Author:   Mario Beauchamp, starged@gmail.com
#
# Comments: This was inspired by EasyGrid and adapted for CIETmap/OpenEV.
#           It is a replacement for pgugrid.
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

import gtk
import gobject
from gvsignaler import Signaler

class DataSource(object):
    def __init__(self, data, fields):
        self.fields = fields
        self.order = None
        self.sort_order = None
        self.filter = None
        self.subset = None
        self.data = data

    def __len__(self):
        if self.filter is not None:
            return len(self.filter)
        elif self.subset is not None:
            return len(self.subset)

        return len(self.data)
    
    def __getslice__(self, start, end):
        return [self[ii] for ii in xrange(start, end)]

    def append(self, value):
        for n,field in enumerate(self.fields):
            self.data[field].append(value[n])

    def remove(self, n):
        for field in self.fields:
            del self.data[field][n]

    def index(self, item):
        if self.order:
            n = self.order[item]
        elif self.subset:
            n = self.subset[item]
        else:
            n = item
        return n

    def rank(self, item):
        if self.order is not None:
            return self.order.index(item)
        elif self.subset is not None:
            return self.subset.index(item)
        else:
            return item

    def sort(self, values):
        if self.subset:
            indexes = self.subset
        else:
            indexes = range(len(values))
        sort_data = zip(values, indexes)
        sort_data.sort()

        self.sort_order = [x[1] for x in sort_data]

        self._set_order()

    def reverse(self):
        """ Reverse current sort. """
        self.sort_order.reverse()
        self._set_order()

    def remove_sort(self):
        """ Remove current sort. """
        self.sort_order = None
        self._set_order()

    def _set_order(self):
        """ Internal helper method to set order. """
        if self.filter is None:
            self.order = self.sort_order
        else:
            if self.sort_order is not None:
                visible = Set(self.filter)
                self.order = [x for x in self.sort_order if x in visible]
            else:
                self.order = self.filter

    def apply_filters(self, filters):
        if len(filters) == 0:
            self.filter = None
            self._set_order()
            return

        data = self.data
        fields = self.fields

        self.filter = []
        for ix in xrange(len(data)):
            good = True
            for col, values in filters.iteritems():
                if data[ix][fields.get(col, col)] not in values:
                    good = False
                    break
            if good:
                self.filter.append(ix)

        self._set_order()

    def apply_subset(self, subset):
        self.subset = subset

class SimpleGrid(gtk.ScrolledWindow, Signaler):
    def __init__(self, editable=True):
        gtk.ScrolledWindow.__init__(self)
        self.set_shadow_type(gtk.SHADOW_IN)
        self.set_policy(gtk.POLICY_AUTOMATIC,gtk.POLICY_AUTOMATIC)
        self.editable = editable
        self.sortable = True

        self.tree = gtk.TreeView()
        self.tree.set_rules_hint(True)
        self.add(self.tree)

        self.tree.connect('button-press-event', self.on_button_press_event)
##        self.tree.connect('key_press_event', self.on_key_press_event)

        selection = self.get_selection()
        selection.set_mode(gtk.SELECTION_MULTIPLE)
        selection.connect('changed', self.on_selection_changed)
        self.selecting = False
        self.updating = False

        self.current_sort = None, None
        self.model = None
        self.data = None
        self.menu = None
        self.menu_action = None
        self.row_changed_id = None

        self.publish('selected-changed')

    def __getattr__(self, name):
        """ Delegate anything we can't handle to the treeview. """
        return getattr(self.tree, name)

    def attach_data(self, data=None):
        if data is None:
            data = self.data
        else:
            self.data = data
        self.load_store()

    def set_data_source(self, data, titles, types, parms=None):
        if parms is None:
            parms = [{}] * len(data[0])

        # Remove existing columns MB: check if necessary
        for col in self.get_columns():
            self.remove_column(col)
        
        self.model = gtk.ListStore(*types)

        # Set up columns
        for mix, ctype in enumerate(types):
            column = self.set_up_column(mix, titles[mix], ctype, parms[mix])
            self.append_column(column)

            if self.sortable:
                column.set_clickable(True)
                column.connect('clicked', self.sort_on_column, mix)

        # Add a dummy final column to take up spare space
        dummy_renderer = gtk.CellRendererText()
        dummy_renderer.set_property('text', '')
        padder = gtk.TreeViewColumn('', dummy_renderer)
        padder.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        padder.set_fixed_width(1)
        self.append_column(padder)
        self.set_fixed_height_mode(('width' in parms[0]))

        # Attach model to the tree
        self.set_editable(self.editable)
        self.attach_data(data)

    def set_up_column(self, col, colname, ctype, keyw):
        """ Return a column with renderer that is suitable.

        :param col: not sure we need this.
        :param keyw: dictionary with info about the column.
        """

        if ctype == 'toggle':
            renderer = gtk.CellRendererToggle()
            renderer.connect('toggled', self.toggle_toggled, col)
            column = gtk.TreeViewColumn(colname, renderer, active=col)
        else:
            renderer = gtk.CellRendererText()
            renderer.connect('edited', self.edited_cell, col)
            column = gtk.TreeViewColumn(colname, renderer, text=col)
            column.set_alignment(0.5)

        width = keyw.get('width')
        if width is not None:
            column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
            column.set_fixed_width(width)

        if ctype in (str,int,float):
            justify = keyw.get('justify', 'centre')
            if justify == 'left':
                renderer.set_property('xalign', 0.0)
            elif justify == 'centre':
                renderer.set_property('xalign', 0.5)
            elif justify == 'right':
                renderer.set_property('xalign', 1.0)

        return column

    def load_store(self):
        self.set_model(None)
        if self.editable:
            self.model.handler_block(self.row_changed_id)
        if len(self.model) == 0:
            for row in self.data:
                self.model.append(row)
        else:
            for x,row in enumerate(self.data):
                self.model[x] = row
        if self.editable:
            self.model.handler_unblock(self.row_changed_id)
        self.set_model(self.model)
        self.set_sort_indicator()

    def set_subset(self, subset=None):
        """Sets an array of shape indexes that constitute a displayable
        subset of the shapes

        subset - list of integer values

        If selected is None or an empty list, then all records will be
        displayed.
        """
        self.data.apply_subset(subset)
##        if subset:
##            nrows = len(subset)
##        else:
##            nrows = len(self.data)
##        self.table_size = min(self.MAX_TABLE_SIZE, nrows)
        self.model.clear()
        self.load_store()

    def clear(self):
        if self.data:
            del self.data
            self.data = None

    def sort_on_column(self, col, column):
        """ Sort data on column """
        self.set_model(None)
        current_col, sort_type = self.current_sort
        if current_col != col:
            sort_type = None
            if current_col is not None:
                current_col.set_sort_indicator(False)

        if sort_type is None:
            self.data.sort_on_column(column)
            self.current_sort = col, gtk.SORT_ASCENDING
        elif sort_type == gtk.SORT_ASCENDING:
            self.data.reverse()
            self.current_sort = col, gtk.SORT_DESCENDING
        else:
            self.data.remove_sort()
            self.current_sort = None, None
            col.set_sort_indicator(False)

        self.load_store()
        self.set_sort_indicator()

    def set_sort_indicator(self):
        col, sort_type = self.current_sort
        if sort_type is not None:
            col.set_sort_indicator(True)
            col.set_sort_order(sort_type)

    def on_button_press_event(self, view, event):
        if event.button == 3 and self.menu:
            self.menu.popup(None, None, None, event.button, event.time)
            return True

        return False

    def on_selection_changed(self, selection):
        if self.selecting:
            return
        self.notif('selected-changed')

    def clear_selection(self):
        self.selecting = True
        selection = self.get_selection()
        selection.unselect_all()
        self.selecting = False

    def get_selected(self):
        selection = self.get_selection()
        lst,rows = selection.get_selected_rows()
        return [self.data.index(r[0]) for r in rows]

    def set_selected(self, selected):
        self.clear_selection()
        selection = self.get_selection()
        self.selecting = True
        for sel in selected:
            row = (self.data.rank(sel),)
            selection.select_path(row)
        self.selecting = False

    def set_editable(self, enable=True):
        self.editable = enable
        columns = self.get_columns()[:-1] # we do not want the padder column
        for col in columns:
            rend = col.get_cell_renderers()[0]
            rend.set_property('editable', enable)
        if enable:
            self.row_changed_id = self.model.connect('row-changed', self.on_row_changed)
        elif self.row_changed_id:
            self.model.disconnect(self.row_changed_id)

    def get_font_width(self):
        from pango import PIXELS
        ctx = self.get_pango_context()
        font_desc = ctx.get_font_description()
        metrics = ctx.get_metrics(font_desc)
        return PIXELS(metrics.get_approximate_char_width())

    def calculate_widths(self, titles, widths):
        parms = []
        font_width = self.get_font_width() + 2
        for ix, title in enumerate(titles):
            parm = {}
            width = font_width * max(len(title), widths[ix]) + 20
            parm['width'] = min(255,width)
            parms.append(parm)

        return parms

    def add_column(self, title, ctype=str, fwidth=8):
        # add field to data and reset model
        self.set_model(None)
        self.data.fields.append(title)
        ncols = len(self.data.fields)
        types = [str]*ncols
        self.model = gtk.ListStore(*types)

        width = (self.get_font_width()+2) * max(len(title), fwidth) + 20
        column = self.set_up_column(ncols-1, title, str, {'width':width})
        if self.sortable:
            column.set_clickable(True)
            column.connect('clicked', self.sort_on_column, ncols-1)
        self.insert_column(column, ncols-1)
        self.set_editable(self.editable)
        self.load_store()

    def add_row(self, values=None):
        self.data.append(values)
        self.model.append(values)

    def del_row(self, row):
        iter = self.model.get_iter((row,))
        if iter:
            self.model.remove(iter)
            recid = self.data.index(row)
            del self.data[recid]

    def edited_cell(self, cell, path, value, column):
        lst = self.model
        iter = lst.get_iter_from_string(path)
        ctype = lst.get_column_type(column)
        if ctype == gobject.TYPE_INT:
            value = int(value)
        elif ctype in (gobject.TYPE_FLOAT,gobject.TYPE_DOUBLE):
            value = float(value)
        if value != lst.get_value(iter, column):
            lst.set_value(iter, column, value)

    def on_row_changed(self, lst, path, iter):
        recid = self.data.index(path[0])
        self.data[recid] = lst[path[0]]

    def on_columns_changed(self, *args):
        print 'columns-changed', args

    def make_menu(self, entries, action=None):
        menu = gtk.Menu()
        for entry in entries:
            item = gtk.MenuItem(entry)
            menu.append(item)
            item.connect('activate', self.do_menu_action)
            item.show()
        if action is None:
            self.menu_action = self.menu_clicked
        else:
            self.menu_action = action
        self.set_popup_menu(menu)

    def do_menu_action(self, item):
        text = item.get_child().get_text()
        lst,rows = self.get_selection().get_selected_rows()
        if text:
            self.menu_action(text, rows[0][0])

    def set_popup_menu(self, menu):
        self.menu = menu

    def menu_clicked(self, *args):
        return False

class Grid(gtk.Frame, Signaler):
    MAX_TABLE_SIZE = 5000
    def __init__(self, editable=True, buffered=False):
        gtk.Frame.__init__(self)
        self.editable = editable
        self.sortable = True
        self.filterable = False
        self.buffered = buffered
        self.set_shadow_type(gtk.SHADOW_IN)
        vbox = gtk.VBox()
        self.add(vbox)
        hbox = gtk.HBox()
        vbox.pack_start(hbox)

        self.tree = gtk.TreeView()
        hbox.pack_start(self.tree)
        self.set_rules_hint(True)

        self.tree.connect('move_cursor', self.on_move_cursor)
        self.tree.connect('size-allocate', self.on_size_allocate)
##        self.tree.connect('columns-changed', self.on_columns_changed)
##        self.tree.connect('cursor_changed', self.on_cursor_changed)
        self.tree.connect('button-press-event', self.on_button_press_event)
        self.tree.connect('scroll-event', self.on_scroll_event)
##        self.tree.connect('key_press_event', self.on_key_press_event)

        selection = self.get_selection()
        selection.set_mode(gtk.SELECTION_MULTIPLE)
        if buffered:
            selection.set_select_function(self.on_selection)
        else:
            selection.connect('changed', self.on_selection_changed)
        self.selected = []
        self.selecting = False
        self.add_to_selection = False
        self.updating = False

        self.filters = {}
        self.current_sort = None, None
        self.first_table_row = 0
        self.first_table_col = 0
        self.row_height = 25
        self.nrows = 0
        self.ncols = 5
        self.model = None
        self.data = None
        self.menu = None
        self.menu_action = None
        self.row_changed_id = None

        self.vsb = gtk.VScrollbar()
        hbox.pack_end(self.vsb, expand=False)
        self.hsb = gtk.HScrollbar()
        vbox.pack_start(self.hsb, expand=False)

        self.vadj = self.vsb.get_adjustment()
        self.vadj.connect('value-changed', self.on_vadj_value_changed)
        if not buffered:
            self.tree.get_vadjustment().connect('value-changed', self.on_vadj_value_changed)

        self.hadj = self.hsb.get_adjustment()
        self.hadj.step_increment = 1.0
        self.hadj.connect('value-changed', self.on_hadj_value_changed)

        # hack to scroll the proper way...
        import os
        if os.name == 'nt':
            self.scroll_to = self.point_scroll
        else:
            self.scroll_to = self.cell_scroll

        self.publish('selected-changed')

    def __getattr__(self, name):
        """ Delegate anything we can't handle to the treeview. """
        return getattr(self.tree, name)

    def attach_data(self, data=None):
        self.selected = []
        if data is None:
            data = self.data
        else:
            self.data = data
        self.table_size = min(self.MAX_TABLE_SIZE, len(data))
        self.load_store(0)
        self.ncols = min(5, len(data.fields))
        self.cursor_pos = 0.0
        #self.set_cursor(0)

        self.hadj.upper = len(data.fields)
        self.hadj.page_increment = self.ncols

    def set_data_source(self, data, titles, types, parms=None):
        if parms is None:
            parms = [{}] * len(data[0])

        # Remove existing columns MB: check if necessary
        for col in self.get_columns():
            self.remove_column(col)
        
        self.model = gtk.ListStore(*types)

        # Set up columns
        for mix, ctype in enumerate(types):
            column = self.set_up_column(mix, titles[mix], ctype, parms[mix])
            self.append_column(column)

            if self.sortable:
                column.set_clickable(True)
                column.connect('clicked', self.sort_on_column, mix)

        # Add a dummy final column to take up spare space
        dummy_renderer = gtk.CellRendererText()
        dummy_renderer.set_property('text', '')
        padder = gtk.TreeViewColumn('', dummy_renderer)
        padder.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
        padder.set_fixed_width(1)
        self.append_column(padder)
        self.set_fixed_height_mode(('width' in parms[0]))

        # Attach model to the tree
        self.set_editable(self.editable)
        self.attach_data(data)
        # scroll to 1rst col of 1rst row 
        self.vadj.set_value(0.0)
        self.hadj.set_value(0.0)

    def set_up_column(self, col, colname, ctype, keyw):
        """ Return a column with renderer that is suitable.

        :param col: not sure we need this.
        :param keyw: dictionary with info about the column.
        """

        if ctype == 'toggle':
            renderer = gtk.CellRendererToggle()
            renderer.connect('toggled', self.toggle_toggled, col)
            column = gtk.TreeViewColumn(colname, renderer, active=col)
        else:
            renderer = gtk.CellRendererText()
            renderer.connect('edited', self.edited_cell, col)
            column = gtk.TreeViewColumn(colname, renderer, text=col)
            column.set_alignment(0.5)

        width = keyw.get('width')
        if width is not None:
            column.set_sizing(gtk.TREE_VIEW_COLUMN_FIXED)
            column.set_fixed_width(width)

##        render_parms = {'scale': keyw.get('scale', 1.0)}
        if ctype in (str,int,float):
            justify = keyw.get('justify', 'centre')
            if justify == 'left':
                renderer.set_property('xalign', 0.0)
            elif justify == 'centre':
                renderer.set_property('xalign', 0.5)
            elif justify == 'right':
                renderer.set_property('xalign', 1.0)

##        elif ctype in (int, long):
##            render_parms['format'] = '#,##0'
##            column.set_cell_data_func(renderer, self.render_number,
##                                      render_parms)
##            renderer.set_property('xalign', 1.0)
##            
##        elif ctype == float:
##            precision =  keyw.get('precision', 0)
##            render_parms['format'] = '(#,##0.' + ('0' * precision) + ')'
##            column.set_cell_data_func(renderer, self.render_number,
##                                      render_parms)
##            renderer.set_property('xalign', 1.0)

        return column

    def load_store(self, pos):
        self.first_table_row = pos
        if self.buffered:
            lastpos = min(pos+self.table_size, len(self.data))
        else:
            lastpos = len(self.data)

        self.set_model(None)
        if self.editable:
            self.model.handler_block(self.row_changed_id)

        if len(self.model) == 0:
            for row in self.data[pos:lastpos]:
                self.model.append(row)
        else:
            for x,row in enumerate(self.data[pos:lastpos]):
                self.model[x] = row

        if self.editable:
            self.model.handler_unblock(self.row_changed_id)

        self.set_model(self.model)
        self.set_sort_indicator()
        if self.buffered:
            self.update_selection()

    def update_selection(self):
        self.updating = True
        ftr = self.first_table_row
        selection = self.get_selection()
        for row in xrange(ftr, ftr+self.table_size):
            idx = self.data.index(row)
            if idx in self.selected:
                selection.select_path((row-ftr,))
        self.updating = False

    def set_subset(self, subset=None):
        """Sets an array of shape indexes that constitute a displayable
        subset of the shapes

        subset - list of integer values

        If selected is None or an empty list, then all records will be
        displayed.
        """
        self.data.apply_subset(subset)
        if subset:
            nrows = len(subset)
        else:
            nrows = len(self.data)
        self.table_size = min(self.MAX_TABLE_SIZE, nrows)
        self.model.clear()
        self.load_store(0)

    def clear(self):
        if self.data:
            del self.data
            self.data = None

    def on_vadj_value_changed(self, adj):
        """ Call-back for the adjustment for the vertical scrollbar. """
        if self.updating:
            return
        value = adj.get_value()
        self.updating = True
        if adj == self.vadj:
            self.cursor_pos = value
            self.scroll_view(value)
        else:
            value = min(self.vadj.upper, value)
            self.cursor_pos = value
            self.vadj.value = value
        self.updating = False

    def on_hadj_value_changed(self, adj):
        """ Call-back for the adjustment for the horizontal scrollbar. """
        row = int(self.vadj.get_value()/self.row_height)
        value = adj.get_value()
        self.first_table_col = int(value)
        col = self.get_column(self.first_table_col)
        r = self.get_cell_area((row,), col)
        self.scroll_to_point(r.x, -1)

    def scroll_view(self, value):
        """ Called when vertical scrollbar value has changed.

        If we are lucky, current pos is already visible.

        Otherwise we need to re-load the store and/or adjust our
        view into that store.
        """

        ftr = self.first_table_row
        pos = int(round(value/self.row_height,0))

        if not self.buffered:
            self.scroll_to(pos)
            return
        # Don't move beyond the end of the data
        last_visible_pos = min(pos + self.nrows-1, len(self.data)-1)

        if pos < ftr or last_visible_pos >= (ftr + self.table_size):
            # Need a new buffer of data
            if pos < ftr:
                # want pos, nrows above bottom of table => pos-ftr == ts-nrows
                ftr = max(0, pos+self.nrows-self.table_size)
                pos -= ftr
            else:
                # want pos at the top, if possible
                ftr = min(pos, len(self.data)-self.table_size)
                if value >= self.vadj.upper-self.vadj.page_size:
                    pos = self.table_size - self.nrows
                else:
                    pos = 0

            self.load_store(ftr)
            self.scroll_to(pos)
        else:
            self.scroll_to(pos-ftr)

    def cell_scroll(self, pos):
        self.scroll_to_cell((pos,), use_align=True)

    def point_scroll(self, pos):
        self.scroll_to_point(-1, pos*(self.row_height+2))

    def sort_on_column(self, col, column):
        """ Sort data on column """
        self.set_model(None)
        current_col, sort_type = self.current_sort
        if current_col != col:
            sort_type = None
            if current_col is not None:
                current_col.set_sort_indicator(False)

        if sort_type is None:
            self.data.sort_on_column(column)
            self.current_sort = col, gtk.SORT_ASCENDING
        elif sort_type == gtk.SORT_ASCENDING:
            self.data.reverse()
            self.current_sort = col, gtk.SORT_DESCENDING
        else:
            self.data.remove_sort()
            self.current_sort = None, None
            col.set_sort_indicator(False)

        self.load_store(self.first_table_row)
        self.set_sort_indicator()

    def set_sort_indicator(self):
        col, sort_type = self.current_sort
        if sort_type is not None:
            col.set_sort_indicator(True)
            col.set_sort_order(sort_type)

    def on_scroll_event(self, view, event):
        value = self.vadj.get_value()
        if event.direction == gtk.gdk.SCROLL_DOWN:
            value = min(value+self.row_height, self.vadj.upper-self.vadj.page_size)
        elif event.direction == gtk.gdk.SCROLL_UP:
            value = max(0, value-self.row_height)
        self.vadj.set_value(value)

    def on_button_press_event(self, view, event):
        x = int(event.x)
        y = int(event.y)
        time = event.time
        current = view.get_path_at_pos(x, y)

        button = event.button
        if event.state & gtk.gdk.CONTROL_MASK:
            self.add_to_selection = True

        if button == 3 and self.menu:
            self.menu.popup(None, None, None, button, event.time)
            return True

        return False

    def on_move_cursor(self, tree, movetype, direction):
        """ Call-back for keys which move the cursor in the view. """
        if not self.buffered:
            return False
        sb_value = self.vadj.get_value()
        path = tree.get_cursor()[0]

        if path is None:
            return False

        current_tr = path[0]

        # See what type of move this is.
        if movetype == gtk.MOVEMENT_DISPLAY_LINES:
            offset = direction
        elif movetype == gtk.MOVEMENT_PAGES:
            offset = (self.nrows-2) * direction
        elif movetype == gtk.MOVEMENT_BUFFER_ENDS:
            if direction < 0:
                offset = -(self.first_table_row + current_tr)
            else:
                offset = len(self.data) - (self.first_table_row + current_tr)
        else:
            # If we can't tell it is probably a horizontal movement
            return False


        # Work out new row (in the store) that we should show.
        new_tr = current_tr + offset
        self.cursor_pos += self.row_height*offset
        if new_tr >= self.table_size or new_tr < self.first_table_row:
            self.vadj.set_value(self.cursor_pos)

        return False

    def apply_filter_for_column_and_path(self, col, path):
        self.grab_focus()
        self.set_cursor(path, col, 0)

        row = self.model.get_iter(path)
        name = col.get_title()
        column = self.column_lookup[col]

        value = self.data[path[0] + self.first_table_row]
        filters = [value[column]]

        self.set_model(None)
        self.model.clear()

        current_filters = self.filters.get(column, [])

        if current_filters == filters:
            del self.filters[column]
        else:
            self.filters[column] = filters
            
        self.data.apply_filters(self.filters)
        
        self.attach_data(self.data)
        self.set_model(self.model)

    def get_pagesize(self):
        """ Return top and bottom rows in view and nrows. """
        if self.model is None:
            return 0,0,0,0,0
        
        r = self.get_visible_rect()
        top = self.get_path_at_pos(2, 2)
        bottom = self.get_path_at_pos(r.width-2, r.height-1)

        if top is not None:
            top = top[0][0]
        else:
            top = self.first_table_row
            
        if bottom is not None:
            lastcol = bottom[1]
            bottom = bottom[0][0]
        else:
            # bottom of view is beyond the store
            bottom = self.first_table_row + len(self.model)
            lastcol = self.get_column(len(self.data.fields))

        nrows = 1 + bottom - top
        col0 = self.get_column(0)
        row_height = self.get_cell_area((0,), col0).height + col0.get_property('spacing')
        return top, bottom, nrows, lastcol, row_height

    def on_size_allocate(self, *args):
        if self.tree.window is None:
            return 0,0,0,0,0
        top, bottom, nrows, lastcol, row_height = self.get_pagesize()
##        if lastcol.get_title() == '':
##            self.hsb.hide()
##        else:
##            self.hsb.show()
        page_size = row_height * (nrows + 1)
        self.vadj.upper = (len(self.data)+2) * row_height
        self.vadj.step_increment = row_height
        self.vadj.page_increment = page_size - row_height
        self.vadj.page_size = page_size
##        w,h = self.hsb.get_size_request()
##        self.hsb.set_size_request(w-h,h)

##        if self.table_size < nrows:
##            print 'nrows: ',self.nrows
##            self.vsb.hide()
##        else:
##            self.vsb.show()

        self.nrows = nrows
        self.row_height = row_height

    def on_selection(self, path):
        if self.selecting:
            return
        if self.updating:
            return True

        selection = self.get_selection()
        row = self.first_table_row + path[0]
        idx = self.data.index(row)
        self.selecting = True
        if selection.path_is_selected(path):
            selection.unselect_path(path)
            if idx in self.selected:
                self.selected.remove(idx)
        else:
            selection.select_path(path)
            if idx not in self.selected:
                if self.add_to_selection:
                    self.selected.append(idx)
                else:
                    self.selected = [idx]
        self.add_to_selection = False
        self.selecting = False
        self.notif('selected-changed')
        return True

    def on_selection_changed(self, selection):
        if self.selecting:
            return
        self.notif('selected-changed')

    def clear_selection(self):
        self.selecting = True
        selection = self.get_selection()
        selection.unselect_all()
        self.selected = []
        self.selecting = False

    def get_selected(self):
        if self.buffered:
            return self.selected
        else:
            selection = self.get_selection()
            lst,rows = selection.get_selected_rows()
            return [self.row_to_idx(r[0]) for r in rows]

    def set_selected(self, selected):
        self.clear_selection()
        if self.buffered:
            self.selected = selected[:]
            self.update_selection()
        else:
            selection = self.get_selection()
            self.selecting = True
            for sel in selected:
                row = (self.idx_to_row(sel),)
                selection.select_path(row)
            self.selecting = False

    def set_editable(self, enable=True):
        self.editable = enable
        columns = self.get_columns()[:-1] # we do not want the padder column
        for col in columns:
            rend = col.get_cell_renderers()[0]
            rend.set_property('editable', enable)
        if enable:
            self.row_changed_id = self.model.connect('row-changed', self.on_row_changed)
        elif self.row_changed_id:
            self.model.disconnect(self.row_changed_id)

    def get_font_width(self):
        from pango import PIXELS
        ctx = self.get_pango_context()
        font_desc = ctx.get_font_description()
        metrics = ctx.get_metrics(font_desc)
        return PIXELS(metrics.get_approximate_char_width())

    def calculate_widths(self, titles, widths):
        parms = []
        font_width = self.get_font_width() + 2
        for ix, title in enumerate(titles):
            parm = {}
            width = font_width * max(len(title), widths[ix]) + 20
            parm['width'] = min(255,width)
            parms.append(parm)

        return parms

    def add_column(self, title, ctype=str, fwidth=8):
        # add field to data and reset model
        self.set_model(None)
        self.data.fields.append(title)
        ncols = len(self.data.fields)
        types = [str]*ncols
        self.model = gtk.ListStore(*types)

        width = (self.get_font_width()+2) * max(len(title), fwidth) + 20
        column = self.set_up_column(ncols-1, title, str, {'width':width})
        if self.sortable:
            column.set_clickable(True)
            column.connect('clicked', self.sort_on_column, ncols-1)
        self.insert_column(column, ncols-1)
        self.set_editable(self.editable)
        self.load_store(self.first_table_row)

    def add_row(self, values=None):
        self.data.append(values)
        self.model.append(values)

    def del_row(self, row):
        iter = self.model.get_iter((row,))
        if iter:
            self.model.remove(iter)
            recid = self.row_to_idx(row)
            del self.data[recid]

    def edited_cell(self, cell, path, value, column):
        lst = self.model
        iter = lst.get_iter_from_string(path)
        ctype = lst.get_column_type(column)
        if ctype == gobject.TYPE_INT:
            value = int(value)
        elif ctype in (gobject.TYPE_FLOAT,gobject.TYPE_DOUBLE):
            value = float(value)
        if value != lst.get_value(iter, column):
            lst.set_value(iter, column, value)

    def on_row_changed(self, lst, path, iter):
        recid = self.row_to_idx(path[0])
        self.data[recid] = lst[path[0]]

    def on_columns_changed(self, *args):
        print 'columns-changed', args

    def row_to_idx(self, row):
        row += self.first_table_row
        return self.data.index(row)

    def idx_to_row(self, idx):
        return self.data.rank(idx) + self.first_table_row

    def make_menu(self, entries, action=None):
        menu = gtk.Menu()
        for entry in entries:
            item = gtk.MenuItem(entry)
            menu.append(item)
            item.connect('activate', self.do_menu_action)
            item.show()
        if action is None:
            self.menu_action = self.menu_clicked
        else:
            self.menu_action = action
        self.set_popup_menu(menu)

    def do_menu_action(self, item):
        text = item.get_child().get_text()
        lst,rows = self.get_selection().get_selected_rows()
        if text:
            self.menu_action(text, rows[0][0])

    def set_popup_menu(self, menu):
        self.menu = menu

    def menu_clicked(self, *args):
        return False
