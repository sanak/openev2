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
from gtk.keysyms import F9

import gview
from osgeo import gdal
from osgeo.gdalconst import *
import gvutils
import os
from filedlg import file_open
import pgu
import math
import gvhtml
import gviewapp
from layers import Layers

# for better readability
get_pref = gview.get_preference

ratio_list = ['250:1', '200:1', '150:1', '100:1', '80:1', '60:1', '45:1',
              '35:1', '25:1', '18:1', '10:1', '8:1', '4:1', '2:1', '1:1',
              '1:2', '1:4', '1:6', '1:8', '1:10', '1:12', '1:14','1:16',
              '1:20', '1:25', '1:30', '1:40', '1:60', '1:80', '1:100',]

def GvViewWindowFromXML( node, parent, filename=None ):
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
                 show_tracker=1, show_scrollbars=1,
                 menufile='NewMenuFile.xml',
                 iconfile='NewIconFile.xml'):

        gtk.Window.__init__(self)

        if title is None:
            title = 'View %d' % GvViewWindow.next_viewnum
            GvViewWindow.next_viewnum = GvViewWindow.next_viewnum + 1

        self.app = app
        self.set_title('OpenEV: '+title)
        gvhtml.set_help_topic(self, 'mainwindow.html')
        self.view_title = title
        shell = gtk.VBox(spacing=0)
        self.add(shell)
        self.pref_dialog = None
        self.position3D_dialog = None
        self.set_resizable(True)
        self.zoom = 0.0
        self.zoom_flag = 'yes' # see set_zoom_factor_cb()
        self.zoom_factor = None
        self.updating = False

        self.position3D_dialog = None
        self.menufile = menufile
        self.iconfile = iconfile
        self.UImgr = gtk.UIManager()
        self.add_accel_group(self.UImgr.get_accel_group())
        self.actions = gtk.ActionGroup('MainActions')
        self.setup_actions()
        self.UImgr.insert_action_group(self.actions, 0)

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

        self.tool_actions = gtk.ActionGroup('ToolActions')
        self.UImgr.insert_action_group(self.tool_actions, -1)
        self.add_tool_actions()

        # Add the actual GvViewArea for drawing in
        self.viewarea = gview.GvViewArea()

        if get_pref('view_background_color') is not None:
            tokens = get_pref('view_background_color').split()
            self.viewarea.set_background_color( [ float(tokens[0]),
                                                  float(tokens[1]),
                                                  float(tokens[2]),
                                                  float(tokens[3])] )

        # Update Zoom ratio box in toolbar whenever view changes
        # (actually, only when zoom changes)
        self.view_state_changed_id = \
        self.viewarea.connect("view-state-changed", self.update_zoom_cb)
        self.viewarea.connect("active-changed", self.update_zoom_cb)

        #a horizontally paned window for the layer management and the
        #view area

        hpaned = gtk.HPaned()
        shell.pack_start(hpaned)

        layer_widget = Layers(self)
        layer_widget.set_size_request(150, -1)
        layer_widget.tree_view.unset_flags(gtk.CAN_FOCUS)
        hpaned.pack1(layer_widget, resize=False)
        self.layer_widget = layer_widget

        size = (620, 620)
        if show_scrollbars:
            self.scrolled_window = gtk.ScrolledWindow()
            self.set_size_request(size[0], size[1] + 60)
            self.scrolled_window.add(self.viewarea)
            hpaned.pack2(self.scrolled_window, resize=True)
        else:
            self.viewarea.size(size)
            self.scrolled_window = None
            hpaned.pack2(self.viewarea, resize=True)

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

    def setup_actions(self):
        self.actions.add_actions([
            ('File', None, "File"),
            ('ImportFile', gtk.STOCK_OPEN, "Import", None, "Import and Display Raster File", self.file_import_cb),
            ('OpenFile', gtk.STOCK_OPEN, "Open", '<control>O', "Open and Display Raster/Vector File", self.file_open_cb),
            ('OpenRGB', None, "Open RGB", None, None, self.rgb_open_cb),
            ('Open3D', None, "Open 3D", None, None, self.open_3D_request),
            ('SaveVector', gtk.STOCK_SAVE, "Save Vector Layer", None, None, self.save_vector_layer_request),
            ('SaveProject', gtk.STOCK_SAVE, "Save Project", None, None, self.menu_save_project),
            ('SaveProjectAs', gtk.STOCK_SAVE_AS, "Save Project As", None, None, self.menu_save_project_as),
            ('NewView', gtk.STOCK_NEW, "New View", None, "Create a New View", self.menu_new_view),
            ('PrintView', gtk.STOCK_PRINT, "Print", '<control>P', "Print Current View", self.print_cb),
            ('rfl1', '', '', None, None, self.rfl_cb),
            ('rfl2', '', '', None, None, self.rfl_cb),
            ('rfl3', '', '', None, None, self.rfl_cb),
            ('rfl4', '', '', None, None, self.rfl_cb),
            ('rfl5', '', '', None, None, self.rfl_cb),
            ('CloseView', gtk.STOCK_CLOSE, "Close", None, None, self.close),
            ('Exit', gtk.STOCK_QUIT, "Exit", '<control>Q', None, self.exit),
            ('Edit', None, "Edit"),
            ('Undo', gtk.STOCK_UNDO, "Undo", '<control>Z', None, self.undo),
            ('Attrib', None, "Vector Layer Attributes...", None, None, self.show_oeattedit),
            ('EditTools', None, "Edit Toolbar...", None, None, self.app.show_toolbardlg),
            ('Goto', None, "Go To...", None, None, self.goto_dlg),
            ('Pyshell', None, "Python Shell...", None, None, self.pyshell),
            ('Pos3D', None, "3D Position...", None, None, self.position_3d),
            ('Preferences', gtk.STOCK_PREFERENCES, "Preferences...", None, None, self.app.launch_preferences),
            ('Tools', None, "Tools"),
            ('WMSTool', None, "WMS Tool", None, None, self.launch_wms),
            ('Image', None, "Image"),
            ('HistoTool', None, "Histogram Enhance", None, None, self.launch_histo),
            ('FusionTool', None, "Image Fusion", None, None, self.launch_fusion),
            ('Help', None, "Help"),
            ('About', gtk.STOCK_ABOUT, "About...", None, None, self.aboutcb)
            ])

        self.actions.add_actions([
            ('HelpOpenEV', gtk.STOCK_HELP, "Help...", None, None, self.helpcb),
            ], "openevmain.html")

        self.actions.add_actions([
            ('OpenEVWeb', gtk.STOCK_HELP, "Web Page...", None, None, self.helpcb),
            ], "http://OpenEV.Sourceforge.net/")

        self.actions.add_actions([
            ('NoEnh', 'nonelut', None, None, "Revert to no Enhancement", self.nonelut_cb),
            ('Linear', 'linear', None, None, "Linear Stretch/Enhancement", self.linear_cb),
            ('Equal', 'equalize', None, None, "Apply Equalization Enhancement to Raster", self.equalize_cb),
            ('Log', 'log', None, None, "Logarithmic Enhancement to Raster", self.log_cb),
            ('Restretch', 'windowed', None, None, "Windowed Raster Re-enhancement", self.restretch_cb),
            ('Classify', 'classify', None, None, "Classify Layer", self.classify_cb),
            ('Legend', 'legend', None, None, "Show Legend", self.show_legend_cb),
            ])

        self.actions.add_actions([
            ('SeeAll', gtk.STOCK_ZOOM_FIT, None, None, "Fit All Layers", self.seeall_cb),
            ('ZoomIn', gtk.STOCK_ZOOM_IN, None, None, "Zoom in x2", self.zoomin_cb),
            ('ZoomOut', gtk.STOCK_ZOOM_OUT, None, None, "Zoom out x2", self.zoomout_cb),
            ('Refresh', gtk.STOCK_REFRESH, None, None, "Refresh Rasters From Disk", self.refresh_cb),
            ])

    def add_tool_actions(self):
        if self.app is None or not hasattr(self.app, 'Tool_List'):
            return

        actions = []
        for name, ctool in self.app.Tool_List:
            stock_id = None
            accelerator = None
            hint = None
            items = ctool.menu_entries.entries.items()
            if items:
                path, (pos, callback, accelerator) = items[0]
                splitted = path.split('/')
                path,label = '/'.join(splitted[:-1]), splitted[-1]
                id = self.UImgr.new_merge_id()
                # MB TODO: handle position
                self.UImgr.add_ui(id, '/OEMenuBar/'+path, label, name, gtk.UI_MANAGER_MENUITEM, False)
            if ctool.icon_entries.entries:
                items = ctool.icon_entries.entries[0]
                filename, duh, hint, pos, callback, help, itype = items
                stock_id = os.path.basename(filename)[:-4]
                if help:
                    gvhtml.set_help_topic(item, help)
            actions.append((name, stock_id, label, accelerator, hint, callback))

        self.tool_actions.add_actions(actions, self.view_title)

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

        if self.app is None or not hasattr(self.app, 'get_rfl'):
            return

        try:
            lst = self.app.get_rfl()
            for i in range(5):
                rflitem = self.actions.get_action('rfl%s'%(i+1))
                if i < len(lst):
                    rflitem.set_property('label',lst[i])
                else:
                    rflitem.set_property('label','')
                    rflitem.set_visible(False)
        except:
            # Some menus don't have rfl
            pass

    def rfl_cb(self, action):
        self.file_open_by_name(action.get_property('label'), sds_check=0)

    def make_active(self, *args):
        self.app.view_manager.set_active_view( self )

    def key_press_cb( self, viewarea, event, *args ):
        if event.keyval == F9:
            gview.texture_cache_dump()

    def busy_changed_cb(self, *args):
        if gview.manager.get_busy():
            self.idlebusy_pixmap.set_from_stock('busy', gtk.ICON_SIZE_LARGE_TOOLBAR)
        else:
            self.idlebusy_pixmap.set_from_stock('idle', gtk.ICON_SIZE_LARGE_TOOLBAR)

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

    def hide_entry(self, entrystr='File/Exit'):
        """Hide a menu entry (can be temporary)
           Input: entrystr- eg. 'File/Exit', 'File/Close',...
           Returns True for success, False for failure.
        """
        try:
            item = self.UImgr.get_action('/OEMenuBar/'+entrystr)
            if item is not None:
                item.hide()
                return True
            return False
        except:
            return False

    def show_entry(self, entrystr='File/Exit'):
        """Re-show a hidden menu entry
           Input: entrystr- eg. 'File/Exit', 'File/Close',...
           Returns True for success, False for failure.
        """
        try:
            item = self.UImgr.get_action('/OEMenuBar/'+entrystr)
            if item is not None:
                item.show()
                return True
            return False
        except:
            return False

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

    def show_toolbardlg(self, *args):
        self.make_active()
        self.toolbar.show()
        self.toolbar.window.raise_()

    def launch_wms(self, *args):
        from wmstool import WMSDialog
        wms = WMSDialog(self.app)
        wms.show()

    def launch_histo(self, *args):
        from histoEnhance import HistogramWindow
        self.make_active()
        layer = self.app.active_layer()
        if layer is None:
            return
        win = HistogramWindow(layer)
        win.show()

    def launch_fusion(self, *args):
        from fusion import FusionDialog
        self.make_active()
        win = FusionDialog(self.app)
        if win:
            win.show()

    def goto_dlg(self, *args):
        DialogGoTo(self)

    def menu_save_project(self, *args):
        self.app.save_project()

    def menu_save_project_as(self, *args):
        self.app.save_project_as()

    def menu_new_view(self, *args):
        self.app.new_view()

    def menu_new_project(self, *args):
        self.app.clear_project()
        self.app.new_view()

    def save_vector_layer_request(self, *args):
        from filedlg import file_save
        self.make_active()
        cwd = get_pref('recent_directory')

        layer = self.viewarea.active_layer()
        if not layer or not isinstance(layer, gview.GvShapesLayer):
            gvutils.warning('Please select a vector layer using the layer\n'+\
                            'dialog before attempting to save.' )
            return

        file_save("Save Shapefile", cwd, filter=['ogrw'],
                  cb=self.save_vector_layer_with_file,
                  cb_data=layer.parent)

    def save_vector_layer_with_file(self, filename, cwd, shapes_data):
        if not shapes_data.save_to(filename):
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
        from osgeo import ogr
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

        file_open("File To Import", get_pref('recent_directory'), filter=['all'],
                  cb=self.file_import_by_name, app=self.app)

    def file_import_by_name(self, filename, *args):
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

        progress = pgu.ProgressDialog('Import to '+newfile, cancel=True)
        progress.SetDefaultMessage("translated")

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

        filter = ['all','gdalr','ogrr']
        file_open("File Open", get_pref('recent_directory'), filter=filter,
                  cb=self.file_open_name_check, app=self.app, ms=True)

    def rgb_open_cb(self, *args):
        self.make_active()

        filter = ['all','gdalr']
        file_open("RGB File Open", get_pref('recent_directory'), filter=filter,
                  cb=self.rgb_files_open_by_name, app=self.app, ms=True)

    def file_open_name_check(self, filename, cwd, *args):
        if get_pref('save_recent_directory') == 'on':
            gview.set_preference('recent_directory', cwd)
        for file in filename:
           self.file_open_by_name(file)

    def open_subdataset_check( self, dataset ):
        import gvsdsdlg
        dlg = gvsdsdlg.GvSDSDlg(dataset, self)

    def file_open_ap_envisat(self, dataset ):
        options = []
        if get_pref('gcp_warp_mode') is not None \
           and get_pref('gcp_warp_mode') == 'no':
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

        self.updating = True
        if add_to_rfl:
            self.app.add_to_rfl(dataset.GetDescription())

        md = dataset.GetMetadata()
        # special hack for displaying AP envisat specially.
        if md.has_key('MPH_PHASE') and dataset.RasterCount == 2:
            self.file_open_ap_envisat( dataset )
            return

        raster = gview.manager.get_dataset_raster(dataset,1)
        options = []
        if get_pref('gcp_warp_mode') is not None \
           and get_pref('gcp_warp_mode') == 'no':
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
                if md.has_key(gview.GV_PSCI_INTENSITY):
                    intensity = float(md.get(gview.GV_PSCI_INTENSITY))
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
        self.updating = False
        self.viewarea.set_active_layer(raster_layer)
        self.rawgeo_update()
        if len(self.viewarea.list_layers()) == 1: # zoom to 1:1 only if first layer
            self.onetoone_cb()

    def open_rgb_gdal_dataset(self, datasets):
        import vrtutils
        
        self.make_active()

        vrt_opts = vrtutils.VRTCreationOptions(3)
        vrt_tree = vrtutils.serializeCombinedDatasets(datasets, vrt_opts)
        ds = gdal.OpenShared(gdal.SerializeXMLTree(vrt_tree))
        rgbDS = gview.manager.add_dataset(ds)
        rgbDS.SetDescription('VRTDataset')

        red_raster = gview.manager.get_dataset_raster(rgbDS, 1)
        green_raster = gview.manager.get_dataset_raster(rgbDS, 2)
        blue_raster = gview.manager.get_dataset_raster(rgbDS, 3)

        rgb_layer = gview.GvRasterLayer(red_raster, rl_mode=gview.RLM_RGBA)
        rgb_layer.set_source(1, green_raster)
        rgb_layer.set_source(2, blue_raster)
        rgb_layer.set_name('RGB layer')

        self.viewarea.add_layer(rgb_layer)
        self.viewarea.set_active_layer(rgb_layer)
        self.rawgeo_update()
        if len(self.viewarea.list_layers()) == 1: # zoom to 1:1 only if first layer
            self.onetoone_cb()

    def get_gdal_raster_by_gci_type(self, dataset, gci_type):
        band_index = 0
        while band_index < dataset.RasterCount:
            raster = dataset.GetRasterBand(band_index + 1)
            if raster.GetRasterColorInterpretation() == gci_type:
                return raster
            band_index += 1
        return None

    def file_open_by_name(self, filename, lut=None, sds_check=1, *args):
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

    def rgb_files_open_by_name(self, filenames, cwd, *args):
        if len(filenames) != 3:
            gvutils.error('3 files needed for band merging')
            return
        datasets = []
        for file in filenames:
           datasets.append(gview.manager.get_dataset(file))
        
        self.open_rgb_gdal_dataset(datasets)

    def init_custom_icons(self):
        pass

    def init_default_icons(self):
        # Zoom ratio selection box

        combo = pgu.ComboBoxEntry(ratio_list)
        entry = combo.entry
        entry.set_text('1:1')
        entry.set_width_chars(5)
        self.zoom_entry_changed_id = entry.connect('changed', self.set_zoom_factor_cb)
        self.insert_widget_to_bar(combo, 'ZoomFactor')
        self.zoom_factor = combo

        # raw / georeferenced pixmap
        self.rawgeo_pixmap = gtk.Image()
        self.insert_widget_to_bar(self.rawgeo_pixmap, 'RawGeo')

        # idle / busy pixmap
        self.idlebusy_pixmap = gtk.Image()
        self.insert_widget_to_bar(self.idlebusy_pixmap, 'IdleBusy')

        gview.manager.connect('busy-changed', self.busy_changed_cb)

    def create_iconbar(self, iconfile='NewIconFile.xml'):
        icon_cmds = 'self.add_default_iconbar()'
        if iconfile:
            # check that icon file exists.  If not, use default values
            fulliconfile = os.path.join(gview.home_dir, 'xmlconfig', iconfile)
            if os.path.isfile(fulliconfile):
                if self.app and hasattr(self.app, 'load_icons_file_from_xml'):
                    icon_cmds = self.app.load_icons_file_from_xml(fulliconfile)
                else:
                    icon_cmds = 'self.UImgr.add_ui_from_file(fulliconfile)'
            else:
                print "Unable to find view icon configuration file " + iconfile
                print "Loading defaults"
                icon_cmds = 'self.add_default_iconbar()'

        exec icon_cmds
        self.iconbar = self.UImgr.get_widget('/OEToolbar')
        self.iconbar.set_style(gtk.TOOLBAR_ICONS)

        self.init_default_icons()
        self.init_custom_icons()

    def add_default_iconbar(self):
        """Fallback in case of configuration file problems"""
        self.UImgr.add_ui_from_string("""
            <ui>
            <toolbar name='OEToolbar'>
              <toolitem action='OpenFile'/>
              <toolitem action='PrintView'/>
              <separator/>
              <toolitem action='NoEnh'/>
              <toolitem action='Linear'/>
              <toolitem action='Equal'/>
              <toolitem action='Log'/>
              <toolitem action='Restretch'/>
              <toolitem action='Classify'/>
              <toolitem action='Legend'/>
              <placeholder name='ZoomFactor'/>
              <separator/>
              <toolitem action='SeeAll'/>
              <toolitem action='ZoomIn'/>
              <toolitem action='ZoomOut'/>
              <toolitem action='Refresh'/>
              <separator/>
              <placeholder name='RawGeo'/>
              <separator/>
              <toolitem action='HelpOpenEV'/>
              <placeholder name='IdleBusy'/>
            </toolbar>
            </ui>
            """
            )

    def create_menubar(self, menufile='NewMenuFile.xml'):
        menu_cmd = 'self.add_default_menu()'
        if menufile:
            # Check that menu file exists.  If not, use default values
            fullmenufile = os.path.join(gview.home_dir, 'xmlconfig', menufile)
            if os.path.isfile(fullmenufile):
                if self.app and hasattr(self.app, 'load_menus_file_from_xml'):
                    # Application parses the xml file itself
                    menu_cmd = self.app.load_menus_file_from_xml(menufile, self.view_title)
                else:
                    menu_cmd = 'self.UImgr.add_ui_from_file(fullmenufile)'
            else:
                print "Unable to find view menu configuration file " + menufile
                print "Loading defaults"
                menu_cmd = 'self.add_default_menu()'

        exec menu_cmd
        self.menuf = self.UImgr.get_widget('/OEMenuBar')
        if self.app is not None:
            self.app.subscribe('rfl-change',self.show_rfl)

    def add_default_menu(self):
        """Fallback in case of configuration file problems"""
        self.UImgr.add_ui_from_string("""
            <ui>
            <menubar name='OEMenuBar'>
              <menu action='File'>
                <menuitem action='ImportFile'/>
                <menuitem action='OpenFile'/>
                <menuitem action='OpenRGB'/>
                <menuitem action='Open3D'/>
                <menuitem action='SaveVector'/>
                <menuitem action='SaveProject'/>
                <menuitem action='SaveProjectAs'/>
                <menuitem action='NewView'/>
                <menuitem action='PrintView'/>
                <separator/>
                <menuitem action='rfl1'/>
                <menuitem action='rfl2'/>
                <menuitem action='rfl3'/>
                <menuitem action='rfl4'/>
                <menuitem action='rfl5'/>
                <separator/>
                <menuitem action='CloseView'/>
                <menuitem action='Exit'/>
              </menu>
              <menu action='Edit'>
                <menuitem action='Undo'/>
                <menuitem action='Attrib'/>
                <menuitem action='EditTools'/>
                <menuitem action='Goto'/>
                <menuitem action='Pyshell'/>
                <menuitem action='Pos3D'/>
                <menuitem action='Preferences'/>
              </menu>
              <menu action='Tools'>
                <menuitem action='WMSTool'/>
              </menu>
              <menu action='Image'>
                <menuitem action='HistoTool'/>
                <menuitem action='FusionTool'/>
              </menu>
              <menu action='Help'>
                <menuitem action='HelpOpenEV'/>
                <separator/>
                <menuitem action='OpenEVWeb'/>
                <menuitem action='About'/>
              </menu>
            </menubar>
            </ui>
            """
            )

    def do_nothing(self, *args):
        pass

    def insert_widget_to_bar(self, widget, path):
        anchor = self.UImgr.get_widget('/OEToolbar/%s'%path)
        idx = self.iconbar.get_item_index(anchor)
        item = gtk.ToolItem()
        item.add(widget)
        self.iconbar.insert(item, idx)

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
        self.do_enhance('window_restretch')

    def equalize_cb(self, *args):
        self.do_enhance('equalize')

    def linear_cb(self, *args):
        self.do_enhance('linear')

    def log_cb(self, *args):
        self.do_enhance('log')

    def nonelut_cb(self, *args):
        self.do_enhance('none_lut')

    def do_enhance(self, func):
        self.make_active()
        layer = self.viewarea.active_layer()
        if layer and isinstance(layer, gview.GvRasterLayer):
            exec 'layer.%s()' % func
        else:
            gvutils.warning("This can only be applied to a raster layer.\n" +
                            "Select a raster layer for this view in the layers list.")

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
            ratio_text = self.zoom_factor.child.get_text().split(':')
            ratio = [float(ratio_text[0]), float(ratio_text[1])]
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
        if self.updating:
            return

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
        self.make_active()

        # Create Dialog Window
        dialog = Dialog3D(self)
        self.file_dialog_3D = dialog

    def view3d_action(self, dem_filename, drape_filename=None, mesh_lod=None,
                        hscale=None, min_clamp=None, max_clamp=None):

        self.make_active()
        gview.manager.set_busy(True)
        self.viewarea.remove_all_layers()

        # Fill default parameters.
        if drape_filename is None:
            drape_filename = dem_filename
        if mesh_lod is None:
            mesh_lod = 3
        if hscale is None:
            hscale = 1.0
        
        # Get Data
        drape_dataset = gview.manager.get_dataset(drape_filename)
        if drape_dataset is None:
            gvutils.error('Unable to open drape dataset: '+drape_filename)
            return

        DEM_dataset = gview.manager.get_dataset(dem_filename)
        if DEM_dataset is None:
            gvutils.error('Unable to open DEM dataset: '+dem_filename)
            return

        # Get Current View & Prefs
        view = self.viewarea
        options = []
        pref = get_pref('gcp_warp_mode','yes')
        if pref == 'no':
            options.append(('raw','yes'))

        # Set Current View to 3D Mode
        view.set_mode(gview.MODE_3D)
        # view.height_scale(hscale)
        options.append(('mesh_lod',str(mesh_lod)))

        # Create Drape Raster
        drape_raster = gview.manager.get_dataset_raster(drape_dataset, 1)

        # Create Drape Raster Layer
        drape_raster_layer = gview.GvRasterLayer(drape_raster, options, rl_mode=gview.RLM_AUTO)

        #
        # Note: We now set source for initial raster (0) as well, as set_source
        # will apply a default lut, if specified in band metadata.
        #
        drape_raster_layer.set_source(0, drape_raster)

        # Logic to handle RGB and RGBA Layers
        if drape_raster_layer.get_mode() == gview.RLM_RGBA:
            green_raster= gview.manager.get_dataset_raster(drape_dataset, 2)
            blue_raster = gview.manager.get_dataset_raster(drape_dataset, 3)

            drape_raster_layer.set_source(1, green_raster)
            drape_raster_layer.set_source(2, blue_raster)

            if drape_dataset.RasterCount > 3:
                band = drape_dataset.GetRasterBand(4)
                if band.GetRasterColorInterpretation() == gdal.GCI_AlphaBand:
                    drape_raster_layer.blend_mode_set(gview.RL_BLEND_FILTER)
                    drape_raster_layer.set_source(3, gview.manager.get_dataset_raster(drape_dataset, 4))

        # Add to view
        drape_raster_layer.set_name(drape_dataset.GetDescription())
        view.add_layer(drape_raster_layer)

        # Create DEM Raster and Add as Height
        DEM_raster = gview.manager.get_dataset_raster(DEM_dataset, 1)
        DEM_raster.set_name(dem_filename)
        drape_raster_layer.add_height(DEM_raster)

        # perform clamping, if requested
        if min_clamp is not None:
            drape_raster_layer.clamp_height(1, 0, min_clamp)

        if max_clamp is not None:
            drape_raster_layer.clamp_height(0, 1, 0, max_clamp)

        # Modify hscale to be more reasonable in some geo-referenced cases 
        #[hscalex1,dummy] = DEM_raster.pixel_to_georef(0,0)
        #[hscalex2,dummy] = DEM_raster.pixel_to_georef(DEM_dataset.RasterXSize - 1,0)
        #hscale_georef = hscale*abs(hscalex2-hscalex1)/DEM_dataset.RasterXSize
        #view.height_scale(hscale_georef)
        view.height_scale(hscale) 
       
        # Try to make sure everything is visible.
        self.seeall_cb()
        view.set_active_layer(drape_raster_layer)
        gview.manager.set_busy(False)

    def position_3d(self, *args):
        self.make_active()
        if self.position3D_dialog is None:
            self.position3D_dialog = gviewapp.Position_3D_Dialog(self.app.view_manager)
            self.position3D_dialog.connect('destroy', self.destroy_position_3d)
        self.position3D_dialog.present()

        view = self.viewarea
        self.position3D_dialog.update_cb(view)
        view.connect('view-state-changed', self.position3D_dialog.update_cb)

    def destroy_position_3d(self, *args):
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

    def rawgeo_update(self, *args):
        if self.iconbar is None:
            return

        if self.viewarea.get_raw(self.viewarea.active_layer()):
            self.rawgeo_pixmap.set_from_stock('worldg', gtk.ICON_SIZE_LARGE_TOOLBAR)
        else:
            self.rawgeo_pixmap.set_from_stock('worldrgb', gtk.ICON_SIZE_LARGE_TOOLBAR)

