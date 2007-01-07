#! /usr/bin/env python
###############################################################################
#
# Project:  OpenEV
# Purpose:  Implement the generic Plot function for simple internal plotting.
# Author:   Frank Warmerdam <warmerdam@pobox.com>
#           Gillian Walter <gwalter@atlsci.com>
#
###############################################################################
# Copyright (c) 2001, Atlantis Scientific Inc. (www.atlsci.com)
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

import Numeric
import gtk
import os
import gvutils

###############################################################################
# plot()

def plot( data=None, xaxis=None, yaxis=None, xmin=None, xmax=None,
          ymin=None, ymax=None, title=None, cmds=None, 
          terminal = 'openev', output = None, wintitle=None,
          datastyle = None, multiplot = False , multilabels = (),
          multiopts = ()):
    """plot(data [, xaxis=text] [,yaxis=text] [,title=text] 
                  [, xmin=n] [, xmax=n] [, ymin=n] [, ymax=n] 
                  [, cmds=cmd_list]   [,terminal={"openev","gnuplot"}]
                  [, wintitle=text] [, datastyle=text]
                  [, multiplot = {True,False}] [, multilabels = list]
                  [, multiopts = list]
        data -- data array to plot, should be 1-D set of Y data
                or 2-D array with pairs of (x,y) values.
                or 3-D array with pairs of (x,y,z) values '
                or 2-D array with tuples of (x,y1,y2,...,yN) values.'
    """

    ###########################################################################
    # Print usage() message if no options given.

    if data is None:
        print 'Usage: plot(data [, xaxis=text] [,yaxis=text] [,title=text] '
        print '                 [, xmin=n] [, xmax=n] [, ymin=n] [, ymax=n] '
        print '                 [, cmds=cmd_list] '
        print '                 [,terminal={"openev","gnuplot"}]'
        print '                 [,wintitle=text] [, datastyle=text] '
        print '                 [, multiplot = {True,False}]'
        print '                 [, multilabels = list]  [, multiopts = list] )'
        print ''
        print ' data -- data array to plot, should be 1-D set of Y data'
        print '         or 2-D array with pairs of (x,y) values.'
        print '         or 3-D array with pairs of (x,y,z) values '
        print '         or 2-D array with tuples of (x,y1,y2,...,yN) values.'
        print '         for the last multiplot must be true, multilables is'
        print '         a list of text labels for the graphs, and multiopts'
        print '         is a list of gnuplot options to be added to the'
        print '         individual graphs'
        print ''
        return

    ###########################################################################
    # Work out the shape of the data.  A 1-D array is assumed to be Y
    # values.  An Nx2 array is assumed to be (X,Y) values.  A 3-D array is
    # assumed to be (X,Y,Z) values.  If multiplot is True we need
    # a Nxk array, with K at least 2

    try:
        dshape = Numeric.shape( data )
    except:
        raise ValueError, "data argument to plot() does not appear to be a NumPy array"

    dim = len(dshape)

    ###########################################################################
    # Reformat the list into a uniform Nx2 format.

    if multiplot == False:        
        if dim == 1:
            dim=2
            list = []
            for i in range(len(data)):
                list.append( (i,data[i]) )
            data = list
        elif dim == 2 and dshape[1] == 2 and dshape[0] > 1:
            pass
        else:
            raise ValueError, "data argument dimension or shape is not supported."
    else:
        #error checking for multiplot needs work
        if dim > 1:
            pass
        else:
            raise ValueError, "multiplot dimension too small"
    ###########################################################################
    # Setup Plot Options.

    g = llplot()

    if datastyle is not None:
        cmd = 'set data style ' + str(datastyle)
        g.add_cmd(cmd)
    else:    
        g.add_cmd('set data style linespoints')

    if xaxis is not None:
        g.add_cmd( 'set xlabel "%s"' % xaxis )

    if yaxis is not None:
        g.add_cmd( 'set ylabel "%s"' % yaxis )

    if title is not None:
        g.add_cmd( 'set title "%s"' % title )

    if xmin is not None and xmax is not None:
        g.add_cmd( 'set xrange [%s:%s]' % (str(xmin),str(xmax)) )

    if ymin is not None and ymax is not None:
        g.add_cmd( 'set yrange [%s:%s]' % (str(ymin),str(ymax)) )

    if cmds is not None:
        for cmd in cmds:
            g.add_cmd(cmd)

    g.set_data( data,'',dim,1,multiplot,multilabels,multiopts)

    ###########################################################################
    # Generate output.

    if terminal == 'gnuplot':
        g.batch = 0
        g.plot_current()
        raw_input('Please press return to continue...\n')
        return

    elif terminal == 'postscript':

        g.batch = 1

        #if (os.name == 'nt'):
        #    output = string.join(string.split(output,'\\'),'/')

        g.add_cmd( 'set terminal postscript color 10' )
        g.add_cmd( "set output '%s'" % output )

        g.plot_current()

        return

    elif terminal == 'pbm':

        g.batch = 1

        g.add_cmd( 'set terminal pbm color' )
        g.add_cmd( "set output '%s'" % output )

        g.plot_current()

        return

    elif terminal == 'xpm':

        import gdal
        import time

        g.batch = 1

        out_temp = gvutils.tempnam(extension='png')

        g.add_cmd( 'set terminal png ' )
        ##g.add_cmd( 'set terminal png color' )
        g.add_cmd( "set output '%s'" % out_temp )

        g.plot_current()

        pngDS = gdal.Open( out_temp )
        if pngDS is None:
            return None

        xpmDriver = gdal.GetDriverByName( 'XPM' )
        if xpmDriver is None:
            return None

        xpmDriver.CreateCopy( output, pngDS, 0 )

        pngDS = None
        os.unlink( out_temp )

        return

    else:
        import gdal
        import gdalnumeric
        import time
        import gview

        g.batch = 1

        temp_file = gvutils.tempnam()

        # make sure the file has been created
        create_temp = open(temp_file,'w')
        create_temp.close()   

        g.add_cmd( 'set terminal pbm color' )
        g.add_cmd( "set output '%s'" % temp_file )

        g.plot_current()

        time.sleep( 1 )

        image = gdalnumeric.LoadFile( temp_file )
        image_ds = gdalnumeric.OpenArray( image )

        try:
            os.unlink( temp_file )
        except:
            pass
        rlayer = gview.GvRasterLayer( gview.GvRaster(dataset=image_ds, real=1),
                                      rl_mode = gview.RLM_RGBA )

        rlayer.set_source(1, gview.GvRaster(dataset=image_ds, real=2) )
        rlayer.set_source(2, gview.GvRaster(dataset=image_ds, real=3) )

        if terminal == 'rasterlayer':
            return rlayer

        graphwin = GvGraphWindow( rlayer )
        if wintitle is not None:
            graphwin.set_title(wintitle)

