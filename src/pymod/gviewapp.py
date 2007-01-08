#!/usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  GViewApp and related definitions.
# Author:   Frank Warmerdam, warmerdam@pobox.com
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

from gvsignaler import Signaler
import gtk
from gtk.gdk import *
from gtk.keysyms import *

# TEMP GTK2 PORT
#
#import gtkmissing
import sys
import gview
import layerdlg
#import gvenhdlg
import gdal
import gvutils
import os
import pgufilesel
import gvviewwindow
import gvhtml
import gvselbrowser

import pgufont
default_font = pgufont.XLFDFontSpec()
default_font.set_font_part('Family', 'Arial')
default_font.set_font_part('Point Size', '120')

default_preferences = {
            'legend-background-color': (1.0, 1.0, 1.0, 1.0),
            'legend-label-font': default_font,
            'legend-label-font-color': (0.0, 0.0, 0.0, 1.0),
            'legend-title-font': default_font,
            'legend-title-font-color': (0.0, 0.0, 0.0, 1.0),
            'default-font': default_font,
}

gview.set_default_preferences( default_preferences )

class GViewApp(Signaler):
    def __init__(self,toolfile=None,menufile=None,iconfile=None,pyshellfile=None,notools=0):
        self.view_manager = ViewManager()
        self.sel_manager = gvselbrowser.GvSelectionManager( self.view_manager )
        self.pref_dialog = None
        self.filename = None

        # Toolbar
        self.toolbar = Toolbar()
        self.view_manager.set_toolbar( self.toolbar )

        # Other dialogs, etc.
        self.layerdlg = layerdlg.Launch()
        self.view_manager.set_layerdlg(self.layerdlg)

        self.publish('quit')
        self.publish('rfl-change')

        # Verify that float() works properly.
        try:
            x = float('0.9')
        except:
            gvutils.warning( '''It appears that float() doesn\'t work properly on your system.
                             This is likely due to use of a numeric locale with an alternate decimal
                             representation.  Please try setting the LC_NUMERIC environment variable
                             to C and restarting OpenEV.''' )

        # Default configuration files for view and python shell
        self.menufile=menufile
        self.iconfile=iconfile
        self.pyshellfile=pyshellfile

        # External tools to import and add to view menu
        self.Tool_List = []
        if toolfile is not None:
            self.load_tools_file( toolfile )

        if not notools:
            self.scan_tools_directories()

        # Tool index: a dictionary with the tool name as a
        # key and the tool's position in the list as the value
        self.tool_index = {}
        for idx in range(len(self.Tool_List)):
            self.tool_index[self.Tool_List[idx][0]]=idx

        self.shell = None

    def serialize(self,base=None, filename=None):
        if base is None:
            base = [gdal.CXT_Element, 'GViewApp']

        for vw in self.view_manager.view_list:
            base.append( vw.serialize( filename=filename ) )

        return base

    def clear_project( self ):
        self.view_manager.close_all_views()
        pass

    def load_project(self, filename):
        try:
            raw_xml = open(filename).read()
        except:
            gvutils.error( 'Unable to load '+filename )
            return

        tree = gdal.ParseXMLString( raw_xml )
        if tree is None:
            gvutils.error( 'Problem occured parsing project file '+filename )
            return

        if tree[1] != 'GViewApp':
            gvutils.error( 'Root of %s is not GViewApp node.' % filename )
            return

        self.clear_project()
        self.filename = filename
        self.add_to_rfl( filename )

        for subnode in tree[2:]:
            if subnode[0] == gdal.CXT_Element:
                gvutils.XMLInstantiate( subnode, self, filename=filename )

    def save_project_with_name_cb( self, filename, *args ):
        if os.path.splitext(filename)[1] == '':
            filename = filename + '.opf'

        self.save_project( filename )
        self.add_to_rfl( filename )

    def save_project_as( self ):
        if self.filename is None:
            default_filename = 'default.opf'
        else:
            default_filename = self.filename
        pgufilesel.SimpleFileSelect( self.save_project_with_name_cb,
                                     title = 'Project Filename',
                                     default_filename = default_filename )

    def save_project(self, filename = None):
        if filename is None and self.filename is not None:
            filename = self.filename

        if filename is None:
            self.save_project_as()
            return

        tree = self.serialize( filename=filename )
        open( filename, 'w' ).write( gdal.SerializeXMLTree(tree) )

        self.filename = filename

    def load_tools_file(self, toolfile):
        tool_count = 0
        # read in toolfile, initialize tools
        tool_file = open(toolfile,"r")
        cur_module = None
        cur_tool = None
        for new_line in tool_file.readlines():
            [key,val] = gvutils.read_keyval( new_line )
            if (key == "MODULE_NAME"):
                cur_module = val
            elif (key == "TOOL_NAME"):
                cur_tool = val
                if ((cur_module is not None) and (cur_tool is not None)):
                    self.load_tool( cur_module, cur_tool )
                else:
                    raise AttributeError,"Invalid tool file format"

    def scan_tools_directories(self):
        self.scan_tool_directory( os.path.join(gview.home_dir,'tools') )

    def scan_tool_directory(self, dir_name):
        try:
            files = os.listdir(dir_name)
        except:
            return

        old_path = sys.path
        sys.path.append(dir_name)
        for file in files:
            # print file
            if file[-3:] == '.py':
                print 'Loading tools from '+os.path.join(dir_name,file)
                module = file[:-3]

                try:
                    exec "import " + module
                    exec "tool_list = " + module + ".TOOL_LIST"
                    for item in tool_list:
                        exec "tool_inst=" + module + "." + item + "(app=self)"
                        self.Tool_List.append((item,tool_inst))
                except:
                    import traceback
                    traceback.print_exc()
                    print '... failed to load ... skipping.'
                    gdal.Debug( "GDA", '-'*60 )
                    sys_type, sys_value, sys_traceback = sys.exc_info()
                    exp = traceback.format_exception( sys_type, sys_value, sys_traceback )
                    exception = ""
                    for line in exp:
                        exception = exception + line
                    gdal.Debug( "GDA", exception )
                    gdal.Debug( "GDA", '-'*60 )

        # We only add the tool directory to the python path long enough
        # to load the tool files.
        sys.path = old_path

    def load_tool(self, module_name, tool_name ):
        exec "import " + module_name
        exec "cur_tool_class = " + module_name + "." + tool_name + "(app=self)"
        self.Tool_List.append([tool_name,cur_tool_class])

    def request_quit(self, *args):
        dialog = gtk.Dialog('Confirmation',
                     None,
                     gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                     (gtk.STOCK_YES, gtk.RESPONSE_YES,
                      gtk.STOCK_NO, gtk.RESPONSE_NO))
        label = gtk.Label('Are you sure you want to exit OpenEV?')
        dialog.vbox.pack_start(label, True, True, 0)
        label.show()

        response = dialog.run();

        if response == gtk.RESPONSE_YES:
            self.quit()
            return 1
        else:
            return 0

    def quit(self, *args):
        # Save preferences
        gview.save_preferences()

        # Notify listeners of quit event
        Signaler.notify(self,'quit')

    def add_to_rfl(self, filename):
        # Don't add NUMPY arrays to file list.
        if filename[:7] == 'NUMPY::':
            return

        next_value = filename
        for i in range(1,6):
            rbl_name = 'recent_file_'+str(i)
            rbl_value = gview.get_preference(rbl_name)
            gview.set_preference(rbl_name, next_value)

            if rbl_value is None or rbl_value == filename:
                break;

            next_value = rbl_value

        Signaler.notify(self,'rfl-change')

    def get_rfl(self):
        list = []
        for i in range(1,6):
            rbl_name = 'recent_file_'+str(i)
            rbl_value = gview.get_preference(rbl_name)
            if rbl_value is not None:
                list.append(rbl_value)
        return list

    def show_layerdlg(self, *args):
        self.layerdlg.show()
        self.layerdlg.window.raise_()

    def show_toolbardlg(self, *args):
        self.toolbar.show()
        self.toolbar.window.raise_()

    def show_enhdlg(self, *args):
        self.enhdlg = gvenhdlg.EnchancementDialog()

    def load_menus_file_from_xml(self,menufile,view_name):
        # Scan the XML menu file to find which tools to
        # load, and where to position them.

        import string

        menufile = os.path.join(gview.home_dir,'xmlconfig',menufile)

        # menu_list contains a mix of regular and tool menu entries,
        # in order. 
        menu_list = []
        try:
            raw_xml = open(menufile).read()
        except:
            raise AttributeError,"Unable to load " + menufile
            return

        tree = gdal.ParseXMLString( raw_xml )
        if tree is None:
            raise AttributeError,"Problem occured parsing menu file " + menufile
            return

        if tree[1] != 'GViewAppMenu':
            raise AttributeError,"Root of %s is not GViewAppMenu node " % menufile
            return

        # loop over entries getting path,accelerator,callback and arguments
        menu_trees = gvutils.XMLFind( tree, 'Menu')
        if menu_trees is None:
            raise AttributeError,"Invalid menu file format"

        # Tools can be specified in a number of ways.  The
        # <tools> entry can be 'All', 'None', or 'Some'.
        # In the "All" case, all tools will be loaded up.
        # If toolentries are specified, the defaults
        # will be overidden for those tools.  This is the
        # default.  If "Some" is specified, only the
        # tools entered in the xml file will be included.
        # If None is specified, no tools will be loaded
        # and if toolentries are specified an error will
        # be raised.
        tools_to_include = 'All'

        # If tools to include is All, use this list to check for
        # tools that haven't been added yet, and add them at the
        # end using their defaults.
        tools_accounted_for=[]


        menu_list = []

        for node in menu_trees[2:]:
            if node[1] == 'entry':
                node_path  = gvutils.XMLFind( node, 'path')
                if node_path is None:
                    raise AttributeError,"Invalid menu file format - missing path"

                entry_type = gvutils.XMLFindValue( node_path, 'type', '')
                entry_path = gvutils.XMLFindValue( node, 'path','')

                if (string.find(entry_path,"/") == -1):
                    raise AttributeError,"Invalid menu file format - bad path:%s" % entry_path

                if (entry_type != ''):
                    entry_type = "<" + entry_type + ">"
                path_split=string.split(entry_path,"/")
                path_split[-1] = entry_type + path_split[-1]
                entry_path=string.join(path_split,"/")

                entry_accelerator = gvutils.XMLFindValue( node, 'accelerator', 'None')
                if (entry_accelerator != 'None'):
                    (key,mod) = string.split(entry_accelerator,'+')
                    entry_accelerator = "'<" + key + ">" + mod + "'"

                entry_callback = gvutils.XMLFindValue( node, 'callback', 'None')
                entry= "("                                             \
                        + string.join((entry_path,entry_accelerator,   \
                                       entry_callback),",")

                arguments = gvutils.XMLFind( node, 'arguments')
                if arguments is not None:
                    args_list = []
                    args =  gvutils.XMLFind( arguments, 'arg','')
                    if args is not None:
                        for arg in args:
                            args_list.append(gvutils.XMLFindValue( arg, '',''))
                        entry = entry + "," + string.join(args_list,",")

                entry = entry + ")"

                menu_list.append(entry)

            elif node[1] == 'tools':
                tools_to_include=node[2][1]

            elif node[1] == 'simpletoolentry':
                toolname  = gvutils.XMLFindValue( node, 'name')
                if toolname is None:
                    raise AttributeError,"Invalid menu file format - missing tool name"

                if self.tool_index.has_key(toolname) == 0:
                    raise AttributeError,"Invalid menu file format- tool "+toolname+" not loaded."

                ctool=self.Tool_List[self.tool_index[toolname]][1]
                for cpath in ctool.menu_entries.entries.keys():
                    caccel=ctool.menu_entries.entries[cpath][2]
                    if caccel is None:
                        caccel=str(None)
                    else:
                        caccel="'"+caccel+"'"
                    # main application is stored as self.app in GvViewWindow
                    # ccb (current callback string) specifies the path to the
                    # callback from within the gvviewwindow.
                    ccb="self.app.Tool_List[self.app.tool_index['"+toolname+"']]"+\
                        "[1].menu_entries.entries['"+cpath+"'][1]"

                    # The name of the view that launched the tool is passed to
                    # the callback in case the tool needs to locate the view that
                    # launched it (a view doesn't always become the currently 
                    # active view until its view area is clicked on, so simply
                    # getting the active view is not sufficient).  If the tool
                    # wishes to act on the view that launched it rathern than
                    # the currently active layer, it must locate the view with
                    # this name and activate it before proceeding.
                    viewstr="('"+view_name+"')"

                    entry="("+string.join(("'"+cpath+"'",caccel,ccb,viewstr),",")+")"
                    menu_list.append(entry)

                if toolname not in tools_accounted_for:
                    tools_accounted_for.append(toolname)


            elif node[1] == 'complextoolentry':
                toolname  = gvutils.XMLFindValue( node, 'name')
                if toolname is None:
                    raise AttributeError,"Invalid menu file format - missing tool name"

                oldpath  = gvutils.XMLFindValue( node, 'oldpath')

                if oldpath is None:
                    txt="Invalid menu file format - complex tool entry\nrequires oldpath item."
                    raise AttributeError,txt
                oldpath = oldpath[1:-1] # Entries in XML file are surrounded by quotes- get rid of them

                newpath  = gvutils.XMLFindValue( node, 'newpath')
                if newpath is None:
                    txt="Invalid menu file format - complex tool entry\nrequires newpath item."
                    raise AttributeError,txt
                newpath = newpath[1:-1] # Entries in XML file are surrounded by quotes- get rid of them

                if self.tool_index.has_key(toolname) == 0:
                    raise AttributeError,"Invalid menu file format- tool "+toolname+" not loaded."

                ctool=self.Tool_List[self.tool_index[toolname]][1]
                if ctool.menu_entries.entries.has_key(oldpath) == 0:
                    raise AttributeError,'Invalid menu file entry- tool '+toolname+' has no\nmenu entry '+oldpath

                caccel=gvutils.XMLFindValue(node,'accelerator')
                if caccel is None:
                    caccel=ctool.menu_entries.entries[oldpath][2]
                    if caccel is None:
                        caccel=str(None)
                    else:
                        caccel="'"+caccel+"'"
                else:
                    # XML file specifies key sequence string without
                    # the "<" and ">"'s to avoid confusion with tags
                    # (eg. control+D rather than <control>D).  Add them
                    # back in here, since parser expects them.
                    (key,mod)=string.split(caccel,'+')
                    caccel="'<"+key+">"+mod+"'"

                ccb="self.app.Tool_List[self.app.tool_index['"+toolname+"']]"+\
                     "[1].menu_entries.entries['"+oldpath+"'][1]"

                viewstr="('"+view_name+"')"

                entry="("+string.join(("'"+newpath+"'",caccel,ccb,viewstr),",")+")"
                menu_list.append(entry)

                if toolname not in tools_accounted_for:
                    tools_accounted_for.append(toolname)

        if tools_to_include not in ['All','None','Some']:
            raise AttributeError,"Invalid menu file format- <tool> entry should be All, None, or Some."

        if ((tools_to_include == 'None') and (len(tools_accounted_for) > 0)):
            txt = "Invalid menu file format- if <tool> entry is None,\nno "
            txt = txt+"simpletoolentry or complextoolentry items may be specified."
            raise AttributeError,txt

        if tools_to_include == 'All':
            for citem in self.Tool_List:
                if citem[0] not in tools_accounted_for:
                    ctool=citem[1]
                    for cpath in ctool.menu_entries.entries.keys():
                        # default position: find where to insert
                        # tool.
                        cpos=ctool.menu_entries.entries[cpath][0]                        
                        cpos=max(cpos,0)

                        splitpath=string.split(cpath,'/')
                        rootp=string.join(splitpath[:-1],'/')+'/'
                        nchars=len(rootp)
                        matches=0
                        idx=0
                        for nextentry in menu_list:
                            if (len(nextentry) > nchars):
                                if (nextentry[2:nchars+2] == rootp):
                                    matches=matches+1
                            if matches > cpos:
                                break
                            idx=idx+1

                        caccel=ctool.menu_entries.entries[cpath][2]
                        if caccel is None:
                            caccel=str(None)
                        else:
                            caccel="'"+caccel+"'"
                        ccb="self.app.Tool_List[self.app.tool_index['"+citem[0]+"']]"+\
                             "[1].menu_entries.entries['"+cpath+"'][1]"

                        viewstr="('"+view_name+"')"

                        entry="("+string.join(("'"+cpath+"'",caccel,ccb,viewstr),",")+")"
                        menu_list.insert(idx,entry)

        # Move help entries to end so that Help menu is on the far right
        help_list=[]
        idx=0
        for count in range(len(menu_list)):
            if len(menu_list[idx]) > 7:
                if menu_list[idx][:7] == "('Help/":
                    help_list.append(menu_list.pop(idx))
                else:
                    idx=idx+1
            else:
                idx=idx+1

        menu_list.extend(help_list)

        menu_cmd =  "self.menuf.add_entries([" + string.join(menu_list,',') + "])"
        return menu_cmd


    def load_icons_file_from_xml(self,iconfile):
        # Scan the XML icon file to find which tools to
        # load, and where to position them.

        import string

        iconfile = os.path.join(gview.home_dir,'xmlconfig',iconfile)

        # icon_count: current position
        icon_count=0
        try:
            raw_xml = open(iconfile).read()
        except:
            raise AttributeError,"Unable to load " + iconfile
            return

        tree = gdal.ParseXMLString( raw_xml )
        if tree is None:
            raise AttributeError,"Problem occured parsing icon file " + iconfile
            return

        if tree[1] != 'GViewAppIconBar':
            raise AttributeError,"Root of %s is not GViewAppIconBar node " % iconfile
            return

        # loop over entries getting path,accelerator,callback and arguments
        icon_trees = gvutils.XMLFind( tree, 'Iconbar')
        if icon_trees is None:
            raise AttributeError,"Invalid menu file format"

        # Tools can be specified in a number of ways.  The
        # <tools> entry can be 'All', 'None', or 'Some'.
        # In the "All" case, all tools will be loaded up.
        # If toolentries are specified, the defaults
        # will be overidden for those tools.  This is the
        # default.  If "Some" is specified, only the
        # tools entered in the xml file will be included.
        # If None is specified, no tools will be loaded
        # and if toolentries are specified an error will
        # be raised.
        tools_to_include = 'All'

        # If tools to include is All, use this list to check for
        # tools that haven't been added yet, and add them at the
        # end using their defaults.
        tools_accounted_for=[]

        icon_list = []

        for node in icon_trees[2:]:
            if node[1] == 'icon':
                type = None
                icon_label = gvutils.XMLFindValue( node, 'label','None')
                icon_hint = gvutils.XMLFindValue( node, 'hint','None')
                icon_callback = gvutils.XMLFindValue( node, 'callback','None')
                icon_help = gvutils.XMLFindValue( node, 'help','None')
                icon_file = gvutils.XMLFindValue( node, 'xpm','None')
                # xpm files - need to add path and possible help
                if (icon_file != 'None'):
                    type = 'xpm'
                    icon = "self.add_icon_to_bar("                           \
                            + string.join((icon_file,icon_label,icon_hint,   \
                                           icon_callback,icon_help),",")     \
                            + ")" 

                # pixmap files - not adding path or help 
                icon_file = gvutils.XMLFindValue( node, 'pixmap','None')
                if (icon_file!= 'None'):
                    type = 'pixmap'
                    icon = "self.iconbar.append_item("                        \
                            + string.join((icon_label,icon_hint,icon_hint,    \
                                              icon_file,icon_callback),",")   \
                            + ")" 

                # widget  
                icon_file = gvutils.XMLFindValue( node, 'widget','None')
                if (icon_file!= 'None'):
                    type = 'widget'
                    icon_file = gvutils.XMLFindValue( node, 'widget','None')
                    icon = "self.iconbar.append_widget("                       \
                            + string.join((icon_file,icon_hint,icon_hint),",") \
                            + ")" 
                # none of the above
                if type is None:
                    raise AttributeError,"Invalid icon file format - unknown type"

                icon_list.append(icon)                
            elif node[1] == 'tools':
                tools_to_include=node[2][1]
            elif node[1] == 'simpletoolentry':
                toolname  = gvutils.XMLFindValue( node, 'name')
                if toolname is None:
                    raise AttributeError,"Invalid icon file format - missing tool name"

                if self.tool_index.has_key(toolname) == 0:
                    raise AttributeError,"Invalid icon file format- tool "+toolname+" not loaded."

                ctool=self.Tool_List[self.tool_index[toolname]][1]

                idx=0
                for centry in ctool.icon_entries.entries:
                    icon_file=centry[0]

                    icon_label=centry[1]
                    if icon_label is not None:
                        icon_label="'"+icon_label+"'"
                    else:
                        icon_label=str(None)

                    icon_hint=centry[2]
                    if icon_hint is not None:
                        icon_hint="'"+icon_hint+"'"
                    else:
                        icon_hint=str(None)

                    # Ignore position- it is overridden by this entry's location in the
                    # xml file
                    icon_callback=centry[4]
                    icon_help=centry[5]
                    if icon_help is not None:
                        icon_help="'"+icon_help+"'"
                    else:
                        icon_help=str(None)

                    icon_type=centry[6]
                    if icon_type == 'xpm':
                        icon = "self.add_icon_to_bar("                           \
                                + string.join(("'"+icon_file+"'",icon_label,icon_hint,   \
                                "self.app.Tool_List[self.app.tool_index['"+\
                                toolname+"']][1].icon_entries.entries["+\
                                str(idx)+"][4]",icon_help),",")+")"     

                        icon_list.append(icon)
                    else:
                        raise AttributeError,"Invalid icon type "+icon_type+" in tool "+toolname+"."
                    idx=idx+1

                if toolname not in tools_accounted_for:
                    tools_accounted_for.append(toolname)

            elif node[1] == 'complextoolentry':
                toolname  = gvutils.XMLFindValue( node, 'name')
                if toolname is None:
                    raise AttributeError,"Invalid icon file format - missing tool name."

                oindex  = gvutils.XMLFindValue( node, 'index')

                if oindex is None:
                    txt="Invalid icon file format - complex tool entry\nrequires the index of the icon entry\n"
                    txt=txt+"to replace (0...number of entries-1).\n"
                    raise AttributeError,txt
                try:
                    oindex=int(oindex)
                except:
                    raise AttributeError,"Invalid icon file- icon index to replace must be an integer."

                if self.tool_index.has_key(toolname) == 0:
                    raise AttributeError,"Invalid icon file entry- tool "+toolname+" not loaded."

                ctool=self.Tool_List[self.tool_index[toolname]][1]

                if len(ctool.icon_entries.entries) < (oindex+1):
                    txt='Invalid file file entry- for tool '+toolname+'.\n maximum entry index is '
                    txt=txt+str(len(ctool.icon_entries.entries)-1)+'.' 

                icon_file=gvutils.XMLFindValue( node, 'xpm')
                icon_hint=gvutils.XMLFindValue( node, 'hint')
                icon_label=gvutils.XMLFindValue( node, 'label')
                icon_help=gvutils.XMLFindValue( node, 'help')

                if icon_file is None:
                    icon_file="'"+ctool.icon_entries.entries[oindex][0]+"'"
                elif os.path.isfile(icon_file):
                    if os.name == 'nt':
                        icon_file="'"+string.replace(icon_file,"\\","\\\\")+"'"
                    else:
                        icon_file="'"+icon_file+"'"
                elif os.path.isfile(os.path.join(gview.home_dir,'tools',icon_file)):
                    icon_file="'"+os.path.join(gview.home_dir,'tools',icon_file)+"'"
                    if os.name == 'nt':
                        icon_file=string.replace(icon_file,"\\","\\\\")
                elif os.path.isfile(os.path.join(gview.home_dir,'pics',icon_file)):
                    icon_file="'"+os.path.join(gview.home_dir,'pics',icon_file)+"'"
                    if os.name == 'nt':
                        icon_file=string.replace(icon_file,"\\","\\\\") 
                else:
                    txt = "Cannot find file "+tempf+'.  Either the full\n'
                    txt = txt+"path must be specified, or "+tempf+ " must be\n"
                    txt = txt+"placed in the tools or pics directory."
                    raise AttributeError,txt


                if icon_label is None:
                    icon_label=ctool.icon_entries.entries[oindex][1]

                if icon_label is not None:
                    icon_label="'"+icon_label+"'" 
                else:
                    icon_label=str(None)

                if icon_hint is None:
                    icon_hint=ctool.icon_entries.entries[oindex][2]

                if icon_hint is not None:
                    icon_hint="'"+icon_hint+"'"
                else:
                    icon_hint=str(None)

                if icon_help is None:
                    icon_help=ctool.icon_entries.entries[oindex][5]

                if icon_help is not None:
                    icon_help="'"+icon_help+"'"
                else:
                    icon_help=str(None)

                icon_callback=ctool.icon_entries.entries[oindex][4]
                icon_type=ctool.icon_entries.entries[oindex][6]
                if icon_type == 'xpm':
                    icon = "self.add_icon_to_bar("                           \
                            + string.join((icon_file,icon_label,icon_hint,   \
                         "self.app.Tool_List[self.app.tool_index['"+toolname+\
                         "']][1].icon_entries.entries["+str(oindex)+"][4]",\
                                           icon_help),",") + ")"
                    icon_list.append(icon)
                else:
                    raise AttributeError,"Invalid icon type "+icon_type+" in tool "+toolname+"."

                if toolname not in tools_accounted_for:
                    tools_accounted_for.append(toolname)


        if tools_to_include not in ['All','None','Some']:
            raise AttributeError,"Invalid icon file format- <tool> entry should be All, None, or Some."

        if ((tools_to_include == 'None') and (len(tools_accounted_for) > 0)):
            txt = "Invalid icon file format- if <tool> entry is None,\nno "
            txt = txt+"simpletoolentry or complextoolentry items may be specified."
            raise AttributeError,txt

        if tools_to_include == 'All':
            for citem in self.Tool_List:
                if citem[0] not in tools_accounted_for:
                    ctool=citem[1]
                    idx=0
                    for centry in ctool.icon_entries.entries:
                        if centry[6] != 'xpm':            
                            raise AttributeError,"Error loading tool entry for tool "+\
                                  citem[0]+"- icon type "+centry[6]+" invalid."

                        icon_file="'"+centry[0]+"'"
                        icon_label=centry[1]
                        if icon_label is not None:
                            icon_label="'"+icon_label+"'"
                        else:
                            icon_label=str(None)

                        icon_hint=centry[2]    
                        if icon_hint is not None:
                            icon_hint="'"+icon_hint+"'"
                        else:
                            icon_hint=str(None)

                        icon_help=centry[5]
                        if icon_help is not None:
                            icon_help="'"+icon_help+"'"
                        else:
                            icon_help=str(None)

                        # Default position in icon bar used
                        pos=centry[3]
                        icon = "self.add_icon_to_bar(" +\
                                string.join((icon_file,icon_label,icon_hint,   \
                                "self.app.Tool_List[self.app.tool_index['"+\
                                citem[0]+"']][1].icon_entries.entries["+\
                                str(idx)+"][4]",icon_help),",") + ")"

                        pos=max(pos,0)
                        if pos > len(icon_list):
                            icon_list.append(icon)
                        else:
                            icon_list.insert(pos,icon)
                        idx=idx+1    

        return icon_list

    def new_view(self, title=None, menufile=None,iconfile=None, *args):
        # If menu/icon files aren't specified, use application-wide
        # defaults
        if ((menufile is None) and (self.menufile is not None)):
            menufile=self.menufile
        if ((iconfile is None) and (self.iconfile is not None)):
            iconfile=self.iconfile

        view_window = gvviewwindow.GvViewWindow(app=self, title=title, menufile=menufile, iconfile=iconfile)
        view_name=view_window.title
        view_menu = view_window.menuf    

        if ((len(self.Tool_List) > 0) and (menufile is None)):
            # If no menu configuration file is specified, put
            # tools in the default positions specified by
            # the tool menu entry's position parameter.
            for cur_tool_list in self.Tool_List:
                cur_tool = cur_tool_list[1]
                if hasattr(cur_tool.menu_entries.entries,'keys'):
                    for item in cur_tool.menu_entries.entries.keys():
                        view_menu.insert_entry(
                            cur_tool.menu_entries.entries[item][0],
                            item,
                            cur_tool.menu_entries.entries[item][2],
                            cur_tool.menu_entries.entries[item][1],
                            (view_name))

        # Icons- Note: currently it is assumed that the tool icons are
        #        xpms.  Support for pixmaps and widgets will be added
        #        later if necessary (icon type would have to be detected
        #        from last entry of the relevant tool icon entry, and
        #        a function would have to be created to deal with them.
        #        They are slightly more complicated than the xpm case
        #        and wouldn't use insert_tool_icon.  Would also need
        #        code in the complextoolentry case to avoid an icon
        #        of one type (eg. xpm) being replaced by another type
        #        (eg. widget, pixmap).
        #
        #        ALSO: GtkToolbar does not allow callback information
        #              to be specified, so the viewname cannot be
        #              passed as an argument to tool icon callbacks
        #              the way it is for menu callbacks. However,
        #              if the view is needed, it can be obtained
        #              using:
        #              view=args[0].get_toplevel()
        #              The view title is under:
        #              view['title'].  Note that this is not quite
        #              the same as the view's self.title, but is based
        #              on it (usually 'OpenEV: '+self.title)

        if ((len(self.Tool_List) > 0) and (iconfile is None)):
            for cur_tool_list in self.Tool_List:
                cur_tool=cur_tool_list[1]
                for item in cur_tool.icon_entries.entries:
                    view_window.insert_tool_icon(
                        item[0], # filename
                        item[1], # label
                        item[2], # hint text
                        item[4], # callback
                        item[5], # help topic
                        item[3],  # position
                            )
        view_window.show()
        return view_window

    def open_gdal_dataset(self, dataset, lut=None, sds_check=1, *args):
        view = self.view_manager.get_active_view_window()
        if view is None:
            self.new_view()
            view = self.view_manager.get_active_view_window()

        if view is None:
            return

        view.open_gdal_dataset( dataset, lut = lut, sds_check = sds_check )

    def file_open_by_name(self, filename, lut=None, sds_check=1, *args):
        view = self.view_manager.get_active_view_window()
        if view is None:
            self.new_view()
            view = self.view_manager.get_active_view_window()

        if view is None:
            return

        view.file_open_by_name( filename, lut = lut, sds_check = sds_check )

    def launch_preferences(self, *args):
        if self.pref_dialog is None:
            self.pref_dialog = PrefDialog()
            self.pref_dialog.connect('destroy', self.destroy_preferences)
        self.pref_dialog.show()
        self.pref_dialog.window.raise_()

    def destroy_preferences(self,*args):
        self.pref_dialog = None

    def pyshell(self, *args):
        import pyshell
        pyshell.launch(pyshellfile=self.pyshellfile)

    def do_auto_imports(self):
        i = 1
        al = gview.get_preference('auto_load_'+str(i))
        while al is not None:
            try:
                exec 'import '+al
            except:
                print 'auto_load_'+str(i)+' error: import '+al
                print sys.exc_info()[0], sys.exc_info()[1]

            i = i + 1
            al = gview.get_preference('auto_load_'+str(i))

    def active_layer(self):
        return self.view_manager.active_view.viewarea.active_layer()

