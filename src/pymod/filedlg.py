###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Multi-purpose file selection dialog
# Author:   Paul Spencer, spencer@dmsolutions.ca
# Comments: Merged cmfiledlg with filedlg
#
# Developed by Mario Beauchamp (starged@gmail.com)
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

import os
import pygtk
pygtk.require('2.0')
import gtk
import pgu

#dialog_types
FILE_OPEN = gtk.FILE_CHOOSER_ACTION_OPEN
FILE_SAVE = gtk.FILE_CHOOSER_ACTION_SAVE
DIRECTORY_SELECT = gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER

filters = {}
for key,name,pattern in [ 
                ("shape" , "Shapefiles (*.shp)","*.shp"),
                ("gtiff" , "Geotiff Files (*.tif)","*.tif"),
                ("tif" , "Tiff Files (*.tif)","*.tif"),
                ("spatial" , "Spatial Files","*.raw,*.shp"),
                ("gif" , "GIF Files (*.gif)","*.gif"),
                ("image" , "Image Files","*.png,*.gif,*.jpg"),
                ("all" , "All Files (*.*)","*,*.*"),
                ("leg" , "Legend Files","*.leg"),
                ("project" , "Project Files (*.opf)","*.opf"),
                ("gdalr" , 'GDAL Formats','*.vrt,*.tif,*.ntf,*.img,*.asc,*.ddf,*.n1,*.pix,*.mpr,*.mpl,*.pnm,*.hdr,*.bt,*.jp2,*.rsw,*.dem,*.gxf'),
                ("gdalw" , 'GDAL Formats','*.vrt,*.tif,*.ntf,*.img,*.pix,*.mpr,*.mpl,*.pnm,*.hdr,*.bt,*.rsw'),
                ("gdalc" , 'GDAL Formats','*.vrt,*.tif,*.ntf,*.img,*.asc,*.ddf,*.n1,*.pix,*.pnm,*.hdr,*.bt,*.jp2,*.rsw,*.dem,*.gxf'),
                ("ogrr" , 'OGR Formats','*.vrt,*.shp,*.ntf'),
                ("ogrw" , 'OGR Formats','*.shp,*.ntf'),
                ]:
            filter = gtk.FileFilter()
            filter.set_name(name)
            for ext in pattern.split(','):
                filter.add_pattern(ext)
            filters[key] = filter

global last_filter
last_filter = "all"

def get_filter_key(name):
    for key,filter in filters.iteritems():
        fname = filter.get_name()
        if fname == name:
            return key

class FileDialog(gtk.FileChooserDialog):
    """
    TODO: rewrite this...
    FileDialog presents a common interface to several directory/file operations.
    
    The class is initialized for file saving, file opening, or directory selection
    through the dialog_type parameter of __init__.
    
    FILE_OPEN -- allows the user to select an existing file
    
    FILE_SAVE -- allows the user to pick an existing file or type a new one
    
    DIRECTORY_SELECT -- allows the user to pick an existing directory
    
    Usage -- connect signals to FileDialog.ok_button and FileDialog.cancel_button
          -- FileDialog.get_filename() returns the file name
          -- FileDialog.get_directory() returns the directory
          -- FileDialog.set_filter() sets a filter for file name that limits the
             display of files in the file list.
    
    Filter specifications
    
    A file filter is specified as a text string containing the filter name and 
    filter items.  Filter items are separated by commas.  The filter name and
    filter items are separated by a vertical bar |
    
    Multiple filters are separated by a vertical bar
    
    i.e. DBF (*.dbf)|*.dbf|REC (*.rec)|*.rec|All Files (*.*)|*.*
    """
    def __init__(self, title=None, cwd=None, dialog_type=FILE_OPEN, filter=None, app=None):
        if dialog_type == FILE_OPEN:
            buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_OPEN,gtk.RESPONSE_OK)
        elif dialog_type == FILE_SAVE:
            buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_SAVE,gtk.RESPONSE_OK)
        else:
            buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_OK,gtk.RESPONSE_OK)
        gtk.FileChooserDialog.__init__(self, title, None, dialog_type, buttons)

        if filter:
            self.add_filters(filter)

        if not title:
            if dialog_type == FILE_OPEN:
                title = "Open File"
            elif dialog_type == FILE_SAVE:
                title = "Save File"
            elif dialog_type == DIRECTORY_SELECT:
                title = "Select Directory"
        self.set_title(title)

        #setup the current working directory
        if not cwd or not os.path.exists(cwd):
            cwd = os.getcwd()
        self.set_current_folder(cwd)

        if dialog_type == FILE_OPEN and app:
            rfl = app.get_rfl()
            rfl.insert(0, '')
            hbox = gtk.HBox()
            label = gtk.Label('Recent files:')
            combo = pgu.ComboText(strings=rfl, action=self.rfl_selected)
            self.set_extra_widget(hbox)

    def rfl_selected(self, combo):
        filename = combo.get_active_text()
        if filename:
            self.set_filename(filename)

    def add_filters(self, keys):
        for filt in keys:
            filter = filters.get(filt,None)
            if filter:
                self.add_filter(filter)

    def get_directory(self, *args):
        """return the directory"""
        return self.get_current_folder()