###############################################################################
# plot3d()

def plot3d( data=None, xvec=None, yvec=None, xaxis=None, yaxis=None, zaxis=None,
            xmin=None, xmax=None, ymin=None, ymax=None, zmin=None, zmax = None,
            title=None, cmds=None, terminal = 'openev', output = None,
            plottype="parametric", wintitle=None):

    """plot3d(data [,xvec=xaxis_values] [,yvec=yaxis_values] 
                   [, xaxis=text] [,yaxis=text] [,zaxis=text] [,title=text] 
                   [, xmin=n, xmax=n] [, ymin=n, ymax=n] [, zmin=n, zmax=n] 
                   [, cmds=cmd_list] [,terminal={"openev","gnuplot","rasterlayer"}]
                   [, output=ps_filename]
                   [,plottype = {"parametric","contour"}]
                   [, wintitle=text])

        data -- data array to plot, should be 2-D set of Z values.
                   Size of data should be length(x) x length(y), if x
                   and y are present.'
        xvec -- 1-D Vector of values for axis of first dimension of data.
        yvec -- 1-D Vector of values for axis of second dimension of data.
    """

    ###########################################################################
    # Print usage() message if no options given.

    if data is None:
        print 'Usage: plot3d(data [,xvec=xaxis_values] [,yvec=yaxis_values] '
        print '    [, xaxis=text] [,yaxis=text] [,zaxis=text] [,title=text] '
        print '    [, xmin=n, xmax=n] [, ymin=n, ymax=n] [, zmin=n, zmax=n] '
        print '    [, cmds=cmd_list] [,terminal={"openev","gnuplot,"rasterlayer"}]'
        print '    [, output=ps_filename] [,plottype = {"parametric","contour"}]'
        print ''
        print ' data -- data array to plot, should be 2-D set of Z values.'
        print '         Size of data should be length(x) x length(y), if x'
        print '         and y are present.'
        print ' xvec -- 1-D Vector of values for axis of first dimension of data.'
        print ' yvec -- 1-D Vector of values for axis of second dimension of data.'
        print ''
        return

    ###########################################################################
    # Work out the shape of the data.  A 1-D array is assumed to be Y
    # values.  An Nx2 array is assumed to be (X,Y) values.  All others are
    # currently invalid.

    try:
        dshape = Numeric.shape( data )
    except:
        raise ValueError, "data argument to plot() does not appear to be a NumPy array"

    ##########################################################################
    # Make sure xvec, yvec are valid indices for x/y axis and revert to
    # default if not.

    if xvec is None:
        xvec = Numeric.arange(dshape[0])
    else:
        try:
            xshape = Numeric.shape( xvec )
            if (len(xvec) != dshape[0]):
                print 'Incorrect length for xvec- reverting to default.'
                xvec = Numeric.arange(dshape[0])
            elif (len(xshape) > 1):
                print 'xvec should be 1-D- reverting to default.'
                xvec = Numeric.arange(dshape[0])
        except:
            print 'xvec appears not to be a NumPy array- reverting to default.'
            xvec = Numeric.arange(dshape[0])

    if yvec is None:
        yvec = Numeric.arange(dshape[1])
    else:
        try:
            yshape = Numeric.shape( yvec )
            if (len(yvec) != dshape[1]):
                print 'Incorrect length for yvec- reverting to default.'
                yvec = Numeric.arange(dshape[1])
            elif (len(yshape) > 1):
                print 'yvec should be 1-D- reverting to default.'
                yvec = Numeric.arange(dshape[1])
        except:
            print 'yvec appears not to be a NumPy array- reverting to default.'
            yvec = Numeric.arange(dshape[1])


    ###########################################################################
    # Setup Plot Options.

    g = llplot()

    g.batch = 1

    if plottype == "contour":
        g.add_cmd('set nosurface')
        g.add_cmd('set contour')
        g.add_cmd('set view 0,0')
        g.add_cmd('set data style lines')
        g.add_cmd('set cntrparam levels 15')
    else:
