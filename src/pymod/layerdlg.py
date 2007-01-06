##############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Layer Management Dialog
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
import gobject
import gview
import os.path
from gvsignaler import Signaler
import gvhtml

THUMB_W = 24
THUMB_H = 32
EYE_W = 24
LAYER_HDR = "Layer"
THUMB_HDR = "Image"
EYE_HDR = "Visible"

# FIXME: Need a global tooltips object?
# tooltips = GtkTooltips()

static_layer_dialog = None

def Launch():
    global static_layer_dialog

    if static_layer_dialog is None:
        static_layer_dialog = LayerDlg()

    return static_layer_dialog

class LayerDlg(gtk.Window,Signaler):
    def __init__(self):
        gtk.Window.__init__(self)
        self.set_title('Layers')
        self.set_size_request(250, 500)
        self.set_border_width(3)
        self.set_resizable(True)
        self.connect('delete-event',self.close)
        shell = gtk.VBox(spacing=3)
        self.add(shell)
        gvhtml.set_help_topic(self, "layerdlg.html" );

        # View chooser menu
        hbox = gtk.HBox(spacing=3)
        shell.pack_start(hbox, expand=False)
        hbox.pack_start(gtk.Label('View:'), expand=False, padding=3)
        viewopt = gtk.OptionMenu()
        hbox.pack_start(viewopt)
        viewmenu = gtk.Menu()
        viewopt.set_menu(viewmenu)

        # Do we want to include a thumbnail?  This is buggy on some platforms.
        # Note: GTK2 PORT - Thumbnails in tree view not tested
        if gview.get_preference('layer_thumbnail') is None \
           or gview.get_preference('layer_thumbnail') == 'off':
            self.thumbnail = False
        else:
            self.thumbnail = True

        self.updating = False

        #
        # Layer list model
        #
        if self.thumbnail:
           self.list_store = gtk.ListStore( gtk.gdk.Pixbuf, gtk.gdk.Pixbuf, 
                                            gobject.TYPE_STRING)
        else:
           self.list_store = gtk.ListStore( gtk.gdk.Pixbuf, gobject.TYPE_STRING)

        #
        # Layer list view
        #
        self.make_tree_view(self.list_store)

        layerbox = gtk.ScrolledWindow()
        shell.pack_start(layerbox)
        layerbox.add_with_viewport(self.tree_view)

        self.tree_view.get_selection().set_mode(gtk.SELECTION_SINGLE)

        self.tree_view.connect('button-press-event', self.list_pressed)
        self.tree_view.connect('button-release-event', self.list_released)

        # Option buttons
        opts = (('new.xpm', 'New layer', self.new_layer),
                ('raise.xpm', 'Raise layer', self.raise_layer),
                ('lower.xpm', 'Lower layer', self.lower_layer),
                ('delete.xpm','Delete layer', self.delete_layer))
        butbox = gtk.HBox(spacing=1)
        shell.pack_start(butbox, expand=False)
        for opt in opts:
            but = gtk.Button()
            butbox.pack_start(but)

            img = gtk.Image()
            img.set_from_file(os.path.join(gview.home_dir,'pics',opt[0]))
            img.show()
            but.add(img)

            # This works too...
            #pixmap = gtk.gdk.pixmap_create_from_xpm(self.window, None, 
            #    os.path.join(gview.home_dir,'pics',opt[0]))
            #img = gtk.Image()
            #img.set_from_pixmap(pixmap[0], pixmap[1])
            #img.show()
            #but.add(img)

            # tooltips.set_tip(but, opt[1])
            but.connect('clicked', opt[2])

        self.connect('realize', self.realize)

        shell.show_all()
        self.viewopt = viewopt
        self.viewmenu = viewmenu
        self.views = {}
        self.menuitems = {}
        self.selected_view = None

        path =  os.path.join(gview.home_dir,'pics','eye.xpm')
        self.eye_pixbuf = gtk.gdk.pixbuf_new_from_file(path)

        # Publish signals
        self.publish('active-view-changed')
        self.publish('deleted-layer')


    def close(self,*args):
        self.hide()
        return True

    def list_layers(self):
        lst = []
        if self.selected_view is not None:
            lst = self.views[self.selected_view].list_layers()
            # Reverse the list since we want the last draw layer listed first.
            lst.reverse()
        return lst

    def make_tree_view( self, model ):
        """ Form a view for the ListModel """
        self.tree_view = gtk.TreeView( model )

	self.renderer0 = gtk.CellRendererPixbuf()
        self.renderer1 = gtk.CellRendererText()
        self.renderer1.set_property( 'editable', False )
        self.renderer0.set_fixed_size( EYE_W, THUMB_H + 4)
        self.renderer1.set_fixed_size( -1, THUMB_H + 4)

        self.column0 = gtk.TreeViewColumn(EYE_HDR, self.renderer0, pixbuf=0)
        self.tree_view.append_column( self.column0 )
        if self.thumbnail:
            self.renderer2 = gtk.CellRendererPixbuf()
            self.renderer2.set_fixed_size( THUMB_W + 4, THUMB_H + 4)
            self.column1 = gtk.TreeViewColumn(LAYER_HDR, self.renderer1, text=2)
            self.column2 = gtk.TreeViewColumn(THUMB_HDR, self.renderer2, pixbuf=1)
            self.tree_view.append_column( self.column2 )
            self.tree_view.append_column( self.column1 )
        else:
            self.column1 = gtk.TreeViewColumn(LAYER_HDR, self.renderer1, text=1)
            self.tree_view.append_column( self.column1 )

        return self.tree_view

    def add_view(self, name, view):
        # FIXME: connect to view 'destroy' event ?
        self.views[name] = view
        menuitem = gtk.MenuItem(name)
        self.viewmenu.append(menuitem)
        menuitem.connect('activate', self.view_selected, name)
        menuitem.show()
        self.menuitems[name] = menuitem
        if self.viewmenu.get_active() == menuitem:
            self.viewopt.set_history(0)
            menuitem.activate()

    def remove_view(self, name):
        try:
            view = self.views[name]
            menuitem = self.menuitems[name]
        except KeyError:
            return
        self.viewmenu.remove(menuitem)
        self.viewopt.set_history(0)
        del self.views[name]
        del self.menuitems[name]
        if len(self.menuitems) > 0:
            newitem = self.viewmenu.get_active()
            newitem.activate()

        if len(self.views) == 0:
            # FIXME: things get kind of screwed up here...
            # there doesn't seem to be a way to tell gtk.Menu/gtk.OptionMenu
            # that there is nothing active.  This at least is stable...
            # Possible solution: rebuild viewmenu on each view add/remove.
            if (hasattr(self, 'active_change_id')):
                view.disconnect(self.active_change_id)
                self.selected_view = None

    def view_selected(self, item, name):
        # don't use item - view_selected() is called from gvapp with item=None
        if name == self.selected_view: return
        if self.selected_view:
            self.views[self.selected_view].disconnect(self.active_change_id)
        self.selected_view = name
        i = 0
        for x in self.viewmenu.get_children():
            if x == self.menuitems[name]:
                self.viewopt.set_history(i)
                break
            i = i + 1
        Signaler.notify(self, 'active-view-changed')

        view = self.views[name]
        self.active_change_id = view.connect('active-changed',
                                             self.active_layer_changed)
        self.update_layers()
        self.active_layer_changed(view)

        Signaler.notify(self,  'active-view-changed')

    def get_active_view(self):
        if self.selected_view:
            return self.views[self.selected_view]
        else:
            return None

    def update_layers(self,*args):
        if not self.flags() & gtk.REALIZED: return

        self.updating = True

        lst = self.list_store
        view = self.views[self.selected_view]
        layers = self.list_layers()

        # get active layer so we can restore after
        active = view.active_layer()
        if active is not None and active in layers:
            active_row = layers.index(active)
        else:
            active_row = None

        if self.thumbnail:
            thumbnail_mask = create_bitmap_from_data(
                self.window, '\xff' * (THUMB_W/8 * THUMB_H),
                THUMB_W, THUMB_H)

        #
        # Recreate list_store data from layers
        #
        lst.clear()
        for i in range(len(layers)):
            if layers[i].is_visible():
                pixbuf = self.eye_pixbuf
            else:
                pixbuf = None
            if self.thumbnail:
                try:
                    thumbnail = view.create_thumbnail(layers[i],
                                                      THUMB_W, THUMB_H)
                except:
                    thumbnail = ''
                lst.append((pixbuf, thumbnail, layers[i].get_name()))
            else:
                lst.append((pixbuf, layers[i].get_name()))

        # restore active layer selection
        if active_row is not None:
            selection = self.tree_view.get_selection()
            selection.select_path((active_row,))

        self.updating = False

    def realize(self, widget):
        if self.selected_view:
            self.update_layers()

    def active_layer_changed(self, view):
        self.update_layers()
        layers = self.list_layers()
        active = view.active_layer()
        if active is not None and active in layers:
            self.tree_view.get_selection().select_path((layers.index(active),));

    def toggle_visibility(self, row):
        layers = self.list_layers()
        layer = layers[row]

        layer.set_visible(not layer.is_visible())

        self.update_layers()

    def launch_properties(self, row):
        layers = self.list_layers()
        layer = layers[row]
        layer.launch_properties()

    #
    # Mouse released callback in list; toggles visibility or launches properties
    #
    def list_released(self, treeview, event):

        # Get selected item
        #model, iter = treeview.get_selection().get_selected()
        #if iter is not None:
        #    row = model.get_path(iter)

        p = treeview.get_path_at_pos(int(event.x), int(event.y))
        if p is None:
            return False
        path, column, xx, yy = p
        i_row = path[0]

        if event.button == 1:
            if column.get_title() == EYE_HDR:
                self.toggle_visibility(i_row)
        elif event.button == 3:
            self.launch_properties(i_row)
            return True

        return False

    #
    # Mouse pressed callback; selects layer
    #
    def list_pressed(self, treeview, event):
        if self.updating:
            return

        # Get selected item
        #model, iter = treeview.get_selection().get_selected()
        #if iter is not None:
        #    row = model.get_path(iter)

        p = treeview.get_path_at_pos(int(event.x), int(event.y))
        if p is None:
            return False
        path, column, xx, yy = p
        i_row = path[0]

        if column.get_title() == LAYER_HDR:
            view = self.views[self.selected_view]
            layers = self.list_layers()
            view.handler_block(self.active_change_id)
            view.set_active_layer(layers[i_row])
            view.handler_unblock(self.active_change_id)
            return False

        return True

    def new_layer(self, *args):
        if not self.selected_view:
            return
        view = self.views[self.selected_view]
        layer_list = view.list_layers()
        layer_map = {}
        for layer in layer_list:
            layer_map[layer.get_name()] = layer

        counter = 1
        name = 'UserShapes_'+str(counter)
        while layer_map.has_key(name):
            counter = counter + 1
            name = 'UserShapes_'+str(counter)

        shapes = gview.GvShapes(name=name)
        gview.undo_register(shapes)
        layer = gview.GvShapesLayer(shapes)
        view.add_layer(layer)
        view.set_active_layer(layer)


    def raise_layer(self, *args):
        if not self.selected_view: return
        view = self.views[self.selected_view]        
        layers = self.list_layers()
        row = layers.index(view.active_layer())
        index = len(layers) - row - 1
        if row == 0: return
        self.list_store.swap(self.list_store.get_iter(row-1),
                             self.list_store.get_iter(row))
        view.swap_layers(index, index+1)

    def lower_layer(self, *args):
        if not self.selected_view: return
        view = self.views[self.selected_view]        
        layers = self.list_layers()
        row = layers.index(view.active_layer())
        index = len(layers) - row - 1
        if index == 0: return
        self.list_store.swap(self.list_store.get_iter(row),
                             self.list_store.get_iter(row+1))
        view.swap_layers(index-1, index)

    def delete_layer(self, *args):
        if not self.selected_view: return
        view = self.views[self.selected_view]
        layer = view.active_layer()
        layername = layer.get_name()
        if layer is not None:
            view.remove_layer(layer)

        Signaler.notify(self, 'deleted-layer',view,layername)


    def get_selected_layer(self, *args):
        """ Returns a tuple with the name of the active view and the object with the currently selected layer
            in that view.  From the layer you can get the layer name another other useful properties of the
            layer. """
        if not self.selected_view: return
        view = self.views[self.selected_view]
        return (self.selected_view, view.active_layer())
