###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Interactive tool for layer scaling with histogram(s)
# Author:   Mario Beauchamp (starged@gmail.com)
#
# Functions: Left-Click in the markers display to set the min scale. 
#            Right-Click to set the max scale.
#            'Refresh': rebuilds the histogram (from layer, not file).
#            '+'/'-': zoomin/zoomout y axis from 10 preset zooms (10.0 to 0.001).
#            'Load': load histogram from file.
#            'Save': save histogram to file. Name of the file defaults to layer's
#                    file name with '.his' extension.
#            If an histogram file already exists for the layer when the window
#               is launched, it's loaded automatically. The tool will check in image
#               dir and in dir defined by histos_directory preference.
#            'Autoscale': automatic scaling.
#            'Min tail'/'Max tail': adjust the amount of tail trimming.
#            Scaling algorithms can be selected in Combo.
# Caution: building histograms for large images may take a while...
#
# TODO: add histogram manipulation functions, move Functions desc somewhere else
#
# Thanks to Frank W. for helping me with the drawing w/colors!
#
###############################################################################
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

import pygtk
pygtk.require('2.0')
import gtk
import gview
import os.path
from osgeo import gdal
from numpy import array, add, sqrt
from pgu import ComboText
from gvutils import create_stock_button

hisDir = gview.get_preference('histos_directory')
zooms = [10.0,5.0,2.5,1.0,0.5,0.1,0.05,0.01,0.005,0.001]

