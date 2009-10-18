#! /usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  GvViewArea-based plotting.
# Author:   Gillian Walter <gillian.walter@atlantis-scientific.com>
#
# Developed by Atlantis Scientific Inc. (www.atlantis-scientific.com) for
# DRDC Ottawa
#
###############################################################################
# Copyright (c) Her majesty the Queen in right of Canada as represented
# by the Minister of National Defence, 2003.
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

# - For 2D or 3D plotting
#
# - Layers:
#       - data
#       - xlabel/ylabel/title (each is movable)
#       - legend
#       - Grid/tics/axis (optional grid, display of x/y values, etc.)
#
# Problems: 3D legend? Might want to have ability to adjust the 3D plot
#           without legend, "sink" it to a raster (using print), then
#           put the legend layer on top.
#
# Eventually: contours, mesh, etc.

#######################################
# IMMEDIATE TO-DO:
# 1) Fix up plot object/view area/window relationships and decide
#    on appropriate API
#
# 2) Labels/Axis- should be able to reset without discarding when
#                 pxmin/pxmax or borders reset.  Labels need not
#                 be updated when data max/mins are reset, though
#                 axis must be (discontinuities, spacing).


#
# NOTE: To avoid roundoff errors in getting data from interactive plots,
#       make sure openev is compiled with GV_USE_DOUBLE_PRECISION_COORD
#       (otherwise rounding could lead to points that are on the
#       plot boundary not being shown)
#
# LATER TO-DO:
#
# 1) Line symbols at nodes
#

import gview
import numpy
import gtk
import pgumenu
import os
import gvogrfs
from gvsignaler import Signaler

#########################################################################
# Class hierarchy (not completely implemented yet)
#
# Classes meant to be used externally:
#
# GvPlotWindow: window to store a GvSimplePlot, GvMPlotTable, or GvMPlotLayout
# GvPlot: a GvViewArea that stores one or more plot class instances, and.
#         provides an interface for using the internal plot classes.
#         Each plot class instance is assigned a set of extents within
#         the GvPlot which it uses in creating its layers.
#         GvSimplePlot is a simple implementation of this class for the
#         2D cartesian case.  The general case needs more thought.
# GvMPlotTable: A gtk table containing multiple GvPlot instances
# GvMPlotLayout: A gtk layout containing multiple GvPlot instances
#
# The GvMPlotTable/GvMPlotLayout classes should only be required if the
# user wishes to have 2-D and 3-D plots side-by-side or embedded in
# each other, or for displaying multiple 3-D plots (so that zooming/panning
# rotation can be done independently in each 3-D plot).  They have not
# been implemented yet (3-D hasn't been either).
#
# Classes/functions meant to be used internally:
#
# plot classes:
# 
# gvplot_2Ddata_cartesian- class for doing 2-D cartesian plots.  Axis can
#                          be linear or log, and can contain one or more
#                          discontinuities.  More than one array of data
#                          can be plotted.
#
# Each plot class is characterized by one or more data arrays, one
# set of axis (top/left/bottom/right), one set of data and view extents and
# function for converting from data coordinates to view coordinates, one
# set of labels, one border layer (2 area shapes: a background for the border
# and a background for the data frame), and one legend.
#
# If data is to be plotted using two separate ranges and two sets of
# axis are required (eg. velocity over top of acceleration), use two
# plot classes within the same GvPlot using the same plot extents, and don't
# add a border layer to the top plot.
#
# supporting classes used by plot classes (internal):
#
# gvplot_array_cartesian- store arrays after getting rid of inf's and nan's,
#                         and store min/max values of the arrays.
#
# gvplot_layer_defaults- store layer legend labels (also used in the layer
#                        name) and properties.
#
# gvplot_axis- store axis extents and display, tic information.
#
# functions for creating layers:
# Create2DBorderLayer
# CreateArrayDataLayer, CreateGridDataLayer
# CreateAxisLayer
# CreateLabelLayer
# CreateLegendLayer
#
# Layers that form a single plot, created by plot classes (from bottom up):
# Border layer
# Data layer(s)- one or more
# Axis layer
# Label layer
# Legend layer

GVPLOT_DATA_LAYER='Data' # plotted data
GVPLOT_LABEL_LAYER='Labels' # x/y/z labels, title, other annotation
GVPLOT_LEGEND_LAYER='Legend' # Legend
GVPLOT_AXIS_LAYER='Axis' # Axis lines and Axis tic-marks/values
GVPLOT_BORDER_LAYER='Border' # Backdrop to plot and border

# Recognized types of axis
GVPLOT_AXISTYPE_LINEAR='Linear'
GVPLOT_AXISTYPE_LOG='Log'

# Axis display options
GVPLOT_AXIS_NOTSHOWN=0  # Not displayed
GVPLOT_AXIS_NOTICS=1    # Show axis, but no tics
GVPLOT_AXIS_VTICS=2     # Show only major tics, without labels
GVPLOT_AXIS_VTICSLABELS=3 # Show only major tics, with labels
GVPLOT_AXIS_VTICSLABELSTICS=4 # Show major tics with labels and minor tics

DEFAULT_EDGE_PADDING=0.15


if os.name == 'nt':
    #DEFAULT_FONT="-adobe-helvetica-medium-r-*-*-15-*-*-*-*-*-*-*"
    DEFAULT_FONT="Sans 12"
else:
    #DEFAULT_FONT="-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"
    DEFAULT_FONT="Sans 12"




# plot classes

