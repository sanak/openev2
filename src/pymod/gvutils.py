###############################################################################
# $Id: gvutils.py,v 1.1.1.1 2005/04/18 16:38:35 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Convenience widgets, and services built on Gtk widgets.
#           Note that these will eventually be moved into an Atlantis wide
#           set of utility classes in python.
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

import gtk; _gtk = gtk; del gtk
from gtk.gdk import *
import string
import pgu
import os
import sys
import pgufilesel

def is_of_class(class_obj,class_name):
    if class_obj.__name__ == class_name:
        return 1
    for c in class_obj.__bases__:
        if is_of_class(c,class_name) == 1:
            return 1
    return 0
    
class GvOptionMenu(_gtk.OptionMenu):

    def __init__(self, contents, callback = None):
        _gtk.OptionMenu.__init__(self)

        menu = _gtk.Menu()
        self.callback = callback

        item_widget = None
        counter = 0
        for item in contents:
            item_widget = _gtk.RadioMenuItem( item_widget, item )
            item_widget.show()
            item_widget.connect('activate', self.set_om_selection,
                                counter )
            menu.append(item_widget)
            counter = counter + 1

        self.cur_selection = 0
        menu.show()
        self.set_menu(menu)

    def set_history(self, item):
        if item == self.cur_selection:
            return
        
        self.cur_selection = item
        _gtk.OptionMenu.set_history( self, item )

        if self.callback is not None:
            self.callback( self )

    def get_history(self):
        return self.cur_selection
        
    def set_om_selection(self, widget, data ):
        if widget.active:
            self.set_history( data )

pgu.gtk_register('GvOptionMenu',GvOptionMenu)

#
# Copied from GtkExtra
#
class _MessageBox(_gtk.Dialog):
        def __init__(self, message="", buttons=(), pixmap=None,
                     modal=True):
                _gtk.Dialog.__init__(self)
                self.connect("destroy", self.quit)
                self.connect("delete_event", self.quit)
                self.set_modal(modal)
                if modal:
                    self.set_modal(True)
                hbox = _gtk.HBox(spacing=5)
                hbox.set_border_width(5)
                self.vbox.pack_start(hbox)
                hbox.show()
                if pixmap:
                        self.realize()
                        pm = _gtk.Image()
                        pm.set_from_file(pixmap)
                        hbox.pack_start(pm, expand=False)
                        pm.show()
                label = _gtk.Label(message)
                label.set_justify( _gtk.JUSTIFY_LEFT )
                hbox.pack_start(label)
                label.show()

                for text in buttons:
                        b = _gtk.Button(text)
                        b.set_flags(_gtk.CAN_DEFAULT)
                        b.set_data("user_data", text)
                        b.connect("clicked", self.click)
                        self.action_area.pack_start(b)
                        b.show()
                self.ret = None

        def quit(self, *args):
                self.hide()
                self.destroy()
                if self.modal:
                    _gtk.main_quit()
                    
        def click(self, button):
                self.ret = button.get_data("user_data")
                self.quit()

def warning( text ):
    import gview
    import os.path
    
    warning_pixmap = os.path.join(gview.home_dir,'pics','warning.xpm')
    win = _MessageBox(text, ('OK',), pixmap=warning_pixmap, modal=False )
    win.set_title('Warning')
    win.show()
    return

def error( text ):
    import gview
    import os.path
    
    warning_pixmap = os.path.join(gview.home_dir,'pics','warning.xpm')
    win = _MessageBox(text, ('OK',), pixmap=warning_pixmap, modal=True )
    win.set_title('ERROR')
    win.show()
    _gtk.main()
    
    return

def is_shapefile( filename ):
    try:
        ext = string.lower(filename[len(filename)-4:])
        if ext == '.shp' or ext == '.shx' or ext == '.dbf':
            return 1
        else:
            return 0
    except:
        return 0

