#!/usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Label Edit Tool.
# Author:   Frank Warmerdam, warmerdam@pobox.com
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
import gview
import gvselbrowser
import pgucolorsel
import gvutils
import gvogrfs
import gvogrfsgui
import pgufont
import traceback

def launch(interactive=False):
    try:
        gview.gvlabeledit.window.raise_()
    except:
        gview.gvlabeledit = GvLabelEdit(interactive = interactive)

    return gview.gvlabeledit


class GvLabelEdit(gtk.Window):

    def __init__(self, interactive=False, default_ogrfs = None):
        gtk.Window.__init__(self)
        self.set_title('Label Edit Tool')
        gview.app.sel_manager.subscribe( 'active-layer-changed',
                                         self.gui_update )
        gview.app.sel_manager.subscribe( 'selection-changed',
                                         self.gui_update )
        gview.app.sel_manager.subscribe( 'subselection-changed',
                                         self.gui_update )

        if default_ogrfs is None:
            font =pgufont.XLFDFontSpec()
            default_font = gview.get_preference('label-font')
            if default_font is None:
                font.set_font_part('Family', 'Sans')
                font.set_font_part('Size', 12)
            else:
                font.parse_font_spec(default_font)

            color = gview.get_preference('label-color')

            if color is None:
                color = "#88FF88"
            else:
                try:
                    color = color.replace("(", "")
                    color = color.replace(")", "")
                    r, g, b, a = color.split(",")
                    r = float(r)
                    g = float(g)
                    b = float(b)
                    a = float(a)
                    color = ( r, g, b, a )
                    color = gvogrfs.gv_to_ogr_color( color )
                except:
                    color = "#88FF88"
            default_ogrfs = 'LABEL(t:"",f:"%s",c:%s)' % (font, color)


        self.default_ogrfs = default_ogrfs
        self.selected_shape = None
        self.layer = None
        self.interactive = interactive
        self.create_gui()
        self.show()

        self.edit_mode = 0

        self.view = gview.app.view_manager.get_active_view()
        self.key_sig = self.view.connect('key-press-event', self.key_press_cb)
        self.connect('delete-event', self.close)

        self.gui_update()

    def close(self, window, event):
        gview.app.sel_manager.unsubscribe( 'selection-changed',
                                           self.gui_update )
        gview.app.sel_manager.unsubscribe( 'subselection-changed',
                                           self.gui_update )
        gview.app.sel_manager.unsubscribe( 'active-layer-changed',
                                           self.gui_update )
        #remember to disconnect from the view's key press event
        self.view.disconnect(self.key_sig)

    def create_gui(self):
        box1 = gtk.VBox(spacing=10)
        box1.set_border_width(10)
        self.add(box1)
        box1.show()

        #######################################################################
        # Create a control box for the text related widgets.
        self.text_frame = gtk.Frame('Text Style')
        box1.pack_start(self.text_frame,expand=False)

        self.label_style = gvogrfsgui.GvLabelStyle(text_entry = True,
                                                   label_field = False,
                                                   interactive = self.interactive)
        self.label_style.subscribe('ogrfs-changed', self.label_ogrfs_cb)
        self.label_style.subscribe('apply-text-to-field', self.text_apply_cb)
        self.text_frame.add(self.label_style)
        self.text_frame.show()

    def set_default_ogrfs(self, prototype):
        if prototype is None or len(prototype) == 0:
            return

        try:
            ogrfs_obj = gvogrfs.OGRFeatureStylePart()
            ogrfs_obj.parse( prototype )
        except:
            return

        text_parm = gvogrfs.OGRFeatureStyleParam()
        text_parm.parse('t:""')
        ogrfs_obj.set_parm( text_parm )

        self.default_ogrfs = ogrfs_obj.unparse()

    def text_apply_cb(self, widget, field, value ):
        try:
            self.layer = gview.app.sel_manager.get_active_layer()
            shapes = self.layer.get_parent()
            self.selected_shape = self.layer.get_subselected()
            shape_obj = shapes[self.selected_shape]
        except:
            self.selected_shape = -1
            shape_obj = None

        if shape_obj is None:
            return False

        shape_obj = shape_obj.copy()
        shape_obj.set_property( field, value )
        shapes[self.selected_shape] = shape_obj

        self.gui_update()

        return False

    def label_ogrfs_cb(self, *args):
        self.edit_mode = 0

        try:
            shape_obj = (self.layer.get_parent())[self.selected_shape]
        except:
            print traceback.print_exc()
            shape_obj = None

        if shape_obj is None:
            return False

        if self.label_style.ogrfs_obj is None:
            ogrfs = ''
        else:
            ogrfs = self.label_style.ogrfs_obj.unparse()
            self.set_default_ogrfs( ogrfs )

        style = gvogrfs.OGRFeatureStyle()
        try:
            #the shape may or may not have a property already.
            style.parse(shape_obj._gv_ogrfs)
        except:
            style.parse(self.layer.get_property('_gv_ogrfs'))

        style.parse_part(ogrfs)

        shape_obj = shape_obj.copy()
        shape_obj._gv_ogrfs = style.unparse()
        (self.layer.get_parent())[self.selected_shape] = shape_obj

        return False

    def gui_update(self,*args):
        self.edit_mode = 0

        try:
            self.layer = gview.app.sel_manager.get_active_layer()
            shapes = self.layer.get_parent()
            self.selected_shape = self.layer.get_subselected()
            shape_obj = shapes[self.selected_shape]
        except:
            self.selected_shape = -1
            shape_obj = None

        if shape_obj is None:
            # add stuff to grey out interface.
            self.label_style.set_sensitive(False)
            return
        else:
            self.label_style.set_sensitive(True)

        try:
            ogrfs = shape_obj._gv_ogrfs
        except:
            ogrfs = None

        change_enabled = 1
        if ogrfs is None:
            ogrfs = self.layer.get_property('_gv_ogrfs')
            if ogrfs is not None:
                change_enabled = 0

        # Parse the ogrfs.
        try:
            ogrfs_obj = gvogrfs.OGRFeatureStyle()
            ogrfs_obj.parse( ogrfs )
            label_obj = ogrfs_obj.get_part('LABEL')
            if label_obj is not None:
                self.set_default_ogrfs( label_obj.unparse() )
        except:
            ogrfs_obj = None

        # Display type specific information.
        if ((ogrfs_obj is None) or
            ((label_obj is not None) and label_obj.tool_name == 'LABEL')):
            self.label_style.set_ogrfs( label_obj, layer = self.layer,
                                        shape_obj = shape_obj )
            self.label_style.show()

    def key_press_cb(self, view, event, *args):
        if self.edit_mode:
            if event.keyval == gtk.keysyms.Return:
                self.edit_mode = 0
                return False

            self.label_style.text_input(event.keyval)

            return False

        #######################################################################
        # We only want to respond to <ENTER> keystrokes.
        if event.keyval != gtk.keysyms.Return:
            # return
            return False

        if self.layer is None:
            # return
            return False

        self.view = gview.app.view_manager.get_active_view()
        self.new_label()

        # TEMP
        return False

    def new_label(self):
        #######################################################################
        # Create a new point shape at the current pointer location.

        pointer_loc = self.view.get_pointer()
        new_label = gview.GvShape(type=gview.GVSHAPE_POINT)
        new_label.set_node(pointer_loc[0], pointer_loc[1])
        new_label._gv_ogrfs = self.default_ogrfs

        #######################################################################
        # Attach it to the current layer, and make it selected.

        shapes = self.layer.get_parent()
        id = shapes.append(new_label)
        self.layer.clear_selection()
        self.layer.select_shape(id)

        self.edit_mode = 1

        self.window.raise_()
        self.label_style.text_entry.grab_focus()


