###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  NLS localisation module
# Author:   Paul Spencer, pgs@magma.ca
#
###############################################################################
# Copyright (c) 2000, DM Solutions Group Inc. (www.dmsolutions.on.ca)
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

"""
OpenEV Localization Module.

This module is designed to provide the programmer with a simplified interface
to provided customizable or multi-lingual interfaces.

After this module is imported, call get_locales() to determine which modules
are available and set_locale(name) to set the current locale.  If no locale
is set, the module will always return the default value from
get(key, default).

Variables:
----------

_locales - dictionary, a mapping of locale names to locale files
_locale  - the current locale dictionary

Methods:
--------

_load_locale(fname) - loads the locale file as the current locale
get_locales() - returns the names of the locales that can be selected
set_locale(name) - sets the current locale to name
get(key, default) - gets the localized value of key or default if not found
"""

_locales = {}
_locale = None



#load locale files from the locale directory and parse them for locale
#names ... this only happens on the first import in any particular
#python run-time

import gview
import os
import string
from osgeo import gdal

gdal.Debug( "nls", 'initializing localization module' )

locale_dir = os.path.join(gview.find_gview(), "locales")

if os.path.isdir(locale_dir):
    gdal.Debug( "nls", 'loading localization files from %s' % locale_dir )
    fnames = os.listdir(locale_dir)
    for fname in fnames:
        fname = os.path.join(locale_dir, fname)
        gdal.Debug( "nls",  'processing file %s' % fname )
        if os.path.isfile(fname):
            file = open(fname, 'r')
            loc_dict = {}
            lines = file.readlines()
            for line in lines:
                if string.find(line, '=') >= 0:
                    key, value = string.split(line, '=')

                    if key == 'locale-name':
                        value = string.strip(value)
                        gdal.Debug( "nls", 'loading %s locale' % value )
                        _locales[value] = fname
                        break
            file.close()
            lines = None
        else:
            print '%s is not a file' % fname
    gdal.Debug( "nls", 'localization initialization complete' )
else:
    gdal.Debug( "nls", 'cannot locate localization files' )

def _load_locale(fname):
    """
    open a locale file and set the internal locale

    fname - string, the name of the locale file to load
    """
    global _locale
    try:
        file = open(fname, 'r')
    except:
        gdal.Debug( "nls", 'load locale failed' )
        _locale = None
        return

    loc_dict = {}
    lines = file.readlines()
    for line in lines:
        if line[0] == '#':
            continue
        try:
            if string.find(line, '=') >= 0:
                key, value = string.split(line, '=')
                #take care with CR-LF
                #does this work on UNIX?
                value = string.strip(value)
                value = string.replace(value, '\\n', '\n')
                loc_dict[key] = value
        except:
            gdal.Debug( "nls", 'an exception occurred in nls.py' )
            pass

    file.close()
    _locale = loc_dict
    try:
        gdal.Debug( "nls",  'locale changed to %s' % loc_dict['locale-name'] )
    except:
        gdal.Debug( "nls",  'unknown locale loaded' )

def set_locale(locale_name):
    """
    change locales to the specified locale name

    locale_name - string, the name of the locale (must be one of the
                  keys in 'locales'
    """
    global _locales
    try:
        fname = _locales[locale_name]
        _load_locale(fname)
    except:
        gdal.Debug( "nls",  'locale %s not found' % locale_name )

def get_locales():
    """
    Return a list of available locales
    """
    global _locales
    return _locales.keys()

def get(key, default):
    """
    Return a localized value from the current locale or the default
    value if the the value isn't localized in the current locale

    key - the name of the value to get
    default - the value to return if the key isn't found
    """
    global _locale

    if _locale is None:
        return default

    try:
        return _locale[key]
    except:
        return default

def set( key, value ):
    """
    add a value to the locale

    key - the key to add
    value - the value to add
    """
    global _locale
    if _locale is None:
        return

    _locale[key] = value
