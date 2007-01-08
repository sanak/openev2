#!/usr/bin/env python
###############################################################################
# $Id$
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

    GTK2 Port: pguFontControl modified to be a gtk.FontButton with XLFD support.
    Examples in gvogrfsgui and gviewapp.
    """

    xlfd_field_names = ['Foundry','Family','Weight',
                   'Slant','Set','Add Style','Pixel Size',
                   'Point Size','Resolution X','Resolution Y',
                   'Spacing','Average Width','Charset Registry',
                   'Charset Encoding']

    def __init__(self, fontspec=None):
        """
        initialize, optionally parsing a font spec
        """
        self.pango_desc = None
        self.parts = ['*'] * 14
        if fontspec is not None:
            self.parse_font_spec(fontspec)

    def parse_font_spec(self, font_spec):
        """
        parse a font specification

        Now font spec may be XLFD or a pango description.
        """
        self.pango_desc = None

        #coerce XLFDFontSpec classes into strings to parse them
        font_spec = str(font_spec)

        if not font_spec.startswith('-'):
            self.pango_desc = self.parse_font_name(font_spec)
            if self.pango_desc is None:
                gdal.Debug("pgufont", "invalid XLFD(%s), should start with -" % font_spec)
            return

        new_parts = font_spec.split('-')
        del new_parts[0] #remove first (empty) part produced by split

        if len(new_parts) != 14:
            gdal.Debug( "pgufont", 'invalid XLFD(%s), should have 14 parts' % font_spec )
            return

        else:
            self.parts = new_parts

        self.pango_desc = self.get_pango_desc()

    def get_font_part(self, field_name=None, field_number=None):
        """
        Get one part of the font description by field name or number.  If
        both are specified, the result of the field name will be returned
        if it is valid, or if invalid, the field number, or if that is also
        invalid, the an empty string.
        """
        result = ''
        if field_number and field_number <= 14:
            result = self.parts[field_number]
        else:
            gdal.Debug( "pgufont", 'invalid field number (%s), should be 1-14' % field_number)

        fnames = self.xlfd_field_names
        if field_name and field_name in fnames:
            result = self.parts[fnames.index(field_name)]
        else:
            gdal.Debug( "pgufont", 'invalid field name (%s), should be one of %s' \
                % (field_name, self.xlfd_field_names) )

        return result

    def set_font_part(self, field_name, value):
        """
        set a part of the font description
        """
        fnames = self.xlfd_field_names
        if field_name in fnames:
            self.parts[fnames.index(field_name)] = value
        else:
            gdal.Debug( "pgufont", 'invalid field name (%s), should be one of %s' \
                % (field_name, self.xlfd_field_names) )

    def get_display_string(self):
        """
        return a human readable display string for this font
        """
        family = self.get_font_part('Family') + ' '
        weight = self.get_font_part('Weight') + ' '
        if weight in ('* ','normal '): weight = ''
        slant = self.get_font_part('Slant') + ' '
        if slant in ('* ','r '): slant = ''
        elif slant in ('i ','o '): slant = 'Italic '

        unit = ' pt'
        size = self.get_font_part('Point Size')
        if size in ('*',''):
            size = self.get_font_part('Pixel Size')
            unit = ' px'
        else:
            size = size[:-1]
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
        if style[0] in ('I','i'):
            self.pango_desc.set_style(pango.STYLE_ITALIC)
        if style[0] in ('O','o'):
            self.pango_desc.set_style(pango.STYLE_OBLIQUE)

        size = self.get_font_part('Pixel Size')
        if size == '*':
            self.pango_desc.set_size(12)
        else:
            self.pango_desc.set_size(pango.PIXELS(int(size)))

        weight = self.get_font_part('Weight')
        if weight[0] in ('B','b'):
            self.pango_desc.set_weight(pango.WEIGHT_BOLD)

        stretch = self.get_font_part('Set')
        if stretch[0] in ('C','c'):
            self.pango_desc.set_stretch(pango.STRETCH_CONDENSED)
        if stretch[0] in ('E','e'):
            self.pango_desc.set_stretch(pango.STRETCH_EXPANDED)

        return self.pango_desc

    def set_font_name(self, pango_name="Sans 12"):
        self.pango_desc = self.parse_font_name(pango_name)

    def __str__(self):
        """
        return a representation of this xfld as a string
        """
        result = '-'.join(self.parts)

        return '-' + result

    def get_font_name(self):
        """return a representation of this xfld as a pango name"""
        try:
            font_name = self.get_pango_desc().to_string()
        except:
            font_name = "Sans 12"

        return font_name

    def parse_font_name(self, font_name):
        """parse a Pango font name"""
        try:
            pango_desc = pango.FontDescription(font_name)
        except:
            return

        self.parts = ['*'] * 14
        family = pango_desc.get_family()
        if family:
            self.set_font_part('Family', family)
        else:
            self.set_font_part('Family', 'Sans')

        style = pango_desc.get_style()
        if style == pango.STYLE_ITALIC:
            self.set_font_part('Slant', 'i')
        elif style == pango.STYLE_OBLIQUE:
            self.set_font_part('Slant', 'o')

        size = pango_desc.get_size()
        if size:
            self.set_font_part('Pixel Size', str(size/1024))
            self.set_font_part('Point Size', str(10*size/1024))
        else:
            self.set_font_part('Pixel Size', '12')

        weight = pango_desc.get_weight()
        if weight == pango.WEIGHT_BOLD:
            self.set_font_part('Weight', 'b')

        stretch = pango_desc.get_stretch()
        if stretch == pango.STRETCH_CONDENSED:
            self.set_font_part('Set', 'c')
        elif stretch == pango.STRETCH_EXPANDED:
            self.set_font_part('Set', 'e')

        return pango_desc

    def get_width(self, widget):
        context = widget.get_pango_context()
        metrics = context.get_metrics(self.get_pango_desc())
        return pango.PIXELS(metrics.get_approximate_char_width())

    def get_height(self, widget):
        context = widget.get_pango_context()
        metrics = context.get_metrics(self.get_pango_desc())
        return pango.PIXELS(metrics.get_descent())

class FontControl(gtk.FontButton):
    """FontControl is a gtk.FontButton with XLFD support

    fontname - either a XLFD string or a Pango name
    action - optional action to connect to the 'font-set' signal

    """
    def __init__(self, fontname=None, action=None):
        font = XLFDFontSpec(fontname)
        gtk.FontButton.__init__(self, font.get_font_name())
        self.set_use_font(True)
        self.show()
        if action:
            self.connect('font-set', action)

    def get_xlfdfont(self):
        font_name = self.get_font_name()
        font = XLFDFontSpec(font_name)
        font.parse_font_name(font_name)
        return str(font)

    def set_font(self, fontspec):
        font = XLFDFontSpec(fontspec)
        self.set_font_name(font.get_font_name())

