##############################################################################
# $Id: gvogrdlg.py,v 1.1.1.1 2005/04/18 16:38:35 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  OGR Layer Selection and Loading Dialog.
# Author:   Frank Warmerdam, warmerdam@pobox.com
#
###############################################################################
# Copyright (c) 2003, Frank Warmerdam <warmerdam@pobox.com>
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
import os.path
import gvhtml
import ogr


class GvOGRDlg(gtk.Window):
    def __init__(self, ds, viewwindow):
        gtk.Window.__init__(self)
        self.set_title('Vector Layer Selection')
        self.set_size_request(500, 500)
        self.set_border_width(3)
        self.set_policy(True,True,False)
        self.connect('delete-event',self.close)
        shell = gtk.VBox(homogeneous=False,spacing=3)
        self.add(shell)
        gvhtml.set_help_topic(self, "veclayerselect.html" );

        # Layer list
        layerbox = gtk.ScrolledWindow()
        shell.pack_start(layerbox)
        layerlist = gtk.CList(cols=2)
            
        layerbox.add_with_viewport(layerlist)
        layerlist.set_shadow_type(gtk.SHADOW_NONE)
        layerlist.set_selection_mode(gtk.SELECTION_SINGLE)
        layerlist.set_row_height(30)
        layerlist.set_column_width(0, 24)
        #layerlist.connect('select-row', self.layer_selected)
        layerlist.connect('button-press-event', self.list_clicked)

        # Clip to view?

        hbox = gtk.HBox(homogeneous=False)
        shell.pack_start( hbox, expand=False )

        self.clip_to_view_btn = gtk.CheckButton()
        hbox.pack_start( self.clip_to_view_btn, expand=False )

        hbox.pack_start( gtk.Label('Clip To View' ), expand=False )

        # SQL Box.

        hbox = gtk.HBox(homogeneous=False, spacing=3)
        shell.pack_start( hbox,expand=False )
        
        sql_button = gtk.Button('Execute SQL:')
        sql_button.connect('clicked', self.execute_sql)
        hbox.pack_start(sql_button, expand=False)
        
        self.sql_cmd = gtk.Entry()
        hbox.pack_start(self.sql_cmd,expand=True)

        # buttons
        button_box = gtk.HButtonBox()
        button_box.set_layout_default(gtk.BUTTONBOX_START)
        ok_button = gtk.Button('Accept')
        ok_button.connect('clicked', self.accept)
        loadall_button = gtk.Button('Load All')
        loadall_button.connect('clicked', self.load_all)
        cancel_button = gtk.Button('Cancel')
        cancel_button.connect('clicked', self.close)
        help_button = gtk.Button('Help')
        help_button.connect('clicked', self.help_cb)
        button_box.pack_start(ok_button, expand=False)
        button_box.pack_start(loadall_button, expand=False)
        button_box.pack_start(cancel_button, expand=False)
        button_box.pack_start(help_button, expand=False)
        shell.pack_start(button_box,expand=False)

        self.connect('realize', self.realize)
        self.sel_pixmap = gtk.Image().set_from_file(os.path.join(gview.home_dir,'pics',
                                        'ck_on_l.xpm'))
        self.not_sel_pixmap = gtk.Image().set_from_file( os.path.join(gview.home_dir,'pics',
                                        'ck_off_l.xpm'))
        
        shell.show_all()

        self.ds = ds
        self.viewwindow = viewwindow
        self.layerlist = layerlist

        layer_count = ds.GetLayerCount()
        self.layer_names = []
        self.layer_sel = []
        for i in range(layer_count):
            layer = ds.GetLayer( i )
            self.layer_names.append( layer.GetName() )
            self.layer_sel.append( 0 )

        self.show_all()

    def help_cb(self,*args):
        gvhtml.LaunchHTML( "veclayerselect.html" );
    
    def close(self,*args):
        self.ds.Destroy()
        self.hide()
        return True

    def load_all(self,*args):
        for i in range(len(self.layer_sel)):
            self.layer_sel[i] = 1
        self.accept()

    def accept(self,*args):

        if self.clip_to_view_btn.get_active():
            xmin, ymin, xmax, ymax = self.viewwindow.viewarea.get_extents()

            wkt = 'POLYGON((%g %g,%g %g,%g %g,%g %g,%g %g))' % \
                   (xmin,ymax,xmax,ymax,xmax,ymin,xmin,ymin,xmin,ymax)
            rect = ogr.CreateGeometryFromWkt( wkt )
        else:
            rect = None
            
        for i in range(len(self.layer_sel)):
            if self.layer_sel[i]:
                layer = self.ds.GetLayer( i )

                if rect is not None:
                    layer.SetSpatialFilter( rect )
                    
                self.viewwindow.file_open_ogr_by_layer( layer )
                
                if rect is not None:
                    layer.SetSpatialFilter( None )

        if rect is not None:
            rect.Destroy()
            
        self.close()

    def realize(self, widget):
        lst = self.layerlist

        lst.freeze()
        lst.clear()

        i = 0
        for entry in self.layer_names:
            lst.append(('', entry))
                
            lst.set_pixmap(i, 0, self.not_sel_pixmap)

            i = i + 1

        lst.thaw()        

    def list_clicked(self, lst, event):
        row, col = lst.get_selection_info(int(event.x), int(event.y))
        lst.emit_stop_by_name('button-press-event')

        if event.type is gtk.gdk._2BUTTON_PRESS:
            for i in range(len(self.layer_sel)):
                self.layer_sel[i] = 0
                
            self.layer_sel[row] = 1
            self.accept()
        else:
            self.layer_sel[row] = not self.layer_sel[row]
        
        if self.layer_sel[row]:
            lst.set_pixmap(row, 0, self.sel_pixmap)
        else:
            lst.set_pixmap(row, 0, self.not_sel_pixmap)
        
    def execute_sql(self, *args):

        statement = self.sql_cmd.get_text()

        layer = self.ds.ExecuteSQL( statement )

        if layer is not None:
            self.viewwindow.file_open_ogr_by_layer( layer )
            
            self.ds.ReleaseResultsSet( layer )
