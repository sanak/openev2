##############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Perform fusion (aka:pan merging, pansharpening) of panchromatic and RGB.
# Author:  Mario Beauchamp (starged@gmail.com)
# TODO: fix progress
#
# Thanks to John Graham and the SDSU for funding this work when it was needed :)
#
###############################################################################
# This software is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
# 
# You should have received a copy of the GNU Library General Public
# License along with this software; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
###############################################################################

import os
from osgeo import gdal
# uncomment to time
##import time

# temporary
def _(s):
    return s

class GeoTransform:
    """ A more practical way to deal with geotransforms... """
    def __init__(self, geoTr):
        self.set(geoTr)
    
    def __str__(self):
        s = str(self.toList())
        return s[1:-1]

    def set(self, geoTr):
        if geoTr is None: # may not be necessary
            return
        self.ulx = geoTr[0]
        self.xRot = geoTr[2]
        self.uly = geoTr[3]
        self.xRez = geoTr[1]
        self.yRot = geoTr[4]
        self.yRez = geoTr[5]
        
    def toList(self):
        return [self.ulx,self.xRez,self.xRot,self.uly,self.yRot,self.yRez]

class FusionSource:
    def __init__(self, ds):
        self.setDataset(ds)

    def setDataset(self, ds):
        geoTr = ds.GetGeoTransform()
        self.ulx = geoTr[0]
        self.uly = geoTr[3]
        self.xRez = geoTr[1]
        self.yRez = geoTr[5]
        self.xSize = ds.RasterXSize
        self.ySize = ds.RasterYSize
        self.xOff = 0
        self.yOff = 0
        self.xSrc = 0
        self.ySrc = 0
        self.dtype = ds.GetRasterBand(1).DataType
        self.ds = ds

    def GetGeoTransform(self):
        return GeoTransform(self.ds.GetGeoTransform())

    def getExtents(self):
        geoTr = self.GetGeoTransform()
        xmin = geoTr.ulx
        ymax = geoTr.uly
        xmax = xmin + self.ds.RasterXSize*geoTr.xRez
        ymin = ymax + self.ds.RasterYSize*geoTr.yRez
        return xmin,xmax,ymin,ymax