class gvplot_2Ddata_cartesian:
    """ Store an accumulation of _gvplot_arraydata instances,
        along with plot-window specific information.
    """
    def __init__(self,plot_label,pxmin=0.0,pymin=0.0,pxmax=1.0,pymax=1.0):
        """ plot_label- text label to prepend to layer names for this plot
            pxmin/pxmax/pymin/pymax- extents that plot should cover in
                                     main view window.
        """
        ###############################################################
        # Properties that will be common to other 2-D plot classes    #
        ###############################################################

        # Data: one or more gvplot_array_cartesian objects to plot.
        #       Each will be plotted as a separate line layer.  Each item
        #       in array_layer_defaults stores the label (for legend)
        #       and layer properties of the corresponding array_data
        #       item (eg. '_line_width', '_line_color','_gv_ogrfs_line',
        #       '_gl_antialias')
        self.array_data=[]      # Data to plot
        self.array_layer_defaults=[]  # Data plotting properties


        # Axis: zero or more axis/lines to create.  These are all put
        #       together in a single layer.  Layer-wide properties
        #       are stored in axis_layer_properties- use this to
        #       set the colours/widths of all axis/lines at once.
        #       Individual axis properties (start/end points and
        #       discontinuity locations in plot coordinates, locations
        #       of major and minor tics also in plot coordinates,
        #       labels for major tics, formats for labels, colours
        #       if desired) are stored in axis_data.  Individual
        #       properties override layer-wide properties.
        #       By default, 4 keys are created: top, bottom, left,
        #       and right.  More can be added.
        self.axis_data={}
        self.axis_data['top']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['bottom']=gvplot_axis(GVPLOT_AXIS_VTICSLABELS)
        self.axis_data['bottom'].vtics_properties['_gv_ogrfs']=\
            'LABEL(c:#000000FF,'+\
            'f:"'+DEFAULT_FONT+'",t:{label},a:0.0,s:1.0,dx:-10.0,dy:15.0);'+\
            'SYMBOL(c:#000000FF,id:ogr-sym-10,a:0.0,s:1.0)'
        self.axis_data['left']=gvplot_axis(GVPLOT_AXIS_VTICSLABELS)
        self.axis_data['left'].vtics_properties['_gv_ogrfs']=\
            'LABEL(c:#000000FF,'+\
            'f:"'+DEFAULT_FONT+'",t:{label},a:0.0,s:1.0,dx:-40.0,dy:5.0);'+\
            'SYMBOL(c:#000000FF,id:ogr-sym-10,a:270.0,s:1.0)'
        self.axis_data['right']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_layer_properties={}

        # axis defaults
        self.axis_layer_properties['_gl_antialias']='1'
        self.axis_layer_properties['_line_color']='0.0 0.0 0.0 1.0'

        # Extents that plot covers in main view area (plot coordinates)
        self.pxmin=pxmin
        self.pxmax=pxmax
        self.pymin=pymin
        self.pymax=pymax
        self.plot_label=plot_label

        # left/right/top/bottom border padding
        self.lborder=DEFAULT_EDGE_PADDING
        self.rborder=DEFAULT_EDGE_PADDING
        self.tborder=DEFAULT_EDGE_PADDING
        self.bborder=DEFAULT_EDGE_PADDING

        # border backgrounds (inner- behind plotted data;
        # outer- around edge of data).  Border layer is only
        # created if show_inner_border or show_outer_border
        # is set to 1.
        self.show_inner_border=0
        self.show_outer_border=0
        self.inner_border_props={}
        self.outer_border_props={}
        self.outer_border_props['_area_edge_color']='1.0 1.0 1.0 1.0'
        self.outer_border_props['_area_edge_width']='1.0'
        self.outer_border_props['_area_fill_color']='1.0 1.0 1.0 1.0'
        self.inner_border_props['_area_edge_color']='1.0 1.0 1.0 1.0'
        self.inner_border_props['_area_edge_width']='1.0'
        self.inner_border_props['_area_fill_color']='1.0 1.0 1.0 1.0'


        # Label parameters:
        #     label_data- a dictionary containing several lists, each
        #                 with an (x,y) tuple of plot coordinates, and
        #                 a text string or None.
        #     properties- properties to set on the layer (usually just
        #                 contains a key/value pair for _gv_ogrfs,
        #                 which contains display info).  Text for labels
        #                 is taken from the 'label' property set for
        #                 the shape.
        self.label_data={}
        self.label_layer_properties={}

        # defaults
        self.label_layer_properties['_gv_ogrfs_point']=\
             'LABEL(c:#000000FF,f:"'+DEFAULT_FONT+'",t:{label})'

        x0=self.pxmin+(self.lborder/5.0)*(self.pxmax-self.pxmin)
        xm=(self.pxmax+self.pxmin)/2.0+self.lborder-self.rborder
        xf=self.pxmax-(self.rborder/5.0)*(self.pxmax-self.pxmin)
        y0=self.pymin+(self.bborder/4.0)*(self.pymax-self.pymin)
        ym=(self.pymax+self.pymin)/2.0+self.bborder-self.tborder
        yf=self.pymax-(self.tborder/2.0)*(self.pymax-self.pymin)

        self.label_data['xlabel']=[(xm,y0,0),'xlabel']
        self.label_data['ylabel']=[(x0,ym,0),None]
        self.label_data['title']=[(xm,yf,0),None]

        # Legend parameters:
        #    show_legend- 1 if legend should be created; 0 otherwise
        #    location- an x,y tuple that specifies where the top left
        #              corner of the legend should fall as a fraction
        #              of pxmax-pxmin, pymax-pymin (plot extents):
        #              each value should be in the range 0-1.

        self.show_legend=0
        self.legend_location=None


        ########################
        # Specific properties. #
        ########################

        # Each min/max pair corresponds to
        # a continous range in the plot axis
        # NOTE: In the log axis case, these still
        # correspond to the non-logged min/max's;
        # logarithms are taken as the data layer
        # created- ie. for a log plot that ranges
        # from 10^0 to 10^2, the values here would
        # be 1 and 100.
        self.xmins=[]
        self.xmaxs=[]
        self.xloc=None   # Major xtic locations (data coordinates)
        self.xfmt="%g"   # Formatting string for tic labels

        self.ymins=[]
        self.ymaxs=[]
        self.yloc=None   # Major ytic locations (data coordinates)
        self.yfmt="%g"

        self.xaxistype=GVPLOT_AXISTYPE_LINEAR
        self.yaxistype=GVPLOT_AXISTYPE_LINEAR

        #################################################
        # Layers
        #################################################
        self.BorderLayer=None
        self.DataLayers=[]
        self.AxisLayer=None
        self.LabelLayer=None
        self.LegendLayer=None

    def create_layers(self):
        """ Create layers """
        self.BorderLayer=Create2DBorderLayer(self)
        if self.BorderLayer is not None:
            ll=[self.BorderLayer]
        else:
            ll=[]

        self.DataLayers=[]    
        for idx in range(len(self.array_data)):
            self.DataLayers.append(CreateArrayDataLayer(self,idx))
        ll.extend(self.DataLayers)

        self.AxisLayer=CreateAxisLayer(self)
        if ll is not None:
            ll.append(self.AxisLayer)

        self.LabelLayer=CreateLabelLayer(self)
        if self.LabelLayer is not None:
            ll.append(self.LabelLayer)

        self.LegendLayer=Create2DLegendLayer(self)
        if self.LegendLayer is not None:
            ll.append(self.LegendLayer)

        return ll

    def get_layer(self,ltype=GVPLOT_DATA_LAYER,idx=0):
        """ Get the layer of type ltype.  If ltype is 'Data',
            index idx is also specified to indicate which
            data layer to return.  If layer is not present,
            return None.
        """
        if ltype is GVPLOT_DATA_LAYER:
            if len(self.DataLayers) > idx:
                return self.DataLayers[idx]
            else:
                return None
        elif ltype is GVPLOT_AXIS_LAYER:
            return self.AxisLayer
        elif ltype is GVPLOT_BORDER_LAYER:
            return self.BorderLayer
        elif ltype is GVPLOT_LABEL_LAYER:
            return self.LabelLayer
        elif ltype is GVPLOT_LEGEND_LAYER:
            return self.LegendLayer
        else:
            raise RuntimeError,'get_layer: Invalid layer type'

    def create_layer(self,ltype=GVPLOT_DATA_LAYER,idx=0):
        """ Create or update the layer of type ltype.  If ltype is 'Data',
            index idx is also specified to indicate which
            data layer to return.  If layer is not present,
            return None.
        """
        if ltype is GVPLOT_DATA_LAYER:
            if len(self.DataLayers) > idx:
                self.DataLayers[idx]=CreateArrayDataLayer(self,idx)
            elif len(self.DataLayers) == idx:
                self.DataLayers.append(CreateArrayDataLayer(self,idx))
            else:
                raise RuntimeError,'create_layer: invalid data layer index'
        elif ltype is GVPLOT_AXIS_LAYER:
            self.AxisLayer=CreateAxisLayer(self)
            return self.AxisLayer
        elif ltype is GVPLOT_BORDER_LAYER:
            self.BorderLayer=Create2DBorderLayer(self)
            return self.BorderLayer
        elif ltype is GVPLOT_LABEL_LAYER:
            self.LabelLayer=CreateLabelLayer(self)
            return self.LabelLayer
        elif ltype is GVPLOT_LEGEND_LAYER:
            self.LegendLayer=Create2DLegendLayer(self)
            return self.LegendLayer
        else:
            raise RuntimeError,'create_layer: Invalid layer type'



    def get_layers(self):
        """ Return all layers that have been created, but don't
            create new ones.
        """
        if self.BorderLayer is not None:
            ll=[self.BorderLayer]
        else:
            ll=[]

        ll.extend(self.DataLayers)
        if self.AxisLayer is not None:
            ll.append(self.AxisLayer)

        if self.LabelLayer is not None:
            ll.append(self.LabelLayer)

        if self.LegendLayer is not None:
            ll.append(self.LegendLayer)

        return ll

    def update_data_from_plot(self,index):
        """ Update the underlying array data based on the
            modifications to the current plot.  This will
            discard any data outside of the viewing
            ranges.
        """
        newx,newy=self.get_data_from_plot(index)
        self.array_data[index]=gvplot_array_cartesian(newx,newy)

    def merge_data_from_plot(self,index):
        """ Update the underlying array data based on the
            modifications to the current plot.  This will
            try to merge the plot with the underlying data
            (data from the original array that lies outside
            the current viewing range is included).
        """
        newx,newy=self.get_data_from_plot(index)
        # find data that was out of range and not plotted
        xold=self.array_data[index].xarr
        yold=self.array_data[index].yarr
        outx,outy,outz=GetOutlierData(xold,self.xmins,self.xmaxs,
                                      yold,self.ymins,self.ymaxs)
        newx.extend(outx)
        newy.extend(outy)
        newx2,newy2=MakeContiguousXY(newx,newy)
        self.array_data[index]=gvplot_array_cartesian(newx2,newy2)



    def get_data_from_plot(self,index):
        """ Get (potentially modified) data from the the data layer
            corresponding to array_data[index].
        """
        if index > len(self.DataLayers)-1:
            return None

        l=self.DataLayers[index]
        shps=l.get_parent()
        xlists=[]
        ylists=[]
        arrxlists=[]
        arrylists=[]
        idx=0
        for shp in shps:
            xlists.append([])
            ylists.append([])
            for cnodeidx in range(shp.get_nodes()):
                cnode=shp.get_node(cnodeidx)
                xlists[idx].append(cnode[0])
                ylists[idx].append(cnode[1])
            xdata,ydata,ok=self.get_xyposition(numpy.array(xlists[idx]),
                                               numpy.array(ylists[idx]))
            arrxlists.append(xdata)
            arrylists.append(ydata)
            idx=idx+1

        return (arrxlists,arrylists)

    def set_extents(self,xmin=None,xmax=None,xspc=None,nxtics=0,
                    ymin=None,ymax=None,yspc=None,nytics=0,nice=1):
        """ Set data extents for plot.  In each case below, entering
            a value of None indicates that the plot should decide on
            sensible min/max's.  If xmin is an array, xmax must be
            an array of the same length, and vice versa (same for
            ymin/ymax).  Each xmin/xmax pair represents one continuous
            range of values to plot.

            xmin- minimum in x direction (None, single value, or array)
            ymin- minimum in y direction (None, single value, or array)
            xmax- maximum in x direction (None, single value, or array)
            ymax- maximum in y direction (None, single value, or array)
            xspc- approximate tic spacing in x direction (None or single value)
            yspc- approximate tic spacing in y direction (None or single value)
            nxtics- number of minor x tics per major x tic (defaults to 0)
            nytics- number of minor y tics per major y tic (defaults to 0)
            nice- 1 if plot is allowed to alter entered values slightly
                  to make them look 'nice'- eg. 0.015999 can be rounded
                  to 0.16, etc.; 0 if plot must not alter the values.
                  Defaults to 1.
            """

        if len(self.array_data) > 0:
            if xmin is None:
                xmin=self.array_data[0].xmin
                for item in self.array_data[1:]:
                    xmin=min([xmin,item.xmin])

            if xmax is None:
                xmax=self.array_data[0].xmax
                for item in self.array_data[1:]:
                    xmax=max([xmax,item.xmax])

            if ymin is None:         
                ymin=self.array_data[0].ymin
                for item in self.array_data[1:]:
                    ymin=min([ymin,item.ymin])

            if ymax is None:     
                ymax=self.array_data[0].ymax
                for item in self.array_data[1:]:
                    ymax=max([ymax,item.ymax])

            if nice == 1:
                dxmin,dxmax,dxlocs,fmt=GetNiceMinMax(xmin,xmax,xspc,
                                                     self.xaxistype)
                self.xmins=dxmin
                self.xmaxs=dxmax
                self.xlocs=dxlocs
                self.xfmt=fmt

                dymin,dymax,dylocs,fmt=GetNiceMinMax(ymin,ymax,yspc,
                                                     self.yaxistype)
                self.ymins=dymin
                self.ymaxs=dymax
                self.ylocs=dylocs
                self.yfmt=fmt
            else:
                dxmin,dxmax,dxlocs,fmt=GetNiceMinMax(xmin,xmax,xspc,
                                                     self.xaxistype)
                if type(xmin) not in [type([]),type((1,))]:
                    self.xmins=[xmin]
                else:
                    self.xmins=xmin
                if type(xmax) not in [type([]),type((1,))]:
                    self.xmaxs=[xmax]
                else:
                    self.xmaxs=xmax
                self.xlocs=dxlocs
                self.xfmt=fmt

                dymin,dymax,dyspc,fmt=GetNiceMinMax(ymin,ymax,yspc,
                                                    self.yaxistype)
                if type(ymin) not in [type([]),type((1,))]:
                    self.ymins=[ymin]
                else:
                    self.ymins=ymin

                if type(ymax) not in [type([]),type((1,))]:
                    self.ymaxs=[ymax]
                else:
                    self.ymaxs=ymax

                self.ylocs=dylocs
                self.yfmt=fmt
        else:
            if ((xmin is None) or (xmax is None) or (ymin is None) or
                (ymax is None)):
                raise RuntimeError,'set_extents: if no data has been set,'+\
                      'then\nxmin/xmax/ymin/ymax must all be specified.'

            if nice == 1:
                dxmin,dxmax,dxlocs,fmt=GetNiceMinMax(xmin,xmax,xspc,
                                                     self.xaxistype)
                self.xmins=dxmin
                self.xmaxs=dxmax
                self.xlocs=dxlocs
                self.xfmt=fmt

                dymin,dymax,dylocs,fmt=GetNiceMinMax(ymin,ymax,yspc,
                                                     self.yaxistype)
                self.ymins=dymin
                self.ymaxs=dymax
                self.ylocs=dylocs
                self.yfmt=fmt
            else:
                dxmin,dxmax,dxlocs,fmt=GetNiceMinMax(xmin,xmax,xspc,
                                                     self.xaxistype)
                # TO DO: ADD A CHECK HERE AND MAKE SURE THEY ARE LISTS!
                self.xmins=xmin
                self.xmaxs=xmax
                self.xlocs=dxlocs
                self.xfmt=fmt

                dymin,dymax,dyspc,fmt=GetNiceMinMax(ymin,ymax,yspc,
                                                    self.yaxistype)
                self.ymins=list(dymin)
                self.ymaxs=list(dymax)
                self.ylocs=list(dylocs)
                self.yfmt=fmt

        self.set_axis()

    def set_axis(self):
        """ Set the default axis """

        new_axis_data={}
        xbreaks=GetAxisBreaks(self.xmins,self.xmaxs,self.xaxistype)
        ybreaks=GetAxisBreaks(self.ymins,self.ymaxs,self.yaxistype)
        ext=self.get_plot_extents(include_border=0)
        pxlocs,xok=DataToPlot1D(ext[0],ext[3],self.xmins,self.xmaxs,
                            self.xlocs,self.xaxistype)
        pylocs,yok=DataToPlot1D(ext[1],ext[4],self.ymins,self.ymaxs,
                            self.ylocs,self.yaxistype)

        toplocs=[]
        bottomlocs=[]
        leftlocs=[]
        rightlocs=[]
        ext=self.get_plot_extents(include_border=0)

        for item in pxlocs:
            toplocs.append((item,ext[4]))
            bottomlocs.append((item,ext[1]))
        for item in pylocs:
            leftlocs.append((ext[0],item))
            rightlocs.append((ext[3],item))

        # TO DO: add minor tics    
        new_axis_data['top']=self.axis_data['top']
        new_axis_data['top'].set_data(toplocs,list(self.xlocs),self.xfmt,
                                      None,
                                      (ext[0],ext[4],0),
                                      (ext[3],ext[4],0),
                                      xbreaks)
        new_axis_data['bottom']=self.axis_data['bottom']
        new_axis_data['bottom'].set_data(bottomlocs,list(self.xlocs),self.xfmt,
                                      None,
                                      (ext[0],ext[1],0),
                                      (ext[3],ext[1],0),
                                      xbreaks)
        new_axis_data['left']=self.axis_data['left']
        new_axis_data['left'].set_data(leftlocs,list(self.ylocs),self.yfmt,
                                      None,
                                      (ext[0],ext[1],0),
                                      (ext[0],ext[4],0),
                                      ybreaks)
        new_axis_data['right']=self.axis_data['right']
        new_axis_data['right'].set_data(rightlocs,list(self.ylocs),self.yfmt,
                                      None,
                                      (ext[3],ext[1],0),
                                      (ext[3],ext[4],0),
                                      ybreaks)

        self.axis_data=new_axis_data

    #########################################################################
    # Versions of the following functions will be required of most or       #
    # all plot classes.                                                     #
    #########################################################################

    def add_data(self,xarr,yarr,label='',properties=None, interactive=0):
        """ Add an array to the data to plot """
        self.array_data.append(gvplot_array_cartesian(xarr,yarr))
        self.array_layer_defaults.append(
            gvplot_layer_defaults(label,properties,interactive))
        return len(self.array_data)-1

    def set_labels(self,xlabel=None,ylabel=None,title=None):
        """ Set labels. """
        self.label_data['xlabel'][1]=xlabel
        self.label_data['ylabel'][1]=ylabel
        self.label_data['title'][1]=title

    def set_plot_extents(self,pxmin,pxmax,pymin,pymax):
        """ Set/Reset the extents covered by this plot in the view.
            Update labels, axis, data info.
        """

        oldpxmin=self.pxmin
        oldpxmax=self.pxmax
        oldpymin=self.pymin
        oldpymax=self.pymax

        self.pxmin=pxmin
        self.pxmax=pxmax
        self.pymin=pymin
        self.pymax=pymax

        # Update label positions
        for ckey in self.label_data.keys():
            xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.label_data[ckey][0][0],
                              GVPLOT_AXISTYPE_LINEAR)
            ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.label_data[ckey][0][1],
                              GVPLOT_AXISTYPE_LINEAR)
            self.label_data[ckey][0]=(xnew,ynew,0)

        # Update axis information            
        for ckey in self.axis_data.keys():
            for idx in range(len(self.axis_data[key].vtics)):
                xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.axis_data[ckey].vtics[idx][0],
                              GVPLOT_AXISTYPE_LINEAR)
                ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.axis_data[ckey].vtics[idx][1],
                              GVPLOT_AXISTYPE_LINEAR)
                self.axis_data[ckey].vtics[idx]=(xnew,ynew,0)

            for idx in range(len(self.axis_data[key].tics)):
                xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.axis_data[ckey].tics[idx][0],
                              GVPLOT_AXISTYPE_LINEAR)
                ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.axis_data[ckey].tics[idx][1],
                              GVPLOT_AXISTYPE_LINEAR)
                self.axis_data[ckey].tics[idx]=(xnew,ynew,0)

            if self.axis_data[ckey].start is not None:
                xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.axis_data[ckey].start[0],
                              GVPLOT_AXISTYPE_LINEAR)
                ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.axis_data[ckey].start[1],
                              GVPLOT_AXISTYPE_LINEAR)
                self.axis_data[ckey].start=(xnew,ynew,0)

            if self.axis_data[ckey].end is not None:
                xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.axis_data[ckey].end[0],
                              GVPLOT_AXISTYPE_LINEAR)
                ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.axis_data[ckey].end[1],
                              GVPLOT_AXISTYPE_LINEAR)
                self.axis_data[ckey].end=(xnew,ynew,0)

            if self.axis_data[ckey].breaks is not None:
                for idx in range(len(breaks)):
                    xnew1=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                        self.axis_data[ckey].breaks[idx][0][0],
                        GVPLOT_AXISTYPE_LINEAR)
                    ynew1=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                        self.axis_data[ckey].breaks[idx][0][1],
                        GVPLOT_AXISTYPE_LINEAR)
                    xnew2=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                        self.axis_data[ckey].breaks[idx][1][0],
                        GVPLOT_AXISTYPE_LINEAR)
                    ynew2=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                        self.axis_data[ckey].breaks[idx][1][1],
                        GVPLOT_AXISTYPE_LINEAR)
                    self.axis_data[ckey].breaks[idx]=((xnew1,ynew1,0),
                                                      (xnew2,ynew2,0))

        if len(self.xmins) == 0:
            return

        self.reset_axis()

    def get_extents(self):
        """ Get data extents: (xmins, xmaxs, ymins, ymaxs, zmins, zmaxs). """
        return (self.xmins,self.xmaxs,self.ymins,
                self.ymaxs,[0],[0])

    def get_plot_extents(self,include_border=1):
        """ Get plot extents with (include_border=1) or
            without (include_border=0) borders.
            (xmin,ymin,zmin,xmax,ymax,zmax).  For
            this class, zmin=zmax=0 always.  The third
            dimension is included because the CreateAxisLayer
            function will be shared with the 3d case.
        """
        if include_border == 1:
            return (self.pxmin,self.pymin,0,self.pxmax,self.pymax,0)

        dxmin=self.pxmin+(self.pxmax-self.pxmin)*self.lborder
        dxmax=self.pxmax-(self.pxmax-self.pxmin)*self.rborder
        dymin=self.pymin+(self.pymax-self.pymin)*self.bborder
        dymax=self.pymax-(self.pymax-self.pymin)*self.tborder

        return (dxmin,dymin,0,dxmax,dymax,0)


    def get_xyposition(self,x,y):
        """ Get the data position corresponding to GvViewPlot position x,y.
            x and y may be single values or same-length 1-D arrays.
        """

        # Get plot extents minus borders.
        dxmin=self.pxmin+(self.pxmax-self.pxmin)*self.lborder
        dxmax=self.pxmax-(self.pxmax-self.pxmin)*self.rborder
        dymin=self.pymin+(self.pymax-self.pymin)*self.bborder
        dymax=self.pymax-(self.pymax-self.pymin)*self.tborder

        if type(x) == type(numpy.array([])):
            x=numpy.ravel(x)

        if type(y) == type(numpy.array([])):
            y=numpy.ravel(y)

        x,xok=PlotToData1D(dxmin,dxmax,self.xmins,self.xmaxs,x,self.xaxistype)
        y,yok=PlotToData1D(dymin,dymax,self.ymins,self.ymaxs,y,self.yaxistype)

        okarr=numpy.where(xok==0,0,yok)

        return (x,y,okarr)

    def get_plotposition(self,x,y):
        """ Get the GvViewPlot position corresponding to data position x,y.
            x and y may be single values or same-length 1-D arrays.
        """

        # Get plot extents minus borders.
        dxmin=self.pxmin+(self.pxmax-self.pxmin)*self.lborder
        dxmax=self.pxmax-(self.pxmax-self.pxmin)*self.rborder
        dymin=self.pymin+(self.pymax-self.pymin)*self.bborder
        dymax=self.pymax-(self.pymax-self.pymin)*self.tborder

        if type(x) == type(numpy.array([])):
            x=numpy.ravel(x)

        if type(y) == type(numpy.array([])):
            y=numpy.ravel(y)

        x,xok=DataToPlot1D(dxmin,dxmax,self.xmins,self.xmaxs,x,self.xaxistype)
        y,yok=DataToPlot1D(dymin,dymax,self.ymins,self.ymaxs,y,self.yaxistype)

        okarr=numpy.where(xok==0,0,yok)

        return (x,y,okarr)        

    def set_arraylayer_properties(self,idx,props):
        """ Set the display properties for the idxth data layer.
            idx- index of data
            props- a dictionary of properties to set (keys) and their
                   values.

            An error will be raised if there is no idx'th data array
            to plot.

            Example properties:
            _line_color='1.0 0.0 0.0 1.0'
            _line_width='1.0'
            _gl_antialias='1' (turn on display antialiasing)
        """
        self.array_layer_defaults[idx]=props

    def set_axislayer_properties(self,props):
        """ Set the display properties of the axis layer

            Example properties:
            _line_color='1.0 0.0 0.0 1.0'
            _line_width='1.0'
            _gl_antialias='1' (turn on display antialiasing)

        """
        self.axis_layer_properties=props

    def set_label_color(self,color):
        """ Set label layer color to color. """

        import gvogrfs
        cstr2=gvogrfs.gv_to_ogr_color(color)
        if len(cstr2) < 9:
            cstr2=cstr2+'FF'
        if self.label_layer_properties.has_key('_gv_ogrfs_point'):
            ogrfs=gvogrfs.OGRFeatureStyle(
                self.label_layer_properties['_gv_ogrfs_point'])
            if ogrfs.has_part('LABEL'):
                part=ogrfs.get_part('LABEL')
                part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                ogrfs.remove_part('LABEL')
                ogrfs.add_part(part)
            if ogrfs.has_part('SYMBOL'):
                part=ogrfs.get_part('SYMBOL')
                part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                ogrfs.remove_part('SYMBOL')
                ogrfs.add_part(part)
            self.label_layer_properties['_gv_ogrfs_point']=ogrfs.unparse()

    def set_axis_color(self,color):
        """ Set all axis and axis label colours to color.
            color is a tuple of 4 values, each between
            0 and 1, representing red/green/blue/alpha
            components.
        """
        cstr1=str(color[0])+' '+str(color[1])+' '+str(color[2])+\
               ' '+str(color[3])
        self.axis_layer_properties['_line_color']=cstr1

        import gvogrfs
        cstr2=gvogrfs.gv_to_ogr_color(color)
        if len(cstr2) < 9:
            cstr2=cstr2+'FF'
        for caxis in self.axis_data.values():
            caxis.line_properties['_line_color']=cstr1
            if caxis.vtics_properties.has_key('_gv_ogrfs'):
                ogrfs=gvogrfs.OGRFeatureStyle(
                    caxis.vtics_properties['_gv_ogrfs'])
                if ogrfs.has_part('LABEL'):
                    part=ogrfs.get_part('LABEL')
                    part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                    ogrfs.remove_part('LABEL')
                    ogrfs.add_part(part)
                if ogrfs.has_part('SYMBOL'):
                    part=ogrfs.get_part('SYMBOL')
                    part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                    ogrfs.remove_part('SYMBOL')
                    ogrfs.add_part(part)
                caxis.vtics_properties['_gv_ogrfs']=ogrfs.unparse()

            if caxis.tics_properties.has_key('_gv_ogrfs'):
                ogrfs=gvogrfs.OGRFeatureStyle(
                    caxis.tics_properties['_gv_ogrfs'])
                if ogrfs.has_part('LABEL'):
                    part=ogrfs.get_part('LABEL')
                    part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                    ogrfs.remove_part('LABEL')
                    ogrfs.add_part(part)
                if ogrfs.has_part('SYMBOL'):
                    part=ogrfs.get_part('SYMBOL')
                    part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                    ogrfs.remove_part('SYMBOL')
                    ogrfs.add_part(part)
                caxis.tics_properties['_gv_ogrfs']=ogrfs.unparse()


    def set_axis_properties(self,key,lprops,vprops,tprops):
        """ Set the properties of the shapes forming the axis
            referenced by 'key'.  Default keys are 'top, 'bottom',
            'left','right'.  An error will be raised if the axis
            dictionary doesn't contain key.

            Parameters:
                key- key to index axis
                lprops- line properties (if different from layer default)
                vprops- vtic properties
                tprops- tic properties

            Example properties: (defaults for left axis shown here)
            _gv_ogrfs='LABEL(c:#000000FF,'+\
            'f:"-adobe-helvetica-medium-r-*-*-15-*-*-*-*-*-*-*",
            t:{label},a:0.0,s:1.0,dx:-35.0,dy:5.0);'+\
            'SYMBOL(c:#000000FF,id:ogr-sym-10,a:270.0,s:1.0)'

            Note: the label property is always used to get text
                  in CreateAxisLayer, so the t:{label} portion
                  of this should be left alone.
        """

        self.axis_data[key].line_properties=lprops
        self.axis_data[key].vtics_properties=vprops
        self.axis_data[key].tics_properties=tprops


    def set_border_padding(self,top,bottom,left,right):
        """ Set border padding as a fraction of the plot
            extents (0-1).
            top- top padding
            bottom- bottom
            left- left
            right- right
        """
        self.tborder=top
        self.bborder=bottom
        self.lborder=left
        self.rborder=right

    def set_borderlayer_properties(self,show_outer,show_inner,
                                   outer_props,inner_props):
        """ Set border layer properties:
            show_outer- 1 to create outer border, 0 to not create it
            show_inner- 1 to create inner border, 0 to not create it
            outer_props- dictionary of outer border properties
                         (None to leave at defaults)
            inner_props- dictionary of inner border properties
                         (None to leave at defaults)

            Example properties:

            outer_props['_area_edge_color']='1.0 1.0 1.0 1.0'
            outer_props['_area_edge_width']='1.0'
            outer_props['_area_fill_color']='1.0 1.0 1.0 1.0'

        """
        self.show_outer_border=show_outer
        if outer_props is not None:
            self.outer_border_props=outer_props

        self.show_inner_border=show_inner
        if inner_props is not None:
            self.inner_border_props=inner_props

    def set_labellayer_properties(self,label_props):
        """ Set label layer properties:
            Example:
            _gv_ogrfs_point='LABEL(c:#000000FF,
            f:"-adobe-helvetica-medium-r-*-*-15-*-*-*-*-*-*-*",t:{label})'

            Note: the label property is always used to get text
                  in CreateLabelLayer, so the t:{label} portion
                  of this should be left alone.            
        """
        self.label_layer_properties=label_props

    def set_legend(self,show=1,location=None):
        """ Set legend status:
            show- 1 (show) or 0 (hide)
            location- offset as a fraction of
                      current plot extents
                      (xoffset,yoffset); if None,
                      default will be used.
        """
        self.show_legend=show
        if location is not None:
            self.legend_location=location
        else:
            self.legend_location=(self.lborder+0.05,
                                  1.0-self.tborder-0.1)

    def add_axis(self,key,properties,start_tuple,end_tuple,breaks=None,
                 type=GVPLOT_AXIS_NOTICS):
        """ Add an axis (line):
            key- key to reference the axis
            properties- axis properties
            start_tuple- an (x,y) tuple in data coordinates representing
                         the start point of the line.
            end_tuple- an (x,y) tuple in data coordinates representing
                       the start point of the line.
            breaks- breaks in the axis/line (defaults to None, must be
                    a list of start/end tuples if present).  Also in
                    data coordinates.
            type- type of axis (0-4: not shown, shown but no tics, shown
                  with major tics but no labels, shown with major tics
                  and labels, shown with major tics and labels and minor
                  tics).

        """
        pass


