#! /usr/bin/env python
##############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Grid layers, north arrows, etc.
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

import gview
import Numeric
import os
import gvogrfs

#########################################################################
# Grid/graticule layers                                                 #
#########################################################################

def SimpleReferenceGrid(min_x,min_y,max_x,max_y,x_divisions,y_divisions,
                        color=(0.5,1.0,0.5,1.0),xoff=-0.15,yoff=-0.04,
                        label_type=None,shapes_name="Grid"):
    """ Create a reference grid for an unprojected raster.
        min_x, min_y- minimum lat/longs, in decimal degrees
        max_x, max_y- minimum lat/longs, in decimal degrees
        x_divisions- number of divisions in horizontal direction
        y_divisions- number of divisions in vertical direction
        xoff- horizontal offset of vertical labels, as a fraction
              of max_x-min_x.  Offset is relative to min_x.
        yoff- vertical offset of horizontal labels, as a fraction
              of max_y-min_y.  Offset is relative to min_y.
        color- start color for the grid
        label_type- not used yet; might be later for formatting.

    """

    shps=gview.GvShapes(name=shapes_name)
    gview.undo_register( shps )
    shps.add_field('position','string',20)

    if os.name == 'nt':
        font="-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"
    else:
        #font="-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"
        #font="-urw-helvetica-medium-r-normal-*-9-*-*-*-p-*-iso8859-2"
        font="-adobe-helvetica-medium-r-normal-*-8-*-*-*-p-*-iso10646-1"
        #font="-misc-fixed-medium-r-*-*-9-*-*-*-*-*-*-*"


    lxoff=(max_x-min_x)*xoff  # horizontal label placement
    lyoff=(max_y-min_y)*yoff # vertical label placement

    hspc=(max_x-min_x)/x_divisions
    vspc=(max_y-min_y)/y_divisions

    for hval in Numeric.arange(min_x,max_x+hspc/100.0,hspc):
        nshp=gview.GvShape(type=gview.GVSHAPE_LINE)
        nshp.set_node(hval,max_y,0,0)
        nshp.set_node(hval,min_y,0,1)
        shps.append(nshp)
        pshp=gview.GvShape(type=gview.GVSHAPE_POINT)
        pshp.set_node(hval,min_y+lyoff)
        pshp.set_property('position',"%.1f" % hval)
        shps.append(pshp)

    for vval in Numeric.arange(min_y,max_y+vspc/100.0,vspc):
        nshp=gview.GvShape(type=gview.GVSHAPE_LINE)
        nshp.set_node(min_x,vval,0,0)
        nshp.set_node(max_x,vval,0,1)
        shps.append(nshp)
        pshp=gview.GvShape(type=gview.GVSHAPE_POINT)
        pshp.set_node(min_x+lxoff,vval)
        pshp.set_property('position',"%.1f" % vval)
        shps.append(pshp)

    cstr=gvogrfs.gv_to_ogr_color(color)
    if len(cstr) < 9:
        cstr=cstr+"FF"
    clstr=str(color[0])+' '+str(color[1])+' '+str(color[2])+' '+str(color[3])

    layer=gview.GvShapesLayer(shps)
    layer.set_property('_line_color',clstr)
    layer.set_property('_point_color',clstr)
    # Set antialias property so that lines look nice
    # when rotated.
    layer.set_property('_gl_antialias','1')
    layer.set_property('_gv_ogrfs_point',
                       'LABEL(t:{position},f:"'+font+'",c:'+cstr+')')
    layer.set_read_only(True)    

    return layer


