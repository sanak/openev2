#!/usr/bin/env python
###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Graphical tool for translating between formats.
# Author:   Gillian Walter, gillian.walter@atlantis-scientific.com
#
###############################################################################
# Copyright (c) 2003, Atlantis Scientific Inc. (www.atlantis-scientific.com)
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

import gtk
import gview, gdal, gdalconst, layerdlg, gvutils
import os, string
import gviewapp
import pgufilesel
import gvhtml
import vrtutils
import pguprogress

class GDALTool(gviewapp.Tool_GViewApp):

    def __init__(self, app=None,startpath=None):
        gviewapp.Tool_GViewApp.__init__(self,app)

        self.init_dialog()
        self.init_menu()

        # store the id for the roi-changed signal
        # connection so it can be disconnected later
        self.roichanged_id=None

        # Set help topic
        gvhtml.set_help_topic(self.dialog,"Tool_Export.html")

    def init_menu(self):
        self.menu_entries.set_entry("File/Export",1,self.gdal_tool_cb)

    def gdal_tool_cb(self,*args):
        # Activate the view that the export tool was launched from
        # so that it's active layer is displayed in the input field
        for view in self.app.view_manager.view_list:
            if view.title == args[1]:
                self.app.view_manager.set_active_view(view)

        for item in self.show_list:
            item.show()

        if self.button_dict['Mode'].get_active() == True:
            for item in self.adv_show_list:
                item.show()
        else:
            for item in self.adv_show_list:
                item.hide()

        self.dialog.show()

        self.dialog.window._raise()
        self.reconnect()
        self.refresh_cb()

    def reconnect(self,*args):
        self.roichanged_id=self.app.toolbar.roi_tool.connect('roi-changed',self.refresh_roiinfo)

    def set_roitool(self, *args):
        self.app.toolbar.roi_button.set_active(True)

    def refresh_cb(self,*args):
        self.refresh_fileinfo()
        if self.button_dict['IP_window'].get_active():
            self.refresh_roiinfo()

    def refresh_roiinfo(self,*args):
        try:
            roi_info = self.app.toolbar.get_roi()
        except:
            # if roi has been disabled (eg. if current selection
            # mode is poi), leave it at the latest value
            roi_info = None    


        [cview, clayer] = self.app.layerdlg.get_selected_layer()

        if (cview is None) or (clayer is None):
            # roi only makes sense in the context of a view and layer
            return None

        if roi_info is None:
            cds=clayer.get_parent().get_dataset()
            npix=cds.RasterXSize
            nline=cds.RasterYSize
            roi_info = (0,0,npix,nline)
        else:    
            # Get a reference to the active view
            cur_view = self.app.layerdlg.get_active_view()
            if (cur_view.get_raw(clayer) == 0):
                # View is not in row/col coords-- convert
                # Note that region will not be exactly the one drawn,
                # since a rectangle is extracted rather than a 
                # general parallelogram.  Get biggest rectangle.

                roi_info_nogeo = ROI_view_to_pixel(clayer,roi_info)

                max_pix = roi_info_nogeo[0] + roi_info_nogeo[2]
                min_pix = roi_info_nogeo[0]
                max_line = roi_info_nogeo[1] + roi_info_nogeo[3]
                min_line = roi_info_nogeo[1]

                # Make sure ROI is reasonable in terms of extraction
                if min_pix < 0:
                    print 'Warning- negative start pixel.  Resetting to 0.'
                    min_pix = 0
                    if max_pix < min_pix+1:
                        max_pix = min_pix + 1

                if min_line < 0:
                    print 'Warning- negative start line.  Resetting to 0.'
                    min_line = 0
                    if max_line < min_line+1:
                        max_line = min_line + 1

                roi_info = (min_pix,min_line,max_pix-min_pix,max_line-min_line)

        self.frame_dict['IP_window'].update(roi_info)


    def refresh_fileinfo(self,*args):
        clayer = self.app.sel_manager.get_active_layer()
        try:
            fname = clayer.get_parent().get_dataset().GetDescription()
            self.frame_dict['Files'].set_dsfile(fname,'Input')
        except:
            pass

    def init_dialog(self):
        self.dialog = gtk.Window()
        self.dialog.set_title('GDAL Export Tool')
        self.dialog.set_border_width(10)
        self.tips=gtk.Tooltips()
        #self.dialog.set_default_size(500,400)
        self.dialog.set_resizable(True)

        # main shell 
        mainshell = gtk.HBox(spacing=1, homogeneous=False)
        self.dialog.add(mainshell)
        self.show_list=[]
        self.adv_show_list=[]            # advanced show list
        self.show_list.append(mainshell)

        #navigation shell
        navshell = gtk.VBox(spacing=1, homogeneous=False)
        mainshell.pack_start(navshell)
        self.show_list.append(navshell)

        self.frame_dict={}
        self.button_dict={}
        self.frame_dict['Files']=gvutils.GvDataFilesFrame('Data Files',
                                            sel_list=('Input','Output'))
        self.frame_dict['Files'].set_border_width(5)
        self.frame_dict['Files'].set_spacings(5,5)
        self.frame_dict['Files'].show_all()
        navshell.pack_start(self.frame_dict['Files'])
        self.show_list.append(self.frame_dict['Files'])

        bopt_frame=gtk.Frame('Basic Options')
        self.frame_dict['Basic Options']=bopt_frame
        self.show_list.append(bopt_frame)
        navshell.pack_start(bopt_frame,False,False,0)
        bopt_table=gtk.Table(2,4,False)
        bopt_table.set_border_width(5)
        bopt_table.set_row_spacings(5)
        bopt_table.set_col_spacings(5)
        bopt_frame.add(bopt_table)
        self.show_list.append(bopt_table)

        # Might be nice to have more formats below, but
        # this involves error checking to test for
        # supported data types, etc.
        fmtlabel=gtk.Label('Output Format: ')
        fmtlabel.set_alignment(0,0.5)
        self.show_list.append(fmtlabel)
        bopt_table.attach(fmtlabel,0,1,0,1)

        self.format_list = []
        hist_idx=0
	for iDriver in gdal.GetDriverList():
	    try:
		if iDriver.GetMetadata()["DCAP_CREATECOPY"] == "YES":
                    if iDriver.ShortName == 'DTED':
                        # DTED is a special case that needs certain
                        # conditions to be valid.  Skip it.
                        continue
		    self.format_list.append(iDriver.ShortName)
                    # Default to GTiff if possible
                    if iDriver.ShortName == "GTiff":
                        hist_idx=len(self.format_list)-1
	    except KeyError:
		pass        

        #self.format_list=['GTiff']
        self.format_menu = gvutils.GvOptionMenu(self.format_list)
        self.format_menu.set_history(hist_idx)
        self.show_list.append(self.format_menu)
        bopt_table.attach(self.format_menu,1,2,0,1)
        self.button_dict['Format_help']=gtk.Button('Help')
        self.show_list.append(self.button_dict['Format_help'])
        bopt_table.attach(self.button_dict['Format_help'],2,3,0,1)
        reslabel=gtk.Label('Output Resolution: ')
        reslabel.set_alignment(0,0.5)
        self.show_list.append(reslabel)
        bopt_table.attach(reslabel,0,1,1,2)        
        self.res_list = ['Full','1:2','1:4','1:8']        
        self.res_menu = gvutils.GvOptionMenu(self.res_list)
        bopt_table.attach(self.res_menu,1,2,1,2)
        self.show_list.append(self.res_menu)


        self.button_dict['Mode']=gtk.CheckButton('Advanced Options')
        navshell.pack_start(self.button_dict['Mode'])
        self.show_list.append(self.button_dict['Mode'])

        self.frame_dict['IP_window']=DataWindowFrame(navshell)
        self.adv_show_list.append(self.frame_dict['IP_window'])

        iopt_frame=gtk.Frame('Interactive Options')
        self.frame_dict['Interactive Options']=iopt_frame
        self.adv_show_list.append(iopt_frame)
        navshell.pack_start(iopt_frame,False,False,0)
        iopt_table=gtk.Table(3,3,False)
        iopt_table.set_border_width(5)
        iopt_table.set_row_spacings(5)
        iopt_table.set_col_spacings(5)
        iopt_frame.add(iopt_table)
        self.adv_show_list.append(iopt_table)
        self.button_dict['IP_window']=gtk.CheckButton('Window Input File')
        iopt_table.attach(self.button_dict['IP_window'],0,2,0,1)
        self.adv_show_list.append(self.button_dict['IP_window'])                                     
        self.button_dict['Scale']=gtk.CheckButton('Scale to View Settings')
        self.tips.set_tip(self.button_dict['Scale'],'Scale the output bands '+
                          'according to the min/max settings of the '+
                          'currently active raster layer.  This only '+
                          'applies to real data.')
        iopt_table.attach(self.button_dict['Scale'],0,2,1,2)
        self.adv_show_list.append(self.button_dict['Scale'])

        self.button_dict['Refresh']=gtk.Button('Active Layer->Input Filename')
        self.tips.set_tip(self.button_dict['Refresh'],'Set the input '+
                          'filename to that of the currently active layer') 
        iopt_table.attach(self.button_dict['Refresh'],0,1,2,3)
        self.adv_show_list.append(self.button_dict['Refresh'])
        self.button_dict['Enable_ROI']=gtk.Button('Draw ROI mode')
        self.tips.set_tip(self.button_dict['Enable_ROI'],'Re-activate the '+
              'ROI mode used for interactive input file window definition')
        iopt_table.attach(self.button_dict['Enable_ROI'],1,2,2,3)
        self.adv_show_list.append(self.button_dict['Enable_ROI'])        

        self.frame_dict['Other_Advanced']=gtk.Frame('')
        self.frame_dict['Other_Advanced'].set_shadow_type(gtk.SHADOW_NONE)
        self.adv_show_list.append(self.frame_dict['Other_Advanced'])
        oadvbox=gtk.VBox(spacing=5,homogeneous=False)
        oadvbox.set_border_width(5)
        self.adv_show_list.append(oadvbox)

        self.frame_dict['Other_Advanced'].add(oadvbox)

        otable=gtk.Table(2,3,False)
        otable.set_row_spacings(5)
        otable.set_col_spacings(5)
        self.adv_show_list.append(otable)
        oadvbox.pack_start(otable)                
        self._overview_list=['None','Nearest','Average']
        self.overview_menu=gvutils.GvOptionMenu(self._overview_list)
        ovrlabel=gtk.Label('Overviews:')
        self.tips.set_tip(self.overview_menu,'Tiled overview creation options')
        ovrlabel.set_alignment(0,0.5)
        self.adv_show_list.append(ovrlabel)
        otable.attach(ovrlabel,0,1,0,1)
        otable.attach(self.overview_menu,1,2,0,1)
        self.adv_show_list.append(self.overview_menu)

        self._geocode_list=['Default','GCP','Geotransform']
        self.geocoding_menu = gvutils.GvOptionMenu(self._geocode_list)
        geolabel=gtk.Label('Geocoding:')
        self.tips.set_tip(self.geocoding_menu,
                          'Specify the type of georeferencing '+
                          'information to output.  Default is to output '+
                          'all available geocoding from the input file.  '+
                          'If GCP or Geotransform is selected, geocoding '+
                          'information will only be output if it is of the '+
                          'selected type.  This may later be updated to '+
                          'generate information of the specified form if '+
                          'it is not present but can be accurately computed '+
                          'from the existing information.')
        geolabel.set_alignment(0,0.5)
        self.adv_show_list.append(geolabel)
        otable.attach(geolabel,0,1,1,2)
        otable.attach(self.geocoding_menu,1,2,1,2)
        self.adv_show_list.append(self.geocoding_menu)

        opthbox=gtk.HBox(spacing=5,homogeneous=False)
        self.adv_show_list.append(opthbox)
        oadvbox.pack_start(opthbox)
        optlabel=gtk.Label('Create Options:')
        optlabel.set_alignment(0,0.5)
        self.adv_show_list.append(optlabel)
        self.optentry=gtk.Entry()
        self.optentry.set_editable(editable=True)
        self.optentry.set_size_request(400,25)
        self.optentry.set_text('')
        self.adv_show_list.append(self.optentry)
        opthbox.pack_start(optlabel)
        opthbox.pack_start(self.optentry)

        navshell.pack_start(self.frame_dict['Other_Advanced'],
                            False,False,0)

        echbox=gtk.HBox(spacing=5,homogeneous=False)
        echbox.set_border_width(3)
        navshell.pack_end(echbox,False,False,0)
        self.show_list.append(echbox)
        self.button_dict['Close']=gtk.Button('Close')
        echbox.pack_end(self.button_dict['Close'],expand=True)
        self.show_list.append(self.button_dict['Close'])
        self.button_dict['Export']=gtk.Button('Export')
        echbox.pack_end(self.button_dict['Export'],expand=True)
        self.show_list.append(self.button_dict['Export'])

        self.button_dict['Format_help'].connect('clicked',
                                                self.format_help_cb)

        self.button_dict['Enable_ROI'].connect('clicked',self.set_roitool)
        self.button_dict['Refresh'].connect('clicked',
                                            self.refresh_fileinfo)        
        self.button_dict['Export'].connect('clicked',self.export_cb)
        self.button_dict['Close'].connect('clicked',self.close)

        self.button_dict['IP_window'].connect('toggled',
                                              self.ip_window_toggled_cb)
        self.button_dict['Mode'].connect('toggled',self.mode_toggled_cb)

        self.button_dict['IP_window'].set_active(False)
        self.button_dict['Mode'].set_active(False)
        self.frame_dict['IP_window'].set_entry_sensitivities(False)


        # Trap window close event
        self.dialog.connect('delete-event', self.close)

        for item in self.show_list:
            item.show()

        if self.button_dict['Mode'].get_active():
            for item in self.adv_show_list:
                item.show()
        else:
            for item in self.adv_show_list:
                item.hide()

    def mode_toggled_cb(self,*args):
        if self.button_dict['Mode'].get_active():
            for item in self.adv_show_list:
                item.show()
        else:            
            for item in self.adv_show_list:
                item.hide()


    def ip_window_toggled_cb(self,*args):
        if self.button_dict['IP_window'].get_active():
            self.frame_dict['IP_window'].set_entry_sensitivities(True)
            self.set_roitool()
        else:
            self.frame_dict['IP_window'].set_entry_sensitivities(False)

    def format_help_cb(self,*args):
        opformat=self.format_list[self.format_menu.get_history()]
        driver=gdal.GetDriverByName(opformat)
        topic=driver.HelpTopic
        if topic is not None:
            # hash to indicate position in html isn't
            # recognized by mozilla.  Make sure that
            # frmt_various.html is brought up for
            # formats that share this file.
            ttopic=string.split(topic,"#")
            gvhtml.LaunchHTML(ttopic[0])
        else:
            gvutils.warning('No html help available for '+opformat+' format')

    def export_cb(self,*args):
        ipfile=self.frame_dict['Files'].get('Input')
        opfile=self.frame_dict['Files'].get('Output')
        if os.path.isfile(opfile):
            resp=GtkExtra.message_box('Confirmation',opfile +
                                      ' exists.  Overwrite?',('Yes','No'))
            if resp == 'No':
                return
        elif len(opfile) == 0:
            gvutils.error('No output filename entered!')
            return

        use_viewscale=0

        rast = gdal.OpenShared(ipfile, gdalconst.GA_ReadOnly)
        if rast is None:
            if len(ipfile) == 0:
                gvutils.error('Please specify an input file!')
            else:
                gvutils.error('Unable to open ' + ipfile + ' as a GDAL supported file!')
            return

        # Catch the case where the input file consists of in-memory VRT lines
        # and the output format is also VRT.  In this case, the new VRT would
        # no longer be valid once openev was exited because the input file
        # is not on disk (filename looks something like '<VRTDataset....').
        # If the user is just exporting the file as-is, simply copying the
        # original lines to disk will suffice.  However, if they want to
        # window or scale, we'd need more complicated manipulations.  For now,
        # give an error message in that case.
        opformat=self.format_list[self.format_menu.get_history()]

        if (ipfile[0] == '<') and (opformat == 'VRT'):
            if self.res_list[self.res_menu.get_history()] != 'Full':
                msg='Only full output resolution is currently\n'+\
                    'supported for export of in-memory VRTs\n'+\
                    'to on-disk VRTs.'
                gvutils.error(msg)
                return

            if ( (self.button_dict['Mode'].get_active()) and
                ((self.button_dict['IP_window'].get_active()) or
                (self.button_dict['Scale'].get_active()) or
                (len(self.optentry.get_text()) > 0))):
                msg='Scaling, windowing, and advanced creation\n'+\
                    'options are not yet supported for export of \n'+\
                    'in-memory VRTs to on-disk VRTs'
                gvutils.error(msg)
                return

            linelist=string.split(ipfile,'\n')
            newlinelist=[]
            for item in linelist:
                newlinelist.append(item+'\n')
            fh=open(opfile,'w')
            fh.writelines(newlinelist)
            fh.close()

            ovrs=self._overview_list[self.overview_menu.get_history()]
            if ovrs != 'None':
                outds=gdal.OpenShared(opfile)
                if outds is None:
                    gvutils.error('Error opening '+opfile+' for overview creation!')
                    return

                progress = pguprogress.PGUProgressDialog( 'Building overviews...',
                                                  cancel = True )
                if ovrs is 'Nearest':
                    outds.BuildOverviews( "nearest",
                                       callback = progress.ProgressCB )
                else:
                    outds.BuildOverviews( "average_magphase",
                                       callback = progress.ProgressCB )
                progress.destroy()

            return



        vrt_opts=vrtutils.VRTCreationOptions(rast.RasterCount)

        if self._geocode_list[self.geocoding_menu.get_history()] == 'GCP':
            vrt_opts.set_geopref('gcps')
        elif self._geocode_list[self.geocoding_menu.get_history()] == 'Geotransform':
            vrt_opts.set_geopref('geotransform')

        band_list = None

        # Scale the output file according to the current view's
        # min/max
        if self.button_dict['Scale'].get_active():
            try:
                clayer = self.app.sel_manager.get_active_layer()
                if clayer.get_parent().get_dataset().GetDescription() != ipfile:
                    wtxt = 'Input file and active layer file names do not match- may '
                    wtxt = wtxt + 'result in unexpected scaling!'
                    gvutils.warning(wtxt)
                if gvutils.is_of_class(clayer.__class__,'GvRasterLayer') == 0:
                    gvutils.warning('Active layer is not a raster- view scaling ignored!')
                else:
                    src_count=clayer.sources
                    band_list = []
                    RGBAlist=['Red','Green','Blue','Alpha']
                    for src in range(src_count):
                        # layer sources are numbered 0...3; band sources are numbered 1,2,...
                        src_bandnum=clayer.get_data(src).get_band_number()
                        band_list.append(src_bandnum)
                        vrt_opts.set_scaling((clayer.min_get(src),clayer.max_get(src),0,255),(src_bandnum,))
                        vrt_opts.set_datatype(gdal.GDT_Byte,(src_bandnum,))
                        if src_count == 3:
                            vrt_opts.set_color_interp(RGBAlist[src],
                                                      (src_bandnum,))

                    # src_count is three even when there is an alpha channel
                    # for rgb/rgba case
                    if src_count == 3:
                        try:
                            src=3
                            src_bandnum=clayer.get_data(src).get_band_number()
                            band_list.append(src_bandnum)
                            vrt_opts.set_scaling((clayer.min_get(src),
                                                  clayer.max_get(src),0,255),
                                                 (src_bandnum,))
                            vrt_opts.set_datatype(gdal.GDT_Byte,(src_bandnum,))
                            vrt_opts.set_color_interp(RGBAlist[src],
                                                      (src_bandnum,))
                        except:
                            pass

                    use_viewscale=1
                    if clayer.get_mode()==gview.RLM_COMPLEX:
                        # This doesn't deal with complex yet...                        
                        gvutils.error('View scaling option is not yet supported for complex data!')
                        return
                    elif rast._band[0].DataType == gdal.GDT_CInt16:
                        # This doesn't deal with complex yet...                        
                        gvutils.error('View scaling option is not yet supported for complex data!')
                        return
                    elif rast._band[0].DataType == gdal.GDT_CInt32:
                        # This doesn't deal with complex yet...                        
                        gvutils.error('View scaling option is not yet supported for complex data!')
                        return
                    elif rast._band[0].DataType == gdal.GDT_CFloat32:
                        # This doesn't deal with complex yet...                        
                        gvutils.error('View scaling option is not yet supported for complex data!')
                        return
                    elif rast._band[0].DataType == gdal.GDT_CFloat64:
                        # This doesn't deal with complex yet...                        
                        gvutils.error('View scaling option is not yet supported for complex data!')
                        return                    
            except:
                gvutils.error('Unable to find active raster layer for scaling!')
                return

        # Get windowing options
        if self.button_dict['IP_window'].get_active():
            try:
                spix=int(self.frame_dict['IP_window'].entry_dict['start_pix'].get_text())
                sline=int(self.frame_dict['IP_window'].entry_dict['start_line'].get_text())
                npix=int(self.frame_dict['IP_window'].entry_dict['num_pix'].get_text())
                nlines=int(self.frame_dict['IP_window'].entry_dict['num_lines'].get_text())
                if (spix < 0) or (sline < 0):
                    gvutils.error('Negative start pixel and/or line!  Aborting...')
                    return
                if (npix+spix > rast.RasterXSize):
                    gvutils.error('Window is too large (last column in input: '+str(rast.RasterXSize)+')! Aborting...')
                    return
                if (nlines+sline > rast.RasterYSize):
                    gvutils.error('Window is too large (last row in input: '+str(rast.RasterYSize)+')! Aborting...')
                    return
            except:
                gvutils.error('Error retrieving window options!  Aborting...')
                return
        else:
            spix=0
            sline=0
            npix=rast.RasterXSize
            nlines=rast.RasterYSize

        vrt_opts.set_src_window((spix,sline,npix,nlines))

        if self.res_list[self.res_menu.get_history()] != 'Full':
            ovrlevel=int(self.res_list[self.res_menu.get_history()][2])
        else:
            ovrlevel=1

        vrt_opts.set_dst_window((0,0,npix/ovrlevel,nlines/ovrlevel))

        vrt_tree=vrtutils.serializeDataset(rast,vrt_opts,band_list)
        vrt_lines=gdal.SerializeXMLTree(vrt_tree)
        vrtdataset=gdal.Open(vrt_lines)

        driver=gdal.GetDriverByName(opformat)

        # Parse creation options:
        optstr=string.strip(self.optentry.get_text())
        if len(optstr) > 0:
            # should be able to deal with several
            # types of entries, eg.
            # 'TILED=YES','TFW=YES'
            # and
            # TILED=YES,TFW=YES

            if optstr[0] in ["'",'"']:
                split1=string.split(optstr,",")
                copts=[]
                for item in split1:
                    if len(item) > 2:
                        copts.append(item[1:len(item)-1])
            else:    
                copts=string.split(optstr,',')
        else:
            copts=[]

        progress = pguprogress.PGUProgressDialog( 'Export to '+opfile,
                                                  cancel = True )
        progress.SetDefaultMessage("translated")

        outdataset=driver.CreateCopy(opfile,vrtdataset,
                                     options=copts,
                                     callback=progress.ProgressCB)
        if outdataset is None:
            progress.destroy()
            gvutils.error('Unable to create output file '+opfile)
            return

        ovrs=self._overview_list[self.overview_menu.get_history()]
        if ovrs is 'Nearest':
            progress.SetDefaultMessage("overviews built")
            outdataset.BuildOverviews( "nearest",
                                       callback = progress.ProgressCB )

        elif ovrs is 'Average':
            progress.SetDefaultMessage("overviews built")
            outdataset.BuildOverviews( "average_magphase",
                                       callback = progress.ProgressCB )

        progress.destroy()




    def close(self,*args):
        if self.roichanged_id is not None:
            self.app.toolbar.roi_tool.disconnect(self.roichanged_id)
            self.roichanged_id=None

        self.dialog.hide()
        return True