def is_project_file( filename ):
    try:
        ext = string.lower(filename[len(filename)-4:])
        if ext == '.opf':
            return 1

        first_line = open(filename).read(20)
        if first_line[:10] == '<GViewApp>':
            return 1
        else:
            return 0
    except:
        return 0


# GvMenuFactory is just GtkExtra.MenuFactory, with the
# addition of a function to allow you to insert entries
# after the fact...

class GvMenuFactory(_gtk.MenuBar):
    def __init__(self, type=0):
        _gtk.MenuBar.__init__(self)
        self.accelerator = _gtk.AccelGroup()
        self.__menus = {}
        self.__items = {}
    def add_entries(self, entries):
        for entry in entries:
            apply(self.create, tuple(entry))
    def create(self, path, accelerator=None, callback=None, *args):
        last_slash = string.rfind(path, '/')
        if last_slash < 0:
            parentmenu = self
        else:
            parentmenu = self.get_menu(path[:last_slash])
        label = path[last_slash+1:]
        if label == '<separator>':
            item = _gtk.MenuItem()
        elif label[:7] == '<check>':
            item = _gtk.CheckMenuItem(label[7:])
        else:
            item = _gtk.MenuItem(label)
        if label != '<nothing>':
            item.show()
        if accelerator:
            key, mods = self.parse_accelerator(accelerator)
            item.add_accelerator("activate", self.accelerator,
                                     key, mods, 'visible')
        if callback:
            apply(item.connect, ("activate", callback) + args)
        # right justify the help menu automatically
        if string.lower(label) == 'help' and parentmenu == self:
            item.set_right_justified(True)
        parentmenu.append(item)
        self.__items[path] = item
        return item
    def get_menu(self, path):
        if path == '':
            return self
        if self.__menus.has_key(path):
            return self.__menus[path]
        wid = self.create(path)
        menu = _gtk.Menu()
        menu.set_accel_group(self.accelerator)
        wid.set_submenu(menu)
        self.__menus[path] = menu
        return menu
    def parse_accelerator(self, accelerator):
        key = 0
        mods = 0
        done = False
        while not done:
            if accelerator[:7] == '<shift>':
                mods = mods | _gtk.gdk.SHIFT_MASK
                accelerator = accelerator[7:]
            elif accelerator[:5] == '<alt>':
                mods = mods | _gtk.gdk.MOD1_MASK
                accelerator = accelerator[5:]
            elif accelerator[:6] == '<meta>':
                mods = mods | _gtk.gdk.MOD1_MASK
                accelerator = accelerator[6:]
            elif accelerator[:9] == '<control>':
                mods = mods | _gtk.gdk.CONTROL_MASK
                accelerator = accelerator[9:]
            else:
                done = True
                key = ord(accelerator[0])
        return key, mods
    def remove_entry(self, path):
        if path not in self.__items.keys():
            return
        item = self.__items[path]
        item.destroy()
        length = len(path)
        # clean up internal hashes
        for i in self.__items.keys():
            if i[:length] == path:
                del self.__items[i]
        for i in self.__menus.keys():
            if i[:length] == path:
                del self.__menus[i]

    def get_entry(self, path):
        result = []
        if path not in self.__items.keys():
            return result
        item = self.__items[path]
        result.append(item)
        length = len(path)
        # clean up internal hashes
        for i in self.__items.keys():
            if i[:length] == path:
                result.append(self.__items[i])
        for i in self.__menus.keys():
            if i[:length] == path:
                result.append(self.__menus[i])
		
	return result
    def remove_entries(self, paths):
        for path in paths:
            self.remove_entry(path)
    def find(self, path):
        return self.__items[path]

    def insert_entry(self, pos, path, accelerator=None, callback=None, *args):
        # like create, but lets you specify position in menu
        last_slash = string.rfind(path, '/')
        if last_slash < 0:
            parentmenu = self
        else:
            parentmenu = self.insert_get_menu(path[:last_slash])
        label = path[last_slash+1:]
        if label == '<separator>':
            item = _gtk.MenuItem()
        elif label[:7] == '<check>':
            item = _gtk.CheckMenuItem(label[7:])
        else:
            item = _gtk.MenuItem(label)
        if label != '<nothing>':
            item.show()
        if accelerator:
            key, mods = self.parse_accelerator(accelerator)
            item.add_accelerator("activate", self.accelerator,
                             key, mods, 'visible')
        if callback:
            apply(item.connect, ("activate", callback) + args)
        # right justify the help menu automatically
        if string.lower(label) == 'help' and parentmenu == self:
            item.right_justify()
        # all this copying for just the next few line...
        if pos is not None:
            parentmenu.insert(item, pos)
        elif parentmenu == self:
            # Make sure Help retains far-right position
            if self.__menus.has_key('Help'):
                num_main_menus = 0
                for current_path in self.__menus.keys():
                    # Check that it isn't a sub-menu...
                    temp_slash = string.rfind(current_path,'/')
                    if temp_slash < 0:
                        num_main_menus = num_main_menus + 1
                parentmenu.insert(item,max(num_main_menus - 1,1))
            else:
                parentmenu.append(item)
        else:
            parentmenu.append(item)

        self.__items[path] = item
        return item

    def insert_get_menu(self, path):
        # Allows new menus to be placed before help on toolbar
        # by using insert_entry to create parents instead of 
        # create.
        if path == '':
            return self
        if self.__menus.has_key(path):
            return self.__menus[path]
        wid = self.insert_entry(None,path)
        menu = _gtk.Menu()
        menu.set_accel_group(self.accelerator)
        wid.set_submenu(menu)
        self.__menus[path] = menu
        return menu