def SimpleMeasuredGrid(min_x,min_y,max_x,max_y,x_spacing,y_spacing,
                        color=(0.5,1.0,0.5,1.0),xoff=-0.14,yoff=1.04,
                        label_type=None,shapes_name="Grid"):
    """ Create a reference grid for a utm-projected raster.
        min_x, min_y, max_x, max_y- extents that grid should cover
        x_spacing- line spacing in horizontal direction
        y_spacing- line spacing in vertical direction
        xoff- horizontal offset of vertical labels, as a fraction
              of max_x-min_x.  Offset is relative to min_x.
        yoff- vertical offset of horizontal labels, as a fraction
              of max_y-min_y.  Offset is relative to min_y.
        color- start color for the grid
        label_type- not used yet; might be later for formatting.
        shapes_name- name to give the shapes forming the layer.

    """

    shps=gview.GvShapes(name=shapes_name)
    gview.undo_register( shps )
    shps.add_field('position','string',20)

    if os.name == 'nt':
        font="-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"
    else:
        #font="-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"
        #font="-urw-helvetica-medium-r-normal-*-9-*-*-*-p-*-iso8859-2"
        font="-adobe-helvetica-medium-r-normal-*-8-*-*-*-p-*-iso10646-1"
        #font="-misc-fixed-medium-r-*-*-9-*-*-*-*-*-*-*"


    # Round to nearest integer space
    max_x=min_x+Numeric.floor((max_x-min_x)/x_spacing)*x_spacing
    max_y=min_y+Numeric.floor((max_y-min_y)/y_spacing)*y_spacing

    lxoff=(max_x-min_x)*xoff  # horizontal label placement
    lyoff=(max_y-min_y)*yoff # vertical label placement

    for hval in Numeric.arange(min_x,
                               max_x+x_spacing/100.0,
                               x_spacing):
        nshp=gview.GvShape(type=gview.GVSHAPE_LINE)
        nshp.set_node(hval,max_y,0,0)
        nshp.set_node(hval,min_y,0,1)
        shps.append(nshp)
        pshp=gview.GvShape(type=gview.GVSHAPE_POINT)
        pshp.set_node(hval,min_y+lyoff)
        pshp.set_property('position',"%d" % int(hval+0.5))
        shps.append(pshp)

    for vval in Numeric.arange(min_y,
                               max_y+y_spacing/100.0,
                               y_spacing):
        nshp=gview.GvShape(type=gview.GVSHAPE_LINE)
        nshp.set_node(min_x,vval,0,0)
        nshp.set_node(max_x,vval,0,1)
        shps.append(nshp)
        pshp=gview.GvShape(type=gview.GVSHAPE_POINT)
        pshp.set_node(min_x+lxoff,vval)
        pshp.set_property('position',"%d" % int(vval+0.5))
        shps.append(pshp)

    cstr=gvogrfs.gv_to_ogr_color(color)
    if len(cstr) < 9:
        cstr=cstr+"FF"
    clstr=str(color[0])+' '+str(color[1])+' '+str(color[2])+' '+str(color[3])

    layer=gview.GvShapesLayer(shps)
    layer.set_property('_line_color',clstr)
    layer.set_property('_point_color',clstr)
    # Set antialias property so that lines look nice
    # when rotated.
    layer.set_property('_gl_antialias','1')
    layer.set_property('_gv_ogrfs_point',
                       'LABEL(t:{position},f:"'+font+'",c:'+cstr+')')
    layer.set_read_only(True)    

    return layer


