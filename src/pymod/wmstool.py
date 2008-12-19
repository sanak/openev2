# -*- coding: utf-8 -*-
##############################################################################
# wmstool.py, v2.0 2007/01/03
#
# Project:  OpenEV / CIETmap
# Purpose:  WMS Client
# Author:  Mario Beauchamp
#
# TODO: Cache management dialog. Better handling of exceptions.
###############################################################################
#
#  Revision 2.0 2007/01/03
#  GTK2 port
#
#  Revision 1.07b 2005/09/14
#  Added Help button.
#  Fixed for Python 2.2.
#
#  Revision 1.06b 2005/08/16
#  Fixed proxies support so it works with Python <2.3 as well.
#
#  Revision 1.05b 2005/06/15
#  Added support for http proxy.
#  Changed datatype in vrt files from 'Byte' to datatype of returned images.
#    This will ensure correct interpretation of elevation data, for example.
#  Service exception reports now displayed.
#
#  Revision 1.04b 2005/03/30
#  Fixed failure to return the proper SRS in getSRS().
#
#  Revision 1.03b 2005/03/26
#  Corrected problem when saving a new map.
#
#  Revision 1.02b 2005/03/25
#  Fixed a problem when deleting a map or saving an existing one after edits.
#  Changed the name of the default map to "landsat321" instead of "Landsat321".
#  Added a way to put the tool in the icon bar.
#  Added a way to set a more suitable background color.
#
#  Revision 1.01b 2005/03/25
#  Corrected a few bugs. Added way to select layers without opening Props window.
#  Added ServiceCaps.validateURL().
#
#  Revision 1.00b 2005/03/24
#  Beta release.
#
###############################################################################
import gtk
import pgu
import gobject
import os
from osgeo import osr
import vrtutils
from osgeo import gdal
if 'CIETMAP_HOME' in os.environ:
    import cview as gview
    import cietutils as gvutils
else:
    import gview
    import gvutils
import pickle
import urllib
from gvsignaler import Signaler
import pgucolor

wms_dir = os.path.join(gview.home_dir, 'wms')
cachepath = os.path.join(wms_dir, 'cache')
srv_dir = os.path.join(wms_dir, 'services')

global servDic, encoding

# set desired proxy in prefs file (.openev) as such:
# http_proxy=http://170.222.120.200:8000/
proxy = gview.get_preference('http_proxy')
if proxy != None:
    proxyDic = {'http':proxy}
else:
    proxyDic = None

def loadServices():
    global servDic
    servDic = {}
    hosts = os.listdir(srv_dir)
    for host in hosts:
        srvnames = os.listdir(os.path.join(srv_dir,host))
        for name in srvnames:
            servDic[name[:-4]] = host

def openService(srvname):
    host = servDic.get(srvname)
    if not host:
        return False
    path = os.path.join(srv_dir,host, srvname+'.xml')
    if servCaps.open(path):
        # we don't need the buffer here
        del servCaps.buffer
        servCaps.buffer = None
        return True
    else: # unlikely but just in case...
        gvutils.error(path+" does not appear to be a valid GetCapabilities response.")
        return False

def createCapsEntry(srvname, path=srv_dir):
    host,p = urllib.splithost(urllib.splittype(servCaps.href)[1])
    hname = host.split(':')[0]
    newhost = os.path.join(path,hname)

    if not os.path.exists(newhost):
        os.mkdir(newhost)

    newcaps = os.path.join(newhost,srvname+'.xml')
    fd = file(newcaps,'w')
    fd.write(servCaps.buffer)
    fd.close()
    return hname
    
def buildServicesDir(lst, path=srv_dir):
    for line in lst:
        if not line.startswith('#'):
            srvname,url = line.split(',')
            if servCaps.open(url):
                if createCapsEntry(srvname,path) is None:
                    print "Service %s not saved" % srvname
                    outfile = os.path.join(path,srvname+'-tmp.xml')
                    fd = file(outfile,'w')
                    fd.write(servCaps.buffer)
                    fd.close()
                else:
                    print "Service %s saved" % servCaps.title

                del servCaps.buffer
                servCaps.buffer = None
            else:
                print "Service %s not saved" % srvname
                if servCaps.buffer:
                    print "Server responded:\n%s" % servCaps.buffer
                    del servCaps.buffer
                    servCaps.buffer = None
                else:
                    print "Server %s did not respond." % srvname

def fixCaps(xmlStr):
    # fix for unsupported stuff
    idx = xmlStr.find('[')
    if idx > 0:
        idx2 = xmlStr.find(']>') + 1
        return xmlStr.replace(xmlStr[idx:idx2],'')
##    # fix for encoding
##    hdr = xmlStr.split('\n',1)[0]
##    tree = gdal.ParseXMLString(hdr)
##    hdr_elem = makeElement(tree)
##    encoding = hdr_elem.get('encoding')
##    return xmlStr.decode(encoding)
    return xmlStr

def Element(tag, attrib={}, **extra):
    attrib = attrib.copy()
    attrib.update(extra)
    return _ElementInterface(tag, attrib)

def makeElement(node):
    if hasattr(node, 'tag'):
        return node
    attrib = {}
    tag = ''
    text = ''
    children = []
    element = None
    if node[0] == gdal.CXT_Element:
        tag = node[1]
        for subnode in node[2:]:
            if subnode[0] == gdal.CXT_Attribute:
                attrib[subnode[1]] = subnode[2][1]
            elif subnode[0] == gdal.CXT_Element:
                children.append(subnode)
            elif subnode[0] == gdal.CXT_Text:
                text = subnode[1]

        element = Element(tag,attrib)
        element.text = text
        element._children = children

    return element

class _ElementInterface:
    tag = None
    attrib = None
    text = None
    def __init__(self, tag, attrib):
        self.tag = tag
        self.attrib = attrib
        self._children = []

    def __getitem__(self, index):
        return self._children[index]

    def __len__(self):
        return len(self._children)

    def _find(self, path, all=0):
        foundLst=[]
        splitted = path.split('/',1)
        if len(splitted) == 2:
            component, remainder = splitted
        else:
            component, remainder = splitted[0], None
        for subnode in self._children:
            if subnode[1] == component:
                if remainder is None:
                    foundLst.append(subnode)
                    if not all:
                        break
                else:
                    subElem = makeElement(subnode)
                    subLst = subElem._find(remainder,all)
                    if subLst is not None:
                        if all == 1:
                            foundLst.extend(subLst)
                        else:
                            foundLst.append(subLst)
 
        if len(foundLst) == 0:
            return None
        elif all == 1:
            return foundLst
        else:
            return foundLst[0]

    def append(self, element):
        self._children.append(element)

    def get(self, key, default=None):
        return self.attrib.get(key,default)

    def find(self, path):
        result = self._find(path)
        if result is None:
            return

        return makeElement(result)

    def findall(self, path):
        result = self._find(path, all=1)
        if result is None:
            return

        foundLst = []
        for child in result:
            foundLst.append(makeElement(child))

        return foundLst

    def findtext(self, path, default=None):
        result = self.find(path)
        if result is None:
            return default

        return result.text.decode(encoding)

    def getiterator(self, tag=None):
        nodes = []
        for node in self._children:
            elem = makeElement(node)
            if tag is None or elem.tag == tag:
                nodes.append(elem)
        return nodes

    def items(self):
        return self.attrib.items()

    def keys(self):
        return self.attrib.keys()

class ElementTree:
    def __init__(self, element=None):
        self._root = element # first node

    def getroot(self):
        return self._root

    def parse(self, rawXML):
        global encoding
        rawXML = fixCaps(rawXML)
        try:
            tree = gdal.ParseXMLString(rawXML)
        except:
            return None

        element = makeElement(tree[len(tree)-1])
        if element is None:
            return None

        self._root = element
        xmldesc = makeElement(tree[2])
        encoding = xmldesc.get('encoding')

        return rawXML

    def find(self, path):
        return self._root.find(path)

    def findall(self, path):
        return self._root.findall(path)

    def findtext(self, path, default=None):
        return self._root.findtext(path, default)

    def getiterator(self, tag=None):
        return self._root.getiterator(tag)

