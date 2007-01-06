#!/usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  OpenEV General Purpose GvViewWindow class.
# Author:   Frank Warmerdam, warmerdam@pobox.com
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
from gtk.gdk import *
from gtk.keysyms import *
import gtkmissing
import sys

import gview
import gvconst
import layerdlg
import gdal
from gdalconst import *
import gvutils
import os
import pgufilesel
import pguprogress
import math
import gvhtml
import string
import glob
import gviewapp

ratio_list = ['250:1', '200:1', '150:1', '100:1', '80:1', '60:1', '45:1',
              '35:1', '25:1', '18:1', '10:1', '8:1', '4:1', '2:1', '1:1',
              '1:2', '1:4', '1:6', '1:8', '1:10', '1:12', '1:14','1:16',
              '1:20', '1:25', '1:30', '1:40', '1:60', '1:80', '1:100',]

def GvViewWindowFromXML( node, parent, filename=None ):
    import gdal
    instance = \
      GvViewWindow( app = parent, 
        title = gvutils.XMLFindValue( node, "title", '' ),
        show_menu = int(gvutils.XMLFindValue( node, "show_menu", "1")),
        show_icons = int(gvutils.XMLFindValue( node, "show_icons", "1")),
        show_tracker = int(gvutils.XMLFindValue( node, "show_tracker", "1")),
        show_scrollbars = int(gvutils.XMLFindValue( node, "show_scrollbars", "1")),
        menufile = gvutils.XMLFindValue( node, 'menufile', None),
        iconfile = gvutils.XMLFindValue( node, 'iconfile', None))

    if gvutils.XMLFindValue( node, "width", None ) is not None and \
       gvutils.XMLFindValue( node, "height", None ) is not None:
        instance.set_default_size(
            int(gvutils.XMLFindValue( node, "width", "600")),
            int(gvutils.XMLFindValue( node, "height", "600")) )

    viewarea_tree = gvutils.XMLFind( node, "GvViewArea" )   
    if viewarea_tree != None:
        instance.viewarea.initialize_from_xml( viewarea_tree,
                                               filename=filename )
            
    instance.show()

    # We can't move the window till after it is shown.
    if gvutils.XMLFindValue( node, "x", None ) is not None and \
       gvutils.XMLFindValue( node, "y", None ) is not None:
        instance.window_move(
            int(gvutils.XMLFindValue( node, "x", "0")),
            int(gvutils.XMLFindValue( node, "y", "0")) )

    return instance