def SimpleLatLongGrid(min_x,min_y,max_x,max_y,hdeg,hmin,hsec,vdeg,vmin,vsec,
                        color=(0.5,1.0,0.5,1.0),xoff=-0.18,yoff=1.04,
                        label_type=None,shapes_name="Grid"):
    """ Create a reference graticule.
        min_x, min_y- minimum lat/longs, in decimal degrees
        max_x, max_y- minimum lat/longs, in decimal degrees
        hdeg/hmin/hsec- horizontal spacing (degrees/min/sec)
        vdeg/vmin/vsec- vertical spacing (degrees/min/sec)

        decimal degrees=degrees+(minutes/60.0)+(seconds/3600.0)

        xoff- horizontal offset of vertical labels, as a fraction
              of max_x-min_x.  Offset is relative to min_x.
        yoff- vertical offset of horizontal labels, as a fraction
              of max_y-min_y.  Offset is relative to min_y.

        color- start color for the grid
        label_type- not used yet; might be later for formatting.

        extents may be shifted slightly to generate 'nice' labels.

        Note that the min_x, min_y, max_x, max_y extents include
        a border 5% in from each side, and room for labels.        
    """

    shps=gview.GvShapes(name=shapes_name)
    gview.undo_register( shps )
    shps.add_field('position','string',20)

    if os.name == 'nt':
        font="-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"
    else:
        #font="-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"
        #font="-urw-helvetica-medium-r-normal-*-9-*-*-*-p-*-iso8859-2"
        font="-adobe-helvetica-medium-r-normal-*-8-*-*-*-p-*-iso10646-1"
        #font="-misc-fixed-medium-r-*-*-9-*-*-*-*-*-*-*"

    x_spacing=float(hdeg)+(float(hmin)+(float(hsec)/60.0))/60.0
    y_spacing=float(vdeg)+(float(vmin)+(float(vsec)/60.0))/60.0


    # Round to nearest integer space
    max_x=min_x+Numeric.floor((max_x-min_x)/x_spacing)*x_spacing
    max_y=min_y+Numeric.floor((max_y-min_y)/y_spacing)*y_spacing

    lxoff=(max_x-min_x)*xoff  # horizontal label placement
    lyoff=(max_y-min_y)*yoff # vertical label placement

    for hval in Numeric.arange(min_x,
                               max_x+x_spacing/100.0,
                               x_spacing):
        nshp=gview.GvShape(type=gview.GVSHAPE_LINE)
        nshp.set_node(hval,max_y,0,0)
        nshp.set_node(hval,min_y,0,1)
        shps.append(nshp)
        pshp=gview.GvShape(type=gview.GVSHAPE_POINT)
        pshp.set_node(hval,min_y+lyoff)
        hstr=GetLatLongString(hval,'longitude')
        pshp.set_property('position',hstr)
        shps.append(pshp)

    for vval in Numeric.arange(min_y,
                               max_y+y_spacing/100.0,
                               y_spacing):
        nshp=gview.GvShape(type=gview.GVSHAPE_LINE)
        nshp.set_node(min_x,vval,0,0)
        nshp.set_node(max_x,vval,0,1)
        shps.append(nshp)
        pshp=gview.GvShape(type=gview.GVSHAPE_POINT)
        pshp.set_node(min_x+lxoff,vval)
        vstr=GetLatLongString(vval,'latitude')
        pshp.set_property('position',vstr)
        shps.append(pshp)

    cstr=gvogrfs.gv_to_ogr_color(color)
    if len(cstr) < 9:
        cstr=cstr+"FF"
    clstr=str(color[0])+' '+str(color[1])+' '+str(color[2])+' '+str(color[3])

    layer=gview.GvShapesLayer(shps)
    layer.set_property('_line_color',clstr)
    layer.set_property('_point_color',clstr)
    # Set antialias property so that lines look nice
    # when rotated.
    layer.set_property('_gl_antialias','1')
    layer.set_property('_gv_ogrfs_point',
                       'LABEL(t:{position},f:"'+font+'",c:'+cstr+')')
    layer.set_read_only(True)

    return layer

#########################################################################
# North Arrow layers                                                    #
#########################################################################

if os.name == "nt":
    GVNORTHSYM1 = "\\North1"
else:
    GVNORTHSYM1 = "/North1"