class Dialog3D(gtk.Window):
    def __init__(self, win):
        """ 3D File Open Dialog for selecting drape and height data """
        from filedlg import FileOpenButton
        gtk.Window.__init__(self)
        self.set_title('Open 3D')
        self.set_border_width(5)
        self.set_resizable(False)
        gvhtml.set_help_topic(self, 'open3d.html')
        self.win = win
        
        vbox = gtk.VBox(spacing=5)
        self.add(vbox)
        table = gtk.Table()
        table.set_row_spacings(5)
        table.set_col_spacings(5)
        vbox.pack_start(table)
        
        # Drape File Selector
        row = 0
        table.attach(pgu.Label('Select Drape:'), 0, 1, row, row+1)

        self.drape_entry = pgu.Entry()
        table.attach(self.drape_entry, 1, 4, row, row+1)

        button = FileOpenButton(title='Select Drape',
                                cwd=get_pref('recent_directory'),
                                filter=['gdalr'], cb=self.file_selected)
        button.set_args('drape')
        table.attach(button, 4, 5, row, row+1, xoptions=gtk.SHRINK)

        # DEM File Selector
        row += 1
        table.attach(pgu.Label('Select DEM:'), 0, 1, row, row+1)

        self.dem_entry = pgu.Entry()
        table.attach(self.dem_entry, 1, 4, row, row+1)

        button = FileOpenButton(title='Select DEM',
                                cwd=get_pref('recent_directory'),
                                filter=['gdalr'], cb=self.file_selected)
        button.set_args('dem')
        table.attach(button, 4, 5, row, row+1, xoptions=gtk.SHRINK)

        # Mesh LOD and Height Scale
        row += 1
        table.attach(pgu.Label('Mesh LOD:'), 0, 1, row, row+1)

        spin_adjust = gtk.Adjustment(value=3, lower=0, upper=8, step_incr=1)
        self.lod_spin_button = gtk.SpinButton(spin_adjust, climb_rate=1, digits=0)
        table.attach(self.lod_spin_button, 1, 2, row, row+1, xoptions=gtk.SHRINK)

        table.attach(pgu.Label('Height Scaling Factor:'), 2, 3, row, row+1)
        entry = pgu.Entry()
        entry.set_size_request(70, -1)
        entry.set_max_length(7)
        entry.set_text('1.0')
        table.attach(entry, 3, 4, row, row+1, xoptions=gtk.SHRINK)
        self.scale_value = entry
        
        # DEM height clamping options
        row += 1
        toggle = gtk.CheckButton('Clamp Minimum Height to:')
        toggle.set_active(False)
        table.attach(toggle, 0, 3, row, row+1)
        self.min_heightclamp_toggle = toggle

        entry = pgu.Entry()
        entry.set_size_request(70, -1)
        entry.set_max_length(10)
        entry.set_text('0.0')
        table.attach(entry, 3, 4, row, row+1, xoptions=gtk.SHRINK)
        self.min_heightclamp_entry = entry

        row += 1
        toggle = gtk.CheckButton('Clamp Maximum Height to:')
        toggle.set_active(False)
        table.attach(toggle, 0, 3, row, row+1)
        self.max_heightclamp_toggle = toggle

        entry = pgu.Entry()
        entry.set_size_request(70, -1)
        entry.set_max_length(10)
        entry.set_text('100000.0')
        table.attach(entry, 3, 4, row, row+1, xoptions=gtk.SHRINK)
        self.max_heightclamp_entry = entry

        buttons = gtk.HBox(homogeneous=True, spacing=20)
        vbox.pack_start(buttons, expand=False)

        button = gtk.Button(stock=gtk.STOCK_APPLY)
        button.connect('clicked', self.perform_3D_request)
        buttons.pack_start(button, expand=False)
        
        button = gtk.Button(stock=gtk.STOCK_CLOSE)
        button.connect('clicked', self.close)
        buttons.pack_start(button, expand=False)

        button = gtk.Button(stock=gtk.STOCK_HELP)
        button.connect('clicked', win.helpcb, 'open3d.html')
        buttons.pack_start(button, expand=False)

        self.show_all()

    def file_selected(self, filename, cwd, id):
        if id == 'drape':
            self.drape_entry.set_text(filename)
        else:
            self.dem_entry.set_text(filename)

        if get_pref('save_recent_directory') == 'on':
            gview.set_preference('recent_directory', cwd)

    def close(self, *args):
        self.hide()
        self.destroy()

    def perform_3D_request(self, *args):
        """Tries to open selected files, then creates 3D Layer and switches to 3D mode"""
        drape_filename = self.drape_entry.get_text()
        dem_filename = self.dem_entry.get_text()
        mesh_lod = self.lod_spin_button.get_value_as_int()
        hscale = float(self.scale_value.get_text())

        if self.min_heightclamp_toggle.get_active():
            min_clamp = float(self.min_heightclamp_entry.get_text())
        else:
            min_clamp = None

        if self.max_heightclamp_toggle.get_active():
            max_clamp = float(self.max_heightclamp_entry.get_text())
        else:
            max_clamp = None

        # Do real work.
        self.win.view3d_action(dem_filename, drape_filename, mesh_lod, hscale,
                                min_clamp, max_clamp)

