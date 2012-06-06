#!/usr/bin/env python

import gviewapp
import gview
import pygtk
pygtk.require('2.0')
import gtk
from gtk.keysyms import *
import sys
import random
import os


def force_load( filename ):
    ds = gview.manager.get_dataset( filename )
    if ds is None:
        return ds
    
    band_list = []
    for band in range(0,ds.RasterCount):
        band_list.append( ds.GetRasterBand(band+1) )

    for line in range(0,ds.RasterYSize):
        for band in range(0,ds.RasterCount):
            band_list[band].ReadRaster( 0, line, ds.RasterXSize, 1 )

    if ds.RasterCount >= 3:
        rasters = (gview.manager.get_dataset_raster( ds, 1 ),
                   gview.manager.get_dataset_raster( ds, 2 ),
                   gview.manager.get_dataset_raster( ds, 3 ))
    else:
        rasters = [gview.manager.get_dataset_raster( ds, 1 )]

    for rast in rasters:
        rast.force_load()
        
    gview.file_list_ds.append(rasters)
    del gview.file_list_ds[0]

    return ds

def clean_old_layer():
    view = gview.app.view_manager.get_active_view()

    layers = view.list_layers()
    if len(layers) > 1:
        view.remove_layer( layers[0] )

    gview.request_clean = 0

    try:
        ds = force_load( gview.file_list[gview.file_cur+1] )
    except:
        pass
        
def request_clean( *args ):
    gview.request_clean = 1
    
def update_view():
    view = gview.app.view_manager.get_active_view()

    cur_file = gview.file_list[gview.file_cur]
    print 'update_view:' + cur_file

    ds = force_load( cur_file )
    try:
        ds = force_load( cur_file )
    except:
        os.system( 'ls -l ' + cur_file )
        os.system( 'file ' + cur_file )
        return

    old_layer = view.active_layer()
    
    gview.app.file_open_by_name( cur_file )
    view.fit_extents( 0, 0, ds.RasterXSize, ds.RasterYSize )

    layer = view.active_layer()
    layer.set_property( 'force_load', '100' )
    view.queue_draw()
    if old_layer is not None:
        gview.manager.queue_task( 'cleaner', 15, request_clean )

    if old_layer is not None:
        view.remove_layer( old_layer )
        
def advance( step = 1 ):
    gview.file_cur = gview.file_cur + step
    if gview.file_cur < 0:
        gview.file_cur = 0
    if gview.file_cur >= len(gview.file_list):
        gview.file_cur = len(gview.file_list)-1
        
    update_view()
    
def key_press_cb( viewarea, event, *args ):
    print event.keyval
    if event.keyval == gtk.keysyms.space:
        advance(1)

    if event.keyval == gtk.keysyms.minus:
        advance(-1)

    if event.keyval == gtk.keysyms.q:
        sys.exit()

    if event.keyval == gtk.keysyms.j:
        advance(10)

    if event.keyval == gtk.keysyms.p:
        advance(-10)

    if event.keyval == gtk.keysyms.d:
        os.unlink( gview.file_list[gview.file_cur] )
        advance()

    if event.keyval == gtk.keysyms.r:
        gview.file_cur = int(random.random() * len(gview.file_list))
        update_view()
        


# #############################################################################
# Main

app = gviewapp.GViewApp()
gview.app = app
app.subscribe('quit',gtk.main_quit)
app.show_layerdlg()
app.new_view(None)
app.do_auto_imports()

view = gview.app.view_manager.get_active_view()
view.connect('key-press-event', key_press_cb)

gview.file_list = []
gview.file_list_ds = [None, None, None]
gview.file_cur = 0

# Command line parser
i = 1
while i < len(sys.argv):
    arg = sys.argv[i]
    gview.file_list.append( arg )
    i = i + 1

update_view()

gview.request_clean = 0
while 1:
    gtk.main_iteration()

    if gview.request_clean == 1:
        clean_old_layer()