def CreateNorthSymbol(ntype=GVNORTHSYM1,color1=(0.0,0.0,0.0,1.0),
                      color2=(1.0,1.0,1.0,1.0),scale=1.0,symbol_manager=None):
    """ Create the North Symbol and put it in a symbol manager.
        Input:
            ntype- type of north arrow to create
            color1- first color
            color2- second color (if needed)
            scale- amount to scale size by.
            symbol_manager- symbol manager to inject symbol into (a
                            new one will be created if this is set
                            to None).
    """

    if symbol_manager is None:
        sm=gview.GvSymbolManager()
    else:
        sm=symbol_manager

    cstr1=gvogrfs.gv_to_ogr_color(color1)
    if len(cstr1) < 9:
        cstr1=cstr1+"FF"

    cstr2=gvogrfs.gv_to_ogr_color(color2)
    if len(cstr2) < 9:
        cstr2=cstr2+"FF"

    sstr = str(scale).replace('.','_')

    refname=ntype+cstr1[1:]+cstr2[1:]+sstr
    if ntype==GVNORTHSYM1:   
        shape=gview.GvShape(type=gview.GVSHAPE_AREA)
        shape.set_node(1.0*scale,-2.6*scale,node=0)
        shape.set_node(0.0,-0.8*scale,node=1)
        shape.set_node(-1.0*scale,-2.6*scale,node=2)
        shape.set_node(0.0,2.6*scale,node=3)
        shape.set_node(1.0*scale,-2.6*scale,node=4)
        shape.set_property('_gv_ogrfs','PEN(c:'+cstr1+');BRUSH(c:'+\
                           cstr2+')')
        sm.inject_vector_symbol(refname,shape)

    return (refname,sm)

def CreateNorthSymbols(color1=(0.0,0.0,0.0,1.0),color2=(1.0,1.0,1.0,1.0),
                       scale=1.0,symbol_manager=None):
    """ Create North symbols of all types using two specified colors,
        and inject them into a symbol manager.
        Input:
            color1- first color
            color2- second color (if needed)
            scale- amount to scale size by.
            symbol_manager- symbol manager to inject symbols into (a
                            new one will be created if this is set
                            to None).
    """

    if symbol_manager is None:
        sm=gview.GvSymbolManager()
    else:
        sm=symbol_manager

    refnames=[]
    for item in [GVNORTHSYM1]:
        rname,junk=CreateNorthSymbol(color1,color2,scale,sm)
        refnames.append(rname)

    return (refnames,sm)


def SimpleNorthLayer(xoffset,yoffset,ntype=GVNORTHSYM1,
                        color1=(0.0,0.0,0.0,1.0),
                        color2=(1.0,1.0,1.0,1.0),
                        scale=1.0,
                     shapes_name="North Arrow"):
    """ Create a layer with a North arrow symbol,
        with the North arrow located at (xoffset,yoffset).
        The 'ntype' parameter will eventually be used for
        different types of north arrows.  The layer
        will contain two shapes: an area or line, and
        label.
        Input:
            xoffset,yoffset- where to center North Arrow (in
                             display coordinates).

            ntype- index of north arrow type (currently only one type).
            color1- First color and outline color for north arrow. A tuple
                    of 4 values between 0 and 1.
            color2- Second color (not used yet).
            scale- amount to scale size of symbol by.
            shapes_name- name to give the shapes that form
                         the north arrow.
    """

    shps=gview.GvShapes(name=shapes_name)
    gview.undo_register( shps )

    nshp=gview.GvShape(type=gview.GVSHAPE_POINT)
    nshp.set_node(xoffset,yoffset)

    cstr1=gvogrfs.gv_to_ogr_color(color1)
    if len(cstr1) < 9:
        cstr1=cstr1+"FF"

    refname,sm=CreateNorthSymbol(ntype,color1,color2,scale)
    dxstr=str(-1.5*scale)
    dystr=str(-15.0*scale)
    nshp.set_property('_gv_ogrfs',
                      'SYMBOL(c:'+cstr1+',s:4,id:"'+refname+'");'+\
                      'LABEL(c:'+cstr1+',t:"N",dx:'+dxstr+\
                      ',dy:'+dystr+')' )
    shps.append(nshp)

    layer=gview.GvShapesLayer(shps)
    # Set antialias property so that lines look nice
    # when rotated.
    layer.set_property('_gl_antialias','1')

    return layer

#############################################################
# Scale bar layers                                          #
#############################################################

# Types of scale bars
GVSCALE1=0

