##############################################################################
# $Id: calculator.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
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

import Numeric
from Numeric import cos,log,exp,sin,sqrt,maximum,minimum,tan,arcsin,arccos
from Numeric import arctan,bitwise_and,bitwise_or,bitwise_xor,invert,hypot
from Numeric import left_shift,right_shift

import gview
import gvutils
import gviewapp
import gdal
from gdalconst import *
import gdalnumeric


"""Image Calculator is the tool, that operate with the pair of images.
   The source images are the bands of opened layers, the result
   is new layer with only band, that contains image computed from
   sources by given formula. The coefficients for calculation
   defined by user in dialog box."""
   
########################################################################
def get_raster_size(_layer):
    w =  _layer.get_parent().get_dataset().GetRasterBand(1).XSize
    h =  _layer.get_parent().get_dataset().GetRasterBand(1).YSize
    return (w,h)

########################################################################
def get_list_of_layers_as_dict():
    """Returns dictionary of opened layers. The key of dictionary is
    the name of layer, members of dictionary are view object (index = 0)
    and layer object (index = 1)"""

    dict = {}
    for curview in gview.app.view_manager.view_list:
        for curlayer in curview.viewarea.list_layers():
    	    curname = curlayer.get_name()
	    dict[curname] = (curview,curlayer) 
    if dict is None:
	return None
    return dict

########################################################################
def get_list_of_layers_as_menu():
    """Returns gtk.Menu object for selecting of opened layers. 
    If number of opened layers is zero, the result is None"""
    menu = gtk.Menu()
    group = None
    for curview in gview.app.view_manager.view_list:
        for curlayer in curview.viewarea.list_layers():
    	    curname = curlayer.get_name()
	    menuitem = gtk.RadioMenuItem(group,curname)
	    group = menuitem
	    menu.append(menuitem)
	    menuitem.show()
    if group is None:
	return None
    return menu
	    
#########################################################################
def layer_is_raster(layer):
    """returns True if layer is raster and False if else"""
    try:
        layer.get_nodata(0)
        return True
    except:
        return False

#########################################################################
def get_filename_from_vrtdataset(vrt_dataset_name):
    if vrt_dataset_name[:11] != "<VRTDataset": return vrt_dataset_name
    ss = vrt_dataset_name.find("<SourceFilename relativeToVRT")
    ss = vrt_dataset_name.find(">",ss)
    se = vrt_dataset_name.find("</SourceFilename",ss)
    return "SubArea_"+vrt_dataset_name[ss+1:se]
		    

########################################################################
def get_list_of_bands_as_dict():
    """Returns dictionary of the bands of the opened layers. The key 
    of dictionary is the name of layer+band number, members of dictionary 
    are view-object (index = 0), layer-object (index = 1) and band-number"""

    active_layer = gview.app.sel_manager.get_active_layer()
    if not layer_is_raster(active_layer):
    	return None
    _size = get_raster_size(active_layer)
    dict = {}
    for curview in gview.app.view_manager.view_list:
        for curlayer in curview.viewarea.list_layers():
    	    if not layer_is_raster(curlayer):
    	        continue   
    	    curname = curlayer.get_name()
	    curname = get_filename_from_vrtdataset(curname)
	    cursize = get_raster_size(curlayer)
	    if cursize == _size:
		num_bands = curlayer.get_parent().get_dataset().RasterCount
		for i in range(num_bands):
	    	    curband = curname + '.band['+ str(i) + ']'
		    dict[curband] = (curview,curlayer,i) 
    return dict

#########################################################################
def clip_result(numtype, array):
	if numtype is Numeric.UnsignedInt8:
	    return Numeric.clip(array, 0.0, 255.0)
	elif numtype is Numeric.Int16:
	    return Numeric.clip(array, -32768.0, 32767.0)
	elif numtype is Numeric.Int32:
	    return Numeric.clip(array, -2147483648.0, 2147483647.0)
	else:
	    return array