def read_keyval( line ) :
    import re
    import string

    # skip comments & lines that don't contain a '='
    if line[0] == '#' : return [None,None]
    if '=' not in line : return [None,None]

    # Grab the key, val
    [ key, val ] = re.compile( r"\s*=\s*" ).split( line )

    # Strip excess characters from the key string
    key_re = re.compile( r"\b\w+\b" )
    key = key[key_re.search(key).start():]
    key = key[:key_re.search(key).end()]

    # Strip excess characters from the value string
    val = string.strip( val )
    i = string.find( val, ' ' )
    if i > 0 : val = val[0:i]

    return [ key, val ]


def get_tempdir():
    if os.environ.has_key('TMPDIR'):
        tmpdir = os.environ['TMPDIR']
    elif os.environ.has_key('TEMPDIR'):
        tmpdir = os.environ['TEMPDIR']
    elif os.environ.has_key('TEMP'):
        tmpdir = os.environ['TEMP']
    else:
        if os.name == 'nt':
            tmpdir = 'C:'
        else:
            tmpdir = '/tmp'

    return tmpdir

def tempnam( tdir = None, basename = None, extension = None ):
    import os.path
    import gview

    if tdir is None:
        plotfile = gview.get_preference('gvplot_tempfile')
        if plotfile is not None and len(plotfile) > 0:
            if os.path.isdir(plotfile):
                tdir = plotfile
            elif os.path.isdir(os.path.dirname(plotfile)):
                tdir = os.path.dirname(plotfile)
            else:
                tdir = get_tempdir()
        else:
            tdir = get_tempdir()

    if basename is None:
        try:
            pgu.pnm = pgu.pnm + 1
        except:
            pgu.pnm = 1
        basename = 'OBJ_' + str(pgu.pnm)

    if extension is None:
        extension = 'tmp'

    return os.path.join(tdir,basename + '.' + extension)        

def FindExecutable( exe_name ):
    """Try to return full path to requested executable.

    First checks directly, then searches $OPENEV_HOME/bin and the PATH.
    Will add .exe on NT.  Returns None on failure.
    """

    import os.path
    import gview
    import string

    if os.name == 'nt':
        (root, ext) = os.path.splitext(exe_name)
        if ext != '.exe':
            exe_name = exe_name + '.exe'

    if os.path.isfile(exe_name):
        return exe_name

    if os.path.isfile(os.path.join(gview.home_dir,'bin',exe_name)):
        return os.path.join(gview.home_dir,'bin',exe_name)

    exe_path = os.environ['PATH']
    if (os.name == 'nt'):
        path_items = string.split(exe_path,';')
    else:
        path_items = string.split(exe_path,':')

    for item in path_items:
        exe_path = os.path.join(item,exe_name)
        if os.path.isfile(exe_path):
            return exe_path

    return None

