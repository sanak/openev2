###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Utilities for creating vrt files.
# Author:   Gillian Walter, gwalter@atlsci.com
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

import gdal       
import Numeric
import osr
import os
from gvconst import *

CT_SPECTRUM = "Spectrum"
CT_REVERSE_SPECTRUM = "Inverse Spectrum"
CT_COOL_HOT = "Cool -> Hot"
CT_COLD_HOT = "Cold -> Hot"
CT_HOT_COOL = "Hot -> Cool"
CT_HOT_COLD = "Hot -> Cold"
CT_TOPO1 = "Topographic"
CT_SPECIAL = "Special"

class VRTCreationOptions:
    def __init__(self,num_dstbands):
        # num_dstbands: number of bands in destination vrt
        self.band_opts={}
        self.geocode_preference=None
        self.reproj=None

        for cband in range(1,num_dstbands+1):
            self.band_opts[str(cband)]={}
            self.band_opts[str(cband)]['band']=str(cband)
            self.band_opts[str(cband)]['SourceBand']=str(cband)

    def set_geopref(self,geocode_pref=None):
        # Set the geocode preference
        # (None or 'geotransform' or 'gcps')
        if geocode_pref is None:
            self.geocode_preference=None            
        elif geocode_pref == 'geotransform':
            self.geocode_preference='geotransform'
        elif geocode_pref == 'gcps':
            self.geocode_preference = 'gcps'
        else:
            txt="Invalid geocoding preference- options are None,"
            txt=txt+"'geotransform',or 'gcp'.  Resetting to None." 
            print txt
            self.geocode_preference = None

    def get_geopref(self):
        return self.geocode_preference

    def set_reproj(self,proj):
        """ Reproject gcps or geotransform to projection proj (a WKT string)
            during serialization.
        """
        self.reproj=proj

    def set_src_window(self,src_tuple,band_list=None):
        if band_list is None:
            # Window all bands
            for ckey in self.band_opts.keys():
                self.band_opts[ckey]['SrcRect']=src_tuple
        else:
            for cband in band_list:
                self.band_opts[str(cband)]['SrcRect']=src_tuple

    def set_dst_window(self,dst_tuple,band_list=None):
        if band_list is None:
            # Window all bands
            for ckey in self.band_opts.keys():
                self.band_opts[ckey]['DstRect']=dst_tuple
        else:
            for cband in band_list:
                self.band_opts[str(cband)]['DstRect']=dst_tuple

    def set_datatype(self,data_type,band_list=None):
        if band_list is None:
            # Set datatype in all bands
            for ckey in self.band_opts.keys():
                self.band_opts[ckey]['DataType']=gdal.GetDataTypeName(data_type)
        else:
            for cband in band_list:
                self.band_opts[str(cband)]['DataType']=gdal.GetDataTypeName(data_type)

    def set_color_interp(self,color_interp,band_list=None):
        if band_list is None:
            # Window all bands
            for ckey in self.band_opts.keys():
                self.band_opts[ckey]['ColorInterp']=color_interp
        else:
            for cband in band_list:
                self.band_opts[str(cband)]['ColorInterp']=color_interp


    def set_scaling(self,scale_tuple,band_list=None):
        srcmin=scale_tuple[0]
        srcmax=scale_tuple[1]
        dstmin=scale_tuple[2]
        dstmax=scale_tuple[3]
        srcdiff=float(srcmax-srcmin)
        dstdiff=float(dstmax-dstmin)
        if abs(srcdiff) > 0.0:
            ratio=dstdiff/srcdiff
        else:
            print 'Warning- no dynamic range for source. Ratio defaulting to 1.'
            ratio=1.0

        offset=dstmin-(srcmin*ratio)    
        if band_list is None:
            # Window all bands
            for ckey in self.band_opts.keys():
                self.band_opts[ckey]['ScaleRatio']=ratio                
                self.band_opts[ckey]['ScaleOffset']=offset
        else:
            for cband in band_list:
                self.band_opts[str(cband)]['ScaleRatio']=ratio                
                self.band_opts[str(cband)]['ScaleOffset']=offset


    def get_opts(self):
        return self.band_opts

def serializeMetadata(indataset=None, dict=None):
    """ Serialize metadata.

        Inputs:

            indataset- gdal dataset to extract metadata from
                       (None if not using)

            dict- dictionary to use for metadata (None
                  if not using)

        Values in dict will override values in indataset.
    """

    if indataset is not None:
        metadict=indataset.GetMetadata()
        if dict is not None:
            for item in dict.keys():
                metadict[item] = dict[item]
    else:
        metadict=dict

    if len (metadict) > 0:
        metabase=[gdal.CXT_Element,'Metadata']
        for ckey in metadict.keys():
            mdibase=[gdal.CXT_Element,'MDI']
            mdibase.append([gdal.CXT_Attribute,'key',[gdal.CXT_Text,ckey]])
            mdibase.append([gdal.CXT_Text,metadict[ckey]])
            metabase.append(mdibase)
    else:
        metabase=None

    return metabase


def serializeGCPs(indataset=None, vrt_options=None,with_Z=0,gcplist=None,
                  projection_attr_txt=None,reproj=None):
    """ This function can be used to serialize gcps with or
        without an associated gdal dataset.

        Inputs:
            indataset- a gdal dataset
            vrt_options- a VRTCreationOptions object
            with_Z- flag to indicate whether Z values should be included
            gcplist- a gcplist or None.  If it is None, indataset
                     will be searched.
            projection_attr_text- projection of the gcps.  If it is
                      None, indataset will be searched.
            reproj- projection of output gcps.  Only used if vrt_options
                    is None (otherwise it looks in vrt_options for reproj).

        Examples:
            serializeGCPs(dataset,vrtopts)
            serializeGCPs(gcplist=gcps,projection_attr_txt='GEOGCS...',
                          reproj='PROJCS...')
    """

    if gcplist is None:
        gcplist=indataset.GetGCPs()

    if projection_attr_txt is None:
        projection_attr_txt=indataset.GetGCPProjection()

    srtrans=None
    if vrt_options is not None:
        reproj=vrt_options.reproj
        if reproj == projection_attr_txt:
            reproj=None

    if (reproj is not None):
        sr1=osr.SpatialReference()
        sr2=osr.SpatialReference()
        try:
            sr1.ImportFromWkt(reproj)
            try:
                sr2.ImportFromWkt(projection_attr_txt)
                srtrans=osr.CoordinateTransformation(sr2,sr1)
            except:
                srtrans=None
                print 'Warning: unable to reproject gcps- invalid source projection string'
        except:
            srtrans=None
            print 'Warning: unable to reproject gcps- invalid destination projection string'

    if len(gcplist) > 0:
        gcpbase=[gdal.CXT_Element,'GCPList']
        if (srtrans is not None):
            gcpbase.append([gdal.CXT_Attribute,'Projection',
                            [gdal.CXT_Text,reproj]])
        else:
            gcpbase.append([gdal.CXT_Attribute,'Projection',
                            [gdal.CXT_Text,projection_attr_txt]])
        if (vrt_options is None):        
            for gcp in gcplist:
                if srtrans is not None:
                    ngcp=srtrans.TransformPoint(gcp.GCPX,gcp.GCPY,gcp.GCPZ)
                    gcp.GCPX=ngcp[0]
                    gcp.GCPY=ngcp[1]
                    gcp.GCPZ=ngcp[2]
                gcpbase.append(gcp.serialize(with_Z))    
        else:
            bopts=vrt_options.get_opts()[vrt_options.get_opts().keys()[0]]
            if ((not bopts.has_key('SrcRect')) or
                (not bopts.has_key('DstRect'))):
                for gcp in gcplist:
                    if srtrans is not None:
                        ngcp=srtrans.TransformPoint(gcp.GCPX,gcp.GCPY,gcp.GCPZ)
                        gcp.GCPX=ngcp[0]
                        gcp.GCPY=ngcp[1]
                        gcp.GCPZ=ngcp[2]
                    gcpbase.append(gcp.serialize(with_Z))    
            else:
                (spix,sline,xsize,ysize)=bopts['SrcRect']
                (dpix,dline,dxsize,dysize)=bopts['DstRect']
                for gcp in gcplist:
                    gcpbase2=[gdal.CXT_Element,'GCP']
                    if srtrans is not None:
                        ngcp=srtrans.TransformPoint(gcp.GCPX,gcp.GCPY,gcp.GCPZ)
                        gx=ngcp[0]
                        gy=ngcp[1]
                        gz=ngcp[2]
                    else:
                        gx=gcp.GCPX
                        gy=gcp.GCPY
                        gz=gcp.GCPZ
                    gp=(gcp.GCPPixel+dpix-spix)*dxsize/xsize
                    gl=(gcp.GCPLine+dline-sline)*dysize/ysize
                    gcpbase2.append([gdal.CXT_Attribute,'Id',
                                     [gdal.CXT_Text,gcp.Id]])
                    pixval = '%0.15E' % gp       
                    lineval = '%0.15E' % gl
                    xval = '%0.15E' % gx
                    yval = '%0.15E' % gy
                    zval = '%0.15E' % gz
                    gcpbase2.append([gdal.CXT_Attribute,'Pixel',
                                     [gdal.CXT_Text,pixval]])
                    gcpbase2.append([gdal.CXT_Attribute,'Line',
                                     [gdal.CXT_Text,lineval]])
                    gcpbase2.append([gdal.CXT_Attribute,'X',
                                     [gdal.CXT_Text,xval]])
                    gcpbase2.append([gdal.CXT_Attribute,'Y',
                                     [gdal.CXT_Text,yval]])
                    if with_Z:
                        gcpbase2.append([gdal.CXT_Attribute,'Z',
                                         [gdal.CXT_Text,yval]])
                    gcpbase.append(gcpbase2)
    else:
        gcpbase=None

    return gcpbase


