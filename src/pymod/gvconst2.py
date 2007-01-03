###############################################################################
# $Id: gvconst2.py,v 1.1.1.1 2005/04/18 16:38:35 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Declaration of OpenEV constants
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

print 'Whats wrong with this?'
print 'Whats wrong with this?'
print 'Whats wrong with this?'

"""
Constants for use in the GvRasterLayer functions

Constants for texture_mode_set
"""

RL_TEXTURE_REPLACE  = 0
RL_TEXTURE_MODULATE  = 1

print 'Whats wrong with this?'

"""
Constants for zoom_set

Magnification / Minification
"""

RL_FILTER_BILINEAR  = 0
RL_FILTER_NEAREST   = 1

"""
Minification only -- if we ever use mipmaps this is useful
"""

RL_FILTER_TRILINEAR = 2


"""
Preset modes for blend_mode_set
"""

print 'Whats wrong with this?'

RL_BLEND_OFF      = 0
RL_BLEND_FILTER   = 1
RL_BLEND_MULTIPLY = 2
RL_BLEND_ADD      = 3
RL_BLEND_CUSTOM   = 4

"""
Constants for custom blend modes
These can go in both the source and destination modes
"""

RL_BLEND_FACT_ZERO          = 0
RL_BLEND_FACT_ONE           = 1
RL_BLEND_FACT_DST_COLOR     = 2
RL_BLEND_FACT_MIN_DST_COLOR = 3
RL_BLEND_FACT_MIN_SRC_COLOR = 4
RL_BLEND_FACT_SRC_ALPHA     = 5
RL_BLEND_FACT_MIN_SRC_ALPHA = 6

"""
Constants for alpha_mode_set
"""

RL_ALPHA_OFF     = 0
RL_ALPHA_NEVER   = 1
RL_ALPHA_ALWAYS  = 2
RL_ALPHA_LESSER  = 3
RL_ALPHA_LEQUAL  = 4
RL_ALPHA_EQUAL   = 5
RL_ALPHA_GEQUAL  = 6
RL_ALPHA_GREATER = 7
RL_ALPHA_NEQUAL  = 8

"""
Constants to set texture wrapping/clamping
"""

RL_TEXCOORD_CLAMP  = 0
RL_TEXCOORD_REPEAT = 1

"""
Constants to describe type of LUT attached to the RasterLayer
"""

RL_LUT_NONE = 0
RL_LUT_1D   = 1
RL_LUT_2D   = 2

"""
Constants for GvRasterLayer.lut_color_wheel_new() mode arguments.
"""

RL_LUT_MAGNITUDE = 0
RL_LUT_PHASE_ANGLE = 1
RL_LUT_SCALAR = 2
RL_LUT_REAL = 3
RL_LUT_IMAGINARY = 4

"""
GvRasterLayer modes
"""

RLM_AUTO = 0
RLM_SINGLE = 1
RLM_RGBA = 2
RLM_COMPLEX = 3

print 'Whats wrong with this?'


"""
GvShape types.
"""

GVSHAPE_POINT = 1
GVSHAPE_LINE  = 2
GVSHAPE_AREA  = 3
GVSHAPE_COLLECTION = 4


"""
GvRaster and GvShapes change_info types, from gvtypes.h
"""
GV_CHANGE_ADD      = 0x001
GV_CHANGE_REPLACE  = 0x002
GV_CHANGE_DELETE   = 0x003

"""
GvView   2D = Orthonormal Projection
         3D = Perspective
"""
MODE_2D = 0
MODE_3D = 1

print 'Blah?'

"""
GvRaster autoscaling algorithms.
"""

ASAAutomatic = 0
ASAPercentTailTrim = 1
ASAStdDeviation = 2

print 'Blah?'

"""
OGR Feature Style Anchor points for LABELs.
"""

GLRA_LOWER_LEFT  = 1
GLRA_LOWER_CENTER = 2
GLRA_LOWER_RIGHT = 3
GLRA_CENTER_LEFT = 4
GLRA_CENTER_CENTER = 5
GLRA_CENTER_RIGHT = 6
GLRA_UPPER_LEFT = 7
GLRA_UPPER_CENTER = 8
GLRA_UPPER_RIGHT = 9

print 'Blah?'
