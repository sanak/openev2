#!/usr/bin/env python
###############################################################################
# $Id: pgufont.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
#
# Project:  OpenEV Python GTK Utility classes
# Purpose:  Embeddable Font class and font utility classes
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

import gtk
import pango
import gview
import string
import sys
import gdal

"""
pgufont contains utility classes for manipulating and selecting fonts
"""

class XLFDFontSpec:
    """
    encapsulate an X Logical Font Description (XLFD) font specification

    -adobe-helvetica-bold-r-normal--12-120-75-75-p-70-iso8859-1

    The fields in the XLFD are:

    Foundry          the company or organization where the font originated.
    Family           the font family (a group of related font designs).
    Weight           A name for the font's typographic weight For example,
                     'bold' or 'medium').
    Slant            The slant of the font. Common values are 'R' for Roman,
                     'I' for italoc, and 'O' for oblique.
    Set              Width A name for the width of the font. For example,
                     'normal' or 'condensed'.
    Add Style        Additional information to distinguish a font from other
                     fonts of the same family.
    Pixel Size       The body size of the font in pixels.
    Point Size       The body size of the font in 10ths of a point. (A point
                     is 1/72.27 inch)
    Resolution X     The horizontal resolution that the font was designed for.
    Resolution Y     The vertical resolution that the font was designed for .
    Spacing          The type of spacing for the font - can be 'p' for
                     proportional, 'm' for monospaced or 'c' for charcell.
    Average Width    The average width of a glyph in the font. For monospaced
                     and charcell fonts, all glyphs in the font have this width
    Charset Registry The registration authority that owns the encoding for the
                     font. Together with the Charset Encoding field, this
                     defines the character set for the font.
    Charset Encoding An identifier for the particular character set encoding.
    
    GTK2 Port: pguFontControl removed and replaced with the standard
    FontButton control.  Examples in gvogrfsgui and gviewapp.
    """

    xlfd_field_names = ['Foundry','Family','Weight',
                   'Slant','Set','Add Style','Pixel Size',
                   'Point Size','Resolution X','Resolution Y',
                   'Spacing','Average Width','Charset Registry',
                   'Charset Encoding']

    def __init__(self, fontspec = None):
        """
        initialize, optionally parsing a font spec
        """
        self.pango_desc = None
        self.parts = []
        for i in range(14):
            self.parts.append('*')
        if fontspec is not None:
            self.parse_font_spec(fontspec)

    def parse_font_spec(self, font_spec):
        """
        parse a font specification
        
        Now font spec may be XLFD or a pango description.
        """
        self.pango_desc = None
        result = ''

        #coerce XLFDFontSpec classes into strings to parse them
        font_spec = str(font_spec)

        if font_spec[0:1] != '-':
            try:
                self.pango_desc = pango.FontDescription(font_spec)
            except:
                gdal.Debug( "pgufont", "invalid XLFD(%s), should start with -" % font_spec )
            return

        new_parts = string.split(font_spec, '-')
        del new_parts[0] #remove first (empty) part produced by split

        if len(new_parts) != 14:
            gdal.Debug( "pgufont", 'invalid XLFD(%s), should have 14 parts' % font_spec )
            return

        else:
            self.parts = new_parts

    def get_font_part(self, field_name=None, field_number=None):
        """
        Get one part of the font description by field name or number.  If
        both are specified, the result of the field name will be returned
        if it is valid, or if invalid, the field number, or if that is also
        invalid, the an empty string.
        """
        result = ''
        if field_number is not None:
            if field_number < 1 or field_number > 14:
                gdal.Debug( "pgufont", 'invalid field number (%s), should be 1-14' % field_number)
            else:
                result = self.parts[field_number]

        if field_name is not None:
            if self.xlfd_field_names.count(field_name) == 0:
                gdal.Debug( "pgufont", 'invalid field name (%s), should be one of %s' \
                    % (field_name, self.xlfd_field_names) )
            else:
                result = self.parts[self.xlfd_field_names.index(field_name)]

        return result

    def set_font_part(self, field_name, value):
        """
        set a part of the font description
        """
        self.pango_desc = None
        if self.xlfd_field_names.count(field_name) != 0:
            self.parts[self.xlfd_field_names.index(field_name)] = value
        else:
            gdal.Debug( "pgufont", 'invalid field name (%s), should be one of %s' \
                % (field_name, self.xlfd_field_names) )

    def get_display_string(self):
        """
        return a human readable display string for this font
        """
        family = self.get_font_part('Family') + ' '
        weight = self.get_font_part('Weight') + ' '
        if weight == '* ' or weight == 'normal ': weight = ''
        slant = self.get_font_part('Slant') + ' '
        if slant == '* ': slant = ''
        elif slant == 'r ': slant = ''
        elif slant == 'i ' or slant == 'o ': slant = 'Italic '

        unit = ' pt'
        size = self.get_font_part('Point Size')
        if size == '*' or size == '':
            size = self.get_font_part('Pixel Size')
            unit = ' px'
        else:
            size = size[0:len(size)-1]
        return family + weight + slant + size + unit

    def get_font_string(self):
        """
        return this font as a string (for cases where automatic coercion
        doesn't work ...)
        """
        return str(self)
    
    def get_pango_desc(self):
        """ Get a pango font description for this pgufont.
            Assumes font has been set and will not change,
            as once the pango description is computed it
            remains fixed.
        """
        if self.pango_desc is not None:
            return self.pango_desc
        self.pango_desc = pango.FontDescription()
        family = self.get_font_part('Family')
        if family == '*':
            self.pango_desc.set_family('Sans')
        else:
            self.pango_desc.set_family(family)
        style = self.get_font_part('Slant')
        if style[0] == 'I' or style[0] == 'i':
            self.pango_desc.set_style(pango.STYLE_ITALIC)
        if style[0] == 'O' or style[0] == 'o':
            self.pango_desc.set_style(pango.STYLE_OBLIQUE)
        size = self.get_font_part('Pixel Size')
        if size == '*':
            self.pango_desc.set_size(12)
        else:
            self.pango_desc.set_size(int(size))
        weight = self.get_font_part('Weight')
        if weight[0] == 'B' or weight[0] == 'b':
            self.pango_desc.set_weight(pango.WEIGHT_BOLD)
        stretch = self.get_font_part('Set')
        if stretch[0] == 'C' or stretch[0] == 'c':
            self.pango_desc.set_stretch(pango.STRETCH_CONDENSED)
        if stretch[0] == 'E' or stretch[0] == 'e':
            self.pango_desc.set_stretch(pango.STRETCH_EXPANDED)
        
    def set_font_name(self, pango_name = "Sans 12"):
        self.pango_desc = pango.FontDescription(pango_name)
        
    def __str__(self):
        """
        return a representation of this xfld as a string
        """
        result = ''
        for val in self.parts:
            result = result + '-' + val

        return result
