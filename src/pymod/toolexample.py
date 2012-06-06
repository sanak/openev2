#
# Tool designed to be added to gviewapp's self.Tool_List
# To try: openev -t <full path to toolfile_example.txt>
# Assumes that gviewapp at least has a toolbar with 
# roi and poi tools, a layer manager, and view manager.

import gviewapp
import pygtk
pygtk.require('2.0')
import gtk
import os
from osgeo import gdal
from osgeo import gdalnumeric
import numpy as Numeric
import string
from gvsignaler import Signaler

import gview,gvutils


class StoredROIPOI:
    def __init__(self):
        self.roi = (1,1,1,1)
        self.poi = (1,1) # pixel/line in raster coords

        # View/layer used to select poi/roi
        self.roi_layer = None
        self.roi_view = None
        self.poi_layer = None
        self.poi_view = None

    def update_roi(self,roi_info,roilayer,roiview):
        self.roi = roi_info
        self.roi_layer = roilayer
        self.roi_view = roiview

    def update_poi(self,poi_info,poilayer,poiview):
        self.poi = poi_info
        self.poi_layer = poilayer
        self.poi_view = poiview


class GeneralPOITool(gviewapp.Tool_GViewApp):
    def __init__(self,app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.RP_Stored = StoredROIPOI()

        # relegate initialization of dialog, menu
        # and connections to functions so they 
        # can be customized
        self.init_dialog()
        self.init_menu()
        self.init_connections()

    def init_dialog(self):
        self.RP_ToolDlg = General_POIToolDlg()

    def init_menu(self):
        self.menu_entries.set_entry("Tools/POI Analysis Tool",1,self.roipoitool_cb)

    def init_connections(self):
        self.app.toolbar.poi_tool.connect('poi-changed',self.update_poi_cb)
        self.RP_ToolDlg.subscribe('re-activated',self.update_dlgpoi_frame)
        self.RP_ToolDlg.subscribe('poitool-needs-set',self.set_poitool)
        self.RP_ToolDlg.subscribe('analyze-pressed',self.analyze_cb)

    def roipoitool_cb(self, *args):
        self.RP_ToolDlg.show_all()
        self.RP_ToolDlg.make_active()
        self.RP_ToolDlg.window.raise_()

    def update_poi_cb(self, *args):
        # always store last chosen poi
        try:
            poi_info = self.app.toolbar.get_poi()
        except:
            # if poi has been disabled (eg. if current selection
            # mode is roi), leave it at the latest value
            return       

        # Returns the NAME of the view, and a COPY of the layer
        [cview, clayer] = self.app.layerdlg.get_selected_layer()

        if (cview is None) or (clayer is None):
            # poi only makes sense in the context of a view and layer
            return

        # Get a reference to the active view
        cur_view = self.app.layerdlg.get_active_view()
        if (cur_view.get_raw(clayer) == 0):
            # View is not in row/col coords-- convert
            [pixel,line] = clayer.view_to_pixel(poi_info[0],poi_info[1])
            poi_info = (pixel,line)

        self.RP_Stored.update_poi(poi_info,clayer,cview)
       # if tooldlg is active, update its region display frame
        if self.RP_ToolDlg.is_active():
            self.update_dlgpoi_frame()

    def update_dlgpoi_frame(self, *args):
        if self.RP_ToolDlg.is_active():       
            poi_info = self.RP_Stored.poi
            self.RP_ToolDlg.update_poiframe(poi_info)
            if self.RP_ToolDlg.is_auto_updating():
                # Automatic analysis is on
                self.RP_ToolDlg.analyze_cb(self)

        else:
            return

    def set_poitool(self, *args):
        self.app.toolbar.poi_button.set_active(True)

    def analyze_cb(self,*args):
        line = self.RP_ToolDlg.entry_dict['line'].get_text()
        pix = self.RP_ToolDlg.entry_dict['pixel'].get_text()
        print 'In poi tool analyze_cb- POI is: ',line,'L ',pix,'P'


class GeneralROITool(gviewapp.Tool_GViewApp):
    def __init__(self,app=None):
        gviewapp.Tool_GViewApp.__init__(self,app)
        self.RP_Stored = StoredROIPOI()

        # relegate initialization of dialog, menu
        # and connections to functions so they 
        # can be customized
        self.init_dialog()
        self.init_menu()
        self.init_connections()

    def init_dialog(self):
        self.RP_ToolDlg = General_ROIToolDlg()

    def init_menu(self):
        self.menu_entries.set_entry("Tools/ROI Analysis Tool",1,self.roipoitool_cb)

    def init_connections(self):
        self.app.toolbar.roi_tool.connect('roi-changed',self.update_roi_cb)
        self.RP_ToolDlg.subscribe('re-activated',self.update_dlgroi_frame)
        self.RP_ToolDlg.subscribe('roitool-needs-set',self.set_roitool)
        self.RP_ToolDlg.subscribe('analyze-pressed',self.analyze_cb)

    def roipoitool_cb(self, *args):
        self.RP_ToolDlg.show_all()
        self.RP_ToolDlg.make_active()
        self.RP_ToolDlg.window.raise_()

    def update_roi_cb(self, *args):
        # always store last chosen roi
        try:
            roi_info = self.app.toolbar.get_roi()
        except:
            # if roi has been disabled (eg. if current selection
            # mode is poi), leave it at the latest value
            return       

        [cview, clayer] = self.app.layerdlg.get_selected_layer()

        if (cview is None) or (clayer is None):
            # roi only makes sense in the context of a view and layer
            return

        # Get a reference to the active view
        cur_view = self.app.layerdlg.get_active_view()
        if (cur_view.get_raw(clayer) == 0):
            # View is not in row/col coords-- convert
            # Note that region will not be exactly the one drawn,
            # since a rectangle is extracted rather than a 
            # trapezoid.

            [pixel,line] = clayer.view_to_pixel(roi_info[0],roi_info[1])
            [pixel2,line2] = clayer.view_to_pixel(roi_info[0]+roi_info[2],
                                                  roi_info[1]+roi_info[3])
            if pixel > pixel2:
                temp = pixel
                pixel = pixel2
                pixel2 = temp

            if line > line2:
                temp = line
                line = line2
                line2 = temp

            roi_info = (pixel,line,pixel2-pixel,line2-line)

        self.RP_Stored.update_roi(roi_info,clayer,cview)
        # if tooldlg is active, update its region display frame
        if self.RP_ToolDlg.is_active():
            self.update_dlgroi_frame()

    def update_dlgroi_frame(self, *args):
        if self.RP_ToolDlg.is_active():       
            roi_info = self.RP_Stored.roi
            self.RP_ToolDlg.update_roiframe(roi_info)
            if self.RP_ToolDlg.is_auto_updating():
                # Automatic analysis is on
                self.RP_ToolDlg.analyze_cb(self)

        else:
            return

    def set_roitool(self, *args):
        self.app.toolbar.roi_button.set_active(True)

    def analyze_cb(self,*args):
        line = self.RP_ToolDlg.entry_dict['start_line'].get_text()
        pix = self.RP_ToolDlg.entry_dict['start_pix'].get_text()
        sl =  self.RP_ToolDlg.entry_dict['num_lines'].get_text()
        sp =  self.RP_ToolDlg.entry_dict['num_pix'].get_text()
        print 'In roi tool analyze_cb- ROI is: ',line,'L ',pix,'P (',sl,'x',sp,')'


class General_ROIPOIToolDlg(gtk.Window,Signaler):
    # A base class that has 3 frames: a 
    # top one with a "needs updating" string
    # and Analyze button; a middle one
    # that will display either an roi or
    # a poi; and an ending one with 2 toggles
    # and a Set Tool button.
    def __init__(self):
        gtk.Window.__init__(self)

        self.button_dict = {}  # store references to buttons/frames/text entries--
        self.frame_dict = {}   # we may need to access them.
        self.entry_dict = {}

        self.show_list = []    # list of widgets to show

        self.init_setup_window()
        self.init_create_gui_panel()        # base widgets/panel
        self.init_customize_gui_panel()

        # signals
        self.publish('re-activated')
        self.publish('roitool-needs-set')
        self.publish('poitool-needs-set')
        self.publish('analyze-pressed')

        # Basic connections
        self.button_dict['Analyze'].connect('clicked',self.analyze_cb)
        self.button_dict['Activate'].connect('toggled',self.activate_toggled)
        self.button_dict['Auto Update'].connect('toggled',self.auto_update_toggled)
        self.button_dict['Set Tool'].connect('clicked',self.set_tool_cb)

        # Set default sensitivities
        self.button_dict['Auto Update'].set_active(False)
        self.button_dict['Analyze'].set_sensitive(True)
        self.button_dict['Activate'].set_active(False)

        self.connect('delete-event',self.close) 


        for item in self.show_list:
            item.show()

    def init_setup_window(self):
        self.set_title('General Tool')
        self.set_border_width(10)
        self.set_size_request(450,300)

    def init_create_gui_panel(self):

        # Basic Buttons...
        self.button_dict['Analyze'] = gtk.Button('Analyze')
        self.show_list.append(self.button_dict['Analyze'])

        self.button_dict['Activate'] = gtk.CheckButton('Activate')
        self.show_list.append(self.button_dict['Activate'])

        self.button_dict['Auto Update'] = gtk.CheckButton('Auto Update')
        self.show_list.append(self.button_dict['Auto Update'])

        self.button_dict['Set Tool'] = gtk.Button('Set Tool')
        self.show_list.append(self.button_dict['Set Tool'])

        # Basic Frames...

        # first frame
        self.frame_dict['base_frame1'] = gtk.Frame()
        self.show_list.append(self.frame_dict['base_frame1'])

        hbox1 = gtk.HBox(True,5)
        self.show_list.append(hbox1)
        hbox1.pack_end(self.button_dict['Analyze'],True,True,0)
        self.frame_dict['base_frame1'].add(hbox1)

        # second frame- will contain roi or poi info
        self.frame_dict['base_frame2'] = gtk.Frame()
        self.show_list.append(self.frame_dict['base_frame2'])

        # third frame
        self.frame_dict['base_frame3'] = gtk.Frame();
        self.show_list.append(self.frame_dict['base_frame3'])

        hbox3 = gtk.HBox(True,5)
        self.show_list.append(hbox3)
        hbox3.pack_start(self.button_dict['Activate'],True,True,0)
        hbox3.pack_start(self.button_dict['Auto Update'],True,True,0)
        hbox3.pack_start(self.button_dict['Set Tool'],True,True,0)
        self.frame_dict['base_frame3'].add(hbox3)

        # Top level panel...
        self.main_panel = gtk.VBox(False,5)
        self.main_panel.pack_start(self.frame_dict['base_frame1'],False,False,0)
        self.main_panel.pack_start(self.frame_dict['base_frame2'],False,False,0)
        self.main_panel.pack_end(self.frame_dict['base_frame3'],False,False,0)
        self.add(self.main_panel)
        self.show_list.append(self.main_panel)

    def init_customize_gui_panel(self):
        pass

    def analyze_cb(self,*args):
        Signaler.notify(self, 'analyze-pressed')

    def activate_toggled(self,*args):
        if self.button_dict['Activate'].get_active():
            self.set_entry_sensitivities(True)
            self.button_dict['Auto Update'].set_sensitive(True)
            self.button_dict['Auto Update'].set_active(False)
            self.button_dict['Analyze'].set_sensitive(True)
            self.button_dict['Set Tool'].set_sensitive(True)
            Signaler.notify(self, 're-activated')
        else:
            self.set_entry_sensitivities(False)
            self.button_dict['Auto Update'].set_active(False)
            self.button_dict['Auto Update'].set_sensitive(False)
            self.button_dict['Analyze'].set_sensitive(False)
            self.button_dict['Set Tool'].set_sensitive(False)


    def auto_update_toggled(self,*args):
        # If auto-update is on, analyze button should not be sensitive
        if self.button_dict['Auto Update'].get_active():
            self.button_dict['Analyze'].set_sensitive(False)
        else:
            self.button_dict['Analyze'].set_sensitive(True)

    def set_tool_cb(self,*args):
        pass

    def set_entry_sensitivities(self,bool_val):
        pass

    def is_active(self):
        if self.button_dict['Activate'].get_active():
            return True
        else:
            return False

    def make_active(self):
        self.set_tool_cb()
        self.button_dict['Activate'].set_active(True)

    def is_auto_updating(self):
        if self.button_dict['Auto Update'].get_active():
            return True
        else:
            return False

    def close(self,*args):
        # If tool is closed, user probably wants to be rid of it...
        self.button_dict['Activate'].set_active(False)
        self.hide()
        return True


class General_ROIToolDlg(General_ROIPOIToolDlg):
    def __init__(self):
        General_ROIPOIToolDlg.__init__(self)

    def init_setup_window(self):
        self.set_title('General ROI Tool')
        self.set_border_width(10)
        self.set_size_request(450,150)

    def init_customize_gui_panel(self):    
        # By now, main panel, basic frames, buttons have been created
        # Middle frame (region display) must be filled in.

        patch_table = gtk.Table(2,4,False)
        self.show_list.append(patch_table)
        self.frame_dict['base_frame2'].add(patch_table)

        patch_table.set_border_width(5)
        patch_table.set_col_spacings(5)
        patch_table.set_col_spacing(1, 20)

        label1 = gtk.Label('Start Line: ')
        label1.set_alignment(0, 0.5)
        patch_table.attach(label1, 0,1, 0, 1)

        self.entry_dict['start_line'] = gtk.Entry()
        self.entry_dict['start_line'].set_editable(False)
        self.entry_dict['start_line'].set_size_request(90, 25)
        self.entry_dict['start_line'].set_text('1')
        patch_table.attach(self.entry_dict['start_line'], 1,2, 0,1)

        label2 = gtk.Label('Start Pixel: ')
        label2.set_alignment(0, 0.5)
        patch_table.attach(label2, 2,3,0, 1)

        self.entry_dict['start_pix'] = gtk.Entry()
        self.entry_dict['start_pix'].set_editable(False)
        self.entry_dict['start_pix'].set_size_request(90, 25)
        self.entry_dict['start_pix'].set_text('1')
        patch_table.attach(self.entry_dict['start_pix'], 3,4, 0,1)

        label3 = gtk.Label('Num. of Lines: ')
        label3.set_alignment(0, 0.5)
        patch_table.attach(label3, 0,1, 1, 2)

        self.entry_dict['num_lines'] = gtk.Entry()
        self.entry_dict['num_lines'].set_editable(False)
        self.entry_dict['num_lines'].set_size_request(90, 25)
        self.entry_dict['num_lines'].set_text('1')
        patch_table.attach(self.entry_dict['num_lines'], 1,2, 1,2)

        label4 = gtk.Label('Num. of Pixels: ')
        label4.set_alignment(0, 0.5)
        patch_table.attach(label4, 2,3,1, 2)

        self.entry_dict['num_pix'] = gtk.Entry()
        self.entry_dict['num_pix'].set_editable(False)
        self.entry_dict['num_pix'].set_size_request(90, 25)
        self.entry_dict['num_pix'].set_text('1')
        patch_table.attach(self.entry_dict['num_pix'], 3,4, 1,2)

        # Create tooltips
        self.tooltips = gtk.Tooltips()
        tip_text = "Re-enable region-of-interest selection mode (" \
                   "required for this tool's operation)."
        self.tooltips.set_tip(self.button_dict['Set Tool'],tip_text)
        tip_text = 'Automatically update every time the region of ' + \
                   'interest changes.'
        self.tooltips.set_tip(self.button_dict['Auto Update'],tip_text)
        tip_text = 'Enable/Disable this tool.'
        self.tooltips.set_tip(self.button_dict['Activate'],tip_text)
        tip_text = 'Perform analysis.'
        self.tooltips.set_tip(self.button_dict['Analyze'],tip_text)


        self.frame_dict['base_frame2'].show_all()

    def set_tool_cb(self,*args):
        Signaler.notify(self, 'roitool-needs-set')


    def update_roiframe(self,roi_info):
        self.entry_dict['start_line'].set_text(str(int(roi_info[1])))
        self.entry_dict['start_pix'].set_text(str(int(roi_info[0])))
        self.entry_dict['num_lines'].set_text(str(int(roi_info[3])))
        self.entry_dict['num_pix'].set_text(str(int(roi_info[2])))

    def set_entry_sensitivities(self,bool_val):
        self.entry_dict['start_line'].set_sensitive(bool_val)
        self.entry_dict['start_pix'].set_sensitive(bool_val)
        self.entry_dict['num_lines'].set_sensitive(bool_val)
        self.entry_dict['num_pix'].set_sensitive(bool_val)



class General_POIToolDlg(General_ROIPOIToolDlg):
    def __init__(self):
        General_ROIPOIToolDlg.__init__(self)

    def init_setup_window(self):
        self.set_title('General POI Tool')
        self.set_border_width(10)
        self.set_size_request(450,120)

    def init_customize_gui_panel(self):    
        # By now, main panel, basic frames, buttons have been created
        # Middle frame (region display) must be filled in.

        patch_table = gtk.Table(1,4,False)
        self.show_list.append(patch_table)
        self.frame_dict['base_frame2'].add(patch_table)

        patch_table.set_border_width(5)
        patch_table.set_col_spacings(5)
        patch_table.set_col_spacing(1, 20)

        label1 = gtk.Label('Line: ')
        label1.set_alignment(0, 0.5)
        patch_table.attach(label1, 0,1, 0, 1)

        self.entry_dict['line'] = gtk.Entry()
        self.entry_dict['line'].set_editable(False)
        self.entry_dict['line'].set_size_request(90, 25)
        self.entry_dict['line'].set_text('1')
        patch_table.attach(self.entry_dict['line'], 1,2, 0,1)

        label2 = gtk.Label('Pixel: ')
        label2.set_alignment(0, 0.5)
        patch_table.attach(label2, 2,3,0, 1)

        self.entry_dict['pixel'] = gtk.Entry()
        self.entry_dict['pixel'].set_editable(False)
        self.entry_dict['pixel'].set_size_request(90, 25)
        self.entry_dict['pixel'].set_text('1')
        patch_table.attach(self.entry_dict['pixel'], 3,4, 0,1)

        # Create tooltips
        self.tooltips = gtk.Tooltips()
        tip_text = "Re-enable point-of-interest selection mode (" + \
                   "required for this tool's operation)."
        self.tooltips.set_tip(self.button_dict['Set Tool'],tip_text)
        tip_text = 'Automatically update every time the point of ' + \
                   'interest changes.'
        self.tooltips.set_tip(self.button_dict['Auto Update'],tip_text)
        tip_text = 'Enable/Disable this tool.'
        self.tooltips.set_tip(self.button_dict['Activate'],tip_text)
        tip_text = 'Perform analysis.'
        self.tooltips.set_tip(self.button_dict['Analyze'],tip_text)


        self.frame_dict['base_frame2'].show_all()

    def set_tool_cb(self,*args):
        Signaler.notify(self, 'poitool-needs-set')

    def update_poiframe(self,poi_info):
        self.entry_dict['line'].set_text(str(int(poi_info[1])))
        self.entry_dict['pixel'].set_text(str(int(poi_info[0])))

    def set_entry_sensitivities(self,bool_val):
        self.entry_dict['line'].set_sensitive(bool_val)
        self.entry_dict['pixel'].set_sensitive(bool_val)


class Pixel_Tool(GeneralPOITool):
    def __init__(self,app=None):
        GeneralPOITool.__init__(self,app)

    def init_dialog(self):
        self.RP_ToolDlg = Pixel_ToolDlg()

    def init_menu(self):
        self.menu_entries.set_entry("Tools/Pixel Tool",1,self.roipoitool_cb)

    def analyze_cb(self,*args):
        # Find the view and layer
        [cview, clayer] = self.app.layerdlg.get_selected_layer()
        if (cview is None) or (clayer is None):
            # analysis only makes sense in the context of a view and layer
            gvutils.error('No View/Layer selected!')
            return

        text = self.basic_pixel_analysis(cview,clayer)

        self.RP_ToolDlg.update_pixelinfo_text(text)

    def basic_pixel_analysis(self,cview,clayer):
        line = self.RP_ToolDlg.entry_dict['line'].get_text()
        pix = self.RP_ToolDlg.entry_dict['pixel'].get_text()
        disp_text = 'Pixel Attributes: \n'
        fname = clayer.get_name()   # layer title == file name
        disp_text = disp_text + '    Filename: ' + fname + '\n'
        disp_text = disp_text + '    Line: ' + str(line) 
        disp_text = disp_text + '  Pixel: ' + str(pix) + '\n'
        try:
            gdal_dataset = clayer.get_data().get_dataset()
        except:
            disp_text = disp_text + '\n\n<Not a raster layer>\n'
            return disp_text

        [long,lat]=clayer.get_data().pixel_to_georef(float(pix),float(line))
        disp_text = disp_text + '    Latitude: ' + str(lat)
        disp_text = disp_text + '    Longitude: ' + str(long) + '\n'

        try:
            value = gdal_dataset.ReadAsArray(int(float(pix)),int(float(line)),1,1)
        except:
            disp_text = disp_text + '\n\n<Unable to read data>\n'
            return disp_text

        disp_text = disp_text +'\n    Value: ' + str(value[0,0].astype(value.typecode()))
        return disp_text




class Pixel_ToolDlg(General_POIToolDlg):
    def __init__(self):
        General_POIToolDlg.__init__(self)

    def init_setup_window(self):
        self.set_title('Pixel Tool')
        self.set_border_width(10)
        self.set_size_request(450,450)

    def init_customize_gui_panel(self):
        # Inherit all the usual stuff...
        General_POIToolDlg.init_customize_gui_panel(self)

        # Add new frame with pixel info, keeping track of
        # the frame and text object...
        self.frame_dict['pixel_info_frame'] = gtk.Frame()
        self.show_list.append(self.frame_dict['pixel_info_frame'])

        pixel_vbox = gtk.VBox()
        self.show_list.append(pixel_vbox)
        self.frame_dict['pixel_info_frame'].add(pixel_vbox)

        pixel_scroll = gtk.ScrolledWindow()
        self.show_list.append(pixel_scroll)
        pixel_vbox.pack_start(pixel_scroll,expand = True)

        text_buff = gtk.TextBuffer();
        self.entry_dict['pixel_info_text_buffer'] = text_buff;
        text_view = gtk.TextView(text_buff);
        text_view.set_wrap_mode(gtk.WRAP_NONE)
        text_view.set_editable(False)
        self.show_list.append(text_view)
        pixel_scroll.add(text_view)
        self.entry_dict['pixel_info_text_view'] = text_view;
        self.main_panel.pack_start(self.frame_dict['pixel_info_frame'],True,True,0)        

    def update_pixelinfo_text(self,text):
	self.entry_dict['pixel_info_text_buffer'].set_text(text);

class Stats_Tool(GeneralROITool):
    def __init__(self,app=None):
        GeneralROITool.__init__(self,app)

        # Information needed for creating target view window
        self.target_view_title = 'Stats Tool Target Area'
        self.target_view_window = None
        self.target_view_layer = None
        self.target_view_data = None    # Numpy array of data
        self.target_view_ds = None      # gdal dataset for target_view_data

    def init_dialog(self):
        self.RP_ToolDlg = Stats_ToolDlg()

    def init_menu(self):
        self.menu_entries.set_entry("Tools/Stats Tool",3,self.roipoitool_cb)

    def update_dlgroi_frame(self,*args):
        # Update frame as in general tool
        if self.RP_ToolDlg.is_active():       
            roi_info = self.RP_Stored.roi
            self.RP_ToolDlg.update_roiframe(roi_info)

        else:
            return

        self.update_roi_view()

        if self.RP_ToolDlg.is_auto_updating():
            # Automatic analysis is on
            self.RP_ToolDlg.analyze_cb(self)

    def update_roi_view(self,*args):
        # Shouldn't get here if inactive anyway, but just in case...
        if (self.RP_ToolDlg.is_active() == False):
            return

        # Update view based on new frame values
        line = int(float(self.RP_ToolDlg.entry_dict['start_line'].get_text()))
        pix = int(float(self.RP_ToolDlg.entry_dict['start_pix'].get_text()))
        sl =  int(float(self.RP_ToolDlg.entry_dict['num_lines'].get_text()))
        sp =  int(float(self.RP_ToolDlg.entry_dict['num_pix'].get_text()))

        # Find the view and layer
        cview = self.app.view_manager.get_active_view_window().title
        clayer = self.app.sel_manager.get_active_layer()
        if (cview is None) or (clayer is None):
            # Target can only be extracted if a view/layer is selected
            return
        if (sl == 0 or sp == 0):
            print "Trying to extract region with zero lines or zero pixels!"
            return

        try:
            filename = clayer.get_parent().get_dataset().GetDescription()
        except:
            gvutils.error('Unable to determine filename!')
            return

        try:
            target_data = gdalnumeric.LoadFile(filename,pix,line,sp,sl)
            target_ds = gdalnumeric.OpenArray(target_data)
            rl_mode_value = clayer.get_mode()
        except:
            gvutils.error('Unable to extract data and/or display mode info!')
            return

        if self.target_view_window is not None:
            # Need to delete old layer in target window and put in new one
            if self.target_view_layer is not None:
                self.target_view_window.viewarea.remove_layer(self.target_view_layer)
        # Create new layer
        self.target_view_data = target_data
        self.target_view_ds = target_ds
        self.target_view_layer = \
             gview.GvRasterLayer(gview.GvRaster(dataset = self.target_view_ds,real=1),
                                 rl_mode = rl_mode_value)

        if ((rl_mode_value == gview.RLM_RGBA) and (self.target_view_ds.RasterCount > 2)):
            green_raster = gview.GvRaster(dataset = self.target_view_ds,real=2)
            blue_raster = gview.GvRaster(dataset = self.target_view_ds,real=3)

            self.target_view_layer.set_source(1,green_raster)
            self.target_view_layer.set_source(2,blue_raster)

            if self.target_view_ds.RasterCount > 3:
                band = self.target_view_ds.GetRasterBand(4)
                if band.GetRasterColorInterpretation() == gdal.GCI_AlphaBand:
                    self.target_view_layer.blend_mode_set( gview.RL_BLEND_FILTER )
                    alpha_raster = gview.GvRaster(dataset = self.target_view_ds,real=4) 
                    self.target_view_layer.set_source(3,alpha_raster)

        self.target_view_layer.set_name(filename)
        refresh_old_window = 0           # redraw roi first time
        if self.target_view_window is None:
            # Need to create a window to display the current target in
            import gvviewwindow
            self.target_view_window = gvviewwindow.GvViewWindow(self.app,
                 title=self.target_view_title,show_menu = 0, show_icons=0,
                 show_tracker=1,show_scrollbars=1)

            self.target_view_window.connect('destroy',self.close_target_view_cb)
            refresh_old_window = 1

        self.target_view_window.viewarea.add_layer(self.target_view_layer)
        self.target_view_window.show()

        # The roi must be redrawn on the overview if a new window was created
        # (it will have disappeared)
        # HACK!!! 
        if refresh_old_window == 1:
            view_list = self.app.view_manager.get_views()
            for item in view_list:
                if (item.title == cview):
                    self.app.view_manager.set_active_view(item)
                    item.viewarea.set_active_layer(clayer)
                    self.app.toolbar.roi_tool.append((pix,line,sp,sl))

    def analyze_cb(self,*args):
        # Find the view and layer
        cview = self.app.view_manager.get_active_view_window().title
        clayer = self.app.sel_manager.get_active_layer()
        if (cview is None) or (clayer is None):
            # analysis only makes sense in the context of a view and layer
            gvutils.error('No View/Layer selected!')
            return

        text = self.basic_region_analysis(cview,clayer) 

        self.RP_ToolDlg.update_regioninfo_text(text)
        if (self.RP_ToolDlg.button_dict['Log To File'].get_active() == True):
            log_filename = self.RP_ToolDlg.entry_dict['log_file'].get_text()
            if (len(log_filename) < 2):
                gvutils.error('Log file not set!')
            else:
                try:
                    logfile = open(log_filename,'a')
                    logfile.writelines(text + '\n\n')
                    logfile.close()
                except:
                    err_txt = 'Unable to create or append to ' + log_filename + '.'
                    gvutils.error(err_txt)

    def basic_region_analysis(self,cview,clayer):
        line = self.RP_ToolDlg.entry_dict['start_line'].get_text()
        pix = self.RP_ToolDlg.entry_dict['start_pix'].get_text()
        sl =  self.RP_ToolDlg.entry_dict['num_lines'].get_text()
        sp =  self.RP_ToolDlg.entry_dict['num_pix'].get_text()

        number_of_elements = Numeric.multiply.reduce(self.target_view_data.shape)
        if (self.is_patch_complex() == 1):
            # Calculate stats for absloute and power
            max  = Numeric.maximum.reduce(Numeric.ravel(abs(self.target_view_data)))
            mean = Numeric.add.reduce(Numeric.ravel(1.0*(abs(self.target_view_data))))/ \
                           (Numeric.multiply.reduce(self.target_view_data.shape)*1.)
            mean_power = Numeric.add.reduce(Numeric.ravel(Numeric.power(abs(self.target_view_data),2)))/ \
                           (Numeric.multiply.reduce(self.target_view_data.shape)*1.)
            min  = Numeric.minimum.reduce(Numeric.ravel(abs(self.target_view_data)))
            if(number_of_elements > 1):
                var  = (Numeric.add.reduce(Numeric.power(Numeric.ravel(abs(self.target_view_data)) \
                         -mean,2))) /(Numeric.multiply.reduce(self.target_view_data.shape)-1.)
                var_power =  (Numeric.add.reduce(Numeric.power(Numeric.ravel(Numeric.power(abs(self.target_view_data),2)) \
                         -mean_power,2))) /(Numeric.multiply.reduce(self.target_view_data.shape)-1.)
            else:
                var_power = 0
                var   = 0
        else:
            max  = Numeric.maximum.reduce(Numeric.ravel(self.target_view_data))
            mean = Numeric.add.reduce(Numeric.ravel(1.0*self.target_view_data))/ \
                           (Numeric.multiply.reduce(self.target_view_data.shape)*1.)
            mean_power = Numeric.add.reduce(Numeric.ravel(Numeric.power(self.target_view_data,2)))/ \
                           (Numeric.multiply.reduce(self.target_view_data.shape)*1.)
            min  = Numeric.minimum.reduce(Numeric.ravel(self.target_view_data))
            if(number_of_elements > 1):
                var  = (Numeric.add.reduce(Numeric.power(Numeric.ravel(self.target_view_data)-mean,2))) / \
                       (Numeric.multiply.reduce(self.target_view_data.shape)-1.)
                var_power = (Numeric.add.reduce(Numeric.power(Numeric.ravel(Numeric.power(self.target_view_data,2))-mean_power,2))) / \
                       (Numeric.multiply.reduce(self.target_view_data.shape)-1.)
            else:
                var_power = 0
                var   = 0


        disp_text = 'Region Attributes: \n'
        fname = clayer.get_parent().get_dataset().GetDescription()
        disp_text = disp_text + '\tFilename: ' + fname + '\n'
        disp_text = disp_text + '\tLines: ' + str(int(float(line))) + ' to '
        disp_text = disp_text + str(int(float(line))+int(float(sl))-1) + '\n'
        disp_text = disp_text + '\tPixels: ' + str(int(float(pix))) + ' to ' 
        disp_text = disp_text + str(int(float(pix))+int(float(sp))-1)+'\n'
        disp_text = disp_text + '\tRegion Maximum: ' + str(max) +  '\n'
        disp_text = disp_text + '\tRegion Mean: ' + str(mean) +  '\n'
        disp_text = disp_text + '\tRegion Minimum: ' + str(min) +  '\n'
        disp_text = disp_text + '\tRegion Variance: ' + str(var) +  '\n'
        disp_text = disp_text + '\tPower Statistics: '  +  '\n'
        disp_text = disp_text + '\t\tMean Power: ' + str(mean_power) +  '\n'
        disp_text = disp_text + '\t\tPower Variance: ' + str(var_power) +  '\n'
        disp_text = disp_text + '\n'
        return disp_text

    def close_target_view_cb(self,*args):
        self.target_view_layer = None
        self.target_view_window = None
        self.target_view_ds = None

    def is_patch_complex(self):
        data_typecode = self.target_view_data.typecode()
        if (data_typecode == Numeric.Complex):
            return 1
        elif (data_typecode == Numeric.Complex0):
            return 1
        elif (data_typecode == Numeric.Complex8):
            return 1  
        elif (data_typecode == Numeric.Complex16):
            return 1
        elif (data_typecode == Numeric.Complex32):
            return 1  
        elif (data_typecode == Numeric.Complex64):
            return 1
        else:
            return 0        


class Stats_ToolDlg(General_ROIToolDlg):
    def __init__(self):
        General_ROIToolDlg.__init__(self)

    def init_setup_window(self):
        self.set_title('Stats Tool')
        self.set_border_width(10)
        self.set_size_request(450,650)

    def init_customize_gui_panel(self):
        # Inherit all the usual stuff...
        General_ROIToolDlg.init_customize_gui_panel(self)

        # Add new frame with pixel info, keeping track of
        # the frame and text object...
        self.frame_dict['region_info_frame'] = gtk.Frame()
        self.show_list.append(self.frame_dict['region_info_frame'])

        pixel_vbox = gtk.VBox()
        self.show_list.append(pixel_vbox)
        self.frame_dict['region_info_frame'].add(pixel_vbox)

        pixel_scroll = gtk.ScrolledWindow()
        self.show_list.append(pixel_scroll)
        pixel_vbox.pack_start(pixel_scroll,expand = True)

        self.entry_dict['region_info_text'] = gtk.Text(gtk.TextBuffer())
        self.show_list.append(self.entry_dict['region_info_text'])
        self.entry_dict['region_info_text'].set_wrap_mode(gtk.WRAP_NONE)
        self.entry_dict['region_info_text'].set_editable(False)
        pixel_scroll.add(self.entry_dict['region_info_text'])
        self.entry_dict['region_info_text'].set_text('')


        # Add a frame with the log file options
        self.frame_dict['log_frame']=gtk.Frame()
        self.show_list.append(self.frame_dict['log_frame'])

        log_table = gtk.Table(2,4,False)
        self.show_list.append(log_table)
        self.frame_dict['log_frame'].add(log_table)

        log_table.set_border_width(5)
        log_table.set_col_spacings(5)
        log_table.set_col_spacing(1, 20)

        self.button_dict['Log To File'] = gtk.CheckButton('Log To File')
        self.show_list.append(self.button_dict['Log To File'])
        log_table.attach(self.button_dict['Log To File'], 0,1, 0, 1)

        self.button_dict['Select Log'] = gtk.Button('Select Log')
        self.show_list.append(self.button_dict['Select Log'])
        log_table.attach(self.button_dict['Select Log'], 3,4, 0, 1)

        log_label = gtk.Label('Log File (full path): ')
        log_label.set_alignment(0, 0.5)
        log_table.attach(log_label, 0,1, 1, 2)

        self.entry_dict['log_file'] = gtk.Entry()
        self.entry_dict['log_file'].set_editable(True)
        self.entry_dict['log_file'].set_size_request(400,25)
        self.entry_dict['log_file'].set_text('')
        log_table.attach(self.entry_dict['log_file'], 1,4, 1,2)

        self.main_panel.pack_start(self.frame_dict['region_info_frame'],True,True,0)   
        self.main_panel.pack_start(self.frame_dict['log_frame'],False,False,0)      

        # Customized connections
        self.button_dict['Select Log'].connect('clicked',self.select_log_cb)

        # Set default sensitivities for customized tool
        self.button_dict['Log To File'].set_active(False)
        self.button_dict['Log To File'].set_sensitive(True)
        self.button_dict['Select Log'].set_sensitive(True)

    def update_regioninfo_text(self,text):
        self.entry_dict['region_info_text'].set_text(text)

    def update_logfile_text(self,filepath,*args):
        self.entry_dict['log_file'].set_text(filepath)

    def set_entry_sensitivities(self,bool_val):
        self.entry_dict['start_line'].set_sensitive(bool_val)
        self.entry_dict['start_pix'].set_sensitive(bool_val)
        self.entry_dict['num_lines'].set_sensitive(bool_val)
        self.entry_dict['num_pix'].set_sensitive(bool_val)
        self.entry_dict['log_file'].set_sensitive(bool_val)

    def select_log_cb(self,*args):
        import pgufilesel
        pgufilesel.SimpleFileSelect( self.update_logfile_text, 
                                     None,
                                     'Select Log File',
                                     help_topic = 'files.html' )

    def activate_toggled(self,*args):
        if self.button_dict['Activate'].get_active():
            self.set_entry_sensitivities(True)
            self.button_dict['Auto Update'].set_sensitive(True)
            self.button_dict['Auto Update'].set_active(False)
            self.button_dict['Analyze'].set_sensitive(True)
            self.button_dict['Set Tool'].set_sensitive(True)
            self.button_dict['Log To File'].set_sensitive(True)
            self.button_dict['Log To File'].set_active(False)
            self.button_dict['Select Log'].set_sensitive(True)
            Signaler.notify(self, 're-activated')
        else:
            self.set_entry_sensitivities(False)
            self.button_dict['Auto Update'].set_active(False)
            self.button_dict['Auto Update'].set_sensitive(False)
            self.button_dict['Analyze'].set_sensitive(False)
            self.button_dict['Set Tool'].set_sensitive(False)
            self.button_dict['Log To File'].set_sensitive(False)
            self.button_dict['Log To File'].set_active(False)
            self.button_dict['Select Log'].set_sensitive(False)