def XMLFindValue( node, path, default = None ):
    import gdal
    if path == '' or path == None:
        tnode = node
    else:
        tnode = XMLFind( node, path )
        if tnode is None:
            return default

    for child in tnode[2:]:
        if child[0] == gdal.CXT_Text:
            return child[1]

    return default

def XMLFind( node, path, maxfind=1, attr=None,value=None ):
    import gdal
    broken_up = string.split( path, '.', 1 )
    found_list=[]
    if len(broken_up) == 2:
        component, rest_of_path = broken_up
    else:
        component, rest_of_path = broken_up[0], None

    for subnode in node[2:]:
        if subnode[1] == component and \
          (subnode[0] == gdal.CXT_Element or subnode[0] == gdal.CXT_Attribute):
            if rest_of_path is None:
                if ((attr is None) and (value is None)):
                    found_list.append(subnode)
                    if ((maxfind is not None) and (len(found_list) >= maxfind)):
                        break
                else:
                    if XMLFindValue(subnode,attr) == value:
                        found_list.append(subnode)
                    if ((maxfind is not None) and (len(found_list) >= maxfind)):
                        break                        
            else:
                if maxfind is None:
                    submaxfind=maxfind
                else:
                    submaxfind=maxfind-len(found_list)
                    
                sub_list = XMLFind( subnode, rest_of_path, submaxfind,attr, value )
                if sub_list is not None:
                    if submaxfind > 1:
                        # If maxfind > 1, a list of lists is returned...
                        found_list.extend(sub_list)
                    else:
                        found_list.append(sub_list)
                        
    if len(found_list) == 0:
        return None
    elif maxfind == 1:
        return found_list[0]
    else:
        return found_list

def XMLInstantiate( node, parent, filename=None ):
    import gdal

    if len(node) < 2 or node[0] != gdal.CXT_Element:
        raise AttributeError,'corrupt value passed to XMLInstantiate.'
        return None

    classname = node[1]
    module = XMLFindValue( node, 'module', 'gview' )

    try:
        exec "import " + module
        exec "func = %s.%sFromXML" % (module, classname)
        instance = func( node, parent, filename=filename )
        return instance
    except:
        warning( 'Failed to instantiate a %s:%s' % (module, classname) )
        #raise
        return None

def XMLSerializeSimpleObjAttributes( obj, attrib_list, xml_list = [] ):
    """
    This method is used to serlialize a list of simple object attributes
    as elements in a gdal compatible "pseudo-xml-list-tree".  Each attribute
    found on the source attribute will be converted to string type using the
    str() function, and added to the XML tree as an element with the
    element name being the attribute name, and the value being the
    contents of the element.

    This serialization approach (along with XMLDeserializeSimpleObjAttributes
    is intended to make saving and restoring objects with lots of simple
    attributes to and from a project file fairly easy.

    obj -- the object instance from which attributes will be extracted.
    attrib_list -- a list of attribute tuples.  Each tuple contains the
    attribute name and a function for converting a string into the
    appropriate type (normally one of str, int or float).
    xml_list -- the existing tree to which the new elements will be added.

    Returns the modified xml_list.

    Example attribute list:

    attrib_list = [ (filename, str), (xsize, int), (ysize, int) ]

    
    """ 
    import gdal
    
    for item in attrib_list:
        if obj.__dict__.has_key( item[0] ):
            text_value = str(obj.__dict__[item[0]])
            xml_list.append( [gdal.CXT_Element, item[0],
                              [gdal.CXT_Text, text_value] ] )

    return xml_list

