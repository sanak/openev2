###############################################################################
# $Id: gvcorecmds.py,v 1.1.1.1 2005/04/18 16:38:35 uid1026 Exp $
#
# Project:  OpenEV
# Purpose:  Implementation of some sample OpenEV core commands.
# Author:   Frank Warmerdam, warmerdam@pobox.com
#
###############################################################################
# Copyright (c) 2002, Frank Warmerdam <warmerdam@pobox.com>
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
import gvcommand


###############################################################################
# The Commands command.
class GvCommandsCommand(gvcommand.CommandBase):

    """
    List all currently loaded commands and display their usage strings.  

    Parameters: 
        group- used to limit the commands listed to
               those from a single group (eg. commands core
               will display the usage strings of the commands
               in group "core" only).

        /v- verbose mode.  Display full help for commands.
        
    """
    
    def __init__(self):
        self.Name = 'commands'
        self.Usage = 'commands [group] [-v]'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'group', type = 'string_word',
                              required=0 ),
            gvcommand.ArgDef( name = 'v', type = 'switch', required=0 )
            ]
        
    def execute( self, args, line, interp ):
        group=args[0]
        verbose=args[1]
        
        commands = gview.app.shell.get_commands()
        interp.showText( '%-24s %s' % ('Command', 'Usage' ), 'result' )
        interp.showText( '%-24s %s' % ('-------', '-----' ), 'result' )
        for cmd in commands:
            if group is None:
                if verbose == 0:
                    interp.showText( '%-24s %s' % (cmd.Name, cmd.Usage),
                                     'result')
                else:
                    txt=interp.get_command_help(cmd.Name,quiet=1)
                    if txt is None:
                        interp.showText(cmd.Name+':\nNo help available.\n',
                                        'result')
                    else:
                        interp.showText(cmd.Name+':\n'+txt+'\n','result')
            else:
                if (hasattr(cmd,'Group') and (cmd.Group == group)):
                    if verbose == 0:
                        interp.showText( '%-24s %s' % (cmd.Name, cmd.Usage),
                                         'result')                    
                    else:
                        txt=interp.get_command_help(cmd.Name,quiet=1)
                        if txt is None:
                            interp.showText(cmd.Name+
                                            ':\nNo help available.\n','result')
                        else:
                            interp.showText(cmd.Name+':\n'+txt+'\n','result')

        return 1
        
###############################################################################
# Functions command.

