###############################################################################
# $Id: gvselbrowser.py,v 1.1.1.1 2005/04/18 16:38:35 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  GUI component to show the current list of selected objects, and
#           to control a single sub-selection out of that set. 
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
from string import *
from gvsignaler import *
import os.path
import pgu
import gview
import gvutils

class GvSelBrowser(gtk.VBox):

    def __init__(self, spacing=10):
        gtk.VBox.__init__(self, spacing=spacing)

        self.updating = 0
        
        self.sel_manager = gview.app.sel_manager

        self.sel_manager.subscribe('active-layer-changed', self.update_gui)
        self.sel_manager.subscribe('selection-changed', self.update_gui)
        self.sel_manager.subscribe('subselection-changed', self.update_gui)

        self.tooltips = gtk.Tooltips()

        hbox = gtk.HBox(spacing=3)
        self.pack_start(hbox,expand=False)
        self.hbox = hbox

        hbox.pack_start(gtk.Label('Shape:'),expand=False)

        self.oid_tb = gtk.Entry()
        self.oid_tb.set_max_length(7)
        self.oid_tb.connect('activate', self.oid_cb)
        self.oid_tb.connect('focus-out-event', self.oid_cb)
        hbox.pack_start(self.oid_tb)

        left_button = gtk.Button()
        
        print os.path.join(gview.home_dir,'pics', 'pan_left.xpm')
        im = gtk.Image()
        im.set_from_file(os.path.join(gview.home_dir,'pics', 'pan_left.xpm'))
        left_button.add(im)
        self.tooltips.set_tip(left_button,'Cycle Selection Down')
        left_button.connect('clicked', self.cycle_down)
        hbox.pack_start(left_button,expand=False)

        self.n_of_n_label = gtk.Label('XXXX of XXXX')
        hbox.pack_start(self.n_of_n_label)
        
        right_button = gtk.Button()
        im = gtk.Image()
        im.set_from_file(os.path.join(gview.home_dir,'pics', 'pan_rght.xpm'))
        right_button.add(im)
        self.tooltips.set_tip(left_button,'Cycle Selection Up')
        right_button.connect('clicked', self.cycle_up)
        hbox.pack_start(right_button, expand=False)

        hbox = gtk.HBox(spacing=3)
        self.pack_start(hbox)
        self.layer_label = gtk.Label('XXXXXXXXXXXXXXXXXXXXXXXXXXX')
        self.layer_label.set_justify( gtk.JUSTIFY_LEFT )
        hbox.pack_start(self.layer_label, expand=False)

        self.connect('unrealize', self.close)
        
        self.update_gui()
        self.show_all()

    def close(self, *args):
        self.sel_manager.unsubscribe('active-layer-changed', self.update_gui)
        self.sel_manager.unsubscribe('selection-changed', self.update_gui)
        self.sel_manager.unsubscribe('subselection-changed', self.update_gui)
        
    def update_gui(self, *args):
        self.updating = 1
        layer = self.sel_manager.get_active_layer()
        if layer is None:
            self.layer_label.set_text('Layer: <none selected>')
        else:
            self.layer_label.set_text('Layer: '+layer.get_name())
            
        try:
            layer = self.sel_manager.get_active_layer()
            selected = layer.get_selected()
            subsel = layer.get_subselected()
        except:
            self.n_of_n_label.set_text('0 of 0')
            self.oid_tb.set_text('')
            self.updating = 0
            return
        
        self.oid_tb.set_text(str(subsel))

        index_of = self.get_sel_index(subsel,selected)
        
        label = '%d of %d' % (index_of+1, len(selected))
        self.n_of_n_label.set_text(label)
        
        self.updating = 0

    def oid_cb(self, *args):
        if self.updating:
            return
        
        try:
            new_oid = int(self.oid_tb.get_text())
            layer = self.sel_manager.get_active_layer()
            selected = layer.get_selected()
            if new_oid in selected:
                layer.subselect_shape( new_oid )
            else:
                layer.clear_selection()
                if new_oid >= 0:
                    layer.select_shape( new_oid )

            layer.display_change()
        except:
            pass

    def cycle_down(self, *args):
        try:
            layer = self.sel_manager.get_active_layer()
            selected = layer.get_selected()
        except:
            return
        
        index_of = self.get_sel_index( layer.get_subselected(), selected )
        if index_of > 0:
            layer.subselect_shape( selected[index_of-1] )

    def cycle_up(self, *args):
        try:
            layer = self.sel_manager.get_active_layer()
            selected = layer.get_selected()
        except:
            return
            
        index_of = self.get_sel_index( layer.get_subselected(), selected )
        if index_of < len(selected)-1:
            layer.subselect_shape( selected[index_of+1] )

    def get_sel_index(self, subsel, selected):
        index_of = 0
        while index_of < len(selected) \
              and selected[index_of] != subsel:
            index_of = index_of + 1

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
                           understood by openev.ViewManager).

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
        self.view_manager.subscribe('active-view-changed',self.view_change)

        self.view = self.view_manager.get_active_view()
        if self.view is not None:
            self.view_cb_id \
                = self.view.connect('active-changed', self.layer_change)
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

        if self.view_cb_id is not None:
            self.view.disconnect(self.view_cb_id)
            self.view_cb_id = None
            
        self.view = self.view_manager.get_active_view()
        
        if self.view is not None:
            self.view_cb_id \
                = self.view.connect('active-changed', self.layer_change)
        
        Signaler.notify(self, 'active-view-changed')
        self.layer_change()

    def get_active_view_window(self):
        """Fetch active GvViewWindow."""
        return self.view_manager.get_active_view_window()

    def get_active_view(self):
        """Fetch active GvViewArea."""
        return self.view_manager.get_active_view()

    def layer_change(self, *args):
        if self.view is None:
            new_layer = None
        else:
            new_layer = self.view.active_layer()

        if new_layer == self.layer:
            return
        
        if self.layer_selcb_id is not None:
            self.layer.disconnect(self.layer_selcb_id)
            self.layer.disconnect(self.layer_sselcb_id)
            self.layer_selcb_id = None

        self.layer = new_layer

        if self.layer is not None \
           and gvutils.is_of_class(self.layer.__class__, 'GvShapeLayer'):
            self.layer_selcb_id = \
                 self.layer.connect('selection-changed',self.sel_change)
            self.layer_sselcb_id = \
                 self.layer.connect('subselection-changed',self.ssel_change)

        Signaler.notify(self, 'active-layer-changed')
        self.sel_change()
        self.ssel_change()

    def get_active_layer(self):
        return self.layer

    def sel_change(self,*args):
        try:
            new_len = len(self.layer.get_selected())
        except:
            new_len = 0

        if new_len == 0 and self.sel_len == 0:
            return

        self.sel_len = new_len
        Signaler.notify(self, 'selection-changed')

    def ssel_change(self,*args):

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

        Signaler.notify(self, 'subselection-changed')
    

pgu.gtk_register('GvSelBrowser',GvSelBrowser)
