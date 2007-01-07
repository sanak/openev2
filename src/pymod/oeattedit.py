#!/usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Shape attribute display, and editing.
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
import gview
import gvselbrowser
import gvutils
from gvsignaler import *

def launch():
    try:
        gview.oeattedit.window.raise_()
        gview.oeattedit.show()
        gview.oeattedit.reconnect()
        gview.oeattedit.gui_update()
    except:
        try:
            # oeattedit has been created,
            # but not shown
            gview.oeattedit.show()
            gview.oeattedit.reconnect()
            gview.oeattedit.gui_update()
        except:    
            gview.oeattedit = OEAttEdit()
            gview.oeattedit.show()

    return gview.oeattedit

def launch_hidden():
    if not hasattr(gview,'oeattedit'):
        gview.oeattedit = OEAttEdit()

class OEAttEdit(gtk.Window,Signaler):

    def __init__(self):
        gtk.Window.__init__(self)

        self.set_title('Shape Attributes')
        gview.app.sel_manager.subscribe( 'selection-changed',
                                         self.gui_update )
        gview.app.sel_manager.subscribe( 'subselection-changed',
                                         self.gui_update )

        # signal for external tools to connect to
        self.publish('hidden')
        self.publish('shown')

        # self.text_contents = ''
        self.selected_shape = None
        self.layer = None
        self.create_gui()

        self.visibility_flag = 0
        self.gui_update()
        self.connect('delete-event', self.close)

    def show(self):
        gtk.Window.show(self)
        self.visibility_flag = 1
        Signaler.notify(self, 'shown')

    def close(self, *args):
        gview.app.sel_manager.unsubscribe( 'selection-changed',
                                           self.gui_update )
        gview.app.sel_manager.unsubscribe( 'subselection-changed',
                                           self.gui_update )
        self.hide()
        self.visibility_flag = 0
        Signaler.notify(self, 'hidden')

        return True

    def reconnect(self, *args):
        gview.app.sel_manager.subscribe( 'selection-changed',
                                         self.gui_update )
        gview.app.sel_manager.subscribe( 'subselection-changed',
                                         self.gui_update )

    def create_gui(self):
        box1 = gtk.VBox()
        self.add(box1)
        box1.show()

        self.selbrowser = gvselbrowser.GvSelBrowser()
        self.selbrowser.set_border_width(10)
        box1.pack_start( self.selbrowser, expand=False )

        box2 = gtk.VBox(spacing=10)
        box2.set_border_width(10)
        box1.pack_start(box2)
        box2.show()

        table = gtk.Table(2, 2)
        table.set_row_spacing(0, 2)
        table.set_col_spacing(0, 2)
        box2.pack_start(table)
        table.show()


        text_window = gtk.ScrolledWindow()
        text_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        text_buff = gtk.TextBuffer()
        text_view = gtk.TextView(text_buff)
        text_view.show()
        text_window.add(text_view)
        text_view.set_size_request(400, 100)
        text_view.set_editable(True)
        text_view.set_wrap_mode(gtk.WRAP_CHAR)
        text_window.show()

        table.attach(text_window, 0,1, 0,1)
        ##text_view.connect('activate', self.att_update_cb)
        text_view.connect('leave-notify-event', self.att_update_cb)
        self.text_view = text_view
        self.text_buff = text_buff


        separator = gtk.HSeparator()
        box1.pack_start(separator, expand=False)
        separator.show()

        box2 = gtk.VBox(spacing=10)
        box2.set_border_width(10)
        box1.pack_start(box2, expand=False)
        box2.show()

        # new field options
        box3 = gtk.HBox(spacing=10)
        box3.set_border_width(10)
        nf_frame = gtk.Frame('New field properties: type/width/precision')
        nf_frame.add(box3)
        self.new_field_width_entry = gtk.Entry(2)
        self.new_field_width_entry.set_text('20')
        self.new_field_width_entry.set_editable(True)        
        self.new_field_precision_entry = gtk.Entry(2)
        self.new_field_precision_entry.set_text('0')
        self.new_field_precision_entry.set_editable(False)
        self.new_field_precision_entry.set_sensitive(False)

        self.new_field_types = ('string','integer','float')
        self.new_field_type_menu = gvutils.GvOptionMenu(self.new_field_types, self.new_field_precision_cb)
        self.new_field_type_menu.set_history(0)
        box3.pack_start(self.new_field_type_menu)
        box3.pack_start(self.new_field_width_entry,expand=False,fill=False)
        box3.pack_start(self.new_field_precision_entry,expand=False,fill=False)
        box2.pack_start(nf_frame)
        nf_frame.show_all()

        button = gtk.Button("close")
        button.connect("clicked", self.close)
        box2.pack_start(button)
        button.set_flags(gtk.CAN_DEFAULT)
        button.grab_default()
        button.show()

    def new_field_precision_cb(self,*args):
        if self.new_field_types[self.new_field_type_menu.get_history()] == 'float':
            # precision is only relevant for float
            self.new_field_precision_entry.set_editable(True)
            self.new_field_precision_entry.set_sensitive(True)
        else:
            self.new_field_precision_entry.set_text('0')
            self.new_field_precision_entry.set_editable(False)
            self.new_field_precision_entry.set_sensitive(False)

    def gui_update(self,*args):
        text_contents = ''
        self.text_buff.set_text('')
        #self.text.freeze()
        #self.text.delete_text(0,-1)

        self.selected_shape = None

        try:
            self.layer = gview.app.sel_manager.get_active_layer()
            shapes = self.layer.get_parent()
            self.selected_shape = self.layer.get_subselected()
            properties = shapes[self.selected_shape].get_properties()
            for att_name in properties.keys():
                text_contents = text_contents + \
                        att_name + ': ' + properties[att_name] + '\n'
            self.text_buff.insert_at_cursor(text_contents)
            #self.text.insert_defaults(self.text_contents)
        except:
            pass

        #self.text.thaw()

    def att_update_cb(self,*args):
        #if self.text_contents == self.text.get_chars(0,-1):
        #    return

        if self.selected_shape is None:
            return

        shapes = self.layer.get_parent()
        shape = shapes[self.selected_shape]
        if shape is None:
            return

        shape = shape.copy()

        lines = self.text_buff.get_text(self.text_buff.get_bounds()).split('\n')
        #lines = string.split(self.text.get_chars(0,-1),'\n')
        for line in lines:
            tokens = line.split(':',1)
            if len(tokens) == 2:
                value = tokens[1].strip()
                shape.set_property(tokens[0],value)
                property_exists=0
                for cprop in shapes.get_schema():
                    if cprop[0] == tokens[0]:
                        property_exists=1
                if property_exists != 1:
                    ftype = self.new_field_types[self.new_field_type_menu.get_history()]

                    response = \
                       gvutils._message_box('Confirmation',
                         'Create new ' + ftype + '-type property ' + tokens[0] + '?' ,
                                            ('Yes','No'))
                    if response == 'Yes':
                        try:
                            fwidth = int(self.new_field_width_entry.get_text())
                        except:
                            gvutils.error('Field width must be an integer!')
                            continue

                        if ftype == 'float':
                            try:
                                fprec = int(self.new_field_width_entry.get_text())
                            except:
                                gvutils.error('Precision width must be an integer!')
                                continue
                        else:
                            fprec = 0

                        shapes.add_field(tokens[0],ftype,fwidth,fprec)

        shapes[self.selected_shape] = shape
        self.gui_update()

    def insert_text_cb(self,new_text,*args):
        if new_text[0] == '\n':
            self.att_update_cb()
            return False