def XMLDeserializeSimpleObjAttributes( obj, attrib_list, xml_tree ):
    failures = 0
    
    for item in attrib_list:
        text_value = XMLFindValue( xml_tree, item[0], None )
        if text_value is not None:
            try:
                func = item[1]
                typed_value = func( text_value )
                obj.__dict__[item[0]] = typed_value
            except:
                failures = failures + 1
                print 'Failed to decode %s attribute with text value (%s).' \
                      % ( item[0], text_value )
        
    return failures

# XMLPop, XMLInsert, XMLReplaceAttr: tools for manipulating xml files-
# Might be changed or removed later.

def XMLPop(node,path,maxpop=1,attr=None,value=None,overwrite='n'):
    # Pop path from node if path has attr=value.
    # pop up to maxpop instances, where maxpop is
    # 1 by default.  Set maxpop to None to return all
    # instances.
    # Returns (cnode,list of popped nodes), where cnode
    # is a copy of node with the excess stuff removed
    # if overwrite is set to 'y', node is altered and returned
    
    import gdal


    # avoid overwriting the contents of node
    if overwrite=='n':
        import copy
        cnode=copy.deepcopy(node)
    else:
        cnode=node
        
    broken_up = string.split( path, '.', 1 )
    popped_list=[]
    subpopped=[]
    if len(broken_up) == 2:
        component, rest_of_path = broken_up
    else:
        component, rest_of_path = broken_up[0], None

    if ((maxpop is not None) and (maxpop < 1)):
        return (cnode,[])
    
    indx=1
    indxlist=[]
    count=0
    
    for subnode in cnode[2:]:
        indx=indx+1
        if subnode[1] == component and \
          (subnode[0] == gdal.CXT_Element or subnode[0] == gdal.CXT_Attribute):
            if rest_of_path is None:
                if ((attr is None) and (value is None)):
                    # Store index for later popping
                    indxlist.append(indx)
                    count=count+1
                    if ((maxpop is not None) and (count >= maxpop)):
                        break
                else:
                    if XMLFindValue(subnode,attr) == value:
                        indxlist.append(indx)
                        count=count+1
                        if ((maxpop is not None) and (count >= maxpop)):
                            break
            else:
                if maxpop is None:
                    submaxpop=None
                else:
                    submaxpop=maxpop-count
                junk,sub_list = XMLPop(subnode,rest_of_path,submaxpop,attr,value,overwrite='y')
                if len(sub_list) > 0:
                    count=count+len(sub_list)
                    subpopped.extend(sub_list)
                    
                if count >= maxpop:
                    break
                
    # pop the top-level values now
    pcount=0
    for indx in indxlist:
        popped_list.append(cnode.pop(indx-pcount))
        # index should decrease with each pop...
        pcount=pcount+1

        
    return (cnode,popped_list)


def XMLInsert(node,path,newnode,maxinsert=1,attr=None,value=None,overwrite='n'):
    # Append newnode to all instances of path found within node
    # that have attr=value, up to a maximum of maxinsert instances.
    # Set maxinsert to None to insert in all path instances.
    # Return the number of items inserted.
    import gdal

    # avoid overwriting the contents of node
    if overwrite=='n':
        import copy
        cnode=copy.deepcopy(node)
    else:
        cnode=node
     
    broken_up = string.split( path, '.', 1 )
    if ((maxinsert is not None) and (maxinsert < 1)):
        return (cnode,0)
    
    insert_num=0
    if len(broken_up) == 2:
        component, rest_of_path = broken_up
    else:
        component, rest_of_path = broken_up[0], None

    indx=1
    
    if path == '' and attr is None:
        # Insert at top level and return
        cnode.append(newnode)
        return (cnode,1)
    
    for subnode in cnode[2:]:
        indx=indx+1
        if subnode[1] == component and \
          (subnode[0] == gdal.CXT_Element or subnode[0] == gdal.CXT_Attribute):
            if rest_of_path is None:
                if ((attr is None) and (value is None)):
                    subnode.append(newnode)
                    insert_num=insert_num+1
                    if ((maxinsert is not None) and (insert_num >= maxinsert)):
                        return (cnode,insert_num)
                else:
                    if XMLFindValue(subnode,attr) == value:
                        subnode.append(newnode)
                        insert_num=insert_num+1
                        if ((maxinsert is not None) and (insert_num >= maxinsert)):
                            return (cnode,insert_num) 
            else:
                if maxinsert is None:
                    submaxinsert=None
                else:
                    submaxinsert=maxinsert-insert_num
                junk,subinsert=XMLInsert( subnode, rest_of_path,newnode,submaxinsert,attr,value,overwrite='y' )
                insert_num=insert_num+subinsert
                if ((maxinsert is not None) and (insert_num >= maxinsert)):
                    return (cnode,insert_num)
    return (cnode,insert_num)