class GvFunctionsCommand(gvcommand.CommandBase):
    """
    The functions command is used to list all currently
    loaded functions, or to scan a python module for
    functions (module need not be loaded in python
    shell, but must be accessible within the python
    path).  

    Parameters:

        module- if a module name is specified, the module
                will be scanned for functions, and a 
                list of the results will be displayed.
                If no module name is specified, the
                functions currently loaded in the
                python shell will be displayed.

        /v- verbose mode.  Display full help for each function.
        
    """
    
    def __init__(self):
        self.Name = 'functions'
        self.Usage = 'functions [module] [-v]'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'module', type = 'string_word',
                              required=0 ),
            gvcommand.ArgDef( name = 'v', type = 'switch', required=0 )
            ]
        
    def execute( self, args, line, interp ):
        import string
        import os
        import Numeric

        modname=args[0]
        verbose = args[1]
        
        local_dict = interp.locals
        next_txt=''
        if modname is not None:
            # User has specified a module name
            # to search for functions.

            # to do: use the help in the
            # helpfiles, but only if the module
            # names match
            
            if modname in local_dict.keys():
                modinst=local_dict[modname]
                modkeys=dir(local_dict[modname])                
            else:
                try:
                    exec 'import '+modname
                    exec 'modinst='+modname
                    exec 'modkeys=dir('+modname+')'
                except:
                    interp.showText('Unable to locate module '+modname+'.',
                                    'error')
                    return 0
                
            # If particular module is specified,
 
            keys_to_show = []
            for ckey in modkeys:
                ckey_type=None
                exec 'ckey_type=type(modinst.'+ckey+')'
                if (ckey_type == type(Register) and
                     ckey[0] != "_"):
                     keys_to_show.append(ckey)
                elif (ckey_type == type(Numeric.logical_and) and
                    ckey[0] != "_"):
                    keys_to_show.append(ckey)
                elif (ckey_type == type(hasattr) and
                    ckey[0] != "_"):
                    keys_to_show.append(ckey)
    
            interp.showText( 'Functions: '+modname , 'result' )
            interp.showText(
                '------------------------------------------------' , 'result' )
            keys_to_show.sort()
            if verbose == 0:
                count=1
                for ckey in keys_to_show:
                    # In non verbose mode, show 2 columns
                    if (count % 2) == 0:
                        next_txt=next_txt+'\t'+ckey
                        interp.showText(next_txt,'result')
                        next_txt=''
                    else:
                        next_txt=next_txt+'%-24s' % ckey
                    count=count+1
                return 1

            # Verbose mode
            for ckey in keys_to_show:
                # First, check to see if there is help for
                # the function.
                
                txt=interp.get_function_help(ckey,modname,quiet=1)
                if txt is None:
                    txt=interp.get_builtin_help(ckey,modname,quiet=1)

                interp.showText( '%s:\n\n%s' % (ckey, txt),'result')
                
            return 1
            

        interp.showText( 'Functions:' , 'result' )
        interp.showText( '------------------------------------------------' ,
                         'result' )
        keys_to_show=[]
        for ckey in local_dict.keys():
            if (type(local_dict[ckey]) == type(Register) and
               ckey[0] != "_"):
                keys_to_show.append(ckey)
            elif (type(local_dict[ckey]) == type(Numeric.logical_and) and
               ckey[0] != "_"):
                keys_to_show.append(ckey)
            elif (type(local_dict[ckey]) == type(hasattr) and
               ckey[0] != "_"):
                keys_to_show.append(ckey)
               
        keys_to_show.sort()
        count=1
        for ckey in keys_to_show:
            if verbose == 1:
                txt=interp.get_function_help(ckey,quiet=1)
                if txt is None:
                    txt=interp.get_builtin_help(ckey,quiet=1)
                if txt is None:
                    txt='No help available.'
                interp.showText( '%s:\n%s' % (ckey, txt),'result')
            else:
                # In non verbose mode, show 2 columns
                if (count % 2) == 0:
                    next_txt=next_txt+'\t'+ckey
                    interp.showText(next_txt,'result')
                    next_txt=''
                else:
                    next_txt=next_txt+'%-24s' % ckey
                
            count=count+1

        return 1

    
###############################################################################
# NewView command.
class GvNewViewCommand(gvcommand.CommandBase):
    """
    Create a new OpenEV view window.

    Parameters:

        title- title for the new view.
        
    """
    
    def __init__(self):
        self.Name = 'newview'
        self.Usage = 'newview [title]'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'title', type = 'string_chunk',
                              required=0 )
            ]
        
    def execute( self, args, line, interp ):

        title = args[0]

        if title is None or title == '':
            title = None

        if gview.app.shell.standalone == 1:
            gview.app.new_view(title,menufile='PyshellNewViewMenuFile.xml')
        else:
            gview.app.new_view( title )
        return 1
        
###############################################################################
# ClearView command.
class GvClearViewCommand(gvcommand.CommandBase):
    """
    Clear the active view.
    
    The clearview command is used to clear the currently
    active view.  The currently active view is the one
    selected in the layer dialog (Edit->Layers); usually,
    it is the last view modified or clicked on.
    
    """
    
    def __init__(self):
        self.Name = 'clearview'
        self.Usage = 'clearview'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = []
        
    def execute( self, args, line, interp ):

        view = gview.app.sel_manager.get_active_view()

        layer_list = view.list_layers()
        while len(layer_list) > 0:
            view.remove_layer( layer_list[0] )
            layer_list = view.list_layers()

        return 1
        
###############################################################################
# View3D Command
class GvView3DCommand(gvcommand.CommandBase):
    """
    Display a 3D raster in the current OpenEV view.

    Parameters:

        demfile- file to use as for elevation values.

        drapefile- file to use as drape.  Defaults to demfile
                   if drapefile is not specified.

        mesh_lod- mesh level of detail to use in sampling
                  demfile (numeric).

        hscale- amount to scale demfile by to generate
                elevation values (numeric).

    """
    
    def __init__(self):
        self.Name = 'view3d'
        self.Usage = 'view3d <demfile> [drapefile] [mesh_lod] [hscale]'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'demfile', type = 'string_word',
                              required=1 ),
            gvcommand.ArgDef( name = 'drapefile', type = 'string_word',
                              required=0 ),
            gvcommand.ArgDef( name = 'mesh_lod', type = 'numeric',
                              required=0 ),
            gvcommand.ArgDef( name = 'hscale', type = 'numeric', required=0 )
            ]
        
    def execute( self, args, line, interp ):
        
        dem_filename = args[0]
        drape_filename = args[1]
        if drape_filename is None:
            drape_filename = dem_filename
        if args[2] is None:
            mesh_lod = None
        else:
            mesh_lod = int(args[2])
        if args[3] is None:
            hscale = None
        else:
            hscale = float(args[3])

            
        view_win = gview.app.sel_manager.get_active_view_window()

        view_win.view3d_action( dem_filename, drape_filename, mesh_lod, hscale)

        return 1