class ServiceCaps(ElementTree):
    def __init__(self):
        ElementTree.__init__(self)
        self.title = None
        self.buffer = None

    def open(self, url):
        del self._root
        self._root = None
        capsBuf = self.getCapabilities(url)
        # checks
        if capsBuf is None:
            self.buffer = ''
            return False
        self.buffer = self.parse(capsBuf)
        if self._root is None:
            return False
        if self._root.tag not in ('WMT_MS_Capabilities','WMS_Capabilities'):
            return False

        self.version = self._root.get('version')
        servTree = self.find('Service')
        self.name = servTree.findtext('Name')
        self.abstract = servTree.findtext('Abstract')
        self.title = servTree.findtext('Title')
        self.layer = self.find('Capability/Layer')
        self.layer.title = self.layer.findtext('Title')
        res = self.find('Capability/Request/GetMap/DCPType/HTTP/Get/OnlineResource')
        self.href = res.get('xlink:href')
        if not '?' in self.href:
            self.href += '?'
        # we got here so everything is ok.
        return True

    def getCapabilities(self, url):
        type,path = urllib.splittype(url)
        if path.endswith('?'):
            # 1.1.x only
            url += 'VERSION=1.1&SERVICE=WMS&REQUEST=GetCapabilities'
        if type == 'http':
            try:
                opener = urllib.URLopener(proxies=proxyDic)
                fp = opener.open(url)
            except:
                return None
        else:
            try:
                fp = file(url,'r')
            except:
                return None

        return fp.read()

    def getTitles(self, tree):
        keys = []
        for node in tree:
            keys.append(node.findtext('Title'))

        keys.sort()
        return keys

    def getLayerNode(self, title=None, name=None):
        # check if we have the root layer
        if name is None:
            if title == self.layer.title:
                return self.layer
        else:
            layname = self.layer.findtext('Name')
            if layname is not None and layname == name:
                return self.layer

        # else iterate the root tree
        rootTree = self.layer.findall('Layer')
        return self.findLayer(rootTree, title, name)

    def findLayer(self, tree, title, name):
        if name is None:
            laynode = self.findByTitle(tree, title)
        else:
            laynode = self.findByName(tree, name)
        if laynode is not None:
            return laynode
        for layer in tree:
            if layer.find('Layer') is not None:
                layerTree = layer.findall('Layer')
                laynode = self.findLayer(layerTree, title, name)
                if laynode is not None:
                    return laynode

    def findByTitle(self, tree, title):
        for node in tree:
            ntitle = node.findtext('Title')
            if ntitle == title:
                return node

    def findByName(self, tree, name):
        for node in tree:
            nname = node.findtext('Name')
            if nname == name:
                return node

    def getPropList(self, path):
        propTree = self.findall(path)
        propLst = []
        for prop in propTree:
            propLst.append(prop.text)

        propLst.sort()
        return propLst

    def getImageFormats(self):
        frmts = []
        # filter formats we do not support (yet)
        for frmt in self.getPropList('Capability/Request/GetMap/Format'):
            if 'application' not in frmt and 'xml' not in frmt:
                frmts.append(frmt)

        return frmts

    def getExceptFormats(self):
        frmts = []
        for frmt in self.getPropList('Capability/Exception/Format'):
            excep = self.getExceptString(frmt)
            frmts.append(excep)

        return frmts

    def getExceptString(self, excep):
        if excep.startswith('application'):
            ch = '_'
        elif excep.startswith('text'):
            ch = '/'

        return excep.split(ch)[1]

    def getSRSList(self, tree):
        srsLst = []
        srsTree = tree.find('SRS')
        if srsTree is None:
            return

        lst = srsTree.text.split(' ')
        if len(lst) == 1:
            for srsElem in tree.findall('SRS'):
                srsLst.append(srsElem.text)
        else:
            for srsTxt in lst:
                srsLst.append(srsTxt)

        srsLst.sort()
        return srsLst

    def getException(self, excep):
        if excep in ('html','plain'):
            return 'text/' + excep
        else:
            return 'application/vnd.ogc.se_' + excep

    def sortLayerTree(self, laynode):
        titles = []
        layerTree = laynode.getiterator('Layer')
        for node in layerTree:
            titles.append(node.findtext('Title'))
        
        titles.sort()
        sorted = []
        for title in titles:
            sorted.append(self.findByTitle(layerTree, title))

        return sorted

    def encodeURL(self, parmDic, url=None):
        parms = {}
        if url is None:
            url = self.href

        for parm,value in parmDic.items():
            if parm == 'STYLES' or parm == 'map':
                pass
            elif parm == 'LAYERS':
                layerLst = []
                styleLst = []
                for title in parmDic['LAYERS']:
                    node = self.getLayerNode(title)
                    name = node.findtext('Name')
                    layerLst.append(name)
                    idx = parmDic['LAYERS'].index(title)
                    stitle = parmDic['STYLES'][idx]
                    if len(stitle):
                        style = self.findByTitle(node.findall('Style'), stitle)
                        sname = style.findtext('Name')
                    else:
                        sname = ''
                    styleLst.append(sname)
                parms['LAYERS'] = ','.join(layerLst)
                parms['STYLES'] = ','.join(styleLst)
            elif type(value) == type([]):
                parms[parm] = ','.join(parmDic[parm])
            else:
                parms[parm] = parmDic[parm]

        return url + urllib.urlencode(parms)

    def decodeURL(self, url):
        splitLst = url.split('?')
        parmLst = splitLst[1].split('&')
        parmDic = {}
        for parmStr in parmLst:
            parmLst = parmStr.split('=')
            parmDic[parmLst[0]] = urllib.unquote_plus(parmLst[1])
        
        layerLst = parmDic['LAYERS'].split(',')
        styleLst = parmDic['STYLES'].split(',')
        laytitles = []
        stytitles = []
        for name in layerLst:
            node = self.getLayerNode(name=name)
            title = node.findtext('Title')
            laytitles.append(title)
            idx = layerLst.index(name)
            if len(styleLst[idx]):
                style = self.findByName(node.findall('Style'), styleLst[idx])
                stitle = style.findtext('Title')
            else:
                stitle = ''
            stytitles.append(stitle)
            
        parmDic['LAYERS'] = laytitles
        parmDic['STYLES'] = stytitles

        return parmDic

    def validateSRS(self, srs):
        srsLst = self.getSRSList(self.layer)
        for srsTxt in srsLst:
            if srsTxt == srs:
                return True

        return False

    def validateURL(self, url):
        try:
            params = self.decodeURL(url)
        except:
            return "Invalid URL"

    def getMap(self, url):
        opener = urllib.URLopener(proxies=proxyDic)
        f = opener.open(url)
        h = f.info()
        if h.getheader('Content-Type') == 'application/vnd.ogc.se_xml':
            print "Exception occured"
        return f

servCaps = ServiceCaps()
tips = gtk.Tooltips()

class WMSDialog(gtk.Window):
    def __init__(self, app):
        gtk.Window.__init__(self)
        self.set_title("WMS Tool")
        self.app = app
        self.set_resizable(False)
        self.updating = True
        loadServices()
        self.curServ = None
        self.loadMaps()
        self.setMap(self.mapKeys[0])
        self.rezX = 1.0
        self.rezY = 1.0
        self.extentBX = None
        self.viewwin = self.app.view_manager.get_active_view_window()
        self.srs = self.getSRS()
        # uncomment this line if you want to set the view's BG color to yellow
