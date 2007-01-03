##############################################################################
# $Id: compose.py,v 1.1.1.1 2005/04/18 16:38:36 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Tool for combining/merging datasets without loading
#           them directly into memory.
#
# Author:   Andrey Kiselev, dron@remotesensing.org
#	    Iscander Latypov
#           Gillian Walter (modifications)
#
###############################################################################
# Copyright (c) 2004, American Museum of Natural History. All rights reserved.
# This software is based upon work supported by NASA under award
# number NAG5-12333
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
# To do: 1) Nicer editing windows for projection info?  WKT
#           strings can be a bit hard to follow.
#        2) Add Metadata frame and colour interpretation frame
#


import gtk
import gview
import string
import gvutils
import gviewapp
import Numeric
import os
import gdal
import osr
import gdalnumeric
import pgugrid
import vrtutils
import gvsignaler

spc=5

class ComposeTool(gviewapp.Tool_GViewApp):
    
    def __init__(self,app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.init_menu()

    def launch_dialog(self,*args):
        self.win = DatasetComposeDialog(self.app)
        self.win.show()

    def init_menu(self):
        self.menu_entries.set_entry("Image/Compose...",1,
                                    self.launch_dialog)


class DatasetComposeDialog(gtk.Window):
    def __init__(self,app=None):
        gtk.Window.__init__(self)
        self.set_title('Compose Dataset')
        self.set_policy(gtk.FALSE, gtk.TRUE, gtk.TRUE)
        self.set_border_width(10)
        self.shell=gtk.VBox(spacing=spc)
        self.add(self.shell)
        self.tips=gtk.Tooltips()
        self.input_frame=InputFrame(self, self.tips)
        self.show_list=[]
        self.adv_show_list=[]
        self.show_list.append(self.input_frame)
        self.app=app

        self.button_dict={}
        self.button_dict['Mode']=gtk.CheckButton('Advanced Options')
        self.shell.pack_start(self.button_dict['Mode'])
        self.show_list.append(self.button_dict['Mode'])

 
        self.adv_notebook = gtk.Notebook()
        self.shell.pack_start(self.adv_notebook)
        self.adv_show_list.append(self.adv_notebook)
        self.geo_frame=GeocodingFrame(self.tips)
        self.adv_notebook.append_page(self.geo_frame,
                                      gtk.Label('Geocoding')) 

        echbox=gtk.HBox(spacing=5,homogeneous=gtk.FALSE)
        echbox.set_border_width(3)
        self.shell.pack_end(echbox,gtk.FALSE,gtk.FALSE,0)
        self.show_list.append(echbox)
                              
        self.button_dict['Close']=gtk.Button('Close')
        echbox.pack_end(self.button_dict['Close'],expand=gtk.TRUE)
        self.button_dict['Save']=gtk.Button('Save VRT')
        echbox.pack_end(self.button_dict['Save'],expand=gtk.TRUE)
        self.button_dict['New']=gtk.Button('New View')
        echbox.pack_end(self.button_dict['New'],expand=gtk.TRUE)
        self.button_dict['Current']=gtk.Button('Current View')
        echbox.pack_end(self.button_dict['Current'],expand=gtk.TRUE)

        self.tips.set_tip(self.button_dict['Close'],
                          'Exit the Compose Dataset tool')
        self.tips.set_tip(self.button_dict['Save'],
                          'Create dataset and save to a VRT format file')
        self.tips.set_tip(self.button_dict['New'],
                          'Create dataset and display in a new view')
        self.tips.set_tip(self.button_dict['Current'],
                          'Create dataset and display in current view')

        for item in self.show_list:
            item.show_all()
            item.show()

        # geocode frame hides some of its contents
        self.geo_frame.show()
        
        self.button_dict['Save'].connect('clicked',self.create_cb,'Save')
        self.button_dict['New'].connect('clicked',self.create_cb,'New')
        self.button_dict['Current'].connect('clicked',self.create_cb,'Current')
        self.button_dict['Close'].connect('clicked',self.close)        
        self.button_dict['Mode'].connect('toggled',self.mode_toggled_cb)

        self.input_frame.subscribe('output-bands-empty', self.clear_defaults)
        self.input_frame.subscribe('output-bands-notempty',
                                   self.update_defaults)
                       
        self.button_dict['Mode'].set_active(0)
        self.mode_toggled_cb()
        self.shell.show()

    def mode_toggled_cb(self,*args):
        if self.button_dict['Mode'].get_active():
            for item in self.adv_show_list:
                item.show()
        else:
            for item in self.adv_show_list:
                item.hide()
                
    def close(self,*args):
        self.destroy()

    def create_cb(self,*args):
        bands = self.input_frame.get_output_bands()
        if len(bands) == 0:
            gvutils.error('No output bands specified!')
            return

        vrtbase = [gdal.CXT_Element,'VRTDataset']
        vrtbase.append([gdal.CXT_Attribute,'rasterXSize',
                     [gdal.CXT_Text,str(bands[0][0].RasterXSize)]])
        vrtbase.append([gdal.CXT_Attribute,'rasterYSize',
                     [gdal.CXT_Text,str(bands[0][0].RasterYSize)]])

        # Metadata is currently taken from first output band.
        # This may be updatable later.
        mbase = vrtutils.serializeMetadata(bands[0][0])
        if mbase is not None:
            vrtbase.append(mbase)
            
        gbase = self.geo_frame.get_geocoding()
        for item in gbase:
            vrtbase.append(item)

        outband = 1
        for item in bands:
            dict={}
            dict['band']=outband
            dict['SourceBand'] = item[1]
            dict['ColorInterp'] = 'Undefined'
            bbase = vrtutils.serializeBand(item[0],opt_dict=dict)
            vrtbase.append(bbase)
            outband=outband+1

        vrtlines = gdal.SerializeXMLTree(vrtbase)
    
        vrtds = gdal.OpenShared(vrtlines)
        
        if args[1] == 'Save':
            chooser = gtk.FileChooserDialog(title="Save File", parent=self, 
                    action=gtk.FILE_CHOOSER_ACTION_SAVE, buttons=None, backend=None)
            fname = chooser.get_filename()
            print fname
            if fname is None:
                return
            driver = gdal.GetDriverByName('VRT')
            driver.CreateCopy(fname,vrtds)
        elif args[1] == 'New':
            self.app.new_view()
            self.app.open_gdal_dataset(vrtds) 
        else:
            self.app.open_gdal_dataset(vrtds)

    def update_defaults(self, *args):
        bands = self.input_frame.get_output_bands()
        self.geo_frame.update_default_frame(bands[0][0].GetDescription())
        self.geo_frame.update_gcp_frame_to_defaults()
        self.geo_frame.update_geotransform_frame_to_defaults()

    def clear_defaults(self, *args):
        self.geo_frame.update_default_frame(None)
        self.geo_frame.clear_gcp_frame()
        self.geo_frame.clear_geotransform_frame()
    
class GeocodingFrame(gtk.VBox):
    def __init__(self,tips):
        gtk.VBox.__init__(self)

        self.frames = {}
        self.tips=tips
        
        hbox=gtk.HBox()
        hbox.set_border_width(spc)
        self.pack_start(hbox)
        label=gtk.Label("Input Geocoding: ")
        label.set_alignment(0,0.5)
        hbox.pack_start(label)
        self.geocode_menu_list=['Default','GCPs','Geotransform']
        self.geocode_menu = gvutils.GvOptionMenu(self.geocode_menu_list,
                                                 self.geotype_toggled_cb)
        self.tips.set_tip(self.geocode_menu,
                          'Default: Derive geocoding from input bands\n'+
                          'GCPs: Define new ground control points\n'+
                          'Geotransform: Define a new affine transformation')
        hbox.pack_start(self.geocode_menu)
        hbox.show_all()
        self.create_default_frame()
        self.create_gcp_frame()
        self.create_geotransform_frame()
        self.geocode_menu.set_history(0)
        self.geotype_toggled_cb()
        
        self.default_fname=None # dataset to use for default info
        self.default_geotransform=None
        self.default_gcps=None
        self.default_prj=''

    def geotype_toggled_cb(self,*args):
        hist=self.geocode_menu.get_history()
        for idx in range(len(self.geocode_menu_list)):
            if idx == hist:
                self.frames[self.geocode_menu_list[idx]].show()
            else:
                self.frames[self.geocode_menu_list[idx]].hide()
                

    def create_default_frame(self):
        self.frames['Default']=gtk.Frame('')
        self.frames['Default'].set_shadow_type(gtk.SHADOW_NONE)
        vbox=gtk.VBox()
        vbox.set_spacing(spc)
        self.frames['Default'].add(vbox)
        self.default_scrolled_text = gtk.TextView(gtk.TextBuffer())
        self.default_scrolled_text.set_wrap_mode(gtk.WRAP_NONE)
        self.default_scrolled_text.set_editable(gtk.FALSE)
        self.default_scrolled_win = gtk.ScrolledWindow()
        self.default_scrolled_win.set_size_request(200,200)
        self.default_scrolled_win.add( self.default_scrolled_text)
        vbox.pack_start(self.default_scrolled_win,expand=gtk.TRUE)
        self.frames['Default'].show_all()
        
        self.pack_start(self.frames['Default'])

        
    def create_gcp_frame(self):
        self.frames['GCPs']=gtk.Frame('')
        self.frames['GCPs'].set_shadow_type(gtk.SHADOW_NONE)
        vbox=gtk.VBox()
        vbox.set_spacing(spc)
        self.frames['GCPs'].add(vbox)
        self.gcpgrid = pgugrid.pguGrid((3,1,0,1,7,0,0,0,3))
        self.gcpgrid.set_size_request(200,200)
        self.gcplist=[]
        self.gcpgrid.set_source(self.gcplist,
           members=['Id','GCPPixel','GCPLine','GCPX','GCPY','GCPZ'],
           titles = ['ID','Pixel','Line','X','Y','Z'],
           types = ['string','float','float','float','float','float'])
        vbox.pack_start(self.gcpgrid)
        hbox=gtk.HBox()
        hbox.set_spacing(spc)
        vbox.pack_start(hbox)
        self.add_gcp_button=gtk.Button('  Add GCP  ')
        self.add_gcp_button.connect('clicked',self.add_gcp_cb)
        hbox.pack_start(self.add_gcp_button, expand=gtk.FALSE)
        self.tips.set_tip(self.add_gcp_button,'Add a new GCP')
        self.load_gcp_button=gtk.Button('Load GCPs')
        self.tips.set_tip(self.load_gcp_button,
           'Clear existing GCPs and load '+
           'new ones from a text file')
        self.load_gcp_button.connect('clicked',self.load_gcps_cb)
        hbox.pack_start(self.load_gcp_button, expand=gtk.FALSE)
        self.copy_gcp_button=gtk.Button('Copy GCPs')
        self.copy_gcp_button.connect('clicked',self.copy_gcps_cb)
        self.tips.set_tip(self.copy_gcp_button,
            'Clear existing GCPs and copy new '+
            'ones from another GDAL dataset')
        hbox.pack_start(self.copy_gcp_button, expand=gtk.FALSE)
        self.clear_gcp_button=gtk.Button('Clear GCPs')
        self.clear_gcp_button.connect('clicked',self.clear_gcps)
        self.tips.set_tip(self.clear_gcp_button,
            'Clear all GCPs')
        hbox.pack_start(self.clear_gcp_button, expand=gtk.FALSE)
        self.gcpprjbox=ProjectionBox(self.tips)
        vbox.pack_start(self.gcpprjbox)
        self.frames['GCPs'].show_all()
        self.pack_start(self.frames['GCPs'])

    def load_gcps_cb(self,*args):
        """ Load gcps from a text file """
        self.clear_gcps()
        info=getgcpfile()
        if info is None:
            # user pressed cancel
            return
        if ((info[2] is None) or (info[3] is None) or
            (info[4] is None) or (info[5] is None)):
            gvutils.error('Invalid column info for GCP text file!')
            return

        try:
            fh=open(info[0],'r')
            flines=fh.readlines()
        except:
            gvutils.error('Unable to read GCP text file!')
            return

        idx=0
        for cline in flines:
            if string.strip(info[1]) == '':
                # whitespace delimited
                sline=string.split(cline)
            else:
                sline=string.split(cline,info[1])
            try:
                gcp=gdal.GCP()
                gcp.GCPPixel=float(string.strip(sline[info[2]-1]))
                gcp.GCPLine=float(string.strip(sline[info[3]-1]))
                gcp.GCPX=float(string.strip(sline[info[4]-1]))
                gcp.GCPY=float(string.strip(sline[info[5]-1]))
                if info[6] is not None:
                    gcp.GCPZ=float(string.strip(sline[info[6]-1]))
                if info[7] is not None:
                    gcp.Id=string.strip(sline[info[7]-1])
                if info[8] is not None:
                    gcp.Info=string.strip(sline[info[8]-1])
                self.gcplist.append(gcp)
            except:
                # first line might have column names, so
                # ignore errors.  otherwise, report invalid
                # lines
                if idx != 0:
                    print 'Warning: invalid line '+str(idx)+' in GCP file!'
                    
            idx=idx+1

        self.gcpgrid.refresh()

    def clear_gcp_frame(self):
        self.clear_gcps()
        self.gcpprjbox.set_input_projection('')
        self.gcpprjbox.set_output_projection('')

    def update_gcp_frame_to_defaults(self):
        self.clear_gcp_frame()
        if self.default_gcps is not None:
            for item in self.default_gcps:
                gcp=CopyGDALGCP(item) # copy so original doesn't get changed
                self.gcplist.append(gcp)
                
            self.gcpgrid.refresh()
                
            self.gcpprjbox.set_input_projection(self.default_prj)
            self.gcpprjbox.set_output_projection(self.default_prj)
               
    def clear_gcps(self,*args):
        while len(self.gcplist) > 0:
            self.gcplist.pop()
        self.gcpgrid.refresh()

    def copy_gcps_cb(self,*args):
        """ Copy gcps from an existing gdal dataset. """
        fname=GtkExtra.file_sel_box(title="Select GDAL Dataset")
        if fname is None:
            return

        try:
            fh=gdal.OpenShared(fname)
        except:
            gvutils.error('Unable to open '+fname+' as a GDAL dataset!')
            return
        
        gcps=fh.GetGCPs()
        prj=fh.GetGCPProjection()
        self.clear_gcps()
        for gcp in gcps:
            ngcp=CopyGDALGCP(gcp)
            self.gcplist.append(ngcp)
        self.gcpgrid.refresh()
            
        self.gcpprjbox.set_input_projection(prj)

    def add_gcp_cb(self,*args):
        self.gcplist.append(gdal.GCP())
        self.gcpgrid.refresh()

    def create_geotransform_frame(self):
        self.frames['Geotransform']=gtk.Frame('')
        self.frames['Geotransform'].set_shadow_type(gtk.SHADOW_NONE)
        vbox=gtk.VBox()
        vbox.set_spacing(spc)
        self.frames['Geotransform'].add(vbox)
        label=gtk.Label('Xgeo = GT(0) + XPixel*GT(1) + YLine*GT(2)')
        label.set_alignment(0,0.5)
        vbox.pack_start(label)
        label=gtk.Label('Ygeo = GT(3) + XPixel*GT(4) + YLine*GT(5)')
        label.set_alignment(0,0.5)
        vbox.pack_start(label)
        table=gtk.Table(rows=6,columns=3)
        self.geotransform_entries=[]
        for idx in range(6):
            label=gtk.Label('GT('+str(idx)+'):')
            label.set_alignment(0,0.5)
            table.attach(label,0,1,idx,idx+1)
            newentry=gtk.Entry()
            self.geotransform_entries.append(newentry)
            table.attach(newentry,1,2,idx,idx+1)
        vbox.pack_start(table)
        self.geotransformprjbox=ProjectionBox(self.tips)
        vbox.pack_start(self.geotransformprjbox)
        self.frames['Geotransform'].show_all()
        self.pack_start(self.frames['Geotransform'])

    def clear_geotransform_frame(self):
        for item in self.geotransform_entries:
            item.set_text('')
            
        self.geotransformprjbox.set_input_projection('')
        self.geotransformprjbox.set_output_projection('')

    def update_geotransform_frame_to_defaults(self):
        self.clear_geotransform_frame()
        if self.default_geotransform is not None:
            for idx in range(6):
                self.geotransform_entries[idx].set_text(
                    "%f" % self.default_geotransform[idx])
            self.geotransformprjbox.set_input_projection(self.default_prj)
            self.geotransformprjbox.set_output_projection(self.default_prj)
        
    def update_default_frame(self,fname):
        self.default_scrolled_text.get_buffer().delete(0,-1)
        if fname is None:
            self.default_fname=None
            self.default_geotransform=None
            self.default_gcps=None
            self.default_prj=''
            return
            
        sr=osr.SpatialReference()
        
        fh=gdal.OpenShared(fname)
        prj=''
        geot=fh.GetGeoTransform()
        if ((tuple(geot) == (0,1,0,0,0,1)) or
            (tuple(geot) == (0.0,1.0,0.0,0.0,0.0,1.0))):
            self.default_geotransform=None
            gcps = fh.GetGCPs()
            if len(gcps) > 0:
                self.default_gcps=gcps
                txt='Type of Geocoding: GCPs ('+str(len(gcps))+')\n\n'
                prj=fh.GetGCPProjection()
                if sr.ImportFromWkt(prj) == 0:
                    prjtxt=sr.ExportToPrettyWkt(simplify=1)
                else:
                    prjtxt=''
                txt=txt+'Projection: '+prjtxt
            else:
                self.default_gcps=None
                txt='Type of Geocoding: None\n\nProjection: None'
        else:
            self.default_geotransform=geot
            self.default_gcps=None
            txt='Type of Geocoding: Geotransform\n\n'
            prj=fh.GetProjection()
            if sr.ImportFromWkt(prj) == 0:
                prjtxt=sr.ExportToPrettyWkt(simplify=1)
            else:
                prjtxt=''
            txt=txt+'Projection: '+prjtxt
                    
        self.default_scrolled_text.set_text(txt)
        self.default_prj=prj
        self.default_fname=fname
            
    def clear_defaults(self,*args):
        self.update_default_frame(None)

    def get_geocoding(self):
        # returns serialized geocoding information as a list
        gt=self.geocode_menu_list[self.geocode_menu.get_history()]
        serialtxt=[]
        if gt == 'Default':
            if self.default_geotransform is not None:
                if self.default_prj != '':
                    serialtxt.append([gdal.CXT_Element,'SRS',[gdal.CXT_Text,
                                                       self.default_prj]])

                gtxt = vrtutils.serializeGeoTransform(
                               geotransform=self.default_geotransform)
                if gtxt is not None:
                    serialtxt.append(gtxt)
                    
            elif self.default_gcps is not None:
                serialtxt.append(vrtutils.serializeGCPs(
                                 gcplist=self.default_gcps,
                              with_Z=1,projection_attr_txt=self.default_prj))
            else:
                return []
                    
        elif gt == 'GCPs':
            if len(self.gcplist) == 0:
                return []
            
            inprj,outprj=self.gcpprjbox.get_projections()
            if (outprj == '') or (outprj == inprj) or (inprj == ''):
                reproj=None
                if (outprj != '') and (inprj == ''):
                    gvutils.warning('Warning: output projection specified,\n'+
                                    'but no input projection.  Cannot\n'+
                                    'reproject!')
            else:
                reproj=outprj
            gcplistcopy=CopyGDALGCPs(self.gcplist)    
            serialtxt.append(vrtutils.serializeGCPs(
                                             gcplist=gcplistcopy,with_Z=1,
                                             projection_attr_txt=inprj,
                                             reproj=reproj))
        else:
            inprj,outprj=self.geotransformprjbox.get_projections()
            gt=[]
            try:
                for idx in range(6):
                    gt.append(float(
                        self.geotransform_entries[idx].get_text()))
            except:
                gvutils.warning('Invalid Geotransform information- '+
                                'ignoring!')
                return []    

            if (outprj == '') or (outprj == inprj) or (inprj == ''):
                reproj=None
                if (outprj != '') and (inprj == ''):
                    gvutils.warning('Warning: output projection specified,\n'+
                                    'but no input projection.  Cannot\n'+
                                    'reproject!')

                if inprj != '':
                    serialtxt.append([gdal.CXT_Element,'SRS',
                                      [gdal.CXT_Text,inprj]])

                gbase=vrtutils.serializeGeoTransform(geotransform=gt)
                if gbase is not None:
                    serialtxt.append(gbase)
                
            else:
                ds=gdal.OpenShared(self.default_fname)
                gcps=vrtutils.GeoTransformToGCPs(gt,ds.RasterXSize,
                                                 ds.RasterYSize,grid=2)
                serialtxt.append(vrtutils.serializeGCPs(gcplist=gcps,with_Z=1,
                                                 projection_attr_txt=inprj,
                                                 reproj=outprj))
                
        return serialtxt
            

def getprjinfo():
    fname=GtkExtra.file_sel_box(title="Select GDAL Dataset or WKT text file")
    if fname is None:
        return None
    try:
        fh=gdal.OpenShared(fname)
        prj=fh.GetProjection()
        if prj == '':
            prj=fh.GetGCPProjection()
    except:
        fh=open(fname,'r')
        prj=string.strip(fh.readline())
        # prj files from shapes seem to have
        # an extra character at the end
        if prj[-1:] == '\x00':
            prj=prj[:-1]

    sr=osr.SpatialReference()
    val=sr.ImportFromWkt(prj)
    if val != 0:
        gvutils.error('Invalid projection information in '+fname+'!')
        return None

    return prj

def editprjinfo(wktinit=''):
    win=editprjwin("Projection (Well Known Text string)",wktinit)
    win.show()
    gtk.mainloop()
    prj=win.ret
    if prj is None:
        return

    if len(prj) == 0:
        return ''
    
    sr=osr.SpatialReference()
    val=sr.ImportFromWkt(prj)
    if val != 0:
        gvutils.error('Invalid projection information entered!')
        return ''
    
    return prj

class editprjwin(gtk.Window):
    def __init__(self,title,wkt):
        gtk.Window.__init__(self)
        self.set_title(title)
        self.set_size_request(350,300)
        vbox=gtk.VBox()
        self.add(vbox)
        self.text=gtk.TextView(gtk.TextBuffer())
        self.text.set_editable(gtk.TRUE)
        self.text.get_buffer().set_text(wkt)
        vbox.pack_start(self.text)
        hbox=gtk.HBox()
        vbox.pack_start(hbox,expand=gtk.FALSE)
        self.cancel_button=gtk.Button('  Cancel  ')
        self.ok_button=gtk.Button('     OK     ')
        hbox.pack_end(self.cancel_button,expand=gtk.FALSE)
        hbox.pack_end(self.ok_button,expand=gtk.FALSE)
        
        self.cancel_button.connect('clicked', self.quit)
        self.ok_button.connect('clicked', self.ok_cb)
        self.ret=None
        self.show_all()
        
    def quit(self, *args):
        self.hide()
        self.destroy()
        gtk.mainquit()
        
    def ok_cb(self, b):
        buf = self.text.get_buffer()
        self.ret = buf.get_text(*buf.get_bounds())
        self.quit()
        

def getgcpfile():
    win=GCPFileDialog()
    win.show()
    gtk.mainloop()
    return win.ret

class GCPFileDialog(gtk.FileSelection):
    def __init__(self):
        gtk.FileSelection.__init__(self)
        self.set_title('Select GCP Text File')
        self.connect("destroy", self.quit)
        self.connect("delete_event", self.quit)
        gtk.grab_add(self) # is modal
        table=gtk.Table(rows=8,columns=3)
        self.entries=[]
        labels=['Delimiter: ',
                'Pixel Column # (1-N: required): ',
                'Line Column # (required): ',
                'X (East/West) Column # (required): ',
                'Y (North/South) Column # (required): ',
                'Z (Height) Column # (optional): ',
                'ID Column # (optional): ',
                'Info Column # (optional): ']
        for idx in range(len(labels)):
            nl=gtk.Label(labels[idx])
            nl.set_alignment(0,0.5)
            self.entries.append(gtk.Entry(maxlen=3))
            table.attach(nl,0,1,idx,idx+1)
            table.attach(self.entries[idx],1,2,idx,idx+1)
        self.main_vbox.pack_end(table)
        table.show_all()           
      	self.cancel_button.connect('clicked', self.quit)
        self.ok_button.connect('clicked', self.ok_cb)
        self.ret = None
        
    def quit(self, *args):
        self.hide()
      	self.destroy()
       	gtk.mainquit()

    def ok_cb(self, b):
        ret=[self.get_filename(),self.entries[0].get_text()]
        for idx in range(1,len(self.entries)):
            try:
                val=int(self.entries[idx].get_text())
            except:
                val=None
            ret.append(val)
        self.ret = tuple(ret)
        self.quit()

class ProjectionBox(gtk.Table):
    def __init__(self,tips):
        gtk.Table.__init__(self,rows=2,columns=7,homogeneous=gtk.FALSE)
        self.set_row_spacings(spc)
        self.set_col_spacings(spc)
        self.set_col_spacing(2,3*spc)
        label=gtk.Label('Input Projection: ')
        label.set_alignment(0,0.5)
        self.attach(label,0,1,0,1)
        self.buttons={}
        self.buttons['input-edit']=gtk.Button('      Edit      ')
        self.buttons['input-load']=gtk.Button('Load From File')
        self.buttons['input-view']=gtk.Button('Load From View')
        tips.set_tip(self.buttons['input-edit'],
                     'Edit projection information (should '+
                     'be a Well Known Text string)')
        tips.set_tip(self.buttons['input-load'],
                     'Load projection information from a GDAL '+
                     'dataset, or a file with a single line '+
                     'of Well Known Text specifying a projection.')
        tips.set_tip(self.buttons['input-view'],
                     'Load projection information from the '+
                     'currently active view.')
        self.attach(self.buttons['input-edit'],0,1,1,2)
        self.attach(self.buttons['input-view'],1,2,1,2)
        self.attach(self.buttons['input-load'],2,3,1,2)
        self.inprj=''

        label=gtk.Label('Output Projection: ')
        self.attach(label,3,4,0,1)
        self.out_toggle_useinput=gtk.RadioButton(label='Use Input')
        tips.set_tip(self.out_toggle_useinput,
                     'Geocoding information entered above '+
                     'will not be reprojected.')
        self.out_toggle_reproj=gtk.RadioButton(label='Reproject',
                          group=self.out_toggle_useinput)
        tips.set_tip(self.out_toggle_reproj,
                     'Attempt to reproject geocoding information '+
                     'entered above to the Output projection.  No '+
                     'resampling of the binary data will take place.  '+
                     'If the input '+
                     'geocoding is a Geotransform, it will be '+
                     'converted to 16 GCPs prior to reprojection. '+
                     'Note that applications that do not use GDAL '+
                     'may not recognize this information, as it '+
                     'is less common for applications to recognize '+
                     'GCPs than Geotransforms.')
        self.out_toggle_useinput.connect('toggled',self.toggle_output_cb)
        self.attach(self.out_toggle_useinput,4,5,0,1)
        self.attach(self.out_toggle_reproj,5,6,0,1)
        self.buttons['output-edit']=gtk.Button('Edit')
        self.buttons['output-view']=gtk.Button('Load From View')
        self.buttons['output-load']=gtk.Button('Load From File')
        tips.set_tip(self.buttons['output-edit'],
                     'Edit projection information (should '+
                     'be a Well Known Text string)')
        tips.set_tip(self.buttons['output-load'],
                     'Load projection information from a GDAL '+
                     'dataset, or a file with a single line '+
                     'of Well Known Text specifying a projection.')
        tips.set_tip(self.buttons['output-view'],
                     'Load projection information from the '+
                     'currently active view.')
        self.attach(self.buttons['output-edit'],3,4,1,2)
        self.attach(self.buttons['output-view'],4,5,1,2)
        self.attach(self.buttons['output-load'],5,6,1,2)        
        self.outprj=''
        
        self.buttons['input-load'].connect('clicked',self.load_input_cb)
        self.buttons['input-view'].connect('clicked',self.view_input_cb)
        self.buttons['output-load'].connect('clicked',self.load_output_cb)
        self.buttons['output-view'].connect('clicked',self.view_output_cb)
        self.buttons['input-edit'].connect('clicked',self.edit_input_cb)
        self.buttons['output-edit'].connect('clicked',self.edit_output_cb)

        self.toggle_output_cb()
        
    def load_input_cb(self,*args):
        prj=getprjinfo()
        if prj is not None:
            self.inprj=prj

    def load_output_cb(self,*args):
        prj=getprjinfo()
        if prj is not None:
            self.outprj=prj

    def view_input_cb(self,*args):
        prj=gview.app.sel_manager.get_active_view().get_projection()
        if prj is None:
            gvutils.error('Current view does not contain projection info!')
            self.inprj=''
            return

        if len(prj) == 0:
            gvutils.error('Current view does not contain projection info!')
            self.inprj=''
            return
        
        sr=osr.SpatialReference()
        val=sr.ImportFromWkt(prj)
        if val == 0:
            self.inprj=prj
        else:
            gvutils.error('Current view contains invalid projection info!')
            self.inprj=''
            
    def view_output_cb(self,*args):
        prj=gview.app.sel_manager.get_active_view().get_projection()
        if prj is None:
            gvutils.error('Current view does not contain projection info!')
            self.outprj=''
            return

        if len(prj) == 0:
            gvutils.error('Current view does not contain projection info!')
            self.outprj=''
            return
        
        sr=osr.SpatialReference()
        val=sr.ImportFromWkt(prj)
        if val == 0:
            self.outprj=prj
        else:
            gvutils.error('Current view contains invalid projection info!')
            self.outprj=''
            
    def edit_input_cb(self,*args):
        prj=editprjinfo(self.inprj)
        if prj is not None:
            self.inprj=prj

    def edit_output_cb(self,*args):
        prj=editprjinfo(self.outprj)
        if prj is not None:
            self.outprj=prj

    def set_input_projection(self,prj):
        self.inprj=prj

    def set_output_projection(self,prj):
        self.outprj=prj
        
    def toggle_output_cb(self,*args):
        if self.out_toggle_useinput.get_active() == 1:
            self.buttons['output-load'].set_sensitive(0)
            self.buttons['output-view'].set_sensitive(0)
            self.buttons['output-edit'].set_sensitive(0)
        else:
            self.buttons['output-load'].set_sensitive(1)
            self.buttons['output-view'].set_sensitive(1)
            self.buttons['output-edit'].set_sensitive(1)

    def get_projections(self,*args):
        if self.out_toggle_useinput.get_active() == 1:
            return (self.inprj, self.inprj)
        else:
            return (self.inprj, self.outprj)
        
class InputFrame(gvsignaler.Signaler):
    def __init__(self,parent,tips):
        self.frame=gtk.Frame('Raster Bands')
        self.tips=tips
        self.input_bands=get_list_of_bands_as_dict()
        self.output_bands={}
        
        hbox1=gtk.HBox(spacing=spc)
        hbox1.set_border_width(spc)
        self.frame.add(hbox1)

        # source (input)
        srcvbox=gtk.VBox(spacing=spc)
        label=gtk.Label('Input:')
        label.set_alignment(0,0.5)
        srcvbox.pack_start(label,expand=gtk.FALSE)
        hbox1.pack_start(srcvbox)
	source_win = gtk.ScrolledWindow()
	source_win.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
	source_win.set_size_request(300,200)
	srcvbox.pack_start(source_win)

	source_list = gtk.List()
	source_list.set_selection_mode(gtk.SELECTION_MULTIPLE)
	source_win.add_with_viewport(source_list)
	source_list.append_items(self.input_bands.keys())

        # signals sent up to top level so that defaults can
        # be updated.  Defaults are updated whenever the output
        # bands are cleared, or the first one is specified.
        self.publish('output-bands-empty')
        self.publish('output-bands-notempty')

        def src_load(_button,*args):

            fname=GtkExtra.file_sel_box(title="Select GDAL Dataset")
            if fname is None:
                return
            ds=gdal.OpenShared(fname)
            if ds is None:
                gvutils.error('Not a valid gdal dataset!')
                return

            dict={}
            for i in range(1,ds.RasterCount+1):
                curband=fname + '.band[' + str(i) + ']'
                dict[gtk.ListItem(curband)] = (ds,i,curband)
                
            if srctoggle.get_active() == gtk.TRUE:
                slist=vrtutils.GetSimilarFiles(fname)
                for nname in slist:
                    ds = gdal.OpenShared(nname)
                    if ds is None:
                        continue
                    for i in range(1,ds.RasterCount+1):
                        curband=nname + '.band[' + str(i) + ']'
                        dict[gtk.ListItem(curband)] = (ds,i,curband)

            self.add_input_bands(dict)    

        def src_get_active_layers(_button,*args):
            size=None
            if len(self.input_bands) > 0:
                ckey=self.input_bands.keys()[0]
                size=(self.input_bands[ckey][0].RasterXSize,
                      self.input_bands[ckey][0].RasterYSize)
            new_dict=get_list_of_bands_as_dict(size)
            self.add_input_bands(new_dict)

        def src_clear(_button,*args):
            self.clear_input_bands()

        self.source_list=source_list

        # source control buttons
        srcbbox=gtk.HBox(spacing=spc)
        srcvbox.pack_start(srcbbox,expand=gtk.FALSE)
        load_btn=gtk.Button("Load File")
        self.tips.set_tip(load_btn,'Add bands from a file to the input list')
        srcbbox.pack_start(load_btn)
        load_btn.connect("clicked",src_load)
        act_btn=gtk.Button("Views->List")
        self.tips.set_tip(act_btn,'Add bands from views to the input list')
        srcbbox.pack_start(act_btn)
        act_btn.connect("clicked",src_get_active_layers)
        clear_btn=gtk.Button("Clear")
        srcbbox.pack_start(clear_btn)
        clear_btn.connect("clicked",src_clear)

        srctoggle=gtk.CheckButton("Include Similar")
        self.tips.set_tip(srctoggle,'Include bands from same-size files '+
                          'in the same directory when using Load File.')
        srcbbox.pack_start(srctoggle,expand=gtk.FALSE)
        srctoggle.set_active(gtk.TRUE)
        
        # destination
	btn_box = gtk.VBox(spacing=10)
	btn_box.set_border_width(10)
	hbox1.pack_start(btn_box,expand=gtk.FALSE)
	btn_box.show()

	def dest_add(_button,*args):
            sel = source_list.get_selection()
            sel.reverse()  # add in order of selection
            if len(self.output_bands.keys()) == 0:
                refreshflag = 1
            else:
                refreshflag = 0
                
	    for i in sel:
		list_item = gtk.ListItem(self.input_bands[i][2])
		self.dest_list.append_items([list_item])
		list_item.show()
		self.dest_list.select_child(self.dest_list.children()[-1])
		self.output_bands[list_item] = self.input_bands[i]

            if (refreshflag == 1) and (len(sel) > 0):
                self.notify('output-bands-notempty')
                
	def dest_del(_button,*args):
	    selection = self.dest_list.get_selection()
	    self.dest_list.remove_items(selection)
	    for i in selection:
		del self.output_bands[i]
		i.destroy()
	    rest = self.dest_list.children()
	    if len(rest) > 0:
		self.dest_list.select_child(self.dest_list.children()[-1])
            else:
                self.notify('output-bands-empty')
                
        def dest_raise(_button, *args):
            selection = self.dest_list.get_selection()
            if len(selection) != 1:
                return
            pos=self.dest_list.child_position(selection[0])
            if pos < 1:
                return
            self.dest_list.remove_items(selection)
            self.dest_list.insert_items(selection,pos-1)
            self.dest_list.select_item(pos-1)

        def dest_lower(_button, *args):
            selection = self.dest_list.get_selection()
            if len(selection) != 1:
                return
            pos=self.dest_list.child_position(selection[0])
            if pos > len(self.output_bands)-2:
                return
            self.dest_list.remove_items(selection)
            self.dest_list.insert_items(selection,pos+1)
            self.dest_list.select_item(pos+1)
            
            
	add_btn = gtk.Button("Add->")
        add_btn.connect("clicked", dest_add)
        # The label below just makes things align more nicely (adds space)
        btn_box.pack_start(gtk.Label(''),expand=gtk.FALSE)
	btn_box.pack_start(add_btn,expand=gtk.FALSE)


        destvbox=gtk.VBox(spacing=spc)
        label=gtk.Label('Output:')
        label.set_alignment(0,0.5)
        destvbox.pack_start(label,expand=gtk.FALSE)
	dest_win = gtk.ScrolledWindow()
	dest_win.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
	dest_win.set_size_request(300,200)
	destvbox.pack_start(dest_win)

        hbox1.pack_start(destvbox)
        destbbox=gtk.HBox(spacing=spc)
        destvbox.pack_start(destbbox,expand=gtk.FALSE)

        pix = gtk.gdk.pixmap_colormap_create_from_xpm(None,
            gtk.gdk.colormap_get_system(), None,
             os.path.join(gview.home_dir,'pics','delete.xpm'))
        im = gtk.Image()
        im.set_from_pixmap(*pix)
	del_btn = gtk.Button()
        del_btn.add(im)
        del_btn.connect("clicked", dest_del)
	destbbox.pack_start(del_btn,expand=gtk.FALSE)

        pix = gtk.gdk.pixmap_colormap_create_from_xpm(None,
            gtk.gdk.colormap_get_system(), None,
             os.path.join(gview.home_dir,'pics','raise.xpm'))
        im = gtk.Image()
        im.set_from_pixmap(*pix)
        r_btn = gtk.Button()
        r_btn.add(im)
        r_btn.connect("clicked", dest_raise)
	destbbox.pack_start(r_btn,expand=gtk.FALSE)
        
        pix = gtk.gdk.pixmap_colormap_create_from_xpm(None,
            gtk.gdk.colormap_get_system(), None,
             os.path.join(gview.home_dir,'pics','lower.xpm'))
        im = gtk.Image()
        im.set_from_pixmap(*pix)
        l_btn = gtk.Button()
        l_btn.add(im)
        l_btn.connect("clicked", dest_lower)
	destbbox.pack_start(l_btn,expand=gtk.FALSE)
        
	self.dest_list = gtk.List()
	self.dest_list.set_selection_mode(gtk.SELECTION_BROWSE)
	dest_win.add_with_viewport(self.dest_list)

        parent.shell.pack_start(self.frame)

    def clear_input_bands(self):
        self.input_bands={}
        self.source_list.select_all()
        sel=self.source_list.get_selection()
        self.source_list.remove_items(sel)

    def set_input_bands(self,bands):
        self.input_bands=bands
        self.clear_input_bands()
        self.source_list.append_items(self.input_bands.keys())
        for item in self.input_bands.keys():
            item.show()

    def add_input_bands(self,bands):
        if len(self.output_bands) > 0:
            keys=self.output_bands.keys()
            xsize,ysize=(self.output_bands[keys[0]][0].RasterXSize,
                         self.output_bands[keys[0]][0].RasterYSize)
        elif len(self.input_bands) > 0:
            keys=self.input_bands.keys()
            xsize,ysize=(self.input_bands[keys[0]][0].RasterXSize,
                         self.input_bands[keys[0]][0].RasterYSize)
        else:
            keys=bands.keys()
            xsize,ysize=(bands[keys[0]][0].RasterXSize,
                         bands[keys[0]][0].RasterYSize)
            
        # Only add bands that are of the same size
        invalid=0
        oldbands=[]
        for ckey in self.input_bands.keys():
            oldbands.append(self.input_bands[ckey][2])

        newbands=[]    
        for ckey in bands.keys():
            cxsize,cysize=(bands[ckey][0].RasterXSize,
                           bands[ckey][0].RasterYSize)
            if (cxsize == xsize) and (cysize == ysize):
                if ((bands[ckey][2] not in oldbands) and
                    (bands[ckey][2] not in newbands)):
                    self.input_bands[ckey]=bands[ckey]
                    newbands.append(self.input_bands[ckey][2])
            else:
                invalid=1
                
        if invalid != 0:
            txt='Bands for composed dataset must all\n'+\
                'be the same size.  Bands that are not\n'+\
                str(xsize)+' pixels x '+str(ysize)+' lines\n'+\
                'will be excluded from the input list.'
            gvutils.warning(txt)
        print 'Xsize: ',xsize,' Ysize: ',ysize        
        # clear old list in case some of the new keys replace
        # older ones
        self.source_list.select_all()
        sel=self.source_list.get_selection()
        self.source_list.remove_items(sel)
        self.source_list.append_items(self.input_bands.keys())
        for item in self.input_bands.keys():
            item.show()

    def get_output_bands(self):
        """ Return a list of bands.  Each list is
            a tuple- (gdal dataset, band number)
        """
        dlist=self.dest_list.children()
        out_list=[]
        for item in dlist:
            out_list.append((self.output_bands[item][0],
                             self.output_bands[item][1]))
        return out_list
    
    def show_all(self,*args):
        self.frame.show_all()
 
    def show(self,*args):
        self.frame.show()


def CopyGDALGCP(gcp):
    ngcp=gdal.GCP()
    ngcp.GCPPixel=gcp.GCPPixel
    ngcp.GCPLine=gcp.GCPLine
    ngcp.GCPX=gcp.GCPX
    ngcp.GCPY=gcp.GCPY
    ngcp.GCPZ=gcp.GCPZ
    ngcp.Id=gcp.Id
    ngcp.Info=gcp.Info

    return ngcp

def CopyGDALGCPs(gcplist):
    ngcplist=[]
    for gcp in gcplist:
        ngcplist.append(CopyGDALGCP(gcp))
    return ngcplist

               
########################################################################
def get_raster_size(_layer):
    w =  _layer.get_parent().get_dataset().GetRasterBand(1).XSize
    h =  _layer.get_parent().get_dataset().GetRasterBand(1).YSize
    return (w,h)
 

#########################################################################
def get_list_of_bands_as_dict(size=None):
    """Returns dictionary of the bands of the opened layers. The key 
    of dictionary is the ListItem object produced from the name of
    layer+band number, members of dictionary are view-object (index = 0),
    layer-object (index = 1), band-number and name"""

    layer = gview.app.sel_manager.get_active_layer()
    if layer is None:
	return {}
    if size is None:
        size = get_raster_size(layer)
    dict = {}
    for curview in gview.app.view_manager.view_list:
        for curlayer in curview.viewarea.list_layers():
    	    curname = curlayer.get_name()
            cursize = get_raster_size(curlayer)
	    if cursize == size:
		num_bands = curlayer.get_parent().get_dataset().RasterCount
                ds=curlayer.get_parent().get_dataset()
		for i in range(1,num_bands+1):
	    	    curband = curname + '.band['+ str(i) + ']'
		    dict[gtk.ListItem(curband)] = (ds,i,curband)
    if dict is None:
	return None
    return dict


TOOL_LIST = ['ComposeTool']