###############################################################################
# Help Command

class GvHelpCommand(gvcommand.CommandBase):
    """
    Print help for a function or command.

    The help command is used to print the help for a function
    or command and/or to launch the help graphical user interface 
    (GUI).  The help GUI displays help for currently loaded commands 
    and functions, and may display help on other functions
    and commands if additional help files have been registered.
       
    The procedure followed by the help GUI to locate information
    is the following:

    1) Look for a function or command of the requested name in 
       the registered help file text, searching first commands,
       then functions.  If the function or command is loaded 
       locally,  try to determine the function or command's 
       parent module by searching the function/command's attributes.
       If text is found and the module name specified in 
       the file matches the function or command's parent
       module, this text will be displayed.  If the command or
       function's parent module cannot be found from a search
       of the function/command's attributes, the text will be 
       displayed anyway.

    2) If no suitable registered help is found but cf_name is
       recognized as a loaded command, print the command's usage
       string and group.

    3) If no registered help is found but cf_name is recognized
       as a function, print the function's __doc__ string.

    Note that the help GUI takes a "snapshot" of the shell at
    the time it is launched.  If you load new functions or
    commands and want to view help on them, relaunch the GUI
    or use the command line help.

    Parameters:

        cf_name- command or function name.

        /g- launch the help GUI.
        
    """

    def __init__(self):
        self.Name = 'help'
        self.Usage = 'help [cf_name] [-g]'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'cf_name', type = 'string_word',
                              required=0 ),
            gvcommand.ArgDef( name = 'g', type = 'switch', required=0 )
            ]

    def execute(self, args, line, interp):
        cf_name = args[0]
        gui_switch=args[1]
        
        import Numeric

        if ((cf_name is None) and (gui_switch == 0)):
            print interp.get_command_help('help')
            return 1
            

        if gui_switch == 1:
            # user requested GUI help
            import pyshell
            helpwin=pyshell.PyshellHelpDialog()
            if cf_name is not None:
                if cf_name in helpwin.total_cmd_keys:
                    txt=interp.get_command_help(cf_name)
                    if cf_name in helpwin.loaded_cmd_keys:
                        if txt is not None:
                            txt='\t\t\t'+cf_name+' (command)\n\n'+txt
                        else:
                            txt='\t\t\t'+cf_name+\
                            ' (command)\n\nNo help available.'
                    
                    else:
                        if txt is not None:
                            txt='\t\t\t'+cf_name+\
                            ' (command- not loaded)\n'+txt
                        else:
                            txt='\t\t\t'+cf_name+\
                            ' (command- not loaded)\n\nNo help available.'
                elif cf_name in helpwin.total_func_keys:
                    txt=interp.get_function_help(cf_name)
                    if cf_name in helpwin.loaded_func_keys:
                        if txt is not None:
                            txt='\t\t\t'+cf_name+\
                            ' (function)\n\n'+txt
                        else:
                            txt='\t\t\t'+cf_name+\
                            ' (function)\n\nNo help available.'
                    
                    else:
                        if txt is not None:
                            txt='\t\t\t'+cf_name+\
                            ' (function- not loaded)\n'+txt
                        else:
                            txt='\t\t\t'+cf_name+\
                            ' (function- not loaded)\n\nNo help available.'
                elif cf_name in helpwin.total_builtin_keys:
                    txt=interp.get_builtin_help(cf_name)
                    if cf_name in helpwin.loaded_builtin_keys:
                        if txt is not None:
                            txt='\t\t\t'+cf_name+\
                            ' (built-in function)\n\n'+txt
                        else:
                            txt='\t\t\t'+cf_name+\
                            ' (built-in function)\n\nNo help available.'
                    
                    else:
                        if txt is not None:
                            txt='\t\t\t'+cf_name+\
                            ' (built-in function- not loaded)\n'+txt
                        else:
                            txt='\t\t\t'+cf_name+\
                  ' (built-in function- not loaded)\n\nNo help available.'

                else:
                    txt=cf_name+\
                 ' not recognized as a command or function.\nNo help available'
                    
                helpwin.update_text(txt)
            
            return 1

        # GUI not requested
        if interp.cmdlist.has_key(cf_name):
            interp.showText('Command '+cf_name+':','report')
            txt=interp.get_command_help(cf_name)
            if txt is None:
                txt='No help available.\n'
            interp.showText(txt,'report')
        elif (interp.locals.has_key(cf_name) and
              (type(interp.locals[cf_name]) == type(Register))):
            interp.showText('Function '+cf_name+':','report')
            txt=interp.get_function_help(cf_name)
            if txt is None:
                txt='No help available.\n'
            interp.showText(txt,'report')
        elif (interp.locals.has_key(cf_name) and
              (type(interp.locals[cf_name]) == type(Numeric.cos))):
            interp.showText('ufunc '+cf_name+':','report')
            txt=interp.get_function_help(cf_name)
            if txt is None:
                txt='No help available.\n'
            interp.showText(txt,'report')
        elif (interp.locals.has_key(cf_name) and
              (type(interp.locals[cf_name]) == type(hasattr))):
            interp.showText('Built-in function '+cf_name+':','report')
            txt=interp.get_builtin_help(cf_name)
            if txt is None:
                txt='No help available.\n'
            interp.showText(txt,'report')               
        elif interp.get_command_help(cf_name) is not None:
            # command isn't loaded, but is registered
            interp.showText('Command '+cf_name+': (not loaded)','report')
            txt=interp.get_command_help(cf_name)
            interp.showText(txt,'report')
        elif interp.get_function_help(cf_name) is not None:
            # command isn't loaded, but is registered
            interp.showText('Function '+cf_name+': (not loaded)','report')
            txt=interp.get_function_help(cf_name)
            interp.showText(txt,'report')
        elif interp.get_builtin_help(cf_name) is not None:
            # command isn't loaded, but is registered
            interp.showText('Built-in function '+cf_name+': (not loaded)',
                            'report')
            txt=interp.get_function_help(cf_name)
            interp.showText(str(txt),'report')
            
        else:
            interp.showText('No help found for '+cf_name+'.','report')

        return 1

