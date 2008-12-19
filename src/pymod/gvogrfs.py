#!/usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV / CIETmap
# Purpose:  Classes for building, and parsing OGR Feature Style Specifications
# Author:   Frank Warmerdam, warmerdam@pobox.com
#
# Maintained by Mario Beauchamp (starged@gmail.com) for CIETcanada
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

def gv_to_ogr_color(rgba):
    if len(rgba) == 3:
        rgba = (rgba[0], rgba[1], rgba[2], 1.0)

    red = min(255,max(0,int(rgba[0] * 255 + 0.5)))
    green = min(255,max(0,int(rgba[1] * 255 + 0.5)))
    blue = min(255,max(0,int(rgba[2] * 255 + 0.5)))
    alpha = min(255,max(0,int(rgba[3] * 255 + 0.5)))

    color = '#%02X%02X%02X' % (red, green, blue)
##    color = '#%02x%02x%02x%02x' % tuple([int(c*255.999) for c in color])
    if alpha != 255:
        color = color + '%02X' % alpha

    return color

def ogr_to_gv_color(ogr_color):
    if len(ogr_color) == 9:
        return (int(ogr_color[1:3],16) / 255.0,
                int(ogr_color[3:5],16) / 255.0,
                int(ogr_color[5:7],16) / 255.0,
                int(ogr_color[7:9],16) / 255.0)
    elif len(ogr_color) == 7:
        return (int(ogr_color[1:3],16) / 255.0,
                int(ogr_color[3:5],16) / 255.0,
                int(ogr_color[5:7],16) / 255.0,
                1.0)
    else:
        return (0,0,0,1.0)

class OGRFeatureStyleParam:
    def __init__(self, parm=None):
        if parm:
            self.parse(parm)

    def set(self, name, value, role='string_value', units=''):
        self.param_name = name
        self.units = units
        self.value = value
        self.role = role

    def parse(self, parm):
        key, value = parm.split(':', 1)
        self.param_name = key

        #trap params that have no value
        if not value:
            self.role = 'numeric_value'
            self.value = ''
            self.units = ''
            return

        # Handle units
        self.units = ''
        if value[-2:] in ['px','pt','mm','cm','in']:
            self.units = value[-2:]
            value = value[:-2]
        elif value[-1:] == 'g':
            self.units = value[-1:]
            value = value[:-1]

        if value[0] == '"':
            if value[-1:] != '"':
                raise ValueError, 'unterminated literal - ' + parm

            self.role = 'string_value'
            self.value = value[1:-1]

        elif value[0] == '{':
            if value[-1:] != '}':
                raise ValueError, 'unterminated fieldname - ' + parm

            self.role = 'field_name'
            self.value = value[1:-1]

        else:
            self.role = 'numeric_value'
            self.value = value

    def unparse(self):
        if self.role == 'numeric_value':
            frmt = '%s:%s%s'
        elif self.role == 'field_name':
            frmt = '%s:{%s}%s'
        else:
            frmt = '%s:"%s"%s'

        return frmt % (self.param_name, self.value, self.units)

    def __str__(self):
        result = '  parm=%s  role=%12s  value=%-20s' % (self.param_name, self.role, self.value)
        if self.units:
            result += (' units:'+self.units)

        return result