class FusionObject:
    def __init__(self):
        self.rgb = None
        self.pan = None
        self.method = 'Kernel'
        self.xSize = 0
        self.ySize = 0
        self.xRez = 1.0
        self.yRez = -1.0
        self.ratio = 1.0
        self.sharp = '0.15'
        self.frmt = 'GTiff'
        self.dtype = 'Byte'
        self.swaprb = 0
        self.blockX = 128
        self.blockY = 128
        self.copt = ['tiled=yes']
        self.alg = 'Cubic Spline'
        self.reuse = 0
        self.geoTr = None
        self.resizeFn = None
        self.xfile = ''
        self.srcDt = None
        self.rgbds = None
        self.rgbxds = None
        self.rgbvrt = None
        self.reszCmd = None

    def setSources(self, srcrgb, srcpan, win=None):
        self.setRGB(srcrgb)
        self.setPan(srcpan)
        if self.rgb is None or self.pan is None:
            return
        self.setSize(win)
    
    def setRGB(self, srcrgb):
        if self.rgb is not None:
            self.rgb.setDataset(srcrgb)
        else:
            try: # we don't do ungeoref'd
                geoTr = srcrgb.GetGeoTransform()
                proj = srcrgb.GetProjection()
            except:
                proj = None
            if proj is None:
                return
            self.rgb = FusionSource(srcrgb)
        
        rgbpath = srcrgb.GetDescription()
        rgbname = os.path.splitext(rgbpath)[0]
        self.xfile = rgbname + '-x'
        self.rgbvrt = rgbname + '-in.vrt'
        self.ratio = self.rgb.xRez/self.xRez

    def setPan(self, srcpan):
        if self.pan is not None:
            self.pan.setDataset(srcpan)
        else:
            try: # we don't do ungeoref'd
                geoTr = srcpan.GetGeoTransform()
                proj = srcpan.GetProjection()
            except:
                proj = None
            if proj is None:
                return
            self.pan = FusionSource(srcpan)

        self.xRez = self.pan.xRez
        self.yRez = self.pan.yRez
        self.srcDt = self.pan.dtype
        self.ratio = self.rgb.xRez/self.xRez
        self.pan.xOff = 0
        self.pan.yOff = 0

    def setOutputFile(self, outfile=None):
        if outfile is None:
            meta = gdal.GetDriverByName(self.frmt).GetMetadata()
            rgbpath = self.rgb.ds.GetDescription()
            rgbname = os.path.splitext(rgbpath)[0]
            outfn = rgbname + '-f.'
            if 'DMD_EXTENSION' in meta:
                outfn += meta.get('DMD_EXTENSION')
        else:
            outfn = outfile
        self.outfile = outfn

    def setSize(self, win=None):
        xRez = self.xRez
        yRez = self.yRez
        xOff = 0
        yOff = 0
        winx = 0
        winy = 0
        xminRgb,xmaxRgb,yminRgb,ymaxRgb = self.rgb.getExtents()
        xminPan,xmaxPan,yminPan,ymaxPan = self.pan.getExtents()
        if win is not None:
            xOff = win[0]
            yOff = win[1]
            winx = win[2]
            winy = win[3]
        if winx > 0:
            xmin = max(xminRgb,xminPan,xOff)
            xmax = min(xmaxRgb,xmaxPan,winx+xOff)
        else:
            xmin = max(xminRgb,xminPan)
            xmax = min(xmaxRgb,xmaxPan)
        if winy > 0:
            ymin = max(yminRgb,yminPan,yOff)
            ymax = min(ymaxRgb,ymaxPan,winy+yOff)
        else:
            ymin = max(yminRgb,yminPan)
            ymax = min(ymaxRgb,ymaxPan)

        # snap to RGB pixel (to be improved)
        if self.ratio > 1:
            # correct x's
            rez = self.rgb.xRez
            halfrez = rez/2.0
            diff = (xmin-xminRgb)%rez
            if diff > 0:
                xmin += rez-diff
            diff = (xmax-xmin)%rez
            if diff > 0:
                xmax -= diff

            # correct y's
            rez = -self.rgb.yRez
            diff = (ymin-yminRgb)%rez
            if diff > 0:
                ymin += rez-diff
            diff = (ymax-ymin)%rez
            if diff > 0:
                ymax -= diff

        self.xSize = int( round((xmax - xmin)/xRez) )
        self.ySize = int( round((ymax - ymin)/-yRez) )

        self.ulx = xmin
        self.uly = ymax
        self.rgb.xOff = int( round((xmin - xminRgb)/self.rgb.xRez) )
        self.rgb.yOff = int( round((ymax - ymaxRgb)/self.rgb.yRez) )
        self.rgb.xSize = int( round(self.xSize/self.ratio) )
        self.rgb.ySize = int( round(self.ySize/self.ratio) )
        self.pan.xOff += int( round((xmin - xminPan)/xRez) )
        self.pan.yOff += int( round((ymax - ymaxPan)/yRez) )

    def getGeoTransform(self):
        geoTr = GeoTransform([self.ulx, self.xRez, 0.0, self.uly, 0.0, self.yRez])
        return geoTr.toList()

    def setTiling(self, blkx, blky):
        self.blockX = blkx
        self.blockY = blky
        if blkx > 0:
            self.copt.append('blockxsize=%s' % blkx)
            self.copt.append('blockysize=%s' % blky)
        elif 'tiled=yes' in self.copt:
            self.copt.remove('tiled=yes')

    def getResamplingAlg(self):
        alg = None
        if self.resizeFn == self.gdalWarp:
            if self.alg == 'Bilinear':
                alg = '-rb'
            elif self.alg == 'Cubic':
                alg = '-rc'
            elif self.alg == 'Cubic Spline':
                alg = '-rcs'
            else:
                alg = '-rn'
        elif self.resizeFn == self.reprojectImage:
            if self.alg == 'Bilinear':
                alg = gdal.GRA_Bilinear
            elif self.alg == 'Cubic':
                alg = gdal.GRA_Cubic
            elif self.alg == 'Cubic Spline':
                alg = gdal.GRA_CubicSpline
            else:
                alg = gdal.GRA_NearestNeighbour

        return alg

    def validateResized(self):
        rgbxds = None
        xvrt = self.xfile + '.vrt'
        if self.rgbxds is not None:
            rgbxds = self.rgbxds
        elif os.path.exists(xvrt):
            rgbxds = gdal.Open(xvrt)
            if rgbxds is None:
                os.remove(xvrt)
                if os.path.exists(self.xfile):
                    os.remove(self.xfile)
                return
        else:
            return

        if rgbxds.RasterXSize == self.xSize and rgbxds.RasterYSize == self.ySize:
            geoTr = rgbxds.GetGeoTransform()
            if geoTr[0] == self.ulx and geoTr[3] == self.uly:
                return rgbxds

        rgbxds = None
        os.remove(xvrt)
        if os.path.exists(self.xfile):
            os.remove(self.xfile)

    def constructFilteredVRT(self):
        xsz = str(self.xSize)
        ysz = str(self.ySize)
        xoff = str(self.pan.xOff)
        yoff = str(self.pan.yOff)
        if self.method == 'IHS':
            vrtStr = '<VRTDataset rasterXSize=\"'+xsz+'\" rasterYSize=\"'+ysz+'\">\n'
            vrtStr += '  <VRTRasterBand dataType="Float32" band="1">\n'
            vrtStr += '    <SimpleSource>\n      <SourceFilename relativeToVRT=\"0\">'
            vrtStr += self.pan.ds.GetDescription()+'</SourceFilename>\n      <SourceBand>1</SourceBand>\n'
            vrtStr += '      <SrcRect xOff=\"'+xoff+'\" yOff=\"'+yoff+'\" xSize=\"'+xsz+'\" ySize=\"'+ysz+'\"/>\n'
            vrtStr += '      <DstRect xOff=\"0\" yOff=\"0\" xSize=\"'+xsz+'\" ySize=\"'+ysz+'\"/>\n'
            vrtStr += '    </SimpleSource>\n  </VRTRasterBand>\n</VRTDataset>\n'
        else:
            vrtStr = '<VRTDataset rasterXSize=\"'+xsz+'\" rasterYSize=\"'+ysz+'\">\n'
            vrtStr += '  <VRTRasterBand dataType="Float32" band="1">\n'
            vrtStr += '    <KernelFilteredSource>\n      <SourceFilename relativeToVRT=\"0\">'
            vrtStr += self.pan.ds.GetDescription()+'</SourceFilename>\n      <SourceBand>1</SourceBand>\n'
            vrtStr += '      <SrcRect xOff=\"'+xoff+'\" yOff=\"'+yoff+'\" xSize=\"'+xsz+'\" ySize=\"'+ysz+'\"/>\n'
            vrtStr += '      <DstRect xOff=\"0\" yOff=\"0\" xSize=\"'+xsz+'\" ySize=\"'+ysz+'\"/>\n'
            vrtStr += '      <Kernel normalized=\"0\">\n        <Size>3</Size>\n'
            vrtStr += '        <Coefs>-1 -1 -1 -1 8 -1 -1 -1 -1</Coefs>\n      </Kernel>'
            vrtStr += '      <ScaleRatio>'+self.sharp+'</ScaleRatio>'
            vrtStr += '    </KernelFilteredSource>\n  </VRTRasterBand>\n</VRTDataset>\n'

        ds = gdal.Open(vrtStr, gdal.GA_Update)
        if ds is None:
            return

        ds.SetGeoTransform(self.getGeoTransform())
        ds.SetProjection(self.pan.ds.GetProjection())

        if saveVrt:
            drv = ds.GetDriver()
            drv.CreateCopy(os.path.join(os.curdir,'panfiltered.vrt'), ds)

        return ds

    #################################### Resizing functions #####################################
    def gdalWarp(self):
        cmd = 'gdalwarp -et 0.0 -wt Float32 '
        alg = self.getResamplingAlg()
        cmd += ' '.join(('-of', self.frmt,
                        '-ts', str(self.xSize), str(self.ySize),
                        alg, '-co tiled=yes',
                        self.rgbvrt, self.xfile))
        os.system('echo ' + cmd)
        err = os.system(cmd)
        if not err:
            return gdal.Open(self.xfile)

    def reprojectImage(self):
        drv = gdal.GetDriverByName(self.frmt)
        rgbxds = drv.Create(self.xfile, self.xSize, self.ySize,
                             bands=3,
                             eType=self.srcDt,
                             options=['tiled=yes'])
        try:
            rgbxds.SetProjection(self.rgb.ds.GetProjection())
            rgbxds.SetGeoTransform(self.getGeoTransform())
        except:
            pass
        
        alg = self.getResamplingAlg()
        try:
            gdal.ReprojectImage(gdal.Open(self.rgbvrt), rgbxds, eResampleAlg=alg)
        except:
            rgbxds = None
        return rgbxds

    #############################################################################################
    def prepareDatasets(self):
        pands = self.constructFilteredVRT()
        if pands is None:
            print "Could not process "+os.path.basename(self.pan.ds.GetDescription())
            return None,None
    
        self.rgbds = self.processRGB()
        if self.rgbds is None:
            print "Could not process "+os.path.basename(self.rgb.ds.GetDescription())
            return None,None

        if self.dtype == 'UInt16':
            dt = gdal.GDT_UInt16
        else:
            dt = gdal.GDT_Byte
        if self.outfile[:7] == 'preview':
            frmt = 'Mem'
        else:
            frmt = self.frmt
        outds = gdal.GetDriverByName(frmt).Create(self.outfile, self.xSize, self.ySize,
                                                    bands=3, eType=dt, options=self.copt)
        
        try: # just in case rgbds has no proj or output format doesn't support georef
            outds.SetProjection(self.rgb.ds.GetProjection())
            outds.SetGeoTransform(self.getGeoTransform())
        except:
            pass
    
        return pands,outds
    
    def processRGB(self):
        srcwin = ' '.join(('-srcwin',str(self.rgb.xOff),str(self.rgb.yOff),
                                     str(self.rgb.xSize),str(self.rgb.ySize)))
    
        rgbfile = self.rgb.ds.GetDescription()
        cmd = ' '.join(('gdal_translate -of VRT',srcwin,rgbfile,self.rgbvrt))
        os.system(cmd)

        if self.ratio != 1:
            if self.reuse:
                rgbxds = self.validateResized()
                if rgbxds is None:
                    rgbxds = self.resizeFn()
            else:
                if os.path.exists(self.xfile):
                    os.remove(self.xfile)
                rgbxds = self.resizeFn()
        else:
            rgbxds = gdal.Open(self.rgbvrt)

        if rgbxds is None:
            return

        if rgbxds.GetDriver().ShortName == 'VRT':
            rgbds = rgbxds
        else:
            drv = gdal.GetDriverByName('VRT')
            rgbds = drv.CreateCopy(self.xfile+'.vrt', rgbxds)
        
        rgbds.SetProjection(self.rgb.ds.GetProjection())
        rgbds.SetGeoTransform(self.getGeoTransform())
        return rgbds
    
    def fuseBands(self, panDS, outDS, progress=None):
        from osgeo._gdal import Band_WriteRaster
        rgbDS = self.rgbds
        try:
            from numpy import clip, maximum, zeros
        except:
            from Numeric import clip, maximum, zeros
        # input bands
        if self.swaprb:
            redBand = rgbDS.GetRasterBand(3)
            blueBand = rgbDS.GetRasterBand(1)
        else:
            redBand = rgbDS.GetRasterBand(1)
            blueBand = rgbDS.GetRasterBand(3)
        greenBand = rgbDS.GetRasterBand(2)
        panBand = panDS.GetRasterBand(1)
    
        # output bands
        redBandOut = outDS.GetRasterBand(1)
        greenBandOut = outDS.GetRasterBand(2)
        blueBandOut = outDS.GetRasterBand(3)

        xbloff,ybloff,xsz,ysz = 0,0,0,0
        # for clarity...
        xSize = self.xSize
        ySize = self.ySize
        blkX = self.blockX
        blkY = self.blockY

        def ihsMergePix(R, G, B, I):
            scale = 3.0 * I/(R+G+B+1.0)
            scaleFn(R*scale, G*scale, B*scale)
    
        def mergePix(R, G, B, I):
            scaleFn(I+R, I+G, I+B)
    
        def clipToByte(R, G, B):
            writeBands(clip(R, 0, 255).astype('b'),
                       clip(G, 0, 255).astype('b'),
                       clip(B, 0, 255).astype('b'))
    
        def clipAndScale(R, G, B):
            writeBands(clip(R*0.124573, 0, 255).astype('b'),
                       clip(G*0.124573, 0, 255).astype('b'),
                       clip(B*0.124573, 0, 255).astype('b'))
    
        def clipToInt(R, G, B):
            writeBands(clip(R, 0, 2047).astype('w'),
                       clip(G, 0, 2047).astype('w'),
                       clip(B, 0, 2047).astype('w'))
    
        if self.dtype == 'Byte':
            outtype = gdal.GDT_Byte
            if self.srcDt == gdal.GDT_Byte:
                scaleFn = clipToByte
                typecode = 'b'
            else:
                scaleFn = clipAndScale
                typecode = 'w'
        else:
            outtype = gdal.GDT_UInt16
            scaleFn = clipToInt
            typecode = 'w'

        def writeBands(outR, outG, outB):
            Band_WriteRaster(redBandOut, xbloff, ybloff, xsz, ysz, outR.tostring(), xsz, ysz, outtype)
            Band_WriteRaster(greenBandOut, xbloff, ybloff, xsz, ysz, outG.tostring(), xsz, ysz, outtype)
            Band_WriteRaster(blueBandOut, xbloff, ybloff, xsz, ysz, outB.tostring(), xsz, ysz, outtype)
    
        def processBands():
            # read into buffers
            red = redBand.ReadAsArray(xbloff, ybloff, xsz, ysz, xsz, ysz)
            green = greenBand.ReadAsArray(xbloff, ybloff, xsz, ysz, xsz, ysz)
            blue = blueBand.ReadAsArray(xbloff, ybloff, xsz, ysz, xsz, ysz)
            pan = panBand.ReadAsArray(xbloff, ybloff, xsz, ysz, xsz, ysz)
            # merge
            mergeFn(red.astype('f'), green.astype('f'), blue.astype('f'), pan)

        def reportProgress(complete):
            if progress is None:
                gdal.TermProgress(complete)
            else:
                if progress.ProgressCB(complete,'') == 0:
                    progress.destroy()
                    return 1

        if self.method == 'IHS':
            mergeFn = ihsMergePix
        else:
            mergeFn = mergePix
    
        if progress is None:
            gdal.TermProgress(0.0, msg="Merging...\n")
    
        if blkX == 0:
            denom = ySize - 1
            xbloff = 0
            xsz = xSize
            ysz = 1
            for line in range(ySize):
                ybloff = line
                processBands()
                if reportProgress(float(line)/denom):
                    return 0
        else:
            fullX = int(xSize/blkX)
            fullY = int(ySize/blkY)
            for yblk in range(fullY):
                xsz = blkX
                ysz = blkY
                ybloff = yblk * blkY
                for xblk in range(fullX):
                    xbloff = xblk * blkX
                    processBands()

                # deal with last Xblock, if any
                xbloff = fullX * blkX
                if xSize > xbloff:
                    xsz = xSize - xbloff
                    ysz = max(blkY, ySize - fullY*blkY)
                    processBands()
    
                if reportProgress(float(ybloff)/ySize):
                    return 0

            # deal with last Yblock, if any
            ybloff = fullY * blkY
            if ySize > ybloff:
                xsz = blkX
                ysz = ySize - ybloff
                for xblk in range(fullX):
                    xbloff = xblk * blkX
                    processBands()
    
                # deal with last Xblock, if any
                xbloff = fullX * blkX
                if xSize > xbloff:
                    xsz = xSize - xbloff
                    processBands()

                if reportProgress(float(ybloff)/ySize):
                    return 0

        if progress is not None:
            progress.destroy()

        return 1

