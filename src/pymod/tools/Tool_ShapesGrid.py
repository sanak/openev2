###############################################################################
# $Id: Tool_ShapesGrid.py,v 1.1.1.1 2005/04/18 16:38:37 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Tool using scrollable text area widget for displaying GvShapes data
# Author:  
#
###############################################################################
# Copyright (c) 2003, Atlantis Scientific Inc. (www.atlantis-scientific.com)
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

import gviewapp
import gtk
import gview
import pgugrid
import gvutils
import Numeric

class ShapesGridTool(gviewapp.Tool_GViewApp):

    def __init__(self, app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
     
        self.layer = None
        self.shapes = None
        self.viewarea = None
        self.layer_teardown_id=None
        
        self.init_dialog()
        self.init_menu()

    def init_dialog(self):
        self.dialog = gtk.Window()
        self.dialog.set_title('Tabular Shapes Attribute Grid Demo')
        self.dialog.set_default_size(300,400)
        self.dialog.set_policy(False, True, True)

        shell = gtk.VBox(spacing=5)
        shell.set_border_width(10)
        self.dialog.add(shell)

        self.pgugrid=pgugrid.pguGrid(config=(3,2,0,2,4,2,0,0,2))


        shell.pack_start(self.pgugrid,expand=True)

        hbox=gtk.HBox(spacing=5)
        shell.pack_start(hbox)
        self.column_button=gtk.CheckButton("Schema properties only")
        self.column_button.connect("toggled",self.refresh_columns)
        self.column_button.set_active(True)
        hbox.pack_start(self.column_button)
        
        rbutton = gtk.Button("Refresh columns")
        rbutton.connect("clicked", self.refresh_columns)
        hbox.pack_start(rbutton,expand=False)
        
        
        button = gtk.Button("close")
        button.connect("clicked", self.close)
        shell.pack_start(button,expand=False)
        button.show()
        
        shell.show_all()

        # Trap window close event
        self.dialog.connect('delete-event', self.close)

        self.app.sel_manager.subscribe( 'active-layer-changed',
                                         self.layer_update )

        self.pgugrid.subscribe("clicked",self.clicked_cb)
        self.pgugrid.connect("button-release-event",self.clicked_nocolumns_cb)
        #self.pgugrid.subscribe("cell-selection-changed",self.cell_cb)
        #self.pgugrid.subscribe("cell-changed",self.cellch_cb)
        #self.pgugrid.subscribe("row-selection-changed",self.row_cb)
        #self.pgugrid.subscribe("column-selection-changed",self.column_cb)

        # Popup menus:
        itemlist=[#('Configuration',None,None),
                  ('Set subset/Selected',self.set_subset_selected_cb,None),
                  ('Set subset/Unseleted',self.set_subset_unselected_cb,None),
                  ('Set subset/All',self.set_subset_all_cb,None),
                  ('New layer/Selected',self.new_layer_selected_cb,None),
                  ('New layer/Unselected',self.new_layer_unselected_cb,None),
                  ('Edit Schema',self.edit_schema,None),
                  #('Query',None,None),
                  #('Save',None,None),
                  #('Help',None,None)
                  ]

        self.cell_popup_menu=[]          
        self.cell_popup_menu.append(gtk.Menu())
        for label, func, args in itemlist:
            menu_item = gtk.MenuItem(label)
	    if func is not None:
                menu_item.connect("activate", func, args)
            self.cell_popup_menu[0].append(menu_item)
            
        self.cell_popup_menu[0].show_all()
        
        itemlist2=[#('Configuration',None,None),
                  ('Set subset/Selected',self.set_subset_selected_cb,None),
                  ('Set subset/Unseleted',self.set_subset_unselected_cb,None),
                  ('Set subset/All',self.set_subset_all_cb,None),
                  ('New layer/Selected',self.new_layer_selected_cb,None),
                  ('New layer/Unselected',self.new_layer_unselected_cb,None),
                  ('Edit Schema',self.edit_schema,None),
                  ('Add property',self.add_property_column,None),
                  #('Query',None,None),
                  #('Save',None,None),
                  #('Help',None,None)
                  ]

        self.cell_popup_menu.append(gtk.Menu())
        for label, func, args in itemlist2:
            menu_item = gtk.MenuItem(label)
	    if func is not None:
                menu_item.connect("activate", func, args)
            self.cell_popup_menu[1].append(menu_item)
            
        self.cell_popup_menu[1].show_all()
        # Only do actions when grid is visible
        self.active=0

                  
    def init_menu(self):
        self.menu_entries.set_entry("Tools/Tabular Shapes Grid",1,self.show_cb)
        
    def clicked_cb(self,*args):
        """ Show popup menus """
        
        if self.shapes is None:
            return

        row=args[1]
        if row == -1:
            return
        
        event=args[3]
        if ((event.button == 3) and (not
            (event.state & gtk.GDK.CONTROL_MASK)) and (not
            (event.state & gtk.GDK.SHIFT_MASK))):

            if self.column_button.get_active() == True:
                self.cell_popup_menu[0].popup(None,None,None,event.button,
                                      event.time)
            else:
                self.cell_popup_menu[1].popup(None,None,None,event.button,
                                      event.time)

    def clicked_nocolumns_cb(self,*args):
        """ In case where source has no columns defined yet,
            pop up a menu (since clicked_cb won't be called
            until grid has columns defined- grid only sends
            out clicked notification if columns have been
            defined) """

        event=args[1]
        if self.shapes is not None:
            cc=self.pgugrid.get_current_columns()
            if len(cc[0]) == 0:
                if self.column_button.get_active() == True:
                    self.cell_popup_menu[0].popup(None,None,None,event.button,
                                          event.time)
                else:
                    self.cell_popup_menu[1].popup(None,None,None,event.button,
                                          event.time)
                

    def set_subset_selected_cb(self,*args):
        """ Set the grid subset to selected rows only """

        selected=self.pgugrid.get_selected_row_indices()
        
        if len(selected) == 0:
            gvutils.warning('No rows selected- ignoring!')
        else:
            self.pgugrid.set_subset(selected)

    def set_subset_unselected_cb(self,*args):
        """ Set the grid subset to selected rows only """
        
        unselected=self.pgugrid.get_unselected_row_indices()

        if len(unselected) == 0:
            gvutils.warning('No rows unselected- ignoring!')
        else:
            self.pgugrid.set_subset(unselected)

    def set_subset_all_cb(self,*args):
        """ Set the grid to show all shapes """

        self.pgugrid.set_subset(None)

    def new_layer_selected_cb(self,*args):
        """ Create new layer of only selected shapes """
        
        sel=self.pgugrid.get_selected_row_indices()
        if len(sel) == 0:
            gvutils.warning('No rows selected- ignoring!')
            return

        newshps=gview.GvShapes(name='Selected')
        src=self.shapes
        if src is None:
            gvutils.warning('No source layer found- ignoring!')
            return
        
        schema=src.get_schema()
        for item in schema:
            newshps.add_field(item[0],item[1],item[2],item[3])

        for idx in sel:
            if src[idx] is not None:
                newshps.append(src[idx].copy())

        gview.undo_register(newshps)
        clayer=gview.GvShapesLayer(newshps)

        cview=self.app.new_view()
        cview.viewarea.add_layer(clayer)
        cview.viewarea.set_active_layer(clayer)

    def new_layer_unselected_cb(self,*args):
        """ Create new layer of only selected shapes """
        
        unselected=self.pgugrid.get_unselected_row_indices()

        if len(unselected) == 0:
            gvutils.warning('No rows unselected- ignoring!')
            return

        newshps=gview.GvShapes(name='Unselected')
        src=self.shapes
        if src is None:
            gvutils.warning('No source layer found- ignoring!')
            return
        
        schema=src.get_schema()
        for item in schema:
            newshps.add_field(item[0],item[1],item[2],item[3])

        for idx in unselected:
            if src[idx] is not None:
                newshps.append(src[idx].copy())

        gview.undo_register(newshps)
        clayer=gview.GvShapesLayer(newshps)

        cview=self.app.new_view()
        cview.viewarea.add_layer(clayer)
        cview.viewarea.set_active_layer(clayer)

    def get_columns(self,shapes):
        """ Get the columns for a shapes object """

        # should skip to except here if layer is a raster
        shp_schema = self.shapes.get_schema()

        klist=[]
        fdict={}
        for item in shp_schema:
            klist.append(item[0])
            if item[1] == 'integer':
                fdict[item[0]]="%"+str(item[2])+"d"
            elif item[1] == 'float':
                fdict[item[0]]="%"+str(item[2])+"."+\
                             str(item[3])+"f"
            else:
                fdict[item[0]]="%"+str(item[2])+"s"

        if self.column_button.get_active() == False:
            for shp in self.shapes:
                klist.extend(shp.get_properties().keys())

            #klist.sort()
            # Get unique keys
            set={}
            key_list=\
               [set.setdefault(e,e) for e in klist if not set.has_key(e)]
            
        else:
            #klist.sort()
            key_list=klist

        flist=[]
        for item in key_list:
            if fdict.has_key(item):
                flist.append(fdict[item])
            else:
                flist.append("%s")
            
        return key_list,flist

    def edit_schema(self,*args):
        """ Edit the shapes schema. """
        if self.shapes is None:
            return

        sch=SchemaDialog(self.shapes,self)
        
            
    def add_property_column(self,*args):
        """ Add a property column """

        import GtkExtra
        pname=GtkExtra.input_box(title="Property Name",
              message="Please enter a name for the property")

        if ((pname is None) or (len(pname) == 0)):
            return
        
        current_columns=self.pgugrid.get_current_columns()
        mems=current_columns[0]
        titles=current_columns[1]
        editables=current_columns[2]
        formats=current_columns[3]
        types=current_columns[4]
        nodatas=current_columns[5]
        justify=current_columns[6]
        tjustify=current_columns[7]
        
        mems.append(pname)
        titles.append(pname)
        editables.append(1)
        formats.append(None)
        types.append('string')
        nodatas.append('')
        justify.append(0)
        tjustify.append(2)

        self.pgugrid.define_columns(members=mems,titles=titles,
             editables=editables,formats=formats,types=types,
             nodata=nodatas,justify=justify,title_justify=tjustify)
        
        
    def refresh_columns(self,*args):
        """ Update column headers """
        if self.shapes is not None:
            key_list,fmt_list=self.get_columns(self.shapes)
            self.pgugrid.define_columns(members=key_list,formats=fmt_list)

    def show_cb(self,*args):
        # Activate the view that the grid was launched from
        # (confusing for user if launching the grid from
        # one view shows the vectors for another)
        for view in self.app.view_manager.view_list:
            if view.title == args[1]:
                self.app.view_manager.set_active_view(view)
                
        self.active=1
        self.dialog.show_all()
        self.dialog.window.raise_()
        self.layer_update()

    def close(self,*args):
        self.active=0
        self.layer_teardown_cb()
        self.dialog.hide()
        return True
    

    def layer_update(self,*args):
        # Disconnect from the old layer
        if self.active == 0:
            return
        
        self.layer_teardown_cb()

        try:
            self.layer = self.app.sel_manager.get_active_layer()
            self.viewarea = self.app.sel_manager.get_active_view()
            self.shapes = self.layer.get_parent()

            # should skip to except here if layer is a raster
            shp_schema = self.shapes.get_schema()

        except:
            # Layer is None or a raster
            self.layer = None
            self.shapes = None
            self.viewarea = None
            self.pgugrid.set_source(None,expose=1)
            self.layer_teardown_id=None
            return

        try:
            # get display columns
            key_list,fmt_list=self.get_columns(self.shapes)

            self.pgugrid.set_source(self.layer,self.viewarea)
            self.pgugrid.define_columns(members=key_list,formats=fmt_list)
            
            self.layer_teardown_id = \
                self.layer.connect('teardown',self.layer_teardown_cb)
        except:
            # Layer is None or a raster, or couldn't be loaded
            self.layer = None
            self.shapes = None
            self.viewarea = None
            self.layer_teardown_id=None
            self.pgugrid.set_source(None,expose=1)
            gvutils.warning('Unable to load shapes into grid!')
            return

    def layer_teardown_cb(self,*args):
        self.pgugrid.clear()
        if self.layer_teardown_id is not None:
            self.layer.disconnect(self.layer_teardown_id)
            self.layer_teardown_id = None
        self.viewarea = None


    def clicked_test_cb(self,*args):
        """ Test the grid selection mechanism- debug code """
        print "clicked..."
        print "grid row: ",args[1]
        src=self.pgugrid.grid2src(args[1])
        print "source row: ",src
        print "grid row 2: ",self.pgugrid.src2grid(src)
        print "grid column: ",args[2]
        print "dir(event): ",dir(args[3])

    def cell_cb(self,*args):
        """ Test the grid selection mechanism- debug code """
        print "cell_cb..."
        print "cell selection: ",args[1]
            
    def cellch_cb(self,*args):
        """ Test the grid selection mechanism- debug code """
        print "cellch_cb..."
        print "altered cell: ",args[1]
            
    def row_cb(self,*args):
        """ Test the grid selection mechanism- debug code """
        print "row_cb..."
        print "row selection: ",args[1]
        src=self.pgugrid.src2grid(args[1])
        print "grid row selection: ",src
        print "row selection 2: ",self.pgugrid.grid2src(src)
        
    def column_cb(self,*args):
        """ Test the grid selection mechanism- debug code """
        print "column_cb..."
        print "column selection: ",args[1]
            

class SchemaDialog(gtk.Window):
    def __init__(self,shapes,shapesgridtool=None):
        gtk.Window.__init__(self)
        self.set_title('Schema')
        shell=gtk.VBox(spacing=5)
        shell.set_border_width(10)
        self.add(shell)
        self.grid=pgugrid.pguGrid(config=(2,0,0,1,4,0,0,0))
        self.grid.subscribe("cell-changed",self.changed_field)
        self.shapes=shapes
        self.shapesgridtool=shapesgridtool
        shell.pack_start(self.grid)

        # New field
        box3 = gtk.Table(rows=5,cols=3)
        box3.set_row_spacings(5)
        box3.set_col_spacings(5)
        box3.set_border_width(10)
        nf_frame =  gtk.Frame('Add Field')
        nf_frame.add(box3)
        self.new_field_name_entry =  gtk.Entry(10)
        self.new_field_name_entry.set_text('')
        self.new_field_name_entry.set_editable(True) 
        self.new_field_width_entry =  gtk.Entry(2)
        self.new_field_width_entry.set_text('20')
        self.new_field_width_entry.set_editable(True)        
        self.new_field_precision_entry =  gtk.Entry(2)
        self.new_field_precision_entry.set_text('0')
        self.new_field_precision_entry.set_editable(False)
        self.new_field_precision_entry.set_sensitive(False)
        
        self.new_field_types = ('string','integer','float')
        self.new_field_type_menu = gvutils.GvOptionMenu(self.new_field_types, self.new_field_precision_cb)
        self.new_field_type_menu.set_history(0)
        box3.attach(gtk.Label('Name'),0,1,0,1)
        box3.attach(self.new_field_name_entry,1,2,0,1)
        box3.attach(gtk.Label('Type'),0,1,1,2)
        box3.attach(self.new_field_type_menu,1,2,1,2)
        box3.attach(gtk.Label('Width'),0,1,2,3)
        box3.attach(self.new_field_width_entry,1,2,2,3)
        box3.attach(gtk.Label('Precision'),0,1,3,4)
        box3.attach(self.new_field_precision_entry,1,2,3,4)
        button = gtk.Button("Add")
        box3.attach(button,0,2,4,5)
        button.connect("clicked", self.add_field)
        
        shell.pack_start(nf_frame)
        nf_frame.show_all()
        
        # Ability to delete fields?
        self.fill_grid()
        self.grid.resize_to_default()
        self.show_all()

    def fill_grid(self):
        """ Get the schema and fill the grid """
        sch=self.shapes.get_schema()
        schl=[]
        for item in sch:
            schl.append(list(item))
        if len(schl) > 0:    
            self.grid.set_source(schl,expose=0)
        self.grid.define_columns(titles=['Name','Type','Width','Precision'],
                            editables=[0,0,1,1])

    def add_field(self,*args):
        """ Add field """
        import string
        
        sch=self.shapes.get_schema()
        name=self.new_field_name_entry.get_text()

        for item in sch:
            if string.lower(item[0]) == string.lower(name):
                gvutils.error('Field '+name+' already present!')
                return

        
        ftype = self.new_field_types[self.new_field_type_menu.get_history()]


        try:
            fwidth = int(self.new_field_width_entry.get_text())
        except:
            gvutils.error('Field width must be an integer!')
            return

        if ftype == 'float':
            try:
                fprec = int(self.new_field_precision_entry.get_text())
            except:
                gvutils.error('Precision width must be an integer!')
                return
        else:
            fprec = 0

        self.shapes.add_field(name,ftype,fwidth,fprec)
        self.fill_grid()
        if self.shapesgridtool is not None:
            self.shapesgridtool.refresh_columns()
            
    def changed_field(self,*args):
        """ User changed a field """
        cell=args[1]
        if cell[1] == 2:
            # width updated
            idx=str(cell[0]+1)
            self.shapes.set_property('_field_width_'+str(idx),
                      self.grid.get_cell_data_string(cell[0],cell[1]))
            if self.shapesgridtool is not None:
                self.shapesgridtool.refresh_columns()
          
        elif cell[1] == 3:
            # precision updated
            # width updated
            idx=str(cell[0]+1)
            if self.grid.get_cell_data_string(cell[0],1) == 'float':
                self.shapes.set_property('_field_precision_'+str(idx),
                     self.grid.get_cell_data_string(cell[0],cell[1]))
                if self.shapesgridtool is not None:
                    self.shapesgridtool.refresh_columns()
                
            else:
                if self.grid.get_cell_data_string(cell[0],cell[1]) != '0':
                    self.grid.set_cell_data_string(cell[0],cell[1],0)
                    gvutils.error('Precision can only be reset for float.')
                   
        else:
            gvutils.error('Name and type of a field cannot be changed.')
            

    def new_field_precision_cb(self,*args):
        if self.new_field_types[self.new_field_type_menu.get_history()] ==\
               'float':
            # precision is only relevant for float
            self.new_field_precision_entry.set_editable(True)
            self.new_field_precision_entry.set_sensitive(True)
        else:
            self.new_field_precision_entry.set_text('0')
            self.new_field_precision_entry.set_editable(False)
            self.new_field_precision_entry.set_sensitive(False)
            
        

TOOL_LIST = ["ShapesGridTool"]
                   
        
