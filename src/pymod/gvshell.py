###############################################################################
# $Id: gvshell.py,v 1.1.1.1 2005/04/18 16:38:35 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Extra intrinsics for OpenEV PyShell environment.  Used from
#           gviewapp.pyshell().
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

import gview
import gdalnumeric
import gdal

###############################################################################
# define easy file view command
def display(array, prototype_name = None):
    import Numeric
    if len(Numeric.shape(array)) == 1:
        array=Numeric.reshape(array,(1,Numeric.shape(array)[0]))
        
    array_name = gdalnumeric.GetArrayFilename(array)
    ds = gview.manager.get_dataset( array_name )
    if prototype_name is not None:
        prototype_ds = gdal.Open( prototype_name )
        gdalnumeric.CopyDatasetInfo( prototype_ds, ds )
            
    gview.app.file_open_by_name( array_name )

###############################################################################
# Utility to get ROI marked with tool from array
def get_roi(num_array):
    roi = gview.app.toolbar.get_roi()
    
    x1 = int(roi[0])
    y1 = int(roi[1])
    x2 = int(roi[0] + roi[2]) + 1
    y2 = int(roi[1] + roi[3]) + 1
    
    return num_array[...,y1:y2,x1:x2]

###############################################################################
def roi():
    return gview.app.toolbar.get_roi()


###############################################################################
#                        local_vars_list()
#
# This function is invoked on locals() by entering "locals" in the
# pyshell.py command window.  The MyInteractiveConsole.push() method
# intercepts the input line and replaces 'locals' with the call.
#
# This function attempts to print out a list of all local variables in the
# command environment that have been defined since the shell was instantiated.
# This is accomplished by keeping a list of variables (and methods) at the
# startup time in the gview.shell_base_vars variable.  This is done by
# the GViewApp.pyshell() function.
#
# All objects existing at startup time are ignored, all others are listed for
# the user.
#
# An attempt is made to distinguish between NumPy arrays, and other variables.
# Numpy arrays will have their shape and type printed in a user friendly
# (hopefully) manner, while other locals are printed with just their name,
# and type.
# 
def local_vars_list( var_list = None, typestrings=None ):
    """local_vars_list( var_list = None, typestrings=None)
       var_list: list of variables to search through
       typestrings: list of types to include (strings
                    corresponding to type(var).__name__,
                    where var is a variable of the type
                    to search for).
    """
    
    import Numeric
    img_type = type(Numeric.array((1,2)))

    type_dict = {'b': 'UnsignedInt8', 'D': 'CFloat64', 'F':'CFloat32',
                 'd': 'Float64', 'f': 'Float32', 'l': 'Int32',
                 '1': 'Int8', 's': 'Int16', 'i': 'Int32' }

    list = var_list.keys()
    txtlist=[]
    for varname in list:
        if varname in gview.shell_base_vars:
            continue

        var = var_list[varname]
        t = type(var)

        # Only show variables of type typestring
        if ((typestrings is not None) and
            (t.__name__ not in typestrings)):
            continue

        if t == img_type:
            shape = Numeric.shape(var)
            shape_str = None
            for dim in shape:
                if shape_str == None:
                    shape_str = ') '
                else:
                    shape_str = 'x' + shape_str
                    
                shape_str = str(dim) + shape_str
            shape_str = ' (' + shape_str
                
            try:
                txtlist.append(varname+ shape_str+ type_dict[var.typecode()])
            except:
                txtlist.append(varname+ shape_str+ var.typecode())
        else:
            txtlist.append('%s (%s)' % (varname, t.__name__))

    return txtlist