def GeoTransformToGCPs(gt,num_pixels,num_lines,grid=2):
    """ Form a gcp list from a geotransform. If grid=0, just use 4
        corners.  If grid=1, split each dimension once.  If grid=2,
        split twice, etc:

        grid=0             grid=1                grid=2

        *           *      *     *     *         *   *   *   *

                                                 *   *   *   *    
                           *     *     *
                                                 *   *   *   *

        *           *      *     *     *         *   *   *   *

        This function is meant to be used to convert a geotransform
        to gcp's so that the geocoded information can be reprojected.

        Inputs: gt- geotransform to convert to gcps
                num_pixels- number of pixels in the dataset
                num_lines- number of lines in the dataset
                grid- see above.  Defaults to 2.

    """

    gcp_list=[]

    parr=Numeric.arange(0.0,num_pixels+1.0,num_pixels/(grid+1.0))
    larr=Numeric.arange(0.0,num_lines+1.0,num_lines/(grid+1.0))

    for idx in range(len(parr)*len(larr)):
        cgcp=gdal.GCP()
        pix=parr[idx % len(parr)]
        line=larr[idx/len(larr)]
        cgcp.Id=str(idx)
        cgcp.GCPX=gt[0]+(pix*gt[1])+(line*gt[2])
        cgcp.GCPY=gt[3]+(pix*gt[4])+(line*gt[5])
        cgcp.GCPZ=0.0
        cgcp.GCPPixel=pix
        cgcp.GCPLine=line

        gcp_list.append(cgcp)

    return gcp_list

def serializeGeoTransform(indataset=None,vrt_options=None,geotransform=None):
    if geotransform is None:
        gt=indataset.GetGeoTransform()
    else:
        gt=geotransform

    default_geo=(0.0,1.0,0.0,0.0,0.0,1.0)
    # Don't add anything if the transform
    # is the default values
    usegeo=0
    gbase=None        
    for i in range(6):
        if gt[i] != default_geo[i]:
            usegeo=1

    if usegeo==1:
        if (vrt_options is None):
            geo_text='  %0.22E, %0.22E, %0.22E, %0.22E, %0.22E, %0.22E' % (gt[0], gt[1], gt[2], gt[3], gt[4], gt[5])
        else:
            bopts=vrt_options.get_opts()[vrt_options.get_opts().keys()[0]]

            if ((not bopts.has_key('SrcRect')) or (not bopts.has_key('DstRect'))):
                geo_text='  %0.22E, %0.22E, %0.22E, %0.22E, %0.22E, %0.22E' % (gt[0], gt[1], gt[2], gt[3], gt[4], gt[5])
            else:    
                # Geotransform should be updated to reflect the window
                # All the bands should have the same windowing/overview options,
                # so just look at first one

                (spix,sline,xsize,ysize)=bopts['SrcRect']
                (dpix,dline,dxsize,dysize)=bopts['DstRect']
                gt0=gt[0]+gt[1]*(spix-dpix)+gt[2]*(sline-dline)
                gt3=gt[3]+gt[4]*(spix-dpix)+gt[5]*(sline-dline)
                # floor- if xsize/ysize/dxsize/dysize are non-integer,
                # gdal will truncate, so account for that here.
                gt1=float(gt[1])*Numeric.floor(xsize)/Numeric.floor(dxsize)
                gt2=float(gt[2])*Numeric.floor(ysize)/Numeric.floor(dysize)
                gt4=float(gt[4])*Numeric.floor(xsize)/Numeric.floor(dxsize)
                gt5=float(gt[5])*Numeric.floor(ysize)/Numeric.floor(dysize)
                geo_text='  %0.22E, %0.22E, %0.22E, %0.22E, %0.22E, %0.22E' % (gt0, gt1, gt2, gt3, gt4, gt5)

        gbase=[gdal.CXT_Element,'GeoTransform',[gdal.CXT_Text,geo_text]]
    else:
        gbase=None

    return gbase

def serializeCombinedDatasets(indatasetlist,vrt_options_list=None,
                              band_lists=None):
    """ Combines bands from several datasets into one.  Uses
        metadata and georeferencing from the FIRST one,
        and determines raster size from first one.
        indatasetlist- a list of gdal datasets
        vrt_options_list- a single vrt_options instance applicable
                          to all the datasets, or else a list
                          of vrt_options (same length as
                          indatasetlist).
        band_lists- list of lists of bands, or None (use all bands).
    """

    if vrt_options_list is None:
        band_opts=[]
        for ds in indatasetlist:
            band_opts.append({})
    else:
        band_opts=[]
        if type(vrt_options_list) == type(list):
            if len(vrt_options_list) != len(indatasetlist):
                raise 'VRT option list length does not match dataset list '+\
                      'length!'
            for opts in vrt_options_list:
                band_opts.append(opts.get_opts())
        else:
            for ds in indatasetlist:
                band_opts.append(vrt_options_list.get_opts())

    # band opts: A dictionary of band option dictionaries,
    # indexed by the str(band number)
    base=[gdal.CXT_Element,'VRTDataset']
    if vrt_options_list is None:
        base.append([gdal.CXT_Attribute,'rasterXSize',
                     [gdal.CXT_Text,str(indatasetlist[0].RasterXSize)]])
        base.append([gdal.CXT_Attribute,'rasterYSize',
                     [gdal.CXT_Text,str(indatasetlist[0].RasterYSize)]])
    else:
        bopts=band_opts[0]
        if bopts.has_key('DstRect'):
            # For now, get dataset size form first band
            (dpix,dline,dxsize,dysize)=bopts['DstRect']
            base.append([gdal.CXT_Attribute,'rasterXSize',
                         [gdal.CXT_Text,str(dxsize)]])
            base.append([gdal.CXT_Attribute,'rasterYSize',
                         [gdal.CXT_Text,str(dysize)]])
        else:
            base.append([gdal.CXT_Attribute,'rasterXSize',
                         [gdal.CXT_Text,str(indatasetlist[0].RasterXSize)]])
            base.append([gdal.CXT_Attribute,'rasterYSize',
                         [gdal.CXT_Text,str(indatasetlist[0].RasterYSize)]])

    mbase=serializeMetadata(indatasetlist[0])
    if mbase is not None:
        base.append(mbase)

    if vrt_options_list is None:
        geopref=None
        vrt_options=None
    elif type(vrt_options_list) == type([]):
        geopref=vrt_options_list[0].get_geopref()
        vrt_options=vrt_options_list[0]
    else:
        geopref=vrt_options_list.get_geopref()
        vrt_options=vrt_options_list

    if ((geopref == 'gcps') or (geopref is None)):
        # If preference is gcps or none, copy any available gcp information
        gcpbase=serializeGCPs(indatasetlist[0],vrt_options)
        if gcpbase is not None:
            base.append(gcpbase)

        if ((gcpbase is None) and (geopref == 'gcps')):
            print 'Warning- No gcp information to transfer.'

    if ((geopref == 'geotransform') or (geopref is None)):
        # If preference is geotransform or none, copy any available
        # geotransform information.  If geopref is None and
        # vrt indicates that the user wishes to reproject georeferencing,
        # convert to gcps and serialize them.

        srs_text=indatasetlist[0].GetProjection()
        if vrt_options is not None:
            reproj=vrt_options.reproj
            if reproj == srs_text:
                reproj=None
        else:
            reproj=None

        if ((reproj is not None) and (srs_text is not None) and
            (len(srs_text) > 0) and (gcpbase is None)):

            if geopref is None:
                gcps=GeoTransformToGCPs(indatasetlist[0].GetGeoTransform(),
                                        indatasetlist[0].RasterXSize,
                                        indatasetlist[0].RasterYSize)
                gcpbase=serializeGCPs(indatasetlist[0],vrt_options,
                         gcplist=gcps,projection_attr_txt=srs_text)

                if gcpbase is not None:
                    base.append(gcpbase)
            else:
                print 'Warning- reprojection of a geotransform to a '+\
                      '\n        new geotransform not supported.'
        elif (reproj is None):
            if ((srs_text is not None) and (len(srs_text) > 0)):
                prjbase=[gdal.CXT_Element,'SRS',[gdal.CXT_Text,srs_text]]      
                base.append(prjbase)

            gbase=serializeGeoTransform(indatasetlist[0],vrt_options)
            if gbase is not None:
                base.append(gbase)

            if ((gbase is None) and (geopref == 'geotransform')):
                print 'Warning- No geotransform information to transfer.'
        elif gcpbase is None:
            print 'Warning- No reprojectable geotransform information found.'

    for idx in range(len(indatasetlist)):
        indataset=indatasetlist[idx]
        if band_lists is None:
            band_list=None
        else:
            band_list=band_lists[idx]

        if band_list is None:
            for cband in range(1,indataset.RasterCount+1):
                if band_opts[idx].has_key(str(cband)):
                    bbase=serializeBand(indataset,
                                        opt_dict=band_opts[idx][str(cband)])
                    base.append(bbase)
                else:
                    opt_dict={}
                    opt_dict['band']=str(cband)
                    opt_dict['SourceBand']=str(cband)
                    bbase=serializeBand(indataset,opt_dict=opt_dict)
                    base.append(bbase)
        else:
            for cband in band_list:
                if band_opts.has_key(str(cband)):
                    bbase=serializeBand(indataset,
                                        opt_dict=band_opts[idx][str(cband)])
                    base.append(bbase)
                else:
                    opt_dict={}
                    opt_dict['band']=str(cband)
                    opt_dict['SourceBand']=str(cband)
                    bbase=serializeBand(indataset,opt_dict=opt_dict)
                    base.append(bbase)

    return base


