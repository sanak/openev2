###############################################################################
# $Id$
#
# Project:  Python Gtk Utility Widgets
# Purpose:  Core PGU stuff, such as registering new PyGtk classes.
# Author:   Frank Warmerdam, warmerda@home.com
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

import gtk as _gtk
import _gv

_name2cls = {}

def _obj2inst(obj):
    objname = _gv.gv_get_type_name(obj)
    if _name2cls.has_key(objname):
        return _name2cls[objname](_obj=obj)
    raise 'gtk type ' + objname + ' not found'

def gtk_register(name, class_obj):
    _name2cls[name] = class_obj