class Toolbar(gtk.Window):
    def __init__(self):
        gtk.Window.__init__(self)
        self.set_size_request(-1,425)
        self.set_title("Tools")

        gvhtml.set_help_topic(self, "edittools.html")

        toolbox = gview.GvToolbox()        
        toolbox.add_tool('select', gview.GvSelectionTool())
        toolbox.add_tool('zoompan', gview.GvZoompanTool())
        toolbox.add_tool('line', gview.GvLineTool())
        toolbox.add_tool('rect', gview.GvRectTool())
        toolbox.add_tool('rotate', gview.GvRotateTool())
        toolbox.add_tool('area', gview.GvAreaTool())
        toolbox.add_tool('node', gview.GvNodeTool())
        toolbox.add_tool('point', gview.GvPointTool())
        toolbox.add_tool('pquery', gview.GvPointTool())
        self.roi_tool = gview.GvRoiTool()
        toolbox.add_tool('roi', self.roi_tool)
        self.poi_tool = gview.GvPoiTool()
        toolbox.add_tool('poi', self.poi_tool)

        self.UImgr = gtk.UIManager()
        self.add_accel_group(self.UImgr.get_accel_group())
        self.actiongroup = gtk.ActionGroup('EditToolbar')

        # Add radio button tools
        self.actiongroup.add_radio_actions([
            ('zoompan', None, "Zoom", None, "Zoom/Pan mode"),
            ('point', None, "Point Edit", None, "Point editing tool"),
            ('pquery', None, "Point Query", None, "Point query tool"),
            ('line', None, "Draw Line", None, "Line drawing tool"),
            ('rotate', None, "Rotate/Resize", None, "Rotate/resize symbol tool"),
            ('rect', None, "Draw Rectangle", None, "Rectangle drawing tool"),
            ('area', None, "Draw Area", None, "Area drawing tool"),
            ('node', None, "Edit Node", None, "Node edit tool"),
            ('labels', None, "Draw Labels", None, "Label drawing tool"),
            ('roi', None, "Draw ROI", None, "ROI drawing tool"),
            ('poi', None, "Choose POI", None, "POI selection tool"),
            ('select', None, "Select", None, "Selection tool"),
            ], on_change=self.toggle)

        # Add toggle tools
        self.actiongroup.add_toggle_actions([
            ('LinkViews', None, "Link views", None, "Link views together", self.link),
            ('Cursor', None, "Cursor", None, "Create cursor in all views", self.cursor),
            ])

        # Add the actiongroup to the uimanager
        self.UImgr.insert_action_group(self.actiongroup, 0)
        self.UImgr.add_ui_from_string(
        """
        <toolbar name='EditToolbar'>
          <toolitem action='select'/>
          <toolitem action='zoompan'/>
          <toolitem action='point'/>
          <toolitem action='pquery'/>
          <toolitem action='line'/>
          <toolitem action='rotate'/>
          <toolitem action='rect'/>
          <toolitem action='area'/>
          <toolitem action='node'/>
          <toolitem action='labels'/>
          <toolitem action='roi'/>
          <toolitem action='poi'/>
          <separator/>
          <toolitem action='LinkViews'/>
          <toolitem action='Cursor'/>
        </toolbar>
        """)

        toolbar = self.UImgr.get_widget('/EditToolbar')
        toolbar.set_style(gtk.TOOLBAR_TEXT)
        toolbar.set_orientation(gtk.ORIENTATION_VERTICAL)
        self.add(toolbar)

        self.select_button = self.actiongroup.get_action('select')
        self.roi_button = self.actiongroup.get_action('roi')
        self.poi_button = self.actiongroup.get_action('poi')
        self.cursor_button = self.actiongroup.get_action('Cursor')

        toolbox.activate_tool("select")
        toolbar.show_all()
        self.toolbox = toolbox
        self.toolbar = toolbar
        self.link = gview.GvViewLink()
        self.connect('delete-event', self.close)

    def close(self, *args):
        self.hide()
        return True

    def toggle(self, action, current):
        data = current.get_name()

        # For Point Query Tool:
        # Make the special point query layer the current layer and if there
        # isn't one, create it. 
        if data == "pquery":
            view = self.toolbox.get_view()
            if view is not None:
                layer_list = view.list_layers()
                result_layer = None
                for layer in layer_list:
                    if layer.get_property('pquery') is not None:
                        result_layer = layer

                if result_layer is None:
                    result_layer = gview.GvPqueryLayer()
                    gview.undo_register( result_layer.get_parent() )
                    result_layer.set_property('pquery','true')
                    view.add_layer(result_layer)

                view.set_active_layer( result_layer )

        if data == 'labels':
            import gvlabeledit
            gvlabeledit.launch()
            data = 'select'

        self.toolbox.activate_tool(data)

    def link(self, but):
        if (but.get_active()):
            self.link.enable()
        else:
            self.link.disable()

    def cursor(self, but):
        if (but.get_active()):
            if ((gview.get_preference('cursor_type') is not None) and
            (gview.get_preference('cursor_type') == 'nonadaptive')):
                self.link.set_cursor_mode(1)
            else:
                self.link.set_cursor_mode(2)     
        else:
            self.link.set_cursor_mode(0)

    def add_view(self, view):
        self.toolbox.activate(view)
        self.link.register_view(view)

    def get_roi(self):
        return self.roi_tool.get_rect()

    def get_poi(self):
        return self.poi_tool.get_point()