##        self.viewwin.viewarea.set_background_color((1,1,0.8,1))
        self.display_change_id = self.viewwin.viewarea.connect('view-state-changed', self.viewChanged)
        self.active_changed_id = self.viewwin.viewarea.connect('active-changed', self.layerChanged)

        if self.createGUI():
            self.updateGUI()
            self.updateExtentGUI()
            self.updating = False
            self.show_all()

    def open_file(self, filename):
        if 'CIETMAP_HOME' in os.environ:
            gvutils.file_open_by_name(filename, self.viewwin.viewarea)
        else:
            self.viewwin.open_gdal_dataset(gdal.Open(filename))

    def createGUI(self):
        mainbox = gtk.VBox(spacing=5)
        mainbox.set_border_width(5)
        self.add(mainbox)

        frame = gtk.Frame("Map")
        mainbox.pack_start(frame, expand=False)

        # Map ctrl
        hbox = gtk.HBox(spacing=10)
        hbox.set_border_width(5)
        frame.add(hbox)

        self.mapCB = pgu.ComboText(strings=self.mapKeys, action=self.mapSelected)
        self.mapCB.set_size_request(203,-1)
        hbox.pack_start(self.mapCB, expand=False)

        but = gvutils.create_stock_button(gtk.STOCK_PREFERENCES, self.setup)
        but.set_size_request(32,32)
        tips.set_tip(but,"Setup maps and services")
        hbox.pack_start(but, expand=False)

        but = gvutils.create_stock_button(gtk.STOCK_NETWORK, self.addLayer)
        but.set_size_request(32,32)
        tips.set_tip(but,"Get map from server")
        hbox.pack_start(but, expand=False)

        self.colorBT = pgucolor.ColorButton()
        tips.set_tip(self.colorBT,"Map background color")
        hbox.pack_start(self.colorBT, expand=False)

        # Map params
        mainbox.pack_start(self.createCornersGUI(), expand=False)
        # Resolution
        mainbox.pack_start(self.createResolutionGUI(), expand=False)
        # Size
        mainbox.pack_start(self.createSizeGUI(), expand=False)

        # Extent
        frame = gtk.Frame("Extent")
        mainbox.pack_start(frame, expand=False)

        self.extentBX = gtk.HBox(spacing=5)
        self.extentBX.set_border_width(5)
        frame.add(self.extentBX)

        # Controls
        frame = gtk.Frame("Controls")
        mainbox.pack_start(frame, expand=False)

        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(5)
        frame.add(vbox)

        hbox = gtk.HBox(spacing=5)
        vbox.pack_start(hbox, expand=False)

        self.autoTO = gtk.CheckButton(label="Update from")
        self.autoTO.set_active(True)
        self.autoTO.connect('toggled', self.autoUpdateChanged)
        hbox.pack_start(self.autoTO, expand=False)

        self.viewModeRB = gtk.RadioButton(label="view")
        self.viewModeRB.set_active(True)
        self.viewModeRB.connect('toggled', self.autoUpdateChanged)
        hbox.pack_start(self.viewModeRB, expand=False)

        self.layerModeRB = gtk.RadioButton(label="layer", group=self.viewModeRB)
        self.layerModeRB.connect('toggled', self.autoUpdateChanged)
        hbox.pack_start(self.layerModeRB, expand=False)

        self.epsgTE = gtk.Entry()
        self.epsgTE.set_size_request(50,20)
        self.epsgTE.set_text(self.getEPSG())
        self.epsgTE.connect('activate', self.epsgChanged)
        hbox.pack_end(self.epsgTE, expand=False)
        hbox.pack_end(gtk.Label("EPSG:"), expand=False)

        bbox = gtk.HBox(spacing=5)
        vbox.pack_start(bbox, expand=False)

        but = gtk.Button("Refresh Services")
        but.connect('clicked', self.refreshServices)
        tips.set_tip(but,"Refresh all services in services.txt")
        bbox.add(but)
        
        but = gtk.Button("Validate Map")
        but.connect('clicked', self.validateMap)
        tips.set_tip(but,"Validate map parameters. Only SRS is currently checked.")
        bbox.add(but)

        but = gtk.Button("World Map")
        but.connect('clicked', self.loadWorld)
        tips.set_tip(but,"Load a MODIS World Map")
        bbox.add(but)
        
        # Buttons
        bbox = gtk.HButtonBox()
        mainbox.pack_start(bbox, expand=False)

        but = gtk.Button(stock=gtk.STOCK_HELP)
        but.connect('clicked', self.help)
        bbox.add(but)

        but = gtk.Button(stock=gtk.STOCK_CLOSE)
        but.connect('clicked', self.close)
        bbox.add(but)

        return True

    def createCornersGUI(self):
        frame = gtk.Frame("Corners")
        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(5)
        frame.add(vbox)

        # create entries
        isgeo = self.srs.IsGeographic()
        self.ulxGE = GeoEntry(isgeo)
        self.ulyGE = GeoEntry(isgeo)
        self.lrxGE = GeoEntry(isgeo)
        self.lryGE = GeoEntry(isgeo)

        # Upper left corner
        hbox = gtk.HBox(spacing=5)
        vbox.pack_start(hbox, expand=False)

        hbox.pack_start(gtk.Label("UL "), expand=False)
        hbox.pack_start(gtk.Label("X:"), expand=False)
        self.ulxGE.connect('changed', self.updateFromEntry, 'ulx')
        hbox.pack_start(self.ulxGE, expand=False)

        self.ulyGE.connect('changed', self.updateFromEntry, 'uly')
        hbox.pack_end(self.ulyGE, expand=False)
        hbox.pack_end(gtk.Label("Y:"), expand=False)

        # Lower right corner
        hbox = gtk.HBox(spacing=5)
        vbox.pack_start(hbox, expand=False)

        hbox.pack_start(gtk.Label("LR "), expand=False)
        hbox.pack_start(gtk.Label("X:"), expand=False)
        self.lrxGE.connect('changed', self.updateFromEntry, 'lrx')
        hbox.pack_start(self.lrxGE, expand=False)

        self.lryGE.connect('changed', self.updateFromEntry, 'lry')
        hbox.pack_end(self.lryGE, expand=False)
        hbox.pack_end(gtk.Label("Y:"), expand=False)

        return frame

    def createResolutionGUI(self):
        frame = gtk.Frame("Resolution")
        table = gtk.Table()
        table.set_col_spacings(5)
        table.set_border_width(5)
        frame.add(table)

        # create entries
        isgeo = self.srs.IsGeographic()
        self.rezxGE = GeoEntry(isgeo, dgsz=15, scsz=50, mtsz=90)
        self.rezxGE.setRound(3)
        self.rezxGE.connect('changed', self.setResolution, 'rezX')
        self.rezyGE = GeoEntry(isgeo, dgsz=15, scsz=50, mtsz=90)
        self.rezyGE.connect('changed', self.setResolution, 'rezY')
        self.rezyGE.setRound(3)

        table.attach(pgu.Label("X:"), 0, 1, 0, 1, xoptions=gtk.SHRINK)
        table.attach(self.rezxGE, 1, 2, 0, 1, xoptions=gtk.SHRINK)

        table.attach(gtk.Label("--"), 2, 3, 0, 1)
        self.linkTO = gtk.CheckButton()
        self.linkTO.set_active(True)
        table.attach(self.linkTO, 3, 4, 0, 1, xoptions=gtk.SHRINK)
        table.attach(gtk.Label("--"), 4, 5, 0, 1)

        table.attach(pgu.Label("Y:"), 5, 6, 0, 1, xoptions=gtk.SHRINK)
        table.attach(self.rezyGE, 6, 7, 0, 1, xoptions=gtk.SHRINK)

        return frame

    def createSizeGUI(self):
        frame = gtk.Frame("Size")
        table = gtk.Table()
        table.set_row_spacings(5)
        table.set_col_spacings(5)
        table.set_border_width(5)
        frame.add(table)

        table.attach(pgu.Label("Width: "), 0, 1, 0, 1, xoptions=gtk.SHRINK)
        self.wTE = gtk.Entry()
        self.wTE.set_size_request(50,24)
        self.wTE.connect('changed', self.updateFromEntry, 'w')
        table.attach(self.wTE, 1, 2, 0, 1, xoptions=gtk.SHRINK)
        table.attach(pgu.Label("px"), 2, 3, 0, 1)

        isgeo = self.srs.IsGeographic()
        self.xGE = GeoEntry(isgeo)
        self.xGE.connect('changed', self.updateFromEntry, 'x')
        table.attach(self.xGE, 3, 4, 0, 1, xoptions=gtk.SHRINK)

        table.attach(pgu.Label("Height:"), 0, 1, 1, 2, xoptions=gtk.SHRINK)
        self.hTE = gtk.Entry()
        self.hTE.set_size_request(50,24)
        self.hTE.connect('changed', self.updateFromEntry, 'h')
        table.attach(self.hTE, 1, 2, 1, 2, xoptions=gtk.SHRINK)
        table.attach(pgu.Label("px"), 2, 3, 1, 2)

        self.yGE = GeoEntry(isgeo)
        self.yGE.connect('changed', self.updateFromEntry, 'y')
        table.attach(self.yGE, 3, 4, 1, 2, xoptions=gtk.SHRINK)

        return frame

    def updateExtentGUI(self):
        for child in self.extentBX.get_children():
            child.unrealize()
            child.destroy()

        if 'Extents' in self.curMap:
            vbox = gtk.VBox(spacing=3)
            self.extentBX.add(vbox)
            for parm,value in self.curMap['Extents'].iteritems():
                box = gtk.HBox(spacing=2)
                vbox.pack_start(box, expand=False)
                box.pack_start(gtk.Label(parm+": "), expand=False)
                box.pack_start(gtk.Label(value), expand=False)
            but = gvutils.create_stock_button(gtk.STOCK_EDIT, self.editExtentClicked)
            self.extentBX.pack_start(but, expand=False)
        else:
            self.extentBX.pack_start(gtk.Label("No extents defined"), expand=False)

        self.extentBX.show_all()

    def colorClicked(self, but):
        self.bgColor = but.get_color()

    def help(self, but):