# set to 1 to save pan filtered VRT
saveVrt = 0

def open_rgb_file(filename):
    ds = gdal.Open(filename)
    if ds.RasterCount > 2:
        return ds
    del ds # is that necessary?

def open_pan_file(filename):
    ds = gdal.Open(filename)
    if ds.RasterCount == 1:
        return ds
    del ds # is that necessary?

def getLayersDict(view):
    rgbdict = {}
    pandict = {}
    if gview.get_preference('save_recent_directory') == 'on':
        recent_dir = gview.get_preference('recent_directory')
    else:
        recent_dir = None


    for layer in view.viewarea.list_layers():
        mode = layer.get_mode()
        name = os.path.basename(layer.get_name())
        if mode == gview.RLM_RGBA:
            rgbdict[name] = layer
        elif mode == gview.RLM_SINGLE:
            pandict[name] = layer

    # if no RGB or pan layer in the active view, open one
    if not rgbdict:
        filename = pgufilesel.GetFileName("Select RGB", recent_dir)
        if filename:
            ds = open_rgb_file(filename)
            if ds:
                name = os.path.basename(filename)
                view.open_gdal_dataset(ds)
                rgbdict[name] = view.viewarea.active_layer()

    if not pandict:
        filename = pgufilesel.GetFileName("Select Pan", recent_dir)
        if filename:
            ds = open_pan_file(filename)
            if ds:
                name = os.path.basename(filename)
                view.open_gdal_dataset(ds)
                pandict[name] = view.viewarea.active_layer()

    return rgbdict,pandict