class HistogramFrame(gtk.Frame):
    def __init__(self, name, srcIdx, hisSrc, master):
        gtk.Frame.__init__(self, name)
        self.set_name(name)
        self.srcIdx = srcIdx
        self.master = master
        self.updating = True
        self.connect('destroy', self.cleanup)

        self.min = self.master.layer.min_get(srcIdx)
        self.max = self.master.layer.max_get(srcIdx)

        self.createGUI()
        self.setHisto(hisSrc)
        self.scaledHisSrc = []

        self.updating = False

    def createGUI(self):
        vbox = gtk.VBox()
        vbox.set_border_width(3)
        self.add(vbox)

        self.histoDA = gtk.DrawingArea()
        self.histoDA.set_size_request(256,100)
        self.histoDA.connect('expose-event', self.exposeHisto)
        vbox.pack_start(self.histoDA, expand=False)

        self.markersDA = gtk.DrawingArea()
        self.markersDA.set_size_request(256,8)
        self.markersDA.set_events(gtk.gdk.BUTTON_PRESS_MASK)
        self.markersDA.connect('expose-event', self.exposeMarkers)
        self.markersDA.connect('button-press-event', self.click)
        vbox.pack_start(self.markersDA, expand=False)

        vbox.add(gtk.HSeparator())
        box = gtk.HBox(spacing=5)
        vbox.add(box)

        box.pack_start(gtk.Label("Mean:"), expand=False)
        self.meanLB = gtk.Label()
        box.pack_start(self.meanLB, expand=False)
        box.pack_start(gtk.Label(" Std Dev:"), expand=False)
        self.stdLB = gtk.Label()
        box.pack_start(self.stdLB, expand=False)

    def cleanup(self, *args):
        # is that really necessary?
        self.hisSrc = None

    def refreshHisto(self, *args):
        self.drawHisto()
        self.buildPercents()

    def buildPercents(self):
        self.hiPct = []
        self.hisSrc.reverse()
        tot = 0
        start = 0
        lo = self.master.lo
        if self.master.hi == 254:
            self.hiPct.append(0)
            start = 1
        for bin in self.hisSrc[start:-1]:
            tot += bin
            self.hiPct.append(tot/self.master.pixCnt*100)
        self.hiPct.append(0)
        self.hiPct.reverse()

        self.hisSrc.reverse()
        tot = 0
        start = 0
        self.lowPct = []
        if self.master.lo == 1:
            self.lowPct.append(0)
            start = 1
        for bin in self.hisSrc[start:-1]:
            tot += bin
            self.lowPct.append(tot/self.master.pixCnt*100)
        self.lowPct.append(0)

    def drawHisto(self):
        area = self.histoDA.window
        if area is None:
            return

        gc = area.new_gc()
        cm = area.get_colormap()
        style = self.histoDA.get_style()
        w,h = self.histoDA.size_request()
        area.draw_rectangle(style.white_gc, True, 0, 0, w, h)
        zoom = zooms[self.master.zoomindex]
        if self.scaledHisSrc:
            if self.name == 'Red':
                gc.foreground = cm.alloc_color(65535,49150,49150)
            elif self.name == 'Green':
                gc.foreground = cm.alloc_color(49150,65535,49150)
            elif self.name == 'Blue':
                gc.foreground = cm.alloc_color(49150,49150,65535)
            else:
                gc.foreground = cm.alloc_color(49150,49150,49150)

            area.draw_lines(gc, self.getScaledBins(zoom))
            
        if self.name == 'Red':
            gc.foreground = cm.alloc_color(65535,0,0)
        elif self.name == 'Green':
            gc.foreground = cm.alloc_color(0,65535,0)
        elif self.name == 'Blue':
            gc.foreground = cm.alloc_color(0,0,65535)
        else:
            gc = style.black_gc

        area.draw_lines(gc, self.getBins(zoom))

    def exposeHisto(self, *args):
        self.drawHisto()

    def drawMarkers(self):
        area = self.markersDA.window
        if area is None:
            return

        min = int(self.min)
        max = int(self.max)
        style = self.markersDA.get_style()
        area.draw_rectangle(style.bg_gc[gtk.STATE_NORMAL], True, 0, 0, 256, 8)
        area.draw_line(style.black_gc, min, 0, min, 6)
        area.draw_line(style.black_gc, max, 0, max, 6)

    def exposeMarkers(self, *args):
        self.drawMarkers()

    def updateStats(self, mean, std):
        self.meanLB.set_text(str(mean))
        self.stdLB.set_text(str(std))

    def getHisto(self):
        return self.hisSrc

    def setHisto(self,histo):
        self.hisSrc = histo
        self.buildPercents()

    def setScaledHisto(self,histo):
        self.scaledHisSrc = histo
        self.drawHisto()

    def click(self, widget, event):
        value = event.x
        if event.button == 3:
            self.max = value
            pct = self.hiPct[int(value)]
            self.master.setMax(self.srcIdx, value, pct)
        else:
            self.min = value
            pct = self.lowPct[int(value)]
            self.master.setMin(self.srcIdx, value, pct)

        self.drawMarkers()

    def getMinMax(self, tail=None):
        if self.updating:
            return

        if tail == 'min':
            self.min = self.getMinBin()
        elif tail == 'max':
            self.max = self.getMaxBin()
        elif tail == 'both':
            self.min = self.getMinBin()
            self.max = self.getMaxBin()
        else:
            self.min = 0.0
            self.max = 255.0

        self.drawMarkers()
        return self.min,self.max

    def getMinBin(self):
        for pct in self.lowPct[1:]:
            if pct > self.master.mintail:
                return self.lowPct.index(pct)
        return 0

    def getMaxBin(self):
        for pct in self.hiPct[1:]:
            if pct < self.master.maxtail:
                return self.hiPct.index(pct)
        return 255

    def getBins(self,zoom):
        bins = []
        w,h = self.histoDA.size_request()
        lo = self.master.lo
        hi = self.master.hi
        maxVal = max(self.hisSrc[lo:hi+1]) * zoom
        if lo == 1:
            bins.append((0, 99))
        for x in range(lo, hi+1):
            y = min(int(h - h * self.hisSrc[x]/maxVal), h-1)
            y = max(0, y)
            bins.append((x, y))
        if hi == 254:
            bins.append((255, 99))
        return bins

    def getScaledBins(self,zoom):
        bins = []
        w,h = self.histoDA.size_request()
        maxVal = max(self.scaledHisSrc[1:]) * zoom
        bins.append((0, 99))
        for x in range(1, 256):
            y = min(int(h - h * self.scaledHisSrc[x]/maxVal), h-1)
            y = max(0, y)
            bins.append((x, y))
        return bins