class gvplot_3Ddata_cartesiangrid:
    """ Store an accumulation of _gvplot_arraydata instances,
        along with plot-window specific information.
    """
    def __init__(self,plot_label,pxmin=0.0,pymin=0.0,pzmin=0.0,pxmax=1.0,
                 pymax=1.0,pzmax=1.0):
        """ plot_label- text label to prepend to layer names for this plot
            pxmin/pxmax/pymin/pymax/pzmin/pzmax- extents that plot should
            cover in main view window.
        """
        self.array_data=[]      # Data to plot
        self.array_layer_defaults=[]  # Data plotting properties


        # Axis: zero or more axis/lines to create.  These are all put
        #       together in a single layer.  Layer-wide properties
        #       are stored in axis_layer_properties- use this to
        #       set the colours/widths of all axis/lines at once.
        #       Individual axis properties (start/end points and
        #       discontinuity locations in plot coordinates, locations
        #       of major and minor tics also in plot coordinates,
        #       labels for major tics, formats for labels, colours
        #       if desired) are stored in axis_data.  Individual
        #       properties override layer-wide properties.
        #       By default, 4 keys are created: top, bottom, left,
        #       and right.  More can be added.
        self.axis_data={}
        self.axis_data['topleft']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['topright']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['topfront']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['topback']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['bottomleft']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['bottomright']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['bottomfront']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['bottomback']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['leftfront']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['leftback']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['rightfront']=gvplot_axis(GVPLOT_AXIS_NOTICS)
        self.axis_data['rightback']=gvplot_axis(GVPLOT_AXIS_NOTICS)

        # axis defaults
        self.axis_layer_properties={}
        self.axis_layer_properties['_gl_antialias']='1'
        self.axis_layer_properties['_line_color']='0.0 0.0 0.0 1.0'

        # Extents that plot covers in main view area (plot coordinates)
        self.pxmin=pxmin
        self.pxmax=pxmax
        self.pymin=pymin
        self.pymax=pymax
        self.pzmin=pzmin
        self.pzmax=pzmax
        self.plot_label=plot_label

        # left/right/top/bottom border padding
        self.lborder=DEFAULT_EDGE_PADDING
        self.rborder=DEFAULT_EDGE_PADDING
        self.tborder=DEFAULT_EDGE_PADDING
        self.bborder=DEFAULT_EDGE_PADDING
        self.fborder=DEFAULT_EDGE_PADDING
        self.bkborder=DEFAULT_EDGE_PADDING # back


        # Label parameters:
        #     label_data- a dictionary containing several lists, each
        #                 with an (x,y,z) tuple of plot coordinates, and
        #                 a text string or None.
        #     properties- properties to set on the layer (usually just
        #                 contains a key/value pair for _gv_ogrfs,
        #                 which contains display info).  Text for labels
        #                 is taken from the 'label' property set for
        #                 the shape.
        self.label_data={}
        self.label_layer_properties={}

        # defaults
        self.label_layer_properties['_gv_ogrfs_point']=\
             'LABEL(c:#000000FF,f:"'+DEFAULT_FONT+'",t:{label})'

        x0=self.pxmin+(self.lborder/5.0)*(self.pxmax-self.pxmin)
        xm=(self.pxmax+self.pxmin)/2.0+self.lborder-self.rborder
        xf=self.pxmax-(self.rborder/5.0)*(self.pxmax-self.pxmin)
        y0=self.pymin+(self.fborder/4.0)*(self.pymax-self.pymin)
        ym=(self.pymax+self.pymin)/2.0+self.fborder-self.bkborder
        yf=self.pymax-(self.bkborder/2.0)*(self.pymax-self.pymin)
        z0=self.pzmin+(self.bborder/4.0)*(self.pzmax-self.pzmin)
        zm=(self.pzmax+self.pzmin)/2.0+self.bborder-self.tborder
        zf=self.pzmax-(self.tborder/2.0)*(self.pzmax-self.pzmin)

        self.label_data['xlabel']=[(xm,y0,z0),None]
        self.label_data['ylabel']=[(x0,ym,z0),None]
        self.label_data['zlabel']=[(x0,y0,zm),None]
        self.label_data['title']=[(xm,ym,zf),None]


        ########################
        # Specific properties. #
        ########################

        # Each min/max pair corresponds to
        # a continous range in the plot axis
        # NOTE: In the log axis case, these still
        # correspond to the non-logged min/max's;
        # logarithms are taken as the data layer
        # created- ie. for a log plot that ranges
        # from 10^0 to 10^2, the values here would
        # be 1 and 100.
        self.xmins=[]
        self.xmaxs=[]
        self.xloc=None   # Major xtic locations (data coordinates)
        self.xfmt="%g"   # Formatting string for tic labels

        self.ymins=[]
        self.ymaxs=[]
        self.yloc=None   # Major ytic locations (data coordinates)
        self.yfmt="%g"

        self.zmins=[]
        self.zmaxs=[]
        self.zloc=None   # Major ztic locations (data coordinates)
        self.zfmt="%g"

        self.xaxistype=GVPLOT_AXISTYPE_LINEAR
        self.yaxistype=GVPLOT_AXISTYPE_LINEAR
        self.zaxistype=GVPLOT_AXISTYPE_LINEAR

        #################################################
        # Layers
        #################################################
        self.DataLayers=[]
        self.AxisLayer=None
        self.LabelLayer=None

    def create_layers(self):
        """ Create layers """

        ll=[]
        self.DataLayers=[]
        for idx in range(len(self.array_data)):
            self.DataLayers.append(CreateGridArrayDataLayer(self,idx))
        ll.extend(self.DataLayers)

        self.AxisLayer=CreateAxisLayer(self)
        if ll is not None:
            ll.append(self.AxisLayer)

        self.LabelLayer=CreateLabelLayer(self)
        if self.LabelLayer is not None:
            ll.append(self.LabelLayer)

        return ll

    def get_layer(self,ltype=GVPLOT_DATA_LAYER,idx=0):
        """ Get the layer of type ltype.  If ltype is 'Data',
            index idx is also specified to indicate which
            data layer to return.  If layer is not present,
            return None.
        """
        if ltype is GVPLOT_DATA_LAYER:
            if len(self.DataLayers) > idx:
                return self.DataLayers[idx]
            else:
                return None
        elif ltype is GVPLOT_AXIS_LAYER:
            return self.AxisLayer
        elif ltype is GVPLOT_LABEL_LAYER:
            return self.LabelLayer
        else:
            raise RuntimeError,'get_layer: Invalid layer type'

    def create_layer(self,ltype=GVPLOT_DATA_LAYER,idx=0):
        """ Create or update the layer of type ltype.  If ltype is 'Data',
            index idx is also specified to indicate which
            data layer to return.  If layer is not present,
            return None.
        """
        if ltype is GVPLOT_DATA_LAYER:
            if len(self.DataLayers) > idx:
                self.DataLayers[idx]=CreateGridArrayDataLayer(self,idx)
            elif len(self.DataLayers) == idx:
                self.DataLayers.append(CreateGridArrayDataLayer(self,idx))
            else:
                raise RuntimeError,'create_layer: invalid data layer index'
        elif ltype is GVPLOT_AXIS_LAYER:
            self.AxisLayer=CreateAxisLayer(self)
            return self.AxisLayer
        elif ltype is GVPLOT_LABEL_LAYER:
            self.LabelLayer=CreateLabelLayer(self)
            return self.LabelLayer
        else:
            raise RuntimeError,'create_layer: Invalid layer type'



    def get_layers(self):
        """ Return all layers that have been created, but don't
            create new ones.
        """

        ll.extend(self.DataLayers)
        if self.AxisLayer is not None:
            ll.append(self.AxisLayer)

        if self.LabelLayer is not None:
            ll.append(self.LabelLayer)

        return ll

    def set_extents(self,xmin=None,xmax=None,xspc=None,nxtics=0,
                    ymin=None,ymax=None,yspc=None,nytics=0,
                    zmin=None,zmax=None,zspc=None,nztics=0,nice=1):
        """ Set data extents for plot.  In each case below, entering
            a value of None indicates that the plot should decide on
            sensible min/max's.  If xmin is an array, xmax must be
            an array of the same length, and vice versa (same for
            ymin/ymax).  Each xmin/xmax pair represents one continuous
            range of values to plot.

            xmin- minimum in x direction (None, single value, or array)
            ymin- minimum in y direction (None, single value, or array)
            zmin- minimum in z direction (None, single value, or array)
            xmax- maximum in x direction (None, single value, or array)
            ymax- maximum in y direction (None, single value, or array)
            zmax- maximum in z direction (None, single value, or array)
            xspc- approximate tic spacing in x direction (None or single value)
            yspc- approximate tic spacing in y direction (None or single value)
            zspc- approximate tic spacing in y direction (None or single value)
            nxtics- number of minor x tics per major x tic (defaults to 0)
            nytics- number of minor y tics per major y tic (defaults to 0)
            nztics- number of minor y tics per major y tic (defaults to 0)
            nice- 1 if plot is allowed to alter entered values slightly
                  to make them look 'nice'- eg. 0.015999 can be rounded
                  to 0.16, etc.; 0 if plot must not alter the values.
                  Defaults to 1.
            """

        if len(self.array_data) > 0:
            if xmin is None:
                xmin=self.array_data[0].xmin
                for item in self.array_data[1:]:
                    xmin=min([xmin,item.xmin])

            if xmax is None:
                xmax=self.array_data[0].xmax
                for item in self.array_data[1:]:
                    xmax=max([xmax,item.xmax])

            if ymin is None:         
                ymin=self.array_data[0].ymin
                for item in self.array_data[1:]:
                    ymin=min([ymin,item.ymin])

            if ymax is None:     
                ymax=self.array_data[0].ymax
                for item in self.array_data[1:]:
                    ymax=max([ymax,item.ymax])

            if zmin is None:         
                zmin=self.array_data[0].zmin
                for item in self.array_data[1:]:
                    zmin=min([zmin,item.zmin])

            if zmax is None:     
                zmax=self.array_data[0].zmax
                for item in self.array_data[1:]:
                    zmax=max([zmax,item.zmax])

            if nice == 1:
                dxmin,dxmax,dxlocs,fmt=GetNiceMinMax(xmin,xmax,xspc,
                                                     self.xaxistype)
                self.xmins=dxmin
                self.xmaxs=dxmax
                self.xlocs=dxlocs
                self.xfmt=fmt

                dymin,dymax,dylocs,fmt=GetNiceMinMax(ymin,ymax,yspc,
                                                     self.yaxistype)
                self.ymins=dymin
                self.ymaxs=dymax
                self.ylocs=dylocs
                self.yfmt=fmt

                dzmin,dzmax,dzlocs,fmt=GetNiceMinMax(zmin,zmax,zspc,
                                                     self.zaxistype)
                self.zmins=dzmin
                self.zmaxs=dzmax
                self.zlocs=dzlocs
                self.zfmt=fmt
            else:
                dxmin,dxmax,dxlocs,fmt=GetNiceMinMax(xmin,xmax,xspc,
                                                     self.xaxistype)
                if type(xmin) not in [type([]),type((1,))]:
                    self.xmins=[xmin]
                else:
                    self.xmins=xmin
                if type(xmax) not in [type([]),type((1,))]:
                    self.xmaxs=[xmax]
                else:
                    self.xmaxs=xmax
                self.xlocs=dxlocs
                self.xfmt=fmt

                dymin,dymax,dyspc,fmt=GetNiceMinMax(ymin,ymax,yspc,
                                                    self.yaxistype)
                if type(ymin) not in [type([]),type((1,))]:
                    self.ymins=[ymin]
                else:
                    self.ymins=ymin

                if type(ymax) not in [type([]),type((1,))]:
                    self.ymaxs=[ymax]
                else:
                    self.ymaxs=ymax

                self.ylocs=dylocs
                self.yfmt=fmt

                dzmin,dzmax,dzspc,fmt=GetNiceMinMax(zmin,zmax,zspc,
                                                    self.zaxistype)
                if type(zmin) not in [type([]),type((1,))]:
                    self.zmins=[zmin]
                else:
                    self.zmins=zmin

                if type(zmax) not in [type([]),type((1,))]:
                    self.zmaxs=[zmax]
                else:
                    self.zmaxs=zmax

                self.zlocs=dzlocs
                self.zfmt=fmt                
        else:
            if ((xmin is None) or (xmax is None) or (ymin is None) or
                (ymax is None) or (zmin is None) or (zmax is None)):
                raise RuntimeError,'set_extents: if no data has been set,'+\
                  'then\nxmin/xmax/ymin/ymax/zmin/zmax must all be specified.'

            if nice == 1:
                dxmin,dxmax,dxlocs,fmt=GetNiceMinMax(xmin,xmax,xspc,
                                                     self.xaxistype)
                self.xmins=dxmin
                self.xmaxs=dxmax
                self.xlocs=dxlocs
                self.xfmt=fmt

                dymin,dymax,dylocs,fmt=GetNiceMinMax(ymin,ymax,yspc,
                                                     self.yaxistype)
                self.ymins=dymin
                self.ymaxs=dymax
                self.ylocs=dylocs
                self.yfmt=fmt

                dzmin,dzmax,dzlocs,fmt=GetNiceMinMax(zmin,zmax,zspc,
                                                     self.zaxistype)
                self.zmins=dzmin
                self.zmaxs=dzmax
                self.zlocs=dzlocs
                self.zfmt=fmt
            else:
                dxmin,dxmax,dxlocs,fmt=GetNiceMinMax(xmin,xmax,xspc,
                                                     self.xaxistype)
                # TO DO: ADD A CHECK HERE AND MAKE SURE THEY ARE LISTS!
                self.xmins=xmin
                self.xmaxs=xmax
                self.xlocs=dxlocs
                self.xfmt=fmt

                dymin,dymax,dyspc,fmt=GetNiceMinMax(ymin,ymax,yspc,
                                                    self.yaxistype)
                self.ymins=list(dymin)
                self.ymaxs=list(dymax)
                self.ylocs=list(dylocs)
                self.yfmt=fmt

                dzmin,dzmax,dzspc,fmt=GetNiceMinMax(zmin,zmax,zspc,
                                                    self.zaxistype)
                self.zmins=list(dzmin)
                self.zmaxs=list(dzmax)
                self.zlocs=list(dzlocs)
                self.zfmt=fmt

        self.set_axis()

    def set_axis(self):
        """ Set the default axis """

        new_axis_data={}
        xbreaks=GetAxisBreaks(self.xmins,self.xmaxs,self.xaxistype)
        ybreaks=GetAxisBreaks(self.ymins,self.ymaxs,self.yaxistype)
        zbreaks=GetAxisBreaks(self.zmins,self.zmaxs,self.zaxistype)
        ext=self.get_plot_extents(include_border=0)
        pxlocs,xok=DataToPlot1D(ext[0],ext[3],self.xmins,self.xmaxs,
                            self.xlocs,self.xaxistype)
        pylocs,yok=DataToPlot1D(ext[1],ext[4],self.ymins,self.ymaxs,
                            self.ylocs,self.yaxistype)
        pzlocs,zok=DataToPlot1D(ext[2],ext[5],self.zmins,self.zmaxs,
                            self.zlocs,self.zaxistype)

        bottomlocs=[]
        toplocs=[]
        leftlocs=[]
        rightlocs=[]
        frontlocs=[]
        backlocs=[]
        ext=self.get_plot_extents(include_border=0)

        for item in pxlocs:
            backlocs.append((item,ext[4]))
            frontlocs.append((item,ext[1]))
        for item in pylocs:
            leftlocs.append((ext[0],item))
            rightlocs.append((ext[3],item))
        for item in pzlocs:
            bottomlocs.append((ext[2],item))
            toplocs.append((ext[5],item))

        # TO DO: add minor tics    
        new_axis_data['topleft']=self.axis_data['topleft']
        new_axis_data['topleft'].set_data(toplocs,list(self.xlocs),self.xfmt,
                                      None,
                                      (ext[0],ext[1],ext[5]),
                                      (ext[0],ext[4],ext[5]),
                                      xbreaks) 
        new_axis_data['topright']=self.axis_data['topright']
        new_axis_data['topright'].set_data(toplocs,list(self.xlocs),self.xfmt,
                                      None,
                                      (ext[3],ext[1],ext[5]),
                                      (ext[3],ext[4],ext[5]),
                                      xbreaks) 
        new_axis_data['topfront']=self.axis_data['topfront']
        new_axis_data['topfront'].set_data(toplocs,list(self.xlocs),self.xfmt,
                                      None,
                                      (ext[0],ext[1],ext[5]),
                                      (ext[3],ext[1],ext[5]),
                                      ybreaks) 
        new_axis_data['topback']=self.axis_data['topback']
        new_axis_data['topback'].set_data(toplocs,list(self.xlocs),self.xfmt,
                                      None,
                                      (ext[0],ext[4],ext[5]),
                                      (ext[3],ext[4],ext[5]),
                                      ybreaks)
        new_axis_data['bottomleft']=self.axis_data['bottomleft']
        new_axis_data['bottomleft'].set_data(bottomlocs,list(self.xlocs),
                                             self.xfmt,
                                      None,
                                      (ext[0],ext[1],ext[2]),
                                      (ext[0],ext[4],ext[2]),
                                      xbreaks)
        new_axis_data['bottomright']=self.axis_data['bottomright']
        new_axis_data['bottomright'].set_data(bottomlocs,list(self.xlocs),
                                              self.xfmt,
                                      None,
                                      (ext[3],ext[1],ext[2]),
                                      (ext[3],ext[4],ext[2]),
                                      xbreaks)
        new_axis_data['bottomfront']=self.axis_data['bottomfront']
        new_axis_data['bottomfront'].set_data(bottomlocs,list(self.xlocs),
                                              self.xfmt,
                                      None,
                                      (ext[0],ext[1],ext[2]),
                                      (ext[3],ext[1],ext[2]),
                                      xbreaks)
        new_axis_data['bottomback']=self.axis_data['bottomback']
        new_axis_data['bottomback'].set_data(bottomlocs,list(self.xlocs),
                                             self.xfmt,
                                      None,
                                      (ext[0],ext[4],ext[2]),
                                      (ext[3],ext[4],ext[2]),
                                      xbreaks)
        new_axis_data['leftfront']=self.axis_data['leftfront']
        new_axis_data['leftfront'].set_data(leftlocs,list(self.ylocs),
                                            self.yfmt,
                                      None,
                                      (ext[0],ext[1],ext[2]),
                                      (ext[0],ext[1],ext[5]),
                                      ybreaks)
        new_axis_data['leftback']=self.axis_data['leftback']
        new_axis_data['leftback'].set_data(leftlocs,list(self.ylocs),self.yfmt,
                                      None,
                                      (ext[0],ext[4],ext[2]),
                                      (ext[0],ext[4],ext[5]),
                                      ybreaks)
        new_axis_data['rightfront']=self.axis_data['rightfront']
        new_axis_data['rightfront'].set_data(rightlocs,list(self.ylocs),
                                             self.yfmt,
                                      None,
                                      (ext[3],ext[1],ext[2]),
                                      (ext[3],ext[1],ext[5]),
                                      ybreaks)
        new_axis_data['rightback']=self.axis_data['rightback']
        new_axis_data['rightback'].set_data(rightlocs,list(self.ylocs),
                                            self.yfmt,
                                      None,
                                      (ext[3],ext[4],ext[2]),
                                      (ext[3],ext[4],ext[5]),
                                      ybreaks)

        self.axis_data=new_axis_data

    #########################################################################
    # Versions of the following functions will be required of most or       #
    # all plot classes.                                                     #
    #########################################################################

    def add_data(self,xarr,yarr,zarr,label='',properties=None):
        """ Add an array to the data to plot """
        self.array_data.append(gvplot_grid_cartesian(xarr,yarr,zarr))
        self.array_layer_defaults.append(
            gvplot_layer_defaults(label,properties,interactive=0))
        return len(self.array_data)-1

    def set_labels(self,xlabel=None,ylabel=None,zlabel=None,title=None):
        """ Set labels. """
        self.label_data['xlabel'][1]=xlabel
        self.label_data['ylabel'][1]=ylabel
        self.label_data['zlabel'][1]=zlabel
        self.label_data['title'][1]=title

    def set_plot_extents(self,pxmin,pxmax,pymin,pymax,pzmin,pzmax):
        """ Set/Reset the extents covered by this plot in the view.
            Update labels, axis, data info.
        """

        oldpxmin=self.pxmin
        oldpxmax=self.pxmax
        oldpymin=self.pymin
        oldpymax=self.pymax
        oldpzmin=self.pzmin
        oldpzmax=self.pzmax

        self.pxmin=pxmin
        self.pxmax=pxmax
        self.pymin=pymin
        self.pymax=pymax
        self.pzmin=pzmin
        self.pzmax=pzmax

        # Update label positions
        for ckey in self.label_data.keys():
            xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.label_data[ckey][0][0],
                              GVPLOT_AXISTYPE_LINEAR)
            ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.label_data[ckey][0][1],
                              GVPLOT_AXISTYPE_LINEAR)
            znew=DataToPlot1D(pzmin,pzmax,oldpzmin,oldpzmax,
                              self.label_data[ckey][0][2],
                              GVPLOT_AXISTYPE_LINEAR)
            self.label_data[ckey][0]=(xnew,ynew,znew)

        # Update axis information            
        for ckey in self.axis_data.keys():
            for idx in range(len(self.axis_data[key].vtics)):
                xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.axis_data[ckey].vtics[idx][0],
                              GVPLOT_AXISTYPE_LINEAR)
                ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.axis_data[ckey].vtics[idx][1],
                              GVPLOT_AXISTYPE_LINEAR)
                znew=DataToPlot1D(pzmin,pzmax,oldpzmin,oldpzmax,
                              self.axis_data[ckey].vtics[idx][2],
                              GVPLOT_AXISTYPE_LINEAR)
                self.axis_data[ckey].vtics[idx]=(xnew,ynew,znew)

            for idx in range(len(self.axis_data[key].tics)):
                xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.axis_data[ckey].tics[idx][0],
                              GVPLOT_AXISTYPE_LINEAR)
                ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.axis_data[ckey].tics[idx][1],
                              GVPLOT_AXISTYPE_LINEAR)
                znew=DataToPlot1D(pzmin,pzmax,oldpzmin,oldpzmax,
                              self.axis_data[ckey].tics[idx][2],
                              GVPLOT_AXISTYPE_LINEAR)
                self.axis_data[ckey].tics[idx]=(xnew,ynew,znew)

            if self.axis_data[ckey].start is not None:
                xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.axis_data[ckey].start[0],
                              GVPLOT_AXISTYPE_LINEAR)
                ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.axis_data[ckey].start[1],
                              GVPLOT_AXISTYPE_LINEAR)
                znew=DataToPlot1D(pzmin,pzmax,oldpzmin,oldpzmax,
                              self.axis_data[ckey].start[2],
                              GVPLOT_AXISTYPE_LINEAR)
                self.axis_data[ckey].start=(xnew,ynew,znew)

            if self.axis_data[ckey].end is not None:
                xnew=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                              self.axis_data[ckey].end[0],
                              GVPLOT_AXISTYPE_LINEAR)
                ynew=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                              self.axis_data[ckey].end[1],
                              GVPLOT_AXISTYPE_LINEAR)
                znew=DataToPlot1D(pzmin,pzmax,oldpzmin,oldpzmax,
                              self.axis_data[ckey].end[2],
                              GVPLOT_AXISTYPE_LINEAR)
                self.axis_data[ckey].end=(xnew,ynew,znew)

            if self.axis_data[ckey].breaks is not None:
                for idx in range(len(breaks)):
                    xnew1=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                        self.axis_data[ckey].breaks[idx][0][0],
                        GVPLOT_AXISTYPE_LINEAR)
                    ynew1=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                        self.axis_data[ckey].breaks[idx][0][1],
                        GVPLOT_AXISTYPE_LINEAR)
                    znew1=DataToPlot1D(pzmin,pzmax,oldpzmin,oldpzmax,
                        self.axis_data[ckey].breaks[idx][0][2],
                        GVPLOT_AXISTYPE_LINEAR)
                    xnew2=DataToPlot1D(pxmin,pxmax,oldpxmin,oldpxmax,
                        self.axis_data[ckey].breaks[idx][1][0],
                        GVPLOT_AXISTYPE_LINEAR)
                    ynew2=DataToPlot1D(pymin,pymax,oldpymin,oldpymax,
                        self.axis_data[ckey].breaks[idx][1][1],
                        GVPLOT_AXISTYPE_LINEAR)
                    znew2=DataToPlot1D(pzmin,pzmax,oldpzmin,oldpzmax,
                        self.axis_data[ckey].breaks[idx][1][2],
                        GVPLOT_AXISTYPE_LINEAR)
                    self.axis_data[ckey].breaks[idx]=((xnew1,ynew1,znew1),
                                                      (xnew2,ynew2,znew2))

        if len(self.xmins) == 0:
            return

        self.reset_axis()

    def get_extents(self):
        """ Get data extents: (xmins, xmaxs, ymins, ymaxs, zmins, zmaxs). """
        return (self.xmins,self.xmaxs,self.ymins,
                self.ymaxs,self.zmins,self.zmaxs)

    def get_plot_extents(self,include_border=1):
        """ Get plot extents with (include_border=1) or
            without (include_border=0) borders.
            (xmin,ymin,zmin,xmax,ymax,zmax).
        """
        if include_border == 1:
            return (self.pxmin,self.pymin,self.pzmin,self.pxmax,self.pymax,
                    self.pzmax)

        dxmin=self.pxmin+(self.pxmax-self.pxmin)*self.lborder
        dxmax=self.pxmax-(self.pxmax-self.pxmin)*self.rborder
        dymin=self.pymin+(self.pymax-self.pymin)*self.fborder
        dymax=self.pymax-(self.pymax-self.pymin)*self.bkborder
        dzmin=self.pzmin+(self.pzmax-self.pzmin)*self.bborder
        dzmax=self.pzmax-(self.pzmax-self.pzmin)*self.tborder

        return (dxmin,dymin,dzmin,dxmax,dymax,dzmax)


    def get_xyposition(self,x,y,z):
        """ Get the data position corresponding to GvViewPlot position x,y,z.
            x, y and z may be single values or same-size arrays.
        """

        # Get plot extents minus borders.
        dxmin=self.pxmin+(self.pxmax-self.pxmin)*self.lborder
        dxmax=self.pxmax-(self.pxmax-self.pxmin)*self.rborder
        dymin=self.pymin+(self.pymax-self.pymin)*self.fborder
        dymax=self.pymax-(self.pymax-self.pymin)*self.bkborder
        dzmin=self.pzmin+(self.pzmax-self.pzmin)*self.bborder
        dzmax=self.pzmax-(self.pzmax-self.pzmin)*self.tborder

        arrshp=None
        if type(x) == type(numpy.array([])):
            arrshp=x.shape
            x=numpy.ravel(x)

        if type(y) == type(numpy.array([])):
            y=numpy.ravel(y)

        if type(y) == type(numpy.array([])):
            z=numpy.ravel(z)

        x,xok=PlotToData1D(dxmin,dxmax,self.xmins,self.xmaxs,x,self.xaxistype)
        y,yok=PlotToData1D(dymin,dymax,self.ymins,self.ymaxs,y,self.yaxistype)
        z,zok=PlotToData1D(dzmin,dzmax,self.zmins,self.zmaxs,z,self.zaxistype)

        okarr=numpy.where(xok==0,0,yok)
        okarr=numpy.where(zok==0,0,okarr)

        if arrshp is not None:
            x=numpy.reshape(x,arrshp)
            y=numpy.reshape(y,arrshp)
            z=numpy.reshape(z,arrshp)
            okarr=numpy.reshape(okarr,arrshp)

        return (x,y,z,okarr)

    def get_plotposition(self,x,y,z):
        """ Get the GvViewPlot position corresponding to data position x,y.
            x and y may be single values or same-length 1-D arrays.
        """

        # Get plot extents minus borders.

        dxmin=self.pxmin+(self.pxmax-self.pxmin)*self.lborder
        dxmax=self.pxmax-(self.pxmax-self.pxmin)*self.rborder
        dymin=self.pymin+(self.pymax-self.pymin)*self.fborder
        dymax=self.pymax-(self.pymax-self.pymin)*self.bkborder
        dzmin=self.pzmin+(self.pzmax-self.pzmin)*self.bborder
        dzmax=self.pzmax-(self.pzmax-self.pzmin)*self.tborder

        arrshp=None
        if type(x) == type(numpy.array([])):
            arrshp=x.shape
            x=numpy.ravel(x)

        if type(y) == type(numpy.array([])):
            y=numpy.ravel(y)

        if type(y) == type(numpy.array([])):
            z=numpy.ravel(z)

        x,xok=DataToPlot1D(dxmin,dxmax,self.xmins,self.xmaxs,x,self.xaxistype)
        y,yok=DataToPlot1D(dymin,dymax,self.ymins,self.ymaxs,y,self.yaxistype)
        z,zok=DataToPlot1D(dzmin,dzmax,self.zmins,self.zmaxs,z,self.zaxistype)

        okarr=numpy.where(xok==0,0,yok)
        okarr=numpy.where(zok==0,0,okarr)

        if arrshp is not None:
            x=numpy.reshape(x,arrshp)
            y=numpy.reshape(y,arrshp)
            z=numpy.reshape(z,arrshp)
            okarr=numpy.reshape(okarr,arrshp)


        return (x,y,z,okarr)        

    def set_arraylayer_properties(self,idx,props):
        """ Set the display properties for the idxth data layer.
            idx- index of data
            props- a dictionary of properties to set (keys) and their
                   values.

            An error will be raised if there is no idx'th data array
            to plot.

            Example properties:
            _area_fill_color='1.0 0.0 0.0 1.0'
            _area_edge_color='1.0'
            _gl_antialias='1' (turn on display antialiasing)
        """
        self.array_layer_defaults[idx]=props

    def set_axislayer_properties(self,props):
        """ Set the display properties of the axis layer

            Example properties:
            _line_color='1.0 0.0 0.0 1.0'
            _line_width='1.0'
            _gl_antialias='1' (turn on display antialiasing)

        """
        self.axis_layer_properties=props

    def set_label_color(self,color):
        """ Set label layer color to color. """

        import gvogrfs
        cstr2=gvogrfs.gv_to_ogr_color(color)
        if len(cstr2) < 9:
            cstr2=cstr2+'FF'
        if self.label_layer_properties.has_key('_gv_ogrfs_point'):
            ogrfs=gvogrfs.OGRFeatureStyle(
                self.label_layer_properties['_gv_ogrfs_point'])
            if ogrfs.has_part('LABEL'):
                part=ogrfs.get_part('LABEL')
                part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                ogrfs.remove_part('LABEL')
                ogrfs.add_part(part)
            if ogrfs.has_part('SYMBOL'):
                part=ogrfs.get_part('SYMBOL')
                part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                ogrfs.remove_part('SYMBOL')
                ogrfs.add_part(part)
            self.label_layer_properties['_gv_ogrfs_point']=ogrfs.unparse()

    def set_axis_color(self,color):
        """ Set all axis and axis label colours to color.
            color is a tuple of 4 values, each between
            0 and 1, representing red/green/blue/alpha
            components.
        """
        cstr1=str(color[0])+' '+str(color[1])+' '+str(color[2])+\
               ' '+str(color[3])
        self.axis_layer_properties['_line_color']=cstr1

        import gvogrfs
        cstr2=gvogrfs.gv_to_ogr_color(color)
        if len(cstr2) < 9:
            cstr2=cstr2+'FF'
        for caxis in self.axis_data.values():
            caxis.line_properties['_line_color']=cstr1
            if caxis.vtics_properties.has_key('_gv_ogrfs'):
                ogrfs=gvogrfs.OGRFeatureStyle(
                    caxis.vtics_properties['_gv_ogrfs'])
                if ogrfs.has_part('LABEL'):
                    part=ogrfs.get_part('LABEL')
                    part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                    ogrfs.remove_part('LABEL')
                    ogrfs.add_part(part)
                if ogrfs.has_part('SYMBOL'):
                    part=ogrfs.get_part('SYMBOL')
                    part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                    ogrfs.remove_part('SYMBOL')
                    ogrfs.add_part(part)
                caxis.vtics_properties['_gv_ogrfs']=ogrfs.unparse()

            if caxis.tics_properties.has_key('_gv_ogrfs'):
                ogrfs=gvogrfs.OGRFeatureStyle(
                    caxis.tics_properties['_gv_ogrfs'])
                if ogrfs.has_part('LABEL'):
                    part=ogrfs.get_part('LABEL')
                    part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                    ogrfs.remove_part('LABEL')
                    ogrfs.add_part(part)
                if ogrfs.has_part('SYMBOL'):
                    part=ogrfs.get_part('SYMBOL')
                    part.set_parm(gvogrfs.OGRFeatureStyleParam('c:'+cstr2))
                    ogrfs.remove_part('SYMBOL')
                    ogrfs.add_part(part)
                caxis.tics_properties['_gv_ogrfs']=ogrfs.unparse()


    def set_axis_properties(self,key,lprops,vprops,tprops):
        """ Set the properties of the shapes forming the axis
            referenced by 'key'.  Default keys are 'top, 'bottom',
            'left','right'.  An error will be raised if the axis
            dictionary doesn't contain key.

            Parameters:
                key- key to index axis
                lprops- line properties (if different from layer default)
                vprops- vtic properties
                tprops- tic properties

            Example properties: (defaults for left axis shown here)
            _gv_ogrfs='LABEL(c:#000000FF,'+\
            'f:"-adobe-helvetica-medium-r-*-*-15-*-*-*-*-*-*-*",
            t:{label},a:0.0,s:1.0,dx:-35.0,dy:5.0);'+\
            'SYMBOL(c:#000000FF,id:ogr-sym-10,a:270.0,s:1.0)'

            Note: the label property is always used to get text
                  in CreateAxisLayer, so the t:{label} portion
                  of this should be left alone.
        """

        self.axis_data[key].line_properties=lprops
        self.axis_data[key].vtics_properties=vprops
        self.axis_data[key].tics_properties=tprops


    def set_border_padding(self,top,bottom,left,right,front,back):
        """ Set border padding as a fraction of the plot
            extents (0-1).
            top- top padding
            bottom- bottom
            left- left
            right- right
            front- front
            back- back
        """
        self.tborder=top
        self.bborder=bottom
        self.lborder=left
        self.rborder=right
        self.fborder=front
        self.bkborder=back

    def set_labellayer_properties(self,label_props):
        """ Set label layer properties:
            Example:
            _gv_ogrfs_point='LABEL(c:#000000FF,
            f:"-adobe-helvetica-medium-r-*-*-15-*-*-*-*-*-*-*",t:{label})'

            Note: the label property is always used to get text
                  in CreateLabelLayer, so the t:{label} portion
                  of this should be left alone.            
        """
        self.label_layer_properties=label_props

    def add_axis(self,key,properties,start_tuple,end_tuple,breaks=None,
                 type=GVPLOT_AXIS_NOTICS):
        """ Add an axis (line):
            key- key to reference the axis
            properties- axis properties
            start_tuple- an (x,y) tuple in data coordinates representing
                         the start point of the line.
            end_tuple- an (x,y) tuple in data coordinates representing
                       the start point of the line.
            breaks- breaks in the axis/line (defaults to None, must be
                    a list of start/end tuples if present).  Also in
                    data coordinates.
            type- type of axis (0-4: not shown, shown but no tics, shown
                  with major tics but no labels, shown with major tics
                  and labels, shown with major tics and labels and minor
                  tics).

        """
        pass


