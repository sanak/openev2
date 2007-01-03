##############################################################################
# $Id: open_subarea.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Interactive tool to perform calculations on pair of images.
# Authors:  Iscander Latypov
#	    Andrey Kiselev, dron@remotesensing.org
#
###############################################################################
# Copyright (c) 2004, American Museum of Natural History. All rights reserved.
# This software is based upon work supported by NASA under award
# number NAG5-12333
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
import gview, gdal, gdalconst, gvutils
import gviewapp
import vrtutils
import pgugrid
import osr

def CoordFrame(name,f_names,visible):
    frame = gtk.Frame(name)
    table = gtk.Table(2,4,gtk.FALSE)
    table.set_border_width(5)
    table.set_row_spacings(5)
    table.set_col_spacings(5)
    frame.add(table)
    table.show()
    list_entries = []
    k = 0
    for i in range(2):
        for j in range(2):
            label = gtk.Label(f_names[k])
	    label.set_alignment(0, 0.5)
            table.attach(label,2*j,2*j+1,i,i+1)
            label.show()
            entry = gtk.Entry()
            list_entries.append(entry)
            entry.show()
            table.attach(entry,2*j+1,2*j+2,i,i+1)
	    k = k + 1
	    
    return [frame, list_entries, visible]

def SetInitialCoord(dict_item, coords_list):
    """This routine sets initial values of out image 
    to coordinates of input image corners"""
    for i in range(4):
	dict_item[1][i].set_text(str(coords_list[i]))    

def PixelCoordToGeocoord(x,y,geotransform):
    px = geotransform[0]
    py = geotransform[3]
    px += geotransform[1] * x + geotransform[2] * y
    py += geotransform[4] * x + geotransform[5] * y
    return (px, py)
    
def GeocoordToPixelCoord(px,py,geotransform):
    s = px - geotransform[0]
    t = py - geotransform[3]
    det = geotransform[1] * geotransform[5] - geotransform[2] * geotransform[4]
    x = (s * geotransform[5] - geotransform[2] * t) / det
    y = (t * geotransform[1] - geotransform[4] * s) / det
    return (x, y)
    
def GetProjRect(pixcoords,geotransform):    
    (ulx,uly) = PixelCoordToGeocoord(pixcoords[1],pixcoords[0],geotransform)
    (lrx,lry) = PixelCoordToGeocoord(pixcoords[3],pixcoords[2],geotransform)
    return (uly, ulx, lry, lrx)

def GeocoordToLatLong(gx,gy,raster):
    """Build Spatial Reference object based on coordinate system,
    fecthed from the opened dataset"""
    srs = osr.SpatialReference()
    srs.ImportFromWkt(raster.GetProjection())
    srsLatLong = srs.CloneGeogCS()
    ct = osr.CoordinateTransformation(srs, srsLatLong)
    (lat, long, height) = ct.TransformPoint(gx, gy)
    return (lat, long)		

def LatLongToGeocoord(lat,long,raster):
    """Build Spatial Reference object based on coordinate system,
    fecthed from the opened dataset"""
    srs = osr.SpatialReference()
    srs.ImportFromWkt(raster.GetProjection())
    srsLatLong = srs.CloneGeogCS()
    ct = osr.CoordinateTransformation(srsLatLong,srs)
    (gx, gy, height) = ct.TransformPoint(lat, long)
    return (gx, gy)		

def GetGeogrRect(raster,geocoord):
    (ul_lat, ul_long) = GeocoordToLatLong(geocoord[1],geocoord[0],raster)
    (br_lat, br_long) = GeocoordToLatLong(geocoord[3],geocoord[2],raster)
    return (ul_lat,br_lat,ul_long,br_long)		

def ProjRectToPixelRect(proj_rect,geotransform):
    (lx,ty) = GeocoordToPixelCoord(proj_rect[0],proj_rect[1],geotransform)
    (rx,by) = GeocoordToPixelCoord(proj_rect[2],proj_rect[3],geotransform)
    return (ty, lx, by-ty, rx-lx)
    
def EmptyGeotransform(gt):
    if gt[0] != 0 or gt[1] != 1 or gt[2] != 0:
        return gtk.FALSE
    if gt[3] != 0 or gt[4] != 0 or gt[5] != 1:
        return gtk.FALSE;
    return gtk.TRUE
    