def serializeDataset(indataset,vrt_options=None,band_list=None):
    if vrt_options is None:
        band_opts={}
    else:
        band_opts=vrt_options.get_opts()

    # band opts: A dictionary of band option dictionaries,
    # indexed by the str(band number)
    base=[gdal.CXT_Element,'VRTDataset']
    if vrt_options is None:
        base.append([gdal.CXT_Attribute,'rasterXSize',
                     [gdal.CXT_Text,str(indataset.RasterXSize)]])
        base.append([gdal.CXT_Attribute,'rasterYSize',
                     [gdal.CXT_Text,str(indataset.RasterYSize)]])
    else:
        bopts=band_opts[band_opts.keys()[0]]
        if bopts.has_key('DstRect'):
            # For now, get dataset size form first band
            (dpix,dline,dxsize,dysize)=bopts['DstRect']
            base.append([gdal.CXT_Attribute,'rasterXSize',
                         [gdal.CXT_Text,str(dxsize)]])
            base.append([gdal.CXT_Attribute,'rasterYSize',
                         [gdal.CXT_Text,str(dysize)]])
        else:
            base.append([gdal.CXT_Attribute,'rasterXSize',
                         [gdal.CXT_Text,str(indataset.RasterXSize)]])
            base.append([gdal.CXT_Attribute,'rasterYSize',
                         [gdal.CXT_Text,str(indataset.RasterYSize)]])

    mbase=serializeMetadata(indataset)
    if mbase is not None:
        base.append(mbase)

    geopref=None
    if vrt_options is not None:
        geopref=vrt_options.get_geopref()

    gcpbase=None    
    if ((geopref == 'gcps') or (geopref is None)):
        # If preference is gcps or none, copy any available gcp information
        gcpbase=serializeGCPs(indataset,vrt_options)
        if gcpbase is not None:
            base.append(gcpbase)

        if ((gcpbase is None) and (geopref == 'gcps')):
            print 'Warning- No gcp information to transfer.'

    if ((geopref == 'geotransform') or (geopref is None)):
        # If preference is geotransform or none, copy any available
        # geotransform information.  If geopref is None and
        # vrt indicates that the user wishes to reproject georeferencing,
        # convert to gcps and serialize them.

        srs_text=indataset.GetProjection()
        if vrt_options is not None:
            reproj=vrt_options.reproj
            if reproj == srs_text:
                reproj=None
        else:
            reproj=None

        if ((reproj is not None) and (srs_text is not None) and
            (len(srs_text) > 0) and (gcpbase is None)):

            if geopref is None:
                gcps=GeoTransformToGCPs(indataset.GetGeoTransform(),
                                        indataset.RasterXSize,
                                        indataset.RasterYSize)
                gcpbase=serializeGCPs(indataset,vrt_options,gcplist=gcps,
                                      projection_attr_txt=srs_text)

                if gcpbase is not None:
                    base.append(gcpbase)
            else:
                print 'Warning- reprojection of a geotransform to a '+\
                      '\n        new geotransform not supported.'
        elif (reproj is None):
            if ((srs_text is not None) and (len(srs_text) > 0)):
                prjbase=[gdal.CXT_Element,'SRS',[gdal.CXT_Text,srs_text]]      
                base.append(prjbase)

            gbase=serializeGeoTransform(indataset,vrt_options)
            if gbase is not None:
                base.append(gbase)

            if ((gbase is None) and (geopref == 'geotransform')):
                print 'Warning- No geotransform information to transfer.'
        elif gcpbase is None:
            print 'Warning- No reprojectable geotransform information found.'


    if band_list is None:
        for cband in range(1,indataset.RasterCount+1):
            if band_opts.has_key(str(cband)):
                bbase=serializeBand(indataset,opt_dict=band_opts[str(cband)])
                base.append(bbase)
            else:
                opt_dict={}
                opt_dict['band']=str(cband)
                opt_dict['SourceBand']=str(cband)
                bbase=serializeBand(indataset,opt_dict=opt_dict)
                base.append(bbase)
    else:
        for cband in band_list:
            if band_opts.has_key(str(cband)):
                bbase=serializeBand(indataset,opt_dict=band_opts[str(cband)])
                base.append(bbase)
            else:
                opt_dict={}
                opt_dict['band']=str(cband)
                opt_dict['SourceBand']=str(cband)
                bbase=serializeBand(indataset,opt_dict=opt_dict)
                base.append(bbase)

    return base

def serializeRawBand(SourceFilename, band, DataType, ByteOrder,
                      ImageOffset, PixelOffset, LineOffset,Description=None):
    """ Serialize a raw (flat binary) raster band.
        Inputs:

        SourceFilename- filename of a gdal dataset (a string)

        band- band number that the serialized band will correspond
              to in the output dataset (an integer).

        DataType- data type (a string).  May be 'Byte', 'UInt16',
                  'Int16', 'UInt32', 'Int32', 'Float32', 'Float64',
                  'CInt16', 'CInt32', 'CFloat32', 'CFloat64'.

        ByteOrder- MSB or LSB (string)

        ImageOffset- offset to first pixel of band (int)

        PixelOffset- offset between successive pixels in the input (int)

        LineOffset- offset between successive lines in the input (int)

        Description- OPTIONAL description for the band (a string)
    """
    base=[gdal.CXT_Element,'VRTRasterBand']

    base.append([gdal.CXT_Attribute,'dataType',
                     [gdal.CXT_Text,DataType]])

    base.append([gdal.CXT_Attribute,'band',[gdal.CXT_Text,str(band)]])

    base.append([gdal.CXT_Attribute,'subClass',
                 [gdal.CXT_Text,"VRTRawRasterBand"]])

    if Description is not None:
        base.append([gdal.CXT_Element,'Description',
                       [gdal.CXT_Text,Description]])

    base.append([gdal.CXT_Element,'SourceFilename',
                       [gdal.CXT_Text,SourceFilename]])
    base[len(base)-1].append([gdal.CXT_Attribute,'relativeToVRT',
                    [gdal.CXT_Text,GetRelativeToVRT(SourceFilename)]])

    base.append([gdal.CXT_Element,'ByteOrder',
                       [gdal.CXT_Text,ByteOrder]])

    base.append([gdal.CXT_Element,'ImageOffset',
                       [gdal.CXT_Text,str(ImageOffset)]])

    base.append([gdal.CXT_Element,'PixelOffset',
                       [gdal.CXT_Text,str(PixelOffset)]])

    base.append([gdal.CXT_Element,'LineOffset',
                       [gdal.CXT_Text,str(LineOffset)]])

    return base

