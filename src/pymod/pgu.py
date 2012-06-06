###############################################################################
# $Id$
#
# Project:  Python Gtk Utility Widgets
# Purpose:  Was core PGU stuff, now is much more.
# Author:   Frank Warmerdam, warmerda@home.com
# Comments: Merged with pguentry, pguprogress, pgutextarea and pgutogglebutton
#           and removed pgu prefixes. Added many more widgets...
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

import pygtk
pygtk.require('2.0')
import gtk

# temporary
def _(s):
    return s

# we may not need this anymore...
def gtk_register(name, class_obj):
    pass

class Label(gtk.Label):
    """Label is a left aligned gtk.Label with added capabilities

    txt - string
    markup - an optional Pango text markup language as the text

    """
    def __init__(self, txt='', markup=''):
        gtk.Label.__init__(self, txt)
        self.set_alignment(0,0.5)
        if markup:
            self.set_markup(markup)

class ComboText(gtk.ComboBox):
    """ComboText is a direct replacement for gtk.Combo with added capabilities

    strings - a list or tuple of strings
    model - an optional gtk.ListStore to use as model
    action - an optional action connecting to the changed signal

    """
    def __init__(self, strings=[], model=None, action=None):
        if model is None:
            model = gtk.ListStore(str)
        gtk.ComboBox.__init__(self, model)
        cell = gtk.CellRendererText()
        self.pack_start(cell, True)
        self.add_attribute(cell, 'text', 0)
        self.show_all()
        self.cur_selection = 0
        self.action = action
        if strings:
            self.set_popdown_strings(strings)
        if action:
            gtk.ComboBox.connect(self, 'changed', action)

    def __getattr__(self, attr):
        if attr in self.__dict__:
            return getattr(self, attr)
        return getattr(gtk.ComboBox, attr)

    def connect(self, signal='changed', cb=None, *args):
        gtk.ComboBox.connect(self, signal, cb, *args)

    def get_active_text(self):
        model = self.get_model()
        active = self.get_active()
        if active < 0:
            return None
        return model[active][0]

    def set_active_text(self, text):
        model = self.get_model()
        for n,item in enumerate(model):
            if item[0] == text:
                self.set_active(n)

    # Old style GtkCombo API
    def set_popdown_strings(self, strings):
        lst = self.get_model()
        lst.clear()
        for s in strings:
            lst.append((s,))
        self.set_active(0)

    def get_text(self):
        return self.get_active_text()

    def set_text(self, text):
        self.set_active_text(text)

class LabelComboText(gtk.HBox):
    """LabelComboText is a ComboText with a label

    label - a string 
    strings - a list or tuple of strings
    model - an optional gtk.ListStore to use as model
    action - an optional action connecting to the changed signal

    """
    def __init__(self, label, strings=[], model=None, action=None):
        gtk.HBox.__init__(self, spacing=10)
        self.label = Label(label)
        self.pack_start(self.label, expand=False)
        self.combo = ComboText(strings, model, action)
        self.pack_end(self.combo, expand=False)
        self.show_all()

    def __getattr__(self, attr):
        if attr in self.__dict__:
            return getattr(self, attr)
        return getattr(self.combo, attr)

class ComboBox(gtk.ComboBox):
    """ComboBox is a gtk.ComboBox with added capabilities

    model - a gtk.ListStore to use as model
    action - an optional action connecting to the changed signal

    Current implementation is limited to 2 columns of gtk.gdk.Pixbuf and strings.

    """
    def __init__(self, model, action=None):
        gtk.ComboBox.__init__(self, model)
        for n,item in enumerate(model[0]):
            if isinstance(item, str):
                self.txt = gtk.CellRendererText()
                self.txt.set_property('editable', False)
                self.pack_start(self.txt)
                self.add_attribute(self.txt, 'text', n)
            elif isinstance(item, gtk.gdk.Pixbuf):
                self.pix = gtk.CellRendererPixbuf()
                self.pix.set_property('xalign', 0)
                self.pack_start(self.pix)
                self.add_attribute(self.pix, 'pixbuf', n)

        if action:
            gtk.ComboBox.connect(self, 'changed', action)

    def connect(self, signal='changed', cb=None, *args):
        gtk.ComboBox.connect(self, signal, cb, *args)

    def __getattr__(self, attr):
        if attr in self.__dict__:
            return getattr(self, attr)
        return getattr(self.combo, attr)

    def get_active_text(self):
        model = self.get_model()
        active = self.get_active()
        if active < 0:
            return

        item = model[active]
        if isinstance(item[0], str):
            return item[0]
        else:
            return item[1]

    def set_text(self, text):
        idx = 0
        model = self.get_model()
        if len(model[0]) == 2:
            idx = 1

        for n,item in enumerate(model):
            if item[idx] == text:
                self.set_active(n)

