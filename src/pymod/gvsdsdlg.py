##############################################################################
# $Id$
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

#indices into the liststore
TOGGLE_COLUMN = 0
TEXT_COLUMN = 1
FILENAME_COLUMN = 2

class GvSDSDlg(gtk.Dialog):
    '''Creates and a modal dialog to ask which subdatasets should be selected.
    The run function shows the dialog and returns a list of the selected dataset filenames when the ok button is pressed.'''
    
    def __init__(self, parent, dataset):
        '''creates and shows the window, populating the list and setting callbacks.'''
        
        #initialize the dialog
        gtk.Dialog.__init__(self, 'SubDataset Selection', 
                            parent, 
                            gtk.DIALOG_MODAL|gtk.DIALOG_DESTROY_WITH_PARENT|gtk.DIALOG_NO_SEPARATOR,
                            (gtk.STOCK_OK, gtk.RESPONSE_ACCEPT,
                             gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))
        self.set_size_request(400, 300)
        self.set_border_width(3)
        self.set_resizable(True)

        #create the list
        self.liststore = gtk.ListStore(bool, str, str)#contents are (is checked, display name, file name)
        treeview = gtk.TreeView(self.liststore)
        
        #put list in scrolled window
        layerbox = gtk.ScrolledWindow()
        self.vbox.pack_start(layerbox)
        layerbox.add_with_viewport(treeview)
        
        #creating a single column that contains text and a checkbox - no headers
        celltoggle = gtk.CellRendererToggle()
        celltext = gtk.CellRendererText()
        column = gtk.TreeViewColumn()
        column.pack_start(celltoggle, expand=False)
        column.add_attribute(celltoggle, 'active', TOGGLE_COLUMN)
        column.pack_start(celltext, expand=True)
        column.add_attribute(celltext, 'text', TEXT_COLUMN)
        treeview.append_column(column)
        treeview.set_headers_visible(False)
        
        #get callback on toggled
        celltoggle.set_property('activatable', True)
        celltoggle.connect('toggled', self.toggled_cb, self.liststore)

        #get the subdatasets and add to the list
        for entry in dataset.GetSubDatasets(): #returns a list of tuples (file name, description)
            #indices are TOGGLE_COLUMN, TEXT_COLUMN, FILENAME_COLUMN
            self.liststore.append( [True, entry[1], entry[0]] )


    def run(self):
        '''Overload the run function to return a list of the selected datasets when ok is pressed.'''
        self.show_all()
        ret = gtk.Dialog.run(self)
        if ret == gtk.RESPONSE_ACCEPT:
            #get the selected datasets
            selectedDatasets = []
            for item in self.liststore:
                if item[TOGGLE_COLUMN]:
                    selectedDatasets.append(item[FILENAME_COLUMN])
            return selectedDatasets
        else:
            return ret
        

    def toggled_cb(self, cellrenderertoggle, path, liststore):
        '''Callback for when an item is toggled.  
        The change needs to be reflected in the liststore.'''
        iter = liststore.get_iter(path)
        liststore.set(iter, TOGGLE_COLUMN, not cellrenderertoggle.get_active())
        