###############################################################################
# $Id$
#
# Project:  OpenEV / CIETmap
# Purpose:  GUI component to show the current list of selected objects, and
#           to control a single sub-selection out of that set. 
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

import os
if 'CIETMAP_HOME' in os.environ:
    import cview as gview
else:
    import gview

import pygtk
pygtk.require('2.0')
import gtk
from gvutils import create_stock_button
from gvsignaler import Signaler

class GvSelBrowser(gtk.VBox):
    def __init__(self, spacing=10):
        gtk.VBox.__init__(self, spacing=spacing)
        self.updating = False

        self.sel_manager = gview.app.sel_manager

        self.sel_manager.subscribe('active-layer-changed', self.update_gui)
        self.sel_manager.subscribe('selection-changed', self.update_gui)
        self.sel_manager.subscribe('subselection-changed', self.update_gui)

        hbox = gtk.HBox(spacing=3)
        self.pack_start(hbox, expand=False)
        self.hbox = hbox

        hbox.pack_start(gtk.Label('Shape:'), expand=False)

        self.oid_tb = gtk.Entry(max=7)
        self.oid_tb.connect('activate', self.oid_cb)
        self.oid_tb.connect('focus-out-event', self.oid_cb)
        hbox.pack_start(self.oid_tb)

        left_button = create_stock_button(gtk.STOCK_GO_BACK, self.cycle_down)
        hbox.pack_start(left_button, expand=False)
        left_button.set_tooltip_text('Cycle Selection Down')

        self.n_of_n_label = gtk.Label('XXXX of XXXX')
        hbox.pack_start(self.n_of_n_label)

        right_button = create_stock_button(gtk.STOCK_GO_FORWARD, self.cycle_up)
        right_button.set_tooltip_text('Cycle Selection Up')
        hbox.pack_start(right_button, expand=False)

        hbox = gtk.HBox(spacing=3)
        self.pack_start(hbox)
        self.layer_label = gtk.Label('XXXXXXXXXXXXXXXXXXXXXXXXXXX')
        self.layer_label.set_alignment(0,0.5)
        hbox.pack_start(self.layer_label, expand=False)

        self.connect('unrealize', self.close)

        self.update_gui()
        self.show_all()

    def close(self, *args):
        self.sel_manager.unsubscribe('active-layer-changed', self.update_gui)
        self.sel_manager.unsubscribe('selection-changed', self.update_gui)
        self.sel_manager.unsubscribe('subselection-changed', self.update_gui)

    def update_gui(self, *args):
        self.updating = True
        layer = self.sel_manager.get_active_layer()
        if layer is None:
            self.layer_label.set_text('Layer: <none selected>')
        else:
            self.layer_label.set_text('Layer: ' + layer.name)

        try:
            layer = self.sel_manager.get_active_layer()
            selected = layer.get_selected()
            subsel = layer.get_subselected()
        except:
            self.n_of_n_label.set_text('0 of 0')
            self.oid_tb.set_text('')
            self.updating = False
            return

        self.oid_tb.set_text(str(subsel))

        index_of = self.get_sel_index(subsel, selected)

        label = '%d of %d' % (index_of+1, len(selected))
        self.n_of_n_label.set_text(label)

        self.updating = False

    def oid_cb(self, *args):
        if self.updating:
            return

        try:
            new_oid = int(self.oid_tb.get_text())
            layer = self.sel_manager.get_active_layer()
            selected = layer.get_selected()
            if new_oid in selected:
                layer.subselect_shape(new_oid)
            else:
                layer.clear_selection()
                if new_oid >= 0:
                    layer.select_shape(new_oid)

            layer.display_change()
        except:
            pass

    def cycle_down(self, *args):
        try:
            layer = self.sel_manager.get_active_layer()
            selected = layer.get_selected()
        except:
            return

        index_of = self.get_sel_index(layer.get_subselected(), selected)
        if index_of > 0:
            layer.subselect_shape(selected[index_of-1])

    def cycle_up(self, *args):
        try:
            layer = self.sel_manager.get_active_layer()
            selected = layer.get_selected()
        except:
            return

        index_of = self.get_sel_index(layer.get_subselected(), selected)
        if index_of < len(selected)-1:
            layer.subselect_shape(selected[index_of+1])

    def get_sel_index(self, subsel, selected):
        index_of = 0
        while index_of < len(selected) and selected[index_of] != subsel:
            index_of += 1

        if index_of >= len(selected):
            return -1
        else:
            return index_of