##        from gvhtml import LaunchHTML
##        LaunchHTML('http://pages.infinit.net/starged/openev/wmstool/home.htm')
        gvutils.warning("Help not available in this implementation")

    def layerChanged(self, view):
        if not self.autoTO.get_active() or self.updating:
            return

        layer = view.active_layer()
        if layer is None or not isinstance(layer, gview.GvRasterLayer):
            return

        srs = self.getSRS()
        if not srs.IsSame(self.srs):
            self.srs = srs
            self.epsgTE.set_text(self.getEPSG())
            self.updateModeGUI()

        self.updateGUI()

    def viewChanged(self, view):
        if not self.autoTO.get_active() or self.updating or self.layerModeRB.get_active():
            return

        self.updateGUI()

    def editExtentClicked(self, *args):
        self.showEditExtentGUI()

    def mapSelected(self, combo):
        if self.updating:
            return

        mapKey = combo.get_active_text()
        if mapKey:
            self.setMap(mapKey)
            self.updateExtentGUI()

    def extentChanged(self, entry, name):
        if self.updating:
            return
        self.curMap['Extents'][name] = entry.get_text()

    def autoUpdateChanged(self, *args):
        if not self.autoTO.get_active():
            return

        if self.layerModeRB.get_active():
            layer = self.viewwin.viewarea.active_layer()
            if layer is None or not isinstance(layer, gview.GvRasterLayer):
                return

        srs = self.getSRS()
        if not srs.IsSame(self.srs):
            self.srs = srs
            self.epsgTE.set_text(self.getEPSG())
            self.updateModeGUI()

        self.updateGUI()

    def epsgChanged(self, entry):
        espgTxt = entry.get_text()
        self.srs = self.getSRS(int(espgTxt))
        self.updateModeGUI()
        self.updateGUI()

    def showEditExtentGUI(self):
        for child in self.extentBX.get_children():
            child.unrealize()
            child.destroy()

        parms = servCaps.decodeURL(self.curMap['URL'])
        layerNode = servCaps.getLayerNode(parms['LAYERS'][0])
        extentTree = layerNode.findall('Extent')
        vbox = gtk.VBox(spacing=3)
        self.extentBX.pack_start(vbox, expand=False)

        for extent in extentTree:
            name = extent.get('name')
            extentTE = ExtentEntry(extent)
            extentTE.connect('changed', self.extentChanged, name)
            vbox.pack_start(extentTE, expand=False)
            value = extent.text
            if name in self.curMap['Extents']:
                value = self.curMap['Extents'][name]
            extentTE.entry.set_text(value)

        self.extentBX.show_all()

    def updateGUI(self, *args):
        self.updating = True
        if self.viewModeRB.get_active():
            params = self.getInfoFromView()
        elif self.layerModeRB.get_active():
            params = self.getInfoFromLayer()
        else:
            return

        self.ulxGE.setValue(params[0])
        self.ulyGE.setValue(params[1])
        self.lrxGE.setValue(params[2])
        self.lryGE.setValue(params[3])
        self.rezxGE.setValue(self.rezX)
        self.rezyGE.setValue(abs(self.rezX))
        self.wTE.set_text(str(params[4]))
        self.hTE.set_text(str(params[5]))
        self.xGE.setValue(params[6])
        self.yGE.setValue(params[7])
        self.updating = False

    def updateModeGUI(self):
        self.updating = True
        isgeo = self.srs.IsGeographic()
        self.ulxGE.setMode(isgeo)
        self.ulyGE.setMode(isgeo)
        self.lrxGE.setMode(isgeo)
        self.lryGE.setMode(isgeo)
        self.rezxGE.setMode(isgeo)
        self.rezyGE.setMode(isgeo)
        self.xGE.setMode(isgeo)
        self.yGE.setMode(isgeo)
        self.updating = False

    def updateFromEntry(self, entry, id):
        if self.updating:
            return

        self.updating = True
        rezX = self.rezX
        rezY = self.rezY
        if id == 'lrx':
            lrx = self.lrxGE.getValue()
            ulx = self.ulxGE.getValue()
            x = abs(ulx - lrx)
            self.xGE.setValue(x)
            w = int(x/rezX)
            self.wTE.set_text(str(w))
        elif id == 'lry':
            lry = self.lryGE.getValue()
            uly = self.ulyGE.getValue()
            y = abs(uly - lry)
            self.yGE.setValue(y)
            h = int(y/abs(rezY))
            self.hTE.set_text(str(h))
        elif id in ('w','ulx'):
            txt = self.wTE.get_text()
            if not txt: return
            w = int(txt)
            x = w * rezX
            self.xGE.setValue(x)
            ulx = self.ulxGE.getValue()
            lrx = ulx + x
            self.lrxGE.setValue(lrx)
        elif id in ('h','uly'):
            txt = self.hTE.get_text()
            if not txt: return
            h = int(txt)
            y = h * rezY
            self.yGE.setValue(abs(y))
            uly = self.ulyGE.getValue()
            lry = uly + y
            self.lryGE.setValue(lry)
        elif id == 'x':
            x = self.xGE.getValue()
            w = int(x/rezX)
            self.wTE.set_text(str(w))
            ulx = self.ulxGE.getValue()
            lrx = ulx + x
            self.lrxGE.setValue(lrx)
        elif id == 'y':
            y = self.yGE.getValue()
            h = int(y/abs(rezY))
            self.hTE.set_text(str(h))
            uly = self.ulyGE.getValue()
            lry = uly - y
            self.lryGE.setValue(lry)

        self.updating = False

    def getInfoFromView(self):
        view = self.viewwin.viewarea
        bbox = view.get_extents()
        w = view.get_width()
        h = view.get_height()
        ulx,lry,lrx,uly = bbox
        dx = abs(ulx - lrx)
        dy = abs(uly - lry)
        self.rezX = dx/w
        self.rezY = -dy/h
        return (ulx,uly,lrx,lry,w,h,dx,dy)

    def getInfoFromLayer(self):
        layer = self.viewwin.viewarea.active_layer()
        if layer is None:
            return
        ds = layer.parent.get_dataset()
        w = ds.RasterXSize
        h = ds.RasterYSize
        geoTr = ds.GetGeoTransform()
        ulx = geoTr[0]
        uly = geoTr[3]
        self.rezX = geoTr[1]
        self.rezY = geoTr[5]
        lrx = ulx + w*self.rezX
        lry = uly + h*self.rezY
        dx = abs(ulx - lrx)
        dy = abs(uly - lry)
        return (ulx,uly,lrx,lry,w,h,dx,dy)

    def addLayer(self, *args):
        mapfn = self.checkCache(self.curMap['Name'])
        self.updateMapURL()

        wmsFile = servCaps.getMap(self.curMapURL)
        h = wmsFile.info()
##        print wmsFile, h
        if h.getheader('Content-Type') == 'application/vnd.ogc.se_xml':
##            wmsFile = file('c:\\FWTools\\wms\\cache\\landsat321_23-exception.xml')
            excepStr = ''
            for line in wmsFile:
                if '<ServiceException>' in line:
                    excepStr += wmsFile.next()
##                print line
            wmsFile.close()
            gvutils.error("Error getting map. Server responded:\n\n" + excepStr)
##            excepFile = open(mapfn+'-exception.xml','wb')
##            excepFile.write(buf)
##            excepFile.close()
            return
            
        layerFile = open(mapfn+'.tmp','wb')
        layerFile.write(wmsFile.read())
        layerFile.close()
        wmsFile.close()

        ds = gdal.Open(mapfn+'.tmp', gdal.GA_ReadOnly)
        if ds is None:
            gvutils.error("Error opening image")
            return

        w = ds.RasterXSize
        h = ds.RasterYSize
        mapVrt = vrtutils.VRTDatasetConstructor(w, h)

        mapVrt.SetSRS(self.srs.ExportToWkt())
        mapVrt.SetGeoTransform(self.getGeoTransform())

        nBands = ds.RasterCount
        fname = ds.GetDescription()
        if nBands == 1:
            band = ds.GetRasterBand(1)
            nodata = band.GetNoDataValue()
            ci = band.GetRasterColorInterpretation()
            dtype = gdal.GetDataTypeName(band.DataType)
            if ci == gdal.GCI_PaletteIndex:
                ct = band.GetRasterColorTable()
                mapVrt.AddSimpleBand(fname, 1, dtype, NoDataValue=nodata, ColorInterp='Palette', colortable=ct)
            else:
                mapVrt.AddSimpleBand(fname, 1, dtype, NoDataValue=nodata, ColorInterp='Grey')
        else:
            for bandno in range(1, nBands+1):
                band = ds.GetRasterBand(bandno)
                nodata = band.GetNoDataValue()
                dtype = gdal.GetDataTypeName(band.DataType)
                mapVrt.AddSimpleBand(fname, bandno, dtype, NoDataValue=nodata)

        mapFile = open(mapfn+'.vrt','w')
        mapFile.writelines(mapVrt.GetVRTString())
        mapFile.close()

        self.open_file(mapfn+'.vrt')

    def checkCache(self, mapname):
        count = 1
        filtLst = filter(lambda n:n.startswith(mapname) and n.endswith('vrt'),
                         os.listdir(cachepath))
        inLst = True
        while inLst:
            name = '%s_%s' % (mapname, count)
            if name+'.vrt' in filtLst:
                count += 1
            else:
                inLst = False

        return os.path.join(cachepath, name)

    def setup(self, *args):
        SetupDialog(self)

    def loadMaps(self):
        try:
            maps = pickle.load(open(os.path.join(wms_dir,'maps.dat'),'r'))
        except:
            maps = {'True Color Landsat': {'Name':"landsat321",
                                            'Title':"True Color Landsat",
                                            'Server':"OnEarth",
                                            'URL':"http://wms.jpl.nasa.gov/wms.cgi?VERSION=1.1.1&LAYERS=global_mosaic_base"+ \
                                                    "&STYLES=visual&FORMAT=image%2Fpng&REQUEST=GetMap&SERVICE=WMS" + \
                                                    "&EXCEPTIONS=application%2Fvnd.ogc.se_xml"
                                           }
                    }

        self.maps = maps
        self.mapKeys = self.maps.keys()
        self.mapKeys.sort()

    def setMap(self, mapKey):
        self.updating = True
        self.curMap = self.maps[mapKey]
        srvname = self.curMap['Server']
        if self.curServ is not None:
            if self.curServ == srvname: # don't open if it is already
                self.updating = False
                return
        if openService(srvname):
            self.curServ = srvname
        self.updating = False

    def validateMap(self, *args):
        # will eventually have more checks, only SRS for now
        result = ""
        parms = self.addParms()
        self.updateMapURL(parms)
        check = servCaps.validateURL(self.curMapURL)
        if check is not None:
            result += check
        elif not servCaps.validateSRS(parms['SRS']):
            result += "Invalid SRS\n"
        if len(result):
            result += "Map is invalid. Verify parameters above.\nURL:"
        else:
            result += "Map is valid.\nLocal file:"
            result += self.checkCache(self.curMap['Name'])
            result += "\nURL:"

        showValidateMapResult(result,self.curMapURL)

    def addParms(self):
        bbox = []
        bbox.append(self.ulxGE.get_text())
        bbox.append(self.lryGE.get_text())
        bbox.append(self.lrxGE.get_text())
        bbox.append(self.ulyGE.get_text())

        parms = {}
        parms['BBOX'] = bbox
        parms['WIDTH'] = self.wTE.get_text()
        parms['HEIGHT'] = self.hTE.get_text()
        parms['SRS'] = 'EPSG:'+self.epsgTE.get_text()

        color = self.colorBT.get_color()
        if color[3] == 0.0:
            parms['TRANSPARENT'] = 'TRUE'
        else:
            color = int(color[0]*16711425)+int(color[1]*65535)+int(color[2]*255)
            parms['BGCOLOR'] = hex(color)
            parms['TRANSPARENT'] = 'FALSE'

        if 'Extents' in self.curMap:
            for extent,value in self.curMap['Extents'].items():
                if extent in ('time','elevation'):
                    parms[extent] = value
                else:
                    parms['dim_'+extent] = value

        return parms

    def updateMapURL(self, parms=None):
        if parms is None:
            parms = self.addParms()
        self.curMapURL = servCaps.encodeURL(parms, self.curMap['URL']+'&')

    def getGeoTransform(self):
        ulx = self.ulxGE.getValue()
        uly = self.ulyGE.getValue()
        rezX = self.rezX
        rezY = self.rezY
        return [ulx,rezX,0,uly,0,rezY]

    def getSRS(self, epsg=None):
        srs = osr.SpatialReference()
        if epsg is None:
            view = self.viewwin.viewarea
            if view.projection:
                srs.ImportFromWkt(view.projection)
                return srs

        try: # in case of epsg not recognised
            srs.ImportFromEPSG(epsg)
        except:
            srs.ImportFromEPSG(4326) # default: WGS84 GEOGCS

        return srs

    def getEPSG(self):
        if self.srs.IsGeographic():
            epsg = self.srs.GetAuthorityCode('GEOGCS')
        else:
            epsg = self.srs.GetAuthorityCode('PROJCS')

        return str(epsg)

    def setResolution(self, entry, id):
        if self.updating:
            return

        if id == 'rezX':
            self.rezX = self.rezxGE.getValue()
            self.updateFromEntry(entry,'w')
            if self.linkTO.get_active():
                self.rezY = -self.rezX
                self.rezyGE.setValue(self.rezX)
        elif id == 'rezY':
            self.rezY = -self.rezyGE.getValue()
            self.updateFromEntry(entry,'h')

    def loadWorld(self, *args):
        worldfile = os.path.join(wms_dir, 'world-modis.tif')
        if os.path.exists(worldfile):
            self.open_file(worldfile)

    def refreshServices(self, *args):
        notsaved = []
        progress = pgu.ProgressDialog("Refreshing Services...")
        progress.show()
        for n,srvname in enumerate(servDic.keys()):
            progress.ProgressCB(float(n)/len(servDic), "Processing service %s..." % srvname)
            openService(srvname)
            if servCaps.open(servCaps.href):
                if createCapsEntry(srvname) is None:
