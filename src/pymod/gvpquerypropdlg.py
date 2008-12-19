###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  GvPqueryLayer Properties Dialog
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

import gvvectorpropdlg
import gvutils
import pgucolorsel
import gvhtml

pq_prop_dialog_list = []

def LaunchPQueryPropDialog(layer):
    # Check list to see if dialog exists - make it visible
    for test_dialog in pq_prop_dialog_list:
        if test_dialog.layer == layer:
            test_dialog.update_gui()
            test_dialog.present()
            return test_dialog

    # Create new dialog if one doesn't exist already
    new_dialog = GvPQueryPropDialog(layer)
    pq_prop_dialog_list.append( new_dialog )
    return new_dialog

class GvPQueryPropDialog(gvvectorpropdlg.GvVectorPropDialog):

    def __init__(self, layer):
        gtk.Window.__init__(self)
        self.set_title('GView')
        self.layer = layer
        self.updating = False

        gvhtml.set_help_topic( self, "gvpquerypropdlg.html" )

        # create the general layer properties dialog
        self.create_notebook()
        self.create_pane1()

        if self.layer is not None:
            self.layer.connect('display-change', self.refresh_cb)

        # Setup Object Drawing Properties Tab
        self.pane2 = gtk.VBox(spacing=10)
        self.pane2.set_border_width(10)
        self.notebook.append_page( self.pane2, gtk.Label('Draw Styles'))

        vbox = gtk.VBox(spacing=10)
        self.pane2.add(vbox)

        # Create Color control.
        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(gtk.Label('Color:'),expand=False)
        self.point_color = \
                 pgucolorsel.ColorControl('Point Color',
                                          self.color_cb,'_point_color')
        box.pack_start(self.point_color)

        # Point size
        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(gtk.Label('Point Size:'),expand=False)
        self.point_size = gtk.Combo()
        self.point_size.set_popdown_strings(
            ('1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '15', '20') )
        self.point_size.entry.connect('changed', self.point_size_cb)
        box.pack_start(self.point_size,expand=False)

        # Coordinate
        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(gtk.Label('Coordinate:'),expand=False)

        self.coord_om = gvutils.GvOptionMenu(
            ('Off','Raster Pixel/Line','Georeferenced','Geodetic (lat/long)'),
            self.set_coordinate_mode)
        box.pack_start(self.coord_om,expand=False)

        # Raster Value
        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(gtk.Label('Pixel Value:'),expand=False)

        self.pixel_mode_om = \
            gvutils.GvOptionMenu(('On','Off'), self.set_pixel_mode)
        box.pack_start(self.pixel_mode_om,expand=False)

        self.update_gui()

        self.show_all()

    # Initialize GUI state from underlying object state.
    def update_gui(self):

        # GTK2 PORT PENDING - GTK_OBJECT_DESTROYED should no longer be used;
        #   if necessary should respond to destroy signal.
        #if self.flags( DESTROYED ) > 0:
        #    return
        #

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

        self.set_color_or_default('_point_color', self.point_color)
        # point size
        self.point_size.entry.delete_text(0,-1)
        if self.layer.get_property('_point_size') is None:
            self.point_size.entry.insert_text('6')
        else:
            self.point_size.entry.insert_text(
                self.layer.get_property('_point_size'))

        # coordinate mode
        mode = self.layer.get_property( '_coordinate_mode' )
        if mode is None:
            self.coord_om.set_history(2)
        elif mode == 'off':
            self.coord_om.set_history(0)
        elif mode == 'raster':
            self.coord_om.set_history(1)
        elif mode == 'latlong':
            self.coord_om.set_history(3)
        else:
            self.coord_om.set_history(2)

        # pixel mode
        mode = self.layer.get_property( '_pixel_mode' )
        if mode is None or mode != 'off':
            self.pixel_mode_om.set_history(0)
        else:
            self.pixel_mode_om.set_history(1)

        self.updating = False

    # Dialog closed, remove references to python object
    def close( self, widget, args ):
        pq_prop_dialog_list.remove(self)

    def set_coordinate_mode(self, om):
        if self.coord_om.get_history() == 0:
            self.layer.set_property( '_coordinate_mode', 'off')
        elif  self.coord_om.get_history() == 1:
            self.layer.set_property( '_coordinate_mode', 'raster')
        elif  self.coord_om.get_history() == 2:
            self.layer.set_property( '_coordinate_mode', 'georef')
        elif  self.coord_om.get_history() == 3:
            self.layer.set_property( '_coordinate_mode', 'latlong')
        self.layer.display_change()

    def set_pixel_mode(self, om):
        if om.get_history() == 0:
            self.layer.set_property( '_pixel_mode', 'on')
        else:
            self.layer.set_property( '_pixel_mode', 'off')
        self.layer.display_change()