def GetDriverList():
    list = []
    for iDriver in range(gdal.GetDriverCount()):
        list.append( gdal.GetDriver(iDriver) )
    return list

if __name__ != '__main__':
    import gtk
    import gview
    import gvutils
    import gviewapp
    import pgu
    import pgufilesel

    class FusionDialog(gtk.Window):
        def __init__(self, app=None):
            gtk.Window.__init__(self)
            self.set_title("Image Fusion")
            self.set_resizable(False)

            self.app = app
            view = app.view_manager.get_active_view_window()
            self.nprv = 0

            self.rgbDict, self.panDict = getLayersDict(view)
            if not (self.rgbDict and self.panDict):
                gvutils.error("RGB and Pan images required")
                self.destroy()
                return None

            self.roichanged_id = app.toolbar.roi_tool.connect('roi-changed',self.getROIinfo)
            app.toolbar.roi_button.set_active(True)
    
            self.createGUI()
            self.show_all()
    
            self.fus = self.initFusionObject()
            if self.fus is None:
                gvutils.error("Could not initialize Fusion")
                self.close()
            self.updateExtentEntries()
            self.setFilename()

        def initFusionObject(self):
            fus = FusionObject()
            layer = self.rgbDict[self.rgbDict.keys()[0]]
            rgb = layer.get_dataset()
            layer = self.panDict[self.panDict.keys()[0]]
            pan = layer.get_dataset()

            try:
                fus.setSources(rgb, pan)
            except:
                return

            return fus

        def createGUI(self):
            mainbox = gtk.VBox(spacing=5)
            mainbox.set_border_width(5)
            self.add(mainbox)

            frame = gtk.Frame("Input")
            mainbox.pack_start(frame, expand=False)
    
            table = gtk.Table()
            table.set_border_width(5)
            table.set_row_spacings(5)
            table.set_col_spacings(5)
            frame.add(table)

            # Input layers
            row = 0
            # RGB source
            label = pgu.Label("RGB:")
            table.attach(label, 0, 1, row, row+1)
            self.rgbCB = pgu.ComboText(strings=self.rgbDict.keys())
            table.attach(self.rgbCB, 1, 2, row, row+1)

            row += 1
            # Pan source
            label = pgu.Label("Pan:")
            table.attach(label, 0, 1, row, row+1)
            self.panCB = pgu.ComboText(strings=self.panDict.keys())
            table.attach(self.panCB, 1, 2, row, row+1)

            # output
            frame = gtk.Frame("Output")
            mainbox.pack_start(frame, expand=False)
    
            table = gtk.Table()
            table.set_border_width(5)
            table.set_row_spacings(5)
            table.set_col_spacings(5)
            frame.add(table)

            row = 0
            label = pgu.Label("Format:")
            table.attach(label, 0, 1, row, row+1)
            wDrvs = filter(lambda drv: 'DCAP_CREATE' in drv.GetMetadata(), GetDriverList())
            drivers = map(lambda d: d.ShortName, wDrvs)
            drivers.sort()
            self.formatCB = pgu.ComboText(strings=drivers)
            self.formatCB.set_active_text('GTiff')
            self.formatCB.set_tooltip_text("Output formats. Some may not work.")
            table.attach(self.formatCB, 1, 2, row, row+1)
    
            row += 1
            label = pgu.Label("Output file:")
            table.attach(label, 0, 1, row, row+1)
            self.outTE = gtk.Entry()
            table.attach(self.outTE, 1, 2, row, row+1)
    
            row += 1
            label = pgu.Label("Create options:")
            table.attach(label, 0, 1, row, row+1)
            self.coptTE = gtk.Entry()
            self.coptTE.set_text("tiled=yes")
            table.attach(self.coptTE, 1, 2, row, row+1)
    
            # resampling options
            frame = gtk.Frame("Resampling")
            mainbox.pack_start(frame, expand=False)
    
            table = gtk.Table()
            table.set_border_width(5)
            table.set_row_spacings(5)
            table.set_col_spacings(5)
            frame.add(table)

            row = 0
            # warp options
            label = pgu.Label("Algorithm")
            table.attach(label, 0, 1, row, row+1)
            self.algCB = pgu.ComboText(strings=("Nearest Neighbor","Bilinear","Cubic","Cubic Spline"))
            table.attach(self.algCB, 1, 2, row, row+1)
            self.algCB.set_tooltip_text("Resampling algorithm. Cubic Spline is recommended for best results")
    
            row += 1
            label = pgu.Label("Resize:")
            table.attach(label, 0, 1, row, row+1)
            box = gtk.HBox(spacing=5)
            table.attach(box, 1, 2, row, row+1)

            self.reszFn0CB = gtk.RadioButton(label="Gdalwarp")
            box.pack_start(self.reszFn0CB, expand=False)
            self.reszFn1CB = gtk.RadioButton(label="gdal.py", group=self.reszFn0CB)
            box.pack_end(self.reszFn1CB, expand=False)

            row += 1
            label = pgu.Label("Datatype:")
            table.attach(label, 0, 1, row, row+1)
            box = gtk.HBox(spacing=5)
            table.attach(box, 1, 2, row, row+1)

            row += 1
            self.swapTO = gtk.CheckButton(label="Swap R-B")
            table.attach(self.swapTO, 0, 2, row, row+1)
            self.swapTO.set_tooltip_text("Swap Red and Blue bands. Used for Quickbird.")
    
            self.sat0RB = gtk.RadioButton(label="Byte")
            box.pack_start(self.sat0RB, expand=False)
            self.sat1RB = gtk.RadioButton(label="UInt16", group=self.sat0RB)
            box.pack_end(self.sat1RB, expand=False)
    
            # window options
            frame = gtk.Frame("Window")
            mainbox.pack_start(frame, expand=False)
    
            table = gtk.Table()
            table.set_border_width(5)
            table.set_row_spacings(5)
            table.set_col_spacings(5)
            frame.add(table)

            row = 0
            self.xoffTE = pgu.LabelEntry("Pixel:", width=50)
            table.attach(self.xoffTE, 0, 1, row, row+1)

            self.yoffTE = pgu.LabelEntry("Line:", width=50)
            table.attach(self.yoffTE, 1, 2, row, row+1)

            row += 1
            self.widthTE = pgu.LabelEntry("Width:", width=50)
            table.attach(self.widthTE, 0, 1, row, row+1)

            self.heightTE = pgu.LabelEntry("Height:", width=50)
            table.attach(self.heightTE, 1, 2, row, row+1)

            row += 1
            self.panXoffTE = pgu.LabelEntry("X offset:", width=50)
            self.panXoffTE.set_text("0")
            table.attach(self.panXoffTE, 0, 1, row, row+1)
            self.panXoffTE.set_tooltip_text("Pan X offset")

            self.panYoffTE = pgu.LabelEntry("Y offset:", width=50)
            self.panYoffTE.set_text("0")
            table.attach(self.panYoffTE, 1, 2, row, row+1)
            self.panYoffTE.set_tooltip_text("Pan Y offset")
    
            # Params
            frame = gtk.Frame("Fusion Method")
            mainbox.pack_start(frame, expand=False)
            vbox = gtk.VBox(spacing=3)
            vbox.set_border_width(5)
            frame.add(vbox)
            box = gtk.HBox(spacing=5)
            vbox.add(box)
    
            self.met0RB = gtk.RadioButton(label="IHS")
            tipTxt = "Standard Intensity-Hue-Saturation merging. "
            tipTxt += "Sharpness setting has no effect. "
            self.met0RB.set_tooltip_text(tipTxt)
            box.add(self.met0RB)
    
            self.met1RB = gtk.RadioButton(label="Kernel",group=self.met0RB)
            tipTxt = "Pan is run through a convolution kernel and added to RGB. "
            tipTxt += "Recommended sharpness: 0.15-0.25 for Ikonos and Quickbird, "
            tipTxt += "0.3-0.5 for Landsat and SPOT. "
            tipTxt += "Can be set higher if imagery is enhanced."
            self.met1RB.set_tooltip_text(tipTxt)
            box.add(self.met1RB)
    
            # params
            vbox2 = gtk.VBox(spacing=5)
            vbox2.set_border_width(5)
            vbox.add(vbox2)
    
            box = gtk.HBox(spacing=5)
            vbox2.add(box)
            box.pack_start(gtk.Label("Sharpness:"), expand=False)
            self.adjSharp = gtk.Adjustment(0.15,0.0,1.0,0.01,0.1)
            slider = gtk.HScale(self.adjSharp)
            slider.set_digits(2)
            box.add(slider)
    
            # Buttons
            box = gtk.HBox(homogeneous=1, spacing=5)
            mainbox.pack_start(box, expand=False)
    
            mergeBT = gtk.Button("Merge")
            mergeBT.connect('clicked', self.compute, 'merge')
            mergeBT.set_tooltip_text("Proceed with merging")
            box.add(mergeBT)
    
            pviewBT = gvutils.create_stock_button('eye', self.compute, 'pview')
            pviewBT.set_tooltip_text("Preview merging")
            box.add(pviewBT)
    
            closeBT = gtk.Button(stock=gtk.STOCK_CLOSE)
            closeBT.connect('clicked',self.close)
            box.add(closeBT)
    
            # do the connects last so events are not fired during init 
            self.panCB.connect(cb=self.panChanged)
            self.rgbCB.connect(cb=self.rgbChanged)
            self.formatCB.connect(cb=self.formatChanged)
    
        def close(self, *args):
            if self.roichanged_id is not None:
                self.app.toolbar.roi_tool.disconnect(self.roichanged_id)
                self.roichanged_id = None
            if self.fus.rgbds is not None:
                del self.fus.rgbds
            # what else to clean up?
            self.destroy()
            return True
    
        def getROIinfo(self, *args):
            fu = self.fus # for clarity...
            try:
                roi_info = self.app.toolbar.get_roi()
            except:
                roi_info = None
    
            fu.pan.xOff = int(self.panXoffTE.get_text())
            fu.pan.yOff = int(self.panYoffTE.get_text())
            fu.setSize(roi_info)
            self.updateExtentEntries()
    
        def formatChanged(self, combo):
            self.fus.frmt = combo.get_active_text()
            self.setFilename()
    
        def rgbChanged(self, combo):
            nkey = combo.get_active_text()
            rgbLayer = self.rgbDict[nkey]
            self.fus.setRGB(rgbLayer.get_dataset())
            self.fus.setSize()
            self.updateExtentEntries()
            self.setFilename()
    
        def panChanged(self, combo):
            nkey = combo.get_active_text()
            panLayer = self.panDict[nkey]
            self.fus.setPan(panLayer.get_dataset())
            self.fus.setSize()
            self.updateExtentEntries()

        def getReszFunc(self):
            if self.reszFn0CB.get_active():
                return self.fus.gdalWarp
            elif self.reszFn1CB.get_active():
                return self.fus.reprojectImage

        def updateExtentEntries(self):
            fu = self.fus # for clarity...
            self.xoffTE.set_text(str(fu.pan.xOff))
            self.yoffTE.set_text(str(fu.pan.yOff))
            self.widthTE.set_text(str(fu.xSize))
            self.heightTE.set_text(str(fu.ySize))

        def updateFusionInfo(self):
            fu = self.fus # for clarity...
            fu.swaprb = self.swapTO.get_active()
            fu.sharp = str(self.adjSharp.get_value())
            fu.outfile = self.outTE.get_text()
            fu.alg = self.algCB.get_active_text()
            fu.frmt = self.formatCB.get_active_text()
            fu.resizeFn = self.getReszFunc()
            fu.copt = [self.coptTE.get_text()]
            fu.dtype = ('Byte','UInt16')[self.sat1RB.get_active()]
            if self.met0RB.get_active():
                fu.method = 'IHS'
            elif self.met1RB.get_active():
                fu.method = 'Kernel'
            if 'tiled=yes' in fu.copt:
                fu.blockX = 128
                fu.blockY = 128

        def compute(self, bt, id):
            progress = pgu.ProgressDialog("Merging images", cancel=True)
            progress.show()
            self.updateFusionInfo()
            fu = self.fus # for clarity...
    
            # uncomment to time
            # t0 = time.clock()
            if id == 'pview':
                self.nprv += 1
                fu.reuse = 1
                fu.outfile = 'preview'+str(self.nprv)
            else:
                fu.reuse = 0

            panDS,outDS = fu.prepareDatasets()
            if panDS is None or fu.rgbds is None or outDS is None:
                gvutils.error("Could not open necessary files.")
                progress.destroy()
                return False

            progress.SetDefaultMessage("merged")
            result = fu.fuseBands(panDS, outDS, progress)
            # uncomment to time
            # print time.clock() - t0
    
            if result:
                if id == 'pview':
                    self.app.open_gdal_dataset(outDS)
                else:
                    del outDS
                    self.app.open_gdal_dataset(gdal.Open(fu.outfile))
            else:
                del outDS
                if not fu.reuse:
                    os.remove(outfile)
                gvutils.error("Error merging images.")
                return False

        def setFilename(self):
            self.fus.setOutputFile()
            self.outTE.set_text(self.fus.outfile)