###############################################################################
# GvLocalsCommand(gvcommand.CommandBase):
class GvLocalsCommand(gvcommand.CommandBase):
    """
    List local variables and functions loaded/created by the user.  

    Parameters:

        type- type of object to list.  eg. locals array
              will list all Numeric python arrays currently
              in the environment.  type may also be a
              comma separated list of variable types to 
              display.  If type is not specified, all
              variables/functions loaded by the user will
              be listed.

    """
    
    def __init__(self):
        self.Name='locals'
        self.Usage = 'locals [type]'
                     
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'type', type = 'string_chunk')
            ]

    def execute(self, args, line, interp):
        searchtypes=args[0]
        import gvshell

        if ((searchtypes is not None) and (len(searchtypes) > 0)):
            import string
            typelist=string.split(searchtypes,',')
            txtlst=gvshell.local_vars_list(interp.locals,typelist)
        else:
            txtlst=gvshell.local_vars_list(interp.locals)

        for txt in txtlst:
            interp.showText(txt,'result')

        return 1

###############################################################################
# execute a series of pyshell lines (commands or functions) from a text file
#
# NOTE: This is really a placeholder- the interpreter should intercept macro
#       command lines at the top level and deal with them there because this
#       particular command could not be implemented using the GvCommand
#       structure (it would have resulted in nested interpreter "push" calls,
#       which lead to infinite loops because of buffer clearing issues).  The
#       SOLE purpose of putting this placeholder here is so that the various
#       help functions will treat the macro command as a regular command
#       (macro should look like any other command to the user).
#       
class GvMacroCommand(gvcommand.CommandBase):
    """
    Run a sequence of python statements and commands from a file.  

    The macro command will look for macro_file in the
    following locations, using the first one it finds:

    1) macro_file
    2) OPENEV_MACRO_PATH/macro_file
    3) OPENEVHOME/macros/macro_file
    4) OPENEV_HOME/macros/macro_file

    where OPENEV_MACRO_PATH, OPENEVHOME, and OPENEV_HOME
    are environment variables.  OPENEV_MACRO_PATH may 
    contain multiple search directories separated by
    semi-colons (;).

    Parameters:

        macro_file- name of the macro text file to run.
                    OpenEV macros are text files and must
                    start with the line:
                    # openev macro
                    
    """
    
    def __init__(self):
        self.Name = 'macro'
        self.Usage = 'macro <macro_file>'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'macro_file', type = 'string_word',
                              required=1 )
            ]

    def execute(self, args, line, interp):
        interp.showText('In macro placeholder command- should not be here!',
                        'error')
        return 1

    
