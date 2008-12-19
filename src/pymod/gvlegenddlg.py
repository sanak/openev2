###############################################################################
# $Id$
#
# Project:  CIETMap / OpenEV
# Purpose:  Implement Legend Display Dialog
# Author:   Frank Warmerdam, warmerda@home.com
#
# Maintained by Mario Beauchamp (starged@gmail.com) for CIETcanada
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

# differentiate between CIETMap and OpenEV
import os
if 'CIETMAP_HOME' in os.environ:
    import cview as gview
    from cietutils import set_help_topic
    pref = gview.app.pref_manager.get_preference
else:
    import gview
    pref = gview.get_preference
    from gvhtml import set_help_topic

import gtk
import pango
from gvclassification import GvClassification
from gvogrfs import gv_to_ogr_color
from pgucolor import color_string_to_tuple

# temporary
def _(s):
    return s

legend_dialogs = []
white = (1.0, 1.0, 1.0, 1.0)
black = (0.0, 0.0, 0.0, 1.0)

def find_legend(layer):
    for ld in legend_dialogs:
        if ld.layer == layer:
            return ld

def show_legend(layer):
    ld = find_legend(layer)
    if ld is not None:
        ld.present()
        return ld

    ld = GvLegendDialog()
    ld.set_raster(layer)
    legend_dialogs.append(ld)
    return ld