##                    print "Service %s not saved" % url
                    outfile = os.path.join(srv_dir, srvname+'-tmp.xml')
                    fd = file(outfile,'w')
                    fd.write(servCaps.buffer)
                    fd.close()
                    notsaved.append(srvname+" (Saved as %s)" % srvname+'-tmp.xml')
                del servCaps.buffer
                servCaps.buffer = None
##                else:
##                    print "Service %s updated" % servCaps.title
            else:
##                print "Service %s not updated" % srvname
                notsaved.append(srvname+" (did not respond)")

        progress.ProgressCB(1.0, "Done.")
        progress.destroy()
        if notsaved:
            gvutils.error("The following services were not updated:\n%s" % '\n'.join(notsaved))

    def close(self, *args):
        self.viewwin.viewarea.disconnect(self.active_changed_id)
        self.viewwin.viewarea.disconnect(self.display_change_id)
        self.hide()
        self.destroy()

class SetupDialog(gtk.Window):
    def __init__(self, mapwin):
        gtk.Window.__init__(self)
        self.set_title("Setup")
        self.set_resizable(False)
        self.curServ = None
        self.mapDic = mapwin.maps
        self.mapKeys = mapwin.mapKeys
        self.mapWin = mapwin
        self.curMap = None
        self.updating = True
        self.editingMap = False
        self.editingSrv = False
        self.layerDlg = None
        self.connect('delete-event',self.close)

        if self.createGUI():
            self.show_all()
            self.updateServiceInfo()
            self.updateMapInfo()
        else:
            return

        self.updating = False
        self.mapCB.set_active_text(mapwin.curMap['Title'])

    def createGUI(self):
        mainbox = gtk.VBox(spacing=5)
        mainbox.set_border_width(5)
        self.add(mainbox)

        # Maps
        frame = gtk.Frame("Maps")
        mainbox.pack_start(frame, expand=False)

        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(5)
        frame.add(vbox)

        hbox = gtk.HBox(spacing=5)
        vbox.pack_start(hbox, expand=False)

        self.mapCB = pgu.ComboBoxEntry(action=self.mapSelected)
        hbox.pack_start(self.mapCB, expand=False)

        # map buttons
        bbox = gtk.HBox(spacing=5)
        hbox.pack_end(bbox, expand=False)

        but = gvutils.create_stock_button(gtk.STOCK_NEW, self.enterMap)
        tips.set_tip(but,"Create a new map")
        bbox.add(but)

        but = gvutils.create_stock_button(gtk.STOCK_DELETE, self.delMap)
        tips.set_tip(but,"Delete the selected map")
        bbox.add(but)

        but = gvutils.create_stock_button(gtk.STOCK_SAVE, self.saveMap)
        tips.set_tip(but,"Save the selected map")
        bbox.add(but)
##        but.set_sensitive(False)
        self.saveMapBut = but

        self.mapnameTE = gtk.Entry()
        self.mapnameTE.set_size_request(100,-1)
        hbox.pack_end(self.mapnameTE, expand=False)
        hbox.pack_end(gtk.Label("Name:"), expand=False)

        swin = gtk.ScrolledWindow()
        swin.set_size_request(-1,150)
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        swin.set_shadow_type(gtk.SHADOW_IN)
        vbox.pack_start(swin, expand=False)

        self.selectedTV = pgu.CList(titles=["Layer","Style"])
##        rend0.set_fixed_size(270,-1)
##        self.selectedTV.connect('button-press-event', self.selectedRow)
        selection = self.selectedTV.get_selection()
        selection.connect('changed', self.selectedRow)
        swin.add(self.selectedTV)

        # Services
        frame = gtk.Frame("Services")
        mainbox.pack_start(frame, expand=False)

        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(5)
        frame.add(vbox)

        hbox = gtk.HBox(spacing=5)
        vbox.pack_start(hbox, expand=False)

        self.servCB = pgu.ComboBoxEntry(action=self.serviceSelected)
        self.servCB.set_size_request(277,-1)
        self.servCB.entry.connect('activate', self.newService)
        self.servCB.set_sensitive(False)
        hbox.pack_start(self.servCB, expand=False)

        # services buttons
        bbox = gtk.HBox(spacing=5)
        hbox.pack_end(bbox, expand=False)

        but = gvutils.create_stock_button(gtk.STOCK_NEW, self.enterService)
        tips.set_tip(but,"Add a new service")
        bbox.add(but)

        but = gvutils.create_stock_button(gtk.STOCK_DELETE, self.delService)
        tips.set_tip(but,"Delete the selected service")
        bbox.add(but)

        but = gvutils.create_stock_button(gtk.STOCK_SAVE, self.saveService)
        tips.set_tip(but,"Save service entry")
        bbox.add(but)
        but.set_sensitive(False)
        self.saveServBut = but

        # Service caps
        hbox = gtk.HBox(spacing=5)
        vbox.pack_start(hbox, expand=False)

        # Image formats
##        box.pack_start(pgu.Label('Format:'), expand=False)
        self.formatCB = pgu.LabelComboText("Format:")
        self.formatCB.combo.set_size_request(220,-1)
        hbox.pack_start(self.formatCB, expand=False)

        # Exception formats
##        box.pack_start(pgu.Label('Except:'), expand=False)
        self.exceptCB = pgu.LabelComboText("Except:")
        self.exceptCB.combo.set_size_request(88,-1)
##        self.exceptCB.set_size_request(205,-1)
        hbox.pack_end(self.exceptCB, expand=False)

        # Layers
        frame = gtk.Frame("Available Layers")
        mainbox.pack_start(frame, expand=False)

        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(5)
        frame.add(vbox)

        swin = gtk.ScrolledWindow()
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        swin.set_size_request(-1,250)
        swin.set_shadow_type(gtk.SHADOW_IN)
        vbox.pack_start(swin, expand=False)

        self.layersTR = pgu.Tree(titles=["Layers"])
        self.layersTR.set_headers_visible(False)
        self.layersTR.connect('button-press-event', self.layerItemSelected)
        selection = self.layersTR.get_selection()
        selection.connect('changed', self.selectedRow)
        swin.add(self.layersTR)
