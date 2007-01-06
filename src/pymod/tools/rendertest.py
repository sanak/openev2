###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Interactive Test Suite for Various Rendering Modes.
# Author:   Frank Warmerdam, warmerdam@pobox.com
#
###############################################################################
# Copyright (c) 2003, Frank Warmerdam <warmerdam@pobox.com>
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
import string           
import gvutils
import os
import gviewapp

class RenderTestTool(gviewapp.Tool_GViewApp):

    def __init__(self,app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.init_menu()

    def launch_dialog(self,*args):
        self.win = RenderTest()
        self.win.show()

    def init_menu(self):
        self.menu_entries.set_entry("Tools/Render Test",2,
                                    self.launch_dialog)

class RenderTest(gtk.Window):

    def __init__(self,app=None):
        gtk.Window.__init__(self)

        self.set_title('GvShapesLayer Render Test')

        self.view = gview.app.sel_manager.get_active_view()

        self.text_contents = ''
        self.selected_shape = None
        self.layer = None
        self.create_gui()

        self.step_list = [ self.startup,
                           self.vector_symbol,
                           self.vector_symbol_from_file,
                           self.ogrfs_points,
                           self.ogrfs_points2,
                           self.ogrfs_lines,
                           self.raster_symbol,
                           self.simple_poly,
                           self.transparent_poly,
                           self.simple_points_and_lines,
                           self.ogrfs_labels ]
        self.step = 0
        self.cleanup_func = None

    def show(self):
        gtk.Window.show_all(self)
        self.show_step()

    def close(self, *args):
        self.hide()
        self.visibility_flag = 0
        return True

    def create_gui(self):
        box1 = gtk.VBox()
        self.add(box1)
        box1.show()

        text = gtk.TextView(gtk.TextBuffer())
        text.set_size_request(400,150)
        text.set_wrap_mode(gtk.WRAP_NONE)
        text.set_editable(False)
        text.show()
        self.text = text
        box1.pack_start(text, expand=True)

        box2 = gtk.HBox()
        box1.pack_start(box2, expand=False)
        box2.show()

        self.prev_btn = gtk.Button("<--- Previous")
        self.prev_btn.connect("clicked", self.prev_cb)
        box2.pack_start(self.prev_btn)

        self.next_btn = gtk.Button("Next --->")
        self.next_btn.connect("clicked", self.next_cb)
        box2.pack_start(self.next_btn)

    def next_cb( self, *args ):
        if self.step < len(self.step_list)-1:
            self.step = self.step + 1
            self.show_step()

    def prev_cb( self, *args ):
        if self.step > 0:
            self.step = self.step - 1
            self.show_step()

    def show_step( self ):
        self.cleanup()

        func = self.step_list[self.step]
        func()

    def cleanup( self ):
        if self.cleanup_func is not None:
            self.cleanup_func()

        layer_list = self.view.list_layers()
        for layer in layer_list:
            self.view.remove_layer( layer )

    def set_step_name( self, text ):
        self.set_title( '%d: %s' % (self.step, text) )

    def set_text( self, text ):
        self.text.get_buffer().set_text(text)

###############################################################################
#   Initial test screen.

    def startup( self ):
        self.set_step_name( 'Render Test Introduction' )
        self.set_text( 'Welcome to the render test.' )

###############################################################################
#   Display a simple polygon with fill using the layer defaults
#       mechanism. 

    def simple_poly( self ):
        self.set_step_name( 'Simple Polygon Display' )
        self.set_text( 'You should see a five sided polygon with a red edge\n'
                       +'and blue fill.  The edge should be 3 pixels wide.\n'
                       +'The polygon should have a triangular hole.')

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        poly = gview.GvShape( type = gview.GVSHAPE_AREA )
        poly.set_node( 10, 10, node=0 )
        poly.set_node( 80, 10, node=1 )
        poly.set_node( 80, 90, node=2 )
        poly.set_node( 50, 80, node=3 )
        poly.set_node( 10, 90, node=4 )
        poly.set_node( 10, 10, node=5 )
        poly.set_node( 20, 20, node=0, ring=1 )
        poly.set_node( 40, 20, node=1, ring=1 )
        poly.set_node( 30, 50, node=2, ring=1 )
        shapes.append( poly )

        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test_poly' )
        layer.set_property( '_area_edge_color', '255 0 0 255' )
        layer.set_property( '_area_fill_color', '0 0 255 255' )
        layer.set_property( '_area_edge_width', '3' )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )


###############################################################################
#   Transparent poly.

    def transparent_poly( self ):
        self.set_step_name( 'Polygon Transparency' )
        self.set_text( 'You should see two overlaping polygons, with the\n'
                       +'rear one showing through the top one a little.' )

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        poly = gview.GvShape( type = gview.GVSHAPE_AREA )
        poly.set_node( 20, 5, node=0 )
        poly.set_node( 40, 5, node=1 )
        poly.set_node( 30, 50, node=2 )
        shapes.append( poly )

        poly = gview.GvShape( type = gview.GVSHAPE_AREA )
        poly.set_node( 10, 10, node=0 )
        poly.set_node( 80, 10, node=1 )
        poly.set_node( 80, 90, node=2 )
        poly.set_node( 50, 80, node=3 )
        poly.set_node( 10, 90, node=4 )
        poly.set_node( 10, 10, node=5 )
        shapes.append( poly )

        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test_poly' )
        layer.set_property( '_area_edge_color', '255 0 0 255' )
        layer.set_property( '_area_fill_color', '0 0 255 150' )
        layer.set_property( '_area_edge_width', '3' )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )

###############################################################################
#   Simple points and lines.

    def simple_points_and_lines( self ):
        self.set_step_name( 'Simple Points and Lines' )
        self.set_text( 'You should see a four segment red line, and\n'
                       +'a blue cross hair 12 pixels tall.\n\n'
                       +'Try zooming in, the crosshair should *not* get bigger.' )

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 20, 50, node=0 )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_LINE )
        shape.set_node( 10, 10, node=0 )
        shape.set_node( 80, 10, node=1 )
        shape.set_node( 80, 90, node=2 )
        shape.set_node( 50, 80, node=3 )
        shape.set_node( 10, 90, node=4 )
        shapes.append( shape )

        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test_poly' )
        layer.set_property( '_line_color', '255 0 0 255' )
        layer.set_property( '_line_width', '3' )
        layer.set_property( '_point_color', '0 0 255 255' )
        layer.set_property( '_point_size', '12' )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )

###############################################################################
#   Test of raster symbol.

    def raster_symbol( self ):
        self.set_step_name( 'Raster Symbol Test' )
        self.set_text( 'A red symbol (the busy indicator from thet toolbar)\n'
                       +'should be shown.  When zooming in or out it should\n'
                       +'remain the same size on screen.\n'
                       +''
                       +'NOTE: This symbols should really be green but it\n'
                       +'seems that currently the color attribute of\n'
                       +'SYMBOL directives for raster symbols is ignored\n'
                       +'\n'
                       +'\n'
                       +'' )

        shapes = gview.GvShapes()
        gview.undo_register( shapes)
        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test' )

        if os.name == "nt":
            sym_file = gview.home_dir + '\\pics\\busy.xpm'
            sym_file2 = gview.home_dir + '\\pics\\idle.xpm'
            sym_name = '\\three_idle'
        else:
            sym_file = gview.home_dir + '/pics/busy.xpm'
            sym_file2 = gview.home_dir + '/pics/idle.xpm'
            sym_name = '/three_idle'


        # Unoffset icon
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 20, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#00FF00,id:"'+sym_file+'")' )
        shapes.append( shape )

        # Pixel Offset
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 20, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#00FF00,dx:30px,dy:-10px,id:"'+sym_file+'")' )
        shapes.append( shape )

        # Geo Offset
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 20, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#00FF00,dx:30g,dy:-30g,id:"'+sym_file+'")' )
        shapes.append( shape )

        # Inject a vector icon symbol that consists of three raster symbols,
        # but do it on the vector layer.

        sm = layer.get_symbol_manager( 1 )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 0.0, 0, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(dy:-10px,dx:-40px,id:"' + sym_file2 + '");' + \
                           'SYMBOL(id:"' + sym_file2 + '");' + \
                           'SYMBOL(dy:10px,dx:40px,id:"' + sym_file2 + '")' )

        sm.inject_vector_symbol( sym_name, shape )

        # Place the "3" idle symbol.

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 60, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#00FF00,id:"'+sym_name+'")' )
        shapes.append( shape )

        # Add line for context.
        shape = gview.GvShape( type = gview.GVSHAPE_LINE )
        shape.set_node( 10, 10, node=0 )
        shape.set_node( 80, 10, node=1 )
        shape.set_node( 80, 90, node=2 )
        shape.set_node( 50, 80, node=3 )
        shape.set_node( 10, 90, node=4 )
        shapes.append( shape )

        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )

###############################################################################
#   _gv_ogrfs controlled lines. 

    def ogrfs_lines( self ):
        self.set_step_name( 'OGRFS Lines' )
        self.set_text('You should see a thick (8 pixels wide) green line\n'
                      +'with a thin (2 pixel wide) red line down the center.\n'
                      +'\n'
                      +'You should also see a second line in blue rendered \n'
                      +'in a dash-dot pattern\n'
                      +'')

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        shape = gview.GvShape( type = gview.GVSHAPE_LINE )
        shape.set_node( 10, 10, node=0 )
        shape.set_node( 80, 10, node=1 )
        shape.set_node( 80, 90, node=2 )
        shape.set_node( 50, 80, node=3 )
        shape.set_node( 10, 90, node=4 )
        shape.set_property( '_gv_ogrfs',
                            'PEN(c:#00FF00,w:8);PEN(c:#FF0000,w:2)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_LINE )
        shape.set_node( 20, 20, node=0 )
        shape.set_node( 40, 20, node=1 )
        shape.set_node( 40, 40, node=2 )
        shape.set_node( 60, 60, node=3 )
        shape.set_node( 40, 80, node=4 )
        shape.set_property( '_gv_ogrfs', 'PEN(c:#0000FF,w:2,p:ogr-pen-6)' )
        shapes.append( shape )

        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test' )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )



###############################################################################
#   _gv_ogrfs controlled points. 

    def ogrfs_points( self ):
        self.set_step_name( 'OGRFS Point Symbols' )
        self.set_text('You should see two rows of yellow symbols.\n'
                      +'First Row: plus, circle, box, triangle, start\n'
                      +'Second Row: X, filled circle, box, triangle and star\n'
                      +'\n'
                      +'Symbols should stay the same size when you zoom.\n'
                      +'\n'
                      +'\n'
                      +'\n'
                      +'' )

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 15, 30, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-1)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 15, 15, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-0)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 15, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-2)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 30, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-3)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 45, 15, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-4)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 45, 30, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-5)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 60, 15, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-6)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 60, 30, 35, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-7)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 75, 15, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-8)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 75, 30, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,s:3,id:ogr-sym-9)' )
        shapes.append( shape )

        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test' )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )

###############################################################################
#   _gv_ogrfs controlled points. 

    def ogrfs_points2( self ):
        self.set_step_name( 'OGRFS Point Symbols' )
        self.set_text( \
            'In the bottom right corner should be a yellow triangle\n'
            + 'pointing "north west".\n'
            + '\n'
            + 'In the middle should be a white cross hair.\n'
            + 'To the top/right is a red X.\n'
            + 'To the lower/left is a green X.\n'
            + '\n'
            + 'As you zoom in the red X should remain a constant distance\n'
            + 'in screen pixels from the white cross.  It is "offset" from\n'
            + 'the white cross a distance in screen pixels.\n'
            + '\n'
            + 'The green X is offset by a "georeferenced" distance, and\n'
            + 'should move closer as you zoom out, and further as you zoom\n'
            + 'in.\n'
            + '\n'
            + '\n'
            + '\n' )

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 15, 15, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFF00,a:45,s:3,id:ogr-sym-7)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFFFF,s:2,id:ogr-sym-0)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FF0000,dx:20px,dy:10px,s:1,id:ogr-sym-1)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#00FF00,dx:-10g,dy:-5g,s:1,id:ogr-sym-1)' )
        shapes.append( shape )

        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test' )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )

###############################################################################
#   _gv_labels

    def ogrfs_labels( self ):
        self.set_step_name( 'OGRFS Labels' )
        self.set_text( \
            'At the top of the screen should be two labels (red and blue)\n'
            + 'The red label has a halo effect.  The blue label has a drop\n'
            + 'shadow effect\n'
            + '\n'
            + 'In the middle of the screen should be four labels (red and\n'
            + 'yellow).  They have different anchor positions, and should\n'
            + 'be arrayed around a center reference point (not shown).\n'
            + 'The labels indicate where they should be (ie. TOP_LEFT should\n'
            + 'be to the top, and right of the center point).\n'
            + '\n'
            + 'In the lower left quadrant is a yellow "PIXEL_OFFSET" label\n'
            + 'that should be right and down of the white reference point.\n'
            + 'The offset is in screen pixels, so as you zoom it should\n'
            + 'remain a constant distance away from the reference point.\n'
            + '\n'
            + 'In the bottom right quadrant is a similar situation but\n'
            + 'the GEO_OFFSET label is offset in georeference coordinates\n'
            + 'so it will move closer to the ref point as you zoom out, and\n'
            + 'further from it as you zoom in.\n'
            + '\n'
            + 'All labels should be selectable and manipulatable.  When\n'
            + 'selected they should have a selection box around them.\n'
            + '\n' )

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        # Test anchors.

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#FFFF00,t:"TOP_RIGHT")' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#FF0000,p:7,t:"BOTTOM_RIGHT")' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#FFFF00,p:9,t:"BOTTOM_LEFT")' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#FF0000,p:3,t:"TOP_LEFT")' )
        shapes.append( shape )

        # more anchor tests
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 35, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#00FF00,p:2,t:"TOP_CENTER");' + \
	                   'SYMBOL(id:ogr-sym-0)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 43, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#00FF00,p:5,t:"CENTER_CENTER");' + \
	                   'SYMBOL(id:ogr-sym-0)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 35, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#00FF00,p:8,t:"BOTTOM_CENTER");' + \
	                   'SYMBOL(id:ogr-sym-0)' )
        shapes.append( shape )

        # Test geographic and pixel offsets.

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 25, 25, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFFFF,s:2,id:ogr-sym-0)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 25, 25, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#FFFF00,dx:20px,dy:10px,t:"PIXEL_OFFSET")')
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 60, 25, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FFFFFF,s:2,id:ogr-sym-0)' )
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 60, 25, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#FFFF00,dx:6g,dy:3g,t:"GEO_OFFSET")')
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 75, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#00FF00,b:#339933,t:"Text with shadow",sh:)')
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 80, node=0 )
        shape.set_property('_gv_ogrfs',
                           'LABEL(c:#FF0000,b:#993333,t:"Text with halo",h:)')
        shapes.append( shape )


        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test' )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )

###############################################################################
#   Vector symbols (GvShapes via symbolmanager requested via ogrfs).

    def vector_symbol( self ):
        self.set_step_name( 'OGRFS Vector Symbol' )
        self.set_text( \
            'You should see two three symbols.  In the middle of the screen\n'
            + 'an arrow pointing straight up.  Up and right of that should\n'
            + 'be a crossed arrow (slightly larger) pointing northeast and\n'
            + 'north west.  The north-west arrow should be green, and the\n'
            + 'north-east one red.\n'
            + '\n'
            + 'To the right of that should be a pair of blue arrows meeting\n'
            + 'at their bases.\n'
            + '\n'
            + '\n' )

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        # Insert a vector symbol that looks like an arrow into the symbol
        # manager.

        sm = gview.GvSymbolManager()

        shape = gview.GvShape( type = gview.GVSHAPE_LINE )
        shape.set_node( 0.0, -6, node=0 )
        shape.set_node( 0.0, 6,  node=1 )
        shape.set_node( -2, 4, node=2 )
        shape.set_node( 0.0, 6,  node=3 )
        shape.set_node( 2, 4,  node=4 )
        shape.set_property('_gv_ogrfs','PEN(w:2)' )

        if os.name == "nt":
            sym_name = "\\rendertest_arrow"
        else:
            sym_name = "/rendertest_arrow"

        sm.inject_vector_symbol( sym_name, shape )

        # Place the symbol.

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FF0000,s:2,id:"'+sym_name+'")' )
        shapes.append( shape )

        # Insert a vector symbol consisting of crossed arrows.

        sm = gview.GvSymbolManager()

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 0.0, 0, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(a:45,c:#00FF00,id:"' + sym_name + '");' + \
                           'SYMBOL(a:-45,id:"' + sym_name + '")' )

        if os.name == "nt":
            sym_name2 = "\\rendertest_carrow"
        else:
            sym_name2 = "/rendertest_carrow"
        sm.inject_vector_symbol( sym_name2, shape )

        # Place the symbol. 

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 60, 60, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#FF0000,s:4,id:"'+sym_name2+'")' )
        shapes.append( shape )

        # Insert a vector symbol consisting scaled arrows with offsets.

        sm = gview.GvSymbolManager()

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 0.0, 0, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(s:0.3333,a:45,dx:-6px,dy:-6px,id:"' + sym_name + '");' + \
                           'SYMBOL(s:0.666,a:-45,dx:12px,dy:-12px,id:"' + sym_name + '")')

        if os.name == "nt":
            sym_name2 = "\\rendertest_carrow2"
        else:
            sym_name2 = "/rendertest_carrow2"
        sm.inject_vector_symbol( sym_name2, shape )

        # Place the symbol. 

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 85, 60, node=0 )
        shape.set_property('_gv_ogrfs',
                           'SYMBOL(c:#0000FF,s:4,id:"'+sym_name2+'")' )
        shapes.append( shape )

        # Create the layer and display

        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test' )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )

###############################################################################
#   Loading vector symbols from XML file.

    def vector_symbol_from_file( self ):
        symbols_dir = os.path.join(gview.home_dir, 'symbols' )
        symbol1 = os.path.join( symbols_dir, 'square.xml' )
        symbol2 = os.path.join( symbols_dir, 'square_filled.xml' )
        symbol3 = os.path.join( symbols_dir, 'cross.xml' )
        symbol4 = os.path.join( symbols_dir, 'x.xml' )
        symbol5 = os.path.join( symbols_dir, 'triangle.xml' )
        symbol6 = os.path.join( symbols_dir, 'triangle_filled.xml' )
        symbol7 = os.path.join( symbols_dir, 'circle.xml' )
        symbol8 = os.path.join( symbols_dir, 'circle_filled.xml' )
        symbol9 = os.path.join( symbols_dir, 'dash.xml' )
        self.set_step_name( 'OGRFS Vector Symbol from File' )
        self.set_text( \
            'In the middle of the screen should be 9 green symbols.\n'
        'Symbols loaded from the following files:\n'
        + symbol1 + '\n'
        + symbol2 + '\n'
        + symbol3 + '\n'
        + symbol4 + '\n'
        + symbol5 + '\n'
        + symbol6 + '\n'
        + symbol7 + '\n'
        + symbol8 + '\n'
        + symbol9 + '\n'
            + '\n'
            + '\n' )

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        sm = gview.GvSymbolManager()

        sm.get_symbol(symbol1)
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 40, 40, node=0 )
        shape.set_property('_gv_ogrfs',
               'SYMBOL(c:#FF0000,s:2,id:"' + symbol1 +'")' )
        shapes.append( shape )

        sm.get_symbol(symbol2)
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 40, node=0 )
        shape.set_property('_gv_ogrfs',
               'SYMBOL(c:#FF0000,s:2,id:"' + symbol2 +'")' )
        shapes.append( shape )

        sm.get_symbol(symbol3)
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 60, 40, node=0 )
        shape.set_property('_gv_ogrfs',
               'SYMBOL(c:#FF0000,s:2,id:"' + symbol3 +'")' )
        shapes.append( shape )

        sm.get_symbol(symbol4)
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 40, 50, node=0 )
        shape.set_property('_gv_ogrfs',
               'SYMBOL(c:#FF0000,s:2,id:"' + symbol4 +'")' )
        shapes.append( shape )

        sm.get_symbol(symbol5)
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property('_gv_ogrfs',
               'SYMBOL(c:#FF0000,s:2,id:"' + symbol5 +'")' )
        shapes.append( shape )

        sm.get_symbol(symbol6)
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 60, 50, node=0 )
        shape.set_property('_gv_ogrfs',
               'SYMBOL(c:#FF0000,s:2,id:"' + symbol6 +'")' )
        shapes.append( shape )

        sm.get_symbol(symbol7)
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 40, 60, node=0 )
        shape.set_property('_gv_ogrfs',
               'SYMBOL(c:#FF0000,s:2,id:"' + symbol7 +'")' )
        shapes.append( shape )

        sm.get_symbol(symbol8)
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 60, node=0 )
        shape.set_property('_gv_ogrfs',
               'SYMBOL(c:#FF0000,s:2,id:"' + symbol8 +'")' )
        shapes.append( shape )

        sm.get_symbol(symbol9)
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 60, 60, node=0 )
        shape.set_property('_gv_ogrfs',
               'SYMBOL(c:#FF0000,s:2,id:"' + symbol9 +'")' )
        shapes.append( shape )

        # Create the layer and display

        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test' )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )


TOOL_LIST = ['RenderTestTool']