class Entry(gtk.Entry):
    """Entry is a gtk.Entry with added capabilities."""
    def __init__(self):
        gtk.Entry.__init__(self)

    def set_auto_complete(self, model, length=2):
        """Enable text completion using model."""
        completion = gtk.EntryCompletion()
        completion.set_model(model)
        completion.set_text_column(0)
        completion.set_minimum_key_length(length)
        self.set_completion(completion)

class LabelEntry(gtk.HBox):
    """LabelEntry is an Entry with a Label.

    label - the Label text
    width - the Entry width
    text - optional text to initialize Entry with

    """
    def __init__(self, label, width=200, text=None):
        gtk.HBox.__init__(self, spacing=10)
        self.label = Label(label)
        self.pack_start(self.label, expand=False)
        self.entry = Entry()
        self.entry.set_size_request(width,-1)
        self.pack_end(self.entry, expand=False)
        if text:
            self.set_text(text)
        self.show_all()

    def __getattr__(self, attr):
        if attr in self.__dict__:
            return getattr(self, attr)
        return getattr(self.entry, attr)

    def set_size_request(self, w, h):
        self.entry.set_size_request(w, h)

    def connect(self, signal, action, *args):
        self.entry.connect(signal, action, *args)

class ComboBoxEntry(gtk.HBox):
    """ComboBoxEntry is a gtk.ComboBoxEntry with added capabilities

    strings - a list or tuple of strings
    action - an optional action connecting to the changed signal

    Current implementation is limited to 2 columns of gtk.gdk.Pixbuf and strings.

    """
    def __init__(self, strings=[], action=None):
        """Initialize the combo box, default strings can be passed.
        """
        gtk.HBox.__init__(self)
        self.combo = gtk.combo_box_entry_new_text()
        self.add(self.combo)
        self.set_popdown_strings(strings)
        self.entry = self.combo.child
        if action:
            self.combo.connect('changed', action)

    def connect(self, signal, cb, *args):
        self.combo.connect(signal, cb, *args)

    def __getattr__(self, attr):
        if attr in self.__dict__:
            return getattr(self, attr)
        return getattr(self.combo, attr)

    def set_active_text(self, text):
        model = self.combo.get_model()
        for n,item in enumerate(model):
            if item[0] == text:
                self.combo.set_active(n)

    def set_auto_complete(self, on=True, model=None):
        if on:
            completion = gtk.EntryCompletion()
            if model is None:
                model = self.get_model()
            completion.set_model(model)
            completion.set_text_column(0)
            completion.set_minimum_key_length(2)
            self.entry.set_completion(completion)
        else:
            self.entry.get_completion().set_model(None)

    # Old style GtkCombo API
    def set_popdown_strings(self, strings):
        """set the strings to display."""
        self.combo.get_model().clear()
        for s in strings:
            self.combo.append_text(s)
        self.combo.set_active(0)
        self.strings = strings # is this still necessary?

    def set_text(self, text):
        self.set_active_text(text)

    def get_text(self):
        """Return the text associated with the currently selected item
        """
        return self.entry.get_text()

    def select_string(self, string=''):
        """Cause an item to be selected by a string value
        """
        self.set_active_text(string)