##        self.layersTR.set_fixed_height_mode(True)
##        rend0.set_fixed_size(270,-1)

        # Buttons
        bbox = gtk.HButtonBox()
        mainbox.pack_start(bbox, expand=False)

        but = gtk.Button(label="Get map")
        tips.set_tip(but,"Get map")
        but.connect('clicked', self.getMap)
        bbox.add(but)

        but = gtk.Button("Validate")
        tips.set_tip(but,"Validate map")
        but.connect('clicked', self.validateMap)
        bbox.add(but)

        but = gtk.Button(stock=gtk.STOCK_OK)
        tips.set_tip(but,"Save maps and exit")
        but.connect('clicked', self.doneClicked)
        bbox.add(but)

        but = gtk.Button(stock=gtk.STOCK_CANCEL)
        tips.set_tip(but,"Exit without saving")
        but.connect('clicked', self.close)
        bbox.add(but)

        return True

    def doneClicked(self, *args):
        self.saveData()
        # update map window before closing
        self.updateMapWindow()
        self.close()

    def selectedRow(self, selection):
        if self.updating:
            return
        
##        row = tree.get_selected_row(event)
        tree,iter = selection.get_selected()
        if iter:
            title = tree.get_value(iter, 0)
##        title = tree.list[row][0]
##        title = tree.get_selected_value(event)
            index = None
            if title in self.curMapParms['LAYERS']:
                index = self.curMapParms['LAYERS'].index(title)
            self.showLayerDialog(title, index)
##        return False

    def layerItemSelected(self, lst, event):
##        if event.button == 1:
##            self.selectedRow(lst.get_selection())
##            return False

##        tree,iter = selection.get_selected()
##        if iter:
##            title = tree.get_value(iter, 0)
        if event.type == gtk.gdk._2BUTTON_PRESS:
            title = lst.get_selected_value(event)
            self.updating = True
            self.selectLayer(title)
            self.updateSelectedGUI()
            self.updating = False

        return False

    def showLayerDialog(self, title, index=None):
        if self.checkMap() and index is None:
            if title in self.curMapParms['LAYERS']:
                index = self.curMapParms['LAYERS'].index(title)

        if self.layerDlg is None:
            self.layerDlg = LayerDialog(title, self.curMapParms, index)
            self.layerDlg.subscribe('param-changed', self.updateSelectedGUI)
        else:
            self.layerDlg.create(title, self.curMapParms, index)
##            self.layerDlg.present()

    def updateServiceInfo(self):
        self.updating = True
        servKeys = servDic.keys()
        servKeys.sort()
        self.servCB.set_popdown_strings(servKeys)
        self.updating = False

    def updateMapInfo(self):
        self.updating = True
        self.mapKeys = self.mapDic.keys()
        self.mapKeys.sort()
        self.mapCB.set_popdown_strings(self.mapKeys)
        self.updating = False

    def updateMapGUI(self):
        self.formatCB.set_active_text(self.curMapParms['FORMAT'])
        self.exceptCB.set_active_text(servCaps.getExceptString(self.curMapParms['EXCEPTIONS']))

    def updateMapWindow(self):
        mapwin = self.mapWin
        mapwin.updating = True
        mapwin.mapKeys = self.mapKeys
        mapwin.maps = self.mapDic
        mapwin.mapCB.set_popdown_strings(self.mapWin.mapKeys)
        mapwin.updating = False
        mapwin.mapCB.set_active_text(self.curMap['Title'])

    def mapSelected(self, *args):
        if self.updating or self.editingMap:
            return
        key = self.mapCB.get_active_text()
        if key:
            self.setMap(key)

    def setMap(self, key):
        self.updating = True
        self.curMap = self.mapDic[key]
        self.mapnameTE.set_text(self.curMap['Name'])

        skey = self.curMap['Server']
        if not self.setService(skey):
            self.servCB.entry.set_text("Service entry not found")
            self.updating = False
            return

        self.servCB.set_active_text(skey)
        self.curMapParms = servCaps.decodeURL(self.curMap['URL'])
        if 'Extents' in self.curMap:
            self.curMapParms['Extents'] = self.curMap['Extents']

        self.updateMapGUI()
        self.updateSelectedGUI()
        self.updating = False

    def checkMap(self):
        serv = self.curMap['Server']
        return serv == self.curServ and not serv == 'None'

    def validateMap(self, *args):
        if self.checkMap():
            exmap = self.mapWin.curMap
            self.mapWin.curMap = self.makeMap()
            self.mapWin.validateMap()
            self.mapWin.curMap = exmap

    def enterMap(self, *args):
        self.updating = True
        self.editingMap = True
        self.mapCB.entry.set_text("New map")
        self.mapnameTE.set_text("newmap")
        self.servCB.entry.set_text("Select service")
        self.newMap()
        self.updateSelectedGUI()
        self.updating = False

    def newMap(self):
        self.servCB.set_sensitive(True)
        self.setService('Select service')
        parms = {}
        parms['SERVICE'] = 'WMS'
        parms['REQUEST'] = 'GetMap'
        parms['LAYERS'] = []
        parms['STYLES'] = []
        self.curMapParms = parms

        map = {}
        map['Name'] = "newmap"
        map['Title'] = "New map"
        map['Server'] = "None"
        self.curMap = map

    def saveMap(self, *args):
        self.updating = True
        map = self.makeMap()
        if 'Extents' in self.curMapParms:
            map['Extents'] = self.curMapParms.get('Extents')
            del self.curMapParms['Extents']

        map['URL'] = servCaps.encodeURL(self.curMapParms)
        mapKey = map['Title']
        self.mapDic[mapKey] = map
        self.updateMapInfo()
        self.mapCB.set_active_text(mapKey)
        self.setMap(mapKey)
        self.updateMapWindow()
        self.updating = False
        self.editingMap = False

    def delMap(self, *args):
        self.updating = True
        self.mapDic.pop(self.curMap['Title'])
        self.updateMapInfo()
        self.updating = False
        self.mapCB.set_active(0)
        self.updateMapWindow()

    def getMap(self, *args):
        self.mapWin.curMap = self.makeMap()
        self.mapWin.addLayer()

    def makeMap(self):
        map = {}
        map['Title'] = self.mapCB.get_active_text()
        map['Name'] = self.mapnameTE.get_text()
        map['Server'] = self.servCB.get_active_text()
        self.curMapParms['FORMAT'] = self.formatCB.get_active_text()
        self.curMapParms['EXCEPTIONS'] = servCaps.getException(self.exceptCB.get_active_text())
        if 'Extents' in self.curMapParms:
            map['Extents'] = self.curMapParms['Extents']
        params = self.curMapParms.copy()
        map['URL'] = servCaps.encodeURL(params)

        return map

    def serviceSelected(self, *args):
        if self.updating or self.editingSrv:
            return
        skey = self.servCB.get_active_text()
        self.setService(skey)
        if not self.curMap['Title'] in self.mapDic:
            self.curMapParms['EXCEPTIONS'] = servCaps.getException(self.exceptCB.get_active_text())
            self.curMapParms['FORMAT'] = self.formatCB.get_active_text()
            self.curMapParms['VERSION'] = servCaps.version
            self.curMap['Server'] = skey
            self.curMap['URL'] = servCaps.encodeURL(self.curMapParms)
            if servCaps.layer.find('Dimension') is not None:
                self.curMapParms['Extents'] = {}

    def setService(self, skey):
        self.updating = True
        result = True
        if skey == 'Select service':
            self.updateServiceGUI(skey)
            self.curServ = None
            self.updating = False
            return result

        if skey != self.curServ:
            if openService(skey):
                self.curServ = skey
                self.updateServiceGUI(skey)
                self.updateLayersList()
            else:
                self.updateServiceGUI('')
                self.curServ = None
                result = False

        self.updating = False
        return result

    def enterService(self, *args):
        self.updating = True
        self.editingSrv = True
        self.servCB.set_sensitive(True)
        self.servCB.set_active(-1)
        self.mapCB.set_active(-1)
        self.mapCB.set_sensitive(False)
        tips.set_tip(self.servCB.entry,"Enter service filename/url and hit <Return>")
        self.updateServiceGUI('New service')
        self.updating = False

    def newService(self, entry):
        self.updating = True
        pathname = entry.get_text()
        fname = None
        type,p = urllib.splittype(pathname)
        if type is None:
            fname = pathname
            url = os.path.join(wms_dir, pathname)
        else:
            url = pathname

        if not servCaps.open(url):
            txt = "Could not open %s.\nServer responded:\n%s" % (url, servCaps.buffer)
            gvutils.error(txt)
            del servCaps.buffer
            servCaps.buffer = None
            self.updating = False
            self.editingSrv = False
            return

        self.curServ = servCaps.title
        self.servCB.set_active_text(servCaps.title)
        self.updateServiceGUI(self.curServ)
        self.updateLayersList()
        self.saveServBut.set_sensitive(True)
        self.updating = False

    def delService(self, *args):
        txt = "Do you really want to delete this service?\nAll maps using this service will be deleted as well."
        ret = gvutils.noyes("Delete Service",txt)
        if ret == 'No':
            return
        self.updating = True
        srvname = self.servCB.get_active_text()
        host = servDic.pop(srvname)
        os.remove(os.path.join(srv_dir, host, srvname+'.xml'))
        self.updateServiceInfo()
        for key,mapdic in self.mapDic.items():
            if mapdic['Server'] == srvname:
                mapdic.pop(key)
        self.updateMapInfo()
        self.mapCB.set_active(0)
        self.updateMapWindow()
        self.updating = False

    def saveService(self, *args):
        self.updating = True
        srvKey = self.servCB.get_active_text()
        try:
            host = createCapsEntry(srvKey)
            servDic[srvKey] = host
            self.updateServiceInfo()
        except:
            outfile = os.path.join(wms_dir,'caps-tmp.xml')
            fd = file(outfile,'w')
            fd.write(servCaps.buffer)
            fd.close()
            gvutils.error("The Service entry could not be created.\nSaved as " + outfile)
            srvKey = servDic.keys()[0]
        
        del servCaps.buffer
        servCaps.buffer = None
        self.editingSrv = False
        self.updating = False
        self.servCB.set_active_text(srvKey)

    def updateServiceGUI(self, key):
        if key in ('Select service','New service',''):
            self.formatCB.set_active(-1)
            self.exceptCB.set_active(-1)
            if key == 'Select service':
                tips.set_tip(self.servCB.entry, "Select the service for this map. \n Only one service per map.")
            self.layersTR.clear()
            if key in ('New service',''):
                self.selectedTV.clear()
            return

        self.formatCB.set_popdown_strings(servCaps.getImageFormats())
        self.exceptCB.set_popdown_strings(servCaps.getExceptFormats())

        if servCaps.abstract:
            txt = servCaps.abstract
        else:
            txt = servCaps.title
        tips.set_tip(self.servCB.entry, txt)

    def updateLayersList(self):
        self.layersTR.clear()
        tree = self.layersTR.append(None,[servCaps.layer.title])
        self.addLayerTree(servCaps.layer, tree)
        path = self.layersTR.get_path(tree)
        self.layersTR.expand_row(path, False)

    def updateSelectedGUI(self, *args):
        self.updating = True
        self.selectedTV.clear()
        for laytitle in self.curMapParms['LAYERS']:
            if self.curMapParms['STYLES']:
                idx = self.curMapParms['LAYERS'].index(laytitle)
                stitle = self.curMapParms['STYLES'][idx]
            self.selectedTV.append((laytitle,stitle))
        self.updating = False

    def addLayerTree(self, laynode, tree):
        for node in servCaps.sortLayerTree(laynode):
            subtree = self.layersTR.append(tree, [node.findtext('Title')])
            if node.find('Layer') is not None:
                self.addLayerTree(node, subtree)

    def selectLayer(self, title):
        if not self.checkMap():
            return

        layerNode = servCaps.getLayerNode(title)
        if layerNode.find('Name') is None:
            return

        if title in self.curMapParms['LAYERS']:
            index = self.curMapParms['LAYERS'].index(title)
            del self.curMapParms['LAYERS'][index]
            del self.curMapParms['STYLES'][index]
            if 'Extents' in self.curMapParms and not self.curMapParms['LAYERS']:
                self.curMapParms['Extents'] = {}
        else:
            self.curMapParms['LAYERS'].append(title)
            styleNode = layerNode.find('Style')
            if styleNode is not None:
                style = styleNode.findtext('Title')
            else:
                style = ''
            self.curMapParms['STYLES'].append(style)
            if layerNode.find('Extent') is not None:
                extents = layerNode.findall('Extent')
                for extent in extents:
                    self.curMapParms['Extents'][extent.get('name')] = extent.get('default')

    def saveData(self, *args):
        pickle.dump(self.mapDic, file(os.path.join(wms_dir,'maps.dat'),'w'))

    def close(self, *args):
        if self.layerDlg:
            self.layerDlg.unsubscribe('param-changed', self.updateSelectedGUI)
            self.layerDlg.destroy()

        self.destroy()