class ViewManager(Signaler):

    def __init__(self):
        self.layerdlg = None
        self.toolbar = None
        self.active_view = None
        self.view_list = []
        self.publish( 'active-view-changed' )
        self.updating = False

    def set_layerdlg(self,layerdlg):
        self.layerdlg = layerdlg
        self.layerdlg.subscribe('active-view-changed',self.layerdlg_cb)

    def layerdlg_cb(self,*args):
        self.set_active_view( self.layerdlg.get_active_view() )

    def set_toolbar(self,toolbar):
        self.toolbar = toolbar
        self.toolbar.toolbox.connect('activate',self.toolbar_cb)

    def toolbar_cb(self,*args):
        self.set_active_view( self.toolbar.toolbox.get_view() )

    def add_view(self, new_view ):
        self.updating = True
        self.view_list.append( new_view )
        if self.toolbar is not None:
            self.toolbar.add_view(new_view.viewarea)
        if self.layerdlg is not None:
            self.layerdlg.add_view(new_view.title, new_view.viewarea)
        new_view.connect('destroy', self.view_closing_cb)
        self.updating = False
        self.set_active_view( new_view )

    def view_closing_cb( self, view_window_in, *args ):
        # lookup original ViewWindow instance with internal variables.
        view_window = None
        for v in self.view_list:
            if v == view_window_in:
                view_window = v

        if view_window is None:
            gdal.Debug( "OpenEV",
                        "unexpectedly missing view in ViewManager" )
            return

        self.view_list.remove( view_window )

        if view_window_in == self.active_view:
            if len(self.view_list) > 0:
                self.set_active_view( self.view_list[0] )
            else:
                self.set_active_view( None );

        if self.layerdlg is not None:
            self.layerdlg.remove_view( view_window.title )

        if self.toolbar is not None:
            self.toolbar.toolbox.deactivate( view_window.viewarea )



    def close_all_views( self, *args ):
        old_len = len(self.view_list)+1
        while len(self.view_list) < old_len and old_len > 1:
            old_len = len(self.view_list)
            # If views have menus, make sure the main app
            # doesn't keep trying to update their rfls after 
            # they're gone...
            if self.view_list[0].menuf is not None:
                try:
                    self.view_list[0].app.unsubscribe('rfl-change',
                         self.view_list[0].show_rfl)
                except:
                    pass

            self.view_list[0].destroy()

        if len(self.view_list) > 0:
            print 'failed to destroy all views.'


    def get_views(self):
        return self.view_list

    def get_active_view(self):
        if self.active_view == None:
            return None
        else:
            return self.active_view.viewarea

    def get_active_view_window(self):
        return self.active_view

    def set_active_view(self, new_view):
        if self.updating:
            return

        if new_view == self.active_view:
            return

        if self.active_view is not None \
           and new_view == self.active_view.viewarea:
            return

        for v in self.view_list:
            if v.viewarea == new_view:
                new_view = v

        self.active_view = new_view
        if new_view is not None and new_view.window is not None:
            new_view.window.raise_()

        Signaler.notify(self,'active-view-changed')

        if self.layerdlg is not None and new_view is not None:
            self.layerdlg.view_selected( None, new_view.title )

        if self.toolbar is not None and new_view is not None:
            self.toolbar.toolbox.activate(new_view.viewarea)

