###############################################################################
# $Id
# 
# Project:  OpenEV1/OpenEV2
# Purpose:  Simple GvViewArea for testing
# Author:   Mario Beauchamp, starged@gmail.com
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

def add_layer(filename):
    f,ext = os.path.splitext(filename)
    if ext == '.tif':
        view.add_layer(gview.GvRasterLayer(
                        gview.GvRaster(dataset=gdal.Open(filename)),
                        [('mesh_lod','4')], rl_mode=gview.RLM_SINGLE))
    elif ext == '.shp':
        view.add_layer(gview.GvShapesLayer(
                        gview.GvShapes(shapefilename=filename)))
    else:
        return
    view.set_active_layer(view.list_layers()[0])
    gview.gv_data_registry_dump()

def del_layer(but):
    layer = view.active_layer()
    view.remove_layer(layer)
    layer.destroy()

def open_clicked(but):
    if gtk_ver == '2.0':
        file_dlg = gtk.FileChooserDialog(title='Open File',
                    buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,
                            gtk.STOCK_OPEN,gtk.RESPONSE_OK))
        response = file_dlg.run()
        file_dlg.hide()
        if response == gtk.RESPONSE_OK:
            filename = file_dlg.get_filename()
            add_layer(filename)
        file_dlg.destroy()
    else:
        file_dlg = gtk.GtkFileSelection(title='Open File')
        file_dlg.ok_button.connect('clicked', load_file, file_dlg)
        file_dlg.show()

# GTK1 only
def load_file(but, file_dlg):
    file_dlg.hide()
    filename = file_dlg.get_filename()
    add_layer(filename)
    file_dlg.destroy()
    
def run():
    global view
    view = gview.GvViewArea()
    if gtk_ver == '2.0':
        win = gtk.Window()
        win.connect('delete-event', gtk.main_quit)

        vbox = gtk.VBox()
        view.set_size_request(300,300)

        bbox = gtk.HButtonBox()
        del_but = gtk.Button(stock=gtk.STOCK_DELETE)
        open_but = gtk.Button(stock=gtk.STOCK_OPEN)
        quit_but = gtk.Button(stock=gtk.STOCK_QUIT)
    else:
        win = gtk.GtkWindow()
        win.connect('delete-event', gtk.mainquit)

        vbox = gtk.GtkVBox()
        view.set_usize(300,300)

        bbox = gtk.GtkHButtonBox()
        del_but = gtk.GtkButton('Delete')
        open_but = gtk.GtkButton('Open')
        quit_but = gtk.GtkButton('Quit')

    vbox.set_border_width(5)
    bbox.set_border_width(5)

    # pack stuff
    bbox.pack_start(del_but, expand=False)
    bbox.pack_start(open_but, expand=False)
    bbox.pack_start(quit_but, expand=False)

    vbox.pack_start(view, expand=False)
    vbox.pack_end(bbox, expand=False)
    win.add(vbox)
    
    # connect buttons
    open_but.connect('clicked', open_clicked)
    del_but.connect('clicked', del_layer)
    quit_but.connect('clicked', close, win)
    
    win.show_all()
    
    if gtk_ver == '2.0':
        gtk.main()
    else:
        gtk.mainloop()

def close(but, win):
    win.hide()
    win.destroy()
    if gtk_ver == '2.0':
        gtk.main_quit()
    else:
        gtk.mainquit()

if __name__ == '__main__':
    global gtk_ver
    import sys

    gtk_ver = '2.0' # defaults to GTK2
    if len(sys.argv) == 2:
        gtk_ver = sys.argv[1]

    try:
        import pygtk
        pygtk.require(gtk_ver)
    except ImportError:
        # assume we only have 1.x
        gtk_ver = '1.x'

    import gtk
    import os
    import gdal
    import gview

    run()
