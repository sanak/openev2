###############################################################################
# $Id$
#
# Project:  OpenEV / CIETmap
# Purpose:  XML utilities moved from gvutils
# Author:   Frank Warmerdam, warmerdam@pobox.com
#
# Maintained by Mario Beauchamp (starged@gmail.com) for CIETcanada
#
###############################################################################
# Copyright (c) 2007, CIETcanada
# Copyright (c) 2000-2006, CIETcanada
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
#  $Log$

import os.path
from osgeo.gdal import CXT_Attribute, CXT_Element, CXT_Text

def XMLFindValue(node, path, default=None):
    if not path:
        tnode = node
    else:
        tnode = XMLFind(node, path)
        if tnode is None:
            return default

    for child in tnode[2:]:
        if child[0] == CXT_Text:
            return child[1]

    return default

def XMLFind(node, path, maxfind=1, attr=None, value=None):
    broken_up = path.split('.', 1)
    found_list=[]
    if len(broken_up) == 2:
        component, rest_of_path = broken_up
    else:
        component, rest_of_path = broken_up[0], None

    for subnode in node[2:]:
        if subnode[1] == component and (subnode[0] == CXT_Element or subnode[0] == CXT_Attribute):
            if rest_of_path:
                if maxfind:
                    submaxfind = maxfind - len(found_list)
                else:
                    submaxfind = maxfind

                sub_list = XMLFind(subnode, rest_of_path, submaxfind, attr, value)
                if sub_list:
                    if submaxfind > 1:
                        # If maxfind > 1, a list of lists is returned...
                        found_list.extend(sub_list)
                    else:
                        found_list.append(sub_list)
            else:
                if attr and value:
                    if XMLFindValue(subnode, attr) == value:
                        found_list.append(subnode)
                    if maxfind and len(found_list) >= maxfind:
                        break                        
                else:
                    found_list.append(subnode)
                    if maxfind and len(found_list) >= maxfind:
                        break

    if not found_list:
        return None
    elif maxfind == 1:
        return found_list[0]
    else:
        return found_list

def XMLInstantiate(node, parent, filename=None):
    if len(node) < 2 or node[0] != CXT_Element:
        raise AttributeError,'corrupt value passed to XMLInstantiate.'
        return None

    classname = node[1]
    mod_name = ('gview', 'cview')['CIETMAP_HOME' in os.environ]
    module = XMLFindValue(node, 'module', mod_name)

    try:
        exec "from %s import %sFromXML" % (module, classname)
        exec "func = %sFromXML" % classname
        instance = func(node, parent, filename=filename)
        return instance
    except:
        import sys
        import traceback
        if 'CIETMAP_HOME' in os.environ:
            from cietutils import warning
        else:
            from gvutils import warning

        warning('Failed to instantiate a %s:%s' % (module, classname))
        sys_type, sys_value, sys_traceback = sys.exc_info()
        exp = traceback.format_exception(sys_type, sys_value, sys_traceback)
        exception = ''
        for line in exp:
            exception += line
        print exception
        return None

def XMLSerializeSimpleObjAttributes(obj, attrib_list, xml_list=[]):
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
    for item in attrib_list:
        if item[0] in obj.__dict__:
            text_value = str(obj.__dict__[item[0]])
            xml_list.append( [CXT_Element, item[0], [CXT_Text, text_value]] )

    return xml_list

def XMLDeserializeSimpleObjAttributes(obj, attrib_list, xml_tree):
    failures = 0

    for item in attrib_list:
        text_value = XMLFindValue(xml_tree, item[0], None)
        if text_value:
            try:
                func = item[1]
                typed_value = func(text_value)
                obj.__dict__[item[0]] = typed_value
            except:
                failures += 1
                print 'Failed to decode %s attribute with text value (%s).' % (item[0], text_value)

    return failures