class GvViewWindow(gtk.Window):
    next_viewnum = 1

    def __init__(self, app=None, title=None, show_menu=1, show_icons=1, 
                 show_tracker=1, show_scrollbars=1, menufile='DefaultMenuFile.xml',iconfile='DefaultIconFile.xml'):
        
        gtk.Window.__init__(self)

        if title is None:
            title = 'View %d' % GvViewWindow.next_viewnum
            GvViewWindow.next_viewnum = GvViewWindow.next_viewnum + 1

        self.app = app
        self.set_title('OpenEV: '+title)
        gvhtml.set_help_topic(self, 'mainwindow.html')
        self.view_title = title
        self.file_sel = None
        self.drape_file_sel = None
        self.DEM_file_sel = None
        shell = gtk.VBox(spacing=0)
        self.add(shell)
        self.pref_dialog = None
        self.position3D_dialog = None
        self.set_resizable(True)
        self.zoom = 0.0
        self.zoom_flag = 'yes' # see set_zoom_factor_cb()
        self.zoom_factor = None
        self.position3D_dialog = None
        self.menufile = menufile
        self.iconfile = iconfile

        # Menu bar
        if show_menu > 0:
            self.create_menubar(menufile)
            shell.pack_start(self.menuf, expand=False)
        else:
            self.menuf = None

        if show_icons > 0:
            self.create_iconbar(iconfile)
            shell.pack_start(self.iconbar,expand=False)
        else:
            self.iconbar = None        

        # Add the actual GvViewArea for drawing in
        self.viewarea = gview.GvViewArea()

        if gview.get_preference('view_background_color') is not None:
            tokens = string.split(gview.get_preference('view_background_color'))
            self.viewarea.set_background_color( [ float(tokens[0]),
                                                  float(tokens[1]),
                                                  float(tokens[2]),
                                                  float(tokens[3])] )

        # Update Zoom ratio box in toolbar whenever view changes
	# (actually, only when zoom changes)
        self.view_state_changed_id = \
	    self.viewarea.connect("view-state-changed", self.update_zoom_cb)
        self.viewarea.connect("active-changed", self.update_zoom_cb)

	size = (620, 620)
	if show_scrollbars:
            self.scrolled_window = gtk.ScrolledWindow()
            self.set_size_request(size[0], size[1] + 60)
            self.scrolled_window.add(self.viewarea)
            shell.pack_start(self.scrolled_window, expand=True)
        else:
            self.viewarea.size(size)
            self.scrolled_window = None
            shell.pack_start(self.viewarea, expand=True)

        if show_tracker:
            statusbar = gtk.HBox()
            shell.pack_start(statusbar, expand=False)
            label = gtk.Label()
            statusbar.pack_start(label, expand=False, padding=3)
            tracker = gview.GvTrackTool(label)
            tracker.activate(self.viewarea)
            self.tracker = tracker
        else:
            self.tracker = None

        # End of widgets
        self.viewarea.grab_focus()
        shell.show_all()

        self.show_rfl()

        self.app.view_manager.add_view( self )

        self.rawgeo_update()

        self.viewarea.connect('key-press-event', self.key_press_cb)

        # The close flag is meant to be used
        # by other applications that use OpenEV's windows,
        # but may want other closing behaviour for that
        # particular window (eg. user
        # unable to close, hiding instead of destroying,
        # etc.).  0 is for default, 1 for not doing anything,
        # 2 for hiding.
        self.close_flag=0
        
        # Trap window close event
        self.connect('delete-event', self.close)

    def serialize(self,base=None, filename=None ):
        if base is None:
            base = [gdal.CXT_Element, 'GvViewWindow']
            base.append( [gdal.CXT_Attribute, 'module',
                         [gdal.CXT_Text, 'gvviewwindow']] )

        if self.menuf is None:
            base.append( [gdal.CXT_Attribute, 'show_menu',
                         [gdal.CXT_Text, '0']] )
        if self.iconbar is None:
            base.append( [gdal.CXT_Attribute, 'show_icons',
                         [gdal.CXT_Text, '0']] )
        if self.tracker is None:
            base.append( [gdal.CXT_Attribute, 'show_tracker',
                         [gdal.CXT_Text, '0']] )
        if self.scrolled_window is None:
            base.append( [gdal.CXT_Attribute, 'show_scrollbars',
                         [gdal.CXT_Text, '0']] )

        geometry = self.get_allocation()
        
        base.append( [gdal.CXT_Attribute, 'width',
                      [gdal.CXT_Text, str(geometry[2])]] )
        base.append( [gdal.CXT_Attribute, 'height',
                      [gdal.CXT_Text, str(geometry[3])]] )

        x_pos, y_pos = self.get_position()
        base.append( [gdal.CXT_Attribute, 'x',
                      [gdal.CXT_Text, str(x_pos)]] )
        base.append( [gdal.CXT_Attribute, 'y',
                      [gdal.CXT_Text, str(y_pos)]] )

        base.append( [gdal.CXT_Element, 'title',
                      [gdal.CXT_Text, self.view_title]] )

        if self.menufile is not None:
            base.append( [gdal.CXT_Element, 'menufile',
                      [gdal.CXT_Text, self.menufile]] )

        if self.iconfile is not None:
            base.append( [gdal.CXT_Element, 'iconfile',
                      [gdal.CXT_Text, self.iconfile]] )

        base.append( self.viewarea.serialize( filename=filename ) )

        return base

    def show_rfl(self, *args):
        if self.menuf is None:
            return

        if ((self.app is None) or (hasattr(self.app,'get_rfl') == 0)):
            return

        try:
            list = self.app.get_rfl()
            for i in range(5):
                menuitem = self.menuf.find('File/rfl'+str(i+1))
                if i < len(list):
                    menuitem.get_children()[0].set_text(list[i])
                    menuitem.show()
                else:
                    menuitem.get_children()[0].set_text('')
                    menuitem.hide()
        except:
            # Some menus don't have rfl
            pass

    def rfl_cb(self, menuitem, rfl_index, *args):
        self.file_open_by_name(menuitem.get_children()[0].get(), sds_check=0)
    
    def make_active(self, *args):
        self.app.view_manager.set_active_view( self )

    def key_press_cb( self, viewarea, event, *args ):
        if event.keyval == F9:
            gview.texture_cache_dump()

    def busy_changed_cb(self,*args):
        if gview.manager.get_busy():
            self.idlebusy_pixmap.set_from_pixmap(self.busy_icon[0], self.busy_icon[1])
        else:
            self.idlebusy_pixmap.set_from_pixmap(self.idle_icon[0], self.idle_icon[1])

    def print_cb(self, *args):
        import gvprint
        pd = gvprint.GvPrintDialog( self.viewarea )
        
    def helpcb(self, item, topic='openevmain.html'):
        gvhtml.LaunchHTML( topic )

    def aboutcb(self, *args):
        window = gtk.Window()
        window.set_title('About OpenEV')
        vbox = gtk.VBox(homogeneous=False,spacing=15)
        window.add(vbox)

        im = gtk.Image()
        im.set_from_file(os.path.join(gview.home_dir,'pics',
                                                         'openev.xpm'))
        vbox.pack_start(im)
        # Contributors
        contrib = gtk.VBox(homogeneous=False,spacing=3)
        contrib.pack_start(gtk.Label('Contributors:'))
        contrib.pack_start(gtk.Label('Frank Warmerdam (warmerdam@pobox.com),'))
        contrib.pack_start(gtk.Label('Gillian Walter (gillian.walter@atlantis-scientific.com),'))
        contrib.pack_start(gtk.Label('Peter Farris-Manning (peter.farris-manning@atlantis-scientific.com),'))
        contrib.pack_start(gtk.Label('Paul Spencer (pagemeba@magma.ca),'))
        contrib.pack_start(gtk.Label('Steve Rawlinson,'))
        contrib.pack_start(gtk.Label('Steve Taylor,'))
        contrib.pack_start(gtk.Label('Paul Lahaie,'))
        contrib.pack_start(gtk.Label('and others'))
        vbox.pack_start(contrib)

        # Funded By
        funding = gtk.VBox(homogeneous=False,spacing=3)
        funding.pack_start(gtk.Label('Funding provided by:'))

        im = gtk.Image()
        im.set_from_file( os.path.join(gview.home_dir, 'pics', 'atlantis_logo.xpm'))
        funding.pack_start(im)
        funding.pack_start(gtk.Label('Atlantis Scientific Inc.'))

        im = gtk.Image()
        im.set_from_file(os.path.join(gview.home_dir,'pics','geo_innovation.xpm'))
        funding.pack_start(im)
        funding.pack_start(gtk.Label('GeoInnovations'))
        vbox.pack_start(funding)

        # Other Info
        vbox.pack_start(gtk.Label('Version: 1.8'))        
        vbox.pack_start(gtk.Label('Web Site:  http://OpenEV.sourceforge.net'))
        vbox.pack_start(gtk.Label('(C) Copyright 2000 Atlantis Scientific Inc.  www.atlantis-scientific.com'))
        
        window.show_all()

    def set_close_function(self,ctype=0):
        """ Set the close behaviour for the view:
            Input parameters:
                ctype- 0 for default behaviour (close view,
                      exit when last one is shut); 1
                      so that window can't be closed
                      at all; 2 so that window will be
                      hidden rather than closed.  Note
                      that the application will have to
                      reset the close function back again
                      for cases 1 and 2 if it wants to
                      close the window using the close function.
                      
        """
        self.close_flag=ctype
    
        
    def close(self, *args):
        # first check for non-standard close settings
        if self.close_flag == 1:
            return True
        elif self.close_flag == 2:
            self.hide()
            return True
        elif self.close_flag == 3:
            # Added this case for running from within ipython.  We want
            # to be able to kill all windows and regenerate them when
            # displaying an image.
            if self.menuf is not None:
                self.app.unsubscribe('rfl-change',self.show_rfl)
            self.destroy()
            return True

        
        # what else do we need to do?
        if len(self.app.view_manager.get_views()) == 1:
            if self.app.request_quit() > 0:
                if self.menuf is not None:
                    self.app.unsubscribe('rfl-change',self.show_rfl)
                # request_quit() sends out the order for the view
                # to be shut.  The destroy() and return False are
                # redundant and sometimes result in errors (project
                # files).
                #self.destroy()
                #return False
                return True
            else:
                return True
        else:
            if self.menuf is not None:
                self.app.unsubscribe('rfl-change',self.show_rfl)
            self.destroy()
            return True

    def hide_entry(self,entrystr='File/Exit'):
        """Hide a menu entry (can be temporary)
           Input: entrystr- eg. 'File/Exit', 'File/Close',...
           Returns 1 for success, 0 for failure.
        """
        try:
            item=self.menuf.find(entrystr)
            if item is not None:
                item.hide()
                return 1
            return 0
        except:
            return 0
        
    def show_entry(self,entrystr='File/Exit'):
        """Re-show a hidden menu entry
           Input: entrystr- eg. 'File/Exit', 'File/Close',...
           Returns 1 for success, 0 for failure.
        """
        try:
            item=self.menuf.find(entrystr)
            if item is not None:
                item.show()
                return 1
            return 0
        except:
            return 0
   
    def exit(self, *args ):
        # should ask for confirmation at this point.
        self.app.request_quit()

    def undo(self, *args):
        self.make_active()
        gview.undo_pop()

    def show_oeattedit(self, *args):
        self.make_active()
        
        import oeattedit

        oeattedit.launch()
        
    def show_layerdlg(self, *args):
        self.layerdlg.show()
        self.layerdlg.window.raise_()

    def show_toolbardlg(self, *args):
        self.make_active()
        self.toolbar.show()
        self.toolbar.window.raise_()

    def goto_dlg(self, *args):
        """ Create the GoTo Dialog box with coordinate system option menu and
        text entry fields """
        window = gtk.Window()
        window.set_title('Go To...')
        window.set_border_width(10)
        vbox = gtk.VBox(homogeneous=False,spacing=15)
        window.add(vbox)

        # Make this a selection menu - doesn't work, yet!
        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(gtk.Label('Coordinate System:'),expand=False)
        self.coord_system_om = gvutils.GvOptionMenu(('Row/Col','Native'),
                                                    self.set_coord_system)
        box.pack_start(self.coord_system_om,expand=False)
        
        # Get current position in view native projection
        #   - changing Option Menu updates this in entry fields

        current_pos = self.viewarea.get_translation()
        
        # X Position
        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(gtk.Label('X Position:'),expand=False)
        x_pos_entry = gtk.Entry()
        x_pos_entry.set_max_length(14)
        x_pos_entry.set_text(str(-current_pos[0]))
        box.pack_start(x_pos_entry,expand=False)

        # Y Position
        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(gtk.Label('Y Position:'),expand=False)
        y_pos_entry = gtk.Entry()
        y_pos_entry.set_max_length(14)
        y_pos_entry.set_text(str(-current_pos[1]))
        box.pack_start(y_pos_entry,expand=False)

        # Button to move
        goto_button = gtk.Button('Go To...')
        goto_button.connect('clicked', self.goto_location )
        vbox.pack_start(goto_button,expand=False)

        self.x_pos_entry = x_pos_entry
        self.y_pos_entry = y_pos_entry

        # set default to be native system - must be after x/y_pos_entry are setup
        self.coord_system_om.set_history(1)
        
        window.show_all()

    def set_coord_system(self, om, *args):
        """ Set coordinate system goto coordinates entered in GoTo dialog from
        option menu. """
        current_pos = self.viewarea.get_translation()
        
        if om.get_history() == 0:
            self.goto_coord_system = 'pixel'
        # Lat/Long Not Working Yet!
        elif om.get_history() == 9:
            self.goto_coord_system = 'lat-long'
        elif  om.get_history() == 1:
            self.goto_coord_system = 'native'
        else:
            self.goto_coord_system = 'native'

    def goto_location(self, Button, *args):
        """ Translate view to location specified in GoTo Dialog, using projection """
        self.make_active()

        coord_system = self.goto_coord_system
        str_x = self.x_pos_entry.get_text()
        str_y = self.y_pos_entry.get_text()

        x = string.atof(str_x)
        y = string.atof(str_y)

        if coord_system == 'pixel':
            # Get current raster
            layer = self.viewarea.active_layer()

            if (layer is None) or (gvutils.is_of_class( layer.__class__, 'GvRasterLayer' ) == 0):
                gvutils.warning('Please select a raster layer using the layer dialog.\n')
                return

            raster = layer.get_parent()
            if ((raster is not None) and (self.viewarea.get_raw(layer) == 0)):
                # layer is georeferenced, coordinates are pixel/line
                position = raster.pixel_to_georef(x,y)
            elif (raster is not None):
                # layer is in pixel/line coordinates, coordinates are pixel/line
                position = (x,y)
            else:
                return
            
        # Doesn't work Yet!
        elif coord_system == 'lat-long':
            position = self.viewarea.map_location((x,y))

        elif coord_system == 'native':
            # native - do nothing
            position = (x,y)

        else:
            print 'Error in gvviewwindow.py function goto_location() passed invalid coordinate system'
            # native - do nothing
            position = (x,y)

        self.viewarea.set_translation(-position[0],-position[1])

    def menu_save_project(self, *args):
        self.app.save_project()

    def menu_save_project_as(self, *args):
        self.app.save_project_as()

    def menu_new_view(self, *args):
        self.app.new_view()

    def save_vector_layer_request( self, *args ):
        self.make_active()

        layer = self.viewarea.active_layer()
        if layer is None or \
           gvutils.is_of_class( layer.__class__, 'GvShapesLayer' ) == 0:
            gvutils.warning('Please select a vector layer using the layer\n'+\
                            'dialog before attempting to save.' )
            return

        pgufilesel.SimpleFileSelect( self.save_vector_layer_with_file,
                                     cb_data = layer.get_parent(),
                                     title = 'Save Shapefile',
                                     default_filename = layer.get_name() )

    def save_vector_layer_with_file( self, filename, shapes_data ):
        if shapes_data.save_to( filename ) == 0:
            gvutils.error('Unable to save vectors to:'+filename)

    def destroy_preferences(self,*args):
        self.pref_dialog = None
        
    def file_open_shape_by_name(self, filename):
        self.make_active()
        shape_data = gview.GvShapes(shapefilename=filename)
        if shape_data is None:
            gvutils.error('Unable to open '+filename+' for loading.')
            return
        
        self.app.add_to_rfl(filename)
        gview.undo_register(shape_data)
        
        layer = gview.GvShapesLayer( shape_data )
        layer.set_name(filename)
        self.viewarea.add_layer(layer)
        self.viewarea.set_active_layer(layer)

    def file_open_ogr_by_name(self, filename):
        import ogr
        import gvogrdlg

        self.make_active()

        hDS = ogr.Open( filename )
        if hDS is None:
            return False

        self.app.add_to_rfl(filename)
        
        dlg = gvogrdlg.GvOGRDlg(hDS, self )

        return True
    
    def file_open_ogr_by_layer(self, layer):

        import _gv

        raw_data = _gv.gv_shapes_from_ogr_layer( layer )
        if raw_data is None:
            return False

        shape_data = gview.GvShapes(_obj=raw_data)
        if shape_data is None:
            return

        if len(shape_data) > 0:
            gview.undo_register(shape_data)
        
            layer = gview.GvShapesLayer( shape_data )
            self.viewarea.add_layer(layer)
            self.viewarea.set_active_layer(layer)
        else:
            # I am not sure how to blow away the GvShapes properly.
            pass
                
        return True

    def file_import_cb(self, *args):
        self.make_active()
        pgufilesel.SimpleFileSelect( self.file_import_by_name, None,
                                     'File To Import',
                                     help_topic = 'files.html' )

    def file_import_by_name( self, filename, *args ):
        self.make_active()
        dataset = gdal.Open( filename )
        if dataset is None:
            gvutils.error('Unable to open '+filename+' for import.')
            return

        geotiff = gdal.GetDriverByName("GTiff")
        if geotiff is None:
            gvutils.error("Yikes!  Can't find GeoTIFF driver!")
            return

        newbase, ext = os.path.splitext(filename)
        newfile = newbase + ".tif"
        i = 0
        while os.path.isfile(newfile):
            i = i+1
            newfile = newbase+"_"+str(i)+".tif"

        progress = pguprogress.PGUProgressDialog( 'Import to '+newfile,
                                                  cancel = True )
        progress.SetDefaultMessage( "translated" )

        old_cache_max = gdal.GetCacheMax()
        if old_cache_max < 20000000:
            gdal.SetCacheMax( 20000000 )
        
        new_dataset = geotiff.CreateCopy( newfile, dataset, False,
                                          ['TILED=YES',],
                                          callback = progress.ProgressCB )
        dataset = None

        if progress.cancelled:
            progress.destroy()
            if os.path.isfile(newfile):
                os.unlink(newfile)
            gdal.SetCacheMax( old_cache_max );
            return
            
        if new_dataset == None:
            progress.destroy()
            gvutils.error('Unable to translate '+filename+' to '+newfile)
            if os.path.isfile(newfile):
                os.unlink(newfile)
            gdal.SetCacheMax( old_cache_max );
            return

        progress.SetDefaultMessage( "overviews built" )
        new_dataset.BuildOverviews( "average", callback = progress.ProgressCB )
        new_dataset = None

        progress.destroy()

        gdal.SetCacheMax( old_cache_max );

        # open normally
        self.file_open_by_name( newfile, sds_check=0 )


    def file_open_cb(self, *args):
        self.make_active()

	if gview.get_preference('save_recent_directory') == 'on':
	    recent_dir = gview.get_preference('recent_directory')
	else:
	    recent_dir = None

	pgufilesel.SimpleFileSelect( ok_cb = self.file_open_name_check,
                                     title = 'File Open',
				     default_filename = recent_dir,
                                     help_topic = 'files.html' )