class LayerDialog(gtk.Window, Signaler):
    def __init__(self, title, params, index=None):
        gtk.Window.__init__(self)
        self.set_title("Layer Properties")
        self.set_resizable(False)
        self.connect('delete-event', self.close)
        self.publish('param-changed')
        self.create(title, params, index)

    def create(self, title, params, index=None):
        self.updating = True
        self.layerNode = servCaps.getLayerNode(title)
        self.params = params
        self.index = index
        self.curStyle = None
        self.extentDic = {}
        for child in self.get_children():
            child.unrealize()
            child.destroy()
        self.createGUI()
        self.show_all()
        self.updating = False

    def createGUI(self):
        srsflag = True
        mainbox = gtk.VBox(spacing=5)
        mainbox.set_border_width(5)
        self.add(mainbox)

        frame = gtk.Frame("Information")
        mainbox.pack_start(frame, expand=False)
        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(5)
        frame.add(vbox)

        if self.layerNode.find('Name') is not None:
            self.selectedTO = gtk.CheckButton(label="Selected")
            vbox.pack_start(self.selectedTO)
            if self.index is not None:
                self.selectedTO.set_active(True)
            self.selectedTO.connect('toggled', self.selectedToggled)

        for node in self.layerNode.getiterator():
            tag = node.tag
            if tag in ('Name','Title'):
                label = pgu.Label('%s: %s' % (tag, node.text.decode(encoding)))
                label.set_line_wrap(True)
                vbox.pack_start(label, expand=False)
            elif tag == 'Abstract':
                swin = gtk.ScrolledWindow()
                swin.set_size_request(400,100)
                swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
                vbox.pack_start(swin, expand=False)
                label = pgu.Label(node.text.decode(encoding))
##                label.set_justify(gtk.JUSTIFY_FILL)
                label.set_line_wrap(True)
                swin.add_with_viewport(label)
            elif tag == 'SRS' and srsflag:
                vbox.pack_start(gtk.HSeparator())
                srsLst = servCaps.getSRSList(self.layerNode)
##                box = gtk.HBox(spacing=3)
##                vbox.add(box)
##                box.pack_start(gtk.Label('SRS:'), expand=False)
                srsCB = pgu.LabelComboText("SRS:", strings=srsLst)
                srsCB.set_size_request(110,-1)
                tips.set_tip(srsCB.combo, "For information only. Not selectable")
                vbox.pack_start(srsCB, expand=False)
                srsflag = False
            elif tag in ('LatLonBoundingBox','BoundingBox'):
                vbox.pack_start(self.createBBoxLabel(node), expand=False)
            elif tag in ('Dimension','ScaleHint','MetadataURL','DataURL', \
                         'KeywordList','Attribution','AuthorityURL','Identifier','FeatureListURL'):
                vbox.pack_start(self.createAttrLabel(node), expand=False)

        vbox.add(self.createAttrLabel(self.layerNode))
        if self.layerNode.find('Style') is not None:
            mainbox.pack_start(self.createStylesGUI(), expand=False)
        if self.layerNode.find('Extent') is not None:
            mainbox.pack_start(self.createExtentGUI(), expand=False)

    def createStylesGUI(self):
        frame = gtk.Frame("Style")
        vbox = gtk.VBox(spacing=5)
        vbox.set_border_width(5)
        frame.add(vbox)

        box = gtk.HBox(spacing=5)
        vbox.pack_start(box, expand=False)
        styleTree = self.layerNode.findall('Style')
        styles = servCaps.getTitles(styleTree)
        self.stylesCB = pgu.ComboText(strings=styles)
        box.add(self.stylesCB)

        self.styleTX = gtk.Label()
        vbox.pack_start(self.styleTX, expand=False)
        if self.index is None:
            self.setStyle(styles[0])
        else:
            self.setStyle(self.params['STYLES'][self.index])
        self.stylesCB.set_active_text(self.curStyle)
        self.stylesCB.connect(cb=self.styleChanged)

        legend = self.getStyle(self.curStyle)
        if legend:
            if legend.find('LegendURL') is not None:
                vbox.pack_start(gtk.Label(legend.tag+"..."), expand=False)

        return frame

    def createExtentGUI(self):
        extentTree = self.layerNode.findall('Extent')
        frame = gtk.Frame("Extent")
        vbox = gtk.VBox(spacing=3)
        vbox.set_border_width(5)
        frame.add(vbox)
        for extent in extentTree:
            extentTE = ExtentEntry(extent)
            vbox.pack_start(extentTE, expand=False)
            name = extent.get('name')
            extentTE.connect('changed', self.extentChanged, name)
            self.extentDic[name] = extentTE.entry
            if self.index is not None:
                extentTE.entry.set_text(self.params['Extents'][name])
        return frame

    def createBBoxLabel(self, elem):
        vbox = gtk.VBox(spacing=3)
        vbox.add(gtk.HSeparator())
        if elem.tag == 'BoundingBox':
            vbox.pack_start(gtk.Label(elem.tag+' '+elem.get('SRS')), expand=False)
        else:
            vbox.pack_start(gtk.Label(elem.tag), expand=False)
        label = gtk.Label("maxy"+":"+elem.get('maxy'))
        vbox.pack_start(label, expand=False)
        box = gtk.HBox(spacing=10)
        vbox.add(box)
        lbox = gtk.HBox()
        box.add(lbox)
        rbox = gtk.HBox()
        box.add(rbox)
        label = gtk.Label("minx"+":"+elem.get('minx'))
        label.set_justify(gtk.JUSTIFY_LEFT)
        lbox.add(label)
        label = gtk.Label("maxx"+":"+elem.get('maxx'))
        label.set_justify(gtk.JUSTIFY_RIGHT)
        rbox.add(label)
        label = gtk.Label("miny"+":"+elem.get('miny'))
        vbox.pack_start(label, expand=False)
        return vbox

    def createAttrLabel(self, elem):
        tag = elem.tag
        vbox = gtk.VBox(spacing=3)
        if len(elem.items()) == 0 and tag == 'Layer':
            return vbox
        vbox.add(gtk.HSeparator())
        if tag in ('DataURL','MetadataURL','KeywordList','Attribution','AuthorityURL','Identifier','FeatureListURL'):
            vbox.pack_start(gtk.Label(tag+'...'), expand=False)
            return vbox
        vbox.pack_start(gtk.Label(tag), expand=False)
        for attr,value in elem.items():
            box = gtk.HBox()
            vbox.pack_start(box, expand=False)
            label = gtk.Label(attr+": "+value)
            box.pack_start(label, expand=False)
        return vbox

    def selectedToggled(self, chk):
        if self.updating:
            return
        self.selectLayer(chk.get_active())

    def selectLayer(self, select):
        title = self.layerNode.findtext('Title')
        if select:
            self.params['LAYERS'].append(title)
            if self.curStyle is None:
                style = ''
            else:
                style = self.curStyle
            self.params['STYLES'].append(style)
            if 'Extents' in self.params:
                for extent,entry in self.extentDic.iteritems():
                    self.params['Extents'][extent] = entry.get_text()
            self.index = self.params['LAYERS'].index(title)
        else:
            del self.params['LAYERS'][self.index]
            del self.params['STYLES'][self.index]
            if 'Extents' in self.params and len(self.params['LAYERS']) == 0:
                self.params['Extents'] = {}
            self.index = None

        self.notif('param-changed')

    def styleChanged(self, combo):
        if self.updating:
            return
        sel = combo.get_active_text()
        if sel:
            self.setStyle(sel)
    
    def extentChanged(self, entry, name):
        if self.updating:
            return
        sel = entry.get_text()
        if sel:
            self.params['Extents'][name] = sel

    def setStyle(self, title):
        self.curStyle = title
        if self.index is not None:
            self.params['STYLES'][self.index] = title
            self.notif('param-changed')

        self.updateStyleGUI()

    def getStyle(self, title):
        for style in self.layerNode.findall('Style'):
            if style.findtext('Title') == title:
                return style

    def updateStyleGUI(self):
        style = self.getStyle(self.curStyle)
        if style:
            abstract = style.find('Abstract')
            if abstract is not None:
                self.styleTX.set_text(abstract.text.decode(encoding))

    def close(self, *args):
        self.updating = True
        self.hide()
        return True