#-----------------------------------------------------------------
# GUI STUFF
#-----------------------------------------------------------------

class DataWindowFrame:
    def __init__(self,parent_box,title='Input Window'):
        self.frame=gtk.Frame(title)
        self.show_list=[]
        self.show_list.append(self.frame)

        patch_table = gtk.Table(2,4,False)
        self.show_list.append(patch_table)
        self.frame.add(patch_table)

        patch_table.set_border_width(5)
        patch_table.set_col_spacings(5)
        patch_table.set_col_spacing(1, 20)

        label1 = gtk.Label('Start Line: ')
        label1.set_alignment(0, 0.5)
        patch_table.attach(label1, 0,1, 0, 1)

        self.entry_dict={}

        self.entry_dict['start_line'] = gtk.Entry()
        self.entry_dict['start_line'].set_editable(True)
        self.entry_dict['start_line'].set_size_request(90, 25)
        self.entry_dict['start_line'].set_text('0')
        patch_table.attach(self.entry_dict['start_line'], 1,2, 0,1)

        label2 = gtk.Label('Start Pixel: ')
        label2.set_alignment(0, 0.5)
        patch_table.attach(label2, 2,3,0, 1)

        self.entry_dict['start_pix'] = gtk.Entry()
        self.entry_dict['start_pix'].set_editable(True)
        self.entry_dict['start_pix'].set_size_request(90, 25)
        self.entry_dict['start_pix'].set_text('0')
        patch_table.attach(self.entry_dict['start_pix'], 3,4, 0,1)

        label3 = gtk.Label('Num. of Lines: ')
        label3.set_alignment(0, 0.5)
        patch_table.attach(label3, 0,1, 1, 2)

        self.entry_dict['num_lines'] = gtk.Entry()
        self.entry_dict['num_lines'].set_editable(True)
        self.entry_dict['num_lines'].set_size_request(90, 25)
        self.entry_dict['num_lines'].set_text('1')
        patch_table.attach(self.entry_dict['num_lines'], 1,2, 1,2)

        label4 = gtk.Label('Num. of Pixels: ')
        label4.set_alignment(0, 0.5)
        patch_table.attach(label4, 2,3,1, 2)

        self.entry_dict['num_pix'] = gtk.Entry()
        self.entry_dict['num_pix'].set_editable(True)
        self.entry_dict['num_pix'].set_size_request(90, 25)
        self.entry_dict['num_pix'].set_text('1')
        patch_table.attach(self.entry_dict['num_pix'], 3,4, 1,2)

        self.show_list.append(label1)
        self.show_list.append(label2)
        self.show_list.append(label3)
        self.show_list.append(label4)
        self.show_list.append(self.entry_dict['start_line'])
        self.show_list.append(self.entry_dict['start_pix'])
        self.show_list.append(self.entry_dict['num_lines'])
        self.show_list.append(self.entry_dict['num_pix'])

        parent_box.pack_start(self.frame,False,False,0)


    def update(self,roi_info):
        self.entry_dict['start_line'].set_text(str(int(roi_info[1])))
        self.entry_dict['start_pix'].set_text(str(int(roi_info[0])))
        self.entry_dict['num_lines'].set_text(str(int(roi_info[3])))
        self.entry_dict['num_pix'].set_text(str(int(roi_info[2])))

    def set_entry_sensitivities(self,bool_val):
        self.entry_dict['start_line'].set_sensitive(bool_val)
        self.entry_dict['start_pix'].set_sensitive(bool_val)
        self.entry_dict['num_lines'].set_sensitive(bool_val)
        self.entry_dict['num_pix'].set_sensitive(bool_val)


    def show(self,*args):
        for item in self.show_list:
            item.show()

    def hide(self,*args):
        for item in self.show_list:
            item.hide()


def ROI_view_to_pixel(clayer,roi_info):

    [pixel,line] = clayer.view_to_pixel(roi_info[0],roi_info[1])
    [pixel2,line2] = clayer.view_to_pixel(roi_info[0]+roi_info[2],
                                          roi_info[1]+roi_info[3])
    [pixel3,line3] = clayer.view_to_pixel(roi_info[0],
                                          roi_info[1]+roi_info[3])
    [pixel4,line4] = clayer.view_to_pixel(roi_info[0]+roi_info[2],
                                          roi_info[1])

    # Get pixel-space rectangle
    max_pix = max(pixel,pixel2,pixel3,pixel4)
    min_pix = min(pixel,pixel2,pixel3,pixel4)
    max_line = max(line,line2,line3,line4)
    min_line = min(line,line2,line3,line4)

    roi_info_pixel = (min_pix,min_line,max_pix-min_pix,max_line-min_line)

    return roi_info_pixel


TOOL_LIST = ['GDALTool']