def XMLReplaceAttr( node, path, pathvalue, maxreplace=1, attr=None, value=None, overwrite='n' ):
    # path should end with the attribute to be replaced.  attr and value, if entered, should be
    # at the same level as the attribute to be replaced.
    import gdal
    import os.path
    
    if overwrite == 'n':
        import copy
        cnode=copy.deepcopy(node)
    else:
        cnode=node
    replaced=0
    if ((maxreplace is not None) and (maxreplace < 1)):
        return (cnode,replaced)
    
    if path == '' or path == None:
        print 'Error- No attribute to replace was entered...'
        return
    elif ((attr is None) and (value is None)):
        tnode = XMLFind( cnode, path, maxreplace)
        if tnode is None:
            return (cnode,replaced)
    else:
        top_path,replace_attr=os.path.splitext(path)
        if replace_attr == '':
            replace_attr = top_path
            top_path=''
        else:
            replace_attr=replace_attr[1:] # Get rid of .
        inode = XMLFind( cnode, top_path, None,attr, value )
        if inode is None:
            return (cnode,replaced)
        # Of the paths that have attr=value, see which ones also
        # contain the replace_attr to be replaced.
        tnode=[]
        for item in inode:
            temp=XMLFind( item, replace_attr)
            if temp is not None:
                if maxreplace is None:
                    tnode.append(temp)
                elif (len(tnode)<maxreplace):
                    tnode.append(temp)

        if len(tnode) < 1:
            return (cnode, replaced)
        
        if maxreplace == 1:
            tnode=tnode[0]

    if maxreplace == 1:
        for child in tnode[2:]:
            if child[0] == gdal.CXT_Text:
                child[1] = pathvalue
                replaced=replaced+1
                return (cnode,replaced)
    else:
        for item in tnode:
            for child in item[2:]:
                if child[0] == gdal.CXT_Text:
                    child[1] = pathvalue
                    replaced=replaced+1
                    if ((maxreplace is not None) and (replaced >= maxreplace)):
                        return (cnode,replaced)
        
    return (cnode,replaced)