def serializeBand(indataset=None,opt_dict={}):
    """ Serialize a raster band.
        Inputs:
            indataset- dataset to take default values from for
                       items that are not specified in opt_dict.
                       Set to None if not needed.

            opt_dict- dictionary to take values from.

        opt_dict will be searched for the following keys:

        SourceFilename- filename of a gdal dataset (a string)

        SourceBand- an integer indicating the band from SourceFilename
                    to use.  1 if not specified.

        band- band number that the serialized band will correspond
              to in the output dataset (an integer).  1 if not specified.

        DataType- data type (a string).  May be 'Byte', 'UInt16',
                  'Int16', 'UInt32', 'Int32', 'Float32', 'Float64',
                  'CInt16', 'CInt32', 'CFloat32', 'CFloat64'

        Description- OPTIONAL description to associate with the band (string).

        ColorInterp- OPTIONAL colour interpretation (a string).  One of
                     'Gray', 'Red', 'Green', 'Blue', 'Alpha', 'Undefined', or
                     'Palette'.  If palette is specified, then opt_dict
                     must also have a key 'Palette' with a value that
                     is a gdal ColorTable object.

        NoDataValue- OPTIONAL no data value.  Floating point or integer.

        ScaleOffset, ScaleRatio- OPTIONAL scaling offset and ratio for
                     rescaling the input bands (floating point numbers):
                     outband = ScaleOffset + (ScaleRatio*inband)

        SrcRect- a tuple of four integers specifying the extents of the
                 source to use (xoffset, yoffset, xsize, ysize)

        DstRect- a tuple of four integers specifying the extents that
                 SrcRect will correspond to in the output file.

    """
    base=[gdal.CXT_Element,'VRTRasterBand']

    if opt_dict.has_key('SourceBand'):
        inband=int(opt_dict['SourceBand'])
    else:
        inband=1

    if opt_dict.has_key('band'):    
        outband=int(opt_dict['band'])
    else:
        outband=1

    if opt_dict.has_key('DataType'):
        base.append([gdal.CXT_Attribute,'dataType',
                     [gdal.CXT_Text,opt_dict['DataType']]])        
    elif indataset is not None:
        base.append([gdal.CXT_Attribute,'dataType',
                     [gdal.CXT_Text,
                      gdal.GetDataTypeName(
            indataset.GetRasterBand(inband).DataType)]])
    else:
        base.append([gdal.CXT_Attribute,'dataType',[gdal.CXT_Text,'Byte']])

    base.append([gdal.CXT_Attribute,'band',[gdal.CXT_Text,str(outband)]])

    if opt_dict.has_key('Description'):
        base.append([gdal.CXT_Element,'Description',
                         [gdal.CXT_Text,opt_dict['Description']]])
    elif indataset is not None:
        desc = indataset.GetRasterBand(inband).GetDescription()
        if len(desc) > 0:
            base.append([gdal.CXT_Element,'Description',
                         [gdal.CXT_Text,desc]])

    if opt_dict.has_key('ColorInterp'):
        if opt_dict['ColorInterp'] != 'Undefined':
            base.append([gdal.CXT_Element,'ColorInterp',
                         [gdal.CXT_Text,opt_dict['ColorInterp']]])
            if opt_dict['ColorInterp'] == 'Palette':
                base.append(opt_dict['Palette'].serialize())

    elif indataset is not None:
        cinterp=indataset.GetRasterBand(inband).GetRasterColorInterpretation()
        if cinterp != gdal.GCI_Undefined:
            cinterpname=gdal.GetColorInterpretationName(cinterp)
            base.append([gdal.CXT_Element,'ColorInterp',
                         [gdal.CXT_Text,cinterpname]])
            if cinterpname=='Palette':
                ct = indataset.GetRasterBand(inband).GetRasterColorTable()
                base.append(ct.serialize())

    if opt_dict.has_key('NoDataValue'):
        base.append([gdal.CXT_Element,'NoDataValue',
                         [gdal.CXT_Text,str(opt_dict['NoDataValue'])]])
    elif indataset is not None:
        nodata_val=indataset.GetRasterBand(inband).GetNoDataValue()
        if ((nodata_val is not None) and (nodata_val != '')):
            base.append([gdal.CXT_Element,'NoDataValue',
                         [gdal.CXT_Text,str(nodata_val)]])

    if opt_dict.has_key('ScaleOffset') or opt_dict.has_key('ScaleRatio'):
        ssbase=[gdal.CXT_Element,'ComplexSource']
    else:
        ssbase=[gdal.CXT_Element,'SimpleSource']

    if opt_dict.has_key('SourceFilename'):
        ssbase.append([gdal.CXT_Element,'SourceFilename',
                       [gdal.CXT_Text,opt_dict['SourceFilename']]])
        ssbase[len(ssbase)-1].append([gdal.CXT_Attribute,'relativeToVRT',
                 [gdal.CXT_Text,GetRelativeToVRT(opt_dict['SourceFilename'])]])
    elif indataset is not None:
        ssbase.append([gdal.CXT_Element,'SourceFilename',
                       [gdal.CXT_Text,indataset.GetDescription()]])
        ssbase[len(ssbase)-1].append([gdal.CXT_Attribute,'relativeToVRT',
                 [gdal.CXT_Text,GetRelativeToVRT(indataset.GetDescription())]])
    else:
        ssbase.append([gdal.CXT_Element,'SourceFilename',
                       [gdal.CXT_Text,'']])
        ssbase[len(ssbase)-1].append([gdal.CXT_Attribute,
                                      'relativeToVRT',[gdal.CXT_Text,'0']])

    ssbase.append([gdal.CXT_Element,'SourceBand',[gdal.CXT_Text,str(inband)]])

    srcwinbase=[gdal.CXT_Element,'SrcRect']    

    if opt_dict.has_key('SrcRect'):
        xoff=str(opt_dict['SrcRect'][0])
        yoff=str(opt_dict['SrcRect'][1])
        xsize=str(opt_dict['SrcRect'][2])
        ysize=str(opt_dict['SrcRect'][3])
    elif indataset is not None:
        xoff='0'
        yoff='0'
        xsize=str(indataset.GetRasterBand(inband).XSize)
        ysize=str(indataset.GetRasterBand(inband).YSize)
    else:
        xoff='0'
        yoff='0'
        xsize='0'
        ysize='0'

    srcwinbase.append([gdal.CXT_Attribute,'xOff',[gdal.CXT_Text,xoff]])        
    srcwinbase.append([gdal.CXT_Attribute,'yOff',[gdal.CXT_Text,yoff]])        
    srcwinbase.append([gdal.CXT_Attribute,'xSize',[gdal.CXT_Text,xsize]])        
    srcwinbase.append([gdal.CXT_Attribute,'ySize',[gdal.CXT_Text,ysize]])        
    ssbase.append(srcwinbase)

    dstwinbase=[gdal.CXT_Element,'DstRect']
    if opt_dict.has_key('DstRect'):
        x2off=str(opt_dict['DstRect'][0])
        y2off=str(opt_dict['DstRect'][1])
        x2size=str(opt_dict['DstRect'][2])
        y2size=str(opt_dict['DstRect'][3])
    elif indataset is not None:
        x2off='0'
        y2off='0'
        x2size=str(indataset.GetRasterBand(inband).XSize)
        y2size=str(indataset.GetRasterBand(inband).YSize)
    else:
        x2off='0'
        y2off='0'
        x2size='0'
        y2size='0'

    dstwinbase.append([gdal.CXT_Attribute,'xOff',[gdal.CXT_Text,x2off]])        
    dstwinbase.append([gdal.CXT_Attribute,'yOff',[gdal.CXT_Text,y2off]])        
    dstwinbase.append([gdal.CXT_Attribute,'xSize',[gdal.CXT_Text,x2size]])        
    dstwinbase.append([gdal.CXT_Attribute,'ySize',[gdal.CXT_Text,y2size]])        
    ssbase.append(dstwinbase)

    if opt_dict.has_key('ScaleOffset'):
        ssbase.append([gdal.CXT_Element,'ScaleOffset',[gdal.CXT_Text,str(opt_dict['ScaleOffset'])]])

    if opt_dict.has_key('ScaleRatio'):
        ssbase.append([gdal.CXT_Element,'ScaleRatio',[gdal.CXT_Text,str(opt_dict['ScaleRatio'])]])

    base.append(ssbase)

    return base

def serializeDerivedBand(indataset=None,opt_dict={}):
    """ Serialize a derived raster band.
        Inputs:
            indataset- dataset to take default values from for
                       items that are not specified in opt_dict.
                       Set to None if not needed.

            opt_dict- dictionary to take values from.

        opt_dict will be searched for the following keys:

        band- band number that the serialized band will correspond
              to in the output dataset (an integer).  1 if not specified.

        DataType- data type (a string).  May be 'Byte', 'UInt16',
                  'Int16', 'UInt32', 'Int32', 'Float32', 'Float64',
                  'CInt16', 'CInt32', 'CFloat32', 'CFloat64'

        Description- OPTIONAL description to associate with the band (string).

        ColorInterp- OPTIONAL colour interpretation (a string).  One of
                     'Gray', 'Red', 'Green', 'Blue', 'Alpha', 'Undefined', or
                     'Palette'.  If palette is specified, then opt_dict
                     must also have a key 'Palette' with a value that
                     is a gdal ColorTable object.

        NoDataValue- OPTIONAL no data value.  Floating point or integer.

    """
    base=[gdal.CXT_Element,'VRTRasterBand']

    if opt_dict.has_key('SourceBand'):
        inband=int(opt_dict['SourceBand'])
    else:
        inband=1

    if opt_dict.has_key('band'):    
        outband=int(opt_dict['band'])
    else:
        outband=1

    if opt_dict.has_key('DataType'):
        base.append([gdal.CXT_Attribute,'dataType',
                     [gdal.CXT_Text,opt_dict['DataType']]])        
    elif indataset is not None:
        base.append([gdal.CXT_Attribute,'dataType',
                     [gdal.CXT_Text,
                      gdal.GetDataTypeName(
            indataset.GetRasterBand(inband).DataType)]])
    else:
        base.append([gdal.CXT_Attribute,'dataType',[gdal.CXT_Text,'Byte']])

    base.append([gdal.CXT_Attribute,'band',[gdal.CXT_Text,str(outband)]])

    base.append([gdal.CXT_Attribute,'subClass',
                 [gdal.CXT_Text,"VRTDerivedRasterBand"]])

    if opt_dict.has_key('Description'):
        base.append([gdal.CXT_Element,'Description',
                         [gdal.CXT_Text,opt_dict['Description']]])
    elif indataset is not None:
        desc = indataset.GetRasterBand(inband).GetDescription()
        if len(desc) > 0:
            base.append([gdal.CXT_Element,'Description',
                         [gdal.CXT_Text,desc]])

    if opt_dict.has_key('ColorInterp'):
        if opt_dict['ColorInterp'] != 'Undefined':
            base.append([gdal.CXT_Element,'ColorInterp',
                         [gdal.CXT_Text,opt_dict['ColorInterp']]])
            if opt_dict['ColorInterp'] == 'Palette':
                base.append(opt_dict['Palette'].serialize())

    elif indataset is not None:
        cinterp=indataset.GetRasterBand(inband).GetRasterColorInterpretation()
        if cinterp != gdal.GCI_Undefined:
            cinterpname=gdal.GetColorInterpretationName(cinterp)
            base.append([gdal.CXT_Element,'ColorInterp',
                         [gdal.CXT_Text,cinterpname]])
            if cinterpname=='Palette':
                ct = indataset.GetRasterBand(inband).GetRasterColorTable()
                base.append(ct.serialize())

    if opt_dict.has_key('NoDataValue'):
        base.append([gdal.CXT_Element,'NoDataValue',
                         [gdal.CXT_Text,str(opt_dict['NoDataValue'])]])
    elif indataset is not None:
        nodata_val=indataset.GetRasterBand(inband).GetNoDataValue()
        if ((nodata_val is not None) and (nodata_val != '')):
            base.append([gdal.CXT_Element,'NoDataValue',
                         [gdal.CXT_Text,str(nodata_val)]])

    if not opt_dict.has_key('PixFunctionName'):
        raise 'Derived band pixel function name not specified'
    base.append([gdal.CXT_Element,'PixelFunctionType',
                [gdal.CXT_Text,str(opt_dict['PixFunctionName'])]])

    return base