############################################################
# Store commands to a file

class GvJournalCommand(gvcommand.CommandBase):
    """
    Store statements entered at the command line to a text file.

    Parameters:

        filename- file to store text in (required the first
                  time the journal command is entered in a
                  given session). 

        mode- whether to turn journaling on (/on) or off (/off).  
              Defaults to /on.

        umode- whether to append to filename (/a) or overwrite
               it (/w).  Defaults to /a.
               
    """
    
    def __init__(self):
        self.Name = 'journal'
        self.Usage = "journal <filename> [mode] [umode]" 
        
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'filename', type = 'string_word',
                              required=0 ),
            gvcommand.ArgDef( name = 'mode', type ='multi_switch', required=0,
                              valid_list=['-off','-on','/off','/on'] ), 
            gvcommand.ArgDef( name = 'umode', type = 'multi_switch',
                              required=0,
                              valid_list=['/w','-w','/a','-a'])            
            ]

    def execute(self, args, line, interp):
        fname = args[0]
        state=args[1]
        mode=args[2]

        # If journal file was open before,
        # close it.
        
        if interp.journal_fh is not None:
            interp.journal_fh.close()
            interp.journal_fh = None

        if (state is not None) and (state in ['-off','/off']):
            return 1

        if ((fname is not None) and (len(fname) == 0) and
            (interp.journal_fname is None)):
            fname = SelectFile("Journal File") 
            
        if (fname is not None) and (len(fname) > 0):
            interp.journal_fname=fname

        if interp.journal_fname is not None:
            if ((mode is not None) and ((mode == 'w') or
               (mode == '/w') or (mode == '-w'))):
                interp.journal_fh=open(interp.journal_fname,'w')
                interp.showText('overwriting '+interp.journal_fname,'result')
            else:
                interp.journal_fh=open(interp.journal_fname,'a')
                interp.showText('appending to '+interp.journal_fname, 'result')

            return 1
        else:
            interp.showText(
                'Unable to launch journaling- no filename specified','error')
            return 0

        
#############################################################################
# import a group of commands
class GvLoadextCommand(gvcommand.CommandBase):
    """
    Load a command extension module.

    The loadext command is used to load an extension
    module of OpenEV commands.  This is similar
    to the import keyword for pure python code.
    
    """
    
    def __init__(self):
        self.Name = 'loadext'
        self.Usage = 'loadext <extension_name>'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'extension_name', type = 'string_word',
                              required=1 )
            ]

    def execute(self, args, line, interp):

        # Get the module name
        module_name = args[0]

        # Get the current python shell
        import gview
        
        exec "import " + module_name
        exec "reload(" + module_name + ")"
        exec module_name + '.Register(gview.app.shell)'

        return 1

############################################################
# Get the currently active raster or vector file and load up the relevant part

