##############################################################################
# $Id$
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

import pygtk
pygtk.require('2.0')
import gtk
import gview
import os.path
import gvhtml
from osgeo import ogr


class GvOGRDlg(gtk.Window):
    def __init__(self, ds, viewwindow):
        gtk.Window.__init__(self)
        self.set_title('Vector Layer Selection')
        self.set_size_request(350, 600)
        self.set_border_width(3)
        self.set_resizable(True)
        self.connect('delete-event',self.close)
        shell = gtk.VBox(homogeneous=False,spacing=3)
        self.add(shell)
        gvhtml.set_help_topic(self, "veclayerselect.html" );

        # Layer list
        layerbox = gtk.ScrolledWindow()
        shell.pack_start(layerbox)
        layerstore = gtk.ListStore(str)
        layerlist = gtk.TreeView(layerstore)
        column = gtk.TreeViewColumn('Layer', gtk.CellRendererText(), text=0)
        layerlist.append_column(column)
        layerlist.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

        layerbox.add_with_viewport(layerlist)

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
        button_box.set_layout(gtk.BUTTONBOX_START)
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

        shell.show_all()

        self.ds = ds
        self.viewwindow = viewwindow
        self.layerstore = layerstore
        self.layerlist = layerlist

        layer_count = ds.GetLayerCount()
        for i in range(layer_count):
            layer = ds.GetLayer( i )
            layerstore.append([ '%s' % layer.GetName() ])

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

        for i in self.layerlist.get_selection().get_selected_rows()[1]:
            layer = self.ds.GetLayer( i[0] )

            if rect is not None:
                layer.SetSpatialFilter( rect )

            self.viewwindow.file_open_ogr_by_layer( layer )

            if rect is not None:
                layer.SetSpatialFilter( None )

        if rect is not None:
            rect.Destroy()

        self.close()

    def execute_sql(self, *args):

        statement = self.sql_cmd.get_text()

        layer = self.ds.ExecuteSQL( statement )

        if layer is not None:
            self.viewwindow.file_open_ogr_by_layer( layer )

            self.ds.ReleaseResultsSet( layer )
