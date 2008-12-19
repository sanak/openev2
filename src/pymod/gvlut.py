###############################################################################
# $Id$
#
# Project:  OpenEV / CIETmap
# Purpose:  Class providing lut utilities
# Author:   Pete Nagy
#
# Maintained by Mario Beauchamp (starged@gmail.com) for CIETcanada
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

from osgeo import gdal
import math
from gvconst import *

class GvLut:
    """
    Provide some utilities for creating luts.  Previously, luts were
    created by specifying a lut type in the GvRasterLayer class, but this
    would create a lut and set it on all sources.  Now by moving the lut
    creation out into utility functions managed by this class, we can
    create the luts and set them on individual sources.

    At some point this class may wrap the libgv GvRasterLut.  It is
    sufficient for now to create GvRasterLuts by setting a byte lut
    on the source, as libgv will use the byte lut to compute a higher
    resolution GvRasterLut.
    """

    def create_lut(lut_type, raster = None):
        """
        Get a 256 entry byte array of the specified type.
        """
        if lut_type in GV_RASTER_LUT_ENHANCE_TYPES:
            fn_name = GV_RASTER_LUT_ENHANCE_TYPES.get(lut_type)
        else:
            if lut_type in GV_RASTER_LUT_ENHANCE_TYPES.values():
                fn_name = lut_type
            else:
                return None

        # Call specific lut enhancement function.
        code = "new_lut = GvLut." + fn_name
        if fn_name == GV_RASTER_LUT_ENHANCE_TYPES.get(LUT_ENHANCE_EQUALIZE):
            code += "(raster)"
        else:
            code += "()"

        exec code

        return new_lut

    create_lut = staticmethod(create_lut)

    def linear():

        lut = ''
        for ii in range(256):
            lut = lut + chr(ii)

        return lut

    linear = staticmethod(linear)

    def log():

        lut = ''
        for ii in range(256):
            value = int((255 * (math.log(1.0+ii) / math.log(256.0)))+0.5)
            if value < 0 :
                value = 0
            elif value >= 255:
                value = 255
            lut = lut + chr(int(value))

        return lut

    log = staticmethod(log)

    def root():

        lut = ''
        for ii in range(256):
            value = 255 * math.sqrt(ii/255.0)
            if value < 0 :
                value = 0
            elif value >= 255:
                value = 255
            lut = lut + chr(int(value))

        return lut

    root = staticmethod(root)

    def square():

        lut = ''
        for ii in range(256):
            value = 255 * math.pow(ii/255.0,2.0)
            if value < 0 :
                value = 0
            elif value >= 255:
                value = 255
            lut = lut + chr(int(value))

        return lut

    square = staticmethod(square)

    def equalize(raster = None):
        if raster is None:
            return None

        (smin, smax) = raster.autoscale()

        gdal_band = gdal.Band(raster.get_band())
        histogram = gdal_band.GetHistogram(smin, smax, approx_ok = 1)

        cum_hist = []
        total = 0
        for bucket in histogram:
            cum_hist.append(total + bucket/2)
            total = total + bucket

        if total == 0:
            total = 1
            gdal.Debug( 'OpenEV',
                   'Histogram total is zero in GvRasterLayer.equalize()' )

        lut = ''
        for ii in range(256):
            value = (cum_hist[ii] * 256L) / total
            if value < 0 :
                value = 0
            elif value >= 255:
                value = 255
            lut = lut + chr(int(value))

        return lut

    equalize = staticmethod(equalize)