def serializeSource(bbase, indataset=None,opt_dict={}):
    """ Serialize a source and add to band base.
        Inputs:
            indataset- dataset to take default values from for
                       items that are not specified in opt_dict.
                       Set to None if not needed.

            opt_dict- dictionary to take values from.

        opt_dict will be searched for the following keys:

        SourceFilename- filename of a gdal dataset (a string)

        SourceBand- an integer indicating the band from SourceFilename
                    to use.  1 if not specified.

        Description- OPTIONAL description to associate with the band (string).

        NoDataValue- OPTIONAL no data value.  Floating point or integer.

        ScaleOffset, ScaleRatio- OPTIONAL scaling offset and ratio for
                     rescaling the input bands (floating point numbers):
                     outband = ScaleOffset + (ScaleRatio*inband)

        SrcRect- a tuple of four integers specifying the extents of the
                 source to use (xoffset, yoffset, xsize, ysize)

        DstRect- a tuple of four integers specifying the extents that
                 SrcRect will correspond to in the output file.        
    """

    if opt_dict.has_key('SourceBand'):
        inband=int(opt_dict['SourceBand'])
    else:
        inband=1

    if opt_dict.has_key('ScaleOffset') or opt_dict.has_key('ScaleRatio'):
        ssbase=[gdal.CXT_Element,'ComplexSource']
    else:
        ssbase=[gdal.CXT_Element,'SimpleSource']

    if opt_dict.has_key('SourceFilename'):
        ssbase.append([gdal.CXT_Element,'SourceFilename',
                       [gdal.CXT_Text,opt_dict['SourceFilename']]])
        ssbase[len(ssbase)-1].append([gdal.CXT_Attribute,'relativeToVRT',
                 [gdal.CXT_Text,GetRelativeToVRT(opt_dict['SourceFilename'])]])
    elif indataset is not None:
        ssbase.append([gdal.CXT_Element,'SourceFilename',
                       [gdal.CXT_Text,indataset.GetDescription()]])
        ssbase[len(ssbase)-1].append([gdal.CXT_Attribute,'relativeToVRT',
                 [gdal.CXT_Text,GetRelativeToVRT(indataset.GetDescription())]])
    else:
        ssbase.append([gdal.CXT_Element,'SourceFilename',
                       [gdal.CXT_Text,'']])
        ssbase[len(ssbase)-1].append([gdal.CXT_Attribute,
                                      'relativeToVRT',[gdal.CXT_Text,'0']])

    ssbase.append([gdal.CXT_Element,'SourceBand',[gdal.CXT_Text,str(inband)]])

    srcwinbase=[gdal.CXT_Element,'SrcRect']    

    if opt_dict.has_key('SrcRect'):
        xoff=str(opt_dict['SrcRect'][0])
        yoff=str(opt_dict['SrcRect'][1])
        xsize=str(opt_dict['SrcRect'][2])
        ysize=str(opt_dict['SrcRect'][3])
    elif indataset is not None:
        xoff='0'
        yoff='0'
        xsize=str(indataset.GetRasterBand(inband).XSize)
        ysize=str(indataset.GetRasterBand(inband).YSize)
    else:
        xoff='0'
        yoff='0'
        xsize='0'
        ysize='0'

    srcwinbase.append([gdal.CXT_Attribute,'xOff',[gdal.CXT_Text,xoff]])        
    srcwinbase.append([gdal.CXT_Attribute,'yOff',[gdal.CXT_Text,yoff]])        
    srcwinbase.append([gdal.CXT_Attribute,'xSize',[gdal.CXT_Text,xsize]])        
    srcwinbase.append([gdal.CXT_Attribute,'ySize',[gdal.CXT_Text,ysize]])        
    ssbase.append(srcwinbase)

    dstwinbase=[gdal.CXT_Element,'DstRect']
    if opt_dict.has_key('DstRect'):
        x2off=str(opt_dict['DstRect'][0])
        y2off=str(opt_dict['DstRect'][1])
        x2size=str(opt_dict['DstRect'][2])
        y2size=str(opt_dict['DstRect'][3])
    elif indataset is not None:
        x2off='0'
        y2off='0'
        x2size=str(indataset.GetRasterBand(inband).XSize)
        y2size=str(indataset.GetRasterBand(inband).YSize)
    else:
        x2off='0'
        y2off='0'
        x2size='0'
        y2size='0'

    dstwinbase.append([gdal.CXT_Attribute,'xOff',[gdal.CXT_Text,x2off]])        
    dstwinbase.append([gdal.CXT_Attribute,'yOff',[gdal.CXT_Text,y2off]])        
    dstwinbase.append([gdal.CXT_Attribute,'xSize',[gdal.CXT_Text,x2size]])        
    dstwinbase.append([gdal.CXT_Attribute,'ySize',[gdal.CXT_Text,y2size]])        
    ssbase.append(dstwinbase)

    if opt_dict.has_key('ScaleOffset'):
        ssbase.append([gdal.CXT_Element,'ScaleOffset',[gdal.CXT_Text,str(opt_dict['ScaleOffset'])]])

    if opt_dict.has_key('ScaleRatio'):
        ssbase.append([gdal.CXT_Element,'ScaleRatio',[gdal.CXT_Text,str(opt_dict['ScaleRatio'])]])

    bbase.append(ssbase)

    return bbase

def GetSimilarFiles(filename):
    """ Looks in the directory of filename for files with the same size
        and extension, and returns a list containing their full paths.
    """

    import os
    import glob

    fdir=os.path.dirname(filename)
    froot,fext=os.path.splitext(filename)
    flist=glob.glob(os.path.join(fdir,'*'+fext))

    fsize=os.path.getsize(filename)
    slist=[]
    for item in flist:
        if os.path.getsize(item) == fsize:
            slist.append(item)

    return slist

def GetRelativeToVRT(path):
    """ Returns '0' if path is absolute, '1' if it is
        relative to vrt.
    """

    if path[0] == '<':
        # input path is an in-memory vrt file
        return "0"

    if os.path.isabs(path):
        return "0"
    else:
        return "1"