# (From Frank)
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
        self.set_title("Legend: Empty")
        self.set_resizable(True)
        self.set_default_size(300, 300)

        self.layer = None
        self.teardown_id = None
        self.changed_id = None
        self.resizing = False
        self.resize_count = 0

        self.viewarea = gview.GvViewArea()
        back_color = pref('legend-background-color', white)
        self.viewarea.set_background_color(color_string_to_tuple(back_color))

        self.shapes = gview.GvShapes()
        self.shapes.set_name('Legend shapes')
        self.vlayer = gview.GvShapesLayer(self.shapes)
        self.vlayer.set_name('Legend layer')
        self.viewarea.add_layer(self.vlayer)
        self.add(self.viewarea)

        self.connect('delete-event', self.close)
        self.show_all()

        self.changing_view_state = False
        self.viewarea.connect('view-state-changed', self.view_state_cb)
        if 'CIETMAP_HOME' in os.environ:
            set_help_topic(self, _('cm-help-legend'))

    def set_raster(self, layer):
        self.layer = layer

        self.changed_id = self.layer.connect('changed', self.check_for_legend_change_cb)
        self.teardown_id = self.layer.connect('teardown', self.close)

        self.cls = GvClassification()
        self.cls.add_layer(layer)
        self.view_state_cb()

    def prepare_legend(self):
        if self.resizing:
            return

        class_cn = self.layer.get_property('Class_sn')
        if class_cn:
            self.Class_sn = int(class_cn)

        #remove any existing shapes
        self.shapes.delete_shapes(range(len(self.shapes)))
        # get preferences
        samp_x_size = pref('legend-sample-x-size', 20)
        samp_y_size = pref('legend-sample-y-size', 20)

        title_font = pref('legend-title-font','Serif 20')
        title_font_color = pref('legend-title-font-color', black)
        title_ogr_color = gv_to_ogr_color(color_string_to_tuple(title_font_color))

        label_font = pref('legend-label-font','Serif 14')
        label_font_color = pref('legend-label-font-color', black)
        label_ogr_color = gv_to_ogr_color(color_string_to_tuple(label_font_color))

        # utility layout for measuring text w/h
        layout = pango.Layout(self.viewarea.get_pango_context())

        try:
            font = pango.FontDescription(title_font)
        except:
            font = pango.FontDescription('Serif 20')
        layout.set_font_description(font)

        #handle multi-line text for the title.
        lines = self.cls.title.split('\\n')

        layout.set_text(self.cls.title.replace('\\n', '\n'))
        title_width, title_height = layout.get_pixel_size()
        h = title_height/len(lines)

        x_offset = 10  #start title 10 pixels in from left edge
        col_offset = 30 #space columns apart
        y_offset = 35  #start title 35 pixels down from top edge
        max_height = 0
        title_width += x_offset + 10
        title_height += y_offset + 10

        #handle large fonts in the sample text
        try:
            font = pango.FontDescription(label_font)
        except:
            # get a default font if preferred one
            # can't be loaded.
            font = pango.FontDescription('Serif 14')
        layout.set_font_description(font)

        layout.set_text(self.cls.name[0])
        w,h = layout.get_pixel_size()
        samp_offset = max(samp_y_size, h) + 10

        cols = int(self.cls.count / 8)
        samps = min(8, self.cls.count)

        samp_height = samps * (samp_offset) + 10
        samp_width = x_offset

        for i in range(cols + 1):
            idx = 8 * i
            col_width = 0
            while idx < self.cls.count and idx < 8 * (i + 1):
                name = self.cls.name[idx]
                layout.set_text(name)
                width = samp_x_size + 20 + layout.get_pixel_size()[0]
                col_width = max(col_width, width)
                idx += 1
            samp_width += (col_width + col_offset)
        samp_width += 10
        total_width = max(title_width, samp_width)
        total_height = title_height + samp_height

        #resize the window appropriately
        self.resizing = True

        x,y,width,height,bpp = self.window.get_geometry()
        if width < total_width or height < total_height:
            self.resize_count += 1
            if self.resize_count < 2:
                self.set_size_request(total_width, total_height)

        self.resizing = False

        for line in lines:
            samp_text = gview.GvShape()
            samp_text.add_node(x_offset, y_offset)
            ogrfs = 'LABEL(c:%s,f:"%s",t:"%s")' % (title_ogr_color, title_font, line)
            samp_text.set_property('_gv_ogrfs', ogrfs)

            self.shapes.append(samp_text)
            y_offset += h

        txt = _('Legend')
        line = lines[0]
        if len(line) > 6 and line[:6] != txt and line[:6] != 'legend':
            self.set_title( '%s: %s...' % (txt, line) )
        else:
            self.set_title(line + '...')

        y_offset += 10
        title_offset = y_offset

        max_width = 0
        max_height = 0

        for class_id in range(self.cls.count):
            color = self.cls.get_color(class_id)
            symbol = self.cls.get_symbol(class_id)
            scale = self.cls.get_scale(class_id)
            name = self.cls.name[class_id]
            if symbol:
                samp = gview.GvShape(type=gview.GVSHAPE_POINT)
                samp.set_node(x_offset+samp_x_size/2, y_offset+samp_y_size/2)
                ogrfs_color = gv_to_ogr_color(color)
                ogrfs = 'SYMBOL(id:%s,c:%s,s:%s)' % (symbol, ogrfs_color, scale/2)
                samp.set_property('_gv_ogrfs', ogrfs)
            else:
                samp = gview.GvShape(type=gview.GVSHAPE_AREA)
                samp.add_node(x_offset, y_offset)
                samp.add_node(x_offset+samp_x_size, y_offset)
                samp.add_node(x_offset+samp_x_size, y_offset+samp_y_size)
                samp.add_node(x_offset, y_offset+samp_y_size)
                samp.add_node(x_offset, y_offset)

                color = '%f %f %f %f' % color
                samp.set_property('_gv_color', color)
                samp.set_property('_gv_fill_color', color)

            self.shapes.append(samp)

            samp_text = gview.GvShape()
            samp_text.add_node(x_offset+samp_x_size+10, y_offset+17)
            ogrfs = 'LABEL(c:%s,f:%s,t:%s)' % (label_ogr_color, label_font, name)
            samp_text.set_property('_gv_ogrfs', ogrfs)
            self.shapes.append(samp_text)

            layout.set_text(name)
            this_width = samp_x_size + 20 + layout.get_pixel_size()[0]
            if max_width < this_width:
                max_width = this_width

            y_offset += samp_offset
            if y_offset+samp_offset > self.viewarea.get_height():
                max_height = max(max_height, y_offset + samp_offset)
                y_offset = title_offset
                x_offset += (col_offset + max_width)
                max_width = 0

        self.vlayer.display_change()

    def check_for_legend_change_cb(self, *args):
        class_cn = self.layer.get_property('Class_sn')
        if class_cn:
            if int(class_cn) == self.Class_sn:
                return
        self.cls.remove_layer(self.layer)
        self.cls.add_layer(self.layer)
        self.prepare_legend()

    def view_state_cb(self, *args):
        if self.changing_view_state:
            return

        self.changing_view_state = True
        w = self.viewarea.get_width()
        h = self.viewarea.get_height()
        self.viewarea.fit_extents(0, h, w, -h)
        self.prepare_legend()
        self.changing_view_state = False

    def close(self, *args):
        if self.teardown_id:
            self.layer.disconnect(self.teardown_id)
            self.layer.disconnect(self.changed_id)

        self.layer = None
##        self.viewarea.signal_handlers_destroy()
##        self.viewarea.destroy()
        self.destroy()
        try:
            legend_dialogs.remove(self)
        except:
            print 'GvLegendDialog.remove failed.'

        return False