class DialogGoTo(gtk.Window):
    def __init__(self, win):
        """ Create the GoTo Dialog box with coordinate system option menu and
        text entry fields """
        gtk.Window.__init__(self)
        self.set_title('Go To...')
        self.win = win

        vbox = gtk.VBox(homogeneous=False, spacing=15)
        vbox.set_border_width(5)
        self.add(vbox)

        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(pgu.Label('Coordinate System:'), expand=False)
        combo = pgu.ComboText(('Row/Col','Native','lat-long'),
                                action=self.set_coord_system)
        box.pack_start(combo, expand=False)

        # Get current position in view native projection
        #   - changing Option Menu updates this in entry fields

        current_pos = self.win.viewarea.get_translation()

        # X Position
        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(pgu.Label('X Position:'), expand=False)
        entry = pgu.Entry()
        entry.set_max_length(14)
        entry.set_text(str(-current_pos[0]))
        box.pack_start(entry, expand=False)
        self.x_pos_entry = entry

        # Y Position
        box = gtk.HBox(spacing=3)
        vbox.pack_start(box, expand=False)
        box.pack_start(pgu.Label('Y Position:'), expand=False)
        entry = pgu.Entry()
        entry.set_max_length(14)
        entry.set_text(str(-current_pos[1]))
        box.pack_start(entry,expand=False)
        self.y_pos_entry = entry

        # Button to move
        goto_button = gtk.Button('Go To...')
        goto_button.connect('clicked', self.goto_location)
        vbox.pack_start(goto_button, expand=False)

        # set default to be native system - must be after x/y_pos_entry are setup
        combo.set_active(1)

        self.show_all()

    def set_coord_system(self, combo, *args):
        """ Set coordinate system goto coordinates entered in GoTo dialog from
        combo. """
        view = self.win.viewarea
        current_pos = view.get_translation()
        x_pos = str(-current_pos[0])
        y_pos = str(-current_pos[1])

        active = combo.get_active()
        if active == 0:
            layer = view.active_layer()
            if isinstance(layer, gview.GvRasterLayer):
                self.goto_coord_system = 'pixel'
                pos = layer.view_to_pixel(-current_pos[0], -current_pos[1])
                x_pos = str(pos[0])
                y_pos = str(pos[1])
        elif active == 2:
            self.goto_coord_system = 'lat-long'
            # MB: need to fix because format_point_query not working properly
            geo = view.format_point_query(-current_pos[0], -current_pos[1])
            lon = geo[1:geo.find(',')]
            if lon[-1] == 'W':
                x_pos = '-' + lon[:-1]
            else:
                x_pos = lon[:-1]
            lat = geo[geo.find(',')+2:geo.find(')')]
            if lat[-1] == 'S':
                y_pos = '-' + lat[:-1]
            else:
                y_pos = lat[:-1]
        else:
            self.goto_coord_system = 'native'

        self.x_pos_entry.set_text(x_pos)
        self.y_pos_entry.set_text(y_pos)

    def goto_location(self, *args):
        """ Translate view to location specified in GoTo Dialog, using projection """
        self.win.make_active()
        view = self.win.viewarea
        # Get current raster
        layer = view.active_layer()

        if not layer or not isinstance(layer, gview.GvRasterLayer):
            gvutils.warning('Please select a raster layer using the layer dialog.\n')
            return

        coord_system = self.goto_coord_system
        str_x = self.x_pos_entry.get_text()
        str_y = self.y_pos_entry.get_text()

        x = float(str_x)
        y = float(str_y)

        position = (x, y)
        if coord_system == 'pixel':
            if not view.get_raw(layer):
                # layer is georeferenced, coordinates are pixel/line
                position = layer.parent.pixel_to_georef(x, y)
        elif coord_system == 'lat-long':
            position = view.map_location((x, y))

        view.set_translation(-position[0], -position[1])