#        g.add_cmd('set parametric')
        g.add_cmd('set data style lines')
        g.add_cmd('set hidden3d nooffset')

    if xaxis is not None:
        g.add_cmd( 'set xlabel "%s"' % xaxis )

    if yaxis is not None:
        g.add_cmd( 'set ylabel "%s"' % yaxis )

    if title is not None:
        g.add_cmd( 'set title "%s"' % title )

    if xmin is not None and xmax is not None:
        g.add_cmd( 'set xrange [%s:%s]' % (str(xmin),str(xmax)) )

    if ymin is not None and ymax is not None:
        g.add_cmd( 'set yrange [%s:%s]' % (str(ymin),str(ymax)) )

    if plottype != "contour":
        if zaxis is not None:
            g.add_cmd( 'set zlabel "%s"' % zaxis )

    if plottype != "contour":
        if zmin is not None and zmax is not None:
            g.add_cmd( 'set zrange [%s:%s]' % (str(zmin),str(zmax)) )

    if cmds is not None:
        for cmd in cmds:
            g.add_cmd(cmd)

    ###########################################################################
    # Attach the data.
    #
    # Note that we emit the x and y values with each data point.  It would
    # be nice to rewrite this to use binary format eventually.

    tup_data = []
    for x_i in range(len(xvec)):
        for y_i in range(len(yvec)):
            tup_data.append( (xvec[x_i], yvec[y_i], data[x_i][y_i]) )

    g.set_data( tup_data, dimension = 3, xlen = len(xvec) )

    ###########################################################################
    # Generate output.

    if terminal == 'gnuplot':
        g.batch = 0

        g.plot_current()
        raw_input('Please press return to continue...\n')

    elif terminal == 'postscript':
        if (os.name == 'nt'):
            output = '/'.join(output.split('\\'))

        g.add_cmd( 'set terminal postscript color 10' )
        g.add_cmd( "set output '%s'" % output )

        g.plot_current()

    else:
        import gdal
        import gdalnumeric
        import time
        import gview

        temp_file = gvutils.tempnam()

        g.add_cmd( 'set terminal pbm color' )
        g.add_cmd( "set output '%s'" % temp_file )

        g.plot_current()

        image = gdalnumeric.LoadFile( temp_file )
        image_ds = gdalnumeric.OpenArray( image )

        try:
            os.unlink( temp_file )
        except:
            pass

        rlayer = gview.GvRasterLayer( gview.GvRaster(dataset=image_ds, real=1),
                                      rl_mode = gview.RLM_RGBA )

        rlayer.set_source(1, gview.GvRaster(dataset=image_ds, real=2) )
        rlayer.set_source(2, gview.GvRaster(dataset=image_ds, real=3) )

        if terminal == 'rasterlayer':
            return rlayer

        graphwin = GvGraphWindow( rlayer )
        if wintitle is not None:
            graphwin.set_title(wintitle)