class gvplot_layer_defaults:
    def __init__(self,label="",properties=None,interactive=0):
        """ Store display information for a layer. """

        # default display properties for point/line layers
        if properties is not None:
            self.properties=properties
        else:
            self.properties={}
            self.properties['_gl_antialias']='1'
            self.properties['_line_width']='1'
            self.properties['_line_color']='1.0 0.0 0.0 1.0'
            self.properties['_gl_antialias']='1'

        self.label=label

        # interactive: 1 if user should be able to edit
        #              the layer; 0 otherwise.
        self.interactive=interactive


# supporting classes for storing plot class components

class gvplot_array_cartesian:
    """ Store arrays and related information.
        Discard nan's and inf's and calculate max/mins
        for plotting convenience.
        xarr- array or list of arrays
        yarr- array or list of arrays
        zarr- array or list of arrays

        xarr, yarr, zarr must be same size.
    """
    def __init__(self,xarr,yarr,zarr=None):


        if type(xarr) == type(numpy.array([1,2])):
            # Below:
            # Data points must have x, y, and z (if present)
            # not equal to nan or inf to be included.
            # This isn't implemented properly yet though
            # because of problems on some platforms with nan/inf
            okarr=numpy.ones(xarr.shape)

            # nt and irix both don't seem to have a notion
            # of nan and inf, so leave out the checks for
            # them...
            #if os.name != 'nt':
            #    okarr=numpy.where(xarr == float('inf'),0,okarr)
            #    okarr=numpy.where(xarr == float('nan'),0,okarr)
            #    okarr=numpy.where(yarr == float('inf'),0,okarr)
            #    okarr=numpy.where(yarr == float('nan'),0,okarr)

            #    if zarr is not None:
            #        okarr=numpy.where(zarr == float('inf'),0,okarr)
            #        okarr=numpy.where(zarr == float('nan'),0,okarr)

            self.xarr=numpy.compress(okarr==1,xarr)
            self.yarr=numpy.compress(okarr==1,yarr)

            if zarr is not None:
                self.zarr=numpy.compress(okarr==1,zarr)

            self.xmin=min(self.xarr)
            self.xmax=max(self.xarr)
            self.ymin=min(self.yarr)
            self.ymax=max(self.yarr)

            if zarr is not None:
                self.zmin=min(self.zarr)
                self.zmax=max(self.zarr)
            else:
                self.zarr=None
                self.zmin=None
                self.zmax=None
        elif type(xarr) == type([1,2]):
            self.xarr=xarr
            self.yarr=yarr
            self.zarr=zarr
            # If nan/inf problems get sorted out, add checking
            # code below...
            self.xmin=min(self.xarr[0])
            self.xmax=max(self.xarr[0])
            self.ymin=min(self.yarr[0])
            self.ymin=max(self.yarr[0])
            if zarr is not None:
                self.zmin=min(self.zarr[0])
                self.zmax=max(self.zarr[0])

            for idx in range(1,len(xarr)):
                self.xmin=min(min(self.xarr[idx]),self.xmin)
                self.xmax=max(max(self.xarr[idx]),self.xmax)
                self.ymin=min(min(self.yarr[idx]),self.ymin)
                self.ymax=max(max(self.yarr[idx]),self.ymax)
                if zarr is not None:
                    self.zmin=min(min(self.zarr[idx]),self.zmin)
                    self.zmax=max(max(self.zarr[idx]),self.zmax)



