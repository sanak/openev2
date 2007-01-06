#!/usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  GvObject - Base class for gv classes
# Author:   Pete Nagy
#
###############################################################################
# Copyright (c) 2005, Vexcel Corp.
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

import _gv
import weakref

class GvObject(object):
    """
    A base class for gv classes that wrap c module classes.

    If the c module class is a gtk subclass, the python wrapper class
    should extend the c module class as well as this class.  The python
    class __init__ should follow the pattern:

    class ClassType(_module.ClassType, GvObject):
        def __init__(self, _obj = None):
            if (_obj == None):
                _module.ClassType.__init__(self)
                _obj = self
            GvObject.__init__(self, _obj)

    Subclasses should look like:

    class ClassTypeSub(_module.ClassTypeSub, ClassType):
        def __init__(self, _obj = None):
            if (_obj == None):
                _module.ClassTypeSub.__init__(self)
                _obj = self
            ClassType.__init__(self, _obj)
    """
    # used to manage the _gv -> pure python table
    _obj_to_instance = weakref.WeakValueDictionary() 
    def __init__(self, _obj = None):
        if _obj != None:
            if self != _obj:
                # print "Assigning ._o to ",_obj.__class__.__name__
                #
                # We want to reference to _obj around, so it does not get
                # garbage collected while we have it wrapped
                self._o = _obj
                # print "Copying:"
                # print "  _obj: ",_obj
                # print "  self: ",self
                _gv.copy_obj(_obj, self)                
                # print "Copied:"
                # print "  _obj: ",_obj
                # print "  self: ",self

                # enter the item into the table
                self._obj_to_instance[id_from_obj(_obj)] = self

    def connect(self, name, callback):
        """This connect method overrides the gobject connect callback.
        The purpose is to track the link between _gv.* types and
        their corresponding pure python subtypes.  This is done to some
        degree in pygtk, but is not extensive enough to cover every
        possible case."""

        def proxy_callback(*args):
            # look arguments up in the table to determine whether
            # the object needs to be replaced by its child instance

            converted_args = []
            for arg in args:
                # Use an integer id as the key to avoid additional
                # reference counts on the _obj instance.
                key = id_from_obj(arg)
                if self._obj_to_instance.has_key(key):
                    converted_args.append(self._obj_to_instance[key])
                else:
                    converted_args.append(arg)
            return callback(*converted_args)
        return super(GvObject, self).connect(name, proxy_callback)


    #
    # __getattribute__ - Overridden to forward calls to underlying ._o
    # object, if one exists.  This is currently not used, as the copy_obj
    # c module function works on its own, provided the class also extends
    # the wrapped object type.
    #
    def xx__getattribute__(self, name):
        dict = object.__getattribute__(self, '__dict__')
        o = dict.get('_o', None)
	if o is not None:
            if o != self:
                if hasattr(o, name):
                    # print "  attrib " + name + " for _o:"
                    # print "    " + str(getattr(o, name))
                    return getattr(o, name)
        # ret_val = object.__getattribute__(self, name)
        # print "  attrib " + name + " for self:"
        # print "    " + str(ret_val)
        return object.__getattribute__(self, name)

def id_from_obj(obj):
    """Return a unique identifier from the object."""
    return id(obj)