class PrefDialog(gtk.Window):
    def __init__(self):
        gtk.Window.__init__(self)
        self.set_title('Preferences')

        gvhtml.set_help_topic( self, "preferences.html" );

        self.default_color = (0.5, 1.0, 0.5, 1.0)
        self.default_font = 'Sans 12'

        self.tips = gtk.Tooltips()


        self.set_border_width(3)
        self.notebook = gtk.Notebook()
        self.add( self.notebook )

        self.create_tracking_tool_prefs()
        self.create_raster_prefs()
        self.create_cache_prefs()
        self.create_paths_and_windows_prefs()
        #self.create_temporaryfile_prefs()

        self.notebook.append_page(self.page_legend(), 
                                  gtk.Label( 'Legend' ))

        self.show_all()

    def gvplot_cb(self, *args):
        fname = self.gvplot_tempfile_entry.get_text()
        gview.set_preference('gvplot_tempfile',fname)

    def create_cache_prefs(self):
        self.cachep = gtk.VBox(spacing=10)
        self.cachep.set_border_width(10)
        self.notebook.append_page(self.cachep, gtk.Label('Caching'))
        table = gtk.Table(rows=2, columns=2)
        table.set_border_width(5)
        table.set_row_spacings(5)
        table.set_col_spacings(5)
        self.cachep.pack_start(table, expand=False)

        # File Cache
        gdal_cache_label = gtk.Label('File Cache (bytes):')
        gdal_cache_label.set_alignment(0, 0.5)
        table.attach(gdal_cache_label, 0, 1, 0, 1)

        self.gdal_cache = gtk.Entry()
        self.gdal_cache.set_max_length(9)
        self.gdal_cache.connect('activate',self.gdal_cb)
        self.gdal_cache.connect('leave-notify-event',self.gdal_cb)
        table.attach(self.gdal_cache, 1, 2, 0, 1)

        self.gdal_cache.set_text(str(gdal.GetCacheMax() \
                                     +gview.raster_cache_get_max()))

        # Texture Cache
        texture_cache_label = gtk.Label('GL Texture (bytes):')
        texture_cache_label.set_alignment(0, 0.5)
        table.attach(texture_cache_label, 0, 1, 1, 2)

        self.texture_cache = gtk.Entry()
        self.texture_cache.set_max_length(9)
        self.texture_cache.connect('activate',self.tcache_cb)
        self.texture_cache.connect('leave-notify-event',self.tcache_cb)
        table.attach(self.texture_cache, 1, 2, 1, 2)

        self.texture_cache.set_text(str(gview.texture_cache_get_max()))

    def create_raster_prefs(self):

        self.rpp = gtk.VBox(spacing=10)
        self.rpp.set_border_width(10)
        self.notebook.append_page( self.rpp, gtk.Label('Raster'))
        table = gtk.Table(rows=6, columns=2)
        table.set_border_width(5)
        table.set_row_spacings(5)
        table.set_col_spacings(5)
        self.rpp.pack_start(table, expand=False)

        # Warp with GCPs
        gcp_warp_label = gtk.Label('Display Georeferenced:')
        gcp_warp_label.set_alignment(0, 0.5)
        table.attach(gcp_warp_label, 0, 1, 0, 1)

        self.gcp_warp_om = \
               gvutils.GvOptionMenu(('Yes','No'), self.set_gcp_warp_mode)
        table.attach(self.gcp_warp_om, 1, 2, 0, 1)

        if gview.get_preference('gcp_warp_mode') is not None \
           and gview.get_preference('gcp_warp_mode') == 'no':
            self.gcp_warp_om.set_history(1)

        # Sample Method
        sm_label = gtk.Label('Overview Sampling:')
        sm_label.set_alignment(0, 0.5)
        table.attach(sm_label, 0, 1, 1, 2)

        self.sm_om = \
               gvutils.GvOptionMenu(('Decimate','Average'),
                                    self.set_sample_method)
        table.attach(self.sm_om, 1, 2, 1, 2)

        if gview.get_preference('default_raster_sample') is not None \
           and gview.get_preference('default_raster_sample') == 'average':
            self.sm_om.set_history(1)

        # Pixel Interpolation
        im_label = gtk.Label('Subpixel Interpolation:')
        im_label.set_alignment(0, 0.5)
        table.attach(im_label, 0, 1, 2, 3)

        self.im_om = \
               gvutils.GvOptionMenu(('Bilinear','Off (Nearest)'),
                                    self.set_interp_method)
        table.attach(self.im_om, 1, 2, 2, 3)

        if gview.get_preference('interp_mode') is not None \
           and gview.get_preference('interp_mode') == 'nearest':
            self.im_om.set_history(1)

        # Default Autoscaling Method
        scale_label = gtk.Label('Autoscaling Method:')
        scale_label.set_alignment(0, 0.5)
        table.attach(scale_label, 0, 1, 3, 4)

        self.scale_om = \
               gvutils.GvOptionMenu(('Percent Tail Trim',
                                     'Standard Deviations'),
                                    self.set_scaling_method)
        table.attach(self.scale_om, 1, 2, 3, 4)

        if gview.get_preference('scale_algorithm') is not None \
           and gview.get_preference('scale_algorithm') == 'std_deviation':
            self.scale_om.set_history(1)

        # Tail Trim Percentage.
        tt_label = gtk.Label('Tail Trim Percentage:')
        tt_label.set_alignment(0, 0.5)
        table.attach(tt_label, 0, 1, 4, 5)

        self.tt_entry = gtk.Entry()
        self.tt_entry.set_max_length(9)
        self.tt_entry.connect('activate',self.tail_trim_cb)
        self.tt_entry.connect('leave-notify-event',self.tail_trim_cb)
        table.attach(self.tt_entry, 1, 2, 4, 5)

        tt_val = gview.get_preference('scale_percent_tail')

        if tt_val is None:
            tt_val = '0.02'

        self.tt_entry.set_text(str(float(tt_val)*100.0))

        # Scaling Standard Deviations.
        sd_label = gtk.Label('Standard Deviations:')
        sd_label.set_alignment(0, 0.5)
        table.attach(sd_label, 0, 1, 5, 6)

        self.sd_entry = gtk.Entry()
        self.sd_entry.set_max_length(9)
        self.sd_entry.connect('activate',self.std_dev_cb)
        self.sd_entry.connect('leave-notify-event',self.std_dev_cb)
        table.attach(self.sd_entry, 1, 2, 5, 6)

        sd_val = gview.get_preference('scale_std_deviations')

        try:
            if sd_val is None or float(sd_val) == 0.0:
                sd_val = '2.5'
        except:
            sd_val = '2.5'

        self.sd_entry.set_text(sd_val)

    def create_paths_and_windows_prefs(self):
        self.pwp = gtk.VBox(spacing=10)
        self.pwp.set_border_width(10)
        self.notebook.append_page(self.pwp, gtk.Label('Program Paths'))
        table = gtk.Table(rows=3, columns=2)
        table.set_border_width(5)
        table.set_row_spacings(5)
        table.set_col_spacings(5)
        self.pwp.pack_start(table, expand=False)

        # HTML Browser
        html_command_label = gtk.Label('Browser Command:')
        html_command_label.set_alignment(0, 0.5)
        table.attach(html_command_label, 0, 1, 0, 1)

        self.html_command = gtk.Entry()
        self.html_command.connect('activate',self.html_cb)
        self.html_command.connect('leave-notify-event',self.html_cb)
        table.attach(self.html_command, 1, 2, 0, 1)

        if gvhtml.GetBrowseCommand() is not None:
            self.html_command.set_text(gvhtml.GetBrowseCommand())
        else:
            self.html_command.set_text('')

        # Temporary paths
        gvplot_tempfile_label = gtk.Label('Plot file (full path):')
        gvplot_tempfile_label.set_alignment(0, 0.5)
        table.attach(gvplot_tempfile_label, 0, 1, 1, 2)

        self.gvplot_tempfile_entry = gtk.Entry()
        self.gvplot_tempfile_entry.set_max_length(100)
        self.gvplot_tempfile_entry.connect('activate',self.gvplot_cb)
        self.gvplot_tempfile_entry.connect('leave-notify-event',self.gvplot_cb)
        table.attach(self.gvplot_tempfile_entry, 1, 2, 1, 2)

        if gview.get_preference('gvplot_tempfile') is not None:
            gvtext = str(gview.get_preference('gvplot_tempfile'))
            self.gvplot_tempfile_entry.set_text(gvtext) 
        else:
            self.gvplot_tempfile_entry.set_text('')

        # Save last visited directory
        save_recent_dir_label = gtk.Label('Save last visited directory:')
        save_recent_dir_label.set_alignment(0, 0.5)
        table.attach(save_recent_dir_label, 0, 1, 2, 3)

        self.save_recent_dir_om = \
               gvutils.GvOptionMenu(('Off','On'), self.set_save_recent_dir)
        table.attach(self.save_recent_dir_om, 1, 2, 2, 3)

        if gview.get_preference('save_recent_directory') is not None \
           and gview.get_preference('save_recent_directory') == 'off':
            self.save_recent_dir_om.set_history(1)
        else:
            self.save_recent_dir_om.set_history(0)

    def html_cb(self, *args):
        command = self.html_command.get_text()
        if len(command) > 0 and command[len(command)-1] != ' ':
            command = command + ' '
        gvhtml.SetBrowseCommand( command )

    def gdal_cb(self, *args):
        value = int(self.gdal_cache.get_text())
        if value < 2000000:
            self.gdal_cache.set_text(str(gdal.GetCacheMax()
                                         +gview.raster_cache_get_max()))
            return 

        if value == gdal.GetCacheMax() + gview.raster_cache_get_max():
            return

        gdal_cache = int( 900000 + (value - 900000) * 0.25)
        gvraster_cache = value - gdal_cache

        gview.set_preference( 'gdal_cache', str(gdal_cache) )
        gdal.SetCacheMax( gdal_cache )

        gview.set_preference( 'gvraster_cache', str(gvraster_cache) )
        gview.raster_cache_set_max(gvraster_cache)

    def tcache_cb(self, *args):
        value = int(self.texture_cache.get_text())
        if value > 4000000:
            gview.set_preference( 'texture_cache', str(value) )
            gview.texture_cache_set_max(value)
        else:
            self.texture_cache.set_text(str(gview.texture_cache_get_max()))

    def create_tracking_tool_prefs(self):
        self.ttp = gtk.VBox(spacing=10)
        self.ttp.set_border_width(10)
        self.notebook.append_page( self.ttp, gtk.Label('Tracking Tool'))
        table = gtk.Table(rows=4, columns=2)
        table.set_border_width(5)
        table.set_row_spacings(5)
        table.set_col_spacings(5)
        self.ttp.pack_start(table, expand=False)

        # Coordinate
        coord_label = gtk.Label('Coordinate:')
        coord_label.set_alignment(0, 0.5)
        table.attach(coord_label, 0, 1, 0, 1)

        self.coord_om = gvutils.GvOptionMenu(
            ('Off','Raster Pixel/Line','Georeferenced','Geodetic (lat/long)'),
            self.set_coordinate_mode)
        table.attach(self.coord_om, 1, 2, 0, 1)

        if gview.get_preference('_coordinate_mode') is not None:
            if gview.get_preference('_coordinate_mode') == 'raster':
                self.coord_om.set_history(1)
            elif gview.get_preference('_coordinate_mode') == 'georef':
                self.coord_om.set_history(2)
            elif gview.get_preference('_coordinate_mode') == 'latlong':
                self.coord_om.set_history(3)
            else:
                self.coord_om.set_history(0)
        else:
                self.coord_om.set_history(2)

        # Lat/Long Display format (dms or decimal)
        degree_mode_label = gtk.Label('Lat/Long Format:')
        degree_mode_label.set_alignment(0, 0.5)
        table.attach(degree_mode_label, 0, 1, 1, 2)

        self.degree_mode_om = gvutils.GvOptionMenu(('ddd:mm:ss.ss', \
                                                    'ddd.ddddddd'), self.set_degree_mode)
        table.attach(self.degree_mode_om, 1, 2, 1, 2)

        if gview.get_preference('_degree_mode') is not None \
           and gview.get_preference('_degree_mode') == 'decimal':
            self.degree_mode_om.set_history(1)
        else:
            self.degree_mode_om.set_history(0)

        # Raster Value
        pixel_mode_label = gtk.Label('Pixel Value:')
        pixel_mode_label.set_alignment(0, 0.5)
        table.attach(pixel_mode_label, 0, 1, 2, 3)

        self.pixel_mode_om = \
            gvutils.GvOptionMenu(('On','Off'), self.set_pixel_mode)
        table.attach(self.pixel_mode_om, 1, 2, 2, 3)

        if gview.get_preference('_pixel_mode') is not None \
           and gview.get_preference('_pixel_mode') == 'off':
            self.pixel_mode_om.set_history(1)
        else:
            self.pixel_mode_om.set_history(0)

        # NODATA mark
        nodata_mode_label = gtk.Label('Show NODATA mark:')
        nodata_mode_label.set_alignment(0, 0.5)
        table.attach(nodata_mode_label, 0, 1, 3, 4)

        self.nodata_mode_om = \
            gvutils.GvOptionMenu(('On','Off'), self.set_nodata_mode)
        table.attach(self.nodata_mode_om, 1, 2, 3, 4)

        if gview.get_preference('_nodata_mode') is not None \
           and gview.get_preference('_nodata_mode') == 'off':
            self.nodata_mode_om.set_history(1)
        else:
            self.nodata_mode_om.set_history(0)

    def set_coordinate_mode(self, om):
        if self.coord_om.get_history() == 0:
            gview.set_preference( '_coordinate_mode', 'off')
        elif  self.coord_om.get_history() == 1:
            gview.set_preference( '_coordinate_mode', 'raster')
        elif  self.coord_om.get_history() == 2:
            gview.set_preference( '_coordinate_mode', 'georef')
        elif  self.coord_om.get_history() == 3:
            gview.set_preference( '_coordinate_mode', 'latlong')

    def set_pixel_mode(self, om):
        if om.get_history() == 1:
            gview.set_preference( '_pixel_mode', 'off')
        else:
            gview.set_preference( '_pixel_mode', 'on')

    def set_nodata_mode(self, om):
        if om.get_history() == 1:
            gview.set_preference( '_nodata_mode', 'off')
        else:
            gview.set_preference( '_nodata_mode', 'on')

    def set_degree_mode(self, om):
        if om.get_history() == 1:
            gview.set_preference( '_degree_mode', 'decimal')
        else:
            gview.set_preference( '_degree_mode', 'dms')

    def set_gcp_warp_mode(self, om):
        if om.get_history() == 1:
            gview.set_preference( 'gcp_warp_mode', 'no' )
        else:
            gview.set_preference( 'gcp_warp_mode', 'yes' )

    def set_sample_method(self, om):
        if om.get_history() == 0:
            gview.set_preference( 'default_raster_sample', 'sample' )
        else:
            gview.set_preference( 'default_raster_sample', 'average' )

    def set_interp_method(self, im):
        if im.get_history() == 0:
            gview.set_preference( 'interp_mode', 'linear' )
        else:
            gview.set_preference( 'interp_mode', 'nearest' )

    def set_scaling_method(self, om):
        if om.get_history() == 0:
            gview.set_preference( 'scale_algorithm', 'percent_tail_trim' )
        else:
            gview.set_preference( 'scale_algorithm', 'std_deviation' )

    def tail_trim_cb(self,*args):
        try:
            gview.set_preference( 'scale_percent_tail',
                                  str(float(self.tt_entry.get_text())/100.0) )
        except:
            pass

    def std_dev_cb(self,*args):
        try:
            if float(self.sd_entry.get_text()) > 0.0:
                gview.set_preference( 'scale_std_deviations',
                                      self.sd_entry.get_text())
        except:
            pass

    def set_save_recent_dir(self, om):
        if om.get_history() == 0:
            gview.set_preference( 'save_recent_directory', 'off')
        else:
            gview.set_preference( 'save_recent_directory', 'on')

    def page_legend(self):
        """
        properties for the legend dialog
        """
        import pgucolor

        vbox = gtk.VBox()
        table = gtk.Table(rows=1, columns=3)
        table.set_border_width(6)
        table.set_row_spacings(6)
        table.set_col_spacings(6)
        vbox.pack_start(table)

        # Background color
        lbl = gtk.Label('Legend Background Color:')
        table.attach(lbl, 0, 1, 0, 1,
                        xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)

        color = gview.get_preference('legend-background-color', 
                                     self.default_color)
        cb = pgucolor.ColorButton(color)
        cb.connect('color-set', self.set_color_preference,
                        'legend-background-color')
        table.attach(cb, 1, 2, 0, 1,
                        xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)
        self.tips.set_tip(cb,
                'Click to change the default color for the legend background')

        # Title Font
        lbl = gtk.Label('Title Font:')
        table.attach(lbl, 0, 1, 1, 2,
                        xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)

        color = gview.get_preference('legend-title-font-color',
                                     self.default_color)
        cb = pgucolor.ColorButton(color)
        cb.connect('color-set', self.set_color_preference, 
                   'legend-title-font-color')
        table.attach(cb, 1, 2, 1, 2,
                xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)
        self.tips.set_tip(cb, 
                'Click to change the default color for the legend font')

        # Get preference as XLFD font spec or pango name
        font_spec = gview.get_preference('legend-title-font', self.default_font)
        self.title_font_button = pgufont.FontControl(font_spec, self.set_title_font_cb)

        table.attach(self.title_font_button, 2, 3, 1, 2,
                        xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)
        self.tips.set_tip(self.title_font_button,
                           'Select a font for the legend title')

        # Label Font
        lbl = gtk.Label('Label Font:')
        table.attach(lbl, 0, 1, 2, 3,
                        xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)
        color = gview.get_preference('legend-label-font-color',
                                     self.default_color)
        cb = pgucolor.ColorButton(color)
        cb.connect('color-set', self.set_color_preference, 
                   'legend-label-font-color')
        table.attach(cb, 1, 2, 2, 3,
                        xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)
        self.tips.set_tip(cb, 
                'Click to change the default color for the legend font')

        # Get preference as XLFD font spec or pango name
        font_spec = gview.get_preference('legend-label-font', self.default_font)
        self.label_font_button = pgufont.FontControl(font_spec, self.set_label_font_cb)

        table.attach(self.label_font_button, 2, 3, 2, 3,
                        xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)
        self.tips.set_tip(self.label_font_button, 'Select a font for legend labels')

        # Sample Size

        lbl = gtk.Label( 'Legend Samples:')
        table.attach(lbl, 0, 1, 3, 4,
                xoptions=gtk.SHRINK, yoptions=gtk.SHRINK)

        lbl = gtk.Label( 'X Size:')
        table.attach(lbl, 1, 2, 3, 4,
                xoptions=gtk.SHRINK, yoptions=gtk.SHRINK)


        x = gview.get_preference('legend-sample-x-size', 30)
        y = gview.get_preference('legend-sample-y-size', 20)

        spin_adjust = gtk.Adjustment(value=float(x), lower=0.0,
                        upper=50.0, step_incr=1.0)
        spin = gtk.SpinButton(spin_adjust)
        spin.set_digits(0)
        spin.set_size_request(75,20)
        spin.connect('value-changed', self.set_spin_preference, 
                     'legend-sample-x-size')
        spin.connect('focus-out-event', self.check_spin_preference, 
                     'legend-sample-x-size')
        table.attach(spin, 2, 3, 3, 4,
                        xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)
        self.tips.set_tip(spin, 'The X Size of a sample on the legend dialog')

        lbl = gtk.Label( 'Y Size:')
        table.attach(lbl, 3, 4, 3, 4,
                xoptions=gtk.SHRINK, yoptions=gtk.SHRINK)

        spin_adjust = gtk.Adjustment(value=float(y), lower=0.0,
                        upper=50.0, step_incr=1.0)
        spin = gtk.SpinButton(spin_adjust)
        spin.set_digits(0)
        spin.set_size_request(75, 20)
        spin.connect('value-changed', self.set_spin_preference, 
                     'legend-sample-y-size')
        spin.connect('focus-out-event', self.check_spin_preference, 
                     'legend-sample-y-size')
        table.attach(spin, 4, 5, 3, 4,
                        xoptions = gtk.SHRINK, yoptions=gtk.SHRINK)
        self.tips.set_tip(spin, 'The Y Size of a sample on the legend dialog')
        return vbox

    def set_title_font_cb(self, font_btn):
        gview.set_preference('legend-title-font', font_btn.get_xlfdfont())

    def set_label_font_cb(self, font_btn):
        gview.set_preference('legend-label-font', font_btn.get_xlfdfont())

    def set_entry_text(self, widget, dlg, entry, pref):
        directory = dlg.get_directory()
        entry.set_text(directory)
        entry.grab_focus()
        gview.set_preference( pref, directory )
        dlg.destroy()

    def set_color_preference(self, widget, pref):
        ##def set_color_preference(self, widget, color, pref):
        """
        """
        gview.set_preference(pref, str(widget.get_color()))

    def set_toggle_preference( self, widget, pref ):
        """set a preference from a toggle button
        """
        gview.set_preference( pref, str(widget.get_active()) )

    def set_any_preference(self, widget, pref, func):
        """
        set a preference from the results of a function call
        """
        gview.set_preference(pref, str(func()))

    def set_directory_preference(self, widget, event, pref, func):
        """
        set a preference for a directory.  Check the value first.
        """
        self.current_widget = widget

        value = func()
        if not os.path.isdir( value ) and value != "":
            warning( "Invalid Path:\n%s" % value )
            widget.set_text( gview.get_preference( pref, gview.home_dir)  )
        else:
            self.set_any_preference(widget, pref, func)

    def set_menu_preference(self, widget, pref, value):
        """
        """
        gview.set_preference(pref, str(value))

    def set_spin_preference(self, widget, pref):
        """
        """
        val = widget.get_value()

        if widget.get_text() != str(widget.get_value_as_int()):
            if widget.get_text() == str(round(val,2)):
                gview.set_preference( pref, 
                                str(round(val,2)))
            else:
                try:
                    i = float( widget.get_text() )
                except:
                    i = 0
                widget.set_value( i )
        else:
            gview.set_preference(pref, str(widget.get_value_as_int()))

    def check_spin_preference(self, widget, event, pref):
        """
        added to catch unset preferences in spin buttons on focus-out events.
        """
        self.set_spin_preference( widget, pref )