# XMLPop, XMLInsert, XMLReplaceAttr: tools for manipulating xml files
def XMLPop(node, path, maxpop=1, attr=None, value=None, overwrite='n'):
    """
    Pop path from node if path has attr=value.
    pop up to maxpop instances, where maxpop is
    1 by default.  Set maxpop to None to return all instances.
    Returns (cnode,list of popped nodes), where cnode
    is a copy of node with the excess stuff removed
    if overwrite is set to 'y', node is altered and returned
    """

    # avoid overwriting the contents of node
    if overwrite == 'n':
        import copy
        cnode = copy.deepcopy(node)
    else:
        cnode=node

    broken_up = path.split('.', 1)
    popped_list = []
    subpopped = []
    if len(broken_up) == 2:
        component, rest_of_path = broken_up
    else:
        component, rest_of_path = broken_up[0], None

    if maxpop and maxpop < 1:
        return (cnode,[])

    indx = 1
    indxlist = []
    count = 0

    for subnode in cnode[2:]:
        indx += 1
        if subnode[1] == component and (subnode[0] == CXT_Element or subnode[0] == CXT_Attribute):
            if rest_of_path:
                if maxpop:
                    submaxpop = maxpop - count
                else:
                    submaxpop = None
                junk, sub_list = XMLPop(subnode, rest_of_path, submaxpop, attr, value, overwrite='y')
                if len(sub_list) > 0:
                    count += len(sub_list)
                    subpopped.extend(sub_list)

                if count >= maxpop:
                    break
            else:
                if attr and value:
                    if XMLFindValue(subnode, attr) == value:
                        indxlist.append(indx)
                        count += 1
                        if maxpop and count >= maxpop:
                            break
                else:
                    # Store index for later popping
                    indxlist.append(indx)
                    count += 1
                    if maxpop and count >= maxpop:
                        break

    # pop the top-level values now
    pcount = 0
    for indx in indxlist:
        popped_list.append( cnode.pop(indx - pcount) )
        # index should decrease with each pop...
        pcount += 1

    return (cnode, popped_list)

def XMLInsert(node, path, newnode, maxinsert=1, attr=None, value=None, overwrite='n'):
    """
    Append newnode to all instances of path found within node
    that have attr=value, up to a maximum of maxinsert instances.
    Set maxinsert to None to insert in all path instances.
    Return the number of items inserted.
    """
    # avoid overwriting the contents of node
    if overwrite == 'n':
        import copy
        cnode = copy.deepcopy(node)
    else:
        cnode = node

    broken_up = path.split('.', 1)
    if maxinsert and maxinsert < 1:
        return (cnode, 0)

    insert_num = 0
    if len(broken_up) == 2:
        component, rest_of_path = broken_up
    else:
        component, rest_of_path = broken_up[0], None

    indx = 1

    if not path and attr is None:
        # Insert at top level and return
        cnode.append(newnode)
        return (cnode, 1)

    for subnode in cnode[2:]:
        indx += 1
        if subnode[1] == component and (subnode[0] == CXT_Element or subnode[0] == CXT_Attribute):
            if rest_of_path:
                if maxinsert:
                    submaxinsert = maxinsert - insert_num
                else:
                    submaxinsert = None
                junk, subinsert = XMLInsert( subnode, rest_of_path, newnode, submaxinsert, attr, value, overwrite='y' )
                insert_num += subinsert
                if maxinsert and insert_num >= maxinsert:
                    return (cnode, insert_num)
            else:
                if attr and value:
                    if XMLFindValue(subnode,attr) == value:
                        subnode.append(newnode)
                        insert_num += 1
                        if maxinsert and insert_num >= maxinsert:
                            return (cnode, insert_num) 
                else:
                    subnode.append(newnode)
                    insert_num += 1
                    if maxinsert and insert_num >= maxinsert:
                        return (cnode, insert_num)

    return (cnode,insert_num)

def XMLReplaceAttr(node, path, pathvalue, maxreplace=1, attr=None, value=None, overwrite='n'):
    """
    path should end with the attribute to be replaced.  attr and value, if entered, should be
    at the same level as the attribute to be replaced.
    """
    if overwrite == 'n':
        import copy
        cnode = copy.deepcopy(node)
    else:
        cnode = node

    replaced = 0
    if maxreplace and maxreplace < 1:
        return (cnode, replaced)

    if not path:
        print 'Error- No attribute to replace was entered...'
        return
    elif attr is None and value is None:
        tnode = XMLFind(cnode, path, maxreplace)
        if tnode is None:
            return (cnode, replaced)
    else:
        top_path, replace_attr = os.path.splitext(path)
        if replace_attr:
            replace_attr = replace_attr[1:] # Get rid of .
        else:
            replace_attr = top_path
            top_path = ''

        inode = XMLFind(cnode, top_path, None, attr, value)
        if inode is None:
            return (cnode, replaced)
        # Of the paths that have attr=value, see which ones also
        # contain the replace_attr to be replaced.
        tnode=[]
        for item in inode:
            temp = XMLFind(item, replace_attr)
            if temp:
                if maxreplace is None:
                    tnode.append(temp)
                elif len(tnode) < maxreplace:
                    tnode.append(temp)

        if len(tnode) < 1:
            return (cnode, replaced)

        if maxreplace == 1:
            tnode = tnode[0]

    if maxreplace == 1:
        for child in tnode[2:]:
            if child[0] == CXT_Text:
                child[1] = pathvalue
                replaced += 1
                return (cnode, replaced)
    else:
        for item in tnode:
            for child in item[2:]:
                if child[0] == CXT_Text:
                    child[1] = pathvalue
                    replaced += 1
                    if maxreplace and replaced >= maxreplace:
                        return (cnode, replaced)

    return (cnode, replaced)