class HistogramWindow(gtk.Window):
    def __init__(self, layer):
        gtk.Window.__init__(self)
        self.layer = layer
        ds = layer.get_dataset()
        self.gdt = ds.GetRasterBand(1).DataType
        self.teardown_id = self.layer.connect('teardown', self.close)

        self.zoomindex = 3
        self.mintail = 0.1
        self.maxtail = 1.0
        self.lo = 1
        self.hi = 254
        self.lut = None
        self.histoDisplays = []

        self.updating = True
        self.createGUI()
        self.show_all()
        self.updating = False
        self.autoScale()

    def createGUI(self):
        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(5)
        self.add(vbox)
        histoBox = gtk.VBox(spacing=3)
        vbox.add(histoBox)
        histoList = []

        hisdir,f = os.path.split(self.layer.name)
        name,ext = os.path.splitext(f)
        self.set_title(name+" Histogram")
        hisfile = os.path.join(hisdir, name+'.his')
        if os.path.exists(hisfile): # histofile is in same dir
            histoList = self.loadHistos(hisfile)
        elif hisDir is not None:
            hisfile = os.path.join(hisDir, name+'.his')
            if os.path.exists(hisfile): # histofile is in histos dir
                histoList = self.loadHistos(self.hisFile)
        self.hisFile = hisfile

        if self.layer.get_mode() == gview.RLM_RGBA:
            if not histoList:
                for isrc in range(3):
                    histoList.append(self.buildHisto(isrc))

            self.pixCnt = self.getPixCount(histoList)
            histoDisp = HistogramFrame("Red", 0, histoList[0], self)
            histoBox.pack_start(histoDisp, expand=False)
            self.histoDisplays.append(histoDisp)

            histoDisp = HistogramFrame("Green", 1, histoList[1], self)
            histoBox.pack_start(histoDisp, expand=False)
            self.histoDisplays.append(histoDisp)

            histoDisp = HistogramFrame("Blue", 2, histoList[2], self)
            histoBox.pack_start(histoDisp, expand=False)
            self.histoDisplays.append(histoDisp)
        else:
            if not histoList:
                histoList.append(self.buildHisto(0))
            self.pixCnt = self.getPixCount(histoList)
            histoDisp = HistogramFrame("Raster", 0, histoList[0], self)
            histoBox.pack_start(histoDisp, expand=False)
            self.histoDisplays.append(histoDisp)

        ctrlbox = gtk.HBox(homogeneous=1, spacing=5)
        vbox.pack_start(ctrlbox, expand=False)

        button = create_stock_button('refresh', self.refresh)
        ctrlbox.pack_start(button, expand=False)
        button.set_tooltip_text("rebuilds the histogram from layer")
        button = create_stock_button(gtk.STOCK_ZOOM_IN, self.zoom, 'in')
        ctrlbox.pack_start(button, expand=False)
        button.set_tooltip_text("zoom in")
        button = create_stock_button(gtk.STOCK_ZOOM_OUT, self.zoom, 'out')
        ctrlbox.pack_start(button, expand=False)
        button.set_tooltip_text("zoom out")
        button = create_stock_button(gtk.STOCK_OPEN, self.load)
        ctrlbox.pack_start(button, expand=False)
        button.set_tooltip_text("load histogram")
        button = create_stock_button(gtk.STOCK_SAVE, self.save)
        ctrlbox.pack_start(button, expand=False)
        button.set_tooltip_text("save histogram")

        frame = gtk.Frame("Autoscale")
        vbox.pack_start(frame, expand=False)
        scalebox = gtk.VBox(spacing=5)
        scalebox.set_border_width(5)
        frame.add(scalebox)

        box = gtk.HBox(spacing=5)
        scalebox.pack_start(box, expand=False)

        box.pack_start(gtk.Label("Remove: "), expand=False)
        blackCK = gtk.CheckButton("Black")
        blackCK.set_active(True)
        blackCK.connect('toggled', self.setLimits, 'lo')
        box.pack_start(blackCK, expand=False)
        whiteCK = gtk.CheckButton("White")
        whiteCK.set_active(True)
        whiteCK.connect('toggled', self.setLimits, 'hi')
        box.pack_start(whiteCK, expand=False)

        box = gtk.HBox(spacing=5)
        scalebox.pack_start(box, expand=False)

        box.pack_start(gtk.Label("Min tail:"), expand=False)
        minAdj = gtk.Adjustment(self.mintail, 0.0, 99.9, 0.01, 0.1)
        self.minSpin = gtk.SpinButton(minAdj, 0.1, 2)
        self.minSpin.set_size_request(55, -1)
        self.minSpin.connect('changed', self.adjustMin)
        box.pack_start(self.minSpin, expand=False)

        maxAdj = gtk.Adjustment(self.maxtail, 0.0, 99.9, 0.01, 0.1)
        self.maxSpin = gtk.SpinButton(maxAdj, 0.1, 2)
        self.maxSpin.set_size_request(55, -1)
        self.maxSpin.connect('changed', self.adjustMax)
        box.pack_end(self.maxSpin, expand=False)
        box.pack_end(gtk.Label("Max tail:"), expand=False)

        algCB = ComboText(strings=("None","linear","log","root","square"), action=self.setAlg)
        algCB.set_size_request(70,-1)
        scalebox.pack_start(algCB, expand=False)

    def close(self, *args):
        self.layer.disconnect(self.teardown_id)

        del self.histoDisplays
        self.layer = None
        self.destroy()

    def refresh(self, bt):
        histoList = []
        for histoDisp in self.histoDisplays:
            histo = self.buildHisto(histoDisp.srcIdx)
            histoDisp.setHisto(histo)
            histoList.append(histo)

        self.pixCnt = self.getPixCount(histoList)
        self.update()
        self.autoScale()

    def update(self):
        for histoDisp in self.histoDisplays:
            histoDisp.refreshHisto()

    def updateSpins(self, lo=None, hi=None):
        self.updating = True
        if lo is not None:
            self.minSpin.set_value(lo)
        if hi is not None:
            self.maxSpin.set_value(hi)
        self.updating = False

    def updateScaled(self, histoDisp):
        scaledHisto = self.buildScaledHisto(histoDisp)
        histoDisp.setScaledHisto(scaledHisto)
        mean,std = self.getStats(scaledHisto)
        histoDisp.updateStats(round(mean,1), round(std,1))
        
    def zoom(self, bt, id):
        if id == 'in':
            self.zoomindex = min(self.zoomindex+1,8)
        else:
            self.zoomindex = max(self.zoomindex-1,0)

        for histoDisp in self.histoDisplays:
            histoDisp.drawHisto()

    def load(self, bt):
        from pgufilesel import pguFileSelection
        hisdir,hisfile = os.path.split(self.hisFile)