class Position_3D_Dialog(gtk.Window):
    def __init__(self, view_manager):
        gtk.Window.__init__(self)
        self.set_title('3D Position')
        self.set_border_width(3)
        self.create_position_dialog()
        self.create_lookAt_dialog()
        self.show_all()
        self.view_manager = view_manager

    def create_position_dialog(self):
        self.dialog = gtk.VBox(homogeneous=False, spacing=3)
        self.add(self.dialog)
        self.dialog.pack_start(gtk.Label('Current Position:'))

        # x
        x_box = gtk.HBox(homogeneous=False, spacing=5)
        self.dialog.pack_start(x_box, expand=False)

        x = gtk.Label('X: ')
        x_value = gtk.Entry()
        x_value.set_max_length(10)
        x_value.set_text('')
        x_box.pack_start(x, expand=False)
        x_box.pack_start(x_value, expand=False)

        # y
        y_box = gtk.HBox(homogeneous=False, spacing=5)
        self.dialog.pack_start(y_box, expand=False)

        y = gtk.Label('Y: ')
        y_value = gtk.Entry()
        y_value.set_max_length(10)
        y_value.set_text('')
        y_box.pack_start(y, expand=False)
        y_box.pack_start(y_value, expand=False)

        # z
        z_box = gtk.HBox(homogeneous=False, spacing=5)
        self.dialog.pack_start(z_box, expand=False)

        z = gtk.Label('Z: ')
        z_value = gtk.Entry()
        z_value.set_max_length(10)
        z_value.set_text('')
        z_box.pack_start(z, expand=False)
        z_box.pack_start(z_value, expand=False)

        self.x_value = x_value
        self.y_value = y_value
        self.z_value = z_value

        self.x_value.connect('activate', self.set_position_cb)
        self.x_value.connect('leave-notify-event',self.set_position_cb)
        self.y_value.connect('activate', self.set_position_cb)
        self.y_value.connect('leave-notify-event',self.set_position_cb)
        self.z_value.connect('activate', self.set_position_cb)
        self.z_value.connect('leave-notify-event',self.set_position_cb)


    def create_lookAt_dialog(self):
        # Assume create_position_dialog called

        # Row or x
        self.dialog.pack_start(gtk.HSeparator())
        self.dialog.pack_start(gtk.Label('Looking At Position:'))
        row_box = gtk.HBox(homogeneous=False, spacing=5)
        self.dialog.pack_start(row_box, expand=False)

        row = gtk.Label('X: ')
        row_value = gtk.Entry()
        row_value.set_max_length(10)
        row_value.set_text('')
        row_box.pack_start(row, expand=False)
        row_box.pack_start(row_value, expand=False)

        # Column or y
        col_box = gtk.HBox(homogeneous=False, spacing=5)
        self.dialog.pack_start(col_box, expand=False)

        col = gtk.Label('Y: ')
        col_value = gtk.Entry()
        col_value.set_max_length(10)
        col_value.set_text('')
        col_box.pack_start(col, expand=False)
        col_box.pack_start(col_value, expand=False)

        self.row_value = row_value
        self.col_value = col_value

        self.row_value.connect('activate', self.set_look_at_cb)
        self.row_value.connect('leave-notify-event',self.set_look_at_cb)
        self.col_value.connect('activate', self.set_look_at_cb)
        self.col_value.connect('leave-notify-event',self.set_look_at_cb)


    def update_cb(self, view, *args):
        # Reset Dialog values

        eye_pos = view.get_eye_pos()
        if eye_pos:
            self.x_value.set_text(str(round(eye_pos[0],2)))
            self.y_value.set_text(str(round(eye_pos[1],2)))
            self.z_value.set_text(str(round(eye_pos[2],2)))
        else:
            self.x_value.set_text('')
            self.y_value.set_text('')
            self.z_value.set_text('')

        lookat_pos = view.get_look_at_pos()
        if lookat_pos:
            self.row_value.set_text(str(round(lookat_pos[0],2)))
            self.col_value.set_text(str(round(lookat_pos[1],2)))
        else:
            self.row_value.set_text('')
            self.col_value.set_text('')

    def update_test(self, view, *args):
        eye_pos = view.get_eye_pos()
        lookat_pos = view.get_look_at_pos()

        view.set_3d_view_look_at((eye_pos[0]+300, eye_pos[1], eye_pos[2]) , lookat_pos)



    def set_look_at_cb(self, *args):
        view = self.view_manager.get_active_view()

        # get lookat values, except if at horizon, then None
        try:
            lookat_pos = (float(self.row_value.get_text()), float(self.col_value.get_text()))
        except ValueError:
            lookat_pos = None

        eye_pos = view.get_eye_pos()

        if lookat_pos:
            view.set_3d_view_look_at(eye_pos, lookat_pos )

    def set_position_cb(self, *args):
        view = self.view_manager.get_active_view()

        lookat_pos = view.get_look_at_pos()
        entries = self.x_value.get_text(), self.y_value.get_text(), self.z_value.get_text()
        if '' in entries:
            return

        eye_pos = tuple(map(float, entries))

        # Could be looking at horizon
        if lookat_pos:
            view.set_3d_view_look_at(eye_pos, lookat_pos)
        else:
            eye_dir = view.get_eye_dir()
            view.set_3d_view(eye_pos, eye_dir)



