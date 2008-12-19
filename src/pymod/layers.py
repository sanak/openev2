##############################################################################
# $Id$
#
# Project:  CIETmap
# Purpose:  Layer Management Window
# Author:   Paul Spencer, spencer@dmsolutions.ca
#
# This code is taken from layerdlg.py, part of the OpenEV package and modified
# for the CIETmap project
#
# The original code used a separate window for the layer dialog that was shared
# between many views.  This has been changed so that the layer dialog is
# associated with a single view and is in a widget that can be embedded inside
# the main application window.
#
# Developed by Mario Beauchamp (starged@gmail.com) for CIETcanada
# Originally developed by DM Solutions Group (www.dmsolutions.ca)
#
###############################################################################
# Copyright (c) 2000-2008, CIETcanada
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

# temporary
def _(s):
    return s

import gtk
from pgu import CList
from gvutils import yesno, create_pixbuf, create_stock_button
from gvsignaler import Signaler
import gview

EYE_W = 24

tooltips = gtk.Tooltips()

class Layers(gtk.Frame, Signaler):
    def __init__(self, viewwin):
        """Initialize the Layers list.
        Takes a single parameter, viewwin,
        which is the view window for which this is a layer list.
        Its only purpose is to access the viewarea because we don't want
        to reference it directly
        """
        gtk.Frame.__init__(self)
        scrlwin = gtk.ScrolledWindow()
        scrlwin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        scrlwin.set_shadow_type(gtk.SHADOW_NONE)
        self.viewwin = viewwin

        #setup connection to view area
        self.viewwin.viewarea.connect('active-changed', self.active_layer_changed)

        self.UImgr = gtk.UIManager()
        self.setup_actions()
        self.make_popup_menu()
        self.updating = True

        icons = ('ly_r','ly_p','ly_l','ly_a','ly_t')
        # layer on icon set
        self.layer_on = [create_pixbuf(icon+'_on') for icon in icons]
        # layer off icon set
        self.layer_off = [create_pixbuf(icon+'_off') for icon in icons]

        #the main layout widget
        shell = gtk.VBox()
        self.add(shell)
        shell.pack_start(scrlwin)

        tree_view = self.make_tree_view()
        scrlwin.add(tree_view)
        tree_view.connect('button-press-event', self.list_clicked)
        tree_view.connect('select-row', self.row_selected)

        # Option buttons
        opts = ((gtk.STOCK_GO_UP, _("Move Up"), self.raise_layer),
                (gtk.STOCK_GO_DOWN, _("Move Down"), self.lower_layer),
                (gtk.STOCK_REMOVE, _("Remove All Layers"), self.remove_all),
                (gtk.STOCK_DELETE, _("Remove Layer"), self.delete_layer))
        self.butbox = gtk.HBox(spacing=1, homogeneous=True)
        shell.pack_start(self.butbox, expand=False)
        for stock,tip,cb in opts:
            but = create_stock_button(stock, cb)
            self.butbox.pack_start(but)
            tooltips.set_tip(but, tip)

        self.connect('realize', self.realize)
        self.publish('deleted-layer')

        shell.show_all()
        self.tree_view = tree_view

        self.updating = False

    def setup_actions(self):
        self.actiongroup = gtk.ActionGroup('PopupActions')

        self.actiongroup.add_actions([
            ('MoveUp', gtk.STOCK_GO_UP, _("Move Up"), None, None, self.raise_layer),
            ('MoveDown', gtk.STOCK_GO_DOWN, _("Move Down"), None, None, self.lower_layer),
            ('RemoveLayer', gtk.STOCK_DELETE, _("Remove Layer"), None, None, self.delete_layer),
            ('FitActive', 'fit_actv', _("Fit Layer"), None, None, self.fit_layer),
            ('Legend', 'legend', _("Legend"), None, None, self.show_legend),
            ('ClassifyLayer', 'classify', _("Classify Layer"), None, None, self.classify_layer),
            ('Properties', gtk.STOCK_PREFERENCES, _("Properties"), None, None, self.launch_properties),
            ])

        self.UImgr.insert_action_group(self.actiongroup, 0)

    def make_popup_menu(self):
        self.UImgr.add_ui_from_string(
        """
        <popup name='PopupMenu'>
            <menuitem action='MoveUp'/>
            <menuitem action='MoveDown'/>
            <separator/>
            <menuitem action='RemoveLayer'/>
            <separator/>
            <menuitem action='FitActive'/>
            <menuitem action='Legend'/>
            <menuitem action='ClassifyLayer'/>
            <menuitem action='Properties'/>
        </popup>
        """)

    def make_tree_view(self):
        """ Form a view for the ListModel """
        types = (gtk.gdk.Pixbuf, str, int)
        titles = ("Vis", _("Layers"),"")

        tree_view = CList(titles, types)
        tree_view.get_renderer(0).set_fixed_size(EYE_W, EYE_W)

        img = gtk.image_new_from_stock('eye', gtk.ICON_SIZE_MENU)
        img.show()
        tree_view.get_column(0).set_widget(img)
        tree_view.get_column(2).set_visible(False)

        return tree_view

    def update_layers(self, *args):
        """Update the layer list"""
        #check to make sure the widget is realized
        if not self.flags() & gtk.REALIZED:
            return

        self.updating = True
        # gather a list of layers and ids from the view
        layers = self.viewwin.viewarea.list_layers()
        ids = [id(lyr) for lyr in layers]
        # gather a list of layers from the treeview list
        layers_lst = self.get_layers()
        # remove old layers from the treeview list
        for layer in layers_lst:
            if layer not in ids:
                self.remove_layer(layer)

        # add new layers to list
        for layer in layers:
            if id(layer) not in layers_lst:
                self.add_layer(layer)

        # restore active layer selection
        active = self.viewwin.viewarea.active_layer()
        if active and id(active) in layers_lst:
            self.tree_view.select_row(layers_lst.index(id(active)))
        self.updating = False

    def add_layer(self, layer):
        """Add a layer to the list"""
        layer.connect('meta-changed', self.layer_meta_changed)

        #draw appropriate layer icon - raster default
        icons = (self.layer_off, self.layer_on)[layer.is_visible()]
        if hasattr(layer, 'layer_type'):
            pixbuf = icons[layer.layer_type]
        else:
            pixbuf = icons[0]

        self.tree_view.list.prepend((pixbuf, layer.name, id(layer)))

    def realize(self, widget):
        """Once the widget is initialized, update the list"""
        # MB: do we really need this?
        self.update_layers()

    def active_layer_changed(self, *args):
        """track changes in the active layer"""
        if self.updating:
            return

        self.update_layers()

    def get_selected_layer(self):
        lst, iter = self.tree_view.get_selection().get_selected()
        if iter:
            return self.get_layer_by_id(lst.get_value(iter, 2))

    def get_layers(self):
        return [r[2] for r in self.tree_view.list]

    def toggle_visibility(self, row):
        pix = self.tree_view.list[row][0]
        layer = self.get_layer_by_id(self.tree_view.list[row][2])
        visible = layer.is_visible()
        icons = (self.layer_off, self.layer_on)[visible]
        idx = icons.index(pix)
        icons = (self.layer_off, self.layer_on)[not visible]
        self.tree_view.list[row][0] = icons[idx]
        layer.set_visible(not visible)

    def launch_properties(self, action):
        layer = self.get_selected_layer()
        ret = 'Yes'
        if layer.get_property('Class_sn'):
            if layer.parent[0].get_property('_gv_ogrfs'):
                msg = "This layer is classified.\nSetting properties on this layer "
                msg += "will remove\nexisting classification.\n"
                msg += "Do you wish to continue?"
                ret = yesno(_("Classified Layer"), _(msg))
                if ret == 'Yes':
                    layer.declassify()

        if ret == 'Yes':
            layer.launch_properties()

    def row_selected(self, selection):
        if self.updating:
            return
        lst, iter = selection.get_selected()
        if iter:
            layer = self.get_layer_by_id(lst.get_value(iter, 2))
            self.updating = True
            self.viewwin.viewarea.set_active_layer(layer)
            self.updating = False

    def list_clicked(self, treeview, event):
        p = treeview.get_path_at_pos(int(event.x), int(event.y))
        if p is None:
            return False

        path, column, xx, yy = p
        i_row = path[0]
        layer = self.get_layer_by_id(treeview.list[i_row][2])

        if event.button == 1:
            title = column.get_title()
            if event.type == gtk.gdk._2BUTTON_PRESS and title != "Vis":
                layer.launch_properties()
            elif event.type == gtk.gdk.BUTTON_PRESS and title == "Vis":
                treeview.emit_stop_by_name('button-press-event')
                self.toggle_visibility(i_row)
        elif event.button == 3:
            if layer.get_property('Class_sn') is None:
                vis = False
            else:
                vis = True
            self.actiongroup.get_action('Legend').set_visible(vis)
            if isinstance(layer, gview.GvRasterLayer):
                self.actiongroup.get_action('Properties').set_visible(True)
                self.actiongroup.get_action('ClassifyLayer').set_visible(True)
            else:
                vis = False
                self.actiongroup.get_action('ClassifyLayer').set_visible(not vis)
                self.actiongroup.get_action('Properties').set_visible(not vis)
            menu = self.UImgr.get_widget('/PopupMenu')
            menu.popup(None, None, None, event.button, event.time)

        return False

    def fit_layer(self, *args):
        """Handle request to zoom to layer extents."""
        view = self.viewwin.viewarea
        xmin, ymin, w, h = self.get_selected_layer().extents()

        xmax = xmin + w
        ymax = ymin + h

        buf = 0.05
        xmin = xmin - (xmax-xmin) * buf
        xmax = xmax + (xmax-xmin) * buf
        ymin = ymin - (ymax-ymin) * buf
        ymax = ymax + (ymax-ymin) * buf

        view.fit_extents(xmin, ymin, xmax-xmin, ymax-ymin)

    def raise_layer(self, *args):
        """Handle request to move a layer up"""
        view = self.viewwin.viewarea
        lst = self.tree_view.list
        layers = self.get_layers()
        row = self.get_layer_row(view.active_layer())
        index = len(layers) - row - 1
        if row == 0:
            return

        lst.swap(lst.get_iter(row-1), lst.get_iter(row))
        view.swap_layers(index, index+1)

    def lower_layer(self, *args):
        """Handle request to move a layer down"""
        view = self.viewwin.viewarea
        lst = self.tree_view.list
        layers = self.get_layers()
        row = self.get_layer_row(view.active_layer())
        index = len(layers) - row - 1
        if index == 0:
            return

        lst.swap(lst.get_iter(row), lst.get_iter(row+1))
        view.swap_layers(index-1, index)

    def delete_layer(self, *args):
        """Handle request to remove a layer"""
        view = self.viewwin.viewarea
        d_layer = view.active_layer()
        if d_layer:
            self.updating = True
            view.remove_layer(d_layer)
            self.updating = False
            self.remove_layer(id(d_layer))
            self.notif('deleted-layer', view, d_layer.name)

    def remove_layer(self, layer):
        row = self.get_layers().index(layer)
        iter = self.tree_view.get_iter((row,))
        self.tree_view.list.remove(iter)

    def remove_all(self, *args):
        self.updating = True
        self.viewwin.viewarea.remove_all_layers()
        self.updating = False
        self.tree_view.list.clear()
        self.notif('deleted-layer', None, '')

    def classify_layer(self, *args):
        """Handle request to classify a layer."""
        self.get_selected_layer().classify()

    def show_legend(self, *args):
        """Display the legend for the current layer if it is classified"""
        layer = self.get_selected_layer()
        if layer.get_property('Class_sn'):
            layer.show_legend()

    def layer_meta_changed(self, layer):
        """the metadata on a layer changed, update its name."""
        row = self.get_layer_row(layer)
        self.tree_view.set_text(row, 1, layer.name)

    def get_layer_by_id(self, lid):
        """Fetch the layer for the given id"""
        for layer in self.viewwin.viewarea.list_layers():
            if id(layer) == lid:
                return layer

    def get_layer_row(self, layer):
        """Returns the row index for that layer"""
        return self.get_layers().index(id(layer))