class VRTDatasetConstructor:
    """ Class to use for creating vrt datasets from scratch at
        the python level.

        Initial inputs:
            pixels- number of pixels in dataset
            lines- number of lines in dataset
    """
    def __init__(self,pixels,lines):
        self.base=[gdal.CXT_Element,'VRTDataset']
        self.base.append([gdal.CXT_Attribute,'rasterXSize',
                     [gdal.CXT_Text,str(pixels)]])
        self.base.append([gdal.CXT_Attribute,'rasterYSize',
                     [gdal.CXT_Text,str(lines)]])
        self.band_idx=1
        self.xsize=pixels
        self.ysize=lines

    def write(self, vrt_filename):
        """Write the contenst of the vrt string to the provided filename."""
        fh = file(vrt_filename, 'w')
        fh.write(self.GetVRTString())
        fh.close()

    def AddSimpleBand(self, SourceFilename, SourceBand, DataType,
                      SrcRect=None, DstRect=None, ColorInterp='Undefined',
                      colortable=None, NoDataValue=None,
                      ScaleOffset=None,ScaleRatio=None,
                      Description=None, metadict=None ):
        """ Add a simple raster band

        SourceFilename- filename of a gdal dataset (a string)

        SourceBand- an integer indicating the band from SourceFilename
                    to use.

        DataType- data type (a string).  May be 'Byte', 'UInt16',
                  'Int16', 'UInt32', 'Int32', 'Float32', 'Float64',
                  'CInt16', 'CInt32', 'CFloat32', 'CFloat64'

        SrcRect- a tuple of four integers specifying the extents of the
                 source to use (xoffset, yoffset, xsize, ysize).
                 Defaults to (0, 0, xsize, ysize) for the dataset.

        DstRect- a tuple of four integers specifying the extents that
                 SrcRect will correspond to in the output file.
                 Defaults to (0, 0, xsize, ysize) for the dataset.

        ColorInterp- OPTIONAL colour interpretation (a string).  One of
                     'Gray', 'Red', 'Green', 'Blue', 'Alpha', 'Undefined', or
                     'Palette'.  If 'Palette' is specified, then colortable
                     must also be specified.

        colortable- OPTIONAL GDAL colortable object (only used if ColorInterp
                    is set to 'Palette').

        NoDataValue- OPTIONAL no data value.  Floating point or integer.

        ScaleOffset, ScaleRatio- OPTIONAL scaling offset and ratio for
                     rescaling the input bands (floating point numbers):
                     outband = ScaleOffset + (ScaleRatio*inband)

        Description- OPTIONAL description for the band (a string).

        metadict-   OPTIONAL metadata for band, may contain default lut
                    enhancement type.

        """
        opt_dict={'SourceFilename':SourceFilename,'SourceBand':SourceBand,
                  'DataType':DataType,'ColorInterp':ColorInterp}

        if ColorInterp == 'Palette':
            if colortable is None:
                raise 'Color table not specified!'
            opt_dict['Palette']=colortable

        if SrcRect is not None:
            opt_dict['SrcRect']=SrcRect
        else:
            opt_dict['SrcRect']=(0,0,self.xsize,self.ysize)

        if DstRect is not None:
            opt_dict['DstRect']=DstRect
        else:
            opt_dict['DstRect']=(0,0,self.xsize,self.ysize)

        if ScaleOffset is not None:
            opt_dict['ScaleOffset']=ScaleOffset

        if ScaleRatio is not None:
            opt_dict['ScaleRatio']=ScaleRatio

        if NoDataValue is not None:
            opt_dict['NoDataValue']=NoDataValue

        if Description is not None:
            opt_dict['Description']=Description

        opt_dict['band']=self.band_idx

        bbase=serializeBand(None,opt_dict)

        if metadict is not None:
            mbase = serializeMetadata(dict=metadict)
            bbase.append(mbase)

        self.base.append(bbase)
        self.band_idx=self.band_idx+1

        return bbase

    def AddRawBand(self, SourceFilename, DataType, ByteOrder,
                      ImageOffset, PixelOffset, LineOffset,
                      Description=None):
        """ Add a flat binary source raster band.
            Inputs:
                SourceFilename- path to source file name (string)

                DataType- datatype (string).  One of:
                                Byte        Float64
                                UInt16      CInt16
                                Int16       CInt32
                                UInt32      CFloat32
                                Int32       CFloat64
                                Float32

                ByteOrder- MSB or LSB (string)

                ImageOffset- offset to first pixel of band (int)

                PixelOffset- offset between successive pixels in the input (int)

                LineOffset- offset between successive lines in the input (int)

                Description- OPTIONAL description for the band (a string). 
        """
        bbase=serializeRawBand(SourceFilename,self.band_idx, DataType,
                               ByteOrder,ImageOffset,PixelOffset,
                               LineOffset,Description)

        self.base.append(bbase)
        self.band_idx=self.band_idx+1

        return bbase

    def AddDerivedBand(self, DataType, PixFunctionName, ColorInterp='Undefined',
                       colortable=None, NoDataValue=None, Description=None, metadict=None ):
        """ Add a derived raster band.  This function is different from the
            other functions in that it does not add the derived band to the
            dataset base, but rather returns the band base so that it may
            be used to add sources.

        DataType- data type (a string).  May be 'Byte', 'UInt16',
                  'Int16', 'UInt32', 'Int32', 'Float32', 'Float64',
                  'CInt16', 'CInt32', 'CFloat32', 'CFloat64'

        ColorInterp- OPTIONAL colour interpretation (a string).  One of
                     'Gray', 'Red', 'Green', 'Blue', 'Alpha', 'Undefined', or
                     'Palette'.  If 'Palette' is specified, then colortable
                     must also be specified.

        colortable- GDAL colortable object (only used if ColorInterp
                    is set to 'Palette').

        NoDataValue- OPTIONAL no data value.  Floating point or integer.

        Description- OPTIONAL description for the band (a string).

        metadict-   OPTIONAL metadata for band, may contain default lut
                    enhancement type.

        """
        opt_dict={'DataType':DataType,'ColorInterp':ColorInterp,
                  'PixFunctionName':PixFunctionName}

        if ColorInterp == 'Palette':
            if colortable is None:
                raise 'Color table not specified!'
            opt_dict['Palette']=colortable

        if NoDataValue is not None:
            opt_dict['NoDataValue']=NoDataValue

        if Description is not None:
            opt_dict['Description']=Description

        opt_dict['band']=self.band_idx

        bbase=serializeDerivedBand(None,opt_dict)

        if metadict is not None:
            mbase = serializeMetadata(dict=metadict)
            bbase.append(mbase)

        self.base.append(bbase)
        self.band_idx=self.band_idx+1

        return bbase

    def AddSource(self, bbase, SourceFilename, SourceBand,
                  SrcRect=None, DstRect=None, NoDataValue=None,
                  ScaleOffset=None, ScaleRatio=None, Description=None):
        """ Add a simple source to a derived raster band

        bbase- base string of derived band, to append source

        SourceFilename- filename of a gdal dataset (a string)

        SourceBand- an integer indicating the band from SourceFilename
                    to use.

        SrcRect- a tuple of four integers specifying the extents of the
                 source to use (xoffset, yoffset, xsize, ysize).
                 Defaults to (0, 0, xsize, ysize) for the dataset.

        DstRect- a tuple of four integers specifying the extents that
                 SrcRect will correspond to in the output file.
                 Defaults to (0, 0, xsize, ysize) for the dataset.

        NoDataValue- OPTIONAL no data value.  Floating point or integer.

        ScaleOffset, ScaleRatio- OPTIONAL scaling offset and ratio for
                     rescaling the input bands (floating point numbers):
                     outband = ScaleOffset + (ScaleRatio*inband)

        Description- OPTIONAL description for the band (a string).

        """

        opt_dict={'SourceFilename':SourceFilename,'SourceBand':SourceBand}

        if SrcRect is not None:
            opt_dict['SrcRect']=SrcRect
        else:
            opt_dict['SrcRect']=(0,0,self.xsize,self.ysize)

        if DstRect is not None:
            opt_dict['DstRect']=DstRect
        else:
            opt_dict['DstRect']=(0,0,self.xsize,self.ysize)

        if NoDataValue is not None:
            opt_dict['NoDataValue']=NoDataValue

        if Description is not None:
            opt_dict['Description']=Description

        opt_dict['band']=self.band_idx

        serializeSource(bbase, opt_dict=opt_dict)

    def AddMetadata(self, metadict):
        """ Add metadata from a dictionary """
        if len(metadict.keys()) < 1:
            return

        mbase=serializeMetadata(dict=metadict)
        self.base.append(mbase)

    def SetSRS(self, projection):
        """ Set projection information for geotransform (a WKT string)"""
        prjbase=[gdal.CXT_Element,'SRS',[gdal.CXT_Text,projection]]      
        self.base.append(prjbase)

    def SetGeoTransform(self, gt):
        """ Add a geotransform (input is a tuple of 6 numbers) """
        gbase=serializeGeoTransform(geotransform=gt)
        self.base.append(gbase)

    def SetGCPs(self, gcps, projection='', reprojection=None):
        """ Add gcps from a list of GDAL GCP objects.
            Optional projection argument should be a WKT string.
            Optional reprojection argument should also be a WKT
            string, and should only be specified if the projection
            argument is also specified.
        """
        gcpbase=serializeGCPs(gcplist=gcps,projection_attr_txt=projection,
                              reproj=reprojection)
        self.base.append(gcpbase)

    def GetVRTLines(self):
        """ Return lines suitable for writing to a vrt file. """
        return gdal.SerializeXMLTree(self.base)

    def GetVRTString(self):
        """ Return a vrt string that can be opened as a gdal dataset. """
        lines = self.GetVRTLines()
        vrtstr=''
        for item in lines:
            vrtstr=vrtstr+item
        return vrtstr