def SimpleScalebarLayer(xoffset,yoffset,dwidth,swidth,
                        angle,units_label=None,stype=GVSCALE1,
                        color1=(0.0,0.0,0.0,1.0),
                        color2=(1.0,1.0,1.0,1.0),
                        offset=-0.2,
                        shapes_name="Scale Bar"):
    """ Create a layer with a Scale bar located at
        stretching from dmin to dmax on the display
        Input:
            xoffset,yoffset- where to center scale bar (in
                             display coordinates)

            dwidth- width of scale bar in display coordinates
            swidth- width of scale bar in scale coordinates
                    (same if scale units and geocoding units
                    are the same; different if for example
                    display is UTM- meters- and scale bar is
                    in km)
            angle- angle of scale bar relative to display
                   (in RADIANS)
            units_label- label for units (left out if None)
            stype- index of scale bar type (currently must be 0)
            color1- First color and outline color for scale bar. A tuple
                    of 4 values between 0 and 1.
            color2- Second color (only used in alternating scale bars)
            offset- Vertical offset of labels for scale bar as a
                    fraction of scale bar width.
            shapes_name- name to give the shapes that form
                         the scalebar.
    """

    shps=gview.GvShapes(name=shapes_name)
    gview.undo_register( shps )
    shps.add_field('label','string',20)

    if os.name == 'nt':
        font="-adobe-helvetica-medium-r-*-*-15-*-*-*-*-*-*-*"
    else:
        #font="-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"
        #font="-urw-helvetica-medium-r-normal-*-9-*-*-*-p-*-iso8859-2"
        font="-adobe-helvetica-medium-r-normal-*-8-*-*-*-p-*-iso10646-1"
        #font="-misc-fixed-medium-r-*-*-9-*-*-*-*-*-*-*"

    sc=dwidth/swidth
    svals,labels=GetScaleBlocks(swidth)

    cstr1=str(color1[0])+' '+str(color1[1])+' '+str(color1[2])+\
           ' '+str(color1[3])
    cstr2=str(color2[0])+' '+str(color2[1])+' '+str(color2[2])+\
           ' '+str(color2[3])

    if stype == GVSCALE1:
        # Rectangle with alternating filled/unfilled
        # sections.
        smax=svals[len(svals)-1]

        # rectangle nodes before rotation
        hbr=(svals-(smax/2.0))*sc # horizontal- shift to -smax/2:+smax/2
                                  # so that rectangle is centered about 0.
        # rectangle extends from -smax/20 (bbr) to +smax/20 (tbr) vertically,
        # labels are placed at + or - smax/5
        tbr=smax/20.0*Numeric.ones(Numeric.shape(hbr))
        bbr=-1*smax/20.0*Numeric.ones(Numeric.shape(hbr))
        lbr=offset*smax*Numeric.ones(Numeric.shape(hbr))

        # units label location before rotation
        uxbr=(hbr[len(hbr)-1]-hbr[0])*0.05+hbr[len(hbr)-1]

        # rotate
        ctheta=Numeric.cos(angle)
        stheta=Numeric.sin(angle)
        tx=hbr*ctheta-tbr*stheta + xoffset
        ty=hbr*stheta+tbr*ctheta + yoffset
        bx=hbr*ctheta-bbr*stheta + xoffset
        by=hbr*stheta+bbr*ctheta + yoffset
        lx=hbr*ctheta-lbr*stheta + xoffset
        ly=hbr*stheta+lbr*ctheta + yoffset
        ux=uxbr*ctheta + xoffset
        uy=uxbr*stheta + yoffset

        # LATER: once shape collections are working, use them instead
        # so that entire scale bar can be selected and shifted as
        # a whole rather than separate shapes...

        #shp=gview.GvShape(type=gview.GVSHAPE_COLLECTION)
        #shp.set_property('_gv_ogrfs_point',
        #               'LABEL(t:{label},f:"%s",c:#000000FF)' % font)
        #shps.append(shp)
        for idx in range(len(tx)-1):
            nshp=gview.GvShape(type=gview.GVSHAPE_AREA)
            nshp.add_node(tx[idx],ty[idx],0)
            nshp.add_node(tx[idx+1],ty[idx+1],0)
            nshp.add_node(bx[idx+1],by[idx+1],0)
            nshp.add_node(bx[idx],by[idx],0)
            nshp.add_node(tx[idx],ty[idx],0)
            if idx % 2:
                nshp.set_property('_gv_color',cstr1)
                nshp.set_property('_gv_fill_color',cstr2)
            else:
                nshp.set_property('_gv_color',cstr1)
                nshp.set_property('_gv_fill_color',cstr1)

            shps.append(nshp)

        for idx in range(len(lx)):      
            if labels[idx] is not None:
                lshp=gview.GvShape(type=gview.GVSHAPE_POINT)
                lshp.set_node(lx[idx],ly[idx])
                lshp.set_property('label',labels[idx])
                shps.append(lshp)

        if units_label is not None:
            lshp=gview.GvShape(type=gview.GVSHAPE_POINT)
            lshp.set_node(ux,uy)
            lshp.set_property('label',units_label)
            shps.append(lshp)

    layer=gview.GvShapesLayer(shps)
    # Set antialias property so that lines look nice
    # when rotated.
    layer.set_property('_gl_antialias','1')
    cstr3=gvogrfs.gv_to_ogr_color(color1)
    if len(cstr3) < 9:
        cstr3=cstr3+"FF"

    layer.set_property('_gv_ogrfs_point',
                       'LABEL(t:{label},f:"%s",c:%s)' % (font,cstr3))
    return layer

