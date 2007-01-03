##############################################################################
# $Id: gvsdsdlg.py,v 1.1.1.1 2005/04/18 16:38:35 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Subdataset Selection Dialog
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
from gtk import TRUE, FALSE
import gview
import os.path
import gvhtml


class GvSDSDlg(gtk.Window):
    def __init__(self, dataset, viewwindow):
        gtk.Window.__init__(self)
        self.set_title('SubDataset Selection')
        self.set_size_request(400, 300)
        self.set_border_width(3)
        self.set_policy(TRUE,TRUE,FALSE)
        self.connect('delete-event',self.close)
        shell = gtk.VBox(spacing=3)
        self.add(shell)
        #gvhtml.set_help_topic(self, "layerdlg.html" );

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

        # buttons
        button_box = gtk.HButtonBox()
        button_box.set_layout(gtk.BUTTONBOX_START)
        ok_button = gtk.Button('Accept')
        ok_button.connect('clicked', self.accept)
        apply_button = gtk.Button('Cancel')
        apply_button.connect('clicked', self.close)
        cancel_button = gtk.Button('Help')
        cancel_button.connect('clicked', self.help_cb)
        button_box.pack_start(ok_button, expand=FALSE)
        button_box.pack_start(apply_button, expand=FALSE)
        button_box.pack_start(cancel_button, expand=FALSE)
        shell.pack_start(button_box,expand=FALSE)

        self.connect('realize', self.realize)

        self.sel_pixmap = \
            gtk.Image().set_from_file(os.path.join(gview.home_dir,'pics',
                                        'ck_on_l.xpm'))
        self.not_sel_pixmap = \
            gtk.Image().set_from_file(os.path.join(gview.home_dir,'pics',
                                        'ck_off_l.xpm'))
        
        shell.show_all()

        self.dataset = dataset
        self.viewwindow = viewwindow
        self.layerlist = layerlist

        self.sds = dataset.GetSubDatasets()
        self.sds_sel = []
        for entry in self.sds:
            self.sds_sel.append( 0 )
            
        self.show_all()

    def help_cb(self,*args):
        pass
    
    def close(self,*args):
        self.hide()
        return TRUE

    def accept(self,*args):
        for i in range(len(self.sds_sel)):
            if self.sds_sel[i]:
                self.viewwindow.file_open_by_name( self.sds[i][0] )
        self.close()

    def realize(self, widget):
        lst = self.layerlist
        sds = self.sds

        lst.freeze()
        lst.clear()

        i = 0
        for entry in sds:
            lst.append(('', entry[1]))
                
            lst.set_pixmap(i, 0, self.not_sel_pixmap)

            i = i + 1

        lst.thaw()        

    def list_clicked(self, lst, event):
        #print event.type
        
        row, col = lst.get_selection_info(int(event.x), int(event.y))        
	lst.emit_stop_by_name('button-press-event')

        if event.type is gtk.gdk._2BUTTON_PRESS:
            for i in range(len(self.sds_sel)):
                self.sds_sel[i] = 0
                
            self.sds_sel[row] = 1
            self.accept()
        else:
            self.sds_sel[row] = not self.sds_sel[row]
        
        if self.sds_sel[row]:
            lst.set_pixmap(row, 0, self.sel_pixmap)
        else:
            lst.set_pixmap(row, 0, self.not_sel_pixmap)
        