class gvplot_grid_cartesian:
    """ Store arrays and related information.
        Discard nan's and inf's and calculate max/mins
        for plotting convenience.
        xarr- 2D array or list of arrays
        yarr- 2D array or list of arrays
        zarr- 2D array or list of arrays

        xarr, yarr, zarr must be same size.
    """
    def __init__(self,xarr,yarr,zarr=None):

        if type(xarr) == type(numpy.array([1,2])):
            self.xarr=xarr
            self.yarr=yarr
            self.zarr=zarr

            self.xmin=min(numpy.ravel(self.xarr))
            self.xmax=max(numpy.ravel(self.xarr))
            self.ymin=min(numpy.ravel(self.yarr))
            self.ymax=max(numpy.ravel(self.yarr))

            if zarr is not None:
                self.zmin=min(numpy.ravel(self.zarr))
                self.zmax=max(numpy.ravel(self.zarr))
            else:
                self.zarr=None
                self.zmin=None
                self.zmax=None

        elif type(xarr) == type([1,2]):
            self.xarr=xarr
            self.yarr=yarr
            self.zarr=zarr

            self.xmin=min(numpy.ravel(self.xarr[0]))
            self.xmax=max(numpy.ravel(self.xarr[0]))
            self.ymin=min(numpy.ravel(self.yarr[0]))
            self.ymin=max(numpy.ravel(self.yarr[0]))
            if zarr is not None:
                self.zmin=min(numpy.ravel(self.zarr[0]))
                self.zmax=max(numpy.ravel(self.zarr[0]))

            for idx in range(1,len(xarr)):
                self.xmin=min(min(numpy.ravel(self.xarr[idx])),self.xmin)
                self.xmax=max(max(numpy.ravel(self.xarr[idx])),self.xmax)
                self.ymin=min(min(numpy.ravel(self.yarr[idx])),self.ymin)
                self.ymax=max(max(numpy.ravel(self.yarr[idx])),self.ymax)
                if zarr is not None:
                    self.zmin=min(numpy.ravel(min(self.zarr[idx])),self.zmin)
                    self.zmax=max(numpy.ravel(max(self.zarr[idx])),self.zmax)



class gvplot_axis:
    """ Axis/line properties. """
    def __init__(self,displaytype=GVPLOT_AXIS_VTICSLABELS,tics_per_vtic=4):
        # Display defaults, set before plotting.  Can
        # be overridden through function calls.  These
        # are not erased when data max/mins are reset.

        self.display_type=displaytype # whether or not to show axis, tics, etc.
        self.tics_per_vtic=tics_per_vtic

        self.vtics_properties={}
        self.tics_properties={}

        # Tic label/format defaults, except for label text
        # (determined by plotting functions).
        self.vtics_properties['_gv_ogrfs']='LABEL(c:#000000FF,'+\
            'f:"'+DEFAULT_FONT+'",t:{label},a:0.0,s:1.0,dx:0.0,dy:0.0);'+\
            'SYMBOL(c:#000000FF,id:ogr-sym-10,a:0.0,s:1.0)'
        self.tics_properties['_gv_ogrfs']=\
                     'SYMBOL(c:#000000FF,id:ogr-sym-10,a:0.0,s:0.5)'

        # Axis line properties (color, etc) if different from overall
        # axis layer default
        self.line_properties={}

        # Properties calculated by plotting object

        # major tics (bigger, may have values)
        self.vtics=[]   # xyz locations in view coords.
        self.vtics_labels=[] # text for labels
        self.vtics_fmt=None  # format for labels eg. "%6f"

        # minor tics
        self.tics=[]

        # start and end points of line in
        # view area coordinates
        self.start=None
        self.end=None

        # discontinuities: each a tuple of 2 values
        # (start,end) where (0 <start < end < 1),and
        # breaks[idx][1] < breaks[idx+1][0].  Start
        # and end correspond to fractions of the distance
        # along the axis.
        self.breaks=None 

    def set_data(self,vtics,vtics_labels,vtics_fmt,tics,start,end,breaks):
        """ Set the tic data. """
        self.vtics=vtics
        self.vtics_labels=vtics_labels
        self.vtics_fmt=vtics_fmt
        self.tics=tics
        self.start=start
        self.end=end
        self.breaks=breaks


#######################################################################
# Layer creation functions                                            #
#######################################################################

def CreateAxisLayer(plot_data,name=None):
    """ Create Axis and lines (including tics and axis numbering) """

    if name is None:
        name=plot_data.plot_label+' '+GVPLOT_AXIS_LAYER

    shapes=gview.GvShapes(name=name)
    layer=gview.GvShapesLayer(shapes=shapes)
    for caxis in plot_data.axis_data.values():
        if caxis.display_type == GVPLOT_AXIS_NOTSHOWN:
            continue
        x0,y0,z0=caxis.start
        xf,yf,zf=caxis.end
        if caxis.breaks is None:
            shp=gview.GvShape(type=gview.GVSHAPE_LINE)
            shp.set_node(x0,y0,z0,0)
            shp.set_node(xf,yf,zf,1)

            shapes.append(shp)
        else:
            last_x=x0
            last_y=y0
            last_z=z0
            for btuple in caxis.breaks:
                shp=gview.GvShape(type=gview.GVSHAPE_LINE)
                shp.set_node(last_x,last_y,last_z,0)
                shp.set_node(x0 +(btuple[0]*(xf-x0)),
                             yf +(btuple[0]*(yf-y0)),
                             zf +(btuple[0]*(zf-z0)),1)
                last_x = x0 + btuple[1]*(xf-x0)
                last_y = y0 + btuple[1]*(yf-y0)
                last_z = z0 + btuple[1]*(zf-z0)
                shapes.append(shp)

        if caxis.display_type == GVPLOT_AXIS_NOTICS:
            continue

        for idx in range(len(caxis.vtics)):
            shp=gview.GvShape(type=gview.GVSHAPE_POINT)
            shp.set_node(caxis.vtics[idx][0],caxis.vtics[idx][1])
            if caxis.display_type != GVPLOT_AXIS_VTICS:
                shp.set_property('label',
                     caxis.vtics_fmt % caxis.vtics_labels[idx])
            for ckey in caxis.vtics_properties.keys():
                shp.set_property(ckey,
                  caxis.vtics_properties[ckey])
            shapes.append(shp)

        if caxis.display_type != GVPLOT_AXIS_VTICSLABELSTICS:
            continue

        for idx in range(len(caxis.tics)):
            shp=gview.GvShape(type=gview.GVSHAPE_POINT)
            shp.set_node(caxis.tics[idx][0],caxis.tics[idx][1])
            for ckey in caxis.tics_properties.keys():
                shp.set_property(ckey,
                  caxis.tics_properties[ckey])
            shapes.append(shp)    

    if len(shapes) == 0:
        return None

    for ckey in plot_data.axis_layer_properties.keys():
        layer.set_property(ckey,plot_data.axis_layer_properties[ckey])

    return layer


def Create2DBorderLayer(plot_data,name=None):
    """ Create the plot background. """

    if ((plot_data.show_inner_border == 0) and
        (plot_data.show_outer_border == 0)):
        return None

    if name is None:
        name=plot_data.plot_label+' '+GVPLOT_BORDER_LAYER

    shapes=gview.GvShapes(name=name)
    layer=gview.GvShapesLayer(shapes=shapes)
    inner=plot_data.get_plot_extents(include_border=0)
    outer=plot_data.get_plot_extents(include_border=1)

    # shapes: outer rings clockwise; inner rings
    #         counterclockwise.
    if plot_data.show_outer_border == 1:
        nshp=gview.GvShape(type=gview.GVSHAPE_AREA)
        nshp.set_node(outer[0],outer[3],0.0,0,0)
        nshp.set_node(outer[2],outer[3],0.0,1,0)
        nshp.set_node(outer[2],outer[1],0.0,2,0)
        nshp.set_node(outer[0],outer[1],0.0,3,0)
        nshp.set_node(outer[0],outer[3],0.0,4,0)
        nshp.set_node(inner[0],inner[3],0.0,0,1)
        nshp.set_node(inner[0],inner[1],0.0,1,1)
        nshp.set_node(inner[2],inner[1],0.0,2,1)
        nshp.set_node(inner[2],inner[3],0.0,3,1)
        nshp.set_node(inner[0],inner[3],0.0,4,1)        
        for cprop in plot_data.outer_border_props.keys():
            nshp.set_property(cprop,
                          plot_data.outer_border_props[cprop])
        shapes.append(nshp)

    if plot_data.show_inner_border == 1:
        nshp=gview.GvShape(type=gview.GVSHAPE_AREA)
        nshp.set_node(inner[0],inner[3],0.0,0,0)
        nshp.set_node(inner[2],inner[3],0.0,1,0)
        nshp.set_node(inner[2],inner[1],0.0,2,0)
        nshp.set_node(inner[0],inner[1],0.0,3,0)
        nshp.set_node(inner[0],inner[3],0.0,4,0)
        for cprop in plot_data.inner_border_props.keys():
            nshp.set_property(cprop,
                          plot_data.inner_border_props[cprop])
        shapes.append(nshp)

    return layer    

def CreateLabelLayer(plot_data,name=None):
    """ Create annotation (label) layer. """

    if name is None:
        name=plot_data.plot_label+' '+GVPLOT_LABEL_LAYER

    shapes=gview.GvShapes(name=name)
    layer=gview.GvShapesLayer(shapes=shapes)
    for item in plot_data.label_data.values():
        if item[1] is None:
            continue

        shp=gview.GvShape(type=gview.GVSHAPE_POINT)
        shp.set_node(item[0][0],item[0][1],item[0][2])
        shp.set_property('label',item[1])
        shapes.append(shp)

    if len(shapes) == 0:
        return None

    for prop in plot_data.label_layer_properties.keys():
        layer.set_property(prop,
                    plot_data.label_layer_properties[prop])

    return layer


def Create2DLegendLayer(plot_data,name=None):
    """ Create a legend layer. """

    if plot_data.show_legend == 0:
        return None

    if name is None:
        name=plot_data.plot_label+' '+GVPLOT_LEGEND_LAYER

    shapes=gview.GvShapes(name=name)
    layer=gview.GvShapesLayer(shapes=shapes)
    ext=plot_data.get_plot_extents(include_border=1)

    xrng=ext[3]-ext[0]
    yrng=ext[4]-ext[1]
    x0 = plot_data.legend_location[0]*xrng+ext[0]
    y0 = plot_data.legend_location[1]*yrng+ext[1]

    idx=0
    for item in plot_data.array_layer_defaults:
        try:
            color=item.properties['_line_color']
        except:
            continue

        tmp = color.split()
        rgba=[]
        for tmp2 in tmp:
            rgba.append(float(tmp2))

        cstr=gvogrfs.gv_to_ogr_color(rgba)

        if (item.label is not None) and (len(item.label) > 0):
            dshp=gview.GvShape(type=gview.GVSHAPE_POINT)
            dshp.set_node(x0,y0)
            dshp.set_property('_gv_ogrfs','LABEL(c:'+cstr+\
              ',t:"'+item.label+'",f:"'+DEFAULT_FONT+'")')
            shapes.append(dshp)


        y0=y0-0.1*yrng                
        idx=idx+1

    return layer