class Tool_GViewApp:
    # Abstract base class to derive tools from
    def __init__(self,app=None):
        self.app = app
        self.menu_entries = Tool_GViewAppMenuEntries()
        self.icon_entries = Tool_GViewAppIconEntries()
        self.pymenu_entries = Tool_GViewAppMenuEntries()
        self.pyicon_entries = Tool_GViewAppIconEntries()
    # def set_menu(self):
    #    placeholder function- make self.menu_entries.set_entry
    #    calls here...
    #    pass


class Tool_GViewAppMenuEntries:
    # Class to store entries to be added to openev's menu
    def __init__(self):
        self.entries = {}

    def set_entry(self,item,position=0,callback=None,accelerator=None):
        # item = a string describing menu location
        # position = default location in the menu (integer): Ignored if an
        #            xml menu entry is specified for the tool.  Note:
        #            when used, the position refers to position in the
        #            lowest level menu.  Eg. if a menu entry is
        #            'File/menu1/entryN', position refer's to entryN's
        #            position within menu1, not menu1's position in
        #            File.  For more flexibility, use the xml form of
        #            configuration.
        # callback = callback
        # accelerator = shortcut key

        if (type(item) == type('')):
            if (type(position) == type(0)):
                self.entries[item] = (position,callback, accelerator)
            else:
                raise AttributeError,"position should be an integer"
        else:
            raise AttributeError,"Menu entry item must be a string"


