###############################################################################
# $Id$
#
# Project:  Python Gtk Utility Widgets
# Purpose:  Core PGU stuff, such as registering new PyGtk classes.
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

import gtk
import _gv

_name2cls = {}

def _obj2inst(obj):
    objname = _gv.gv_get_type_name(obj)
    if _name2cls.has_key(objname):
        return _name2cls[objname](_obj=obj)
    raise 'gtk type ' + objname + ' not found'

def gtk_register(name, class_obj):
    _name2cls[name] = class_obj

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
        if active > -1:
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