def CreateGridArrayDataLayer(plot_data,index,name=None):
    """ Create a 3D grid data layer.
        plot_data- plot object (eg. gvplot_grid_cartesian)
        index- index to array to plot
        name- layer name (if set to None, the layer name
              will be 'Data: '+plot_data.array_layer_defaults.label)
    """

    if index > len(plot_data.array_data)-1:
        raise RuntimeError,'Create Data Layer: invalid data index!'

    if name is None:
        if plot_data.array_layer_defaults[index].label is not None:
            name=plot_data.plot_label+GVPLOT_DATA_LAYER+' '+\
              plot_data.array_layer_defaults[index].label
        else:
            name=plot_data.plot_label+GVPLOT_DATA_LAYER+' '+str(index)

    shapes=gview.GvShapes(name=name)
    layer=gview.GvShapesLayer(shapes=shapes) 

    if type(plot_data.array_data[index].xarr) == type([]):
        xinlist=plot_data.array_data[index].xarr
        yinlist=plot_data.array_data[index].yarr
        zinlist=plot_data.array_data[index].zarr
    else:
        xinlist=[plot_data.array_data[index].xarr]
        yinlist=[plot_data.array_data[index].yarr]
        zinlist=[plot_data.array_data[index].zarr]

    ext=plot_data.get_plot_extents(include_border=0)
    okinlist=[]

    for idx in range(len(xinlist)):
        xarr,xok=DataToPlot1D(ext[0],ext[3],
                              plot_data.xmins,plot_data.xmaxs,
                              xinlist[idx],
                              plot_data.xaxistype)

        yarr,yok=DataToPlot1D(ext[1],ext[4],
                              plot_data.ymins,plot_data.ymaxs,
                              yinlist[idx],
                              plot_data.yaxistype)

        okarr=numpy.where(yok == 0,0,xok)

        zarr,zok=DataToPlot1D(ext[2],ext[5],
                              plot_data.zmins,plot_data.zmaxs,
                              zinlist[idx],
                              plot_data.zaxistype)
        okarr=numpy.where(zok == 0,0,okarr)

        for idx in range(xarr.shape[0]-1):
            for idx2 in range(xarr.shape[1]-1):
                if (okarr[idx,idx2]+okarr[idx,idx2+1]+okarr[idx+1,idx2]+
                    okarr[idx+1,idx2+1] == 4):
                    ashp=gview.GvShape(type=gview.GVSHAPE_AREA)
                    ashp.add_node(xarr[idx,idx2],yarr[idx,idx2],
                                  zarr[idx,idx2])
                    ashp.add_node(xarr[idx+1,idx2],yarr[idx+1,idx2],
                                  zarr[idx+1,idx2])
                    ashp.add_node(xarr[idx+1,idx2+1],yarr[idx+1,idx2+1],
                                  zarr[idx+1,idx2+1])
                    ashp.add_node(xarr[idx,idx2],yarr[idx,idx2],
                                  zarr[idx,idx2])
                    shapes.append(ashp)
                    ashp=gview.GvShape(type=gview.GVSHAPE_AREA)
                    ashp.add_node(xarr[idx,idx2],yarr[idx,idx2],
                                  zarr[idx,idx2])
                    ashp.add_node(xarr[idx,idx2+1],yarr[idx,idx2+1],
                                  zarr[idx,idx2+1])
                    ashp.add_node(xarr[idx+1,idx2+1],yarr[idx+1,idx2+1],
                                  zarr[idx+1,idx2+1])
                    ashp.add_node(xarr[idx,idx2],yarr[idx,idx2],
                                  zarr[idx,idx2])
                    shapes.append(ashp)
                    #lshp=gview.GvShape(type=gview.GVSHAPE_LINE)
                    #lshp.add_node(xarr[idx,idx2],yarr[idx,idx2],
                    #              zarr[idx,idx2])
                    #lshp.add_node(xarr[idx+1,idx2],yarr[idx+1,idx2],
                    #              zarr[idx+1,idx2])
                    #lshp.add_node(xarr[idx+1,idx2+1],yarr[idx+1,idx2+1],
                    #              zarr[idx+1,idx2+1])
                    #lshp.add_node(xarr[idx,idx2+1],yarr[idx,idx2+1],
                    #              zarr[idx,idx2+1])
                    #lshp.add_node(xarr[idx,idx2],yarr[idx,idx2],
                    #              zarr[idx,idx2])
                    #shapes.append(lshp)

    for ckey in plot_data.array_layer_defaults[index].properties.keys():
        layer.set_property(ckey,
              plot_data.array_layer_defaults[index].properties[ckey])

    return layer


def CreateArrayDataLayer(plot_data,index,name=None):
    """ Create a data layer.
        plot_data- plot object (eg. gvplot_2Ddata_cartesian)
        index- index to array to plot
        name- layer name (if set to None, the layer name
              will be 'Data: '+plot_data.array_layer_defaults.label)
    """

    if index > len(plot_data.array_data)-1:
        raise RuntimeError,'Create Data Layer: invalid data index!'

    if name is None:
        if plot_data.array_layer_defaults[index].label is not None:
            name=plot_data.plot_label+GVPLOT_DATA_LAYER+' '+\
              plot_data.array_layer_defaults[index].label
        else:
            name=plot_data.plot_label+GVPLOT_DATA_LAYER+' '+str(index)

    shapes=gview.GvShapes(name=name)
    layer=gview.GvShapesLayer(shapes=shapes) 

    ext=plot_data.get_plot_extents(include_border=0)
    okinlist=[]

    if type(plot_data.array_data[index].xarr) == type([]):
        xinlist=plot_data.array_data[index].xarr
        yinlist=plot_data.array_data[index].yarr
        if plot_data.array_data[index].zarr is not None:
            zinlist=plot_data.array_data[index].zarr
    else:
        xinlist=[plot_data.array_data[index].xarr]
        yinlist=[plot_data.array_data[index].yarr]
        if plot_data.array_data[index].zarr is not None:
            zinlist=[plot_data.array_data[index].zarr]

    for idx in range(len(xinlist)):
        xarr,xok=DataToPlot1D(ext[0],ext[3],
                              plot_data.xmins,plot_data.xmaxs,
                              xinlist[idx],
                              plot_data.xaxistype)

        yarr,yok=DataToPlot1D(ext[1],ext[4],
                              plot_data.ymins,plot_data.ymaxs,
                              yinlist[idx],
                              plot_data.yaxistype)

        okarr=numpy.where(yok == 0,0,xok)

        if plot_data.array_data[index].zarr is not None:
            zarr,zok=DataToPlot1D(ext[2],ext[5],
                                plot_data.zmins,plot_data.zmaxs,
                                zinlist[idx],
                                plot_data.zaxistype)
            okarr=numpy.where(zok == 0,0,okarr)
        else:
            zarr=numpy.zeros(numpy.shape(xarr),numpy.float64)

        # NOTE: taking the for-loop down to the c-level didn't
        # speed up the plot time.  The render time seems
        # to be the limiting factor.
        newshps=gview.gv_shapes_lines_for_vecplot(xarr,yarr,zarr,okarr)
        for idx in range(len(newshps)):
            shapes.append(newshps[idx].copy())

        #lshp=gview.GvShape(type=gview.GVSHAPE_LINE)
        #last_valid=0
        #for idx2 in range(len(xarr)):
        #    if okarr[idx2] == 1:
        #        lshp.add_node(xarr[idx2],yarr[idx2],zarr[idx2],0)
        #        last_valid=1
        #    else:
        #        if last_valid == 1:
        #            shapes.append(lshp)
        #            lshp=gview.GvShape(type=gview.GVSHAPE_LINE)
        #            last_valid=0

    #if last_valid == 1:
    #    shapes.append(lshp)

    for ckey in plot_data.array_layer_defaults[index].properties.keys():
        layer.set_property(ckey,
              plot_data.array_layer_defaults[index].properties[ckey])

    return layer


# PLOT class (GvPlot)- stores one or more plots
class GvSimplePlot(gview.GvViewArea):
    """ View area wrapper designed for plotting.
        NOTE: window containing GvSimplePlot view area must
        have its show_all function called for the first time
        ATER the GvSimplePlot widget is inserted, but
        BEFORE plot is called.  If these two conditions
        don't hold, then the configure event that
        sets up the plot area's internal c-level
        size parameters sets them to invalid values,
        and the plot extents will be set incorrectly
        (plot will be a teeny blob in the center).
        Calling show_all before and after the widget
        is inserted also will not work (the second
        call doesn't seem to do anything).
    """
    def __init__(self, _obj=None, bgcolor=(1.0,1.0,1.0,1.0)):
        gview.GvViewArea.__init__(self)
        self.set_background_color(bgcolor)
        self.set_border_padding()
        self.bgcolor=bgcolor
        self.mode='2D'

        # plot information
        self.plots=[]


    def set_border_padding(self,top=DEFAULT_EDGE_PADDING,
                         bottom=DEFAULT_EDGE_PADDING,
                         left=DEFAULT_EDGE_PADDING,
                         right=DEFAULT_EDGE_PADDING):
        """ Set border padding for plot as a fraction of view
            area (top,bottom,left, right).
        """
        self.tborder=top
        self.bborder=bottom
        self.lborder=left
        self.rborder=right

    def oplot(self, yarr=None, xarr=None, xmin=None,xmax=None,xspc=None,
              ymin=None, ymax=None, yspc=None,datalabel=None,
              color=(0.0,0.0,1.0,1.0),drawstyle='_', reset_extents=1,
              with_legend=0,interactive=0):
        """ Plot overtop of an existing simple 2-D plot """

        if yarr is None:
            print 'Usage: oplot(yarr,[,xarr][,xmin=n][,xmax=n][,xspc=n]'
            print '            [,ymin=n][,ymax=n][yspc=n][,datalabel=text]'
            print '            [,color=4-tuple][,drawstyle=text]'
            print '            [,reset_extents=0 or 1][,with_legend=0 or 1])'
            print ''
            print ' yarr -- 1-D array of y values to plot'
            print ' xarr -- x values (optional): if present, must be same'
            print '         length as yarr.'
            print ' xmin -- minimum x (may be rounded for nice labels)'
            print ' xmax -- maximum x (may be rounded for nice labels)'
            print ' xspc -- # x tics (may be rounded for nice labels)'
            print ' ymin -- minimum y (may be rounded for nice labels)'
            print ' ymax -- maximum y (may be rounded for nice labels)'
            print ' yspc -- # ytics (may be rounded for nice labels)'
            print ' datalabel -- data label: add a legend (label datalabel) '
            print ' color -- a tuple of rgba values, each ranging from '
            print '           0.0-1.0 (color to draw data line).'
            print " drawstyle -- linestyle.  Currently only '_' is supported. "
            print ' reset_extents -- reset extents (1) or leave extents as '
            print '                  they are (0).  xmin/xmax/ymin/ymax will'
            print '                  be ignored if this is 0.'
            print ' with_legend -- 1 to add a legend, 0 otherwise.  Do not '
            print '                add until all datasets have been put on.'
            print ' interactive -- 0 if data in plot is not user-editable;'
            print '                1 if it is.  Defaults to 0.'
            print ''
            return

        if len(self.plots) == 0:
            raise RuntimeError,'No plot to plot over.'

        try:
            dshape = numpy.shape( yarr )
        except:
            raise ValueError,"data argument to plot() does not appear to be "+\
                          "a NumPy array"

        dim = len(dshape)

        if dim == 2 and (dshape[0] == 1):
            yarr=numpy.reshape(yarr,(dshape[1],))
        elif dim == 2 and (dshape[1] == 1):
            yarr=numpy.reshape(yarr,(dshape[0],))
        elif dim != 1:
            raise ValueError,\
                  "data argument dimension or shape is not supported."

        if xarr is None:
            xarr=numpy.array(range(len(yarr)))

        cstr=str(color[0])+' '+str(color[1])+' '+str(color[2])+\
              ' '+str(color[3])
        props={}
        props['_line_color']=cstr
        props['_gl_antialias']='1'
        idx=self.plots[0].add_data(xarr,yarr,datalabel,props,interactive)
        if with_legend == 1:
            self.plots[0].set_legend()
        if reset_extents == 1:
            self.plots[0].set_extents(xmin=xmin,ymin=ymin,xmax=xmax,ymax=ymax,
                                      xspc=xspc,yspc=yspc) 
            self.clear_layers()
            ll=self.plots[0].create_layers()
            for item in ll:
                self.add_layer(item)        
        else:
            self.plots[0].create_layer(ltype=GVPLOT_DATA_LAYER,idx=idx)
            # Clear the layers from the view then re-add them so that
            # they are in the right order.
            self.clear_layers()
            ll=self.plots[0].get_layers()
            for item in ll:
                self.add_layer(item)        

        xsize=float(self.get_width())
        ysize=float(self.get_height())
        self.fit_extents(0.0,0.0,1.0,ysize/xsize)

    def plot(self,yarr=None,xarr=None, xlabel=None, ylabel=None, xmin=None,
             xmax=None,xspc=None,ymin=None, ymax=None, yspc=None,
             title=None,datalabel=None,
             color=(1.0,0.0,0.0,1.0),drawstyle='_',interactive=0):
        """ Simple 2-D plot """

        if yarr is None:
            print 'Usage: plot(yarr,[,xarr][, xlabel=text] [,ylabel=text]'
            print '                 [,xmin=n][, xmax=n][, xspc=n][, ymin=n]'
            print '                 [, ymax=n][, yspc=n]'
            print '                 [title=text] [datalabel=text]'
            print ''
            print ' yarr -- 1-D array of values to plot'
            print ' xarr -- x values (optional): if present, must be same'
            print '         length as yarr.'
            print ' xlabel -- text for x label '
            print ' xmin -- minimum x (may be rounded for nice labels)'
            print ' xmax -- maximum x (may be rounded for nice labels)'
            print ' xspc -- x spacing (if not present, default will be used)'
            print ' ylabel -- text for y label '
            print ' ymin -- minimum y (may be rounded for nice labels)'
            print ' ymax -- maximum y (may be rounded for nice labels)'
            print ' yspc -- y spacing (if not present, default will be used)'
            print ' title -- title text'
            print ' datalabel -- data label: add a legend (label datalabel) '
            print ' color -- a tuple of rgba values, each ranging from '
            print '           0.0-1.0 (color to draw data line).'
            print " drawstyle -- linestyle.  Currently only '_' is supported. "
            print ' interactive -- 0 if data in plot is not user-editable;'
            print '                1 if it is.  Defaults to 0.'
            print ''
            return

        try:
            dshape = numpy.shape( yarr )
        except:
            raise ValueError,"data argument to plot() does not appear to be "+\
                          "a NumPy array"

        dim = len(dshape)

        if dim == 2 and (dshape[0] == 1):
            yarr=numpy.reshape(yarr,(dshape[1],))
        elif dim == 2 and (dshape[1] == 1):
            yarr=numpy.reshape(yarr,(dshape[0],))
        elif dim != 1:
            raise ValueError,\
                  "data argument dimension or shape is not supported."

        if xarr is None:
            xarr=numpy.array(range(len(yarr)))

        self.clear()
        xsize=float(self.get_width())
        ysize=float(self.get_height())
        self.plots.append(gvplot_2Ddata_cartesian('1: ',0.0,0.0,1.0,
                                                  ysize/xsize))
        self.plots[0].set_border_padding(self.tborder,self.bborder,
                                         self.lborder,self.rborder)
        cstr=str(color[0])+' '+str(color[1])+' '+str(color[2])+\
              ' '+str(color[3])
        props={}
        props['_line_color']=cstr
        props['_gl_antialias']='1'
        self.plots[0].add_data(xarr,yarr,datalabel,props,interactive)
        self.plots[0].set_extents(xmin=xmin,xmax=xmax,ymin=ymin,
                                  ymax=ymax,xspc=xspc,yspc=yspc)

        self.plots[0].set_labels(xlabel,ylabel,title)

        if self.bgcolor == (0.0,0.0,0.0,1.0):
            self.plots[0].set_axis_color((1.0,1.0,1.0,1.0))
            self.plots[0].set_label_color((1.0,1.0,1.0,1.0))

        ll=self.plots[0].create_layers()
        for item in ll:
            self.add_layer(item)

        self.fit_extents(0.0,0.0,1.0,ysize/xsize)

    def seeall(self):
        """ Set extents so that whole plot is shown in view. """
        xsize=float(self.get_width())
        ysize=float(self.get_height())
        self.fit_extents(0.0,0.0,1.0,ysize/xsize)

    def plot3D(self,xarr,yarr,zarr):
        """ Simple 3-D plot """
        self.set_mode(gview.MODE_3D)

        props={}
        props['_area_edge_color']='0.0 0.0 0.0 1.0'
        props['_area_fill_color']='0.0 0.0 1.0 1.0'
        #props['_line_color']='0.0 0.0 0.0 1.0'
        props['_gl_antialias']='1'

        self.clear()
        xsize=float(self.get_width())
        ysize=float(self.get_height())
        self.plots.append(gvplot_3Ddata_cartesiangrid('1: ',0.0,0.0,0.0,
                                                  1.0,1.0,ysize/xsize))
        self.plots[0].add_data(xarr,yarr,zarr,'',props)
        self.plots[0].set_extents()
        #self.plots[0].set_labels(xlabel,ylabel,title)

        self.plots[0].set_axis_color((0.0,0.0,0.0,1.0))
        self.plots[0].set_label_color((0.0,0.0,0.0,1.0))

        ll=self.plots[0].create_layers()
        for item in ll:
            self.add_layer(item)

        self.set_3d_view_look_at((1.2,-0.3,0.5),(-10.0,20.0))

    def set_plot(self,plot):
        """ Function to allow user to set plot class directly """
        self.clear()
        self.plots=[]
        self.plots.append(plot)

        xsize=float(self.get_width())
        ysize=float(self.get_height())
        self.plot.set_plot_extents(0.0,0.0,1.0,ysize/xsize)

        ll=self.plots[0].create_layers()
        for item in ll:
            self.add_layer(item)        


    def begin_plot(self,xlabel=None,ylabel=None,title=None):
        """ Begin constructing a plot (use to construct a plot
            without displaying at each iteration).
        """

        self.clear()
        xsize=float(self.get_width())
        ysize=float(self.get_height())
        self.plots.append(gvplot_2Ddata_cartesian('1: ',0.0,0.0,1.0,
                                                  ysize/xsize))
        self.plots[0].set_border_padding(self.tborder,self.bborder,
                                         self.lborder,self.rborder)

        self.plots[0].set_labels(xlabel,ylabel,title)

        if self.bgcolor == (0.0,0.0,0.0,1.0):
            self.plots[0].set_axis_color((1.0,1.0,1.0,1.0))
            self.plots[0].set_label_color((1.0,1.0,1.0,1.0))

    def shift_label(self,label_key,xytuple,plot_idx=0):
        """ Shift label referenced by label_key by the
            amounts in xytuple for the plot_idxth plot.
            label_key- eg. 'xlabel','ylabel','title'
            xytuple- (x,y), where x and y are fractions
                     of the plot extents
            plot_idx- plot index of plot whose labels are
                      to be adjusted.
        """
        ext=self.plots[plot_idx].get_plot_extents(include_border=1)
        xrng=ext[3]-ext[0]
        yrng=ext[4]-ext[1]
        xold=self.plots[plot_idx].label_data[label_key][0][0]
        yold=self.plots[plot_idx].label_data[label_key][0][1]

        self.plots[plot_idx].label_data[label_key][0]=(xold+xytuple[0]*xrng,
                                               yold+xytuple[1]*yrng,
                                               0)

    def add_data(self, xarr=None, yarr=None,datalabel=None,
              color=(0.0,0.0,1.0,1.0),drawstyle='_', interactive=0):

        try:
            dshape = numpy.shape( xarr )
            dshape = numpy.shape( yarr )
        except:
            raise ValueError,"data argument to plot() does not appear to be "+\
                          "a NumPy array"

        dim = len(dshape)

        if dim == 2 and (dshape[0] == 1):
            yarr=numpy.reshape(yarr,(dshape[1],))
        elif dim == 2 and (dshape[1] == 1):
            yarr=numpy.reshape(yarr,(dshape[0],))
        elif dim != 1:
            raise ValueError,\
                  "data argument dimension or shape is not supported."

        if xarr is None:
            xarr=numpy.array(range(len(yarr)))

        cstr=str(color[0])+' '+str(color[1])+' '+str(color[2])+\
              ' '+str(color[3])
        props={}
        props['_line_color']=cstr
        props['_gl_antialias']='1'
        self.plots[0].add_data(xarr,yarr,datalabel,props,interactive)


    def end_plot(self,xmin=None,xmax=None,ymin=None,ymax=None,
                 xtics=None,ytics=None,with_legend=0):

        self.plots[0].set_extents(xmin=xmin,xmax=xmax,ymin=ymin,
                                  ymax=ymax)
        xspc=None
        yspc=None
        if xtics is not None:
            # Currently this will only work for continuous
            # plots.
            xmin=self.plots[0].xmins[0]
            xmax=self.plots[0].xmaxs[len(self.plots[0].xmaxs)-1]
            xspc=(xmax-xmin)/(xtics-1)
            rord=numpy.log10(abs(xspc))
            nrord=rord % 1
            spc=pow(10,numpy.floor(rord))
            min_diff=abs(xspc-spc)
            for i in [1,2,5]:
                nspc=i*pow(10,numpy.floor(rord))
                if abs(nspc-spc) < min_diff:
                    min_diff=abs(xspc-nspc)
                    spc=nspc
            xspc=spc

        if ytics is not None:
            # Currently this will only work for continuous
            # plots.
            ymin=self.plots[0].ymins[0]
            ymax=self.plots[0].ymaxs[len(self.plots[0].ymaxs)-1]
            yspc=(ymax-ymin)/(ytics-1)
            rord=numpy.log10(abs(yspc))
            nrord=rord % 1
            spc=pow(10,numpy.floor(rord))
            min_diff=abs(yspc-spc)
            for i in [1,2,5]:
                nspc=i*pow(10,numpy.floor(rord))
                if abs(nspc-spc) < min_diff:
                    min_diff=abs(yspc-nspc)
                    spc=nspc
            yspc=spc

        self.plots[0].set_extents(xmin=xmin,xmax=xmax,ymin=ymin,
                                  ymax=ymax,xspc=xspc,yspc=yspc)

        if with_legend == 1:
            self.plots[0].set_legend()

        ll=self.plots[0].create_layers()
        for item in ll:
            self.add_layer(item)

        xsize=float(self.get_width())
        ysize=float(self.get_height())            
        self.fit_extents(0.0,0.0,1.0,ysize/xsize)

    def get_xy_position(self,x,y,plotidx=0):
        """ Get the data position corresponding to GvViewPlot position x,y
            for the plot with index plotidx.
            Input:
               x- x value(s)
               y- y value(s)
               plotidx- index of plot to use in conversion, if more than
                        one plot is present in the view area.

            x and y may be single values or same-length 1-D arrays.

            Output:
                x- data x position
                y- data y position
                okarr- array indicating where the conversion is valid
                       (ie. within plot bounds)
        """
        x,y,okarr=self.plots[plotidx].get_xyposition(x,y)

        return (x,y,okarr)

    def get_plotposition(self,x,y,plotidx=0):
        """ Get the GvViewPlot position corresponding to data position x,y
            for plot plotidx.

            Input:
               x- x value(s)
               y- y value(s)
               plotidx- index of plot to use in conversion, if more than
                        one plot is present in the view area.

            x and y may be single values or same-length 1-D arrays.

            Output:
                x- plot x position
                y- plot y position
                okarr- array indicating where the conversion is valid
                       (ie. within plot bounds)
        """
        x,y,okarr=self.plots[plotidx].get_plotposition(x,y)

        return (x,y,okarr)

    def clear_layers(self):
        """ Remove all layers. """

        for clayer in self.list_layers():
            self.remove_layer(clayer)

    def clear(self):
        """ Clear all layers AND plot information. """

        for clayer in self.list_layers():
            self.remove_layer(clayer)

        self.plots=[]