#-----------------------------------------------------------------
# GvDataFilesFrame- function to create data file frame and entries
#-----------------------------------------------------------------
class GvDataFilesFrame(_gtk.Frame):
    def __init__(self,title='',sel_list=('Input','Output'),editable=True):
        _gtk.Frame.__init__(self)
        self.set_label(title)
        self.channels=sel_list

        self.show_list = []
        self.file_dict = {}
        self.button_dict = {}
        self.entry_dict = {}

        #  File options
        file_table = _gtk.Table(len(self.channels),5,False)
        file_table.set_row_spacings(3)
        file_table.set_col_spacings(3)
        self.table = file_table
        self.add(file_table)
        self.show_list.append(file_table)

        for idx in range(len(self.channels)):
            ch = self.channels[idx]
            self.button_dict[ch] = _gtk.Button(ch)
            self.button_dict[ch].set_size_request(100,25)
            self.show_list.append(self.button_dict[ch])
            file_table.attach(self.button_dict[ch], 0,1, idx,idx+1)
            self.entry_dict[ch] = _gtk.Entry()
            self.entry_dict[ch].set_editable(editable)
            self.entry_dict[ch].set_size_request(400, 25)
            self.entry_dict[ch].set_text('')
            self.show_list.append(self.entry_dict[ch])
            self.set_dsfile('',ch)
            file_table.attach(self.entry_dict[ch], 1,5, idx,idx+1)
            if editable == True:
                self.entry_dict[ch].connect('leave-notify-event',self.update_ds)
                

        for bkey in self.button_dict.keys():
            self.button_dict[bkey].connect('clicked',self.set_dsfile_cb,bkey)

    def set_border_width(self,width):
        self.table.set_border_width(width)

    def set_spacings(self, rowspc, colspc):
        self.table.set_row_spacings(rowspc)
        self.table.set_col_spacings(colspc)

    def update_ds(self,*args):
        for ch in self.channels:
            self.set_dsfile(self.entry_dict[ch].get_text(),ch)

    def show(self,*args):
        for item in self.show_list:
            item.show()


    def set_dsfile_cb(self,*args):
        fkey = args[1]
        file_str = 'Select ' + fkey + ' File'
        pgufilesel.SimpleFileSelect(self.set_dsfile,
                                    fkey,
                                    file_str)

    def set_dsfile(self,fname,fkey):
        self.file_dict[fkey] = fname
        
        # Save selected file directory
        head = os.path.dirname(fname)
        if len(head) > 0:
            if os.access(head,os.R_OK):
                pgufilesel.simple_file_sel_dir = head+os.sep
                
        if self.entry_dict.has_key(fkey):            
            if self.file_dict[fkey] is None:
                self.entry_dict[fkey].set_text('')
            else:
                self.entry_dict[fkey].set_text(
                     self.file_dict[fkey])

    def get(self,fkey):
        if self.file_dict.has_key(fkey):
            return self.file_dict[fkey]
        else:
            return None

