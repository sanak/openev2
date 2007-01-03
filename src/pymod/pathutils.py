###############################################################################
#
# Project:  OpenEV
# Purpose:  Paths utilities
# Author:   Julien Demaria, demaria.julien@free.fr
#
###############################################################################
# Copyright (c) 2004, Julien Demaria <demaria.julien@free.fr>
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


import os.path

try:
    real_path = os.path.realpath
except:
    real_path = os.path.abspath

def split_all( path ):
    """
    Return a list (whithout any '') corresponding to the splitted path.
    """
    splitted_path = []
    (h, t) = os.path.split( path )
    old_h = h + 'dummy'
    while h != old_h:
        old_h = h
        if t != '':
            splitted_path.insert( 0, t )
        (h, t) = os.path.split( h )
    if h != '':
        splitted_path.insert( 0, h )
    return splitted_path

def join_all( path ):
    """
    Return a path from its corresponding splitted list form.
    """
    return apply( os.path.join, path )

def relative_path( path, ref_path ):
    """
    Return the relative form of path, with ref_path as reference.
    If path and ref_path don't have any common prefix,
    raise ValueError, 'Not any common prefix'
    The function internally works on os.path.realpath() forms
    of path and ref_path.
    """
    realpath = real_path( path )
    basepath = ''
    basepath = os.path.basename( realpath )
    realpath = os.path.dirname( realpath )

    real_ref_path = real_path( ref_path )
    real_ref_path = os.path.dirname( real_ref_path )

    split_path = split_all( realpath )
    split_ref_path = split_all( real_ref_path )

    # Find common prefix
    for i, v in enumerate( zip( split_path, split_ref_path ) ):
        if v[0] != v[1]:
            if i == 0:
                # Case where the 2 paths don't have any common prefix
                # (Note the under Unix platform there is always at least
                #  '/' as common prefix...).
                raise ValueError, 'Not any common prefix'
            else:
                break

    if split_path[i] == split_ref_path[i]:
        # Handle the case of one of the path is the prefix of the other
        i += 1
    path_tail = split_path[i:]
    ref_path_tail = split_ref_path[i:]
    rel_path = ['..'] * len( ref_path_tail )
    rel_path.extend( path_tail )
    if basepath != '':
        rel_path.append( basepath )

    return join_all( rel_path )

def PortablePathFromXML( str ):
    t = eval( str )
    p = PortablePath( join_all( t[0] ) )
    p.rel_path = t[1]
    return p

class PortablePath:

    def __init__( self, path, ref_path = None ):
        """
        Create a portable path from a path and optionnaly a reference path.
        A portable path contains :
            - the normalized absolutized splitted form of the given path ;
            - the relativized (with ref_path as reference) splitted form of
              the given path if it exists.
        A portable path internally uses os.path.realpath() forms of path and
        ref_path.
        """
        self.abs_path = split_all( real_path( path ) )
        try:
            self.rel_path = split_all( relative_path( path, ref_path ) )
        except:
            self.rel_path = None

    def local_path( self, ref_path = None ):
        """
        Return the path corresponding to the portable path for the current
        local platform.
        The method tests in the portable path the existence of, in this order :
            - the normalized absolutized form of the original path ;
            - the relativized (with ref_path as reference) form of
              the original path ;
        and then returns the first existing (in os.path.realpath() form).
        If none of them exists,
        raise ValueError, 'Cannot compute a local path from the portable path'
        The method internally works on os.path.realpath() form of ref_path.
        """
        p = join_all( self.abs_path )
        if os.path.exists( p ):
            return p
        else:
            if self.rel_path is None:
                raise ValueError, 'Cannot compute a local path from the portable path'
            r = os.path.dirname( real_path( ref_path ) )
            p = real_path( os.path.join( r, join_all( self.rel_path ) ) )
            if os.path.exists( p ):
                return p
            raise ValueError, 'Cannot compute a local path from the portable path'

    def serialize( self ):
        return repr( [self.abs_path, self.rel_path] )