class OpenSubArea(gviewapp.Tool_GViewApp):
    def __init__(self, app = None):
	gviewapp.Tool_GViewApp.__init__(self,app)
	self.init_menu()
   
    def init_menu(self):
        self.menu_entries.set_entry("File/Open Subarea...",4,self.launch_dialog)
    
    def file_selection_ok(self,*args):
	self.source_name = self.o_s_d.get_filename()
	self.o_s_d.hide()
	
	rast = gdal.OpenShared(self.source_name, gdalconst.GA_ReadOnly)
	self.input_rast = rast
	
	self.pixsubarea = [0,0,rast.RasterYSize,rast.RasterXSize]
	SetInitialCoord(self.frame_dict['pixcoord'],self.pixsubarea)
	
	self.geotransform = rast.GetGeoTransform()
	
	if EmptyGeotransform(self.geotransform):
	    self.coord_system.set_history(0)
	    self.update_gui()
	    self.coord_system.hide()
	else:
	    self.projsubarea = GetProjRect(self.pixsubarea,self.geotransform)
	    SetInitialCoord(self.frame_dict['geocoord'],self.projsubarea) 
	
	    self.geogsubarea = GetGeogrRect(rast,self.projsubarea)
	    SetInitialCoord(self.frame_dict['geodetic'],self.geogsubarea) 
	
	self.band_list = []
	for i in range(rast.RasterCount):
	    item = ["Band " + str(i), "Yes"]
	    self.band_list.append(item)
	if len(self.band_list) > 1:    
            self.band_grid.set_source(self.band_list,expose=0)
	    grid_titles = ['Band number', 'Load']
	    self.band_grid.define_columns(titles=grid_titles,editables=[0,1])
            self.band_grid.resize_to_default()
	    for i in range(len(self.band_list)):
		self.band_num_list.append(i+1)
            self.band_grid.show_all()
        else:
            self.band_grid.hide()
 	self.dialog.show()
	    
    def launch_dialog(self,*args):
	self.band_num_list = []
	self.init_dialog()
	self.o_s_d = gtk.FileSelection("Source File Open")
        self.o_s_d.ok_button.connect("clicked",self.file_selection_ok) 
        self.o_s_d.cancel_button.connect("clicked",lambda x: self.o_s_d.hide())
   	self.o_s_d.show()
    	
    def open_subarea_cb(self,*args):
 	self.vrt_options = vrtutils.VRTCreationOptions(len(self.band_list))
 
	if self.geocoding == 1:
	    # get data from pixel-frame and transform lat/long
	    # to proj and to pixels	 	
 	    g_east = float(self.frame_dict['geodetic'][1][0].get_text())
 	    g_west = float(self.frame_dict['geodetic'][1][1].get_text())
 	    g_north = float(self.frame_dict['geodetic'][1][2].get_text())
 	    g_south = float(self.frame_dict['geodetic'][1][3].get_text())
 	    (east,north) = LatLongToGeocoord(g_east,g_north,self.input_rast)
 	    (west,south) = LatLongToGeocoord(g_west,g_south,self.input_rast)
 	    proj_rect = (east,north,west,south)
 	    (sline,spix,nlines,npix) = \
		ProjRectToPixelRect(proj_rect, self.geotransform)
 	elif self.geocoding == 2:
 	    # get data from pixel-frame and translate proj to pixels
 	    east = float(self.frame_dict['geocoord'][1][1].get_text())
 	    west = float(self.frame_dict['geocoord'][1][3].get_text())
 	    north = float(self.frame_dict['geocoord'][1][0].get_text())
 	    south = float(self.frame_dict['geocoord'][1][2].get_text())
 	    proj_rect = (east,north,west,south)
 	    (sline,spix,nlines,npix) = \
		ProjRectToPixelRect(proj_rect,self.geotransform)
 	else:
 	    # get data from pixel-frame
 	    spix = int(self.frame_dict['pixcoord'][1][1].get_text())
 	    sline = int(self.frame_dict['pixcoord'][1][0].get_text())
 	    npix = int(self.frame_dict['pixcoord'][1][3].get_text())
 	    nlines = int(self.frame_dict['pixcoord'][1][2].get_text())
 	
	if spix < 0:
	    spix = 0
	elif spix >  self.pixsubarea[3]:
	    gvutils.error("Source Area does not cover required Rectangle")
	    return
	if sline < 0:
	    sline = 0
	elif sline >  self.pixsubarea[2]:
	    gvutils.error("Source Area does not cover required Rectangle")
	    return
	if spix + npix > self.pixsubarea[3]:
	    npix = self.pixsubarea[3] - spix
	if sline + nlines > self.pixsubarea[2]:
	    nlines = self.pixsubarea[2] - sline
	
 	self.vrt_options.set_src_window((spix,sline,npix,nlines),self.band_num_list)
 	self.vrt_options.set_dst_window((0,0,npix,nlines))
 	
        vrt_tree=vrtutils.serializeDataset(self.input_rast,self.vrt_options,self.band_num_list)
        vrt_lines=gdal.SerializeXMLTree(vrt_tree)
        vrtdataset=gdal.Open(vrt_lines)
        gview.app.open_gdal_dataset(vrtdataset)
        self.close()
    	    	
    def update_gui(self,*args):
	for item in self.frame_dict.keys():
	    self.frame_dict[item][2] = gtk.FALSE
	    self.frame_dict[item][0].hide()
 	self.geocoding = self.coord_system.get_history()
	if self.geocoding == 0:
	    self.frame_dict['pixcoord'][2] = gtk.TRUE
	elif self.geocoding == 1:
	    self.frame_dict['geodetic'][2] = gtk.TRUE
	else:
	    self.frame_dict['geocoord'][2] = gtk.TRUE

        for item in self.frame_dict.keys():
            if self.frame_dict[item][2]: 
              self.frame_dict[item][0].show()
      
    def init_dialog(self):
	self.dialog = gtk.Window()
        self.dialog.set_title('Open Subarea')
        self.dialog.set_border_width(10)   
        
        mainshell = gtk.VBox(spacing=5)
        self.dialog.add(mainshell)
	mainshell.show()
	
	self.geocoding = 0
	coord_system_list = ["Pixels", "Geodetic (Lat/Long)", "Georeferenced"]
	self.coord_system = \
	       gvutils.GvOptionMenu(coord_system_list,self.update_gui)
	mainshell.pack_start(self.coord_system)
	self.coord_system.show()
	
	self.frame_dict = {}
	pix_fields_names = \
	    ('Start Line','Start Pixel','Num of Lines','Num of Pixels')
	self.frame_dict['pixcoord'] = \
	    CoordFrame('Pixel Coordinates', pix_fields_names, gtk.TRUE)
	mainshell.pack_start(self.frame_dict['pixcoord'][0], expand=gtk.FALSE)

	geo_fields_names = ('Westmost Longitude', 'Eastmost Longitude', \
	    'Northmost Latitude', 'Southmost Latitude')
	self.frame_dict['geodetic'] = \
	    CoordFrame('Geodetic (Lat/Long) Coordinates', \
	    geo_fields_names, gtk.FALSE)
	mainshell.pack_start(self.frame_dict['geodetic'][0], expand=gtk.FALSE)

	proj_fields_names = ('Northing', 'Westing', 'Southing', 'Easting')
	self.frame_dict['geocoord'] = \
	    CoordFrame('Georeferenced Coordinates', proj_fields_names, gtk.FALSE)
	mainshell.pack_start(self.frame_dict['geocoord'][0], expand=gtk.FALSE)
	
	self.band_grid = pgugrid.pguGrid(config=(2,0,1,1,4,0,0,0))
	self.band_grid.subscribe("cell-selection-changed",self.band_selected_cb)
	mainshell.pack_start(self.band_grid,expand=gtk.TRUE)
	
	button_box = gtk.HBox(spacing = 10)
	mainshell.pack_start(button_box, expand=gtk.FALSE)
	button_box.show()
	btOK = gtk.Button('OK')
 	button_box.pack_start(btOK)
 	btOK.connect("clicked",self.open_subarea_cb)
 	btOK.show()
 	
	btCancel = gtk.Button('Cancel')
 	button_box.pack_start(btCancel)
	btCancel.connect("clicked", self.close)
	btCancel.show()

        # Trap window close event
        self.dialog.connect('delete-event', self.close)

        for item in self.frame_dict.keys():
            if self.frame_dict[item][2]: 
              self.frame_dict[item][0].show()
    
        for item in self.frame_dict.keys():
            if self.frame_dict[item][2]: 
              self.frame_dict[item][0].show()
 
    def band_selected_cb(self,widget,cell):
        row = cell[0][0]
        if self.band_list[row][1] == "NO":
            self.band_list[row][1] = "YES"
	    self.band_num_list.append(row+1)	
	else:
	    self.band_list[row][1] = "NO"
	    self.band_num_list.remove(row+1)	
	self.band_grid.refresh()

    def close(self,*args):
        self.dialog.destroy()
    

TOOL_LIST = ['OpenSubArea']
