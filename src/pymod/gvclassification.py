###############################################################################
# $Id$
#
# Project:  CIETMap / OpenEV
# Purpose:  GvClassification class responsible for managing classification
#           related properties on a GvRasterLayer.
# Author:   Frank Warmerdam, warmerda@home.com
#
###############################################################################
# Copyright (c) 2000, Frank Warmerdam <warmerda@home.com>
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
#
# TODO: Add Sybmols
#

import gview
import gdal
import gvutils
import Numeric

###############################################################################
#
# Classification Type Handling
#
# A classification type dictates how the default classification for a given set
# of layers is created.  To support different types of classifications, you
# add a new CLASSIFY_XXXXXX declaration using the next available number, update
# CLASSIFY_LAST to the new maximum number, and then add an entry to the
# classification_types dictionary that maps a name to the  number and add code
# to GvClassification.prepare_default() to handle the new type.
#
###############################################################################

#classification types
CLASSIFY_BASE = 0 #used for validation
CLASSIFY_DISCRETE = 0
CLASSIFY_EQUAL_INTERVAL = 1
CLASSIFY_QUANTILE = 2
CLASSIFY_NORM_SD = 3
CLASSIFY_LAST = CLASSIFY_NORM_SD #used for validation

#classification type dictionary
classification_types = { 'Discrete Values' : CLASSIFY_DISCRETE, 
                         'Equal Interval' : CLASSIFY_EQUAL_INTERVAL,
                         'Quantiles' : CLASSIFY_QUANTILE,
                         'Normal Std Dev' : CLASSIFY_NORM_SD }