###############################################################################
# llplot - low level plot object used to manage plot state, and pipe handling.
#
# Only utilized from within gvplot.py.

class llplot:

    def __init__(self):
        self.cmds = []
        self.batch = 1
        self.data = None
        self.data_title = ''
        self.pipe = None
        self.tmpnam = None
        self.base_command = self.find_gnuplot()
        if self.base_command is None:
            raise RuntimeError, "Unable to find gnuplot executable."
        self.dimension = 2

    def find_gnuplot( self ):
        import gview
        exe = gview.get_preference('gnuplot')
        if exe is not None:
            if os.path.isfile(exe):
                return exe
            else:
                gvutils.warning( 'Disregarding gnuplot preference "%s", executable not found.' % exe )

        exe = gvutils.FindExecutable( 'gnuplot' )
        if exe is None:
            exe = gvutils.FindExecutable( 'pgnuplot.exe' )
        if exe is None:
            exe = gvutils.FindExecutable( 'wgnupl32.exe' )
        if exe is None:
            exe = gvutils.FindExecutable( 'wgnuplot.exe' )

        return exe

    def add_cmd( self, command ):
        self.cmds.append( command )

    def set_data( self, data, data_title = '', dimension = 2, xlen = 1,
                  multiplot = False, multilabels = (),multiopts =()):
        self.data = data
        self.data_title = data_title
        self.dimension = dimension
        self.xlen = xlen
        self.multiplot = multiplot
        self.multilabels = multilabels
        self.multiopts = multiopts

        if (multiplot == True) and (len(multilabels) == 0):
            self.multilabels = range(self.data.shape[1]-1)

        if (multiplot == True) and (len(multiopts) == 0):
            self.multiopts=[]
            for i in range(self.data.shape[1]-1):
                self.multiopts.append("")


    def plot_current( self ):
        """Data is a list of (x,y) pairs"""

        if self.batch:
            self.open_tmpfile()
        else:
            self.open_pipe()

        for cmd in self.cmds:
            self.write(cmd + '\n')

        if self.multiplot == False:
            if self.dimension == 2:
                self.write( 'plot "-" title "%s"\n' % self.data_title )
            else:
                self.write( 'splot "-" title "%s"\n' % self.data_title )

        else:
            cmd = '"-" title "' + str(self.multilabels[0]) + '" ' + self.multiopts[0]
            for i in range(self.data.shape[1]-2):
                cmd = cmd + ',"-" title "' + str(self.multilabels[i+1]) + '" ' + self.multiopts[i+1]
            #print "cmd", cmd    
            self.write( 'plot ' + cmd  +'\n')

        self.write_data()

        self.complete_command()

        self.cleanup()

    def write_data( self ):
        if self.multiplot == False:
            if self.dimension == 2:
                for pnt in self.data:
                    self.write( '%s %s\n' % (pnt[0], pnt[1]) )
                self.write('e\n')
            else:
                i = 0
                for pnt in self.data:
                    self.write( '%s %s %s\n' % (pnt[0], pnt[1], pnt[2]) )
                    i = i + 1
                    if i % self.xlen == 0:
                        self.write( '\n' )

                self.write('e\n')
        else:
            for i in range(self.data.shape[1]-1):
                for pnt in self.data:
                    self.write( '%s %s\n' % (pnt[0], pnt[1+i]) )
                self.write('e\n')    


    def open_pipe( self ):
        import os

        self.pipe = os.popen(self.base_command, 'w')

        # forward write and flush methods:
        self.write = self.pipe.write
        self.flush = self.pipe.flush

    def open_tmpfile( self ):
        import os

        self.tmpnam = gvutils.tempnam()
        self.pipe = open(self.tmpnam, 'w')

        # forward write and flush methods:
        self.write = self.pipe.write
        self.flush = self.pipe.flush

    def complete_command( self ):
        if self.pipe is not None:
            self.pipe.flush()

        if self.tmpnam is not None:
            self.pipe.close()
            self.pipe = None

            # command = self.base_command + ' < ' + self.tmpnam
            command = self.base_command + ' ' + self.tmpnam

            os.system( command )

    def cleanup( self ):
        # Closing the pipe makes the graphs
        # disappear immediately, so leave out
        # pipe cleanup in gnuplot case.
        if ((self.batch == 1) and (self.pipe is not None)):
            self.pipe.close()
            self.pipe = None

        if self.tmpnam is not None:
            os.unlink( self.tmpnam )