def GetScaleBlocks(width):
    """ Get 'nice' scale bar block sizes and start/end
        values.
        Input:
            width- geocoded width

        Output:
            values- geocoded values at divisions (array)
            labels- labels for divisions
    """

    rord=Numeric.log10(abs(width)/2.0)
    nrord=rord % 1

    if nrord < Numeric.log10(2):
        spc=0.2*pow(10,Numeric.floor(rord))
        smallspc=spc
        bigspc=5*spc
        newspc=[0,smallspc,smallspc*2,smallspc*3,smallspc*4,smallspc*5]
    elif nrord < Numeric.log10(5):
        spc=0.5*pow(10,Numeric.floor(rord))
        smallspc=spc
        bigspc=5*spc
        newspc=[0,smallspc,smallspc*2,smallspc*3,smallspc*4]
    else:
        spc=pow(10,Numeric.floor(rord))
        smallspc=spc
        bigspc=spc*5
        newspc=[0,smallspc,smallspc*2,smallspc*3,smallspc*4,smallspc*5]

    if len(newspc) == 5:
        #labels=['0',None,"%g" % smallspc*2,None,"%g" % (smallspc*4)]
        labels=['0',None,None,None,"%g" % (smallspc*4)]
    else:
        labels=['0',None,None,None,None,"%g" % (smallspc*5)]

    temp_max=newspc[len(newspc)-1]
    start=temp_max
    for temp in Numeric.arange(start,width-bigspc/2,bigspc):
        temp_max=temp_max+bigspc
        newspc.append(temp_max)
        labels.append("%g" % temp_max)

    #start=temp_max
    #for temp in Numeric.arange(start,width-smallspc/2,smallspc):
    #    labels.append(None)
    #    temp_max=temp_max+smallspc 
    #    newspc.append(temp_max)       

    return (Numeric.array(newspc,Numeric.Float32),labels)


#############################################################
# Utility functions                                         #
#############################################################

# "Nice" function logic:
#
# spacings: restrict to three significant digits; if
#           a third one is present, it must be 5.
#
# min/max values: there should be an integer number of
# "spacings" between min and max values.  min and max
# values may each be rounded by up to tolerance*spacing
# or tolerance*(max-min)/divisions in either direction
# in order to be "nicer".  Tolerance defaults to 0.5.
#
# "Nice" numbers have fewer significant digits
# (eg. 3000 is "nicer" than 3013), and having 5 as
# the last significant digit is "nicer" than having
# 1,2,3,4,6,7,8,9.
#
# TO DO: Create "nice" functions, and use them in the grids.