class VRTDatasetXMLUtil(VRTDatasetConstructor):
    """ Class to use for creating vrt datasets from scratch at
        the python level.

        Initial inputs:
            SourceFilename - Name of source file for sources added to bands
                             in this dataset
            src_ds    - Source data set
            DataType  - Datatype default for bands added to dataset (string)
            DatasetDesc - Description of dataset, may be used for naming
                          'views' created as 'in memory' datasets.
            shape     - A tuple containing the number of (lines, pixels), if
                        src_ds is None
    """
    def __init__(self, SourceFilename, src_ds=None,
            DataType=None, DatasetDesc=None, shape=None):

        assert (src_ds is not None) or (shape is not None)
        if src_ds is None:
            assert shape is not None and isinstance(shape, tuple) and len(shape)==2
            assert DataType is not None
            self.Pixels, self.Lines = shape
        elif shape is None:
            assert src_ds is not None
            self.Pixels=src_ds.RasterXSize
            self.Lines=src_ds.RasterYSize

        if DataType != None:
            self.DataType=DataType
        else:
            self.DataType=gdal.GetDataTypeName(src_ds.GetRasterBand(1).DataType)

        self.SourceFilename=SourceFilename
        self.band_trees=[]

        VRTDatasetConstructor.__init__(self, self.Pixels, self.Lines)

        if DatasetDesc != None:
            self.base.append([gdal.CXT_Element,'Description',
                             [gdal.CXT_Text, DatasetDesc]])

    def append_band_XML(band_XML):
        if self.band_trees.__contains__(band_XML):
            self.band_trees.remove(band_XML)
        self.base.append(band_XML)

    def add_raw_band(self, SourceFilename, DataType, ByteOrder,
                      ImageOffset, PixelOffset, LineOffset,
                      Description=None):
        VRTDatasetConstructor.AddRawBand(slef, SourceFilename, DataType, ByteOrder,
                        ImageOffset, PixelOffset, LineOffset, Description)

    def add_derived_band(self, DataType, PixFunctionName, ColorInterp='Undefined',
                         colortable=None, NoDataValue=None, Description=None, 
                         enhance=None, lut_min=None, lut_max=None):
        """ Add a simple raster band
        DataType- data type (a string).  May be 'Byte', 'UInt16',
                  'Int16', 'UInt32', 'Int32', 'Float32', 'Float64',
                  'CInt16', 'CInt32', 'CFloat32', 'CFloat64'

        PixFunctionName - Name of pixel function applied to sources to
                 generate band data

        ColorInterp- OPTIONAL colour interpretation (a string).  One of
                     'Gray', 'Red', 'Green', 'Blue', 'Alpha', 'Undefined', or
                     'Palette'.  If 'Palette' is specified, then colortable
                     must also be specified.

        colortable- GDAL colortable object (only used if ColorInterp
                    is set to 'Palette').

        NoDataValue- OPTIONAL no data value.  Floating point or integer.

        Description- OPTIONAL description for the band (a string).

        enhance- OPTIONAL initial lut enhancement type to be applied to band, 
                 see gvconst.GV_RASTER_LUT_ENHANCE_TYPES

        """
        metadict=dict_append_lut_type(enhance)
        dict_append_min_max(lut_min, lut_max, metadict)
        bbase = VRTDatasetConstructor.AddDerivedBand(self, DataType, PixFunctionName, ColorInterp, 
                                                     colortable, NoDataValue, Description, metadict)
        self.band_trees.append(bbase)
        return bbase

    def add_simple_band(self, SourceFilename, SourceBand, DataType,
                        SrcRect=None, DstRect=None, ColorInterp='Undefined',
                        colortable=None, NoDataValue=None, ScaleOffset=None,
                        ScaleRatio=None, Description=None, enhance=None,
                        lut_min=None, lut_max=None):
        metadict=dict_append_lut_type(enhance)
        dict_append_min_max(lut_min, lut_max, metadict)
        return self.AddSimpleBand(SourceFilename, SourceBand, DataType, SrcRect,
                                  DstRect, ColorInterp, colortable, NoDataValue,
                                  ScaleOffset, ScaleRatio, Description, metadict)

    def add_band_with_src(self, SourceFilename, SourceBand=-1, ColorInterp='Undefined', 
                colortable=None,
                 NoDataValue=None, Description=None, PixFunctionName=None, 
                 enhance=None, lut_min=None, lut_max=None):

        if PixFunctionName == None:
            metadict=dict_append_lut_type(enhance)
            dict_append_min_max(lut_min, lut_max, metadict)
            return self.AddSimpleBand(SourceFilename, SourceBand, self.DataType,
                                      None, None, ColorInterp, colortable, NoDataValue,
                                      None, None, Description, metadict)
        else:
            return self.add_derived_band(self.DataType, PixFunctionName, ColorInterp,
                                         colortable, NoDataValue, Description, enhance,
                                         lut_min, lut_max)

    def add_band(self, SourceBand=-1, ColorInterp='Undefined', colortable=None,
                 NoDataValue=None, Description=None, PixFunctionName=None, 
                 enhance=None, lut_min=None, lut_max=None):
        return self.add_band_with_src(self.SourceFilename, SourceBand, ColorInterp, 
                 colortable,
                 NoDataValue, Description, PixFunctionName, enhance, lut_min, lut_max)

    def add_source(self, bbase, SourceBand, NoDataValue=None, Description=None):
        self.AddSource(bbase, self.SourceFilename, SourceBand,
                       None, None, NoDataValue, None, None, Description)

    def get_VRT_lines(self):
        """ Return lines suitable for writing to a vrt file. """

        #
        # Bands created with create_band_XML are now added to the tree
        #
        for item in band_trees:
            self.base.append(item)

        return gdal.SerializeXMLTree(self.base)

    def get_VRT_string(self):
        """ Return a vrt string that can be opened as a gdal dataset. """
        lines = self.GetVRTLines()
        vrtstr=''
        for item in lines:
            vrtstr=vrtstr+item
        return vrtstr

    def create_single_dataset(src_ds, ds_name, band_idx):
        """ Create a VRT dataset using the specified band index in the 
            specified dataset.  The band size and datatype will be based on the
            source dataset.
        """
        # Pending
        return None

    create_single_dataset = staticmethod(create_single_dataset)

    def create_RGB_dataset(SourceFilename, src_ds, 
                           red_band_idx, green_band_idx, blue_band_idx,
                           Description=None, r_lut=None, g_lut=None, b_lut=None,
                           lut_min=None, lut_max=None):
        """ Create a VRT dataset using the three specified band indexes in the 
            specified dataset for red, green, and blue bands.  The band sizes and
            datatypes will be based on the source dataset.
        """

        if src_ds == None or src_ds.RasterXSize < 1 or src_ds.RasterYSize < 1:
            raise 'Invalid source dataset'
        if red_band_idx < 1 or red_band_idx > src_ds.RasterCount:
            raise 'Invalid red band index'
        if green_band_idx < 1 or green_band_idx > src_ds.RasterCount:
            raise 'Invalid green band index'
        if blue_band_idx < 1 or blue_band_idx > src_ds.RasterCount:
            raise 'Invalid blue band index'

        vrt_ds = VRTDatasetXMLUtil(SourceFilename, src_ds,
                                   gdal.GetDataTypeName( \
                                       src_ds.GetRasterBand(red_band_idx).DataType), Description)
        vrt_ds.add_band(red_band_idx, ColorInterp="Red", Description="Red (" +
                        src_ds.GetRasterBand(red_band_idx).GetDescription() + ")",
                        enhance=r_lut, lut_min=lut_min, lut_max=lut_max)
        vrt_ds.add_band(green_band_idx, ColorInterp="Green", Description="Green (" +
                        src_ds.GetRasterBand(green_band_idx).GetDescription() + ")",
                        enhance=g_lut, lut_min=lut_min, lut_max=lut_max)
        vrt_ds.add_band(blue_band_idx, ColorInterp="Blue", Description="Blue (" +
                        src_ds.GetRasterBand(blue_band_idx).GetDescription() + ")",
                        enhance=b_lut, lut_min=lut_min, lut_max=lut_max)

        return vrt_ds

    create_RGB_dataset = staticmethod(create_RGB_dataset)

    def create_PSCI_dataset(SourceFilename, src_ds, hue_band_idx, intensity_band_idx=None,
                            Description=None, color_table=None, h_lut=None, i_lut=None,
                            lut_min=None, lut_max=None):
        """ Create a VRT dataset using the specified band indexes in the 
            specified dataset for hue and intensity bands.  The band sizes and
            datatypes will be based on the source dataset.
         """

        if src_ds == None or src_ds.RasterXSize < 1 or src_ds.RasterYSize < 1:
            raise 'Invalid source dataset'
        if hue_band_idx < 1 or hue_band_idx > src_ds.RasterCount:
            raise 'Invalid hue band index'

        vrt_ds = VRTDatasetXMLUtil(SourceFilename, src_ds,
                                   gdal.GetDataTypeName( \
                                       src_ds.GetRasterBand(hue_band_idx).DataType), 
                                   Description)

        vrt_ds.add_band(hue_band_idx, ColorInterp="Palette", Description="Hue (" +
                        src_ds.GetRasterBand(hue_band_idx).GetDescription() + ")",
                        colortable=color_table, enhance=h_lut,
                        lut_min=lut_min, lut_max=lut_max)

        if intensity_band_idx != None:
            if intensity_band_idx < 1 or intensity_band_idx > src_ds.RasterCount:
                raise 'Invalid intensity band index'
            vrt_ds.add_band(intensity_band_idx, Description="Intensity (" +
                            src_ds.GetRasterBand(intensity_band_idx).GetDescription() + ")",
                            enhance=i_lut, lut_min=lut_min, lut_max=lut_max)

        return vrt_ds

    create_PSCI_dataset = staticmethod(create_PSCI_dataset)

    def create_color_table(ct_type):
        """
        Create an instance of a predefined GDAL color table
        """
        colortable = gdal.ColorTable()
        if (ct_type == CT_SPECTRUM):
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,   0, (255,   0,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,  43, (255, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,  85, (  0, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 128, (  0, 255, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 171, (  0,   0, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 213, (255,   0, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 255, (255,   0,   0))
            return colortable
        if (ct_type == CT_REVERSE_SPECTRUM):
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,   0, (255,   0,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,  43, (255,   0, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,  85, (  0,   0, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 128, (  0, 255, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 171, (  0, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 213, (255, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 255, (255,   0,   0))
            return colortable
        if (ct_type == CT_COOL_HOT) or (ct_type == CT_COLD_HOT):
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,   0, (  0,   0, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,  64, (  0, 255, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 128, (  0, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 192, (255, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 255, (255,   0,   0))
            return colortable
        if (ct_type == CT_HOT_COOL) or (ct_type == CT_HOT_COLD):
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,   0, (255,   0,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,  64, (255, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 128, (  0, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 192, (  0, 255, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 255, (  0,   0, 255))
            return colortable
        if (ct_type == CT_TOPO1):
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,   0, (  0,   0, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,  60, (  0, 255, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 120, (  0, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 180, (255, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 220, (255, 100,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 255, (255, 255, 255))
            return colortable
        if (ct_type == CT_SPECIAL):
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,   0, (  0,   0, 0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,   1, (  0,   0, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable,  64, (  0, 255, 255))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 128, (  0, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 192, (255, 255,   0))
            VRTDatasetXMLUtil.add_gradient_color_entry(colortable, 255, (255,   0,   0))
            return colortable
        return None

    create_color_table = staticmethod(create_color_table)

    def add_gradient_color_entry(color_table, index, entry):
        """
        Add color entry to a GDAL color table by applying a gradient between the
        colors at the last entry index and the new entry index.

        @param color_table - GDAL color table to modify
        @param index - Integer index of new color to add
        @param entry - (red, green, blue) tuple for new color entry
        @param bHueOnly - True to set color gradient based on HSV hue difference
        between colors, False to compute gradient between RGB colors.
        """

        # Get last entry from table
        n_entries = color_table.GetCount()
        n_new = index - n_entries + 1

        # If no start entry, set colors to new entry
        if (n_entries == 0):
            for ii in range(0, index + 1):
                color_table.SetColorEntry(ii, entry)
            return

        # Get last entry, defining start of gradient
        last_entry = color_table.GetColorEntry(n_entries - 1)

        c1 = last_entry[0]
        c2 = last_entry[1]
        c3 = last_entry[2]
        c4 = last_entry[3]

        # Note: Invalid hues are set to 0 (red)

        # Loop new entries (last will be set to poEntry)
        for ii in range(1, n_new):
            gradient_pcnt = float(ii) / n_new

            # Compute gradient
            red = int((entry[0] - c1) * gradient_pcnt + c1 + .5)
            green = int((entry[1] - c2) * gradient_pcnt + c2 + .5)
            blue = int((entry[2] - c3) * gradient_pcnt + c3 + .5)
            if (len(entry) > 3):
                alpha = int((entry[3] - c4) * gradient_pcnt + c4 + .5)
            else:
                alpha = int((255 - c4) * gradient_pcnt + c4 + .5)

            if (red < 0):
                red = 0
            if (red > 255):
                red = 255
            if (green < 0):
                green = 0
            if (green > 255):
                green = 255
            if (blue < 0):
                blue = 0
            if (blue > 255):
                blue = 255
            if (alpha < 0):
                alpha = 0
            if (alpha > 255):
                alpha = 255

            # Set gradient entry
            color_table.SetColorEntry(n_entries + ii - 1, (red, green, blue, alpha))

        # Set final entry
        color_table.SetColorEntry(index, entry)

    add_gradient_color_entry = staticmethod(add_gradient_color_entry)


def create_RGB_dataset(red_band_info, green_band_info, blue_band_info,
        Description=None, r_lut=None, g_lut=None, b_lut=None):
    """ Create a VRT dataset using the three specified band indexes in the 
        specified dataset for red, green, and blue bands.  The band sizes and
        datatypes will be based on the source dataset.

        *_band_info = (<filename>, <band index>, (lut_min, lut_max))

    """
    assert isinstance(red_band_info, tuple) and len(red_band_info)==3
    assert isinstance(green_band_info, tuple) and len(green_band_info)==3
    assert isinstance(blue_band_info, tuple) and len(blue_band_info)==3
    red_band_idx = red_band_info[1]
    green_band_idx = green_band_info[1]
    blue_band_idx = blue_band_info[1]

    red_band_file = red_band_info[0]
    green_band_file = green_band_info[0]
    blue_band_file = blue_band_info[0]

    red_min, red_max = red_band_info[2]
    green_min, green_max = green_band_info[2]
    blue_min, blue_max = blue_band_info[2]

    red_ds = gdal.OpenShared(red_band_file)
    green_ds = gdal.OpenShared(green_band_file)
    blue_ds = gdal.OpenShared(blue_band_file)

    if red_band_idx < 1 or red_band_idx > red_ds.RasterCount:
        raise 'Invalid red band index'
    if green_band_idx < 1 or green_band_idx > green_ds.RasterCount:
        raise 'Invalid green band index'
    if blue_band_idx < 1 or blue_band_idx > blue_ds.RasterCount:
        raise 'Invalid blue band index'

    dt = gdal.GetDataTypeName(red_ds.GetRasterBand(red_band_idx).DataType)
    vrt_ds = VRTDatasetXMLUtil(None, None, dt,
                               shape=(red_ds.RasterXSize, red_ds.RasterYSize))
    vrt_ds.add_band_with_src(red_band_file, red_band_idx, ColorInterp="Red", Description="Red (" +
                    red_ds.GetRasterBand(red_band_idx).GetDescription() + ")",
                    enhance=r_lut, 
                    lut_min=red_min, lut_max=red_max)
    vrt_ds.add_band_with_src(green_band_file, green_band_idx, ColorInterp="Green", Description="Green (" +
                    green_ds.GetRasterBand(green_band_idx).GetDescription() + ")",
                    enhance=g_lut, 
                    lut_min=green_min, lut_max=green_max)
    vrt_ds.add_band_with_src(blue_band_file, blue_band_idx, ColorInterp="Blue", Description="Blue (" +
                    blue_ds.GetRasterBand(blue_band_idx).GetDescription() + ")",
                    enhance=b_lut, 
                    lut_min=blue_min, lut_max=blue_max)

    return vrt_ds


def create_PSCI_dataset(hue_band_info, intensity_band_info=None,
        Description=None, color_table=None, h_lut=None, i_lut=None):
    """ Create a VRT dataset using the specified band indexes in the 
        specified dataset for hue and intensity bands.  The band sizes and
        datatypes will be based on the source dataset.

        *_band_info = (<filename>, <band index>, (lut_min, lut_max))

     """
    assert isinstance(hue_band_info, tuple) and len(hue_band_info)==3
    if intensity_band_info is not None:
        assert isinstance(intensity_band_info, tuple) \
                and len(intensity_band_info)==3

    hue_band_idx = hue_band_info[1]
    hue_band_file = hue_band_info[0]
    hue_min, hue_max = hue_band_info[2]
    hue_ds = gdal.OpenShared(hue_band_file)
    if hue_band_idx < 1 or hue_band_idx > hue_ds.RasterCount:
        raise 'Invalid hue band index'

    dt = gdal.GetDataTypeName(hue_ds.GetRasterBand(hue_band_idx).DataType)
    vrt_ds = VRTDatasetXMLUtil(None, None, dt, Description,
                               shape=(hue_ds.RasterXSize, hue_ds.RasterYSize))

    vrt_ds.add_band_with_src(hue_band_file, hue_band_idx, ColorInterp="Palette", 
                    Description="Hue (" +
                    hue_ds.GetRasterBand(hue_band_idx).GetDescription() + ")",
                    colortable=color_table, enhance=h_lut, 
                    lut_min=hue_min, lut_max=hue_max)

    if intensity_band_info is not None:
        intensity_band_idx = intensity_band_info[1]
        intensity_band_file = intensity_band_info[0]
        intensity_min, intensity_max = intensity_band_info[2]
        intensity_ds = gdal.OpenShared(intensity_band_file)

        if intensity_band_idx < 1 or intensity_band_idx > intensity_ds.RasterCount:
            raise 'Invalid intensity band index'
        vrt_ds.add_band_with_src(intensity_band_file, intensity_band_idx,
                        Description="Intensity (" +
                        intensity_ds.GetRasterBand(intensity_band_idx).GetDescription() + ")",
                        enhance=i_lut, 
                        lut_min=intensity_min, lut_max=intensity_max)

    return vrt_ds


def dict_append_lut_type(lut_type, metadict=None):
    if lut_type in GV_RASTER_LUT_ENHANCE_TYPES:
        fn_name = GV_RASTER_LUT_ENHANCE_TYPES.get(lut_type)
    else:
        if lut_type in GV_RASTER_LUT_ENHANCE_TYPES.values():
            fn_name = lut_type
        else:
            fn_name = None
    if fn_name is not None:
        if metadict is None:
            metadict = {}
        metadict[GV_LUT_TYPE] = fn_name
    return metadict

def dict_append_min_max(lut_min=None, lut_max=None, metadict=None):
    if metadict is None:
        metadict = {}
    if lut_min is not None:
        metadict[GV_LUT_MIN] = str(lut_min)
    if lut_max is not None:
        metadict[GV_LUT_MAX] = str(lut_max)
    return metadict


if __name__ ==  '__main__':
    import string

    ds=VRTDatasetConstructor(2000,2000)
    ds.AddSimpleBand('reltest.tif',2,'Float32')
    ds.AddSimpleBand('/data/abstest.tif',1,'Byte',
                     SrcRect=(1000,2000,4000,4000),
                     ScaleOffset=2,ScaleRatio=3)
    ds.AddRawBand('rawtest.x00','CFloat32','MSB',0,8,16000)
    for item in string.split(ds.GetVRTLines(),'\n'):
        print item

    #
    # Open a dataset for the following tests.  File must be set to
    # an existing file or exception will be raised and tests skipped.
    #
    src_ds = gdal.OpenShared('d:/data/polsar/yogi_pauli_hh_hhvvmagnitude.vrt')

    #
    # RGB Dataset
    #
    print "Test RGB Dataset:"
    ds = VRTDatasetXMLUtil.create_RGB_dataset('blah_rgb.vrt', src_ds, 1, 1, 2,
                                              "RGB Dataset from yogi_pauli_hh_hhvvmagnitude.vrt",
                                              r_lut=LUT_ENHANCE_ROOT, b_lut=LUT_ENHANCE_SQUARE)
    print ds.GetVRTString()

    #
    # PSCI Dataset
    #
    print "Test PSCI Dataset:"
    color_table = VRTDatasetXMLUtil.create_color_table(CT_SPECTRUM)

    ds = VRTDatasetXMLUtil.create_PSCI_dataset('blah_psci.vrt', src_ds, 2, 1,
                                               "PSCI Dataset from yogi_pauli_hh_hhvvmagnitude.vrt",
                                               color_table, i_lut=LUT_ENHANCE_LOG)
    print ds.GetVRTString()

    #
    # Dataset with derived bands
    #
    print "Test Derived Band Dataset:"

    ds = VRTDatasetXMLUtil('blah_derived.vrt', src_ds)
    bbase = ds.add_band(PixFunctionName="PixFuncBleep",
                        Description="A Magnificent Derived Band")
    ds.add_source(bbase, 1);
    ds.add_source(bbase, 2);

    print ds.GetVRTString()