class OGRFeatureStylePart:
    def __init__(self):
        pass

    def parse(self, style_part):
        style_part = style_part.strip()
        i = style_part.find('(')
        if i == -1:
            raise ValueError, 'no args to tool name - ' + style_part

        self.tool_name = style_part[:i].upper()
        if self.tool_name not in ('PEN', 'BRUSH', 'SYMBOL', 'LABEL'):
            raise ValueError, 'unrecognised tool name - ' + style_part

        if style_part[-1:] != ')':
            raise ValueError, 'missing end bracket - ' + style_part

        tool_parms = style_part[i+1:-1]

        parms_list = []
        i = 0
        last_i = 0
        in_literal = 0
        while i < len(tool_parms):
            if tool_parms[i] == '"':
                if not in_literal or i == 0 or tool_parms[i-1] != '\\':
                    in_literal = not in_literal

            if not in_literal and tool_parms[i] == ',':
                parms_list.append( tool_parms[last_i:i].strip() )
                i += 1
                last_i = i

            i += 1

        if in_literal:
            raise ValueError, 'unterminated string literal - ' + style_part

        parms_list.append(tool_parms[last_i:].strip())
        self.parms = {}
        for parm_literal in parms_list:
            parm = OGRFeatureStyleParam(parm_literal)
            self.parms[parm.param_name] = parm

    def unparse(self):
        valstr = ','.join( [v.unparse() for v in self.parms.values()] )
        return '%s(%s)' % (self.tool_name, valstr)

    def set_parm(self, parm_obj):
        self.parms[parm_obj.param_name] = parm_obj

    def get_parm(self, parm_name, default_value=None):
        if parm_name in self.parms:
            return self.parms[parm_name].value
        else:
            return default_value

    def get_color(self, default_value=None):
        color = self.get_parm('c', None)
        if color:
##        if color and color[0] != '#':
            return ogr_to_gv_color(color)
        else:
            return default_value

    def __str__(self):
        result = 'Tool:%s\n' % self.tool_name
        for parm in self.parms.values():
            result += '  %s\n' % parm

        return result

class OGRFeatureStyle:
    """
    Encapulation of an OGR Feature Style

    This object keeps one tool of each type in a dictionary and allows parsing
    and unparsing of the ogrfs property that would be stored on a vector
    layer.  The semi-colon separator is used for parts. 
    """
    def __init__(self, style=None):
        self.parts = {}

        if style:
            self.parse(style)

    def parse(self, style):
        """
        parse a style into style parts by breaking it apart at any
        ';' not within '"'
        """
        #TODO: check to see if it is of type string 
        #      or can be turned into a string as well
        if style is None:
            return

        style = style.strip()
        if not style:
            print 'empty style'
            return
        in_quote = 0
        part_start = 0
        for i in range(len(style)):
            char = style[i:i+1]
            if char == chr(34):
                in_quote = not in_quote
            if not in_quote and char == ";":
                part = style[part_start:i]
                self.parse_part(part)
                part_start = i + 1
        #check for the last one ...
        if part_start != 0 or not self.parts:
            part = style[part_start:]
            self.parse_part(part)

    def parse_part(self, part):
        """
        parse a single part
        """
        ogr_part = OGRFeatureStylePart()
        try:
            ogr_part.parse(part)
            self.add_part(ogr_part)
        except:
            print 'Invalid part (%s) in feature style definition' % part

    def unparse(self):
        """
        compose the feature style into a string
        """
        return ';'.join( [v.unparse() for v in self.parts.values()] )

    def add_part(self, ogr_part):
        """
        add an OGRFeatureStylePart
        """
        if issubclass(ogr_part.__class__, OGRFeatureStylePart):
            self.parts[ogr_part.tool_name] = ogr_part
        else:
            raise TypeError, 'ogr_part must be an OGRFeatureStylePart'

    def get_part(self, part_name, default=None):
        if part_name in self.parts:
            return self.parts[part_name]
        else:
            return default

    def remove_part(self, part_name):
        if part_name in self.parts:
            del self.parts[part_name]

    def has_part(self, part_name):
        return self.parts.has_key(part_name)

    def __str__(self):
        result = 'Feature Style Definition:\n'
        for part in self.parts.values():
            result += '  %s\n' % part

        return result

if __name__ == '__main__':
    import sys

    fsp = OGRFeatureStylePart()

    while 1:
        line = raw_input('OGR FS:')
        if len(line) == 0:
            sys.exit(0)

        fsp.parse( line )
        print fsp
        print fsp.unparse()