class GvGetCommand(gvcommand.CommandBase):
    """
    Get data from the active OpenEV view/layer.

    The get command is used to grab data from the currently 
    active OpenEV view/layer and place it in variable
    <varname> in the python shell.  If the /s option
    is specified, the data grabbed will be screenshot-style; 
    otherwise, data will be retrieved from the underlying
    raster or shapes object.  

    If the view's active layer is a raster and a region of 
    interest (ROI) is drawn and /s is not specified, data will
    only be extracted from the ROI.  The ROI extracted is 
    always a rectangle, so if the view is in georeferenced 
    display mode, the corners will be reprojected to
    the raster's pixel/line space and a rectangle will
    be chosen to encompass the full area specified
    by the ROI plus extra on the edges to make it 
    rectangular.  ROI's are ignored if /s is specified.

    If the view's active layer is a shapes layer
    and shapes are selected, only those shapes will be 
    extracted.

    In all cases, it is a COPY of the data that is
    extracted; the original will be unchanged by changes
    to varname.

    Note that the /s option may not always work- some
    OpenGL drivers give flakey results. 

    Parameters:

        varname- name of python shell variable to extract 
                 data to (will overwrite any existing
                 variable by that name).
      
        /s- screenshot mode switch (off by default).
        
    """
    
    def __init__(self):
        self.Name = 'get'
        self.Usage = 'get <varname> [/s]'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'varname', type = 'string_token',
                              required=1 ),
            gvcommand.ArgDef( name = 's', type = 'switch', required=0 )
            ]

    def execute(self, args, line, interp):
 
        import gvutils
        import gview
        import gdalnumeric
        
        clayer = gview.app.sel_manager.get_active_layer()
        if clayer is None:
            interp.showText('No layer is currently active!','error')
            return 0
 

        try:
            roi=gview.app.toolbar.get_roi()
        except:
            roi=None

        # /s argument is 1 if user requested screenshot, 0
        # if the underlying data was requested (default).
        # NOTE: roi is ignored for screenshot option
        is_ss=args[1]
        shell_vars={}
        
        if is_ss == 1:
            cview=gview.app.sel_manager.get_active_view()
            if roi is not None:
                txt='Warning- ROI is ignored for screenshot-style get.\n'+\
                     'Grabbing whole view area.\n'
                interp.showText(txt,'error')

            # Note: for now, assume colour mode to start with (since
            # even single band images may have luts applied), and
            # if all three bands are identical in the end (greyscale
            # equivalent), return 1.

            err=cview.print_to_file(cview.get_width(),cview.get_height(),
                                    '_temp.tif','GTiff',1)

            import os
            
            if err != 0:
                interp.showText(
            'Error grabbing screenshot- unable to generate temporary file.\n',
            'error')
                os.unlink('_temp.tif')
                return 0

            try:
                import Numeric
                new_arr=gdalnumeric.LoadFile('_temp.tif')
                if ((max(Numeric.ravel(Numeric.fabs(new_arr[0,:,:] - new_arr[1,:,:])))
                     == 0) and
                    (max(Numeric.ravel(Numeric.fabs(new_arr[2,:,:] - new_arr[1,:,:])))
                     == 0)):
                    shp=Numeric.shape(new_arr)
                    new_arr=Numeric.reshape(new_arr[0,:,:],(shp[1],shp[2]))
            except:
                interp.showText(
                'Error grabbing screenshot- unable to load temporary file.\n',
                'error')
                os.unlink('_temp.tif')
                return 0

            shell_vars[args[0]]=new_arr
            os.unlink('_temp.tif')

            return(1,shell_vars)
        
        if gvutils.is_of_class(clayer.__class__,'GvRasterLayer'):
            ds=clayer.get_parent().get_dataset()
                
            if roi is None:
                shell_vars[args[0]]=gdalnumeric.DatasetReadAsArray(ds)
                return (1,shell_vars)
            else:
                # Here, need to check if georeferencing is on or not and
                # convert to pixel/line coordinates if it is on.
                cview=gview.app.sel_manager.get_active_view()                
                if (cview.get_raw(clayer) == 0):
                    # view is georeferenced- convert corners
                    [pixel,line] = clayer.view_to_pixel(roi[0],roi[1])
                    [pixel2,line2] = clayer.view_to_pixel(roi[0]+roi[2],
                                                          roi[1]+roi[3])
                    [pixel3,line3] = clayer.view_to_pixel(roi[0],
                                                          roi[1]+roi[3])
                    [pixel4,line4] = clayer.view_to_pixel(roi[0]+roi[2],
                                                          roi[1])

                    # Get pixel-space rectangle (offsets of 1 ensure that
                    # only pixels fully enclosed by the roi are included-
                    # int casting will round floating point pixel/line
                    # values down)
                    max_pix = int(max(pixel,pixel2,pixel3,pixel4))
                    min_pix = int(min(pixel,pixel2,pixel3,pixel4))+1
                    max_line = int(max(line,line2,line3,line4))
                    min_line = int(min(line,line2,line3,line4))+1

                    # in pixel/line space, selected region is a parallelogram
                    # but not necessarily a rectangle.  Choose a rectangle
                    # that fully encloses the parallelogram
                    roi = (min_pix,min_line,max_pix-min_pix,max_line-min_line)

                
                shell_vars={}
                shell_vars[args[0]]=gdalnumeric.DatasetReadAsArray(ds,roi[0],
                                                   roi[1],roi[2],roi[3]) 
                return (1,shell_vars)
        elif gvutils.is_of_class(clayer.__class__,'GvShapesLayer'):
            shps=clayer.get_parent()
            selected = clayer.get_selected()

            if len(selected) == 0:
                newshps = gview.GvShapes()
                for item in shps.get_schema():
                    newshps.add_field(item[0],item[1],item[2],item[3])
                for shp in shps:
                    if shp is not None:
                        newshps.append(shp.copy())
                    
                shell_vars={}
                shell_vars[args[0]]=newshps
                return (1,shell_vars)
            else:
                newshps = gview.GvShapes()
                for item in shps.get_schema():
                    newshps.add_field(item[0],item[1],item[2],item[3])
                for idx in selected:
                    newshps.append(shps[idx].copy())
                shell_vars={}
                shell_vars[args[0]]=newshps
                return (1,shell_vars)          
        else:
            interp.showText(
                'Active layer is not a raster or recognized vector layer!',
                'error')
            return 0