class GvSimplePlotWindow(gtk.Window):

    def __init__(self,bgcolor=(1.0,1.0,1.0,1.0)):
        gtk.Window.__init__(self)
        self.plotarea=GvSimplePlot(bgcolor=bgcolor)
        #self.plotarea.fit_extents(0.0,0.0,1.0,0.84)
        self.set_resizable(True)
        shell = gtk.VBox(spacing=0)
        self.add(shell)
        self.set_size_request(650,545)
        
        # Print menu - using gtk instead of the pguMenuFactory - it was giving assertion errors
        uimanager = gtk.UIManager()
        self.add_accel_group(uimanager.get_accel_group())
        uimanager.add_ui_from_string(
                """<ui><menubar name='MenuBar'>
                          <menu action='File'>
                            <menuitem action='Print'/>
                    </menu></menubar></ui>          
                """)
        
        action_group = gtk.ActionGroup("main")
        action_group.add_actions([
            ('File', None, "File"),
            ('Print', gtk.STOCK_PRINT, '_Print', '<control>p', "Print", self.print_cb)
            ])
        uimanager.insert_action_group(action_group, 0)
        menu = uimanager.get_widget('/MenuBar')
        
#        menuf = pgumenu.pguMenuFactory()
#        self.menuf = menuf
#        menuf.add_entries([
#                 ('File/Print', None, self.print_cb)])

        shell.pack_start(menu, expand=False)

        shell.pack_start( self.plotarea )

        self.show_all()

        #self.viewarea.fit_extents(0, self.ysize, self.xsize, -self.ysize )

    def print_cb(self, *args):
        import gvprint
        pd = gvprint.GvPrintDialog( self.plotarea )

    def plot(self,yarr=None,xarr=None, xlabel=None, ylabel=None, xmin=None,
             xmax=None,ymin=None, ymax=None, xspc=None,yspc=None,
             title=None,datalabel=None,
             color=(1.0,0.0,0.0,1.0),drawstyle='_'):
        self.plotarea.plot(yarr,xarr,xlabel,ylabel,xmin,xmax,xspc,
                           ymin,ymax,yspc,title,
                           datalabel,color,drawstyle)

    def oplot(self,yarr=None,xarr=None, xmin=None,
             xmax=None, xspc=None,ymin=None, ymax=None,
              yspc=None, datalabel=None,
             color=(1.0,0.0,0.0,1.0),drawstyle='_',reset_extents=1,
              with_legend=0):
        self.plotarea.oplot(yarr,xarr,xmin,xmax,xspc,ymin,ymax,yspc,
                           datalabel,color,drawstyle,reset_extents,with_legend)

def DataToPlot1D(pmin,pmax,dmins,dmaxs,dpos,axistype=GVPLOT_AXISTYPE_LINEAR):
    """ Transform from data to plot coordinates.
        Input:
            pmin- plot minimum position not including borders (single value)
            pmax- plot maximum position not including borders (single value)
            dmins- data range minima (a list or tuple of at least length 1)
            dmaxs- data range maxima (a list or tuple of length(dmins))
            dpos- data positions to transform
                  (single value or 1-D array)
            axistype- type of axis (eg. linear or log).

        The dmin-dmax ranges must not be overlapping.

        Output:
            ppos- plot position (float or array)
            okarr- integer or array of integers indicating where data
                   is within the plot bounds (1's) or invalid (0's)
    """
    if axistype not in [GVPLOT_AXISTYPE_LINEAR,GVPLOT_AXISTYPE_LOG]:
        raise RuntimeError,'DataToPlot1D: unknown axis type!'

    ptype=type(dpos)
    if ptype != type(numpy.array([])):
        dpos=numpy.array(dpos,numpy.float64)

    # First check for length 1 case and do quick
    # transform and return if it is; otherwise, continue.

    if len(dmins) == 1:
        # Only one continuous range
        if axistype == GVPLOT_AXISTYPE_LINEAR:
            ppos=((float(pmax)-float(pmin))/
                  (float(dmaxs[0])-float(dmins[0])))*\
                  (dpos-float(dmins[0]))+float(pmin)

            okarr=numpy.where(ppos >= pmin,1,0)
            okarr=numpy.where(ppos <= pmax,okarr,0)            

        elif axistype == GVPLOT_AXISTYPE_LOG:
            # Make sure logs don't choke
            okarr=numpy.where(dpos > 0,1,0)
            dpos=numpy.where(okarr == 0,1,dpos)

            ppos=(((float(pmax)-float(pmin))/float(numpy.log10(dmaxs[0])-
                                          numpy.log10(dmins[0])))*\
                  (numpy.log10(dpos)-numpy.log10(dmins[0])))+pmin
            okarr=numpy.where(ppos >= pmin,okarr,0)
            okarr=numpy.where(ppos <= pmax,okarr,0)

        if ptype not in [type(numpy.array([])),type((1,)),type([])]:
            okarr=okarr[0]

        return (ppos,okarr)

    dmaxs=numpy.array(dmaxs,numpy.float64)
    dmins=numpy.array(dmins,numpy.float64)

    if axistype == GVPLOT_AXISTYPE_LINEAR:    
        dwidtharr=numpy.ravel(dmaxs-dmins)
        dwidth=numpy.sum(dwidtharr)
        dcsum=numpy.cumsum(dwidtharr)
        drng=numpy.array(list(dcsum/dwidth).insert(0,0.0))
        prng=pmin+(drng*(pmax-pmin)) # plot range endpoints

        ppos=numpy.zeros((len(dpos),),numpy.float64)
        okarr=numpy.zeros((len(dpos),),numpy.float64)

        for idx in range(len(dmins)):
            sc=numpy.where(dpos >= dmins[idx],1,0)
            sc=numpy.where(dpos <= dmaxs[idx],1,sc)
            okarr=numpy.where(sc == 1,1,okarr)
            sc=sc.astype(numpy.float64)

            ppos=ppos+(sc*(prng[idx]+((prng[idx+1]-prng[idx])*
                     (dpos-dmins[idx])/(dmaxs[idx+1]-dmins[idx]))))

    elif axistype == GVPLOT_AXISTYPE_LOG:
        dmaxs=numpy.log10(dmaxs)
        dmins=numpy.log10(dmins)

        # Make sure logs don't choke
        okarrmask=numpy.where(dpos > 0,1,0)
        dpos=numpy.where(okarrmask == 0,1,dpos)

        dpos=numpy.log10(dpos)

        dwidtharr=numpy.ravel(dmaxs-dmins)
        dwidth=numpy.sum(dwidtharr)
        dcsum=numpy.cumsum(dwidtharr)
        drng=numpy.array(list(dcsum/dwidth).insert(0,0.0))
        prng=pmin+(drng*(pmax-pmin)) # plot range endpoints

        ppos=numpy.zeros((len(dpos),),numpy.float64)
        okarr=numpy.zeros((len(dpos),),numpy.float64)

        for idx in range(len(dmins)):
            sc=numpy.where(dpos >= dmins[idx],1,0)
            sc=numpy.where(dpos <= dmaxs[idx],1,sc)
            okarr=numpy.where(sc == 1,1,okarr)
            sc=sc.astype(numpy.float64)

            ppos=ppos+(sc*(prng[idx]+((prng[idx+1]-prng[idx])*
                     (dpos-dmins[idx])/(dmaxs[idx+1]-dmins[idx]))))

        okarr=okarr*okarrmask

    if ptype not in [type(numpy.array([])),type((1,)),type([])]:
        okarr=okarr[0]

    return (ppos,okarr)

def PlotToData1D(pmin,pmax,dmins,dmaxs,ppos,axistype=GVPLOT_AXISTYPE_LINEAR):
    """ Transform from plot to data coordinates.
        Input:
            pmin- plot minimum position not including borders (single value)
            pmax- plot maximum position not including borders (single value)
            dmins- data range minima (a list or tuple of at least length 1)
            dmaxs- data range maxima (a list or tuple of length(dmins))
            ppos- plot positions to transform
                  (single value or 1-D array)
            axistype- type of axis (eg. linear or log).

        The dmin-dmax ranges must not be overlapping.

        Output:
            dpos- data position (float or array)
            okarr- integer or array of integers indicating where plot
                   position is within bounds (1's) or invalid (0's).
    """
    if axistype not in [GVPLOT_AXISTYPE_LINEAR,GVPLOT_AXISTYPE_LOG]:
        raise RuntimeError,'PlotToData1D: unknown axis type!'

    dtype=type(ppos)
    if dtype != type(numpy.array([])):
        ppos=numpy.array(ppos,numpy.float64)

    okarr=numpy.where(ppos >= pmin,1,0)
    okarr=numpy.where(ppos <= pmax,okarr,0)

    # First check for length 1 case and do quick
    # transform and return if it is; otherwise, continue.    
    if len(dmins) == 1:
        # Only one continuous range
        if axistype == GVPLOT_AXISTYPE_LINEAR:
            dpos=((float(dmaxs[0]-dmins[0])/float(pmax-pmin))*\
                  (ppos-pmin))+dmins[0]

        elif axistype == GVPLOT_AXISTYPE_LOG:
            dpos=numpy.power(10,((((numpy.log10(float(dmaxs[0]))-
                   numpy.log10(float(dmins[0])))/
                  float(pmax-pmin))*(ppos-pmin))+
                  numpy.log10(float(dmins[0]))))

        if dtype not in [type(numpy.array([])),type((1,)),type([])]:
            okarr=okarr[0]

        return (dpos,okarr)


    dmaxs=numpy.array(dmaxs,numpy.float64)
    dmins=numpy.array(dmins,numpy.float64)

    if axistype == GVPLOT_AXISTYPE_LINEAR:    
        dwidtharr=numpy.ravel(dmaxs-dmins)
        dwidth=numpy.sum(dwidtharr)
        dcsum=numpy.cumsum(dwidtharr)
        drng=numpy.array(list(dcsum/dwidth).insert(0,0.0))
        prng=pmin+(drng*(pmax-pmin)) # plot range endpoints
        pfrac=(float(ppos)-float(pmin))/(float(pmax)-float(pmin))
        dpos=numpy.zeros((len(ppos),),numpy.float64)
        for idx in range(len(dmins)):
            # If ppos is between dmins[idx] and dmaxs[idx],
            # sc will be 1; otherwise it will be 0
            sc=(numpy.sign(pfrac-drng[idx])+
                numpy.sign(drng[idx+1]-pfrac))
            # exact boundary of 2 ranges- use lower.
            sc=numpy.where(sc == -1,1,sc)
            sc=numpy.where(sc == 1,0,sc)
            sc=sc.astype(numpy.float64)/2.0
            dpos=dpos+(sc*(dmins[idx]+((dmaxs[idx+1]-dmins[idx])*
                        (ppos-prng[idx])/(prng[idx+1]-prng[idx]))))

    elif axistype == GVPLOT_AXISTYPE_LOG:
        dmaxs=numpy.log10(dmaxs)
        dmins=numpy.log10(dmins)
        dwidtharr=numpy.ravel(dmaxs-dmins)
        dwidth=numpy.sum(dwidtharr)
        dcsum=numpy.cumsum(dwidtharr)
        drng=numpy.array(list(dcsum/dwidth).insert(0,0.0))
        prng=pmin+(drng*(pmax-pmin)) # plot range endpoints
        pfrac=(float(ppos)-float(pmin))/(float(pmax)-float(pmin))
        dpos=numpy.zeros((len(ppos),),numpy.float64)
        for idx in range(len(dmins)):
            # If ppos is between dmins[idx] and dmaxs[idx],
            # sc will be 1; otherwise it will be 0
            sc=(numpy.sign(pfrac-drng[idx])+
                numpy.sign(drng[idx+1]-pfrac))
            # exact boundary of 2 ranges- use lower.
            sc=numpy.where(sc == -1,1,sc)
            sc=numpy.where(sc == 1,0,sc)
            sc=sc.astype(numpy.float64)/2.0
            dpos=dpos+(sc*(dmins[idx]+((dmaxs[idx+1]-dmins[idx])*
                        (ppos-prng[idx])/(prng[idx+1]-prng[idx]))))

        dpos=numpy.power(10,dpos)


    if dtype not in [type(numpy.array([])),type((1,)),type([])]:
        okarr=okarr[0]

    return (dpos,okarr)