class Tool_GViewAppIconEntries:
    # Class to store entries to be added to openev's menu
    def __init__(self):
        self.entries = []

    def set_entry(self,iconfile,hint_text,position=0,callback=None,help_topic=None,label=None,icontype='xpm'):
        # iconfile=icon filename (xpm case), or some other string not yet defined
        #          (pixmap/widget case- not yet supported- may never be)
        # hint_text=tooltip text to use
        # position = default location in the icon bar (integer)
        # callback = callback
        # help topic = html help file (not yet used by anything)
        # label = some gtk think- not sure what this does
        # icontype = 'xpm' (later may allow 'pixmap' or 'widget', but not yet)

        if (type(iconfile) == type('')):
            import os
            if os.path.isfile(iconfile):
                fullfilename=iconfile
            elif os.path.isfile(os.path.join(gview.home_dir,'tools',iconfile)):
                fullfilename=os.path.join(gview.home_dir,'tools',iconfile)
            elif os.path.isfile(os.path.join(gview.home_dir,'pics',iconfile)):
                fullfilename=os.path.join(gview.home_dir,'pics',iconfile)                
            else:
                txt = "Cannot find file "+iconfile+'.  Either the full\n'
                txt = txt+"path must be specified, or "+iconfile+ " must be\n"
                txt = txt+"placed in the tools or pics directory."
                raise AttributeError,txt

            # On nt, path separators need to be trapped and doubled to avoid
            # being interpreted as an escape before special characters.
            if os.name == 'nt':
                fullfilename = fullfilename.replace("\\","\\\\")

            if (type(position) == type(0)):
                self.entries.append((fullfilename,label,hint_text,position,callback,help_topic,icontype))
            else:
                raise AttributeError,"position should be an integer"
        else:
            txt = "Cannot find file "+iconfile+'.  Either the full\n'
            txt = txt+"path must be specified, or "+iconfile+ " must be\n"
            txt = txt+"placed in the tools or pics directory."
            raise AttributeError,txt