# custom widgets and functions

def showValidateMapResult(result, url):
    dlg = gtk.Dialog("Map Validation", None, gtk.DIALOG_MODAL, (gtk.STOCK_CLOSE,0))
##    hbox = gtk.HBox()
##    hbox.set_border_width(3)
##    dlg.vbox.pack_start(hbox, expand=False)

    label = pgu.Label(result)
    dlg.vbox.pack_start(label)

    # just a trick to get the text length
    urltxt = gtk.Label(url)
    w,h = urltxt.size_request()

    urlTE = gtk.Entry()
    urlTE.set_size_request(w+10,-1)
    urlTE.set_text(url)

    urlwin = gtk.ScrolledWindow()
    urlwin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_NEVER)
    urlwin.set_size_request(-1,50)
    urlwin.add_with_viewport(urlTE)
    dlg.vbox.pack_start(urlwin)

    # do this last, just in case...
    urlTE.select_region(0,-1)
    urlTE.copy_clipboard()
    dlg.show_all()
    dlg.run()
    dlg.destroy()

class GeoEntry(gtk.Frame):
    def __init__(self, geo, dgsz=33, mnsz=22, scsz=45, mtsz=110):
        gtk.Frame.__init__(self)
        self.set_shadow_type(gtk.SHADOW_NONE)
        self.hbox = gtk.HBox(spacing=3)
        self.add(self.hbox)
        self.degsz = dgsz
        self.metsz = mtsz
        self.minsz = mnsz
        self.secsz = scsz
        self.round = 2
        self.setMode(geo)

    def connect(self, signal, action, data):
        if self.isGeo:
            self.degTE.connect(signal, action, data)
            self.minTE.connect(signal, action, data)
            self.secTE.connect(signal, action, data)
        else:
            self.metersTE.connect(signal, action, data)

    def setRound(self, value):
        self.round = value

    def setGeoMode(self):
        self.degTE = self.createEntry(self.degsz,"°")
        self.minTE = self.createEntry(self.minsz,"\'")
        self.secTE = self.createEntry(self.secsz,"\"")
        self.isGeo = True

    def setMode(self, geo):
        for child in self.hbox.get_children():
            child.unrealize()
            child.destroy()

        if geo:
            self.setGeoMode()
        else:
            self.setProjMode()

        self.show_all()

    def setProjMode(self):
        self.metersTE = self.createEntry(self.metsz,"m")
        self.isGeo = False

    def createEntry(self, size, lbl):
        te = gtk.Entry()
        te.set_size_request(size,20)
        self.hbox.pack_start(te, expand=False)
        label = gtk.Label(lbl)
        self.hbox.pack_start(label)
        return te

    def getDegrees(self):
        txt = self.degTE.get_text()
        return self.validateInt(txt)

    def getMinutes(self):
        txt = self.minTE.get_text()
        return self.validateInt(txt)

    def getSeconds(self):
        txt = self.secTE.get_text()
        return self.validateFloat(txt)

    def getMeters(self):
        txt = self.metersTE.get_text()
        return self.validateFloat(txt)

    def validateInt(self, txt):
        if txt and txt != '-':
            value = int(txt)
        else:
            value = 0

        return value

    def validateFloat(self, txt):
        if txt and txt != '-':
            value = float(txt)
        else:
            value = 0.0

        return value

    def setDegrees(self, value):
        self.degTE.set_text(str(value))

    def setMinutes(self, value):
        self.minTE.set_text(str(value))

    def setSeconds(self, value):
        self.secTE.set_text(str(round(value, self.round)))

    def setMeters(self, value):
        self.metersTE.set_text(str(value))

    def setValue(self, value):
        if self.isGeo:
            d,m,s = self.deg2dms(value)
            self.setDegrees(d)
            self.setMinutes(m)
            self.setSeconds(s)
        else:
            self.setMeters(value)

    def set_text(self, txt):
        self.setValue(float(txt))

    def getValue(self):
        dms = []
        if self.isGeo:
            dms.append(self.getDegrees())
            dms.append(self.getMinutes())
            dms.append(self.getSeconds())
            return self.dms2deg(dms)
        else:
            return self.getMeters()

    def get_text(self):
        value = self.getValue()
        return str(value)

    def deg2dms(self, dd):
        deg = int(dd)
        dec = abs(dd - deg)
        mn = int(dec*60)
        sec = (dec*60 - mn) * 60
        if round(sec, self.round) >= 60.0:
            sec = 0.0
            mn += 1
        return deg,mn,sec

    def dms2deg(self, dms):
        d,m,s = dms
        dd = abs(d) + m/60.0 + s/3600.0
        if d < 0:
            return -dd
        else:
            return dd

class ExtentEntry(gtk.HBox):
    def __init__(self, extent):
        gtk.HBox.__init__(self, spacing=3)
        name = extent.get('name')
        label = pgu.Label('%s: ' % name)
        self.pack_start(label, expand=False)
        extLst = extent.text.split(',')
        if len(extLst) > 1:
            extentCB = pgu.ComboBoxEntry(strings=extLst)
            extentCB.entry.set_size_request(75,-1)
            self.pack_start(extentCB, expand=False)
            extentCB.set_active_text(extent.get('default'))
            self.entry = extentCB.entry
            dimTree = servCaps.layer.findall('Dimension')
            for elem in dimTree:
                if elem.get('name') == name:
                    units = elem.get('unitSymbol')
                    break
            if units is not None:
                self.pack_start(gtk.Label(units), expand=False)
        else:
            extLst = extent.text.split('/')
            if len(extLst) == 1:
                label = gtk.Label(extent.text)
                self.pack_start(label, expand=False)
                self.entry = label
            else:
                extentTE = gtk.Entry()
                extentTE.set_text(extent.get('default'))
                extentTE.set_size_request(75,-1)
                self.pack_start(extentTE, expand=False)
                self.entry = extentTE
                txt = " from %s to %s by %s" % (extLst[0], extLst[1], extLst[2][1:])
                self.pack_start(gtk.Label(txt), expand=False)

    def connect(self, signal, action, data):
        self.entry.connect(signal, action, data)

if __name__ == '__main__':
    import sys
    if not os.path.exists(wms_dir):
        os.mkdir(wms_dir)
    if not os.path.exists(cachepath):
        os.mkdir(cachepath)
    if not os.path.exists(srv_dir):
        os.mkdir(srv_dir)
    
    if len(sys.argv) > 1:
        intxt = file(sys.argv[1],'r').read()
        lst = intxt.splitlines()
    else:
        lst = ['OnEarth,http://wms.jpl.nasa.gov/wms.cgi?','GLOBE,http://globe.digitalearth.gov/viz-bin/wmt.cgi?']

    if len(sys.argv) > 2:
        srvDir = os.path.join(wms_dir, sys.argv[2])
    else:
        srvDir = srv_dir
    if not os.path.exists(srvDir):
        os.mkdir(srvDir)

    print "Processing list..."
    buildServicesDir(lst, srvDir)
    sys.exit(0)