def MakeContiguousXY(xarr,yarr):
    """ Arrange array data by ascending x. """
    longxlist=[]
    longylist=[]
    for idx in range(len(xarr)):
        longxlist.extend(xarr[idx])
        longylist.extend(yarr[idx])
    lxarr=numpy.array(longxlist) 
    lyarr=numpy.array(longylist)   
    ind=numpy.argsort(lxarr)
    nxarr=numpy.take(lxarr,ind)
    nyarr=numpy.take(lyarr,ind)

    return(nxarr,nyarr)

def GetOutlierData(xarr,xmins,xmaxs,yarr,ymins,ymaxs,
                   zarr=None,zmins=None,zmaxs=None):
    """ Get data that lies outside of the current plot boundaries.
        xarr, yarr, (optional zarr)- array, or list of arrays

        xmins/xmaxs- x boundaries (single value or list of values)
        ymins/ymaxs- y boundaries
        zmins/zmaxs (optional)- z boundaries

        Note: boundaries and data arrays are in data coordinates,
              not plot coordinates (xmins/xmaxs etc. are the plot
              boundaries converted to data coordinates).
    """
    if type(xarr) == type(numpy.array([1,2])):
        xarr=[xarr]
        yarr=[yarr]
        if zarr is not None:
            zarr=[zarr]

    if type(xmins) not in [type([]),type((1,))]:
        xmins=[xmins]
        xmaxs=[xmaxs]

    if type(ymins) not in [type([]),type((1,))]:
        ymins=[ymins]
        ymaxs=[ymaxs]

    if zarr is not None:        
        if type(zmins) not in [type([]),type((1,))]:
            zmins=[zmins]
            zmaxs=[zmaxs]

    nxarr=[]
    nyarr=[]
    nzarr=None
    if zarr is not None:
        nzarr=[]
    else:
        nzarr=None

    for idx in range(len(xarr)):
        tx=xarr[idx]
        ty=yarr[idx]
        if zarr is not None:
            tz=zarr[idx]
        xok=numpy.zeros(numpy.shape(tx))
        for xidx in range(len(xmins)):
           xok=numpy.where((tx >= xmins[xidx]) & (tx <= xmaxs[xidx]),1,xok)
        yok=numpy.zeros(numpy.shape(ty))
        for yidx in range(len(ymins)):
           yok=numpy.where((ty >= ymins[yidx]) & (ty <= ymaxs[yidx]),1,yok)
        if zarr is not None:
            for zidx in range(len(zmins)):
               zok=numpy.where((tz >= zmins[zidx]) & (tz <= zmaxs[zidx]),
                                 1,zok)
        outlier=numpy.where(xok == 0,1,0)
        outlier=numpy.where(yok == 0,1,outlier)
        if zarr is not None:
            outlier=numpy.where(zok == 0,1,outlier)

        nxarr.append(numpy.compress(outlier == 1,tx))
        nyarr.append(numpy.compress(outlier == 1,ty))
        if zarr is not None:              
            nzarr.append(numpy.compress(outlier == 1,tz))

    return (nxarr,nyarr,nzarr)

def GetAxisBreaks(mins,maxs,axis_type=GVPLOT_AXISTYPE_LINEAR,bwidth=0.1):
    """ Get the axis discontinuity info.
        mins- axis range minima.
        maxs- axis range maxima.
        axis_type- type of axis (linear or log)
        bwidth- break width as a percentage of axis length.
    """
    if len(mins) == 1:
        return None

    maxs=numpy.array(maxs)
    mins=numpy.array(mins)

    if axis_type == GVPLOT_AXISTYPE_LOG:
        maxs=numpy.log10(maxs)
        mins=numpy.log10(mins)

    dwidtharr=numpy.ravel(maxs-mins)
    dwidth=numpy.sum(dwidtharr)
    dcsum=numpy.cumsum(dwidtharr)
    drng=numpy.array(list(dcsum/dwidth))
    drng=drng[:len(drng)-1]
    bstarts=drng-(bwidth/2.0)
    bends=drng+(bwidth/2.0)
    bstarts=numpy.where(bstarts < 0.0,0.0,bstarts)
    bends=numpy.where(bends > 1.0,1.0,bends)
    diff=bstarts[1:]-bends[:-1]
    bstarts[1:]=numpy.where(diff < 0,bstarts[1:]+diff/2.0,bstarts[1:])
    bends[:-1]=numpy.where(diff < 0,bends[:-1]-diff/2.0,bends[:-1])

    breaks=[]
    for idx in range(len(bstarts)):
        breaks.append((bstarts[idx],bends[idx]))

    return breaks


def GetNiceMinMax(mins,maxs,pref_spacing,axis_type=GVPLOT_AXISTYPE_LINEAR):
    """ Get 'nice' min/max values that enclose the given range,
        and return the nice min/max along with a list of data
        offsets for major tics, and a formatting string for the
        labels.
    """
    if type(mins) not in [type([]),type((1,))]:
        mins=[mins]
    if type(maxs) not in [type([]),type((1,))]:
        maxs=[maxs]

    if axis_type == GVPLOT_AXISTYPE_LINEAR:
        dmins=[]
        dmaxs=[]
        labels=[]
        for idx in range(len(mins)):
            tmin,tmax,offsets,fmt=GetNiceLinearMinMax(mins[idx],
                                           maxs[idx],pref_spacing)
            dmins.append(tmin)
            dmaxs.append(tmax)
            labels.extend(offsets)

    return (dmins,dmaxs,labels,fmt)

def GetNiceLinearMinMax(minval,maxval,pref_spacing=None):
    """ Linear axis case:
        Get 'nice' max/min values that enclose the given range,
        and return the nice min/max along with a list of data
        offsets for major tics, and a formatting string for the
        labels.
    """
    if abs(maxval-minval) > 0:
        rord=numpy.log10(abs(maxval-minval))
    else:
        maxval=maxval*1.1
        minval=minval*0.9
        if maxval == 0:
            maxval=1.0
            minval=-1.0
        rord=numpy.log10(abs(maxval-minval))

    nrord=rord % 1

    # format
    if rord > 1:
        fmt="%d"
    else:
        fmt="%g"
        #prec=int(abs(rord))+1
        #fmt="%."+str(prec)+"f"

    if pref_spacing is None:
        if nrord < numpy.log10(2):
            spc=0.2*pow(10,numpy.floor(rord))
        elif nrord < numpy.log10(5):
            spc=0.5*pow(10,numpy.floor(rord))
        else:
            spc=pow(10,numpy.floor(rord))
    else:
        spc=pref_spacing

    tmp1=(abs(minval) % spc)/spc
    new_min=minval - (minval % spc)

    new_max=numpy.floor((maxval-new_min)/spc)*spc+new_min

    if maxval-new_max > (float(spc)/10000.0):
        new_max=new_max+spc

    label_offsets=[]
    for label_val in numpy.arange(new_min,new_max+spc/100.0,spc):
        label_offsets.append(label_val)

    return (new_min,new_max,label_offsets,fmt)


class GvSimpleSettingsVBox(gtk.VBox,Signaler):
    """ Class to hold a number of plot-related settings.
        Inputs:
            plotarea- GvSimplePlot to act on
            plot_num- index to plot within plotarea to apply changes to
            include_list- tuple of which tables to include:
                          0- rescale table
                          1- Apply button

            withx,xminlabel,xmaxlabel,
            withy,yminlabel,ymaxlabel,
            withz,zminlabel,zmaxlabel- rescale table settings.

            applylabel- apply button label

        Sends out a 'plot-updated' signal when plot has been
        changed.
    """
    def __init__(self,plotarea=None,plot_num=0,include_list=(0,1),
                 withx=1,xminlabel='X Min',xmaxlabel='X Max',
                 withy=1,yminlabel='Y Min',ymaxlabel='Y Max',
                 withz=0,zminlabel='Z Min',zmaxlabel='Z Max',
                 applylabel='Apply'
                 ):
        gtk.VBox.__init__(self)
        self.set_border_width(5)
        self.set_spacing(5)
        self.plotarea=plotarea
        self.plot_num=plot_num
        self.rescale_table=None
        self.apply_button=None
        for item in include_list:
            if item == 0:
                self.rescale_table=GvSimpleRescaleTable(
                    withx=withx,xminlabel=xminlabel,xmaxlabel=xmaxlabel,
                    withy=withy,yminlabel=yminlabel,ymaxlabel=ymaxlabel,
                    withz=withz,zminlabel=zminlabel,zmaxlabel=zmaxlabel)
                self.pack_start(self.rescale_table)
            elif item == 1:
                self.apply_button=gtk.Button(applylabel)
                self.pack_start(self.apply_button,expand=False)

        if self.apply_button is not None:
            self.apply_button.connect("clicked",self.apply_cb)

        self.publish('plot-updated')

    def apply_cb(self,*args):
        """ Update plot. """
        # If plot was interactive, draw changes back in.
        for idx in range(len(self.plotarea.plots)):
            cplot=self.plotarea.plots[idx]
            for idx2 in range(len(cplot.array_data)):
                if cplot.array_layer_defaults[idx2].interactive == 1:
                    cplot.merge_data_from_plot(idx2)

        if self.rescale_table is not None:
            ext=self.rescale_table.get_settings()      
            self.plotarea.plots[self.plot_num].set_extents(xmin=ext[0],
                  xmax=ext[1],ymin=ext[2],ymax=ext[3])
        ll=self.plotarea.plots[self.plot_num].create_layers()
        self.plotarea.clear_layers()
        for item in ll:
            self.plotarea.add_layer(item)
        xsize=float(self.plotarea.get_width())
        ysize=float(self.plotarea.get_height())            
        self.plotarea.fit_extents(0.0,0.0,1.0,ysize/xsize)
        Signaler.notify(self, 'plot-updated')


class GvSimpleRescaleTable(gtk.Table):
    """ Class to provide a table for rescaling a GvSimplePlot.
        Inputs:
            withx- 1 to include x in table, 0 to not include
            xmin- minimum x (None to start from defaults)
            xmax- maximum x
            withy- 1 to include y in table, 0 to not include
            ymin- minimum y
            ymax- maximum y
            withz- 1 to include y in table, 0 to not include
            zmin- minimum y
            zmax- maximum y
    """
    def __init__(self,withx=1,xmin=None,xmax=None,
                 withy=1,ymin=None,ymax=None,
                 withz=1,zmin=None,zmax=None,xminlabel='X Min',
                 xmaxlabel='X Max',yminlabel='Y Min', ymaxlabel='Y Max',
                 zminlabel='Z Min',zmaxlabel='Z Max'):
        numrows=withx+withy+withz
        gtk.Table.__init__(self,numrows,5)
        self.set_row_spacings(5)
        self.set_col_spacings(5)
        self.xminentry=None
        self.xmaxentry=None
        self.yminentry=None
        self.ymaxentry=None
        self.zminentry=None
        self.zmaxentry=None
        c_row=0
        if withx == 1:
            self.attach(gtk.Label(xminlabel),0,1,0,1)
            self.xminentry=gtk.Entry()
            self.xminentry.set_editable(True)
            if xmin is not None:
                self.xminentry.set_text(str(xmin))
            else:
                self.xminentry.set_text('')
            self.attach(self.xminentry,1,2,0,1)
            self.attach(gtk.Label(xmaxlabel),2,3,0,1)
            self.xmaxentry=gtk.Entry()
            self.xmaxentry.set_editable(True)
            if xmax is not None:
                self.xmaxentry.set_text(str(xmax))
            else:
                self.xmaxentry.set_text('')
            self.attach(self.xmaxentry,3,4,0,1)
            c_row=c_row+1

        if withy == 1:
            self.attach(gtk.Label(yminlabel),0,1,c_row,c_row+1)
            self.yminentry=gtk.Entry()
            self.yminentry.set_editable(True)
            if ymin is not None:
                self.yminentry.set_text(str(ymin))
            else:
                self.yminentry.set_text('')
            self.attach(self.yminentry,1,2,c_row,c_row+1)
            self.attach(gtk.Label(ymaxlabel),2,3,c_row,c_row+1)
            self.ymaxentry=gtk.Entry()
            self.ymaxentry.set_editable(True)
            if ymax is not None:
                self.ymaxentry.set_text(str(ymax))
            else:
                self.ymaxentry.set_text('')
            self.attach(self.ymaxentry,3,4,c_row,c_row+1)
            c_row=c_row+1

        if withz == 1:
            self.attach(gtk.Label(zminlabel),0,1,c_row,c_row+1)
            self.zminentry=gtk.Entry()
            self.zminentry.set_editable(True)
            if zmin is not None:
                self.zminentry.set_text(str(zmin))
            else:
                self.zminentry.set_text('')
            self.attach(self.zminentry,1,2,c_row,c_row+1)
            self.attach(gtk.Label(zmaxlabel),2,3,c_row,c_row+1)
            self.zmaxentry=gtk.Entry()
            self.zmaxentry.set_editable(True)
            if zmax is not None:
                self.zmaxentry.set_text(str(zmax))
            else:
                self.zmaxentry.set_text('')
            self.attach(self.zmaxentry,3,4,c_row,c_row+1)
            c_row=c_row+1

    def get_settings(self):
        """ Return the current settings as a
            (xmin,xmax,ymin,ymax,zmin,zmax) tuple.
            If any aren't present or aren't entered,
            set them to None.
        """
        try:
            xmin=float(self.xminentry.get_text())
        except:
            xmin=None
        try:
            xmax=float(self.xmaxentry.get_text())
        except:
            xmax=None
        try:
            ymin=float(self.yminentry.get_text())
        except:
            ymin=None
        try:
            ymax=float(self.ymaxentry.get_text())
        except:
            ymax=None
        try:
            zmin=float(self.zminentry.get_text())
        except:
            zmin=None
        try:
            zmax=float(self.zmaxentry.get_text())
        except:
            zmax=None

        return (xmin,xmax,ymin,ymax,zmin,zmax)


    def set_settings(self, xmin = None, xmax = None, ymin = None, ymax = None, zmin = None, zmax = None ):
        """ Sets the value xmin,xmax,ymin,ymax,zmin,zmax
            in the table entries.
            A value that is None, has its entry left blank.
        """

        if xmin is not None and self.xminentry is not None:
            self.xminentry.set_text(str(xmin))

        if xmax is not None and self.xmaxentry is not None:
            self.xmaxentry.set_text(str(xmax))

        if ymin is not None and self.yminentry is not None:
            self.yminentry.set_text(str(ymin))

        if ymax is not None and self.ymaxentry is not None:
            self.ymaxentry.set_text(str(ymax))

        if zmin is not None and self.zminentry is not None:
            self.zminentry.set_text(str(zmin))

        if zmax is not None and self.zmaxentry is not None:
            self.zmaxentry.set_text(str(zmax))

        return

if __name__ == '__main__':
    yarr=numpy.arange(20000,typecode='f')/1000.0
    yarr2=numpy.arange(20000,typecode='f')/2000.0
    yarr3=numpy.arange(20000,typecode='f')/1500.0
    yarr=numpy.sin(yarr)
    yarr2=2*numpy.sin(yarr2)
    yarr3=1.5*numpy.sin(yarr3)

    win=GvSimplePlotWindow()
    win.connect('delete-event',gtk.main_quit)
    win.plot(yarr,xlabel='x',ylabel='y',title='y=sin(x)')
    win.show()
    win_settings=gtk.Window()
    win_settings.add(GvSimpleSettingsVBox(win.plotarea))
    win_settings.show_all()

    win2=GvSimplePlotWindow(bgcolor=(0.0,0.0,0.0,1.0))
    win2.show()
    win2.plot(yarr,xlabel='x',ylabel='y',title='y=sin(x)')

    win3=GvSimplePlotWindow()
    win3.show()
    win3.plot(yarr,xmax=5000)
    win3.oplot(yarr2,color=(0.0,0.0,1.0,1.0),reset_extents=0)
    win3.oplot(yarr3,color=(0.0,1.0,0.0,1.0),reset_extents=0)

    win4=GvSimplePlotWindow()
    win4.show()
    win4.plot(yarr,xmin=17010,xmax=19031)
    win4.oplot(yarr2,color=(0.0,0.0,1.0,1.0),datalabel='hi',reset_extents=0)
    win4.oplot(yarr3,color=(0.0,1.0,0.0,1.0),xmin=17010,xmax=19031,
               with_legend=1)

    xg=numpy.zeros((20,20),numpy.Float16)
    yg=numpy.zeros((20,20),numpy.Float16)
    zg=numpy.zeros((20,20),numpy.Float16)
    for idx1 in range(20):
        for idx2 in range(20):
            xg[idx1,idx2]=idx1
            yg[idx1,idx2]=idx2
    zg=numpy.sin(xg*numpy.pi/10)*numpy.sin(yg*numpy.pi/10)
    win5=GvSimplePlotWindow()
    win5.show()
    win5.plotarea.plot3D(xg,yg,zg)

    gtk.main()