if __name__ == '__main__':
    import sys
    def Usage():
        print "Usage: fusion.py [-ihs|krnl] [-sharp value] [-ot type]"
        print "                 [-tiled blkx blky] [-of format]"
        print "                 [-rn|rb|rc|rcs] [-swap] [-co NAME=VALUE]*"
        print "                 [-o out_file] [-poff x y] rgb_file pan_file"
        print
        print "where: -ihs: Standard Intensity-Hue-Saturation merging. Of limited use."
        print "       -krnl: Pan is run through a convolution kernel and blended with RGB."
        print "              Used for Landsat, Quickbird and Ikonos imagery. Default method."
        print "       -sharp: Sharpness (0-1). Default: 0.15. Unused with ihs."
        print "       -ot: Output data type. Only Byte and UInt16 supported. Default Byte."
        print "       -tiled: Specify block size. Default is 128. Use -tiled 0 0 to turn off tiling."
        print "       -rn|rb|rc|rcs: Gdalwarp resampling algorithm. Default is -rcs (best but slowest)."
        print "                      Not used if RGB is already the right size."
        print "       -swap: Swap R and B bands. Only used for Quickbird."
        print "       -co: Usual create options."
        print "       -o: Output filename. Defaults to {rgbname}-f.{ext}."
        print "       -poff: Pan band offsets in pixels."

    names = []
    outfile = None

    argv = gdal.GeneralCmdLineProcessor(sys.argv)
    if len(argv) == 0:
        Usage()
        sys.exit(0)

    fus = FusionObject()
    # Parse command line arguments.
    i = 1
    while i < len(argv):
        arg = argv[i]
        if arg == '-ihs':
            fus.method = 'IHS'
        elif arg == '-krnl':
            fus.method = 'Kernel'
        elif arg == '-sharp':
            i += 1
            fus.sharp = argv[i]
        elif arg == '-ot':
            i += 1
            fus.dtype = argv[i]
        elif arg[:2] == '-r':
            fus.alg = arg
        elif arg == '-of':
            i += 1
            fus.frmt = argv[i]
        elif arg == '-o':
            i += 1
            outfile = argv[i]
        elif arg == '-poff':
            fus.panXoff = int(argv[i+1])
            fus.panYoff = int(argv[i+2])
            i += 2
        elif arg == '-co':
            i += 1
            fus.copt.append(argv[i])
        elif arg == '-swap':
            fus.swaprb = 1
        elif arg == '-tiled':
            blockX = int(argv[i+1])
            blockY = int(argv[i+2])
            fus.setTiling(blockX,blockY)
            i += 2
        elif arg[:1] == '-':
            print "Unrecognised command option: ",arg
            Usage()
            sys.exit( 1 )
        else:
            names.append(arg)
        i += 1

    if len(names) != 2:
        print "2 input files are needed."
        Usage()
        sys.exit(1)

    srcrgb = gdal.Open(names[0])
    if srcrgb is None:
        print "Could not open RGB file "+os.path.basename(names[0])
        sys.exit(1)

    srcpan = gdal.Open(names[1])
    if srcpan is None:
        print "Could not open Pan file "+os.path.basename(names[1])
        sys.exit(1)

    try:
        fus.setSources(srcrgb,srcpan)
    except:
        print "Could not initialise fusion parameters."
        sys.exit(1)

    fus.setOutputFile(outfile)
    fus.resizeFn = fus.gdalWarp
    panDS,outDS = fus.prepareDatasets()

    if panDS is None or fus.rgbds is None or outDS is None:
        print "Could not open necessary files"
        sys.exit(1)

    result = fus.fuseBands(panDS,outDS)
    if result:
        print '\n'+fus.outfile+" successfully merged."
    else:
        print "Problem occured."
    outDS.FlushCache()
    del outDS
    sys.exit(0)