# utility functions
def file_selected(dlg, cb, *cb_data):
    """
    The user has selected a file.
    """
    global last_filter
    if dlg.get_select_multiple():
        selection = dlg.get_filenames()
    else:
        selection = dlg.get_filename()
    cwd = dlg.get_current_folder()
    last_filter = get_filter_key(dlg.get_filter().get_name())
    dlg.destroy()
    cb(selection, cwd, *cb_data)

def file_open(title, cwd, filter=None, cb=None, no_cb=None, app=None, ms=False, cb_data=None):
    file_dlg = FileDialog(title, cwd, FILE_OPEN, filter, app)
    file_dlg.set_select_multiple(ms)
    if last_filter in filter:
        file_dlg.set_filter(filters[last_filter])

    response = file_dlg.run()
    file_dlg.hide()
    if response == gtk.RESPONSE_OK:
        file_selected(file_dlg, cb, cb_data)
    elif response == gtk.RESPONSE_CANCEL and no_cb:
        file_dlg.destroy()
        no_cb(cb_data)

def file_save(title, cwd, filter=None, cb=None, no_cb=None, app=None, filename=None, cb_data=None):
    file_dlg = FileDialog(title, cwd, FILE_SAVE, filter, app)
    if filename:
        file_dlg.set_current_name(os.path.basename(filename))
    response = file_dlg.run()
    file_dlg.hide()
    if response == gtk.RESPONSE_OK:
        file_selected(file_dlg, cb, cb_data)
    elif response == gtk.RESPONSE_CANCEL and no_cb:
        file_dlg.destroy()
        no_cb(cb_data)

# utility widgets
class FileButton(gtk.Button):
    def __init__(self, title=None, cwd=None, filter=None, dialog_type=FILE_OPEN, cb=None, no_cb=None, app=None):
        gtk.Button.__init__(self)
        if dialog_type in (FILE_OPEN, DIRECTORY_SELECT):
            stock = gtk.STOCK_OPEN
        else:
            stock = gtk.STOCK_SAVE
        image = gtk.image_new_from_stock(stock, gtk.ICON_SIZE_BUTTON)
        self.add(image)
        self.connect('clicked', self.launch_dlg)
        self.dialog_type = dialog_type
        self.cwd = cwd
        self.cb = cb
        self.no_cb = no_cb
        self.args = ()
        self.title = title
        self.filter = filter
        self.filename = None
        self.app = app
        self.ms = False
        self.show()

    def set_cb(self, cb, *args):
        self.cb = cb
        self.args = args

    def set_no_cb(self, cb, *args):
        self.no_cb = cb
        self.args = args

    def set_filename(self, filename):
        self.filename = filename

    def set_args(self, *args):
        self.args = args

    def set_label(self, txt):
        image = self.get_children()[0]
        self.remove(image)
        hbox = gtk.HBox(spacing=5)
        self.add(hbox)
        label = gtk.Label(txt)
        hbox.pack_start(image, expand=False)
        hbox.pack_start(label, expand=False)

    def launch_dlg(self, *args):
        if self.dialog_type == FILE_OPEN:
            file_open(self.title, self.cwd, self.filter, self.cb, self.no_cb,
                      self.app, self.ms, *self.args)
        elif self.dialog_type == FILE_SAVE:
            file_save(self.title, self.cwd, self.filter, self.cb, self.no_cb,
                      self.app, self.filename, *self.args)

class FileOpenButton(FileButton):
    def __init__(self, title=None, cwd=None, filter=None, cb=None, no_cb=None, app=None):
        FileButton.__init__(self, title, cwd, filter, FILE_OPEN, cb, no_cb, app)

class FileSaveButton(FileButton):
    def __init__(self, title=None, cwd=None, filter=None, cb=None, no_cb=None, app=None):
        FileButton.__init__(self, title, cwd, filter, FILE_SAVE, cb, no_cb, app)