def GetNiceExtentsByDivisions(minval,maxval,divisions,tolerance):
    """ Try to find nice default extents based on 
        approximate min/max values and a number of divisions.

        Input:
            minval- minimum value
            maxval- maximum value
            divisions- number of divisions
            tolerance- minval and maxval may each be altered by
                       up to tolerance*(maxval-minval)/divisions

        Output:
            newmin- new minimum value
            newmax- new maximum value
            spacing- spacing
    """
    pass

def GetNiceExtentsBySpacing(minval,maxval,spacing,tolerance):
    """ Try to find nice default extents based on 
        approximate min/max values and a spacing.
        If the spacing is itself not a nice value,
        it will also be somewhat adjusted.

        Input:
            minval- minimum value
            maxval- maximum value
            spacing- space between values
            tolerance- minval and maxval may each be altered by
                       up to tolerance*spacing


        Output:
            newmin- new minimum value
            newmax- new maximum value
            newspacing- new spacing
    """
    pass


def GetLatLongString(ddvalue,lltype='latitude'):
    """ Convert a decimal degree value to a
        string appropriate for Lat/Long
        display.

        ddvalue- position in decimal degrees
        lltype- latitude or longitude

        returns: lat/long string
    """
    import Numeric

    deg=int(abs(ddvalue))
    min=int((abs(ddvalue)-deg)*60)
    sec=int((abs(ddvalue)-deg-(float(min)/60.0))*3600.0)
    if lltype == 'latitude':
        if Numeric.sign(ddvalue) == -1:
            ch='S'
        else:
            ch='N'
    else:
        if Numeric.sign(ddvalue) == -1:
            ch='W'
        else:
            ch='E'

    nstr="%dd%d'%.1f''%s" % (deg,min,sec,ch)
    return nstr


def GetLatLongSpacings(min_ddvalue,max_ddvalue,approx_divs):
    """ Get spacing values for latitude or longitude
        min/max values.

        Input:
            min_ddvalue- minimum extent (decimal degrees)
            max_ddvalue- maximum extent (decimal degrees)
            approx_divs- An approximate number of divisions.

        Output:
            (degspc,minspc,secspc)
            degspc- degree spacing (integer)
            minspc- minute spacing (integer)
            secspc- second spacing (float)
    """

    diff=max_ddvalue-min_ddvalue
    degspc=int(diff/approx_divs)
    minspc=int((diff-float(degspc*approx_divs))*60.0/approx_divs)
    sdiff=diff-float(degspc*approx_divs)-(float(minspc*approx_divs)/60.0)

    # second spacing is rounded down to the nearest 0.1
    secspc=float("%.1f" % (sdiff*3600.0/approx_divs))
    if secspc > (sdiff*3600.0/approx_divs):
        secspc=secspc-0.1

    return (degspc,minspc,secspc)

def GetAlphabeticGridString(index):
    """ Get string for reference grid horizontal
        index (1='A',2='B',...,27='AA',...)
    """
    # This function doesn't work yet.
    alphabet=['A','B','C','D','E','F','G','H','I','J','K',
              'L','M','N','O','P','Q','R','S','T','U','V',
              'W','X','Y','Z']
    alen=len(alphabet)
    sc=Numeric.log(alen)
    nletters=int(Numeric.log(index)/sc)
    str=''
    rem=index
    for idx in range(nletters,-1,-1):
        rint=int(rem/pow(alen,idx-1))
        rem=rem-rint*pow(alen,idx-1)
        print 'rint ',rint,' rem: ',rem
        if (rint == 0) and (len(str) == 0):
            continue
        if rem == 0:
            str=str+alphabet[0]
        else:    
            str=str+alphabet[rint-1]

    return str