class GvClassification:

    """Manage layer classification scheme.

    Holds information on a layer classification scheme, and
    can be instantiated from, and serialized to, the properties of the
    GvLayer.  Currently supports GvRasterLayer and GvShapesLayer.
    """

    ###########################################################################
    def __init__(self, layer=None, type=CLASSIFY_EQUAL_INTERVAL):
        self.count = 0
        self.name = []
        self.desc = []
        self.range = []
        self.color = []
        self.title = ''
        self.layers = []
        self.point_symbols = []
        self.symbol_scales = []
        self.set_type( type )

        self.legend_dialog = None

        if layer is not None:
            self.add_layer( layer )

    ###########################################################################
    def dump( self, *args ):
        print str(self)

    ###########################################################################
    def __str__(self):
        s = ''
        s = s + 'Classification Object:\n'
        s = s + 'type: ' + str(self.type) + '\n'
        s = s + 'Title: ' + self.title + '\n'
        s = s + 'count: ' + str(self.count) + '\n'
        for i in range(len(self.name)):
            s = s + 'name: ' + self.name[i] + '\n'
            s = s + 'desc: ' + self.desc[i] + '\n'
            s = s + 'range: ' + str(self.range[i]) + '\n'
            s = s + 'color: ' + str(self.color[i]) + '\n'
            s = s + 'symbol: ' + str(self.point_symbols[i]) + '\n'
            s = s + 'symbol scale: ' + str(self.symbol_scales[i]) + '\n'
        return s

    ###########################################################################
    def add_raster(self, raster, init=1):
        """Add a ranster layer to the list of layers managed by this 
        classification

        DEPRECATED - use add_layer instead
        """
        print "GvClassification.add_raster() is deprecated, " + \
              "use add_layer instead"
        self.add_layer( raster, init )

    ###########################################################################
    def add_layer(self, layer, init = 1, property = None ):
        """Add a layer to the list of layers managed by this
        classification.

        layer -- GvRasterLayer or GvShapesLayer to operate on.

        init -- true (1) to initialize classification from this layer
             -- false(0) to not initialize ...

        property -- the property (attribute) from GvShapesLayers that should
                    be used as the value.

        Initializes classification scheme from indicated layer, remembering
        tied layer.  If available the classification properties are read from
        the GvLayer, otherwise a default classification scheme is prepared."""
        if init:
            self.deserialize( layer.get_properties() )

        if property is not None:
            self.set_classify_property( layer, property )

        if self.get_classify_property( layer ) is None \
           and issubclass(layer.__class__,gview.GvShapesLayer):
            schema = layer.get_parent().get_schema()
            if len(schema) > 0:
                self.set_classify_property( layer, schema[0][0] )

        self.layers.append( layer )

    ###########################################################################
    def remove_raster(self, raster):
        """Remove a raster from the classification

        DEPRECATED - use remove_layer instead
        """
        print "GvClassification.remove_raster() is deprecated, " + \
              "use remove_layer instead"
        self.remove_layer( raster )

    ###########################################################################
    def remove_layer( self, layer):
        """Remove a layer from the classification

        layer -- the GvLayer to remove.
        """
        for n in range(len(self.layers)):
            if layer is self.layers[n]:
                del self.layers[n]
                break

    ###########################################################################
    def remove_all_layers(self):
        """remove all layers currently being managed by this classification"""
        self.layers = []

    ###########################################################################
    def set_classify_property( self, layer, property ):
        """Set property (attribute field) to use for classification."""

        if self.title == 'Legend: ' + str(self.get_classify_property( layer )):
            self.title = 'Legend: ' + property
        layer.set_property( 'Classification_Property', property )

    ###########################################################################
    def get_classify_property( self, layer ):
        """Get property (attribute field) to use for classification."""

        return layer.get_property( 'Classification_Property' )

    ###########################################################################
    def set_class(self, color, range_min, range_max=None,
                    name=None, desc=None, class_id=None, 
                    symbol=None, symbol_scale=None ):
          """Set class info

          Create a new class, or reset the information for an existing class.
          Note that set_class() calls do not cause

          color -- the color value to use as an RGBA tuple with values between
                   0.0 and 1.0.

          range_min -- the minimum value in the pixel value range to be
                       considered part of this class.

          range_max -- the maximum value in the pixel value range to be
                       considered part of this class.  If None, the
                       range_min is used.

          name -- the class name.  If none is provided the class name will
                  be made up from the range.

          desc -- the class description.  If None is provided, an empty
                  string will be used.

          class_id -- the class number to assign, if defaulted a new class will
          be created after the largest existing class number.  Zero
          based.

          symbol -- the symbol to assign to this class (for point layers)

          symbol_scale -- the scale to draw the symbol at

          Returns the class number assigned."""


          if class_id is None:
              class_id = self.count

          if class_id+1 > self.count:
              self.count = class_id+1
              self.name.append('')
              self.desc.append('')
              self.color.append('')
              self.range.append('')
              self.point_symbols.append('')
              self.symbol_scales.append('')

          if desc is None:
              desc = ''

          if range_max is None:
              range_max = range_min

          try:
            range_min = range_min.strip()
            range_max = range_max.strip()
          except:
            pass

          if name is None:
              #name = 'class_%d' % class_id
              name = str(range_min).strip()
              if range_max != range_min and range_max != '':
                name = name + ' - ' + str(range_max).strip()

          if len(color) == 3:
              (r,g,b) = color
              color = (r,g,b,1.0)

          if symbol_scale is None:
            symbol_scale = 2.0

          self.name[class_id] = name
          self.desc[class_id] = desc
          self.color[class_id] = color
          self.range[class_id] = (range_min, range_max)
          self.point_symbols[class_id] = symbol
          self.symbol_scales[class_id] = symbol_scale

          return class_id

    ###########################################################################
    def remove_class(self, class_id):
          """Removes a class

          class_id -- the index of the class to remove."""

          if class_id >= 0 and class_id < self.count:
              del self.name[class_id]
              del self.desc[class_id]
              del self.color[class_id]
              del self.range[class_id]
              del self.point_symbols[class_id]
              del self.symbol_scales[class_id]
              self.count = self.count - 1

    ###########################################################################
    def remove_all_classes(self):
          """Removes all classification data"""

          self.count = 0
          self.name = []
          self.desc = []
          self.range = []
          self.color = []
          self.point_symbols = []
          self.symbol_scales = []

    ###########################################################################
    def update_all_layers(self, rescale=1):
        """Updates all GvLayers managed by this classification"""
        self.order_classes()
        cd = self.serialize()
        for layer in self.layers:
            self.update_layer(layer, cd, rescale )

    ###########################################################################
    def update_layer(self, layer, cd, rescale=1 ):
        ###################################################################
        # Update properties on the GvLayer with the new classification
        # information.

        self.update_layer_properties( layer, cd )

        ###################################################################
        # Do layer specific actions.

        if issubclass(layer.__class__,gview.GvShapesLayer):
            self.update_vector( layer )
        elif issubclass(layer.__class__,gview.GvRasterLayer):
            self.update_raster( layer, rescale )
        else:
            raise ValueError, 'Unsupported layer class in GvClassification'

    ###########################################################################
    def update_raster(self, raster, rescale=1):
          """Updates GvRasterLayer color table.

          Updates the color table on the GvRasterLayer based on the current
          classification scheme.  If rescale is non-zero the GvRaster's
          scaling min and max may also be reset to better handle the given
          data range.

          Note that the GvRasterLayer will rescale data to the range 0-255, and
          the color table is applied after this rescaling.  The range values
          in the GvClassification are in raw data pixel values, but the final
          color table is not.  If the total data ranges of the classes exceeds
          256 then there may be discretization in the generated color table.

          The update_raster() call also causes the classification scheme to
          be updated on the GvRasterLayer's property list.

          raster -- the GvRasterLayer to update

          rescale -- Set to 0 to disable possible changes to GvRaster scaling
          min/max.
          """

          ###################################################################
          # Compute scaling range

          if self.count == 0:
              overall_min = 0
              overall_max = 255
          else:
              overall_min = self.range[0][0]
              overall_max = self.range[0][1]

          for class_id in range(self.count):
              overall_min = min(self.range[class_id][0],overall_min)
              overall_max = max(self.range[class_id][1],overall_max)

          if overall_min < 0 or overall_max > 255 \
             or raster.get_parent().get_band().DataType != gdal.GDT_Byte:
              scale_min = overall_min
              scale_max = overall_max
          else:
              scale_min = 0
              scale_max = 255

          if rescale == 0:
              scale_min = raster.min_get(0)
              scale_max = raster.max_get(0)
          else:
              raster.min_set(0,float(scale_min))
              raster.max_set(0,float(scale_max))

          ###################################################################
          # Build color table

          pct = ''
          for iColor in range(256):
              raw_value = (iColor/255.0) * (scale_max-scale_min) + scale_min

              rep = self.compute_rep( raw_value )
              color = rep[1]

              pct = pct + (chr(int(255*color[0])) + chr(int(255*color[1]))
                           + chr(int(255*color[2])) + chr(int(255*color[3])))

          raster.lut_put( pct )

    ###########################################################################
    def update_vector(self, vector, class_prop = None ):
        """
        Updates GvShapesLayer representation information.

        vector -- the GvShapesLayer to operate on.
        class_prop -- the property based on which to classify.  If not
        provided it will be fetched off the layer metadata.
        """
        if class_prop is None:
            class_prop = self.get_classify_property( vector )


        ###################################################################
        # Set representations for all shapes.

        shapes = vector.get_parent()
        for shape in shapes:

            try:
                raw_value = shape.get_property( class_prop )
            except:
                continue

            try:
                raw_value = float(raw_value)
            except:
                pass

            rep = self.compute_rep( raw_value )

            color = rep[1]
            ogrfs_color = '#%02x%02x%02x%02x' % (int(color[0] * 255.999),
                                                 int(color[1] * 255.999),
                                                 int(color[2] * 255.999),
                                                 int(color[3] * 255.999))

            stype = shape.get_shape_type()
            if stype == gview.GVSHAPE_POINT:
                ogrfs = 'SYMBOL(id:%s,c:%s,s:%s)' % (rep[0], ogrfs_color, 
                                              rep[2])
            elif stype == gview.GVSHAPE_LINE:
                ogrfs = 'PEN(c:%s)' % ogrfs_color
            elif stype == gview.GVSHAPE_AREA:
                ogrfs = 'BRUSH(fc:%s);PEN(c:#010101ff)' % ogrfs_color
            shape.set_property( '_gv_ogrfs', ogrfs )

        shapes.changed()

    ###########################################################################
    def compute_rep( self, raw_value ):

        """
        Compute representation corresponding to a input value.

        raw_value -- the value to run through the classification.

        Returns a list of property information with the following values:

        [0] - symbol name
        [1] - symbol color
        [2] - symbol scale

        """
        rep = []
        symbol = 'ogr-sym-0'
        color = (0.0, 0.0, 0.0, 1.0)
        scale = 2.0

        # Try to produce a string corresponding to the raw value.
        # Ensure that integer values are represented without any decimals
        raw_string = str(raw_value).strip()
        try:
            num_value = float(raw_string)
            if num_value == int(num_value):
                raw_string = str(int(float(raw_string)))
        except:
            pass

        if self.get_type() == CLASSIFY_DISCRETE:
            for i in range(self.count):
                if raw_string == str(self.range[i][0]).strip():
                    symbol = self.point_symbols[i]
                    color = self.color[i]
                    scale = self.symbol_scales[i]
                    break
        else:
            if raw_value < self.range[0][0]:
                symbol = self.point_symbols[0]
                color = self.color[0]
                scale = self.symbol_scales[0]
            elif raw_value > self.range[self.count - 1][1]:
                symbol = self.point_symbols[self.count - 1]
                color = self.color[self.count - 1]
                scale = self.symbol_scales[self.count - 1]
            else:
                for i in range(self.count):
                    #if a value lies directly in the range, then use the value directly
                    if raw_value >= self.range[i][0] \
                       and raw_value <= self.range[i][1] \
                       or raw_value == self.range[i][0]:
                        symbol = self.point_symbols[i]
                        color = self.color[i]
                        scale = self.symbol_scales[i]
                        break

                    #this scales colors and scales for values that are between classes,
                    #but assigns the symbol name from the closest class (favouring the
                    #lower class
                    if i < self.count-1 \
                       and raw_value > self.range[i][1] \
                       and raw_value < self.range[i+1][0]:
                        try:
                            ratio = (raw_value-self.range[i][1]) \
                                    / (self.range[i+1][0] - self.range[i][1])
                        except:
                            ratio = 0.5
                        c1 = self.color[i]
                        c2 = self.color[i+1]
                        s1 = self.symbol_scales[i]
                        s2 = self.symbol_scales[i+1]
                        if ratio > 0.5:
                            symbol = self.point_symbols[i+1]
                        else:
                            symbol = self.point_symbols[i]

                        color = (c1[0] * (1.0 - ratio) + c2[0] * ratio,
                                 c1[1] * (1.0 - ratio) + c2[1] * ratio,
                                 c1[2] * (1.0 - ratio) + c2[2] * ratio,
                                 c1[3] * (1.0 - ratio) + c2[3] * ratio)

                        scale = s1 * (1.0 - ratio) + s2 * ratio

                        break

        rep.append( symbol )
        rep.append( color )
        rep.append( scale )

        return rep

    ###########################################################################
    # Update properties on the GvShapesLayer with the new classification
    # information.
    def update_layer_properties( self, layer, cd ):

        od = layer.get_properties()
        nd = {}

        for key in od.keys():
          if key[:6] != 'Class_':
              nd[key] = od[key]

        for key in cd.keys():
          nd[key] = cd[key]

        # manage a serial number for the class metadata version so the
        # legend can more easily determine if it needs to react.
        if od.has_key('Class_sn'):
          nd['Class_sn'] = str(int(od['Class_sn'])+1)
        else:
          nd['Class_sn'] = '1'

        layer.set_properties( nd )
        layer.changed()

    ###########################################################################
    def swap_classes(self, class_id_1, class_id_2):
          temp = self.name[class_id_1]
          self.name[class_id_1] = self.name[class_id_2]
          self.name[class_id_2] = temp

          temp = self.desc[class_id_1]
          self.desc[class_id_1] = self.desc[class_id_2]
          self.desc[class_id_2] = temp

          temp = self.color[class_id_1]
          self.color[class_id_1] = self.color[class_id_2]
          self.color[class_id_2] = temp

          temp = self.range[class_id_1]
          self.range[class_id_1] = self.range[class_id_2]
          self.range[class_id_2] = temp

    ###########################################################################
    def order_classes(self):
          """Reorder classes in order.

          Reorder classes in ascending over of range_min.  Modify range_max
          values if necessary to avoid overlapping classes."""

          for i in range(self.count):
              for j in range(self.count-i-1):
                  if self.range[j][0] > self.range[j+1][0]:
                      self.swap_classes(j,j+1)

          for i in range(self.count-1):
              if self.range[i][1] > self.range[i+1][0]:
                  self.range[i] = (self.range[i][0], self.range[i+1][0])


    ###########################################################################
    def serialize(self):
          cdict = {}
          for class_id in range(self.count):

              key = 'Class_%d_Name' % class_id
              cdict[key] = self.name[class_id]

              key = 'Class_%d_Desc' % class_id
              cdict[key] = self.desc[class_id]

              key = 'Class_%d_Color' % class_id
              (r, g, b, a) = self.color[class_id]
              r = int(max(0,min(255,r*255.0)))
              g = int(max(0,min(255,g*255.0)))
              b = int(max(0,min(255,b*255.0)))
              a = int(max(0,min(255,a*255.0)))
              cdict[key] = '#%02x%02x%02x%02x' % (r, g, b, a)

              key = 'Class_%d_Range' % class_id
              cdict[key] = '%r:%r' % self.range[class_id]

              if self.point_symbols[class_id] is not None:
                  key = 'Class_%d_Symbol' % class_id
                  cdict[key] = self.point_symbols[class_id]

                  key = 'Class_%d_Scale' % class_id
                  cdict[key] = str(self.symbol_scales[class_id])

          cdict['Classification_Title'] = self.title
          cdict['Classification_Type'] = str(self.get_type())

          return cdict

    ###########################################################################
    def deserialize(self, dict):

        self.remove_all_classes()

        self.title = dict.get('Classification_Title', '')
        type = int(dict.get('Classification_Type', '1'))
        self.set_type( type )

        class_id = 0
        while dict.has_key('Class_%d_Color' % class_id):

            key = 'Class_%d_Name' % class_id
            name = dict.get(key, '')

            key = 'Class_%d_Desc' % class_id
            desc = dict.get(key, '')

            key = 'Class_%d_Color' % class_id
            color_spec = dict.get(key, '#000000FF')

            try:
                color = ( int(color_spec[1:3],16) / 255.0,
                        int(color_spec[3:5],16) / 255.0,
                        int(color_spec[5:7],16) / 255.0,
                        int(color_spec[7:9],16) / 255.0 )
            except:
                color = ( 0.0, 0.0, 0.0, 1.0 )

            key = 'Class_%d_Range' % class_id
            range_spec = dict.get(key, '0 0')
            try:
                (range_min, range_max) = range_spec.split(':')
                range_min = range_min.strip()
                range_max = range_max.strip()
            except:
                range_min = range_spec
                range_min = range_min.strip()
                range_max = None
            #allow ranges to be discrete string values
            try:
                range_min = float(range_min)
                range_max = float(range_max)
            except:
                pass

            key = 'Class_%d_Symbol' % class_id
            try:
                symbol = dict[key]
            except:
                symbol = None

            key = 'Class_%d_Scale' % class_id
            try:
                scale = float(dict[key])
            except:
                scale = None

            self.set_class( color, range_min, range_max, name, desc,
                          class_id, symbol, scale )

            class_id = class_id + 1


    ###########################################################################
    def set_title(self, title):
          """Set new title (for Legend)

      title -- the new title for this classification"""

          self.title = title

    ###########################################################################
    def get_title(self):
        return self.title

    ###########################################################################
    def get_name(self, idx):
        """return the name at the given index or None if the index is invalid"""
        return self.get_value(self.name, idx)

    ###########################################################################
    def set_name(self, idx, name):
        """set the name for the given index"""
        self.set_value(self.name, idx, name)

    ###########################################################################
    def get_color(self, idx):
        """return the color at the given index or None if the index is invalid"""
        return self.get_value(self.color, idx)

    ###########################################################################
    def set_color(self, idx, color):
        """set the color at the given index"""
        if len(color) == 3:
            (r,g,b) = color
            color = (r,g,b,1.0)

        self.set_value(self.color, idx, color)

    ###########################################################################
    def get_desc(self, idx):
        """return the description at the given index or None if the index is invalid"""
        return self.get_value(self.desc, idx)

    ###########################################################################
    def get_range(self, idx):
        """return the range at the given index or None if the index is invalid"""
        return self.get_value(self.range, idx)

    ###########################################################################
    def set_range(self, idx, range_min, range_max):
        self.set_value(self.range, idx, (range_min, range_max))

    ###########################################################################
    def get_value(self, lst, idx):
        """return the item at the given idx from the given list or None if
        something is wrong"""
        result = None
        try:
            result = lst[idx]
        except:
            pass
        return result

    ###########################################################################
    def set_value(self, lst, idx, val):
        """set the item at the given idx in the given list"""
        try:
            lst[idx] = val
        except:
            pass

    ###########################################################################
    def get_symbol(self, idx):
        """return the range at the given index or None if the index is invalid
        """
        return self.get_value(self.point_symbols, idx)

    ###########################################################################
    def set_symbol(self, idx, symbol):
        if symbol[0] == '"':
            symbol = symbol[1:]
        if symbol[-1] == '"':
            symbol = symbol[:-1]
        self.set_value(self.point_symbols, idx, symbol)

    ###########################################################################
    def get_scale(self, idx):
        """return the range at the given index or None if the index is invalid
        """
        return self.get_value(self.symbol_scales, idx)

    ###########################################################################
    def set_scale(self, idx, scale):
        self.set_value(self.symbol_scales, idx, scale)

    ###########################################################################
    def set_type(self, type):
        """Set the classification type - affects prepare_default only
        """
        if type >= CLASSIFY_BASE and type <= CLASSIFY_LAST:
            self.type = type
        else:
            print 'invalid classification type: ', type

    ###########################################################################
    def get_type(self):
        """return the classification type
        """
        return self.type

    ###########################################################################
    def collect_range(self, layer, property = None ):
        if issubclass(layer.__class__,gview.GvShapesLayer):
            shapes = layer.get_parent()
            min_v = None
            max_v = None

            for shape in shapes:
                try:
                    value = float(shape.get_property( property ))
                    if min_v is None:
                        min_v = value
                        max_v = value
                    else:
                        if value < min_v:
                            min_v = value
                        if value > max_v:
                            max_v = value
                except:
                    pass

            return (min_v,max_v)

        elif issubclass(layer.__class__,gview.GvRasterLayer):
            #is there a better way to do this, or is it even necessary?
            if layer.get_mode() == gview.RLM_RGBA:
                return (min(layer.min_get(0),
                            layer.min_get(1),
                            layer.min_get(2)),
                        max(layer.max_get(0),
                            layer.max_get(1),
                            layer.max_get(2)))
            else:
                return (layer.min_get(0), layer.max_get(0))

        else:
            raise ValueError, 'unsupported layer type in collect_range'

    ###########################################################################
    def collect_unique(self, layer, property = None ):
        """
        Collect list of unique values occuring in a GvLayer.

        The returned list is a dictionary with the keys being the unique
        values found, and the values associated with the keys being the
        occurance count for each value.

        layer -- the GvLayer (GvRasterLayer or GvShapesLayer) to be queried
        property -- for GvShapesLayer this is the property name to be scanned.
        """

        if issubclass(layer.__class__,gview.GvShapesLayer):
            shapes = layer.get_parent()

            val_count = {}
            for shape in shapes:
                try:
                    #value = float(shape.get_property( property ))
                    value = shape.get_property( property )
                    if value is not None:
                        if val_count.has_key( value ):
                            val_count[value] = val_count[value] + 1
                        else:
                            val_count[value] = 1
                except:
                    pass

            return val_count

        elif issubclass(layer.__class__,gview.GvRasterLayer):

            raster = layer.get_data()
            datatype = raster.get_band().DataType

            count = 65536
            h_min = raster.get_min()
            h_max = raster.get_max()
            delta = h_max - h_min
            if delta < 0.1:
                delta = 0.1
            h_min = h_min - delta*0.25
            h_max = h_max + delta*0.25

            is_int = 0

            if datatype == gdal.GDT_Byte:
                h_min = 0
                h_max = 256
                count = 256
                is_int = 1
            elif datatype == gdal.GDT_Int16:
                h_min = -32768
                h_max = 32767
                count = 65536
                is_int = 1
            elif datatype == gdal.GDT_UInt16:
                h_min = 0
                h_max = 65536
                count = 65536
                is_int = 1

            histogram = \
                      raster.get_band().GetHistogram( h_min, h_max,
                                                      buckets = count,
                                                      include_out_of_range = 0,
                                                      approx_ok = 0 )
            delta = (h_max - h_min)/count
            val_count = {}
            for i in range(count):
                if histogram[i] > 0:
                    if is_int:
                        value = h_min + i
                    else:
                        value = h_min + delta * i

                    val_count[value] = histogram[i]

            return val_count

        else:
            raise ValueError, 'unsupported layer type in collect_unique'

    ###########################################################################
    def collect_values(self, layer, property = None ):
        """
        Collect the data values from a property field occuring in a GvLayer.

        The returned list contains all values of the property field for shapes
        that have a non-missing value for that field.

        layer -- the GvLayer (GvRasterLayer or GvShapesLayer) to be queried
        property -- for GvShapesLayer this is the property name to be scanned.
        """
        # Obtain the data values for a shapefile vector layer
        if issubclass(layer.__class__,gview.GvShapesLayer):
            shapes = layer.get_parent()

            shpvals = []
            for shape in shapes:
                try:
                    value = float(shape.get_property( property ))
                    if value is not None:
                        shpvals.append(value)
                except:
                    pass

            return shpvals

        # Obtain the data values for a raster layer
        elif issubclass(layer.__class__,gview.GvRasterLayer):

            raster = layer.get_data()
            datatype = raster.get_band().DataType

            count = 65536
            h_min = raster.get_min()
            h_max = raster.get_max()
            delta = h_max - h_min
            if delta < 0.1:
                delta = 0.1
            h_min = h_min - delta*0.25
            h_max = h_max + delta*0.25

            is_int = 0

            if datatype == gdal.GDT_Byte:
                h_min = 0
                h_max = 256
                count = 256
                is_int = 1
            elif datatype == gdal.GDT_Int16:
                h_min = -32768
                h_max = 32767
                count = 65536
                is_int = 1
            elif datatype == gdal.GDT_UInt16:
                h_min = 0
                h_max = 65536
                count = 65536
                is_int = 1

            histogram = \
                      raster.get_band().GetHistogram( h_min, h_max,
                                                      buckets = count,
                                                      include_out_of_range = 0,
                                                      approx_ok = 0 )
            delta = (h_max - h_min)/count
            rastvals = []
            for i in range(count):
                if histogram[i] > 0:
                    if is_int:
                        for j in range(histogram[i]):
                            rastvals.append((h_min + i))
                    else:
                        for j in range(histogram[i]):
                            rastvals.append((h_min + delta * i))
            return rastvals

        else:
            raise ValueError, 'unsupported layer type in collect_unique'


    ###########################################################################
    def quantile(self,values,cats):
        """
        Determine the minimum and maximum values of the break points in a 
        quantile (equal proportions) classification.

        The returned list contains two sublists:
        [0] a list of the minimum values for each category
        [1] a list of the maximum values for each category.

        layer -- the GvLayer (GvRasterLayer or GvShapesLayer) to be queried
        property -- for GvShapesLayer this is the property name to be scanned.
        """

        segl = len(values)/cats # Min number of obs in a category
        lftout = len(values) - (segl*cats) # The "residual" obs needed to be assigned
        catobs = cats*[segl] # initialize the number of observations per category var
        # If there are unassigned obs, assign the needed number of "middle" categories
        # one observation each until the number of assigned obs is equal to actual obs
        if lftout > 0:
            midcat = len(catobs)/2 # the "middle" category
            catobs[midcat] = segl + 1 # up this category by one observation
            # The following loop assigns the remaining obs around the middle category
            for i in range(1,cats):
                if Numeric.sum(catobs) < len(values):
                    catobs[midcat-i] = segl + 1
                else:
                    break
                if Numeric.sum(catobs) < len(values):
                    catobs[midcat+i] = segl + 1
                else:
                    break
        # Use the catobs variable as an index to figure out the location of the break
        # points for the categories
        valsort=Numeric.sort(Numeric.array(values)) # Sort the original data
        maxind = [0]*cats  # the maximum value of each break point
        maxind = list(Numeric.cumsum(catobs) - 1)
        # Based on the index, get the maximum value for each category
        maxval = range(cats) # intialize the maxval variable
        # The following for-loop assigns the top values using maxind
        for i in range(cats):
            maxval[i]=valsort[maxind[i]]
        minval = range(cats) # initialize the bottom break points
        minval[0] = min(values)
        # The following for-loop assigns the other minimum value break points
        for i in range(1,cats):
            minval[i] = maxval[i-1]

        outquant = [minval,maxval]
        return outquant
    ###########################################################################
    def nstddev(self,values):
        """
        Determine the minimum and maximum values of the break points in a 
        classifier based on standard deviations from the mean.  This implicitly
        assumes a symmetric distribution

        The returned list contains two sublists:
        [0] a list of the minimum values for each category
        [1] a list of the maximum values for each category
        [2] the total number of categories as an integer.
        [3] the labels to use in the legend

        layer -- the GvLayer (GvRasterLayer or GvShapesLayer) to be queried
        property -- for GvShapesLayer this is the property name to be scanned.
        """

        mnval = Numeric.sum(values)/len(values) # The mean
        sdval = Numeric.sqrt(Numeric.sum(pow((Numeric.array(values)-mnval),2))/(len(values)-1))
        # The SD
        possds = int((max(values)-mnval)/sdval) # SDs above the mean
        if (max(values)-mnval)/sdval > possds:
            possds = possds + 1
        negsds = int((mnval-min(values))/sdval) # SDs below the mean
        if (mnval-min(values))/sdval > negsds:
            negsds = negsds + 1

        # Handling the maximum category values below the mean
        maxblw = range(negsds)
        blwlbl = range(negsds)
        for i in range(negsds):
            maxblw[((negsds-1)-i)] = mnval - (i*sdval)
            blwlbl[((negsds-1)-i)] = "%i to %i standard deviations" % (-1*i, -1*(i+1))
        # maxblw[(negsds-1)] = mnval - 0.0001

        # Handling the maximum category values above the mean
        maxabv = range(possds)
        abvlbl = range(possds)
        # maxabv[(possds-1)] = max(values)
        # for i in range((possds-1)):
        for i in range(possds):
            maxabv[i] = mnval + ((i+1)*sdval)
            abvlbl[i] = "%i to %i standard deviations" % (i, i+1)
        # maxabv[0] = mnval + 0.0001

        maxval = maxblw + [mnval] + maxabv # concatinate the below and above max values
        meanlb = "Mean: %f" % mnval
        labels = blwlbl + [meanlb] + abvlbl
        minval = range((possds+negsds+1)) # initialize the category min values
        minval[0] = min(values) - 0.0001
        # The following for-loop assigns the other minimum value break points
        for i in range(1,(possds+negsds+1)):
            minval[i] = maxval[i-1]

        # outnsd = [minval,maxval,2*numsds]
        outnsd = [minval,maxval,(possds+negsds+1),labels]
        return outnsd
    ###########################################################################
    def prepare_default(self, count=5):
        """Prepare a default classification scheme.

        count -- the number of classes to create by default (the actual number
                 may differ if creating a discrete classification)

        If the layer is a GvShapesLayer and the classify property is not 
        numeric then the type will be changed to a discrete classication with
        a maximum of 32 discrete values
        """

        if len(self.layers) == 0:
            return

        overall_min = None
        overall_max = None
        bUnique = 0
        unique_vals = []
        symbol = None
        name = None

        for layer in self.layers:
            property = self.get_classify_property( layer )

            if property is not None and \
                issubclass( layer.__class__, gview.GvShapesLayer):
                #determine the type of the 
                props = layer.get_parent().get_properties()
                k = props.keys()[ props.values().index( property ) ]
                try:
                    property_type = props[ "_field_type_%s" % k[12:] ]
                except:
                    property_type = None
                if layer.get_parent()[0].get_shape_type() == gview.GVSHAPE_POINT:
                    symbol = 'ogr-sym-0'
                if property_type == "string":
                    self.set_type( CLASSIFY_DISCRETE )
            else:
                property_type = None

            if self.get_type() == CLASSIFY_DISCRETE:
                vals = self.collect_unique( layer, property )
                keys = vals.keys()
                keys.sort()
                unique_vals.extend( keys )
                count = min( len(unique_vals), 80 )
                if len(unique_vals) > 80:
                    gvutils.warning( '%d discrete values identified, but only the first 80 are being used' % len(unique_vals) )
            else:
                this_min, this_max = self.collect_range( layer, property )
                if overall_min is None:
                    overall_min = this_min
                    overall_max = this_max
                elif this_min is not None:
                    if this_min < overall_min:
                        overall_min = this_min
                    if this_max > overall_max:
                        overall_max = this_max

        if count == 0:
            print 'no values to classify on'
            return

        if overall_min is None and not self.get_type() == CLASSIFY_DISCRETE:
            print 'overall_min still None in prepare_default()!'
            return
        elif self.get_type() == CLASSIFY_DISCRETE:
            overall_min = 0
            overall_max = count

        epsilon = (overall_max-overall_min) * 0.002
        overall_min = overall_min - epsilon
        overall_max = overall_max + epsilon

        # Below are calls to the classifiers that require the entire property field

        if self.get_type() == CLASSIFY_QUANTILE:
            svalues = self.collect_values( layer, property )
            qminmax = self.quantile(svalues,count)

        elif self.get_type() == CLASSIFY_NORM_SD:
            svalues = self.collect_values( layer, property )
            nsdminmax = self.nstddev(svalues)
            count = nsdminmax[2]

        input_incr = (overall_max - overall_min) / count
        color_incr = float(1.0 / (count + 1))

        for n in range(count):
            if self.get_type() == CLASSIFY_EQUAL_INTERVAL:
                range_min = round(overall_min + (input_incr * n), 4)
                range_max = round(overall_min + (input_incr * (n + 1)), 4)
                name = None
            elif self.get_type() == CLASSIFY_QUANTILE:
                range_min = round(qminmax[0][n], 4)
                range_max = round(qminmax[1][n], 4)
                name = None
            elif self.get_type() == CLASSIFY_NORM_SD:
                range_min = round(nsdminmax[0][n], 4)
                range_max = round(nsdminmax[1][n], 4)
                name = nsdminmax[3][n]
            elif self.get_type() == CLASSIFY_DISCRETE:
                try:
                    range_min = unique_vals[n].strip()
                    range_max = ''
                except:
                    range_min = unique_vals[n]
                    range_max = unique_vals[n]
                name = str(unique_vals[n])
            c = float(color_incr * (n + 1))
            self.set_class((c,c,c,1.0), range_min, range_max, name=name, symbol = symbol)          

        if property is not None:
            self.set_title( 'Legend: ' + property )
        else:
            self.set_title('Legend')

if __name__ == '__main__':

    cs = GvClassification()

    print cs.count

    cs.set_class( (1.0, 0.0, 0.0, 1.0), 0 )
    cs.set_class( (0.0, 1.0, 0.0, 1.0), 2 )
    cs.set_class( (0.0, 0.0, 1.0, 1.0), 1, 3 )

    cs.order_classes()

    print cs.count

    d = cs.serialize()

    cs2 = GvClassification()
    cs2.deserialize( d )

    d = cs2.serialize()

    for key in d.keys():
        print key + " = " + d[key]


    print '-------'

    cs.remove_class(1)
    print cs.count
    cs.remove_class(1)
    print cs.count
    d = cs.serialize()
    for key in d.keys():
        print key + ' = ' + d[key]


    #cs2.update_raster()
