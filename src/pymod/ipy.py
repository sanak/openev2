import gtk
import gviewapp
import gview
import gvcommand
import gvcorecmds
import Numeric
import gdalnumeric
import sys
from pyshell import _run_command_line
from IPython.iplib import InteractiveShell

ev_ignore_commands = ['shell', 'help', 'macro', 'journal']

class InterpreterStub:
    def __init__(self, locals):
        self.cmdlist = {}
        self.locals = locals
    def showText(self, text, text_class):
        if text_class == 'error':
            print >> sys.stderr, text
        else:
            print >> sys.stdout, text
    def add_command(self, command):
        self.cmdlist[command.Name.lower()] = command

class ShellStub:
    standalone = None

def create_magics(interp):
    for key, val in interp.cmdlist.items():
        name = val.Name
        if name in ev_ignore_commands:
            continue
        func = gen_magic_func(name)
        func.__doc__ = val.Usage
        setattr(InteractiveShell, 'magic_' + name, func)

def gen_magic_func(name):
    def magic_generic(self, parameter_s=''):
        line = name + ' ' + parameter_s
        return _run_command_line(line)
    return magic_generic

def get_roi(num_array):
    roi = gview.app.toolbar.get_roi()
    
    x1 = int(roi[0])
    y1 = int(roi[1])
    x2 = int(roi[0] + roi[2]) + 1
    y2 = int(roi[1] + roi[3]) + 1
    
    return num_array[...,y1:y2,x1:x2]

def init(locals):
    interp = InterpreterStub(locals)
    gvcorecmds.Register(interp)

    create_magics(interp)

    gview.app = app = gviewapp.GViewApp()
    gview.app.shell = ShellStub()
    gview.app.shell.interp = interp
    app.subscribe('quit',gtk.main_quit)
    app.do_auto_imports()

    gtk.main()
    return app

def evshow(arr, prototype_name=None, new_view=False):
    if len(Numeric.shape(arr)) == 1:
        arr=Numeric.reshape(arr,(1,Numeric.shape(arr)[0]))

    view = gview.app.view_manager.get_active_view_window()
    if view is None or new_view:
        view_window = gview.app.new_view()
        view_window.set_close_function(3)
    array_name = gdalnumeric.GetArrayFilename(arr)
    ds = gview.manager.get_dataset( array_name )
    if prototype_name is not None:
        prototype_ds = gdal.Open( prototype_name )
        gdalnumeric.CopyDatasetInfo( prototype_ds, ds )

    gview.app.file_open_by_name( array_name )

def magic_evshow(self, parameter_s=''):
    "Usage: evshow a [/p=prototype_name] [/n]"

    args = parameter_s.split(' ')
    arr = eval(args[0], globals(), self.locals)

    prototype_name = None
    new_view = False
    for arg in args[1:]:
        if arg.find('/p') > -1:
            p, pname = arg.split('=')
            prototype_name = pname.strip()
        elif parameter_s.find('/n') > -1:
            new_view = True

    evshow(arr, prototype_name, new_view)
    

magic_evshow.__doc__ = evshow.__doc__
InteractiveShell.magic_evshow = magic_evshow
del magic_evshow

if 0:
    # don't reinitialize if the module is reloaded
    try:
        gview.app
    except AttributeError:
        init()