# simulate the old gtk.CList behaviour
class CList(gtk.TreeView):
    """CList is a gtk.TreeView used to more or less simulate a gtk.CList

    titles - list of strings for columns titles
    types - list of columns data types. Only str, int, float and gtk.gdk.Pixbuf
            are supported at this time.

    """
    def __init__(self, titles=[' '], types=[]):
        if not types: # we assume strings if no types list
            types = [str] * len(titles)
        model = gtk.ListStore(*types)
        gtk.TreeView.__init__(self, model)
        for n,title in enumerate(titles):
            if types[n] in (str, int, float):
                rend = gtk.CellRendererText()
                column = gtk.TreeViewColumn(title, rend, text=n)
                self.append_column(column)
            elif types[n] == gtk.gdk.Pixbuf:
                rend = gtk.CellRendererPixbuf()
                column = gtk.TreeViewColumn(title, rend, pixbuf=n)
                self.append_column(column)

        self.list = model
        self.menu_actions = []

    def __getattr__(self, attr):
        if attr in self.__dict__:
            return getattr(self, attr)
        return getattr(self.list, attr)

    def __getitem__(self, n):
        if len(self.list):
            return self.list[n]

    def __len__(self):
        return len(self.list)

    def connect(self, signal, action, *args):
        if signal == 'select-row':
            selection = self.get_selection()
            selection.connect('changed', action, *args)
        else:
            gtk.TreeView.connect(self, signal, action, *args)

    def make_menu(self, entries, action, *args):
        self.menu = gtk.Menu()
        for entry in entries:
            item = gtk.MenuItem(entry)
            self.menu.append(item)
            item.connect('activate', self.do_menu_action, *args)
            item.show()
        self.connect('button-press-event', self.pop_menu)
        self.menu_action = action

    def do_menu_action(self, action):
        self.menu_action(text, self.selected_row)

    def pop_menu(self, menu, event):
        self.emit_stop_by_name('button-press-event')
        path = self.get_path_at_pos(int(event.x), int(event.y))
        if path:
            p,t,x,y = path
            row = p[0]
        else:
            row = len(self)-1
        self.select_row(row, -1)
        menu.popup(None, None, None, event.button, event.time)
        return False

    def get_renderer(self, col):
        column = self.get_column(col)
        return column.get_cell_renderers()[0]

    def select_row(self, row, col=0):
        selection = self.get_selection()
        selection.select_path((row,))
        self.selected_row = row

    def get_selected_row(self, event):
        path = self.get_path_at_pos(int(event.x), int(event.y))
        if path:
            row = path[0][0]
            return row

    def set_text(self, row, col, text):
        self.list[row][col] = text

# simulate the old gtk.Tree behaviour
class Tree(gtk.TreeView):
    """Tree is a gtk.TreeView used to more or less simulate a gtk.Tree

    titles - list of strings for columns titles
    types - list of columns data types. Only str, int, float and gtk.gdk.Pixbuf
            are supported at this time.

    """
    def __init__(self, titles=[' '], types=[]):
        if not types:
            types = [str] * len(titles)
        model = gtk.TreeStore(*types)
        gtk.TreeView.__init__(self, model)

        for n,title in enumerate(titles):
            if types[n] in (str, int, float):
                rend = gtk.CellRendererText()
                column = gtk.TreeViewColumn(title, rend, text=n)
            elif types[n] == gtk.gdk.Pixbuf:
                rend = gtk.CellRendererPixbuf()
                column = gtk.TreeViewColumn(title, rend, pixbuf=n)
            else:
                raise ValueError, "unsupported type %s in pgu.Tree" % types[n]
            self.append_column(column)

        self.list = model
        self.menu_actions = []

    def __getattr__(self, attr):
        if attr in self.__dict__:
            return getattr(self, attr)
        return getattr(self.list, attr)

    def get_selected_row(self, event):
        path = self.get_path_at_pos(int(event.x), int(event.y))
        if path:
            return path[0][0]

    def get_selected_value(self, event, col=0):
        path = self.get_path_at_pos(int(event.x), int(event.y))
        if path:
            iter = self.get_iter(path[0])
            if iter:
                return self.get_value(iter, col)

# unused for now...
class RadioButtonsGroup(gtk.HBox):
    def __init__(self, title=None, choices=[], action=None):
        gtk.HBox.__init__(self, spacing=15)
        if title:
            label = Label(title)
            self.pack_start(label)
        but = gtk.RadioButton(label=choices[0])
        but.set_active(True)
        if action:
            but.connect('toggled', action)
        self.pack_start(but, expand=False)
        for choice in choices[1:]:
            rbut = gtk.RadioButton(label=choice, group=but)
            if action:
                rbut.connect('toggled', action)
            self.pack_start(rbut, expand=False)

        self.group = but.get_group()
        self.group.reverse()
        self.show_all()

    def get_group(self):
        return self.group

    def get_active(self):
        for rb in self.group:
            if rb.get_active():
                return rb

    def get_choice(self):
        rb = self.get_active()
        return rb.get_children()[0].get_text()

    def set_choice(self, choice):
        for rb in self.group:
            text = rb.get_children()[0].get_text()
            if text == choice:
                rb.set_active(True)
                return

    def get_index(self):
        rb = self.get_active()
        return self.group.index(rb)

# previously in pgutogglebutton.py
class ToggleButton(gtk.ToggleButton):
    """a widget for displaying toggled state (on/off)."""
    def __init__(self, pix_on='ck_on_l', pix_off='ck_off_l'):
        gtk.ToggleButton.__init__(self)

        self.pix_on = gtk.image_new_from_stock(pix_on, gtk.ICON_SIZE_BUTTON)
        self.pix_off = gtk.image_new_from_stock(pix_off, gtk.ICON_SIZE_BUTTON)
        self.add(self.pix_off)

        self.active_pix = self.pix_off
        self.set_size_request(20, 20)

        self.connect('toggled', self.expose)
        self.connect('expose-event', self.expose)
        self.show()

    def expose(self, *args):
        active_pix = (self.pix_off,self.pix_on)[self.get_active()]

        if active_pix != self.active_pix:
            self.remove(self.active_pix)
            self.active_pix = active_pix
            self.active_pix.show()
            self.add(self.active_pix)