##        dlg = pguFileSelection("Open Histogram", cwd=hisdir, filter='Histogram files|*.his', multiselect=0)
        dlg = pguFileSelection("Open Histogram", self.hisFile)
        dlg.hide_fileop_buttons()
        dlg.complete('*.his')
        ret = dlg.run()
        if ret == gtk.RESPONSE_OK:
            self.loadHistoFile(dlg.get_filename())
        dlg.destroy()

    def loadHistoFile(self, hisFile):
##        hisFile = dlg.get_filename()
        histoList = self.loadHistos(hisFile)
        
        self.pixCnt = self.getPixCount(histoList)
        for histoDisp in self.histoDisplays:
            histoDisp.setHisto(histoList[histoDisp.srcIdx])
            histoDisp.drawHisto()

        self.autoScale()
        self.hisFile = hisFile

    def loadHistos(self, filename):
        import pickle
        f = file(filename,'r')
        histoList = pickle.load(f)
        f.close()
        return histoList

    def save(self, *args):
        import filedlg
        hisdir,hisfile = os.path.split(self.hisFile)
        dlg = filedlg.FileDialog("Save Histogram as...", cwd=hisdir, dialog_type=filedlg.FILE_SAVE, filter='Histogram files|*.his')
        dlg.ok_button.connect('clicked', self.saveHistoFile, dlg)
        dlg.set_filename(hisfile)
        dlg.show()

    def saveHistoFile(self, bt, dlg):
        import pickle
        histoList = []
        for histoDisp in self.histoDisplays:
            histoList.append(histoDisp.getHisto())
        hisFile = dlg.get_filename()
        f = file(hisFile,'w')
        pickle.dump(histoList, f)
        f.close()
        self.hisFile = hisFile

    def setAlg(self, combo):
        alg = combo.get_active_text()
        self.lut = self.getLut(alg)
        if alg == 'None':
            self.autoScale()
            alg = 'none_lut'
        else:
            self.autoScale('both')

        self.layer.set_property('last_stretch', alg)

    def adjustMin(self, spin):
        if self.updating:
            return
        self.mintail = spin.get_value()
        self.autoScale('min')

    def adjustMax(self, spin):
        if self.updating:
            return
        self.maxtail = spin.get_value()
        self.autoScale('max')

    def setMin(self, isrc, value, pct):
        if self.gdt == 2:
            value /= 0.124573
        self.layer.min_set(isrc, value)
        self.updateSpins(lo=pct)
        self.mintail = pct
        self.updateScaled(self.histoDisplays[isrc])

    def setMax(self, isrc, value, pct):
        if self.gdt == 2:
            value /= 0.124573
        self.layer.max_set(isrc, value)
        self.updateSpins(hi=pct)
        self.maxtail = pct
        self.updateScaled(self.histoDisplays[isrc])

    def setLimits(self, ck, id):
        if id == 'hi':
            if ck.get_active():
                self.hi = 254
            else:
                self.hi = 255
        else:
            if ck.get_active():
                self.lo = 1
            else:
                self.lo = 0

        histoList = []
        for histoDisp in self.histoDisplays:
            histoList.append(histoDisp.getHisto())
        self.pixCnt = self.getPixCount(histoList)
        self.update()
        self.autoScale('both')
        self.updateSpins(self.mintail, self.maxtail)

    def buildHisto(self, isrc):
        band = self.layer.get_data(isrc).get_band()
        if self.gdt == 2:
            return band.GetHistogram(min=-0.5, max=2047.5, buckets=256)
        else:
            return band.GetHistogram()

    def buildScaledHisto(self, histoDisp):
        histo = histoDisp.getHisto()
        ratio = 255.0 / (histoDisp.max - histoDisp.min)
        offset = -(histoDisp.min * ratio)
        scaledHisto = [0] * 256 
        for i in range(256):
            value = int(i * ratio + offset)
            if self.lut is not None:
                if value < 0:
                    value = 0
                elif value > 255:
                    value = 255
                scaled = ord(self.lut[value])
            else:
                scaled = value
            if scaled < 0:
                scaledHisto[0] += histo[i]
            elif scaled > 255:
                scaledHisto[255] += histo[i]
            else:
                scaledHisto[scaled] += histo[i]

        return scaledHisto

    def autoScale(self, tail=None):
        for histoDisp in self.histoDisplays:
            min,max = histoDisp.getMinMax(tail)
            if self.gdt == 2:
                min /= 0.124573
                max /= 0.124573
            isrc = histoDisp.srcIdx

            self.layer.set_source(isrc, self.layer.get_data(isrc), min, max,
                                    self.layer.get_const_value(isrc), self.lut,
                                    self.layer.nodata_get(isrc))
            self.updateScaled(histoDisp)

    def getLut(self, alg):
        from math import log,pow
        def root(value):
            return int(255 * sqrt(i/255.0))
        def loga(value):
            return int((255 * (log(1.0+i) / log(256.0)))+0.5)
        def square(value):
            return int(255 * pow(i/255.0,2.0))

        if alg == 'root':
            fn = root
        elif alg == 'log':
            fn = loga
        elif alg == 'square':
            fn = square
        else:
            return None

        lut = ''
        for i in range(256):
            value = fn(i)
            if value < 0 :
                value = 0
            elif value >= 255:
                value = 255
            lut += chr(value)

        return lut

    def getPixCount(self, hisLst):
        from operator import add
        cnt = reduce(add, hisLst[0])
        if self.lo == 1:
            blkLst = []
            for histo in hisLst:
                blkLst.append(histo[0])
            cnt -= min(blkLst)
        elif self.hi == 254:
            whtLst = []
            for histo in hisLst:
                whtLst.append(histo[255])
            cnt -= min(whtLst)

        return float(cnt)

    def getStats(self, histo):
        m = array(histo[1:])
        sum = 0.0
        sum2 = 0.0
        n = float(add.reduce(m))
        for j in range(len(m)):
            sum = sum + j * m[j]
            sum2 = sum2 + (j ** 2) * float(m[j])
        var = (sum2-(sum**2.0)/n)/n
        return sum/n,sqrt(var)