###############################################################################
# GvGraphWindow -- a very simple window for display graph images in.

class GvGraphWindow(gtk.Window):

    def __init__(self,rlayer):

        gtk.Window.__init__(self)

        import gview

        self.rlayer = rlayer
        raster = rlayer.get_data()
        self.xsize = raster.get_dataset().RasterXSize
        self.ysize = raster.get_dataset().RasterYSize

        self.set_resizable(True)
        # self.set_size_request(self.xsize, self.ysize)
        self.viewarea = gview.GvViewArea()
        self.viewarea.add_layer( self.rlayer )
        self.viewarea.size(self.xsize, self.ysize)
        shell = gtk.VBox(spacing=0)
        self.add(shell)

        # Print menu
        menuf = gvutils.GvMenuFactory()
        self.menuf = menuf
        menuf.add_entries([
                 ('File/Print', None, self.print_cb)])

        shell.pack_start(menuf, expand=False)

        shell.pack_start( self.viewarea )

        self.connect( 'delete-event', self.close )
        self.show_all()

        self.viewarea.fit_extents(0, self.ysize, self.xsize, -self.ysize )

    def print_cb(self, *args):
        import gvprint
        pd = gvprint.GvPrintDialog( self.viewarea )

    def close( self, *args ):
        self.rlayer = None
        self.destroy()


###############################################################################

if __name__ == '__main__':
    data1 = [[1,2], [1.1,3], [1.2,4]]
    plot( data1, xmin = -1, xmax = 5, terminal='gnuplot' )

    data2 = [1,2,5,4,3,3]
    plot( data2, xaxis = 'X', yaxis = 'power', title='Power Cycle',
          ymin = 0, ymax = 6, terminal='gnuplot' )

    data3 = [[1,2,3,4,5],[2,2.5,3,3.5,4],[3,3,3,3,3],[4,3.5,3,2.5,2],[5,4,3,2,1]]
    plot3d( data3, xaxis = 'X', yaxis = 'Y', zaxis = 'F(X,Y)', title='A Plot',
            terminal = 'gnuplot' )

    xvals=[4,5,6,7,8]
    yvals=[12,13,14,15,16]
    plot3d( data3, xaxis = 'X', yaxis = 'Y', zaxis = 'F(X,Y)', title='A Plot',
            terminal = 'gnuplot' , zmin=-1, zmax=5, xvec=xvals, yvec=yvals,
            cmds = ('set view 70,40','set grid','set contour base') )

    xvals=[4,5,6,7]
    yvals=[12,11,14,13,16]
    plot3d( data3, xaxis = 'X', yaxis = 'Y', zaxis = 'F(X,Y)', title='A Plot',
            terminal = 'gnuplot' , zmin=-1, zmax=5, xvec=xvals, yvec=yvals,
            cmds = ('set view 70,40','set grid','set contour base') )