class CheckButton(gtk.HBox):
    def __init__(self, pix_on='ck_on_l', pix_off='ck_off_l', label=None):
        gtk.HBox.__init__(self)
        self.set_spacing(10)
        self.toggle = gtk.ToggleButton()
        if label:
            lbl = gtk.Label(label)
            self.pack_start(lbl, expand=False)

        self.pack_end(self.toggle, expand=False)
        self.pix_on = gtk.image_new_from_stock(pix_on, gtk.ICON_SIZE_BUTTON)

        self.pix_off = gtk.image_new_from_stock(pix_off, gtk.ICON_SIZE_BUTTON)
        self.toggle.add(self.pix_off)

        self.active_pix = self.pix_off
        w,h = self.pix_off.size_request()
        self.toggle.set_size_request(w, h)

        self.connect('toggled', self.expose)
        self.connect('expose-event', self.expose)
        self.toggle.show()
        self.show()

    def expose(self, *args):
        if not self.flags() & gtk.REALIZED:
            return

        if self.get_active():
            active_pix = self.pix_on
        else:
            active_pix = self.pix_off

        if active_pix != self.active_pix:
            self.toggle.remove(self.active_pix)
            self.active_pix = active_pix
            self.active_pix.show()
            self.toggle.add(self.active_pix)

    def connect(self, signal, cb, *args):
        self.toggle.connect(signal, cb, *args)

    def toggled(self):
        self.toggle.toggled()

    def get_active(self):
        return self.toggle.get_active()

    def set_active(self, active):
        self.toggle.set_active(active)

# replacement for pguTextArea
class TextArea(gtk.TextView):
    def __init__(self):
        gtk.TextView.__init__(self)
        self.set_border_window_size(gtk.TEXT_WINDOW_TOP, 5)
        self.set_border_window_size(gtk.TEXT_WINDOW_BOTTOM, 5)
        self.set_border_window_size(gtk.TEXT_WINDOW_RIGHT, 10)
        self.set_border_window_size(gtk.TEXT_WINDOW_LEFT, 10)
        self.selecting = False

    def append_text(self, text):
        buffer = self.get_buffer()
        iter = buffer.get_end_iter()
        buffer.insert(iter, text)

    def scroll_to(self, line):
        buffer = self.get_buffer()
        iter = buffer.get_iter_at_line(line)
        self.scroll_to_iter(iter, 0.1)

    def scroll_to_end(self):
        buffer = self.get_buffer()
        mark = buffer.get_insert()
        iter = buffer.get_end_iter()
        if buffer.get_iter_at_mark(mark) != iter:
            buffer.move_mark(mark, iter)
        self.scroll_mark_onscreen(mark)

    def page_up(self):
        pass

    def page_down(self):
        pass

# previously in pguprogress.py
class ProgressDialog(gtk.Dialog):
    def __init__(self, title="Progress", cancel=False):
        gtk.Dialog.__init__(self)
        self.set_title(title)
        self.min = 0.0
        self.max = 1.0

        self.message = _("complete")
        self.cancelled = False

        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(10)
        self.vbox.pack_start(vbox)

        label = gtk.Label(" 0% " + self.message)
        label.set_alignment(0, 0.5)
        vbox.pack_start(label)
        self.label = label

        pbar = gtk.ProgressBar()
        pbar.set_size_request(200, 20)
        vbox.pack_start(pbar)

        if cancel:
            button = gtk.Button(stock=gtk.STOCK_CANCEL)
            self.cancel = button
            button.connect('clicked', self.CancelCB)
            self.action_area.pack_start(button)

        self.pbar = pbar
        self.show_all()

    def CancelCB(self, *args):
        self.cancelled = True

    def Reset(self):
        self.cancelled = False

    def SetRange(self, min, max):
        self.min = min
        self.max = max

    def SetDefaultMessage(self, message):
        self.message = message

    def ProgressCB(self, complete, message='', *args):
        self.complete = self.min + (self.max-self.min) * complete
        if not message:
            message = self.message

        self.label.set_text('%s%% %s' % (int(complete*100), message))

        self.pbar.set_fraction(complete)
        while gtk.events_pending():
            gtk.main_iteration(False)

        if self.cancelled:
            return False
        else:
            return True