class GvSelectionManager(Signaler):
    """
    Convenient manager for view, layer, and shape selection tracking.

    The GvSelectionManager provides a single object which can be easily
    used to track changes in current view, layer, shape selection and shape
    sub-selection.  This is mainly useful because adding and removing
    callbacks to individual layers and views is a hassle.

    This class is normally accessed as "gview.app.sel_manager", and the
    object instance is normally created by the openev.py startup.  The
    object publishes the following "gvsignaler.py" style signals.  Use
    the "subscribe" method to add a callback.

    active-view-changed -- The current application view has changed (as
                           understood by ViewManager).

    active-layer-changed -- The current layer of the active view (as returned
                            by GvViewArea.active_layer()) has changed, possibly
                            as a result of the current view changing.

    selection-changed -- The shape selection on the current layer (as
                         GvShapeLayer.get_selected()) has changed, possibly
                         as a result of a change of active layer or view.
                         Clearing selection, and selecting non-GvShapeLayers
                         can result in a selection-changed.

    subselection-changed -- The item within the current selection has
                            changed, possible as the result of a change in
                            selection, active layer or active view.

    """

    def __init__(self, view_manager):
        self.view_manager = view_manager
        self.view_manager.subscribe('active-view-changed', self.view_change)

        self.view = self.view_manager.get_active_view()
        if self.view:
            self.view_cb_id = self.view.connect('active-changed', self.layer_change)
        else:
            self.view_cb_id = None

        self.layer = None
        self.layer_selcb_id = None
        self.layer_sselcb_id = None

        self.sel_len = 0
        self.ssel = -1
        self.ssel_layer = None

        self.publish('active-view-changed')
        self.publish('active-layer-changed')
        self.publish('selection-changed')
        self.publish('subselection-changed')

        self.layer_change()

    def view_change(self, *args):
        if self.view == self.view_manager.get_active_view():
            return

        if self.view_cb_id:
            self.view.disconnect(self.view_cb_id)
            self.view_cb_id = None

        self.view = self.view_manager.get_active_view()

        if self.view:
            self.view_cb_id = self.view.connect('active-changed', self.layer_change)

        self.notif('active-view-changed')
        self.layer_change()

    def get_active_view_window(self):
        """Fetch active GvViewWindow."""
        return self.view_manager.get_active_view_window()

    def get_active_view(self):
        """Fetch active GvViewArea."""
        return self.view_manager.get_active_view()

    def layer_change(self, *args):
        if self.view:
            new_layer = self.view.active_layer()
        else:
            new_layer = None

        if new_layer == self.layer:
            return

        if self.layer_selcb_id:
            self.layer.disconnect(self.layer_selcb_id)
            self.layer.disconnect(self.layer_sselcb_id)
            self.layer_selcb_id = None

        self.layer = new_layer

        if self.layer is not None and isinstance(self.layer, gview.GvShapeLayer):
            self.layer_selcb_id = self.layer.connect('selection-changed', self.sel_change)
            self.layer_sselcb_id = self.layer.connect('subselection-changed', self.ssel_change)

        self.notif('active-layer-changed')
        self.sel_change()
        self.ssel_change()

    def get_active_layer(self):
        return self.layer

    def sel_change(self, *args):
        try:
            new_len = len(self.layer.get_selected())
        except:
            new_len = 0

        if not new_len and not self.sel_len:
            return

        self.sel_len = new_len
        self.notif('selection-changed')

    def ssel_change(self, *args):
        try:
            new_ssel = self.layer.get_subselected()
        except:
            new_ssel = -1

        if new_ssel == -1 and self.ssel == -1:
            return

        if self.ssel_layer == self.layer and new_ssel == self.ssel:
            return

        self.ssel_layer = self.layer
        self.ssel = new_ssel

        self.notif('subselection-changed')

    def get_selected(self):
        selected = []
        if self.sel_len:
            selected = self.layer.get_selected()

        return selected

    def get_selected_shape(self):
        selected = self.get_selected()
        if not selected:
            return
        shapes = self.layer.get_parent()
        return shapes[selected[0]]