# buffer function to check for wild cards in filename and then expand them
    def file_open_name_check(self, filename, lut=None,*args):
        if ('*' in filename)or('?' in filename):
            for file in glob.glob(filename):
               self.file_open_by_name(file)
        else:
            self.file_open_by_name(filename)

    def open_subdataset_check( self, dataset ):
        import gvsdsdlg
        dlg = gvsdsdlg.GvSDSDlg(dataset, self)

    def file_open_ap_envisat(self, dataset ):
        options = []
        if gview.get_preference('gcp_warp_mode') is not None \
           and gview.get_preference('gcp_warp_mode') == 'no':
            options.append(('raw','yes'))

        md = dataset.GetMetadata()
        try:
            md1 = md['SPH_MDS1_TX_RX_POLAR']
            md2 = md['SPH_MDS2_TX_RX_POLAR']
        except:
            md1 = ''
            md2 = ''

        raster1 = gview.manager.get_dataset_raster(dataset,1)
        raster2 = gview.manager.get_dataset_raster(dataset,2)

        rl1 = gview.GvRasterLayer(raster1, options, rl_mode=gview.RLM_AUTO )
        rl1.set_name( 'MDS1: ' + md1 )
        rl2 = gview.GvRasterLayer(raster2, options, rl_mode=gview.RLM_AUTO )
        rl2.set_name( 'MDS2: ' + md2 )

        # Add MDS1 to the current view window. 

        self.viewarea.add_layer(rl1)
        self.viewarea.set_active_layer(rl1)
        self.rawgeo_update()
        self.set_title( self.view_title + ': MDS1- ' + md1 )

        # Create a new view window and add MDS2 to it.

        view2 = self.app.new_view()
        view2.viewarea.add_layer(rl2)
        view2.viewarea.set_active_layer(rl2)
        view2.rawgeo_update()
        view2.set_title( view2.title + ': MDS2- ' + md2 )

        # Setup link between views.
        link = gview.GvViewLink()
        link.register_view( self.viewarea )
        link.register_view( view2.viewarea )
        link.enable()

    def open_gdal_dataset(self, dataset, lut=None, sds_check=1, \
			  add_to_rfl=0, *args):
	"""Opens existing GDAL dataset."""

        self.make_active()

        dataset = gview.manager.add_dataset(dataset)
	if dataset is None:
	    return

        if sds_check and len(dataset.GetSubDatasets()) > 0:
            self.open_subdataset_check( dataset )
            return

	if add_to_rfl:
	    self.app.add_to_rfl(dataset.GetDescription())

        md = dataset.GetMetadata()
        # special hack for displaying AP envisat specially.
        if md.has_key('MPH_PHASE') and dataset.RasterCount == 2:
            self.file_open_ap_envisat( dataset )
            return

        raster = gview.manager.get_dataset_raster(dataset,1)
        options = []
        if gview.get_preference('gcp_warp_mode') is not None \
           and gview.get_preference('gcp_warp_mode') == 'no':
            options.append(('raw','yes'))

        if lut:
            raster_layer = gview.GvRasterLayer(raster, options,
                                               rl_mode = gview.RLM_SINGLE )
            raster_layer.lut_put(lut)

        elif dataset.RasterCount > 2:
            raster_layer = gview.GvRasterLayer(raster, options,
                                               rl_mode = gview.RLM_RGBA )
        elif dataset.RasterCount == 2 and \
             dataset.GetRasterBand(2).GetRasterColorInterpretation() == gdal.GCI_AlphaBand:
            raster_layer = gview.GvRasterLayer(raster, options,
                                               rl_mode = gview.RLM_RGBA )
        elif dataset.RasterCount < 3 and \
            (dataset.GetRasterBand(1).GetRasterColorInterpretation() == gdal.GCI_PaletteIndex or
            dataset.GetRasterBand(1).GetRasterColorInterpretation() == gdal.GCI_HueBand):
            raster_layer = gview.GvRasterLayer(raster, options, rl_mode = gview.RLM_PSCI )
        else:
            raster_layer = gview.GvRasterLayer(raster, options,
                                               rl_mode = gview.RLM_AUTO )
        raster_layer.set_name(dataset.GetDescription())

        #
        # Note: We now set source for initial raster (0) as well, as set_source
        # will apply a default lut, if specified in band metadata.
        #
        raster_layer.set_source(0, raster)

        # Logic to handle PSCI layers
        if raster_layer.get_mode() == gview.RLM_PSCI:
            if dataset.RasterCount > 1:
                intensity_raster = gview.manager.get_dataset_raster(dataset,2)
                raster_layer.set_source(1, intensity_raster)
            else:
                intensity = 255
                if md.has_key(gvconst.GV_PSCI_INTENSITY):
                    intensity = float(md.get(gvconst.GV_PSCI_INTENSITY))
                raster_layer.set_source(1, None, const_value=intensity)

        # Lots of logic to handle RGB and RGBA Layers
        if raster_layer.get_mode() == gview.RLM_RGBA \
           and dataset.RasterCount == 2:
            
            alpha_band = gview.manager.get_dataset_raster(dataset,2)
            raster_layer.set_source(1,raster)
            raster_layer.set_source(2,raster)
            raster_layer.set_source(3,alpha_band)

            raster_layer.blend_mode_set( gview.RL_BLEND_FILTER )

        if raster_layer.get_mode() == gview.RLM_RGBA \
           and dataset.RasterCount > 2:
            
            green_raster = gview.manager.get_dataset_raster(dataset,2)
            blue_raster = gview.manager.get_dataset_raster(dataset,3)

            raster_layer.set_source(1,green_raster)
            raster_layer.set_source(2,blue_raster)

            if dataset.RasterCount > 3:
                band = dataset.GetRasterBand(4)
                if band.GetRasterColorInterpretation() == gdal.GCI_AlphaBand:
                    raster_layer.blend_mode_set( gview.RL_BLEND_FILTER )
                    alpha_raster = \
                        gview.manager.get_dataset_raster(dataset, 4)
                    raster_layer.set_source(3,alpha_raster)

        self.viewarea.add_layer(raster_layer)
        self.viewarea.set_active_layer(raster_layer)
        self.rawgeo_update()

    def get_gdal_raster_by_gci_type(self, dataset, gci_type):
        band_index = 0
        while band_index < dataset.RasterCount:
            raster = dataset.GetRasterBand(band_index + 1)
            if raster.GetRasterColorInterpretation() == gci_type:
                return raster
            band_index += 1
        return None

    def file_open_by_name(self, filename, lut=None, sds_check=1, *args):
        head = os.path.dirname(filename)
        if len(head) > 0:
            if os.access(head,os.R_OK):
                pgufilesel.simple_file_sel_dir = head+os.sep

	if gview.get_preference('save_recent_directory') == 'on':
	    gview.set_preference('recent_directory', head+os.sep)

        if gvutils.is_shapefile(filename):
            self.file_open_shape_by_name(filename)
            return

        if gvutils.is_project_file(filename):
            self.app.load_project(filename)
            return

        dataset = gview.manager.get_dataset(filename)
        if dataset is None \
               and gdal.GetLastErrorNo() != 4 \
               and gdal.GetLastErrorNo() != 0:
            gvutils.error( 'Unable to open '+filename+'\n\n' \
                           + gdal.GetLastErrorMsg() )
            return

        # catch ogr file open failure and pop up
        # a warning rather than dumping to screen.
        try:
            if dataset is None and self.file_open_ogr_by_name(filename):
                return

            if dataset is None:
                gvutils.error('Unable to open '+filename+'\n\n' \
                              + gdal.GetLastErrorMsg() )
                return
        except:
            if dataset is None:
                gvutils.error('Unable to open '+filename+'\n\n' \
                              + gdal.GetLastErrorMsg() )
                return

	self.open_gdal_dataset(dataset, lut, sds_check, add_to_rfl=1)

    def init_custom_icons(self):
        pass
    
    def init_default_icons(self):
        # Zoom ratio selection box

        zoom_factor = gtk.combo_box_entry_new_text()
        for item in ratio_list:
            zoom_factor.append_text(item)
        zoom_factor.child.set_text('1:1')
        zoom_factor.child.set_width_chars(5)
        self.zoom_entry_changed_id = zoom_factor.child.connect('changed', self.set_zoom_factor_cb)
        self.zoom_factor = zoom_factor

        # raw / georeferenced pixmap
        self.show()
        self.raw_icon = gtk.gdk.pixmap_create_from_xpm(self.window,None,
                   os.path.join(gview.home_dir,'pics', 'worldg.xpm'))
        self.geo_icon = gtk.gdk.pixmap_create_from_xpm(self.window,None,
                   os.path.join(gview.home_dir,'pics', 'worldrgb.xpm'))
        self.rawgeo_pixmap = gtk.Image()

        # idle / busy pixmap
        self.idle_icon = gtk.gdk.pixmap_create_from_xpm(self.window,None,
                   os.path.join(gview.home_dir,'pics', 'idle.xpm'))
        self.busy_icon = gtk.gdk.pixmap_create_from_xpm(self.window,None,
                   os.path.join(gview.home_dir,'pics', 'busy.xpm'))

        self.idlebusy_pixmap = gtk.Image()

        gview.manager.connect('busy-changed', self.busy_changed_cb)

    def create_iconbar(self, iconfile='DefaultIconFile.xml'):
        self.iconbar = gtk.Toolbar()

        self.init_default_icons()
        self.init_custom_icons()

        self.icon_cmds = None
        if iconfile is not None:
            # check that icon file exists.  If not, use old values for
            # backwards compatibility
            fulliconfile = os.path.join(gview.home_dir,'xmlconfig',iconfile)
            if os.path.isfile(fulliconfile) == 1:
                if (self.app is not None) and hasattr(self.app,'load_icons_file_from_xml'):
                    icon_cmds=self.app.load_icons_file_from_xml( iconfile )
                else:
                    icon_cmds = self.load_icons_file_from_xml( iconfile )
                self.icon_cmds = icon_cmds
            else:
                raise AttributeError,'Unable to find view icon configuration file '+iconfile
        else:
            self.icon_cmds=self.old_icon_cmds()

        for cmd in self.icon_cmds:
            exec cmd
            
        gview.manager.set_busy(True)

    def old_icon_cmds(self):
        icon_cmds=[]
        icon_cmds.append("self.add_icon_to_bar( 'openfile.xpm', None,'Open and Display Raster/Vector File',self.file_open_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'print.xpm', None,'Print Current View',self.print_cb, 'gvprint.html' )")

        icon_cmds.append("self.add_icon_to_bar( 'nonelut.xpm', None,'Revert to no Enhancement',self.nonelut_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'linear.xpm', None,'Linear Stretch/Enhancement',self.linear_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'equalize.xpm', None,'Apply Equalization Enhancement to Raster',self.equalize_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'log.xpm', None,'Logarithmic Enhancement to Raster',self.log_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'windowed.xpm', None,'Windowed Raster Re-enhancement',self.restretch_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'classify.xpm', None,'Classify Raster',self.classify_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'legend.xpm', None,'Show Legend',self.show_legend_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'seeall.xpm', None,'Fit All Layers',self.seeall_cb )")

        # Zoom ratio selection box
        icon_cmds.append("self.iconbar.append_widget(self.zoom_factor, 'Zoom Ratio', 'Zoom Ratio')")

        icon_cmds.append("self.add_icon_to_bar( 'zoomin.xpm', None,'Zoom in x2',self.zoomin_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'zoomout.xpm', None,'Zoom out x2',self.zoomout_cb )")

        icon_cmds.append("self.add_icon_to_bar( 'refresh.xpm', None,'Refresh Rasters From Disk',self.refresh_cb )")

        # raw / georeferenced pixmap
        icon_cmds.append("self.iconbar.append_item(None, 'Georeferenced','Georeferenced', self.rawgeo_pixmap,self.rawgeo_cb )")

        # Help
        icon_cmds.append("self.add_icon_to_bar( 'help.xpm', None,'Launch Online Help',self.helpcb )")

        # idle / busy pixmap
        icon_cmds.append("self.iconbar.append_item(None, 'Busy Indicator','Busy Indicator', self.idlebusy_pixmap,self.do_nothing )")

        return icon_cmds
       
    def create_menubar(self, menufile='DefaultMenuFile.xml'):
        self.menuf = gvutils.GvMenuFactory()

        self.menu_cmd = None        
        if menufile is not None:
            # Check that menu file exists.  If not, load old menu
            # for backwards comaptibility.
            fullmenufile = os.path.join(gview.home_dir,'xmlconfig',menufile)
            if os.path.isfile(fullmenufile) == 1:
                if ((self.app is not None) and hasattr(self.app,'load_menus_file_from_xml')):
                    # Application parses the xml file itself
                    menu_cmd=self.app.load_menus_file_from_xml( menufile, self.view_title )
                else:
                    menu_cmd = self.load_menus_file_from_xml( menufile )
                self.menu_cmd = menu_cmd
                #print menu_cmd
            else:
                raise AttributeError,'Unable to find view menu configuration file '+menufile
        else:
            self.menu_cmd = self.old_menu_cmd()

        exec self.menu_cmd
        self.add_accel_group(self.menuf.accelerator)

        if self.app is not None:
            self.app.subscribe('rfl-change',self.show_rfl)


    def old_menu_cmd(self):
        menu_cmd = "self.menuf.add_entries([" + \
                "('File/Import', None, self.file_import_cb )," + \
                "('File/Open', '<control>O', self.file_open_cb )," + \
                "('File/Open 3D', None, self.open_3D_request)," + \
                "('File/Save Vector Layer', None, self.save_vector_layer_request)," + \
                "('File/Save Project', None, self.menu_save_project)," + \
                "('File/New View', None, self.menu_new_view)," + \
                "('File/Print', None, self.print_cb)," + \
                "('File/<separator>', None, None)," + \
                "('File/rfl1', None, self.rfl_cb, 1)," + \
                "('File/rfl2', None, self.rfl_cb, 2)," + \
                "('File/rfl3', None, self.rfl_cb, 3)," + \
                "('File/rfl4', None, self.rfl_cb, 4)," + \
                "('File/rfl5', None, self.rfl_cb, 5)," + \
                "('File/<separator>', None, None)," + \
                "('File/Close', None, self.close)," + \
                "('File/Exit', '<control>Q', self.exit)," + \
                "('Edit/Undo', None, self.undo)," + \
                "('Edit/Layers...', None, self.app.show_layerdlg)," + \
                "('Edit/Vector Layer Attributes...', None, self.show_oeattedit)," + \
                "('Edit/Edit Toolbar...', None, self.app.show_toolbardlg)," + \
                "('Edit/Go To...', None, self.goto_dlg)," + \
                "('Edit/Python Shell...', None, self.pyshell)," + \
                "('Edit/3D Position...', None, self.position_3d)," + \
                "('Edit/Preferences...', None, self.app.launch_preferences)," + \
                "('Help/Help...', None, self.helpcb, 'openevmain.html')," + \
                "('Help/<separator>', None, None)," + \
                "('Help/Web Page...', None, self.helpcb,'http://OpenEV.Sourceforge.net/')," + \
                "('Help/About...', None, self.aboutcb)])"

        return menu_cmd

    def load_menus_file_from_xml(self, menufile='DefaultMenuFile.xml'):
        # load in menu file and populate menu entries
        menufile = os.path.join(gview.home_dir,'xmlconfig',menufile)
        menu_list = []
        try:
            raw_xml = open(menufile).read()
        except:
            raise AttributeError,"Unable to load " + menufile
            return

        tree = gdal.ParseXMLString( raw_xml )
        if tree is None:
            raise AttributeError,"Problem occured parsing menu file " + menufile
            return

        if tree[1] != 'GViewAppMenu':
            raise AttributeError,"Root of %s is not GViewAppMenu node " % menufile
            return

        # loop over entries getting path,accelerator,callback and arguments
        menu_trees = gvutils.XMLFind( tree, 'Menu')
        if menu_trees is None:
            raise AttributeError,"Invalid menu file format"
      
        for node in menu_trees[2:]:
            if node[1] == 'entry':
                node_path  = gvutils.XMLFind( node, 'path')
                if node_path is None:
                    raise AttributeError,"Invalid menu file format - missing path"
                 
                entry_type = gvutils.XMLFindValue( node_path, 'type', '')
                entry_path = gvutils.XMLFindValue( node, 'path','')
                
                if (string.find(entry_path,"/") == -1):
                    raise AttributeError,"Invalid menu file format - bad path:%s" % entry_path
                    
                if (entry_type != ''):
                    entry_type = "<" + entry_type + ">"
                path_split=string.split(entry_path,"/")
                path_split[-1] = entry_type + path_split[-1]
                entry_path=string.join(path_split,"/")

                entry_accelerator = gvutils.XMLFindValue( node, 'accelerator', 'None')
                if (entry_accelerator != 'None'):
                    (key,mod) = string.split(entry_accelerator,'+')
                    entry_accelerator = "'<" + key + ">" + mod + "'"

                entry_callback = gvutils.XMLFindValue( node, 'callback', 'None')
                entry= "("                                             \
                        + string.join((entry_path,entry_accelerator,   \
                                       entry_callback),",")

                arguments = gvutils.XMLFind( node, 'arguments')
                if arguments is not None:
                    args_list = []
                    args =  gvutils.XMLFind( arguments, 'arg','')
                    if args is not None:
                        for arg in args:
                            args_list.append(gvutils.XMLFindValue( arg, '',''))
                        entry = entry + "," + string.join(args_list,",")

                entry = entry + ")"

                menu_list.append(entry)
            else:
                raise AttributeError,"Invalid menu file format"
            
        # create the menu command to populate the entries
        menu_cmd =  "self.menuf.add_entries([" + string.join(menu_list,',') + "])"
        return menu_cmd


    def load_icons_file_from_xml(self, iconfile='DefaultIconFile.xml'):
        #print "LOADING ICONS"
        # load in icon file and create icon commands
        iconfile = os.path.join(gview.home_dir,'xmlconfig',iconfile)
        icon_list = []
        try:
            raw_xml = open(iconfile).read()
        except:
            raise AttributeError,"Unable to load " + iconfile
            return

        tree = gdal.ParseXMLString( raw_xml )
        if tree is None:
            raise AttributeError,"Problem occured parsing icon file " + iconfile
            return

        if tree[1] != 'GViewAppIconBar':
            raise AttributeError,"Root of %s is not GViewAppIconBar node " % iconfile
            return

        # loop over entries getting icon,label,hint,callback and help
        icon_trees = gvutils.XMLFind( tree, 'Iconbar')
        if icon_trees is None:
            raise AttributeError,"Invalid icon file format"
        
        for node in icon_trees[2:]:
            if node[1] == 'icon':
                type = None
                icon_label = gvutils.XMLFindValue( node, 'label','None')
                icon_hint = gvutils.XMLFindValue( node, 'hint','None')
                icon_callback = gvutils.XMLFindValue( node, 'callback','None')
                icon_help = gvutils.XMLFindValue( node, 'help','None')
                icon_file = gvutils.XMLFindValue( node, 'xpm','None')
                # xpm files - need to add path and possible help
                if (icon_file != 'None'):
                    type = 'xpm'
                    icon = "self.add_icon_to_bar("                           \
                            + string.join((icon_file,icon_label,icon_hint,   \
                                           icon_callback,icon_help),",")     \
                            + ")" 

                # pixmap files - not adding path or help 
                icon_file = gvutils.XMLFindValue( node, 'pixmap','None')
                if (icon_file!= 'None'):
                    type = 'pixmap'
                    icon = "self.iconbar.append_item("                        \
                            + string.join((icon_label,icon_hint,icon_hint,    \
                                              icon_file,icon_callback),",")   \
                            + ")" 

                # widget  
                icon_file = gvutils.XMLFindValue( node, 'widget','None')
                if (icon_file!= 'None'):
                    type = 'widget'
                    icon_file = gvutils.XMLFindValue( node, 'widget','None')
                    icon = "self.iconbar.append_widget("                       \
                            + string.join((icon_file,icon_hint,icon_hint),",") \
                            + ")" 
                # none of the above
                if type is None:
                    raise AttributeError,"Invalid icon file format - unknown type"

                icon_list.append(icon)
            else:
                raise AttributeError,"Invalid icon file format"


        return icon_list

    def do_nothing(self, *args):
        pass
    
    def add_icon_to_bar(self, filename, text, hint_text, cb, help_topic=None):
        full_filename = os.path.join(gview.home_dir,'pics',filename)
        pix = gtk.Image()
        pix.set_from_file(full_filename)
        item = self.iconbar.append_item(text, hint_text, hint_text, pix, cb)
        if help_topic is not None:
            gvhtml.set_help_topic(item, help_topic)

    def insert_tool_icon(self, filename, text, hint_text, cb, help_topic=None, pos=0):
        # Tool specifies full filename (file may not be in pics directory)
        
        pix = gtk.Image()
        pix.set_from_file(filename)
        item = self.iconbar.append_item(text, hint_text, hint_text, pix, cb)
        if help_topic is not None:
            gvhtml.set_help_topic(item, help_topic)

    def classify_cb(self, *args):
        self.make_active()
        if hasattr(self.viewarea.active_layer(),'get_mode'):
            if self.viewarea.active_layer().get_mode() == gview.RLM_COMPLEX:
                gvutils.warning('Complex rasters cannot be classified!')
            elif self.viewarea.active_layer().get_mode() == gview.RLM_RGBA:
                gvutils.warning('Multiband rasters cannot be classified!')
            else:
                self.viewarea.active_layer().classify()
        else:
            self.viewarea.active_layer().classify()
        
    def show_legend_cb(self, *args):
        print "------------------ showing legend"
        self.make_active()
        if self.viewarea.active_layer() is not None:
            self.viewarea.active_layer().show_legend()

    def restretch_cb(self, *args):
        self.make_active()
        try:
            self.viewarea.active_layer().window_restretch()
        except:
            gvutils.warning('This can only be applied to a raster layer.\n' \
                          + 'Select a raster layer for this view in the \nlayers dialog.' )
        
    def equalize_cb(self, *args):
        self.make_active()
        try:
            self.viewarea.active_layer().equalize()
        except:
            gvutils.warning('This can only be applied to a raster layer.\n' \
                          + 'Select a raster layer for this view in the \nlayers dialog.' )
            #import traceback
            #traceback.print_exc()
        
    def linear_cb(self, *args):
        self.make_active()
        try:
            self.viewarea.active_layer().linear()
        except:
            gvutils.warning('This can only be applied to a raster layer.\n' \
                          + 'Select a raster layer for this view in the \nlayers dialog.' )

    def log_cb(self, *args):
        self.make_active()
        try:
            self.viewarea.active_layer().log()
        except:
            gvutils.warning('This can only be applied to a raster layer.\n' \
                          + 'Select a raster layer for this view in the \nlayers dialog.' )
        
    def nonelut_cb(self, *args):
        self.make_active()
        try:
            self.viewarea.active_layer().none_lut()
        except:
            gvutils.warning('This can only be applied to a raster layer.\n' \
                          + 'Select a raster layer for this view in the \nlayers dialog.' )

    def seeall_cb(self,*args):
        self.make_active()
        try:
            self.viewarea.fit_all_layers()
        except:
            pass
        
    def onetoone_cb(self,*args):
        self.make_active()
        try:
            view = self.viewarea
            layer = view.active_layer()
            point1 = view.inverse_map_pointer(layer.pixel_to_view( 0, 0 ))
            point2 = view.inverse_map_pointer(layer.pixel_to_view( 1, 1 ))
            dist = math.sqrt(math.pow((point1[0]-point2[0]),2)
                             + math.pow((point1[1]-point2[1]),2))
            factor = dist / math.sqrt(2)
            view.zoom(-1 * (math.log(factor) / math.log(2)) )
        except:
            gvutils.warning('This operation can only be done if a raster layer is the\nactive layer.  Please select a raster layer for this view in the layers dialog.')

    def set_zoom_factor_focus_cb(self,*args):
        """ Keep the focus in the view window when we selected a new zooming ratio from
        the combo box.  To ensure key events still recieved such as Home, and arrows """
        self.viewarea.grab_focus()
    
    def set_zoom_factor_cb(self,*args):
        self.make_active()
        
        if self.zoom_factor is None:
            # if zoom factor icon doesn't exist (no iconbar)
            return
        
        try:
            ratio_text = string.split(self.zoom_factor.child.get_text(), ':')
            ratio = [string.atof(ratio_text[0]), string.atof(ratio_text[1])]
        except:
            # if invalid text entered do nothing
            return

        # Make sure both values are positive
        if (ratio[0] <= 0.0) or (ratio[1] <= 0.0):
            return

        try:
            view = self.viewarea
            layer = view.active_layer()
            point1 = view.inverse_map_pointer(layer.pixel_to_view( 0, 0 ))
            point2 = view.inverse_map_pointer(layer.pixel_to_view( 1, 1 ))
            dist = math.sqrt(math.pow((point1[0]-point2[0]),2)
                             + math.pow((point1[1]-point2[1]),2))
            factor = dist / math.sqrt(2)
            self.zoom = factor
        
            # Block view-state-changed signal while we update zoom factor
            view.handler_block(self.view_state_changed_id)
            view.zoom(-1 * (math.log((ratio[1]/ratio[0])*factor) / math.log(2)) )
            view.handler_unblock(self.view_state_changed_id)

            self.zoom_flag = 'yes'
        except:
            # To prevent multiple error messages use the zoom_flag to keep track if we
            # have successfully zoomed since last error message, if not then we don't
            # generate another one
            if self.zoom_flag == 'yes':
                self.zoom_flag = 'no'
                gvutils.warning('This operation can only be done if a raster layer is the\nactive layer.  Please select a raster layer for this view\nin the layers dialog.')

        self.viewarea.grab_focus()
          
    def refresh_cb(self, *args):
        self.make_active()
        try:
            layer = self.viewarea.active_layer()
            for isource in range(4):
                raster = layer.get_data(isource)
                if raster is not None:
                    raster.changed()
        except:
            gvutils.warning('The refresh from disk operation can only be\n'+\
                            'applied to raster layers.  Select a raster\n'+\
                            'layer for this view in the layers dialog.')
            pass
        
    def zoomin_cb(self,*args):
        self.make_active()
        try:
            self.viewarea.zoom(1)
        except:
            pass
        
    def zoomout_cb(self,*args):
        self.make_active()
        try:
            self.viewarea.zoom(-1)
        except:
            pass

    def update_zoom_cb(self, *args):
        # Note: we do NOT use temp_zoom to get the zooming factor (doesn't work for
        #       geo-referenced images) we just use it as a way to check if the zoom
        #       has changed and therefore if we have to do the calculation
        temp_zoom = self.viewarea.get_zoom()
        layer = self.viewarea.active_layer()

        if self.zoom_factor is None:
            return False 

        try:
            # Check if zoom factor changed
            if (temp_zoom != self.zoom) and (layer is not None):
                self.zoom = temp_zoom
                self.zoom_flag = 'yes'

                point1 = self.viewarea.inverse_map_pointer(layer.pixel_to_view( 0, 0 ))
                point2 = self.viewarea.inverse_map_pointer(layer.pixel_to_view( 1, 1 ))
                dist = math.sqrt(math.pow((point1[0]-point2[0]),2)
                                 + math.pow((point1[1]-point2[1]),2))
                factor = dist / math.sqrt(2)
            
                if (factor > 1):
                    ratio = str(int(round((factor/1.0),1))) + ':1'
                else:
                    ratio = '1:' + str(int(round(1.0/factor,1)))

                # Block combo box changed signal while we update the text entry
                self.zoom_factor.child.handler_block(self.zoom_entry_changed_id)
                self.zoom_factor.child.set_text(ratio)
                self.zoom_factor.child.handler_unblock(self.zoom_entry_changed_id)
        except:
            self.zoom_factor.child.handler_block(self.zoom_entry_changed_id)
            self.zoom_factor.child.set_text('?:?')
            self.zoom_factor.child.handler_unblock(self.zoom_entry_changed_id)

        # Put focus back into view window
        self.viewarea.grab_focus()

        # Return false to continue propogation of the view-state-changed signal
        return False   

        
    def pyshell(self, *args):
        self.make_active()
        self.app.pyshell()
        
    # -------- 3D File Open and Setup --------
    def open_3D_request(self, *args):
        """ 3D File Open Dialog for selecting drape and height data """
        self.make_active()
        self.drape_dataset = None
        self.DEM_dataset = None

        # Create Dialog Window
        dialog = gtk.Window()
        dialog.set_title('Open 3D')
        dialog.set_border_width(10)
        dialog.set_resizable(False)
        gvhtml.set_help_topic( dialog, 'open3d.html' )
        
        box = gtk.VBox(homogeneous=False, spacing=5)
        dialog.add(box)
        self.file_dialog_3D = dialog
        
        # Drape File Selector
        drape_label = gtk.Label('Select Drape')
        box.pack_start(drape_label)

        self.drape_fileSelectWin = gtk.FileSelection()
        zsChildren = self.drape_fileSelectWin.children()[0].children() 
        for zsChild in zsChildren : zsChild.reparent(box)

        # DEM File Selector
        ruler1 = gtk.HSeparator()
        box.pack_start(ruler1)
        DEM_label = gtk.Label('Select DEM')
        box.pack_start(DEM_label)

        self.DEM_fileSelectWin = gtk.FileSelection()
        zsChildren = self.DEM_fileSelectWin.children()[0].children() 
        for zsChild in zsChildren : zsChild.reparent(box)

        # Mesh LOD and Height Scale
        mesh_opts = gtk.HBox(homogeneous=False, spacing=5)
        lod_label =  gtk.Label('Mesh Level of Detail')
        spin_adjust = gtk.Adjustment(value=3, lower=0, upper=8, step_incr=1)
        self.lod_spin_button = gtk.SpinButton(spin_adjust, climb_rate=1, digits=0)

        hscale_label = gtk.Label('Height Scaling Factor:')
        self.scale_value = gtk.Entry()
        self.scale_value.set_max_length(7)
        self.scale_value.set_max_length(7)
        self.scale_value.set_text('1.0')
        
        mesh_opts.pack_start(lod_label)
        mesh_opts.pack_start(self.lod_spin_button)
        mesh_opts.pack_start(hscale_label)
        mesh_opts.pack_start(self.scale_value)
        box.pack_start(mesh_opts)

        # DEM height clamping options
        min_clamp_opts = gtk.HBox(homogeneous=True,spacing=5)
        self.min_heightclamp_entry = gtk.Entry()
        self.min_heightclamp_entry.set_max_length(10)
        self.min_heightclamp_entry.set_text('0.0')
        min_clamp_label = gtk.Label('Minimum Height:')
        self.min_heightclamp_toggle = gtk.CheckButton('Clamp Minimum Height')
        self.min_heightclamp_toggle.set_active(False)
        min_clamp_opts.pack_start(self.min_heightclamp_toggle)
        min_clamp_opts.pack_start(min_clamp_label)
        min_clamp_opts.pack_start(self.min_heightclamp_entry)
        box.pack_start(min_clamp_opts)

        max_clamp_opts = gtk.HBox(homogeneous=True,spacing=5)
        self.max_heightclamp_entry = gtk.Entry()
        self.max_heightclamp_entry.set_max_length(10)
        self.max_heightclamp_entry.set_text('100000.0')
        max_clamp_label = gtk.Label('Maximum Height:')
        self.max_heightclamp_toggle = gtk.CheckButton('Clamp Maximum Height')
        self.max_heightclamp_toggle.set_active(False)
        max_clamp_opts.pack_start(self.max_heightclamp_toggle)
        max_clamp_opts.pack_start(max_clamp_label)
        max_clamp_opts.pack_start(self.max_heightclamp_entry)
        box.pack_start(max_clamp_opts)

        # Okay/Cancel Buttons
        buttons = gtk.HBox(homogeneous=False, spacing=5)
        okay = gtk.Button('OK')
        okay.set_size_request(64, 32)
        okay.connect('clicked', self.perform_3D_request)
        
        cancel = gtk.Button('Cancel')
        cancel.set_size_request(64, 32)
        cancel.connect('clicked', dialog.destroy)

        help = gtk.Button('Help')
        help.set_size_request(64, 32)
        help.connect('clicked', self.helpcb, 'open3d.html')

        buttons.pack_end(help, expand=False)
        buttons.pack_end(cancel, expand=False)
        buttons.pack_end(okay, expand=False)
        box.pack_start(buttons, expand=False)

        # Show everything but unused fileselection buttons
        dialog.show_all()
        box.children()[1].hide()  # Remove Drape Create/Delete/Rename 
        box.children()[6].hide()  # Remove Drape Ok/Cancel
        box.children()[9].hide()  # Remove DEM Create/Delete
        box.children()[14].hide() # Remove DEM Ok/Cancel
        

    def perform_3D_request(self, *args):
        """Tries to open selected files, then creates 3D Layer and switches to 3D mode"""
        drape_filename = self.drape_fileSelectWin.get_filename()
        dem_filename = self.DEM_fileSelectWin.get_filename()
        mesh_lod = self.lod_spin_button.get_value_as_int()
        hscale = float(self.scale_value.get_text())

        if (self.min_heightclamp_toggle.get_active() == True):
            min_clamp = float(self.min_heightclamp_entry.get_text())
        else:
            min_clamp = None

        if (self.max_heightclamp_toggle.get_active() == True):
            max_clamp = float(self.max_heightclamp_entry.get_text())
        else:
            max_clamp = None

        # Do real work.
        self.view3d_action( dem_filename, drape_filename, mesh_lod, hscale,
                            min_clamp, max_clamp )

        # Clean up File Dialog Window
        self.file_dialog_3D.destroy()

    def view3d_action( self, dem_filename, drape_filename = None,
                       mesh_lod = None, hscale = None,
                       min_clamp = None, max_clamp = None ):

        self.make_active()
        gview.manager.set_busy(True)

        # Fill default parameters.
        if drape_filename is None:
            drape_filename = dem_filename
        if mesh_lod is None:
            mesh_lod = 3
        if hscale is None:
            hscale = 1.0
        
        # Get Data
        drape_dataset = gview.manager.get_dataset(drape_filename)
        if drape_dataset is None or drape_dataset._o is None:
            gvutils.error( 'Unable to open drape dataset: '+drape_filename)
            return

        DEM_dataset = self.raster_open_by_name(dem_filename)
        if DEM_dataset is None or DEM_dataset._o is None:
            return
            
        if (drape_dataset is not None) and (DEM_dataset is not None):
            # Get Current View & Prefs
            view = self.viewarea

            options = []
            if gview.get_preference('_gcp_warp_mode') is not None \
               and gview.get_preference('_gcp_warp_mode') == 'no':
                options.append(('raw','yes'))

            # Set Current View to 3D Mode
            view.set_mode(gvconst.MODE_3D)
            # view.height_scale(hscale)
            options.append(('mesh_lod',str(mesh_lod)))

            band = drape_dataset.GetRasterBand(1)
            interp = band.GetRasterColorInterpretation()
            
            # Create Drape Raster
            drape_raster = gview.manager.get_dataset_raster(drape_dataset,1)
            gview.undo_register(drape_raster)

            # Create Drape Raster Layer
            drape_raster_layer = gview.GvRasterLayer(drape_raster, options,
                                                     rl_mode = gview.RLM_AUTO )

            #
            # Note: We now set source for initial raster (0) as well, as set_source
            # will apply a default lut, if specified in band metadata.
            #
            raster_layer.set_source(0, drape_raster)

            # Logic to handle RGB and RGBA Layers
            if drape_raster_layer.get_mode() == gview.RLM_RGBA:
            
                green_raster= gview.manager.get_dataset_raster(drape_dataset,2)
                blue_raster = gview.manager.get_dataset_raster(drape_dataset,3)

                drape_raster_layer.set_source(1,green_raster)
                drape_raster_layer.set_source(2,blue_raster)

                if drape_dataset.RasterCount > 3:
                    band = drape_dataset.GetRasterBand(4)
                    if band.GetRasterColorInterpretation() == \
                                                gdal.GCI_AlphaBand:
                        drape_raster_layer.blend_mode_set(
                            gview.RL_BLEND_FILTER )
                        drape_raster_layer.set_source(3,
                             gview.manager.get_dataset_raster(drape_dataset,4))

            # Add to view
            drape_raster_layer.set_name(drape_dataset.GetDescription() )
            view.add_layer(drape_raster_layer)
            view.set_active_layer(drape_raster_layer)

            # Create DEM Raster and Add as Height
            DEM_raster = gview.GvRaster(dataset=DEM_dataset)
            DEM_raster.set_name(str(dem_filename))
            drape_raster_layer.add_height(DEM_raster)

            # perform clamping, if requested
            if min_clamp is not None:
                drape_raster_layer.clamp_height(1,0,min_clamp)

            if max_clamp is not None:
                drape_raster_layer.clamp_height(0,1,0,max_clamp)

            # Modify hscale to be more reasonable in some geo-referenced cases 
            #[hscalex1,dummy] = DEM_raster.pixel_to_georef(0,0)
            #[hscalex2,dummy] = DEM_raster.pixel_to_georef(DEM_dataset.RasterXSize - 1,0)
            #hscale_georef = hscale*abs(hscalex2-hscalex1)/DEM_dataset.RasterXSize
            #view.height_scale(hscale_georef)
            view.height_scale(hscale) 
           
            # Try to make sure everything is visible.
            self.seeall_cb()

    def position_3d(self, *args):
        self.make_active()
        if self.position3D_dialog is None:
            self.position3D_dialog = \
                         gviewapp.Position_3D_Dialog(self.app.view_manager)
            self.position3D_dialog.connect('destroy', self.destroy_position_3d)
        self.position3D_dialog.show()
        self.position3D_dialog.window.raise_()

        view = self.viewarea
        self.position3D_dialog.update_cb(view)
        view.connect('view-state-changed', self.position3D_dialog.update_cb)

    def destroy_position_3d(self,*args):
        self.position3D_dialog = None
                
    def raster_open_by_name(self,filename):
        self.make_active()
        gdal.ErrorReset()
        dataset = gdal.Open(filename)
        if dataset is None:
            gvutils.error('Unable to open: '+filename+'\n\n'+ \
                          gdal.GetLastErrorMsg())
            return None
        
        return dataset

    def rawgeo_cb( self, *args ):
        ref_layer = self.viewarea.active_layer()
        self.viewarea.set_raw(ref_layer, not self.viewarea.get_raw(ref_layer))
        self.rawgeo_update()

    def rawgeo_update( self, *args ):
        if self.iconbar is None:
            return

        ref_layer = self.viewarea.active_layer()
        if self.viewarea.get_raw( ref_layer ):
            self.rawgeo_pixmap.set_from_pixmap(self.raw_icon[0], self.raw_icon[1])
        else:
            self.rawgeo_pixmap.set_from_pixmap(self.geo_icon[0], self.geo_icon[1])