#-----------------------------------------------------------------
# GvEntryFrame- function to create a frame with a table of entries.
# Input: a list, or list of list, of strings.  initializing with
# [['e1','e2'],['e3'],['e4','e5']] would create a table like this:
#
# e1: <entry>   e2:<entry>
# e3: <entry>
# e4: <entry>   e5:<entry>
#
# The strings in the list are used to index for returning entry
# values, and must be unique.
#
# If some of the entries must have only certain values, a second
# list of the same size may be supplied.  This should contain
# None where entries should be used, but a tuple of strings
# where an option menu should be used.
#-----------------------------------------------------------------
class GvEntryFrame(_gtk.Frame):
    def __init__(self,title,entry_list,widget_list=None):
        _gtk.Frame.__init__(self)
        self.set_label(title)
        table_rows = len(entry_list)
        cols = 1
        for item in entry_list:
            if type(item) in [type((1,)),type([1])]:
                cols = max(cols,len(item))

        # Note: the extra one column is because on windows,
        # creating a gtk table with N columns sometimes only shows
        # N-1 columns???
        self.table = _gtk.Table(table_rows,cols*2+1,False)
        self.table.set_col_spacings(3)
        self.table.set_row_spacings(3)
        self.add(self.table)
        self.entries = {}
        if widget_list is None:
            ridx=0
            for item in entry_list:
                if type(item) in [type((1,)),type([1])]:
                    cidx=0
                    for item2 in item:
                        label = _gtk.Label(item2)
                        label.set_alignment(0,0.5)
                        self.table.attach(label,cidx,cidx+1,ridx,ridx+1)
                        self.entries[item2] = _gtk.Entry()
                        self.entries[item2].set_max_length(30)
                        self.entries[item2].set_editable(True)
                        cidx = cidx+1
                        self.table.attach(self.entries[item2],
                                          cidx,cidx+1,ridx,ridx+1)
                        cidx = cidx+1
                else:
                    label = _gtk.Label(item)
                    label.set_alignment(0,0.5)
                    self.table.attach(label,0,1,ridx,ridx+1)
                    self.entries[item] = _gtk.Entry()
                    self.entries[item].set_max_length(30)
                    self.entries[item].set_editable(True)
                    self.table.attach(self.entries[item],
                                      1,2,ridx,ridx+1)
                ridx=ridx+1
            
        else:
            ridx=0
            for item in entry_list:
                wtype=widget_list[ridx]
                if type(item) in [type((1,)),type([1])]:
                    cidx=0
                    widx=0
                    for item2 in item:
                        wtype2=wtype[widx]
                        label = _gtk.Label(item2)
                        label.set_alignment(0,0.5)
                        self.table.attach(label,cidx,cidx+1,ridx,ridx+1)
                        if wtype2 is None:
                            self.entries[item2] = _gtk.Entry()
                            self.entries[item2].set_max_length(30)
                            self.entries[item2].set_editable(True)
                        else:
                            self.entries[item2] = GvOptionMenu(wtype2)
                            self.entries[item2].set_history(0)
                            self.entries[item2].contents = wtype2
                        cidx = cidx+1
                        self.table.attach(self.entries[item2],
                                          cidx,cidx+1,ridx,ridx+1)
                        cidx = cidx+1
                        widx = widx+1
                else:
                    label = _gtk.Label(item)
                    label.set_alignment(0,0.5)
                    self.table.attach(label,0,1,ridx,ridx+1)
                    if wtype is None:
                        self.entries[item] = _gtk.Entry()
                        self.entries[item].set_max_length(30)
                        self.entries[item].set_editable(True)
                    else:
                        self.entries[item] = GvOptionMenu(wtype)
                        self.entries[item].set_history(0)
                        self.entries[item].contents = wtype
                    self.table.attach(self.entries[item],
                                      1,2,ridx,ridx+1)
                ridx=ridx+1
            

    def get(self,fkey):
        if self.entries.has_key(fkey):
            if hasattr(self.entries[fkey],'get_text'):
                return self.entries[fkey].get_text()
            else:
                hist=self.entries[fkey].get_history()
                return self.entries[fkey].contents[hist]   
        else:
            return None

    def set_default_values(self,default_dict):
        """Set default entry values.  Input is default_dict,
           a dictionary with keys corresponding to entries
           (strings) and values corresponding to default
           text or menu setting (also strings).
        """
        for ckey in default_dict.keys():
            cval = default_dict[ckey]
            if self.entries.has_key(ckey):
                if hasattr(self.entries[ckey],'set_text'):
                    self.entries[ckey].set_text(cval)
                else:
                    useidx=None
                    for idx in range(len(self.entries[ckey].contents)):
                        if self.entries[ckey].contents[idx] == cval:
                            useidx=idx
                    if useidx is not None:
                        self.entries[ckey].set_history(useidx)
                    else:
                        print cval+' not a valid entry for '+ckey
                    
            else:
                print 'No entry '+ckey+'- skipping'

    def set_default_lengths(self,default_dict):
        """ Set the maximum entry lengths for non-menu entries.
            Input is defalut_dict, a dictionary with keys
            corresponding to entries (strings) and values
            corresponding to entry lengths (integers)
        """
        for ckey in default_dict.keys():
            cval = default_dict[ckey]
            if self.entries.has_key(ckey):
                if hasattr(self.entries[ckey],'set_text'):
                    self.entries[ckey].set_max_length(cval)
                else:
                    print 'Length cannot be set for a menu ('+ckey+')'
                    
            else:
                print 'No entry '+ckey+'- skipping'

    def set_border_width(self,width):
        self.table.set_border_width(width)

    def set_spacings(self, rowspc, colspc):
        self.table.set_row_spacings(rowspc)
        self.table.set_col_spacings(colspc)

                
if __name__ == '__main__':
    dialog = _gtk.Window()

    om = GvOptionMenu( ('Option 1', 'Option 2') )
    om.show()
    dialog.add( om )
    
    dialog.connect('delete-event', _gtk.main_quit)
    dialog.show()

    _gtk.main()