############################################################################
#
#
############################################################################
class CalculatorTool(gviewapp.Tool_GViewApp):
    
    def __init__(self,app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.init_menu()

    def launch_dialog(self,*args):
	try:
	    self.win = CalculatorDialog()
	except:
	    gvutils.error("Please select a raster layer.");
	    return

	self.win.update_gui()
	self.win.show()

    def init_menu(self):
        self.menu_entries.set_entry("Image/Calculator...",2,
                                    self.launch_dialog)

############################################################################
class CalculatorDialog(gtk.Window):

    def __init__(self,app=None):
	rlayer = gview.app.sel_manager.get_active_layer()
	if rlayer is None:
            raise TypeError
	try:
	    gdal_dataset = rlayer.get_data().get_dataset()
	except:
	    raise TypeError
	    
	gtk.Window.__init__(self)
	self.set_title('Image Calculator')
	self.set_policy(False, True, True)
	
	try:
	    self.create_gui()
	except:
	    raise TypeError
	
	gtk.Window.show_all(self)
	
    def close(self,*args):
	self.destroy()
	return True

    def create_gui(self):
	self.dict_of_bands = get_list_of_bands_as_dict()
	if self.dict_of_bands is None:
	    raise
	self.list_of_bands = self.dict_of_bands.keys()
	
	title_width = 120

	box1 = gtk.VBox(spacing=5)     	
        self.add(box1)
        box1.show()

#### OPERATIONS ####################################################
	box_op = gtk.HBox(spacing=10)
	box1.set_border_width(10)
	box1.pack_start(box_op, expand=False)
	box_op.show()
	
	op_label  = gtk.Label('Operation:')
	op_label.set_alignment(0, 0.5)
	box_op.pack_start(op_label, expand=False)

#	The item <operations_dict> is the dictionary, that contains 
#	the list of calculator operation names as keys and the set 
#	of corresponding functions.
	self.operations_dict = {}
	self.operations_dict['Add  [Res = A * X + B * Y + C]'] = \
	    (3,self.add_bands)
	self.operations_dict['Multiply  [Res = X * Y]'] = \
	    (0,self.multiply_bands)
	self.operations_dict['Divide  [Res = A * X / (Y + B) + C]'] = \
	    (3,self.divide_bands)
	self.operations_dict['Vegetation Index  [Res = A * (X - Y) / ( X + Y) + B]'] = (2,self.veg_index)
	
	self.operations_list = self.operations_dict.keys()
	
	self.operation = \
	    gvutils.GvOptionMenu(self.operations_list,self.update_gui)
	box_op.pack_start(self.operation)
	self.operation.show()

### COEFFICIENTS ######################################################
	self.box_coeffs = gtk.HBox(spacing = 10)
        box1.pack_start(self.box_coeffs, expand=False)
        self.box_coeffs.show()

	self.box_coeff_a = gtk.HBox(spacing = 5)
        self.box_coeffs.pack_start(self.box_coeff_a, expand=False)
        self.box_coeff_a.show()

	a_label = gtk.Label('A =')
	a_label.set_alignment(0, 0.5)
        self.box_coeff_a.pack_start(a_label)
	
	self.a_const = gtk.Entry()
	self.a_const.set_size_request(80,30)
	self.a_const.set_text('1')
        self.box_coeff_a.pack_start(self.a_const, expand=False)
        self.a_const.show()
	
	self.box_coeff_b = gtk.HBox(spacing = 5)
        self.box_coeffs.pack_start(self.box_coeff_b, expand=False)
        self.box_coeff_b.show()

	b_label = gtk.Label('B =')
	b_label.set_alignment(0, 0.5)
        self.box_coeff_b.pack_start(b_label)
	
	self.b_const = gtk.Entry()
	self.b_const.set_size_request(80,30)
	self.b_const.set_text('1')
        self.box_coeff_b.pack_start(self.b_const, expand=False)
        self.b_const.show()
	
	self.box_coeff_c = gtk.HBox(spacing = 5)
        self.box_coeffs.pack_start(self.box_coeff_c, expand=False)
        self.box_coeff_c.show()

	c_label = gtk.Label('C =')
	c_label.set_alignment(0, 0.5)
        self.box_coeff_c.pack_start(c_label)
	
	self.c_const = gtk.Entry()
	self.c_const.set_size_request(80,30)
	self.c_const.set_text('1')
        self.box_coeff_c.pack_start(self.c_const, expand=False)
        self.c_const.show()

	self.coeffs_vis = [self.box_coeff_a,self.box_coeff_b,self.box_coeff_c]

### source1 #############################################################	
	frame1 = gtk.Frame("Select Image Bands To Compute")
	frame1.show()
        box1.pack_start(frame1, expand=False)
	
	box2 = gtk.VBox(spacing=10)
        box2.set_border_width(10)
	frame1.add(box2)
	box2.show()
	
	
	box_s1 = gtk.HBox(spacing=10)
        box2.pack_start(box_s1, expand=False)
	box_s1.show()

	source1_label = gtk.Label('Source 1  < X >:')
	source1_label.set_alignment(0, 0.5)
	box_s1.pack_start(source1_label,expand=False)
	
	self.s1_list = gvutils.GvOptionMenu(self.list_of_bands)
	box_s1.pack_start(self.s1_list)
	self.s1_list.show()
	
##  source2 ##############################################################
        box_s2 = gtk.HBox(spacing=10)
        box2.pack_start(box_s2, expand=False)
        box_s2.show()

	source2_label = gtk.Label('Source 2  < Y >:')
	source2_label.set_alignment(0, 0.5)
	box_s2.pack_start(source2_label,expand=False)
	
	self.s2_list = gvutils.GvOptionMenu(self.list_of_bands)
	box_s2.pack_start(self.s2_list)
	self.s2_list.show()
	
#####OUT TYPES#########################################################
	box_types = gtk.HBox(spacing=10)
	box2.pack_start(box_types, expand=False)
	box_types.show()

	types_label  = gtk.Label('Image data type:')
	types_label.set_alignment(0, 0.5)
	box_types.pack_start(types_label, expand=False)
	
	self.types_list = []
	i = GDT_Byte
	while i < GDT_TypeCount:
	    self.types_list.append(gdal.GetDataTypeName(i))
	    i += 1

	self.types = gvutils.GvOptionMenu(self.types_list)
	box_types.pack_start(self.types)
	self.types.show()
	
	
#### NEW VIEW ##########################################################
	self.switch_new_view = gtk.CheckButton("Create New View")
	box1.pack_start(self.switch_new_view)
	self.switch_new_view.show()
	
#### BUTTONS ###########################################################
        box_buttons = gtk.HBox(spacing=15)
        box1.pack_start(box_buttons, expand=False)
        box_buttons.show()

        self.ok_btn = gtk.Button("Ok")
        self.ok_btn.connect("clicked", self.compute)
        box_buttons.pack_start(self.ok_btn)
	
        self.cancel_btn = gtk.Button("Cancel")
        self.cancel_btn.connect("clicked", self.close)
        box_buttons.pack_start(self.cancel_btn)
	
        return True

#########################################################################
#########################################################################	
    def update_gui(self,*args):
	i = self.operation.get_history()
	self.op = self.operations_list[i]
	k = self.operations_dict[self.op][0]
	if k == 0:
	    self.box_coeffs.hide()
	else:
	    self.box_coeffs.show()
	    for j in range(3):
		if j < k:
		    self.coeffs_vis[j].show() 
		else:
		    self.coeffs_vis[j].hide()
#########################################################################	
    def compute(self,*args):
        def get_band(name):
	    layer = self.dict_of_bands[name][1]
	    b_num = self.dict_of_bands[name][2]
	    band = layer.get_parent().get_dataset().GetRasterBand(b_num+1)
	    return band	    

	def create_new_layer(pview,player,w,h):
	    """Creates new raster layer like <player> with width = w
	    and height = h"""
        
	    gview.app.view_manager.set_active_view(pview)
	    pview.viewarea.set_active_layer(player)	
    	    target_ds = player.get_parent().get_dataset()
	    rl_mode_value = player.get_mode()
    	    new_layer = gview.GvRasterLayer( \
		gview.GvRaster(dataset = target_ds,real=1), \
		rl_mode = rl_mode_value)
	    pview.viewarea.list_layers().append(new_layer)
	    return new_layer
	
	
	#  extract computing parameters
	b1_name = self.list_of_bands[self.s1_list.get_history()]
	b2_name = self.list_of_bands[self.s2_list.get_history()]
	if b1_name <> None:
	    if self.switch_new_view.get_active():
		gview.app.new_view()
	    op_index = self.operation.get_history()
	    type_index = self.types.get_history()
	
	    a = float(self.a_const.get_text())
	    b = float(self.b_const.get_text())
	    c = float(self.c_const.get_text())
	    b1 = get_band(b1_name)
	    b2 = get_band(b2_name)

	    type = \
		gdal.GetDataTypeByName(self.types_list[self.types.get_history()])
	    self.numtype = gdalnumeric.GDALTypeCodeToNumericTypeCode(type)
	    if self.numtype == None:
		gvutils.error("Error! Type " + self.numtype + " is not supported!")
		return False 
	    
	    proto_ds = self.dict_of_bands[b1_name][1].get_parent().get_dataset()
	    op_func = self.operations_dict[self.op][1]
	    self.out_buf = Numeric.zeros((b1.YSize, b1.XSize), self.numtype)
	    try:
		op_func(s1=b1,s2=b2,a=a,b=b,c=c)
	    except:
		gvutils.error("Try to change coefficients.")
		return False
	    res_ds = gdalnumeric.OpenArray(self.out_buf,proto_ds)
	    gview.app.open_gdal_dataset(res_ds)
	    self.close()
	else:
	    gvutils.error("Source1 and Source2 have to differ!")
	    return False

#########################################################################
    def add_bands(self,s1,s2,a,b,c):
	"""<add_bands> method computes the linear combination of the bands
	   Arguments:
		s1 - first source band;
		s2 - second source band;
		a,b,c - linear combination coefficients;
	   The output raster pixel density is calculated by formula
	        res = a * X + b * Y + c
	   where  X and Y are densities of first and second source bands
	"""
	for i in range(s1.YSize):
	    s1_buf = gdalnumeric.BandReadAsArray(s1,0,i,s1.XSize,1)[0]
	    s2_buf = gdalnumeric.BandReadAsArray(s2,0,i,s1.XSize,1)[0]
	    temp_buf = a * s1_buf + b * s2_buf + c
	    self.out_buf[i, 0:] = \
		clip_result(self.numtype, temp_buf).astype(self.numtype)
	    
#########################################################################
    def multiply_bands(self,s1,s2,a,b,c):
	"""Method <multiply_bands> computes the production of the bands
	   Arguments:
		s1 - first source band;
		s2 - second source band;
		a,b,c - unusable;
	   The output raster pixel density is calculated by formula
	        res =  X * Y
	   where  X and Y are densities of first and second source bands.
	"""
	for i in range(s1.YSize):
	    s1_buf = gdalnumeric.BandReadAsArray(s1,0,i,s1.XSize,1)[0]
	    s2_buf = gdalnumeric.BandReadAsArray(s2,0,i,s1.XSize,1)[0]
	    temp_buf = 1. * s1_buf * s2_buf
	    self.out_buf[i, 0:] = \
		clip_result(self.numtype, temp_buf).astype(self.numtype)

#########################################################################
    def divide_bands(self,s1,s2,a,b,c):
	"""<divide_bands> method executes the division of the bands
	   Arguments:
		s1 - first source band;
		s2 - second source band;
		a,b,c - coefficients;
	   The output raster pixel density is calculated by formula
	        res = a * X / (b + Y) + c
	   where  X and Y are densities of first and second source bands
	"""
	for i in range(s1.YSize):
	    s1_buf = gdalnumeric.BandReadAsArray(s1,0,i,s1.XSize,1)[0]
	    s2_buf = gdalnumeric.BandReadAsArray(s2,0,i,s1.XSize,1)[0]
	    try:
		temp_buf = a * s1_buf / ( b + s2_buf) + c
	    except:
		raise
	    self.out_buf[i, 0:] = \
		clip_result(self.numtype, temp_buf).astype(self.numtype)
	    
#########################################################################
    def veg_index(self,s1,s2,a,b,c):
	"""Method <veg_index> computes the vegetation index for pair of the bands
	   Arguments:
		s1 - first source band;
		s2 - second source band;
		a,b - coefficients;
		c - unusable;
	   The output raster pixel density is calculated by formula
	        res = a * (X - Y) / (X + Y) + b
	   where  X and Y are densities of first and second source bands.
	"""
	for i in range(s1.YSize):
	    s1_buf = gdalnumeric.BandReadAsArray(s1,0,i,s1.XSize,1)[0]
	    s2_buf = gdalnumeric.BandReadAsArray(s2,0,i,s1.XSize,1)[0]
	    temp_buf1 = s1_buf + s2_buf + 1
	    temp_buf = a * (s1_buf-s2_buf) / temp_buf1 + b
	    self.out_buf[i, 0:] = \
		clip_result(self.numtype, temp_buf).astype(self.numtype)
	
############################################################################
#
#
############################################################################
class RCalculatorTool(gviewapp.Tool_GViewApp):
    
    def __init__(self,app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.init_menu()

    def launch_dialog(self,*args):
	try:
	    self.win = RCalculatorDialog()
	except:
	    gvutils.error("Please select a raster layer.")
	    return
    
	self.win.update_gui()
        self.win.show()

    def init_menu(self):
        self.menu_entries.set_entry("Image/Raster Calculator...", 2,
                                    self.launch_dialog)

############################################################################
class RCalculatorDialog(gtk.Window):

    def __init__(self,app=None):
	rlayer = gview.app.sel_manager.get_active_layer()
	if rlayer is None:
            raise TypeError
        try:
    	    gdal_dataset = rlayer.get_data().get_dataset()
        except:
    	    raise TypeError
	    		
	gtk.Window.__init__(self)
	self.set_title('Image Calculator')
	self.set_policy(False, True, True)
	self.text_pos = 0
	self.tooltips = gtk.Tooltips()
    	self.expression = ""
		   	
	try:
	    self.create_gui()
	except:
            raise TypeError
	    
	gtk.Window.show_all(self)
	
    def close(self,*args):
	self.destroy()
	return True

######################################################################
    def bit_operations_group(self,funcbox):

	self.bfunc_table = gtk.Table(4,2)
	self.bfunc_table.set_border_width(5)
	self.bfunc_table.set_row_spacings(5)
	self.bfunc_table.set_col_spacings(5)
	funcbox.pack_start(self.bfunc_table, expand=False)
	
	btn = gtk.Button("AND")
	btn.connect("clicked",self.fbutton_pressed,"AND()")
        self.bfunc_table.attach(btn,0,1,0,1)

	btn = gtk.Button("OR")
	btn.connect("clicked",self.fbutton_pressed,"OR()")
        self.bfunc_table.attach(btn,1,2,0,1)

	btn = gtk.Button("XOR")
	btn.connect("clicked",self.fbutton_pressed,"XOR()")
        self.bfunc_table.attach(btn,0,1,1,2)

	btn = gtk.Button("invert")
	btn.connect("clicked",self.fbutton_pressed,"invert()")
        self.bfunc_table.attach(btn,1,2,1,2)
	
	btn = gtk.Button("LShift")
	btn.connect("clicked",self.fbutton_pressed,"LShift()")
        self.bfunc_table.attach(btn,0,1,2,3)

	btn = gtk.Button("RShift")
	btn.connect("clicked",self.fbutton_pressed,"RShift()")
        self.bfunc_table.attach(btn,1,2,2,3)
	
	self.bfunc_table.hide()

############################################################################
    def mathematics_group(self,funcbox):
	self.mfunc_table = gtk.Table(4,2)
	self.mfunc_table.set_border_width(5)
	self.mfunc_table.set_row_spacings(5)
	self.mfunc_table.set_col_spacings(5)
	funcbox.pack_start(self.mfunc_table, expand=False)

	btn = gtk.Button("log")
	btn.connect("clicked",self.fbutton_pressed,"log()")
        self.mfunc_table.attach(btn,0,1,0,1)

	btn = gtk.Button("exp")
	btn.connect("clicked",self.fbutton_pressed,"exp()")
        self.mfunc_table.attach(btn,1,2,0,1)

	btn = gtk.Button("sin")
	btn.connect("clicked",self.fbutton_pressed,"sin()")
        self.mfunc_table.attach(btn,0,1,1,2)

	btn = gtk.Button("cos")
	btn.connect("clicked",self.fbutton_pressed,"cos()")
        self.mfunc_table.attach(btn,1,2,1,2)
	
	btn = gtk.Button("sqrt")
	btn.connect("clicked",self.fbutton_pressed,"sqrt()")
        self.mfunc_table.attach(btn,0,1,2,3)

	btn = gtk.Button("tan")
	btn.connect("clicked",self.fbutton_pressed,"tan()")
        self.mfunc_table.attach(btn,1,2,2,3)
	
	btn = gtk.Button("max")
	btn.connect("clicked",self.fbutton_pressed,"max()")
        self.mfunc_table.attach(btn,0,1,3,4)

	btn = gtk.Button("min")
	btn.connect("clicked",self.fbutton_pressed,"min()")
        self.mfunc_table.attach(btn,1,2,3,4)
	
	self.mfunc_table.hide()

######################################################################
    def trigonometry_group(self,funcbox):
	self.tfunc_table = gtk.Table(4,2)
	self.tfunc_table.set_border_width(5)
	self.tfunc_table.set_row_spacings(5)
	self.tfunc_table.set_col_spacings(5)
	funcbox.pack_start(self.tfunc_table, expand=False)

	btn = gtk.Button("sin")
	btn.connect("clicked",self.fbutton_pressed,"sin()")
        self.tfunc_table.attach(btn,0,1,0,1)

	btn = gtk.Button("cos")
	btn.connect("clicked",self.fbutton_pressed,"cos()")
        self.tfunc_table.attach(btn,1,2,0,1)

	btn = gtk.Button("tan")
	btn.connect("clicked",self.fbutton_pressed,"tan()")
        self.tfunc_table.attach(btn,0,1,1,2)

	btn = gtk.Button("asin")
	btn.connect("clicked",self.fbutton_pressed,"asin()")
        self.tfunc_table.attach(btn,1,2,1,2)
	
	btn = gtk.Button("acos")
	btn.connect("clicked",self.fbutton_pressed,"acos()")
        self.tfunc_table.attach(btn,0,1,2,3)

	btn = gtk.Button("atan")
	btn.connect("clicked",self.fbutton_pressed,"atan()")
        self.tfunc_table.attach(btn,1,2,2,3)
	
	btn = gtk.Button("hypot")
	btn.connect("clicked",self.fbutton_pressed,"hypot()")
        self.tfunc_table.attach(btn,0,1,3,4)

	btn = gtk.Button("atan2")
	btn.connect("clicked",self.fbutton_pressed,"atan2()")
        self.tfunc_table.attach(btn,1,2,3,4)
	
	self.tfunc_table.hide()

######################################################################
    def special_funcs_group(self,funcbox):

	self.sfunc_table = gtk.Table(4,2)
	self.sfunc_table.set_border_width(5)
	self.sfunc_table.set_row_spacings(5)
	self.sfunc_table.set_col_spacings(5)
	funcbox.pack_start(self.sfunc_table, expand=False)

	btn = gtk.Button("NDVI")
	btn.connect("clicked",self.fbutton_pressed,"NDVI()")
        self.sfunc_table.attach(btn,0,1,0,1)
	self.tooltips.set_tip(btn, 'NDVI(X,Y) = (Y - X)/(X + Y) * 128 + 128')

	self.sfunc_table.hide()

##############################################################################
    
    def create_gui(self):
	self.dict_of_bands = get_list_of_bands_as_dict()
	if self.dict_of_bands is None:
	    raise TypeError
	self.list_of_bands = self.dict_of_bands.keys()
	self.list_of_bands.sort()
	
	title_width = 120

	box1 = gtk.VBox(spacing=10)
	box1.set_border_width(10)
        self.add(box1)
        box1.show()

	box2 = gtk.VBox(spacing=5)
	box2.set_border_width(5)
	box1.pack_start(box2)
	box2.show()

	self.expression_unit = gtk.Entry()
	self.expression_unit.set_text(self.expression)
	self.expression_unit.connect("changed",self.expression_edit_cb)
        box2.pack_start(self.expression_unit)
        self.expression_unit.show()
	
	box3 = gtk.HBox(spacing=5)
	box1.pack_start(box3)
	box3.show()

#####	
	funcbox = gtk.VBox(spacing=5)
	funcbox.set_border_width(10)
	box3.pack_start(funcbox)
	funcbox.show()

	fg_list = ["Mathematics","Bit Operations","Trigionometry","Special"]
	self.fun_group_list = gvutils.GvOptionMenu(fg_list,self.group_changed)
	funcbox.pack_start(self.fun_group_list, expand=False)
	

	self.mathematics_group(funcbox)
	self.bit_operations_group(funcbox)
	self.trigonometry_group(funcbox)
	self.special_funcs_group(funcbox)
	
#####
	digitbox = gtk.VBox(spacing=10)
	digitbox.set_border_width(10)
	box3.pack_start(digitbox)
	digitbox.show()
	
	digit_table = gtk.Table(5,5)
	digit_table.set_border_width(5)
	digit_table.set_row_spacings(5)
	digit_table.set_col_spacings(5)
	digitbox.pack_start(digit_table)
	digit_table.show()
	
	btn = gtk.Button("Back")
        btn.connect("clicked", self.back_button_pressed)
        digit_table.attach(btn,1,3,0,1)

	btn = gtk.Button("C")
        btn.connect("clicked", self.clear_button_pressed)
        digit_table.attach(btn,3,5,0,1)

        btn = gtk.Button("7")
        btn.connect("clicked", self.button_pressed,"7")
        digit_table.attach(btn,0,1,1,2)

        btn = gtk.Button("8")
        btn.connect("clicked", self.button_pressed,"8")
        digit_table.attach(btn,1,2,1,2)
 
        btn = gtk.Button("9")
        btn.connect("clicked", self.button_pressed,"9")
        digit_table.attach(btn,2,3,1,2)
  
        btn = gtk.Button(" / ")
        btn.connect("clicked", self.button_pressed,"/")
        digit_table.attach(btn,3,4,1,2)
 
        btn = gtk.Button("(")
        btn.connect("clicked", self.button_pressed,"(")
        digit_table.attach(btn,4,5,1,2)

        btn = gtk.Button("4")
        btn.connect("clicked", self.button_pressed,"4")
        digit_table.attach(btn,0,1,2,3)

        btn = gtk.Button("5")
        btn.connect("clicked", self.button_pressed,"5")
        digit_table.attach(btn,1,2,2,3)

        btn = gtk.Button("6")
        btn.connect("clicked", self.button_pressed,"6")
        digit_table.attach(btn,2,3,2,3)
 
        btn = gtk.Button("*")
        btn.connect("clicked", self.button_pressed,"*")
        digit_table.attach(btn,3,4,2,3)

        btn = gtk.Button(")")
        btn.connect("clicked", self.button_pressed,")")
        digit_table.attach(btn,4,5,2,3)
 
        btn = gtk.Button("1")
        btn.connect("clicked", self.button_pressed,"1")
        digit_table.attach(btn,0,1,3,4)
 
        btn = gtk.Button("2")
        btn.connect("clicked", self.button_pressed,"2")
        digit_table.attach(btn,1,2,3,4)

        btn = gtk.Button("3")
        btn.connect("clicked", self.button_pressed,"3")
        digit_table.attach(btn,2,3,3,4)

        btn = gtk.Button("-")
        btn.connect("clicked", self.button_pressed,"-")
        digit_table.attach(btn,3,4,3,4)

        btn = gtk.Button("End")
        btn.connect("clicked", self.end_button_pressed)
        digit_table.attach(btn,4,5,3,4)

        btn = gtk.Button("0")
        btn.connect("clicked", self.button_pressed,"0")
        digit_table.attach(btn,0,1,4,5)

        btn = gtk.Button(",")
        btn.connect("clicked", self.button_pressed,",")
        digit_table.attach(btn,1,2,4,5)
 
        btn = gtk.Button(".")
        btn.connect("clicked", self.button_pressed,".")
        digit_table.attach(btn,2,3,4,5)

        btn = gtk.Button("+")
        btn.connect("clicked", self.button_pressed,"+")
        digit_table.attach(btn,3,4,4,5)

        btn = gtk.Button("=")
        btn.connect("clicked", self.compute)
        digit_table.attach(btn,4,5,4,5)

#####
	rastersbox = gtk.VBox(spacing=5)
	box1.pack_start(rastersbox)
	rastersbox.show()
	
### source list #############################################################	
	frame1 = gtk.Frame("Select Image Bands To Compute")
	frame1.show()
        box1.pack_start(frame1, expand=False)
	
	box2r = gtk.VBox(spacing=10)
        box2r.set_border_width(10)
	frame1.add(box2r)
	box2r.show()
	
	self.s1_list = \
	    gvutils.GvOptionMenu(self.list_of_bands, self.raster_selected_cb)
	box2r.pack_start(self.s1_list)
	self.s1_list.set_history(-1)
	self.s1_list.show()
		
##### OUT TYPES #########################################################
	box_types = gtk.HBox(spacing=10)
	box2r.pack_start(box_types)
	box_types.show()

	types_label = gtk.Label('Image Data Type:')
	types_label.set_alignment(0, 0.5)
	box_types.pack_start(types_label, expand=False)
	
	self.types_list = []
	i = GDT_Byte
	while i < GDT_TypeCount:
	    self.types_list.append(gdal.GetDataTypeName(i))
	    i += 1

	self.types = gvutils.GvOptionMenu(self.types_list)
	box_types.pack_start(self.types)
	self.types.show()
	
#### NEW VIEW ##########################################################
	self.switch_new_view = gtk.CheckButton("Create New View")
	box1.pack_start(self.switch_new_view)
	self.switch_new_view.show()
	
        return True

#########################################################################	
    def expression_edit_cb(self,*args):
	self.expression = self.expression_unit.get_text()
	
#########################################################################
    def button_pressed(self,widget,txt):
	s = self.expression
	self.expression = s[:self.text_pos] + txt + s[self.text_pos:]
	self.expression_unit.set_text(self.expression)
	self.text_pos += len(txt)

#########################################################################
    def fbutton_pressed(self,widget,txt):
	s = self.expression
	self.expression = s[:self.text_pos] + txt + s[self.text_pos:]
	self.expression_unit.set_text(self.expression)
	self.text_pos += len(txt) - 1

#########################################################################
    def clear_button_pressed(self,*args):
	self.expression = ""
	self.text_pos = 0
	self.expression_unit.set_text(self.expression)
	self.expression_unit.set_position(self.text_pos)

#########################################################################
    def back_button_pressed(self,*args):
	self.expression = \
	    self.expression[:self.text_pos-1]+self.expression[self.text_pos:]
	self.text_pos -= 1
	self.expression_unit.set_text(self.expression)
    
#########################################################################
    def raster_selected_cb(self,*args):
	i_band = self.s1_list.get_history() + 1
	if i_band > 0:
	    s_band = "%" + str(i_band) 
	    s = self.expression
	    self.expression = s[:self.text_pos]+s_band+s[self.text_pos:]
	    self.text_pos += len(s_band)
	    self.expression_unit.set_text(self.expression)
	self.s1_list.set_history(-1)

#########################################################################
    def end_button_pressed(self,*args):
	self.text_pos = len(self.expression)
	self.expression_unit.set_position(self.text_pos)

#########################################################################
    def update_gui(self,*args):
	self.group_changed(self.fun_group_list)

#########################################################################
    def group_changed(self,widget):
	i = self.fun_group_list.get_history()
	self.mfunc_table.hide()
	self.tfunc_table.hide()
	self.bfunc_table.hide()
	self.sfunc_table.hide()
	if i == 0:
	    self.mfunc_table.show()
	elif i == 1:
	    self.bfunc_table.show()
	elif i == 2:
	    self.tfunc_table.show()
	else:
	    self.sfunc_table.show()
#########################################################################	
    def get_band(self,name):
	layer = self.dict_of_bands[name][1]
	b_num = self.dict_of_bands[name][2]
	band = layer.get_parent().get_dataset().GetRasterBand(b_num+1)
	return band	    
########################################################################
    def NDVI(self,s1,s2):
	rb = 128.+ 128. *(s2-s1)/(s1+s2)
	return rb
#########################################################################	
    def compute(self,*args):
	import re
	ex_exp = self.expression

	type = gdal.GetDataTypeByName(self.types_list[self.types.get_history()])
	numtype = gdalnumeric.GDALTypeCodeToNumericTypeCode(type)
	if numtype == None:
	    gvutils.error("Error! Type " + numtype + " is not supported!")
	    return False 

	fun_names_dict = {}
	fun_names_dict["max"] = ("maximum",2)
	fun_names_dict["min"] = ("minimum",2)
	fun_names_dict["asin"] = ("arcsin",1)
	fun_names_dict["acos"] = ("arccos",1)
	fun_names_dict["atan"] = ("arctan",1)
	fun_names_dict["AND"] = ("bitwise_and",2)
	fun_names_dict["OR"] = ("bitwise_or",2)
	fun_names_dict["XOR"] = ("bitwise_xor",2)
	fun_names_dict["inv"] = ("invert",1)
	fun_names_dict["LShift"] = ("left_shift",2)
	fun_names_dict["RShift"] = ("right_shift",1)
	fun_names_dict['NDVI'] = ("self.NDVI",2)
	
	sh_names_list = fun_names_dict.keys()
	for item in sh_names_list:
	    ex_exp = re.sub(item,fun_names_dict[item][0],ex_exp)
	     
	test_exp = ex_exp
	test_array = Numeric.zeros((1, 5), numtype)
	for i in range(len(self.list_of_bands)):
	    patt_i = "%"+str(i+1)
	    repl_i = "sb["+str(i)+"]"
	    ex_exp = re.sub(patt_i,repl_i,ex_exp)
	    test_exp = re.sub(patt_i,"test_array",test_exp)
	ex_exp = "rb="+ex_exp
	test_exp = "test_res="+test_exp
	
	try:
	    exec test_exp
	except:
	    gvutils.error("Illegal expression!")
	    return False

	if self.switch_new_view.get_active():
	    gview.app.new_view()
	    
	b1_name = self.list_of_bands[self.s1_list.get_history()]
	band_list = []
	for i in range(len(self.list_of_bands)):
	    band_list.append(self.get_band(self.list_of_bands[i]))
	b1 = self.get_band(b1_name)
	proto_ds = self.dict_of_bands[b1_name][1].get_parent().get_dataset()
	self.out_buf = Numeric.zeros((b1.YSize, b1.XSize), numtype)

	sb = range(len(self.list_of_bands))
	for y in range(b1.YSize):
	    for i in range(len(self.list_of_bands)):
		sb[i] = \
		    gdalnumeric.BandReadAsArray(band_list[i],0,y,b1.XSize,1)[0]
	    try:
		exec ex_exp
	    except:    
		gvutils.error("ZeroDivide?")
		return False
	    self.out_buf[y, 0:] = clip_result(numtype, rb).astype(numtype)
	
	res_ds = gdalnumeric.OpenArray(self.out_buf,proto_ds)
	gview.app.open_gdal_dataset(res_ds)


TOOL_LIST = ['CalculatorTool', 'RCalculatorTool']

