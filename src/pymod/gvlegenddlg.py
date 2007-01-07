###############################################################################
# $Id$
#
# Project:  CIETMap / OpenEV
# Purpose:  Implement Legend Display Dialog
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

import gtk
import gview
import gvclassification
from gvogrfs import gv_to_ogr_color
from pgucolor import color_string_to_tuple
import pgufont
import sys
import gdal

legend_dialogs = []

def show_legend( layer ):

    for ld in legend_dialogs:
        if ld.layer == layer:
            ld.show()
            ld.window.raise_()
            return ld

    ld = GvLegendDialog()
    ld.set_raster( layer )
    legend_dialogs.append( ld )
    return ld

def find_legend( layer ):

    for ld in legend_dialogs:
        if ld.layer == layer:
            return ld
    return None

#
# It was my goal to have the GvLegendDialog use the GvLegendView to
# implement the legend, and that the GvLegendView would manage a legend
# at some assign region of an existing GvViewArea so that legend could
# eventually be embedded in other views.  I haven't gotten around to that
# yet, but it is still my eventual goal.
#
class GvLegendView:
    def __init__(self):
        pass

class GvLegendDialog(gtk.Window):

    def __init__(self):
        gtk.Window.__init__(self)

        self.layer = None
        self.teardown_id = None
        self.changed_id = None
        self.resizing = False
        self.resize_count = 0

        self.set_title('Legend: Empty')
        self.set_resizable(True)
        self.set_size_request(300, 300)
        self.viewarea = gview.GvViewArea()
        back_color = gview.get_preference('legend-background-color')
        if back_color is None:
            back_color = (1.0, 1.0, 1.0, 1.0 )
        else:
            back_color = color_string_to_tuple(back_color)

        self.viewarea.set_background_color( back_color )
        self.shapes = gview.GvShapes()
        self.vlayer = gview.GvShapesLayer(self.shapes)
        self.viewarea.add_layer( self.vlayer )
        self.add( self.viewarea )

        self.connect( 'delete-event', self.close )
        self.show_all()

        self.viewarea.fit_extents(0,
                                  self.viewarea.get_height(),
                                  self.viewarea.get_width(),
                                  -self.viewarea.get_height() )

        self.changing_view_state = 0
        self.viewarea.connect('view-state-changed', self.view_state_cb )

    def expose_event_cb(self, *args):
        # Attempted to work around problem on some platforms of
        # colors disappearing when a window other than the legend
        # is selected, and not reappearing until the legend is resized.
        # Have not yet fixed...
        print "expose_cb entered.."
        return False

    def set_raster(self, layer ):
        self.layer = layer

        self.changed_id = self.layer.connect('changed',
                                              self.check_for_legend_change_cb )
        self.teardown_id = self.layer.connect('teardown', self.close )

        self.classification = gvclassification.GvClassification()
        self.classification.add_layer( layer )

        self.prepare_legend()

    def prepare_legend(self):
        if self.resizing:
            return

        if self.layer.get_property('Class_sn') is not None:
            self.Class_sn = int(self.layer.get_property('Class_sn'))

        #remove any existing shapes
        self.shapes.delete_shapes(range(len(self.shapes)))

        samp_x_size = gview.get_preference('legend-sample-x-size')
        if samp_x_size is None:
            samp_x_size = 20
        else:
            samp_x_size = int(samp_x_size)

        samp_y_size = gview.get_preference('legend-sample-y-size')
        if samp_y_size is None:
            samp_y_size = 20
        else:
            samp_y_size = int(samp_y_size)

        title_font = pgufont.XLFDFontSpec()
        font_spec = gview.get_preference('legend-title-font')
        if font_spec is None:
            title_font.set_font_part('Family', 'times')
            title_font.set_font_part('Pixel Size', '20')
        else:
            title_font.parse_font_spec(font_spec)


        title_font_color = gview.get_preference('legend-title-font-color')
        if title_font_color is None:
            title_font_color =  (0.0, 0.0, 0.0, 1.0)
        else:
            title_font_color = color_string_to_tuple(title_font_color)
        title_ogr_color = gv_to_ogr_color(title_font_color)

        label_font = pgufont.XLFDFontSpec()
        font_spec = gview.get_preference('legend-label-font')
        if font_spec is None:
            label_font.set_font_part('Family', 'times')
            label_font.set_font_part('Pixel Size', '14')
        else:
            label_font.parse_font_spec(font_spec)

        label_font_color = gview.get_preference('legend-label-font-color')
        if label_font_color is None:
            label_font_color = (0.0, 0.0, 0.0, 1.0)
        else:
            label_font_color = color_string_to_tuple(label_font_color)
        label_ogr_color = gv_to_ogr_color(label_font_color)

        #handle large fonts in the sample text
        try:
            gdk_font = gtk.load_font(str(label_font))
        except:
            # get a default font if preferred one
            # can't be loaded.
            gdk_font = gtk.load_font('*')

        h = gdk_font.height("Wj")
        samp_offset = max(samp_y_size, h) + 10

        #handle multi-line text for the title.
        try:
            gdk_title_font = gtk.load_font(str(title_font))
        except:
            gdk_title_font = gtk.load_font('*')

        lines = self.classification.title.split('\\n')

        x_offset = 10  #start title 10 pixels in from left edge
        col_offset = 30 #space columns apart
        y_offset = 35  #start title 35 pixels down from top edge
        title_width = 0
        max_height = 0

        #resize the window appropriately

        title_height = y_offset
        title_width = 0
        for idx in range(len(lines)):
            line = lines[idx]
            title_height = title_height + gdk_title_font.height(line)
            title_width = max(title_width, gdk_title_font.width(line))

        title_height = title_height + 10
        title_width = x_offset + title_width + 10

        cols = int (self.classification.count / 8)
        samps = min( 8, self.classification.count )

        samp_height = samps * (samp_offset) + 10
        samp_width = x_offset

        for i in range( cols + 1):
            idx = 8 * i
            col_width = 0
            while idx < self.classification.count and idx < 8 * (i + 1):
                name = self.classification.name[idx]
                width = samp_x_size + 20 + gdk_font.width(name)
                col_width = max(col_width, width)
                idx = idx + 1
            samp_width = samp_width + col_width + col_offset
        samp_width = samp_width + 10

        total_width = max(title_width, samp_width)
        total_height = title_height + samp_height

        self.resizing = True

        w, h = self.window.get_size()
        if (w < total_width) or \
           (h < total_height):
            self.resize_count = self.resize_count + 1
            if self.resize_count < 2:
                self.set_size_request(total_width, total_height)

        self.resizing = False

        for idx in range(len(lines)):
            line = lines[idx]
            w = gdk_title_font.width(line)
            h = gdk_title_font.height(line)
            title_width = max(title_width, w)

            samp_text = gview.GvShape()
            samp_text.add_node( x_offset, y_offset )
            samp_text.set_property( '_gv_ogrfs',
                                'LABEL(c:' + title_ogr_color + \
                                ',f:"' + str(title_font) + '",' \
                                + 't:"' + line + '")' )

            self.shapes.append(samp_text)
            y_offset = y_offset + h

        if ((len(lines[0]) > 6) and (lines[0][:6] != 'Legend') and
            (lines[0][:6] != 'legend')):
            self.set_title('Legend: ' + lines[0] + '...')
        else:
            self.set_title(lines[0] + '...')

        y_offset = y_offset + 10
        title_offset = y_offset

        max_width = 0
        max_height = 0

        for class_id in range(self.classification.count):
            color = self.classification.get_color( class_id )
            symbol = self.classification.get_symbol( class_id )
            scale = self.classification.get_scale( class_id )
            if symbol is not None:
                samp = gview.GvShape( type = gview.GVSHAPE_POINT )
                samp.add_node( x_offset + (samp_x_size/2), 
                               y_offset + (samp_y_size/2) )
                ogrfs_color = '#%02x%02x%02x%02x' % (int(color[0] * 255.999),
                                                 int(color[1] * 255.999),
                                                 int(color[2] * 255.999),
                                                 int(color[3] * 255.999))
                ogrfs = "SYMBOL(id:%s,c:%s,s:%s)" % (symbol, ogrfs_color, 
                                              scale)
                samp.set_property( "_gv_ogrfs", ogrfs )
            else:
                samp = gview.GvShape( type = gview.GVSHAPE_AREA )
                samp.add_node( x_offset, y_offset )
                samp.add_node( x_offset+samp_x_size, y_offset )
                samp.add_node( x_offset+samp_x_size, y_offset+samp_y_size )
                samp.add_node( x_offset, y_offset+samp_y_size )
                samp.add_node( x_offset, y_offset )

                color = '%f %f %f %f' % color

                samp.set_property( '_gv_color', color )
                samp.set_property( '_gv_fill_color', color )

            self.shapes.append( samp )

            name = self.classification.name[class_id]
            samp_text = gview.GvShape()
            samp_text.add_node( x_offset+samp_x_size+10, y_offset + 17 )
            font = str(label_font)
            samp_text.set_property( '_gv_ogrfs',
                      'LABEL(c:' + label_ogr_color + \
                      ',f:"'+font+'",t:"'+name+'")'  )
            self.shapes.append( samp_text )

            this_width = samp_x_size + 20 + gdk_font.width(name)
            if max_width < this_width:
                max_width = this_width

            y_offset = y_offset + samp_offset

            if y_offset+samp_offset > self.viewarea.get_height():
                max_height = max(max_height, y_offset + samp_offset)
                y_offset = title_offset
                x_offset = x_offset + col_offset + max_width
                max_width = 0

            self.vlayer.changed()

    def check_for_legend_change_cb(self,*args):
        if self.layer.get_property('Class_sn') is not None:
            if int(self.layer.get_property('Class_sn')) == self.Class_sn:
                return
        self.classification.remove_layer( self.layer )
        self.classification.add_layer( self.layer )
        self.prepare_legend()

    def view_state_cb( self, *args ):

        if self.changing_view_state != 0:
            return True

        self.changing_view_state = 1
        self.viewarea.fit_extents(0,
                                  self.viewarea.get_height(),
                                  self.viewarea.get_width(),
                                  -self.viewarea.get_height() )
        self.prepare_legend()
        self.changing_view_state = 0

        return False

    def close( self, *args ):
        if self.teardown_id is not None:
            self.layer.disconnect(self.teardown_id )
            self.layer.disconnect(self.changed_id )

        try:
            legend_dialogs.remove(self)
        except:
            print 'GvLegendDialog.remove failed.'

        self.layer = None
        self.destroy()