######################################################################
#
# Display a variable in a view
#
class GvShowCommand(gvcommand.CommandBase):
    """
    Display a variable in an OpenEV view.

    The show command displays variable varname in an
    OpenEV view.  

    Parameters:

        varname- python shell variable to display.  Must
                 be either a 1-D, 2-D, or 3-D Numeric python
                 array or a GvShapes variable.

        /nocopy- switch to indicate that the original
                 data must be displayed rather than a
                 copy.  Later changes made to varname will
                 be reflected in the view when the 
                 refresh button is pressed; and
                 changes made through the view will
                 be reflected in varname (eg. shapes
                 deletion).  Off by default.

        /o- switch to indicate that varname should be
            displayed in the current view rather than
            in a new view.
            
    """
    
    def __init__(self):
        self.Name = 'show'
        self.Usage = 'show <varname> [/nocopy] [/o]'
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'varname', type = 'variable',
                              required=1 ),
            gvcommand.ArgDef( name = 'nocopy', type = 'switch', required=0 ),
            gvcommand.ArgDef( name = 'o', type = 'switch', required=0 ) 
             ]

    def execute(self, args, line, interp):
        import gdalnumeric
        import gview

        data=args[0][0]
        dataname=args[0][1]
        if data is None:
            interp.showText('No input variable supplied','error')
            return 0
        
        ncswitch=args[1]
        vswitch=args[2]


        if vswitch == 0:
            import gview
            win = gview.app.new_view()
            if gview.app.shell.standalone == 1:
                item = win.menuf.find('File/Exit')
                if item is not None:
                    item.hide()
                item = win.menuf.find('File/New View')
                if item is not None:
                    item.hide()
                item = win.menuf.find('File/Save Project')
                if item is not None:
                    item.hide()

        import Numeric
        if type(data) == type(Numeric.zeros([1,1])):
            if ncswitch == 0:
                # By default, display a copy of the
                # data rather than the original
                import copy
                if len(Numeric.shape(data)) == 1:
                    # reshape 1x1 arrays so they can be
                    # displayed.
                    newdata=Numeric.reshape(data,(1,Numeric.shape(data)[0]))
                else:
                    newdata=copy.deepcopy(data)
            else:
                if len(Numeric.shape(data)) == 1:
                    txt='Vectors for display must have dimensions Nx1 or 1xN,\n'
                    txt=txt+'not N.  Either reshape the array, or turn off the nocopy\nswitch '
                    txt=txt+'so that show is permitted to reshape it for you.'
                    interp.showText(txt,'error')
                    return 0
                
                newdata=data
            
            try:
                # Only array data should get to here
                array_name = gdalnumeric.GetArrayFilename(newdata)
                ds = gview.manager.get_dataset( array_name )
                #if prototype_name is not None:
                #    prototype_ds = gdal.Open( prototype_name )
                #    gdalnumeric.CopyDatasetInfo( prototype_ds, ds )

                gview.app.file_open_by_name( array_name )
                return 1
            except:
                interp.showText('Unable to open array.','error')
                return 0
            
        elif type(data) == type(gview.GvShapes()):
            
            cview = gview.app.sel_manager.get_active_view_window()
            cview.make_active()

            # Get an okay layer name
            layer_list = cview.viewarea.list_layers()
            layer_map = {}           
            for clayer in layer_list:          
                layer_map[clayer.get_name()]=clayer
            counter = 0
                
            name = dataname
            while layer_map.has_key(name):
                counter = counter + 1
                name = dataname + '_'+str(counter)

            if ncswitch == 0:
                newdata=gview.GvShapes()
                for item in data.get_schema():
                    newdata.add_field(item[0],item[1],item[2],item[3])
                for shp in data:
                    if shp is not None:
                        newdata.append(shp.copy())
            else:
                newdata=data
                    
            newdata.set_name(name)
            gview.undo_register(newdata)
            layer = gview.GvShapesLayer(newdata)     
            cview.viewarea.add_layer(layer)
            return 1
        else:
            interp.showText('Unable to recognize input data type.','error')
            return 0

