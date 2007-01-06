###############################################################################
# $Id: mil_symbols.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
#
# Project:  LBTB GDA
# Purpose:  test page for military symbols
# Author:   Paul Spencer (pgs@magma.ca)
#
###############################################################################
# Copyright (c) 2003, Paul Spencer (pgs@magma.ca)
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

class MilitarySymbolTool(gviewapp.Tool_GViewApp):
    
    def __init__(self,app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.init_menu()

    def launch_dialog(self,*args):
        self.win = RenderTest()
        self.win.show()

    def init_menu(self):
        self.menu_entries.set_entry("Tools/Military Symbols",2,
                                    self.launch_dialog)

class RenderTest(gtk.Window):

    def __init__(self,app=None):
        gtk.Window.__init__(self)

        self.set_title('Military Symbols')

        self.view = gview.app.sel_manager.get_active_view()
        
        self.text_contents = ''
        self.selected_shape = None
        self.layer = None
        self.create_gui()

        self.step_list = [ self.startup,
                           self.size_symbols ]
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

        text_view = gtk.TextView(gtk.TextBuffer())
        text_view.set_editable(False)
        text_view.set_size_request(400,150)
        text_view.set_wrap_mode(gtk.WRAP_NONE)
        text_view.show()
        self.text_view = text_view
        box1.pack_start(text_view, expand=True)

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

        text_buff = self.text_view.get_buffer()
        text_buff.set_text(text)

###############################################################################
#   Initial test screen.

    def startup( self ):
        self.set_step_name( 'Military Symbols Introduction' )
        self.set_text( 'Welcome to the military symbols rendering tool.' )


###############################################################################
#   Vector symbols (GvShapes via symbolmanager requested via ogrfs).
        
    def size_symbols( self ):
        self.set_step_name( 'Military Size Symbols' )
        self.set_text( \
            'Here you will see each of the size designators for various sizes\n'
            + 'of military symbols\n' )

        shapes = gview.GvShapes()
        gview.undo_register( shapes)

        sm = gview.GvSymbolManager()
        
        #symbol_size * base_size * 2 define the size of the symbol on screen.
        base_size = 1.0
        symbol_size = 3.0 
        
        #the number of pixels wide to make the task force indicator
        tf_offset = ( symbol_size * base_size * 2 ) + 2 
        symbol_offset = tf_offset
                           
        #create the circle symbol as a multiline with n points
        import math
        circle = gview.GvShape( type=gview.GVSHAPE_LINE )
        filled_circle = gview.GvShape( type=gview.GVSHAPE_AREA )
        n = 8
        radius = base_size
        for i in range( n + 1 ):
            angle = (float(i)*(360.0/float(n))) / (180.0/math.pi)
            x = math.sin( angle ) * radius
            y = math.cos( angle ) * radius
            circle.add_node( x, y, 0.0 )
            filled_circle.add_node( x, y, 0.0 )
        circle.set_property( "_gv_ogrfs", "PEN(w:1)" )
        filled_circle.set_property( "_gv_ogrfs", "PEN(w:1);BRUSH()" )
        sm.inject_vector_symbol( "%scircle" % os.sep, circle )
        sm.inject_vector_symbol( "%sfilled_circle" % os.sep, filled_circle)
        
        #create a vertical bar symbol
        shape = gview.GvShape(type=gview.GVSHAPE_LINE )
        shape.add_node( 0.0, -base_size, 0.0 )
        shape.add_node( 0.0, base_size, 0.0 )
        shape.set_property( "_gv_ogrfs", "PEN(w:1)" )
        sm.inject_vector_symbol( "%svertical_bar" % os.sep, shape )

        #now create a cross from rotated vertical bars
        shape = gview.GvShape(type=gview.GVSHAPE_LINE )
        shape.add_node( -base_size, -base_size, 0.0 )
        shape.add_node( base_size, base_size, 0.0 )
        shape.set_property( "_gv_ogrfs", "PEN(w:1)" )
        sm.inject_vector_symbol( "%sline_45" % os.sep, shape )

        shape = gview.GvShape(type=gview.GVSHAPE_LINE )
        shape.add_node( -base_size, base_size, 0.0 )
        shape.add_node( base_size, -base_size, 0.0 )
        shape.set_property( "_gv_ogrfs", "PEN(w:1)" )
        sm.inject_vector_symbol( "%sline_315" % os.sep, shape )
        
        shape = gview.GvShape(type=gview.GVSHAPE_POINT )
        shape.add_node( 0.0, 0.0, 0.0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(id:%sline_45);SYMBOL(id:%sline_315)" % ( os.sep, os.sep ) )
        sm.inject_vector_symbol( "%scross" % os.sep, shape )

        #taskforce 1
        shape = self.create_taskforce_indicator( 1, symbol_size * base_size, tf_offset )
        sm.inject_vector_symbol( "%stf_1" % os.sep, shape )
        
        #taskforce 2
        shape = self.create_taskforce_indicator( 2, symbol_size * base_size, tf_offset )
        sm.inject_vector_symbol( "%stf_2" % os.sep, shape )

        #taskforce 3
        shape = self.create_taskforce_indicator( 3, symbol_size * base_size, tf_offset )
        sm.inject_vector_symbol( "%stf_3" % os.sep, shape )

        #taskforce 4
        shape = self.create_taskforce_indicator( 4, symbol_size * base_size, tf_offset )
        sm.inject_vector_symbol( "%stf_4" % os.sep, shape )

        #taskforce 5
        shape = self.create_taskforce_indicator( 5, symbol_size * base_size, tf_offset )
        sm.inject_vector_symbol( "%stf_5" % os.sep, shape )
        
        #taskforce 6
        shape = self.create_taskforce_indicator( 6, symbol_size * base_size, tf_offset )
        sm.inject_vector_symbol( "%stf_6" % os.sep, shape )
        

        #team symbol
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 0.0, 0.0, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(id:%scircle,s:%s);SYMBOL(id:%sline_45,s:%s)" % (os.sep, symbol_size, os.sep, symbol_size) )
        sm.inject_vector_symbol( "%steam" % os.sep, shape )
        
        #squad symbol
        shape = self.create_size_indicator( "filled_circle", 1, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%ssquad" % os.sep, shape )

        #section symbol
        shape = self.create_size_indicator( "filled_circle", 2, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%ssection" % os.sep, shape )

        #platoon symbol
        shape = self.create_size_indicator( "filled_circle", 3, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%splatoon" % os.sep, shape )

        #company symbol
        shape = self.create_size_indicator( "vertical_bar", 1, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%scompany" % os.sep, shape )
        
        #battalion symbol
        shape = self.create_size_indicator( "vertical_bar", 2, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%sbattalion" % os.sep, shape )
        
        #regiment symbol
        shape = self.create_size_indicator( "vertical_bar", 3, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%sregiment" % os.sep, shape )
        
        #brigade symbol
        shape = self.create_size_indicator( "cross", 1, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%sbrigade" % os.sep, shape )

        #division symbol
        shape = self.create_size_indicator( "cross", 2, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%sdivision" % os.sep, shape )
        
        print 'Division:' + shape.get_property('_gv_ogrfs')
        
        #corps symbol
        shape = self.create_size_indicator( "cross", 3, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%scorps" % os.sep, shape )
        
        #army symbol
        shape = self.create_size_indicator( "cross", 4, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%sarmy" % os.sep, shape )
        
        #armygroup symbol
        shape = self.create_size_indicator( "cross", 5, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%sarmygroup" % os.sep, shape )
        
        #region symbol
        shape = self.create_size_indicator( "cross", 6, symbol_size, symbol_offset )
        sm.inject_vector_symbol( "%sregion" % os.sep, shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 10, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"team\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 10, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%steam)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 10, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%steam);SYMBOL(c:#00FFFF,id:%stf_1" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 15, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"squad\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 15, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%ssquad)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 15, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%ssquad);SYMBOL(c:#00FFFF,id:%stf_1" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 20, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"section\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 20, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%ssection)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 20, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%ssection);SYMBOL(c:#00FFFF,id:%stf_2" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 25, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"platoon\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 25, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%splatoon)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 25, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%splatoon);SYMBOL(c:#00FFFF,id:%stf_3" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 30, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"company\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 30, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%scompany)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 30, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%scompany);SYMBOL(c:#00FFFF,id:%stf_1" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 35, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"battalion\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 35, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sbattalion)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 35, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sbattalion);SYMBOL(c:#00FFFF,id:%stf_2" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 40, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"regiment\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 40, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sregiment)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 40, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sregiment);SYMBOL(c:#00FFFF,id:%stf_3" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 45, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"brigade\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 45, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sbrigade)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 45, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sbrigade);SYMBOL(c:#00FFFF,id:%stf_1" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 50, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"division\")") 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 50, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sdivision)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 50, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sdivision);SYMBOL(c:#00FFFF,id:%stf_2" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 55, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"corps\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 55, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%scorps)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 55, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%scorps);SYMBOL(c:#00FFFF,id:%stf_3" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 60, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"army\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 60, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sarmy)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 60, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sarmy);SYMBOL(c:#00FFFF,id:%stf_4" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 65, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"armygroup\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 65, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sarmygroup)" % os.sep ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 65, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sarmygroup);SYMBOL(c:#00FFFF,id:%stf_5" % (os.sep,os.sep) ) 
        shapes.append( shape )

        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 10, 70, node=0 )
        shape.set_property( "_gv_ogrfs", "LABEL(c:#00FFFF,t:\"region\")" ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 30, 70, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sregion)" % os.sep ) 
        shapes.append( shape )
        
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 50, 70, node=0 )
        shape.set_property( "_gv_ogrfs", "SYMBOL(c:#00FFFF,id:%sregion);SYMBOL(c:#00FFFF,id:%stf_6" % (os.sep,os.sep) ) 
        shapes.append( shape )

        # Create the layer and display

        #turn off display lists for now.
        #gview.set_preference('display_lists', "OFF" )
        
        layer = gview.GvShapesLayer( shapes )
        layer.set_name( 'test' )
        layer.set_property( "_gl_antialias", "1" )
        self.view.add_layer( layer )
        self.view.set_active_layer( layer )

        self.view.fit_extents( 0, 0, 100, 100 )
        
    def create_size_indicator( self, sym_name, repeat, size, offset ):
        """
        compose a size indicator based on some number (repeat) of symbols
        (sym_name) drawn at scale (size). The symbols are drawn horizontally
        spaced at (offset) pixels apart, centered at 0.
        """
        shape = gview.GvShape( type = gview.GVSHAPE_POINT )
        shape.set_node( 0.0, 0.0, node=0 )
        ogrfs = ""
        dx_val = -1 * (offset / 2) * (repeat - 1)
        for i in range( repeat ):
            dx = ""
            if abs(dx_val) > 0.001:
                dx = ",dx:%spx" % dx_val
            sym = "SYMBOL(id:%s%s%s,s:%s)" % (os.sep, sym_name, dx, size)
            if ogrfs != "":
                ogrfs = ogrfs + ";"
            ogrfs = ogrfs + sym
            dx_val = dx_val + offset
        shape.set_property( "_gv_ogrfs", ogrfs )
        
        return shape
        
    def create_taskforce_indicator( self, repeat, size, offset ):
        """
        create a taskforce indicator which is a rectangle with
        no bottom that fits over a size indicator.  Repeat is the
        number of size indicator symbols to cover.  Offset is
        the number of pixels apart that they are.  Size is the
        vertical size of the symbol.
        """
        
        dx = ((offset/2.0) * (repeat)) + 2
        dy = size + 2

        shape = gview.GvShape( type = gview.GVSHAPE_LINE )
        
        shape.add_node( -dx, -dy )
        shape.add_node( -dx, dy )
        shape.add_node( dx, dy )
        shape.add_node( dx, -dy )
        
        return shape
        
                




TOOL_LIST = ['MilitarySymbolTool']