#######################################################################
#
# Save a variable to file            
#
class GvSaveCommand(gvcommand.CommandBase):
    """
    Save a python shell variable to a file.

    The save command saves python shell variable varname
    to file filename.

    Parameters:

        varname- python shell variable to save.  Must be
                 either a Numeric python array or a 
                 GvShapes object.

        filename- filename to save varname to.

        format- Only for varnames of Numeric python array
                type.  Raster file format to use in save.
                Must be a GDAL write-supported format.

        dataset- Only for varnames of Numeric python array
                 type.  Dataset to copy metadata from in
                 saving.
                 
    """
    
    def __init__(self):
        self.Name = 'save'
        self.Usage = 'save <varname> <filename> [format] [dataset]' 
        
        self.HelpURL = ''
        self.Group = 'core'
        self.Args = [
            gvcommand.ArgDef( name = 'varname', type = 'variable',
                              required=1 ),
            gvcommand.ArgDef( name = 'filename', type = 'string_word',
                              required=1 ),
            gvcommand.ArgDef( name = 'format', type='string_word',
                              required=0 ),
            gvcommand.ArgDef( name = 'dataset', type = 'variable',
                              required=0 )            
            ]                  

    def execute(self, args, line, interp):
        import gdalnumeric
        import gview

        data=args[0][0]
        dataname=args[0][1]
        fname = args[1]
        fmt = args[2]
        dataset=args[3]
        if dataset is not None:
            dataset=dataset[0]
        
        if data is None:
            interp.showText('No input variable supplied','error')
            return 0
        
        import Numeric
        if type(data) == type(Numeric.zeros([1,1])):
            
            # Only array data should get to here                
            if ((fmt is None) or (len(fmt) == 0)):
                fmt = 'GTiff'

            try:
                gdalnumeric.SaveArray(data,fname,fmt,dataset)
                return 1
            except ValueError:
                txt = fmt + ' format not available.  Available formats are:\n'
                import gdal
                for cDriver in gdal.GetDriverList():
                    txt = txt + cDriver.ShortName + ' '
                interp.showText(txt,'error')
            
        else:
            try:
                if fmt is not None:
                    txt='Warning: format option is only available for rasters.\n'
                    txt=txt+'         '+fname+\
                    ' will be saved in shapefile format.'
                    interp.showText(txt,'error')

                if data.save_to(fname) == 0:
                    interp.showText('Unable to save '+\
                                    dataname+' to file '+fname,'error')
                    return 0

                return 1
            except:
                interp.showText('Unable to save '+\
                                dataname+' to file '+fname,'error')

         
###############################################################################
# The Shell command.
class GvShellCommand(gvcommand.CommandBase):
    """
    Execute a shell command.

    The shell command executes the rest of the line as an external
    operating system command (ie. via a DOS or unix shell).  Output
    is redirected to the Python shell window.

    Note that the parent process is effectively blocked till the
    subcommand terminates.

    """

    def __init__(self):
        self.Name = 'shell'
        self.Usage = 'shell <operating system command>'
        self.HelpURL = ''
        self.Group = 'core'

        self.Args = [
            gvcommand.ArgDef( name = 'os_command', type = 'string_chunk' )
            ]

    def execute( self, args, line, anal_win ):

        import os
        #os.system( args[0] )

        out_fd = os.popen( args[0], 'r' )
        result = out_fd.read()
        print result

        return 1
        

def RegisterHelp( target ):
    # If the gvcorecmd_help.txt file
    # is present, register it.
    import os
    bpath,junk=os.path.split(__file__)
    hfname=os.path.join(bpath,'gvcorecmds_help.txt')
    if os.path.isfile(hfname):
        target.add_helpfile(hfname)

def Register( target ):

    target.add_command( GvCommandsCommand() )
    target.add_command( GvFunctionsCommand() )
    target.add_command( GvHelpCommand() )
    target.add_command( GvView3DCommand() )
    target.add_command( GvNewViewCommand() )
    target.add_command( GvClearViewCommand() )
    target.add_command( GvLoadextCommand() )
    target.add_command( GvMacroCommand() ) 
    target.add_command( GvJournalCommand() )  
    target.add_command( GvLocalsCommand() )
    target.add_command( GvGetCommand() )
    target.add_command( GvShellCommand() )    
    target.add_command( GvShowCommand() )    
    target.add_command( GvSaveCommand() )    
    
    RegisterHelp( target )
    
    
        
