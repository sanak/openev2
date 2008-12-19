#! /usr/bin/env python
###############################################################################
#
# Project:  OpenEV
# Purpose:  GTK interface to Python Shell
# Author:   Steve Rawlinson  srawlin@atlsci.com
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

MAX_COMMAND_PATHS = 15
MAX_MODULE_PATHS = 15

import gtk
from gtk.gdk import *
from gtk.keysyms import *
import code, string, sys, os
import gvutils

def launch(pyshellfile=None):
    import gview
    try:
        gview.app.shell.window.raise_()
        gview.app.shell.show()
    except:
        import gvcorecmds

        shell = Shell(pyshellfile=pyshellfile)
        gview.app.shell = shell

        # Initialization Commands
        shell.command('from numpy import *')
        shell.command('from osgeo.gdalnumeric import *')
        shell.command('from gvshell import *')
        shell.command('from gvplot import plot')
        shell.command('gview.shell_base_vars = dir()')
        shell.interp.clear_history()
        gvcorecmds.Register( shell )

        shell.show_all()

    return gview.app.shell

def launch_standalone(pyshellfile=None):
    import gview
    import gvcorecmds

    shell = Shell(pyshellfile=pyshellfile,standalone=1)
    gview.app.shell = shell

    # Initialization Commands 
    shell.command('from numpy import *')
    shell.command('from osgeo.gdalnumeric import *')
    shell.command('from gvshell import *')
    shell.command('from gvplot import plot')
    shell.command('gview.shell_base_vars = dir()')
    gvcorecmds.Register( shell )

    shell.show_all()



class MyInteractiveConsole(code.InteractiveConsole):

    def __init__(self, text_shell, inherit=None, history_list=None,
                 status_bar=None):

        # Note: text_shell is now TextView
        self.text_view = text_shell
        self.text_buff = self.text_view.get_buffer()

        locals = sys.modules['__main__'].__dict__
        if inherit:
            code.InteractiveConsole.__init__(self, locals=locals)
        else:
            code.InteractiveConsole.__init__(self)

        self.cmdlist = {}

        # GTK2 Port PENDING
        # style = text_shell.get_style()
        # self.fg = style.fg[STATE_NORMAL]
        # self.bg = style.white
        # self.font = load_font(
        #     "-*-courier-medium-r-normal-*-14-140-*-*-*-*-iso8859-*")

        # Avoid using GdkColor to allocate colours
        # self.redtext=self.text_shell.get_colormap().alloc(25000,0,0)
        # self.bluetext=self.text_shell.get_colormap().alloc(0,0,32757)
        # self.greentext=self.text_shell.get_colormap().alloc(0,25000,0)

        self.history_list = history_list
        self.status_bar = status_bar
        self.status_msg_id = None

        # Journaling-related variables
        # If journal_fh is None, journaling is off.
        self.journal_fh = None # file handle for journaling
        self.journal_fname = None # file name

        # List of helpfiles to check for command/
        # function help (full paths).
        self.helpfiles=[]
        self.help_cmdtxt={}
        self.help_functxt={}
        self.help_builtintxt={}

        # Macro flag:
        #     in_macro- use to avoid macro commands going into history buffer
        self.in_macro = 0 # Not currently in a macro

        # Error flag: use to exit potentially nested macros without completing
        #             them if an error is encountered.
        self.last_err = 0 # error status of last push command

    def clearFlags(self):
        # Function for top level shell to use to clear interpreter
        # macro and error flags.
        self.in_macro = 0
        self.last_err = 0

    def showProgress(self,percent,msg=None):
        if self.status_bar is None:
            return
        self.status_bar.progress_bar.set_percentage(percent/100.0)
        if msg is not None:
            if (self.status_msg_id is not None):
                self.status_bar.remove(self.status_bar.shell_context,self.status_msg_id)

            self.status_msg_id = self.status_bar.push(self.status_bar.shell_context,msg)

    def clear_history(self):
        if self.history_list is None:
            return
        self.history_list.freeze()
        self.history_list.clear()
        self.history_list.thaw()


    def push(self, line):
        if ((self.in_macro == 0) and (self.history_list is not None)):
            self.history_list.freeze()
            self.history_list.insert(0,([line]))
            # limit history list to 200 commands
            # (gtk doesn't like long clists)
            if self.history_list.rows > 200:
                for count in range(self.history_list.rows-200):
                    self.history_list.remove(200)          
            self.history_list.thaw()

        # locals replaced by a proper command    
        #if line[:6] == 'locals':
        #    line = 'local_vars_list(locals())'

        s_line=string.lstrip(line)
        if ((len(s_line) > 8) and (s_line[:6] == 'macro ')):
            fname=string.strip(s_line[6:])

            if ((len(fname) > 11) and (fname[:11] == 'macro_file=')):
                # User has specified macro_file keyword
                fname=fname[11:]

            # Search order for macro:
            # 1) fname
            # 2) OPENEV_MACRO_PATH/fname (multiple semi-colon separated
            #    paths may be specified in the OPENEV_MACRO_PATH
            #    environment variable).
            # 3) OPENEVHOME/fname
            # 4) OPENEV_HOME/fname
            #
            if not (os.path.isfile(fname)):
                macropaths=os.environ.get('OPENEV_MACRO_PATH')
                if macropaths is not None:
                    for mpath in string.split(macropaths,";"):
                        mspath=string.strip(mpath)
                        if os.path.isfile(os.path.join(mspath,fname)):
                            fname=os.path.join(mspath,fname)
                            break

            if not (os.path.isfile(fname)):
                oevpath=os.environ.get('OPENEVHOME')
                if oevpath is not None:
                    temp=os.path.join(oevpath,'macros')
                    temp=os.path.join(temp,fname)
                    if os.path.isfile(temp):
                        fname=temp

            if not (os.path.isfile(fname)):
                oevpath=os.environ.get('OPENEV_HOME')
                if oevpath is not None:
                    temp=os.path.join(oevpath,'macros')
                    temp=os.path.join(temp,fname)
                    if os.path.isfile(temp):
                        fname=temp


            if os.path.isfile(fname):
                # keep track of indentation, in case macros
                # are called within for loops of other
                # macros
                mac_indent_level=len(line)-len(s_line)
                indent_txt=''
                for count in range(mac_indent_level):
                    indent_txt=indent_txt+' '

                fh=open(fname)
                line1=fh.read(20)
                if (string.find(line1,"openev macro") > 0):
                    fh.seek(0)
                    fh.readline()
                    commandlines=fh.readlines()
                    for cline in commandlines:
                        # Treat each line of a macro as though it was
                        # entered at the command prompt.
                        cline=string.replace(cline,chr(10),"")
                        cline=indent_txt+cline
                        self.my_write(cline+chr(10),'command')

                        # Set in_macro flag before each command, in
                        # case there is a macro within a macro, and
                        # in_macro gets set to 0 after exiting the
                        # nested macro
                        self.in_macro=1

                        self.push(string.rstrip(cline))

                        # If an error has been encountered, return
                        # immediately leaving error flag intact so
                        # that higher-level macros can also detect
                        # it and exit.
                        #
                        # NOTE: later, may want to add an option
                        # to the macro command or a flag on the shell
                        # that tells macros to force their way through
                        # even if an error is encountered...
                        if self.last_err == 1:
                            return 0

                    self.history_pos = None
                    if self.history_list is not None:
                        self.history_list.unselect_all()

                    fh.close()
            else:
                self.showText(fname+' does not exist or is not a file.','error')

            self.in_macro=0
            return 0

        # If journaling is on, write the line unless the line
        # is itself a journal command
        if self.journal_fh is not None:
            if not ((len(line) >= 7) and (line[:7] == 'journal')):
                self.journal_fh.writelines(line+'\n')
                self.journal_fh.flush()


        if len(self.cmdlist) > 0:
            cmd_name, remainder = parse_interpreter_line(line)
            if self.cmdlist.has_key( cmd_name ):
                # HISTORICAL NOTE: the change from cc.execute()'ing at
                # this level to executing inside the shell through 
                # _run_command_line was necessary for commands to work
                # within a for-loop.  For instance:
                #
                # for i in range(3):
                #     print i
                #     newview
                #
                # Before, this used to immediately launch one view, then
                # print the integers 1, 2, and 3 on separate lines because
                # only the for-line and print-line actually got passed
                # through to the the interpreter.
                # The actual sequence of events was:
                # - push the line "for i in range(3):" into interpreter.
                #   Interpreter sees that this is an incomplete loop and
                #   stores it in a buffer, doing nothing.
                # - push the line "    print i" into interpreter.
                #   Since end of loop has not been detected, interpreter
                #   stores line in buffer.
                # - new view command is detected: run newview's
                #   execution code.
                # - push the line "" into the interpreter.  Interpreter
                #   now has a complete loop that can be executed, executes
                #   it, and flushes the buffer.
                #
                # Changing the code so that
                # it is executed within the interpreter allows the newview
                # to launch as one would expect because the code is now
                # all executed in the context of the interpreter:
                #
                # for i in range(3):
                #     print i
                #     _run_command_line('newview')
                #
                # This does introduce one complication: COMMANDS MUST NOT
                # CALL THE interp.push OR code.InteractiveConsole.push
                # FUNCTIONS IN THEIR EXECUTION CODE BECAUSE THE BUFFER 
                # FOR EACH PUSH COMMAND IS NOT CLEARED UNTIL IT COMPLETES,
                # SO CALLING PUSH WITHIN PUSH RESULTS IN A RECURSIVE LOOP.

                # Old code
                #cc.execute()
                #return code.InteractiveConsole.push(self,'')
                # end of old code

                # Get the indentation level of the line
                temp_line=string.lstrip(line)
                indent_level=len(line)-len(temp_line)
                txt=''
                for count in range(indent_level):
                    txt=txt+' '

                if string.find(line,'"') == -1:
                    txt=txt+'_run_command_line("'+line+'")'
                elif string.find(line,"'") == -1:
                    txt=txt+"_run_command_line('"+line+"')"
                else:
                    line2=string.replace(line,"'",'"')
                    txt=txt+"_run_command_line('"+line2+"')"                        

                return code.InteractiveConsole.push(self,txt)


        return code.InteractiveConsole.push(self,line)

    # This is a CommandInterpreter method as per gvcommand.py
    def isInteractive( self ):
        return 1

    # This is a CommandInterpreter method as per gvcommand.py
    def showText( self, text, text_class ):
        text = text + chr(10)
        if text_class == 'error':
            self.my_write( text, 'stderr' )
        else:
            self.my_write( text, 'stdout' )


    def showtraceback(self):
        """Display the exception that just occurred.

        We remove the first stack item because it is our own code.

        The output is written by self.write(), below.

        """
        try:
            import traceback
            if sys.exc_info()[2] is not None:
                exc_info = traceback.extract_tb(sys.exc_info()[2])
                txt = 'Unexpected Error:'
                txt = txt + '\n  Type  : '+str(sys.exc_type)                
                if sys.exc_info()[1] is None: 
                    txt = txt + '\n  Description: '+ 'Undefined\n'

                else:       
                    txt = txt + '\n  Description: '+str(sys.exc_info()[1]) + '\n'

                if len(exc_info) > 2:
                    # Ignore the first 2 tuples- in the context of the interpreter,
                    # these are just the console and the exec statement and are
                    # irrelevant
                    txt = txt + '\n  Traceback:\n'
                    for ctuple in exc_info[2:]:
                        txt = txt + '\n    file       : '+os.path.basename(ctuple[0])
                        txt = txt + '\n    line number: '+str(ctuple[1])
                        txt = txt + '\n    function   : '+str(ctuple[2])
                        txt = txt + '\n    line       : '+str(ctuple[3])+'\n'

            else:
                txt = 'Unexpected Error:'
                txt = txt + '\n    No description available.\n'
        except:
            txt = 'Unexpected Error:'
            txt = txt + '\n    No description available.\n'

        self.write(txt)

        # Status of last line pushed via toplevel shell's echo function
        self.last_err = 1

    def write(self, data):
        # Override base class write
        # Red - tracebacks
        # This method is only used when an error has occurred (eg. a
        # syntax error)- stdout is redirected to my_write.

        # self.text_shell.insert(self.font, self.redtext, self.bg, data)

        self.text_buff.insert_at_cursor(data)

        self.last_err = 1

    def my_write(self, data, name):
        # self.text_shell.freeze()
        # Comment line just below this is deprecated- GdkColor isn't used now
        # Colours need to be in function calls for some reason else segfaults!?!
        def insert(data):
            buffer = self.text_view.get_buffer()

            start, end = buffer.get_bounds()
            mark = buffer.create_mark(None, end, False)

            buffer.insert(end, data)

            #self.text_view.scroll_to_mark(mark, 0, True, 0.0, 1.0)
            self.text_view.scroll_mark_onscreen(mark)
            buffer.delete_mark(mark)


        # Normal output is reported in GREEN
        if name == 'stdout':
            insert(data)

        # Anticipated errors are reported in BLUE
        elif name == 'stderr':
            insert(data)

            # self.text_shell.insert(self.font, self.bluetext, self.bg, data)
            # should still set error flag
            self.last_err = 1
        # Titles are bigger and BLUE
        elif name == 'title':
            insert(data)
            # self.text_shell.insert( 
            #   load_font("-*-courier-bold-o-normal-*-14-140-*-*-*-*-iso8859-*"),
            #   self.bluetext,
            #   self.bg, data )

        # Everything else is just the normal foreground color (black)
        else:
            insert(data)

        # self.text_shell.thaw()

        try:
            # This offset seems to be necessary on windows, which
            # segfaults if vscrollbar.upper or larger values are
            # used.  
            hoffset=self.text_view.window.height
            vscrollbar.set_value( vscrollbar.upper - hoffset )
        except:
            pass

    def get_command_help(self, command,quiet=0):
        # quiet- if quiet is 0, include module,
        #        and group info; otherwise don't.

        if self.cmdlist.has_key(command):
            # Command is loaded.  Determine what module
            # it is in.
            if hasattr(self.cmdlist[command],'__module__'):
                modname=os.path.basename(self.cmdlist[command].__module__)
                mname,ext=os.path.splitext(modname)      
            else:
                mname=None

            # check for registered help for command
            # under module mname.
            if self.help_cmdtxt.has_key(command):
                if mname is None:
                    # If module unknown, return all
                    # text help.
                    ctext=''
                    for centry in self.help_cmdtxt[command]:
                        if quiet == 0:
                            ctext=ctext+'Module: '+centry[0]+'\n'
                            ctext=ctext+'Group: '+centry[1]+'\n\n'

                        ctext=ctext+centry[3] + '\n'
                    return ctext

                for centry in self.help_cmdtxt[command]:
                    if centry[0] == mname:
                        ctext=''
                        if quiet == 0:
                            ctext='Module: '+centry[0]+'\n'
                            ctext=ctext+'Group: '+centry[1]+'\n\n'

                        ctext=ctext+centry[3] + '\n'

                        return ctext


            # No suitable text help found, but command found so
            # construct basic help.
            if quiet == 0:
                txt='Module: '
                if mname is None:
                    txt=txt+'Unknown\n'
                else:
                    txt=txt+mname+'\n'

                if hasattr(self.cmdlist[command],'Group'):
                    txt=txt+'Group: '+self.cmdlist[command].Group + '\n\n'
                else:
                    txt='Group: None\n\n'
            else:
                txt=''

            txt=txt+'Usage: '+self.cmdlist[command].Usage + '\n'
            if hasattr(self.cmdlist[command],'__doc__'):
                txt=txt+_format_doc(self.cmdlist[command].__doc__)+'\n'

            return txt

        elif self.help_cmdtxt.has_key(command):
            # If module unknown, return all
            # text help.
            ctext=''
            for centry in self.help_cmdtxt[command]:
                if quiet == 0:
                    ctext=ctext+'Module: '+centry[0]+'\n'
                    ctext=ctext+'Group: '+centry[1]+'\n'

                ctext=ctext+centry[3] + '\n\n'

            return ctext

        else:                            
            return None    


    def get_function_help(self, func, module_name=None, quiet=0):
        """ Search for documentation on function func.
            If module_name is set to None, search
            through the local shell variables for func
            and try to determine func's module from
            its attributes; otherwise, look for any
            help on function func.
            If module_name is not None, check if
            a func from module module_name is in the shell
            variables or in the help.  If not, try to
            import it.
        """
        import numpy as Numeric

        dtxt=None # default text
        # mname- name of module for shell variable func
        #        if func is present in shell.
        mname=None 

        # Get default python documentation, but don't return
        # it unless module name is okay.
        if ((self.locals.has_key(func)) and
              (type(self.locals[func]) == type(launch)) and
              (hasattr(self.locals[func],'__doc__'))):

            if quiet == 0:
                if hasattr(self.locals[func],'func_code'):
                    modname=os.path.basename(self.locals[func].func_code.co_filename)
                    mname,ext=os.path.splitext(modname)
                    dtxt='Module: '+mname+'\n\n'      
                else:
                    dtxt='Module: unknown\n\n'
            else:
                dtxt=''

            dtxt=dtxt+_format_doc(self.locals[func].__doc__) + '\n'

        elif ((self.locals.has_key(func)) and
              (type(self.locals[func]) == type(Numeric.cos)) and
              (hasattr(self.locals[func],'__doc__'))):

            # Can't determine module from a ufunc's attributes
            if quiet == 0:
                dtxt='Universal function (ufunc)\n\n'
            else:
                dtxt=''

            dtxt=dtxt+_format_doc(self.locals[func].__doc__) + '\n'

        if self.help_functxt.has_key(func):
            # If user specified particular module to
            # search, only return help for that
            # module.
            if module_name is not None:
                for centry in self.help_functxt[func]:
                    if centry[0] == module_name:
                        ctext=''
                        if quiet == 0:
                            ctext='Module: '+centry[0]+'\n\n'

                        ctext=ctext+centry[2]
                        return ctext

            # If module name was not specfied, but
            # func is present in the shell and module
            # can be determined from func, return
            # help for that module only.
            elif mname is not None:
                for centry in self.help_functxt[func]:
                    if centry[0] == mname:
                        ctext=''
                        if quiet == 0:
                            ctext='Module: '+centry[0]+'\n\n'

                        ctext=ctext+centry[2]
                        return ctext

            # If no module specified or found, return
            # all text file help available on func.
            else:
                ctext=''
                for centry in self.help_functxt[func]:
                    if quiet == 0:
                        ctext=ctext+'Module: '+centry[0]+'\n\n'

                    ctext=ctext+centry[2]+'\n\n'

                return ctext

        # At this point, no suitable text file help has been found
        if ((module_name is not None) and
            (mname != module_name)):
            # shell variable doesn't match requested module.
            # try to import module and create help.
            try:
                exec 'import '+module_name
                exec 'funcinst='+module_name+'.'+func
                exec 'docstr=funcinst.__doc__'
            except:
                # couldn't load module or function, or couldn't
                # locate documentation string once loaded
                return None

            if (type(funcinst) == type(launch)):
                if quiet == 0:
                    dtxt='Module: '+module_name+'\n\n'
                else:
                    dtxt=''
                dtxt=dtxt+_format_doc(docstr)+'\n'
                return dtxt

            elif (type(funcinst) == type(Numeric.cos)):
                if quiet == 0:
                    dtxt='Module: '+module_name+'\n\n'
                    dtxt=dtxt+'Universal function (ufunc)\n\n'
                else:
                    dtxt=''
                dtxt=dtxt+_format_doc(docstr)+'\n'
                return dtxt

            else:
                # Not a recognized non-builtin function
                # (type must be either function or ufunc)
                return None

        else:
            return dtxt


    def get_builtin_help(self, func, module_name=None,quiet=0):
        if self.help_builtintxt.has_key(func):
            if module_name is not None:
                for centry in self.help_builtintxt[func]:
                    if centry[0] == module_name:
                        ctext=''
                        if quiet == 0:
                            ctext='Module: '+centry[0]+'\n\n'

                        ctext=ctext+centry[2]
                        return ctext
            else:
                ctext=''
                for centry in self.help_builtintxt[func]:
                    if quiet == 0:
                        ctext=ctext+'Module: '+centry[0]+'\n\n'

                    ctext=ctext+centry[2]+'\n\n'

                return ctext

        # If code gets to here, no suitable help
        # for func has been found.
        if module_name is None:
            if ((self.locals.has_key(func)) and
                  (type(self.locals[func]) == type(hasattr)) and
                  (hasattr(self.locals[func],'__doc__'))):   
                txt=_format_doc(self.locals[func].__doc__) + '\n'
                return txt
            else:
                try:
                    # builtin function
                    txt=_format_doc(self.locals['__builtins__'][func].__doc__)+ '\n'
                    return txt
                except:
                    return None

        # No help has been found yet, and specific module has been requested
        try:
            exec 'import '+module_name
            exec 'funcinst='+module_name+'.'+func
            exec 'docstr=funcinst.__doc__'
            if (type(funcinst) == type(hasattr)):
                if quiet == 0:
                    txt='Module: '+module_name+'\n\n'
                else:
                    txt=''
                txt=txt+_format_doc(docstr) + '\n'
                return txt
            else:
                return None
        except:
            # couldn't load module or function, or couldn't
            # locate documentation string once loaded
            return None

    def add_helpfile(self, helpfilename):
        if helpfilename in self.helpfiles:
            # Already registered.
            return 1

        if os.path.isfile(helpfilename) == 0:
            self.showText('Warning: help file '+helpfilename+'\n does not exist','error')
            return 0

        self.helpfiles.append(helpfilename)
        fh=open(helpfilename)
        helplines=fh.readlines()
        ckey=None
        ctext=''
        ctype=None

        # add a dummy line to the end to force the last
        # command to be assigned (since assignment is
        # done once a new command is started).
        helplines.append('COMMAND_NAME=dummyhelpline')

        for cline in helplines:
            # linetype: 0 if an ordinary line is read, 1 if help for
            # a new command is being defined, 2 if help for a new
            # function is being defined, 3 if help for a new
            # builtin function is being defined.
            linetype=0
            if(( len(cline) > 13) and (cline[:13] == 'COMMAND_NAME=')):
                linetype=1

            if(( len(cline) > 14) and (cline[:14] == 'FUNCTION_NAME=')):
                linetype=2

            if(( len(cline) > 13) and (cline[:13] == 'BUILTIN_NAME=')):
                linetype=3

            if (linetype > 0):
                if (ckey is not None) and (ctype is not None):
                    # Assign last function/command's text, if
                    # present, before going on to next one.
                    if ctype == 'cmd':
                        # split help text into module, group,
                        # html filename, text.
                        parsed_help=_parse_cmdhelp_text(ctext)
                        if parsed_help is None:
                            errtxt='Invalid helpfile '+helpfilename+\
                                    ':\nBad entry for command'+ckey+'.'
                            raise errtxt

                        mname=parsed_help[0]
                        gname=parsed_help[1]
                        hname=parsed_help[2]
                        ctext=parsed_help[3]

                        if self.help_cmdtxt.has_key(ckey):
                            # Some help has already been registered
                            # for a function of this name.
                            # Check if module already has help registered.
                            conflict=0
                            for idx in range(len(self.help_cmdtxt[ckey])):
                                item=self.help_cmdtxt[ckey][idx]
                                if item[0] == mname:
                                    txt='Warning: multiple sets of help found for '+\
                                        'command '+ckey+', module '+mname+'.'+\
                                        '\nIgnoring all but first.\n'
                                    self.showText(txt,'error')
                                    conflict=1

                            if conflict == 0:
                                self.help_cmdtxt[ckey].append([mname,gname,hname,ctext])
                        else:
                            self.help_cmdtxt[ckey]=[]
                            self.help_cmdtxt[ckey].append([mname,gname,hname,ctext])                          

                    elif ctype == 'blt':
                        parsed_help=_parse_funchelp_text(ctext)
                        if parsed_help is None:
                            errtxt='Invalid helpfile '+helpfilename+\
                                    ':\nBad entry for built-in function '+ckey+'.'
                            raise errtxt

                        mname=parsed_help[0]
                        hname=parsed_help[1]
                        ctext=parsed_help[2]

                        if self.help_builtintxt.has_key(ckey):
                            # Some help has already been registered
                            # for a function of this name
                            # Check if module already has help registered.
                            conflict=0
                            for idx in range(len(self.help_builtintxt[ckey])):
                                item=self.help_builtintxt[ckey][idx]
                                if item[0] == mname:
                                    txt='Warning: multiple sets of help found for '+\
                                        'built-in function '+ckey+', module '+mname+'.'+\
                                        '\nIgnoring all but first.'
                                    self.showText(txt,'error')
                                    conflict=1

                            if conflict == 0:
                                self.help_builtintxt[ckey].append([mname,hname,ctext])
                        else:
                            self.help_builtintxt[ckey]=[]
                            self.help_builtintxt[ckey].append([mname,hname,ctext])
                    else:
                        parsed_help=_parse_funchelp_text(ctext)
                        if parsed_help is None:
                            errtxt='Invalid helpfile '+helpfilename+\
                                    ':\nBad entry for function '+ckey+'.'
                            raise errtxt

                        mname=parsed_help[0]
                        hname=parsed_help[1]
                        ctext=parsed_help[2]

                        if self.help_functxt.has_key(ckey):
                            # Some help has already been registered
                            # for a function of this name
                            # Check if module already has help registered.
                            conflict=0
                            for idx in range(len(self.help_functxt[ckey])):
                                item=self.help_functxt[ckey][idx]
                                if item[0] == mname:
                                    txt='Warning: multiple sets of help found for '+\
                                        'function '+ckey+', module '+mname+'.'+\
                                        '\nIgnoring all but first.'
                                    self.showText(txt,'error')
                                    conflict=1

                            if conflict == 0:
                                self.help_functxt[ckey].append([mname,hname,ctext]) 
                        else:
                            self.help_functxt[ckey]=[]
                            self.help_functxt[ckey].append([mname,hname,ctext])

            if (linetype == 1):                 
                junk,ckey = string.split(cline,'=',1)
                ckey=string.strip(ckey)
                ctext=''
                ctype='cmd'
            elif (linetype == 2): 
                junk,ckey = string.split(cline,'=',1)
                ckey=string.strip(ckey)
                ctext=''
                ctype='fnc'
            elif (linetype == 3):
                junk,ckey = string.split(cline,'=',1)
                ckey=string.strip(ckey)
                ctext=''
                ctype='blt'                
            else:
                ctext=ctext+cline

        return 1

class PseudoFile:
# To send stdout to our console

    def __init__(self, shell, name):
        self.shell = shell
        self.name = name

    def write(self, s):
        self.shell.my_write(s, self.name)
        # self.shell.write(s)

        from osgeo import gdal
        gdal.Debug( "stderr", s )

    def writelines(self, l):
        map(self.write, l)

    def flush(self):
        pass

    def isatty(self):
        return 1


# Creates interactive Python Shell
#
# inherit - true if parent's python environment be inherited
#           None if it shouldn't and want a clean environment

class Shell(gtk.Window):
    def __init__(self, inherit=None, width=550, height = 250, standalone=0,pyshellfile=None):

        # Main Window, buttons
        gtk.Window.__init__(self)
        self.set_title('Python Shell')
        self.set_border_width(3)

        self.pyshellfile=pyshellfile


        if pyshellfile is not None:
            guicmds=self.load_pyshell_file_from_xml(self.pyshellfile)
        else:
            # If pyshellfile is None, default to old appearance.

            #menucmds=[]
            #menucmds.append(self.__get_standard_menu_entries())
            #guicmds=(menucmds,None,None,None)
            guicmds=(None,None,None,None)

        # Use a paned window if the history area is present; a normal
        # vbox if it isn't.       
        # panel for menu, messages, icons, command line
        if (guicmds[2] is not None):
            pane1=gtk.VPaned()
            vbox = gtk.VBox(homogeneous=False,spacing=2)
            self.add(pane1)
            pane1.add1(vbox)
        else:
            vbox=gtk.VBox(homogeneous=False,spacing=2)
            self.add(vbox)

        self.standalone = standalone

        # Path Preferences
        # Currently, the module and script paths
        # are treated the same way- they are added
        # to the system path, and python will search
        # all three for commands, modules.
        self.preferences={}

        import gview
        import sys

        mpathstring=""
        for i in range(1,MAX_MODULE_PATHS+1):
            mpath = gview.get_preference('pyshell_module_path'+str(i))
            if mpath is not None:
                if mpath not in sys.path:
                    sys.path.append(mpath)
                if len(mpathstring) == 0:
                    mpathstring=mpath
                else:
                    mpathstring=mpathstring+";"+mpath

        cpathstring=""
        for i in range(1,MAX_COMMAND_PATHS+1):
            cpath = gview.get_preference('pyshell_command_path'+str(i))
            if cpath is not None:
                if cpath not in sys.path:
                    sys.path.append(cpath)
                if len(cpathstring) == 0:
                    cpathstring=cpath
                else:
                    cpathstring=cpathstring+";"+cpath

        self.preferences['MODULE PATHS']=mpathstring    
        self.preferences['COMMAND PATHS']=cpathstring

        self.path_dlg = None

        # Menu
        if (guicmds[0] is not None):
            menuf = gvutils.GvMenuFactory()
            self.menuf = menuf
            if self.standalone != 0:
                self.close_cb = self.close
            else:
                self.close_cb = self.destroy

            for cmd in guicmds[0]:
                exec cmd

            self.add_accel_group(menuf.accelerator)
            vbox.pack_start(menuf,expand=False)
        else:
            self.menuf = None

        # Iconbar
        if (guicmds[1] is not None) and (len(guicmds[1]) > 0):
            self.iconbar = gtk.Toolbar()
            for cmd in guicmds[1]:
                exec cmd

            vbox.pack_start(self.iconbar,expand=False)
        else:
            self.iconbar = None

        top_console = gtk.HBox(homogeneous=False, spacing=2)
        vbox.pack_start(top_console, expand=True)


        # GTK2 Port...
        #text = gtk.Text()
        #text.set_editable(False)
        #top_console.pack_start(text, expand=True)
        text_buff = gtk.TextBuffer()
        text_window = ScrollableTextView(text_buff)
        text_view = text_window._textview
        text_view.set_editable(False)
        text_view.set_wrap_mode(gtk.WRAP_WORD)
        top_console.pack_start(text_window, expand=True)

        # Scrollbars

        #scroll = gtk.VScrollbar()
        #scroll.set_update_policy(gtk.UPDATE_CONTINUOUS)
        #adj = scroll.get_adjustment()
        #adj.lower = 0.0
        #adj.upper = 1.0
        #adj.value = .0
        #text_view.set_focus_vadjustment(adj)
        #top_console.pack_start(scroll, expand=False)

        #Horizontal separator bar
        vbox.pack_start(gtk.HSeparator(), expand=False)

        # Prompt Text Area
        # GTK2 Port...
        #prompt = gtk.Text()
        #prompt.set_size_request(550, 50)
        #prompt.set_editable(True)
        #vbox.pack_start(prompt, expand=False)
        prompt_buff = gtk.TextBuffer()
        prompt_view = gtk.TextView(prompt_buff)
        prompt_view.set_size_request(550, 50)
        prompt_view.set_editable(True)
        vbox.pack_start(prompt_view, expand=False)

        prompt_view.grab_focus()

        if guicmds[2] is not None:
            vbox2=gtk.VBox()
            pane1.add2(vbox2)
            #Horizontal separator bar
            hlabel = gtk.Label('Command History')
            hlabel.set_justify(gtk.JUSTIFY_LEFT)
            #vbox2.pack_start(gtk.HSeparator(), expand=False)
            histhbox=gtk.HBox()
            vbox2.pack_start(histhbox,expand=False,fill=False)
            histhbox.pack_start(hlabel,expand=False,fill=False)
            # History list
            histbox = gtk.ScrolledWindow()
            vbox2.pack_start(histbox)
            histlist = gtk.CList(cols=1)

            # Changed from add_with_viewport to
            # add to avoid gtk. warnings about
            # size allocation for long history
            # lists (> about 65 lines).  Not sure
            # why this is necessary, but someone
            # else had fixed the problem that way
            # according to an email found with google.
            # histbox.add_with_viewport(histlist)
            histbox.add(histlist)

            histlist.set_selection_mode(SELECTION_SINGLE)
            histlist.connect('button-press-event',self.list_clicked)
        else:
            histlist = None

        if guicmds[3] is not None:
            #Horizontal separator bar
            # vbox2 is only created if the history area is created
            # (paned window is only necessary in that case currently).
            if guicmds[2] is not None:
                vbox2.pack_start(gtk.HSeparator(), expand=False)
            else:
                vbox.pack_start(gtk.HSeparator(), expand=False)
            self.status_bar = gtk.Statusbar()            
            self.status_bar.progress_bar = gtk.ProgressBar()
            self.status_bar.shell_context = self.status_bar.get_context_id('shell')
            self.status_bar.pack_start(self.status_bar.progress_bar, expand=False)
            if guicmds[2] is not None:
                vbox2.pack_start(self.status_bar, expand=False)
            else:
                vbox.pack_start(self.status_bar, expand=False)                
        else:
            self.status_bar = None

        # Setup up the size of the dialog
        totalheight = height
        if self.menuf is not None:
            totalheight=totalheight + 50
        if self.iconbar is not None:
            totalheight=totalheight + 50
        if histlist is not None:
            totalheight=totalheight + 250

        self.set_size_request(width, totalheight)
        self.set_resizable(True)

        # Text properties GTK2 Port PENDING...
        #style = text.get_style()
        #self.fg = style.fg[STATE_NORMAL]
        #self.bg = style.white
        #self.font = load_font(
        #    "-*-courier-medium-r-normal-*-14-140-*-*-*-*-iso8859-*")        

        vbox.show_all()
        self.history_list = histlist
        self.text_view = text_view
        self.text_buff = text_buff
        self.prompt_buff = prompt_buff
        self.prompt_view = prompt_view

        # Environment variables
        self.prompt_state = 0
        self.history_buffer = []
        self.history_pos = None

        # watch for key presses such as returns and Ctrl-D
        ###self.prompt_view.connect_after('key-press-event', self.echo)
        self.prompt_view.connect('key-press-event', self.echo)

        # Setup actual python interpreter
        self.interp = MyInteractiveConsole(text_view, inherit, self.history_list,
                                           self.status_bar)

        # Redefine stdout and stderr so they go to our console
        sys.stdout = PseudoFile(self.interp, 'stdout')

        # May 2003- commented out the stderr pseudofile- general
        # openev errors probably shouldn't go to python shell.
        #sys.stderr = PseudoFile(self.interp, 'stderr')

        self.set_prompt()
        self.interp.my_write( \
            '      --== Interactive Python Interpreter ==--\n\n',
            'title')

        # Make sure that function to parse command lines is loaded.
        self.interp.push('from pyshell import _run_command_line')

        # Standalone property: 0 if shell launched
        # from OpenEV or similar application; 1 if
        # shell launched alone.  Shells that are
        # standalone will connect to the gtk quit event.

        if self.standalone != 0:
            self.connect('delete-event',self.close)

    def preferences_cb(self,*args):
        self.pref_gui_cb()

    def launch_help_cb(self,*args):
        PyshellHelpDialog()

    def close(self,*args):
        dialog = gtk.Dialog('Confirmation',
                     gview.app.shell.window,
                     gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                     (gtk.STOCK_YES, gtk.RESPONSE_YES,
                      gtk.STOCK_NO, gtk.RESPONSE_NO))
        label = gtk.Label('Are you sure you want to exit OpenEV Command Shell?')
        dialog.vbox.pack_start(label, True, True, 0)
        label.show()

        response = dialog.run();

        if response == gtk.RESPOSE_YES:
            import gview
            gview.save_preferences() # save path preferences           
            gview.app.quit()

        return True

    def show_progress(self,percent,msg=None):
        if self.status_bar is None:
            return

        self.interp.showProgress(percent,msg)

        # Make sure that progress bar updates
        # immediately
        while gtk.events_pending():
            gtk.main_iteration()

    def echo(self, text, event, *args):
        # Watch for Returns - Main processing loop here!!!!
        m = self.prompt_buff.get_iter_at_mark(self.prompt_buff.get_insert())
        if event.keyval == gtk.keysyms.Return:
            aa, bb = self.prompt_buff.get_bounds()
            input = str(self.prompt_buff.get_text(aa, bb))
            # input = str(self.prompt_buff.get_text(self.prompt_buff.get_bounds()))
            # input = str(self.prompt.get_chars(0, -1))

            # Remove any internal newlines.
            input = string.replace(input,chr(10),"")

            # echo command in text dialog
            self.append_text(input+chr(10))

            # Pick out command, and strip trailing white space (newline)
            command = string.rstrip(input[4:])

            self.interp.clearFlags()
            self.prompt_state = self.interp.push(command)

            # Add command to history buffer
            self.history_buffer.insert(0, command)

            # Reset the prompt if needed, and history position
            self.set_prompt()
            self.history_pos = None
            if self.history_list is not None:
                self.history_list.unselect_all()

            return True

        # Watch for Ctrl-D
        elif (event.keyval in (ord('d'), ord('D'))) \
                 and (event.state & gtk.gdk.CONTROL_MASK):
            aa, bb = self.prompt_buff.get_bounds()
            input = str(self.prompt_buff.get_text(aa, bb))
            print "input ->", input

            # input = str(self.prompt_buff.get_text(self.prompt_buff.get_bounds()))
            # input = str(self.prompt.get_chars(0, -1))
            if len(input) == 4:
                if self.standalone != 0:
                    import gview
                    gview.app.quit()
                else:
                    self.destroy()

        # Up Arrow - back in history list
        elif event.keyval == gtk.keysyms.Up:
            if len(self.history_buffer) > 0:
                self.set_prompt()

                if self.history_pos is None:
                    self.history_pos = -1

                if self.history_pos < len(self.history_buffer) - 1:
                    self.history_pos = self.history_pos + 1

                # GTK2 Port... PENDING
                self.prompt_buff.insert_at_cursor(self.history_buffer[self.history_pos])
                # self.prompt.insert(self.font, self.fg, self.bg, self.history_buffer[self.history_pos])

                if self.history_list is not None:
                    if self.history_pos < self.history_list.rows:
                        self.history_list.select_row(self.history_pos,0)

            return True

        # Down Arrow - forward in history list
        elif event.keyval == gtk.keysyms.Down:
            if (self.history_pos is not None) and (self.history_pos > 0):
                self.set_prompt()
                self.history_pos = self.history_pos - 1
                self.prompt_buff.insert_at_cursor(self.history_buffer[self.history_pos])
                # self.prompt.insert(self.font, self.fg, self.bg, self.history_buffer[self.history_pos])

                # select relevant row in history list
                if self.history_list is not None:
                    if self.history_pos < self.history_list.rows:
                        self.history_list.select_row(self.history_pos,0)

            elif (self.history_pos is not None) and (self.history_pos == 0):
                self.set_prompt()
                self.history_pos = None
                if self.history_list is not None:
                    self.history_list.unselect_all()
            return True


        # Make sure we don't delete the prompt
        elif (event.keyval == gtk.keysyms.BackSpace) and (m.get_offset() < 5):
            self.prompt_buff.insert_at_cursor(' ')
            # self.prompt.insert(self.font, self.fg, self.bg, ' ')

        # Watch user doesn't uses arrow keys to move into prompt
        insert_iter = self.prompt_buff.get_iter_at_mark(self.prompt_buff.get_insert())
        if not insert_iter.backward_chars(4):
            insert_iter.set_offset(4)
            self.prompt_buff.place_cursor(insert_iter)


        return False

    def list_clicked(self, lst, event):
        if self.history_list is None:
            return

        try:
            row, col = lst.get_selection_info(int(event.x), int(event.y))
        except:
            return

        if event.button == 1:
            lst.emit_stop_by_name('button-press-event')
            self.set_prompt()
            self.prompt_buff.insert_at_cursor(lst.get_text(row,col))
            #self.prompt.insert(self.font, self.fg, self.bg, lst.get_text(row,col))
            self.history_list.select_row(row,col)
            if len(self.history_buffer) >= row:
                self.history_pos = row

    def append_text(self, msg):
        self.interp.my_write( msg, 'command' )

    def set_prompt(self):
        if self.prompt_state == 0:
            # Line dealt with in some way - reset prompt
            self.prompt_buff.set_text('>>> ');
            i = self.prompt_buff.get_iter_at_offset(4)
            m = self.prompt_buff.create_mark('prompt0', i, True)
           ## self.prompt_buff.place_cursor(4);
            #self.prompt.delete_text(0, -1)
            #self.prompt.insert(self.font, self.fg, self.bg, '>>> ')
        elif self.prompt_state == 1:
            # More input required
            self.prompt_buff.set_text('... ');
            #self.prompt.delete_text(0, -1)
            #self.prompt.insert(self.font, self.fg, self.bg, '... ')
        else:
            raise RuntimeError, ' Should not get here - pyshell.py '

    def command(self, line):
        """ Takes single line commands, doesn't echo line to console, but output will """

        self.prompt_state = self.interp.push(line)

    def add_command(self, command):
        self.interp.cmdlist[string.lower(command.Name)] = command

    def add_helpfile(self,helpfilename):
        self.interp.add_helpfile(helpfilename)

    def get_commands(self):
        return list(self.interp.cmdlist.values())

    def hide_dialog_cb(self,*args):
        dialog = args[0]
        dialog.hide()
        return True

    def pref_gui_cb(self):
        if self.path_dlg is None:
            new_dlg = gtk.Window()
            new_dlg.connect('delete-event',self.hide_dialog_cb)            
            new_dlg.set_title('Paths')
            new_dlg.set_size_request(450,100)

            self.path_dlg = new_dlg

            path_keys = ['MODULE PATHS','COMMAND PATHS']
            nrows = len(self.preferences.keys())+1
            wtable = gtk.Table(nrows,2,False)
            wtable.set_row_spacings(5)
            wtable.set_col_spacings(5)
            wtable.set_border_width(5)
            new_dlg.add(wtable)


            for idx in range(len(path_keys)):
                startpath=self.preferences[path_keys[idx]]
                clabel = gtk.Label(path_keys[idx])
                wtable.attach(clabel, 0,1, idx,idx+1) 
                centry = gtk.Entry()
                centry.set_max_length(350)
                # replace the path string with
                # an entry object
                self.preferences[path_keys[idx]] = centry
                centry.set_editable(True)
                centry.set_size_request(280, 25)
                centry.set_text(startpath)
                wtable.attach(centry, 1,2, idx,idx+1)

            apply_button = gtk.Button('Apply')
            wtable.attach(apply_button, 1,2, nrows-1,nrows)
            apply_button.connect('clicked',self._update_prefs_cb,'Paths')

            wtable.show()        
            self.path_dlg.show_all()

        else:
            self.path_dlg.show_all()
            self.path_dlg.window.raise_()

    def _update_prefs_cb(self,*args):
        pref_type = args[1]
        if pref_type == 'Paths':
            modpaths = self.preferences['MODULE PATHS'].get_text()
            commandpaths = self.preferences['COMMAND PATHS'].get_text()

            import sys
            import string
            import gview

            # Clear old settings to ''
            # Note: old paths are not popped in case a
            # path is shared (eg. if the user specifies
            # OPENEVHOME as a path), since each path is
            # only added once.

            for idx in range(1,MAX_MODULE_PATHS+1):
                if gview.get_preference('pyshell_module_path'+str(idx)) is not None:
                    gview.set_preference('pyshell_module_path'+str(idx),'')

            for idx in range(1,MAX_COMMAND_PATHS+1):
                if gview.get_preference('pyshell_command_path'+str(idx)) is not None:
                    gview.set_preference('pyshell_command_path'+str(idx),'')

            # Get new settings: paths are assumed to be separated
            # by commas.
            modtokens=string.split(modpaths,";")
            idx=1

            for modpath in modtokens:
                if len(modpath) == 0:
                    continue

                if (modpath not in sys.path) and (os.path.isdir(modpath)):
                    sys.path.append(modpath)

                if (os.path.isdir(modpath)):
                    if idx < MAX_MODULE_PATHS+1:
                        gview.set_preference('pyshell_module_path'+str(idx),modpath)
                    else:
                        gvutils.warning('Only the first'+str(MAX_MODULE_PATHS+1)+\
                                        'module paths will be loaded from preferences.')
                    idx=idx+1
                else:
                    gvutils.warning(modpath+' does not exist or is not a directory.')

            cmdtokens=string.split(commandpaths,";")
            idx=1
            for cmdpath in cmdtokens:
                if len(cmdpath) == 0:
                    continue

                if (cmdpath not in sys.path) and (os.path.isdir(cmdpath)):
                    sys.path.append(cmdpath)

                if (os.path.isdir(cmdpath)):
                    if idx < MAX_COMMAND_PATHS+1:
                        gview.set_preference('pyshell_command_path'+str(idx),cmdpath)
                    else:
                        gvutils.warning('Only the first'+ str(MAX_COMMAND_PATHS+1)+\
                                         ' command paths will be loaded from preferences.')
                    idx=idx+1

                else:
                    gvutils.warning(cmdpath+' does not exist or is not a directory.')

            self.hide_dialog_cb(self.path_dlg)

    def __get_standard_menu_entries(self):
            # python shell menu entries that are available
            # without the addition of tools (not exposed
            # by default).
            menucmd="self.menuf.add_entries(["+\
                "('File/Preferences', None, self.preferences_cb ),"+\
                "('File/Quit', '<control>D', self.close_cb),"+\
                "('Help/Help',None,self.launch_help_cb)"+\
                "])"
            return menucmd

    def load_pyshell_file_from_xml(self,pyshellfile="DefaultPyshellFile.xml"):
        import gview
        from osgeo import gdal

        pyshellfile=os.path.join(gview.home_dir,'xmlconfig',pyshellfile)
        try:
            raw_xml = open(pyshellfile).read()
        except:
            raise AttributeError,"Unable to load " + pyshellfile
            return

        tree = gdal.ParseXMLString( raw_xml )
        if tree is None:
            raise AttributeError,"Problem occured parsing pyshell file " + pyshellfile
            return

        if tree[1] != 'GViewAppPyshell':
            raise AttributeError,"Root of %s is not GViewAppPyshell node " % iconfile
            return

        # Optional parts of python shell: menu, icon bar, history area, progress bar
        # If not specified in the xml file, they will not be included.  Currently
        # History area and progress bar are either present or not, with the []
        # indicating that they should be included.  Later, preferences may or may not
        # be added to customize them.  Menu and icon customization is similar to
        # the main OpenEV shell.
        menucmds=None
        iconcmds=None
        historycmds=None
        progresscmds=None
        for node in tree[2:]:
            if node[1] == 'Menu':
                menucmds=self.parse_menu_xml_node(node)
            elif node[1] == 'Iconbar':
                iconcmds=self.parse_icon_xml_node(node)
            elif node[1] == 'History':
                historycmds=[]
            elif node[1] == 'Progress':
                progresscmds=[]
            else:
                txt="Invalid node %s in pyshell file %s.\n" % node[1],pyshellfile
                txt=txt+"Valid node types are Menu, Iconbar, History, and Progress."
                raise AttributeError,txt

        # This tuple may be extended at a later date    
        return (menucmds,iconcmds,historycmds,progresscmds)

    def parse_menu_xml_node(self,menunode):
        import gview

        tools_to_include='All'
        tools_accounted_for=[]
        menu_list=[]

        for node in menunode[2:]:
            if node[1] == 'entry':
                node_path  = gvutils.XMLFind( node, 'path')
                if node_path is None:
                    raise AttributeError,"Invalid menu file format - missing path"

                entry_type = gvutils.XMLFindValue( node_path, 'type', '')
                entry_path = gvutils.XMLFindValue( node, 'path','')

                if (string.find(entry_path,"/") == -1):
                    raise AttributeError,"Invalid menu file format - bad path:%s" % entry_path

                if (entry_type != ''):
                    entry_type = "<" + entry_type + ">"
                path_split=string.split(entry_path,"/")
                path_split[-1] = entry_type + path_split[-1]
                entry_path=string.join(path_split,"/")

                entry_accelerator = gvutils.XMLFindValue( node, 'accelerator', 'None')
                if (entry_accelerator != 'None'):
                    (key,mod) = string.split(entry_accelerator,'+')
                    entry_accelerator = "'<" + key + ">" + mod + "'"

                entry_callback = gvutils.XMLFindValue( node, 'callback', 'None')
                entry= "("                                             \
                        + string.join((entry_path,entry_accelerator,   \
                                       entry_callback),",")

                arguments = gvutils.XMLFind( node, 'arguments')
                if arguments is not None:
                    args_list = []
                    args =  gvutils.XMLFind( arguments, 'arg','')
                    if args is not None:
                        for arg in args:
                            args_list.append(gvutils.XMLFindValue( arg, '',''))
                        entry = entry + "," + string.join(args_list,",")

                entry = entry + ")"

                menu_list.append(entry)

            elif node[1] == 'tools':
                tools_to_include=node[2][1]

            elif node[1] == 'simpletoolentry':
                toolname  = gvutils.XMLFindValue( node, 'name')
                if toolname is None:
                    raise AttributeError,"Invalid menu file format - missing tool name"

                if gview.app.tool_index.has_key(toolname) == 0:
                    raise AttributeError,"Invalid menu file format- tool "+toolname+" not loaded."

                ctool=gview.app.Tool_List[gview.app.tool_index[toolname]][1]
                for cpath in ctool.pymenu_entries.entries.keys():
                    # Ignore default position- overridden by position in file
                    # Also note: tool callbacks don't have arguments
                    entry_accelerator=ctool.pymenu_entries.entries[cpath][2]
                    if entry_accelerator is None:
                        entry_accelerator=str(None)
                    else:
                        entry_accelerator="'"+entry_accelerator+"'"

                    entry= "("                                             \
                            + string.join(("'"+cpath+"'",entry_accelerator,   \
                           "gview.app.Tool_List[gview.app.tool_index['"+toolname+\
                           "']][1].pymenu_entries.entries['"+cpath+"'][1]"),",")+")"

                    menu_list.append(entry)

                if toolname not in tools_accounted_for:
                    tools_accounted_for.append(toolname)


            elif node[1] == 'complextoolentry':
                toolname  = gvutils.XMLFindValue( node, 'name')
                if toolname is None:
                    raise AttributeError,"Invalid menu file format - missing tool name"

                oldpath  = gvutils.XMLFindValue( node, 'oldpath')

                if oldpath is None:
                    txt="Invalid menu file format - complex tool entry\nrequires oldpath item."
                    raise AttributeError,txt
                oldpath = oldpath[1:-1] # Entries in XML file are surrounded by quotes- get rid of them

                newpath  = gvutils.XMLFindValue( node, 'newpath')
                if newpath is None:
                    txt="Invalid menu file format - complex tool entry\nrequires newpath item."
                    raise AttributeError,txt
                newpath = newpath[1:-1] # Entries in XML file are surrounded by quotes- get rid of them

                if gview.app.tool_index.has_key(toolname) == 0:
                    raise AttributeError,"Invalid menu file format- tool "+toolname+" not loaded."

                ctool=gview.app.Tool_List[gview.app.tool_index[toolname]][1]
                if ctool.pymenu_entries.entries.has_key(oldpath) == 0:
                    raise AttributeError,'Invalid menu file entry- tool '+toolname+\
                          ' has no\nmenu entry '+oldpath

                entry_accelerator=gvutils.XMLFindValue( node, 'accelerator' )
                if entry_accelerator is None:
                    entry_accelerator=ctool.pymenu_entries.entries[oldpath][2]
                else:
                    (key,mod)=string.split(entry_accelerator,'+')
                    entry_accelerator="<"+key+">"+mod

                if entry_accelerator is None:
                    entry_accelerator=str(None)
                else:
                    entry_accelerator="'"+entry_accelerator+"'"

                entry= "("                                             \
                        + string.join(("'"+newpath+"'",entry_accelerator,   \
                        "gview.app.Tool_List[gview.app.tool_index['"+toolname+\
                        "']][1].pymenu_entries.entries['"+oldpath+"'][1]"),",") + ")"                

                menu_list.append(entry)

                if toolname not in tools_accounted_for:
                    tools_accounted_for.append(toolname)


        if tools_to_include not in ['All','None','Some']:
            raise AttributeError,"Invalid menu file format- <tool> entry should be All, None, or Some."

        if ((tools_to_include == 'None') and (len(tools_accounted_for) > 0)):
            txt = "Invalid menu file format- if <tool> entry is None,\nno "
            txt = txt+"simpletoolentry or complextoolentry items may be specified."
            raise AttributeError,txt

        remaining_cmds=[]
        if tools_to_include == 'All':
            for citem in gview.app.Tool_List:
                if citem[0] not in tools_accounted_for:
                    ctool=citem[1]
                    for centry in ctool.pymenu_entries.entries.keys():
                        cpos=ctool.pymenu_entries.entries[centry][0]
                        cpos=max(cpos,0)
                        accel=ctool.pymenu_entries.entries[centry][2]
                        if accel is None:
                            accel=str(None)
                        else:
                            accel="'"+accel+"'"

                        entry= "self.menuf.insert_entry(" \
                                + string.join((str(cpos),"'"+centry+"'",accel,   \
                               "gview.app.Tool_List[gview.app.tool_index['"+citem[0]+\
                               "']][1].pymenu_entries.entries['"+centry+"'][1]"),",")+")"                
                        remaining_cmds.append(entry)

        # create the menu command to populate the entries
        menu_cmds=[]
        menu_cmd =  "self.menuf.add_entries([" + string.join(menu_list,',') + "])"
        menu_cmds.append(menu_cmd)
        if len(remaining_cmds) > 0:
            menu_cmds.extend(remaining_cmds)

        return menu_cmds        

    def parse_icon_xml_node(self,iconnode):
        import gview

        tools_to_include = 'All'
        tools_accounted_for=[]
        tool_entry_list=[]
        icon_list=[]
        for node in iconnode[2:]:
            if node[1] == 'icon':
                type = None
                icon_label = gvutils.XMLFindValue( node, 'label','None')
                icon_hint = gvutils.XMLFindValue( node, 'hint','None')
                icon_callback = gvutils.XMLFindValue( node, 'callback','None')
                icon_help = gvutils.XMLFindValue( node, 'help','None')
                icon_file = gvutils.XMLFindValue( node, 'xpm','None')
                # xpm files - need to add path and possible help
                if (icon_file != 'None'):
                    type = 'xpm'
                    icon = "self.add_icon_to_bar("                           \
                            + string.join((icon_file,icon_label,icon_hint,   \
                                           icon_callback,icon_help),",")     \
                            + ")" 

                # pixmap files - not adding path or help 
                icon_file = gvutils.XMLFindValue( node, 'pixmap','None')
                if (icon_file!= 'None'):
                    type = 'pixmap'
                    icon = "self.iconbar.append_item("                        \
                            + string.join((icon_label,icon_hint,icon_hint,    \
                                              icon_file,icon_callback),",")   \
                            + ")" 

                # widget  
                icon_file = gvutils.XMLFindValue( node, 'widget','None')
                if (icon_file!= 'None'):
                    type = 'widget'
                    icon_file = gvutils.XMLFindValue( node, 'widget','None')
                    icon = "self.iconbar.append_widget("                       \
                            + string.join((icon_file,icon_hint,icon_hint),",") \
                            + ")" 
                # none of the above
                if type is None:
                    raise AttributeError,"Invalid icon file format - unknown type"

                icon_list.append(icon)
            elif node[1] == 'tools':
                tools_to_include=node[2][1]
            elif node[1] == 'simpletoolentry':
                toolname  = gvutils.XMLFindValue( node, 'name')
                if toolname is None:
                    raise AttributeError,"Invalid pyshell file format - missing tool name"

                if gview.app.tool_index.has_key(toolname) == 0:
                    raise AttributeError,"Invalid pyshell file format- tool "+toolname+" not loaded."

                ctool=gview.app.Tool_List[gview.app.tool_index[toolname]][1]
                idx=0
                for centry in ctool.pyicon_entries.entries:
                    icon_file=centry[0]

                    icon_label=centry[1]
                    if icon_label is not None:
                        icon_label="'"+icon_label+"'"
                    else:
                        icon_label=str(None)

                    icon_hint=centry[2]
                    if icon_hint is not None:
                        icon_hint="'"+icon_hint+"'"
                    else:
                        icon_hint=str(None)

                    # Ignore position- it is overridden by this entry's location in the
                    # xml file
                    icon_callback=centry[4]
                    icon_help=centry[5]
                    if icon_help is not None:
                        icon_help="'"+icon_help+"'"
                    else:
                        icon_help=str(None)

                    icon_type=centry[6]
                    if icon_type == 'xpm':
                        icon = "self.add_icon_to_bar("                           \
                                + string.join(("'"+icon_file+"'",\
                                icon_label,icon_hint,   \
                                "gview.app.Tool_List[gview.app.tool_index['"+\
                                toolname+"']][1].pyicon_entries.entries["+\
                                str(idx)+"][4]",icon_help),",") + ")"
                        icon_list.append(icon)
                    else:
                        raise AttributeError,"Invalid icon type "+icon_type+" in tool "+toolname+"."
                    idx=idx+1

                if toolname not in tools_accounted_for:
                    tools_accounted_for.append(toolname)

            elif node[1] == 'complextoolentry':
                toolname  = gvutils.XMLFindValue( node, 'name')
                if toolname is None:
                    raise AttributeError,"Invalid icon file format - missing tool name."

                oindex  = gvutils.XMLFindValue( node, 'index')

                if oindex is None:
                    txt="Invalid icon file format - complex tool entry\n"+\
                        "requires the index of the icon entry\n"
                    txt=txt+"to replace (0...number of entries-1).\n"
                    raise AttributeError,txt
                try:
                    oindex=int(oindex)
                except:
                    raise AttributeError,"Invalid icon file- icon index to replace must be an integer."

                if gview.app.tool_index.has_key(toolname) == 0:
                    raise AttributeError,"Invalid icon file entry- tool "+toolname+" not loaded."

                ctool=gview.app.Tool_List[gview.app.tool_index[toolname]][1]
                if len(ctool.pyicon_entries.entries) < (oindex+1):
                    txt='Invalid file file entry- for tool '+toolname+'.\n maximum entry index is '
                    txt=txt+str(len(ctool.pyicon_entries.entries)-1)+'.' 

                icon_file=gvutils.XMLFindValue( node, 'xpm')
                icon_hint=gvutils.XMLFindValue( node, 'hint')
                icon_label=gvutils.XMLFindValue( node, 'label')
                icon_help=gvutils.XMLFindValue( node, 'help')

                if icon_file is None:
                    icon_file="'"+ctool.icon_entries.entries[oindex][0]+"'"
                elif os.path.isfile(icon_file):
                    if os.name == 'nt':
                        icon_file="'"+string.replace(icon_file,"\\","\\\\")+"'"
                    else:
                        icon_file="'"+icon_file+"'"
                elif os.path.isfile(os.path.join(gview.home_dir,'tools',icon_file)):
                    icon_file="'"+os.path.join(gview.home_dir,'tools',icon_file)+"'"
                    if os.name == 'nt':
                        icon_file=string.replace(icon_file,"\\","\\\\")
                elif os.path.isfile(os.path.join(gview.home_dir,'pics',icon_file)):
                    icon_file="'"+os.path.join(gview.home_dir,'pics',icon_file)+"'"
                    if os.name == 'nt':
                        icon_file=string.replace(icon_file,"\\","\\\\")                
                else:
                    txt = "Cannot find file "+tempf+'.  Either the full\n'
                    txt = txt+"path must be specified, or "+tempf+ " must be\n"
                    txt = txt+"placed in the tools or pics directory."
                    raise AttributeError,txt


                if icon_label is None:
                    icon_label=ctool.pyicon_entries.entries[oindex][1]

                if icon_label is not None:
                    icon_label="'"+icon_label+"'" 
                else:
                    icon_label=str(None)

                if icon_hint is None:
                    icon_hint=ctool.pyicon_entries.entries[oindex][2]

                if icon_hint is not None:
                    icon_hint="'"+icon_hint+"'"
                else:
                    icon_hint=str(None)

                if icon_help is None:
                    icon_help=ctool.pyicon_entries.entries[oindex][5]

                if icon_help is not None:
                    icon_help="'"+icon_help+"'"
                else:
                    icon_help=str(None)

                icon_callback=ctool.pyicon_entries.entries[oindex][4]
                icon_type=ctool.pyicon_entries.entries[oindex][6]
                if icon_type == 'xpm':
                    icon = "self.add_icon_to_bar("                           \
                            + string.join((icon_file,icon_label,icon_hint,   \
                            "gview.app.Tool_List[gview.app.tool_index['"+\
                            toolname+"']][1].pyicon_entries.entries["+\
                            str(oindex)+"][4]",icon_help),",") + ")"
                    icon_list.append(icon)
                else:
                    raise AttributeError,"Invalid icon type "+icon_type+" in tool "+toolname+"."

                if toolname not in tools_accounted_for:
                    tools_accounted_for.append(toolname)


        if tools_to_include not in ['All','None','Some']:
            raise AttributeError,"Invalid pyshell file format- icon <tool>"+\
                  " entry should be All, None, or Some."

        if ((tools_to_include == 'None') and (len(tools_accounted_for) > 0)):
            txt = "Invalid icon file format- if <tool> entry is None,\nno "
            txt = txt+"simpletoolentry or complextoolentry items may be specified."
            raise AttributeError,txt

        if tools_to_include == 'All':
            for citem in gview.app.Tool_List:
                if citem[0] not in tools_accounted_for:
                    ctool=citem[1]
                    idx=0
                    for centry in ctool.pyicon_entries.entries:
                        if centry[6] != 'xpm':            
                            raise AttributeError,"Error loading tool entry for tool "+\
                                  citem[0]+"- icon type "+centry[6]+" invalid."

                        icon_file="'"+centry[0]+"'"
                        icon_label=centry[1]
                        if icon_label is not None:
                            icon_label="'"+icon_label+"'"
                        else:
                            icon_label=str(None)

                        icon_hint=centry[2]    
                        if icon_hint is not None:
                            icon_hint="'"+icon_hint+"'"
                        else:
                            icon_hint=str(None)

                        icon_help=centry[5]
                        if icon_help is not None:
                            icon_help="'"+icon_help+"'"
                        else:
                            icon_help=str(None)

                        # Default position in icon bar used
                        pos=centry[3]
                        icon = "self.add_icon_to_bar(" +\
                                string.join((icon_file,icon_label,icon_hint,   \
                                "gview.app.Tool_List[gview.app.tool_index['"+citem[0]+\
                                "']][1].pyicon_entries.entries["+\
                                str(idx)+"][4]",icon_help),",") + ")"

                        pos=max(pos,0)
                        if pos > len(icon_list):
                            icon_list.append(icon)
                        else:
                            icon_list.insert(pos,icon)
                        idx=idx+1

        return icon_list

    def add_icon_to_bar(self, filename, text, hint_text, cb, help_topic=None):
        # Next line doesn't overwrite filename if it is already a full
        # path (os.path.join is intelligent).
        import gview
        import gvhtml
        if os.name == 'nt':
            filename=string.replace(filename,"\\","\\\\")

        full_filename = os.path.join(gview.home_dir,'pics',filename)
        pix, mask = create_pixmap_from_xpm(self,None,full_filename)
        item = self.iconbar.append_item(text,hint_text, hint_text,
                                        gtk.Pixmap(pix,mask), cb )
        if help_topic is not None:
            gvhtml.set_help_topic(item, help_topic)

def parse_interpreter_line(line):
        tokens = string.split( line, None, 1 )

        # Null input line is considered valid.
        if len(tokens) == 0:
            return ('', '')

        if len(tokens) == 1:
            return (string.lower(tokens[0]), '')
        else:
            return (string.lower(tokens[0]), tokens[1])


class PyshellHelpDialog(gtk.Window):

    def __init__(self):
        gtk.Window.__init__(self)
        self.set_title('Python Shell Help')
        self.set_size_request(500,500)
        #gvhtml.set_help_topic( self, "pyshell_help.html" );

        self.set_border_width(3)
        vbox = gtk.VPaned()
        self.notebook = gtk.Notebook()
        self.add( vbox )
        vbox.add1(self.notebook)


        # Create text window for showing help for selected
        # command/function
        pixel_scroll = gtk.ScrolledWindow()
        pixel_scroll.set_size_request(496,250)
        vbox.add2(pixel_scroll)

        self.help_buff = gtk.TextBuffer()
        self.help_view = gtk.TextView(self.help_buff)
        self.help_view.set_wrap_mode(gtk.WRAP_CHAR)
        self.help_view.set_editable(False)
        pixel_scroll.add(self.help_view)

        # self.help_text = gtk.Text()
        # self.help_text.set_line_wrap(False)
        # self.help_text.set_word_wrap(False)
        # self.help_text.set_editable(False)
        # pixel_scroll.add(self.help_text)
        # self.help_text.insert_defaults('')

        # Number of columns to put in command and
        # function CList's.
        self.ncols_cmd=2
        self.ncols_func=2
        self.ncols_builtin=2

        self.create_commandhelp()
        self.create_functionhelp()
        self.create_builtinhelp()

        self.show_all()


    def create_commandhelp(self):
        self.cpane=gtk.VBox(spacing=10)
        self.cpane.set_border_width(10)
        self.notebook.append_page(self.cpane,gtk.Label('Commands'))
        self.cpane_scroll=gtk.ScrolledWindow()
        self.cpane_scroll.set_size_request(496,250)
        self.cpane.pack_start(self.cpane_scroll)

        self.cpane_list=gtk.CList(cols=self.ncols_cmd)
        for idx in range(self.ncols_cmd):
            self.cpane_list.set_column_width(idx,200)

        self.cpane_list.set_selection_mode(SELECTION_SINGLE)

        self.cpane_scroll.add(self.cpane_list)

        # Add items to the list based on interpreter
        import gview
        self.loaded_cmd_keys=gview.app.shell.interp.cmdlist.keys()
        self.unloaded_cmd_keys=[]
        for citem in gview.app.shell.interp.help_cmdtxt.keys():
            if citem not in self.loaded_cmd_keys:
                self.unloaded_cmd_keys.append(citem)

        self.total_cmd_keys=[]
        self.total_cmd_keys.extend(self.loaded_cmd_keys)
        self.total_cmd_keys.extend(self.unloaded_cmd_keys)
        self.total_cmd_keys.sort()

        if ((len(self.total_cmd_keys) % self.ncols_cmd) == 0):
            nrows=len(self.total_cmd_keys)/self.ncols_cmd
        else:
            nrows=len(self.total_cmd_keys)/self.ncols_cmd + 1

        for crow in range(nrows):
            if (crow < nrows-1) or ((len(self.total_cmd_keys) % self.ncols_cmd) == 0):
                values=[]
                for idx in range(self.ncols_cmd):
                    values.append(self.total_cmd_keys[crow*self.ncols_cmd+idx])

                self.cpane_list.append(tuple(values))
            else:
                values=[]
                for idx in range(self.ncols_cmd):
                    if ((crow*self.ncols_cmd)+idx < len(self.total_cmd_keys)):
                        values.append(self.total_cmd_keys[crow*self.ncols_cmd+idx])
                    else:
                        values.append('')

                self.cpane_list.append(tuple(values))                

        # Connections
        self.cpane_list.connect('button-press-event',self.cmdlist_clicked_cb)

    def cmdlist_clicked_cb(self, lst, event):

        # Should also clear old text at this point
        try:
            row,col=lst.get_selection_info(int(event.x), int(event.y))
        except:
            return

        if event.button == 1:
            lst.emit_stop_by_name('button-press-event')
            ckey=lst.get_text(row,col)
            import gview
            txt=gview.app.shell.interp.get_command_help(ckey)
            if ckey in self.loaded_cmd_keys:
                if txt is not None:
                    txt='\t\t\t'+ckey+' (command)\n\n'+txt
                else:
                    txt='\t\t\t'+ckey+' (command)\n\nNo help available.'

            else:
                if txt is not None:
                    txt='\t\t\t'+ckey+' (command- not loaded)\n\n'+txt
                else:
                    txt='\t\t\t'+ckey+' (command- not loaded)\n\nNo help available.'

            self.update_text(txt)

    def create_functionhelp(self):
        self.fpane=gtk.VBox(spacing=10)
        self.fpane.set_border_width(10)
        self.notebook.append_page(self.fpane,gtk.Label('Functions'))
        self.fpane_scroll=gtk.ScrolledWindow()
        self.fpane_scroll.set_size_request(396,250)
        self.fpane.pack_start(self.fpane_scroll)

        self.fpane_list=gtk.CList(cols=self.ncols_func)
        for idx in range(self.ncols_func):
            self.fpane_list.set_column_width(idx,200)

        self.fpane_list.set_selection_mode(SELECTION_SINGLE)

        self.fpane_scroll.add(self.fpane_list)

        # Add items to the list based on interpreter
        import gview

        # Need numeric to locate ufunc's 
        import numpy as Numeric

        self.loaded_func_keys=[]
        for ckey in gview.app.shell.interp.locals.keys():
            if (type(gview.app.shell.interp.locals[ckey]) == type(launch)):
                self.loaded_func_keys.append(ckey)
            if (type(gview.app.shell.interp.locals[ckey]) == type(Numeric.logical_and)):
                self.loaded_func_keys.append(ckey)

        self.unloaded_func_keys=[]
        for ckey in gview.app.shell.interp.help_functxt.keys():
            if ckey not in self.loaded_func_keys:
                self.unloaded_func_keys.append(ckey)

        self.total_func_keys=[]
        self.total_func_keys.extend(self.loaded_func_keys)
        self.total_func_keys.extend(self.unloaded_func_keys)
        self.total_func_keys.sort()

        if ((len(self.total_func_keys) % self.ncols_func) == 0):
            nrows=len(self.total_func_keys)/self.ncols_func
        else:
            nrows=len(self.total_func_keys)/self.ncols_func + 1

        for crow in range(nrows):
            if (crow < nrows-1) or ((len(self.total_func_keys) % self.ncols_func) == 0):
                values=[]
                for idx in range(self.ncols_func):
                    values.append(self.total_func_keys[crow*self.ncols_func+idx])

                self.fpane_list.append(tuple(values))
            else:
                values=[]
                for idx in range(self.ncols_func):
                    if ((crow*self.ncols_func)+idx < len(self.total_func_keys)):
                        values.append(self.total_func_keys[crow*self.ncols_func+idx])
                    else:
                        values.append('')

                self.fpane_list.append(tuple(values))                

        # Connections
        self.fpane_list.connect('button-press-event',self.funclist_clicked_cb)

    def funclist_clicked_cb(self, lst, event):

        # Should also clear old text at this point
        try:
            row,col=lst.get_selection_info(int(event.x), int(event.y))
        except:
            return

        if event.button == 1:
            lst.emit_stop_by_name('button-press-event')
            ckey=lst.get_text(row,col)
            import gview
            txt=gview.app.shell.interp.get_function_help(ckey)
            if ckey in self.loaded_func_keys:
                if txt is not None:
                    txt='\t\t\t'+ckey+' (function)\n\n'+txt
                else:
                    txt='\t\t\t'+ckey+' (function)\n\nNo help available.\n'                    
            else: 
                if txt is not None:            
                    txt='\t\t\t'+ckey+' (function- not loaded)\n\n'+txt
                else:
                    txt='\t\t\t'+ckey+' (function- not loaded)\n\nNo help available.\n'

            self.update_text(txt)

    def create_builtinhelp(self):
        self.bpane=gtk.VBox(spacing=10)
        self.bpane.set_border_width(10)
        self.notebook.append_page(self.bpane,gtk.Label('Built-in Functions'))
        self.bpane_scroll=gtk.ScrolledWindow()
        self.bpane_scroll.set_size_request(396,250)
        self.bpane.pack_start(self.bpane_scroll)

        self.bpane_list=gtk.CList(cols=self.ncols_builtin)
        for idx in range(self.ncols_builtin):
            self.bpane_list.set_column_width(idx,200)

        self.bpane_list.set_selection_mode(SELECTION_SINGLE)

        self.bpane_scroll.add(self.bpane_list)

        # Add items to the list based on interpreter
        import gview

        self.loaded_builtin_keys=[]
        for ckey in gview.app.shell.interp.locals.keys():
            if (type(gview.app.shell.interp.locals[ckey]) == type(hasattr)):
                self.loaded_builtin_keys.append(ckey)
        for ckey in gview.app.shell.interp.locals['__builtins__'].keys():
            if (type(gview.app.shell.interp.locals['__builtins__'][ckey]) == type(hasattr)):
                self.loaded_builtin_keys.append(ckey)


        self.unloaded_builtin_keys=[]
        for ckey in gview.app.shell.interp.help_builtintxt.keys():
            if ckey not in self.loaded_builtin_keys:
                self.unloaded_builtin_keys.append(ckey)

        self.total_builtin_keys=[]
        self.total_builtin_keys.extend(self.loaded_builtin_keys)
        self.total_builtin_keys.extend(self.unloaded_builtin_keys)
        self.total_builtin_keys.sort()

        if ((len(self.total_builtin_keys) % self.ncols_builtin) == 0):
            nrows=len(self.total_builtin_keys)/self.ncols_builtin
        else:
            nrows=len(self.total_builtin_keys)/self.ncols_builtin + 1

        for crow in range(nrows):
            if (crow < nrows-1) or ((len(self.total_builtin_keys) % self.ncols_builtin) == 0):
                values=[]
                for idx in range(self.ncols_builtin):
                    values.append(self.total_builtin_keys[crow*self.ncols_builtin+idx])

                self.bpane_list.append(tuple(values))
            else:
                values=[]
                for idx in range(self.ncols_builtin):
                    if ((crow*self.ncols_builtin)+idx < len(self.total_builtin_keys)):
                        values.append(self.total_builtin_keys[crow*self.ncols_builtin+idx])
                    else:
                        values.append('')

                self.bpane_list.append(tuple(values))                

        # Connections
        self.bpane_list.connect('button-press-event',self.builtin_clicked_cb)

    def builtin_clicked_cb(self, lst, event):

        # Should also clear old text at this point
        try:
            row,col=lst.get_selection_info(int(event.x), int(event.y))
        except:
            return

        if event.button == 1:
            lst.emit_stop_by_name('button-press-event')
            ckey=lst.get_text(row,col)
            import gview
            txt=gview.app.shell.interp.get_builtin_help(ckey)
            if ckey in self.loaded_builtin_keys:
                if txt is not None:
                    txt='\t\t\t'+ckey+' (built-in function)\n\n'+txt
                else:
                    txt='\t\t\t'+ckey+' (built-in function)\n\nNo help available.\n'                    
            else: 
                if txt is not None:            
                    txt='\t\t\t'+ckey+' (built-in function- not loaded)\n\n'+txt
                else:
                    txt='\t\t\t'+ckey+' (built-in function- not loaded)\n\nNo help available.\n'

            self.update_text(txt)

    def update_text(self,txt):
        self.help_buff.set_text(txt + '\0')
        # self.help_text.freeze()
        # del_len=self.help_text.get_length()
        # self.help_text.backward_delete(del_len)
        # self.help_text.insert_defaults(txt+'\0')
        # self.help_text.thaw()

def _format_doc(doc_string):
    """Reformat docstring for help display."""

    if doc_string is None:
        return 'None'

    dsplit=string.split(doc_string,'\n')
    if len(dsplit) == 1:
        return doc_string

    # if __doc__ has more than one line,
    # all except first will be indented.
    # Get rid of this indent.

    indent=0
    for cline in dsplit[1:]:
        temp=string.lstrip(cline)
        if len(temp) > 0:
            indent=len(cline)-len(temp)
            break

    new_doc=dsplit[0]
    for cline in dsplit[1:]:
        new_doc=string.join([new_doc,cline[indent:]],'\n')

    return new_doc


def _parse_cmdhelp_text(txt):
    """ Parse command help text into module, group, html, text.

        Returns (Module, Group, Html, help_text) if command
        help is successfully parsed; None if parsing
        fails.  Module and Group are required.  Html is
        an optional reference to an Html file. help_text
        is the actual help text.

        Command help entries should be of the form:

        COMMAND_NAME=my_command
        Module: my_module
        Group: my_group
        Html: my_html.html

        documentation...

        The entries must be in that order if all are present,
        but Html may be omitted.  If Html or documentation
        are not present, None will be returned in their place.
    """

    mname=None
    stxt=string.split(txt,'\n')

    if len(stxt) < 2:
        return None

    # first line should be module
    mline=stxt[0]
    if ((len(mline) > 7) and (mline[:7] == 'Module:')):
        mname=string.strip(mline[7:])
    else:
        return None

    # second line should be group
    gline=stxt[1]
    if ((len(gline) > 6) and (gline[:6] == 'Group:')):
        gname=string.strip(gline[6:])
    else:
        return None

    if len(stxt) == 2:
        return (mname,gname,None,None)

    # third line MAY be html
    hline=stxt[2]
    if ((len(hline) > 5) and (hline[:5] == 'Html:')):
        hname=string.strip(hline[5:])
        if len(stxt) > 3:
            htext=string.join(stxt[3:],'\n')
        else:
            htext=None

        return (mname,gname,hname,htext)
    else:
        htext=string.join(stxt[2:],'\n')
        return (mname,gname,None,htext)


def _parse_funchelp_text(txt):
    """ Parse function help text into module, html, text.

        Returns (Module, Html, help_text) if function
        help is successfully parsed; None if parsing
        fails.  Module is required.  Html is
        an optional reference to an Html file. help_text
        is the actual help text.

        Function help entries should be of the form:

        FUNCTION_NAME=my_function
        Module: my_module
        Html: my_html.html

        documentation...

        or

        BUILTIN_NAME=my_builtin_function
        Module: my_module
        Html: my_html.html

        documentation...

        The entries must be in that order if all are present,
        but Html may be omitted.  If Html or documentation
        are not present, None will be returned in their place.
    """

    mname=None
    stxt=string.split(txt,'\n')

    if len(stxt) < 1:
        return None

    # first line should be module
    mline=stxt[0]
    if ((len(mline) > 7) and (mline[:7] == 'Module:')):
        mname=string.strip(mline[7:])
    else:
        return None

    if len(stxt) == 1:
        return (mname,None,None)

    # second line MAY be html
    hline=stxt[1]
    if ((len(hline) > 5) and (hline[:5] == 'Html:')):
        hname=string.strip(hline[5:])
        if len(stxt) > 2:
            htext=string.join(stxt[2:],'\n')
        else:
            htext=None

        return (mname,hname,htext)
    else:
        htext=string.join(stxt[1:],'\n')
        return (mname,None,htext)



def _run_command_line(line):
    # This part is very hackish, though consistent with
    # how openev transfers information between modules
    # in other cases (eg. main app).  In order to allow the
    # function to change the value of a variable in the
    # interpreter, use the fact that the shell is
    # stored globally under the name "gview.app.shell",
    # and the interpreter is stored as "interp" beneath it.
    # and update it's "locals" dictionary to fill in the
    # needed values.

    import gview     
    import gvcommand
    cc=gvcommand.CommandContext(line,gview.app.shell.interp.cmdlist,gview.app.shell.interp)
    return_val = cc.execute()
    # return_val will be 0 or 1 for simple commands; 0 
    # for failure, 1 for success.  If the arguments were
    # modify-in-place, the return value will be a tuple
    # consisting of (0 or 1, {} [,options]).  Here, 0 or 1
    # indicates success or failure as before, {} is a
    # dictionary of variables to set in the command shell
    # (keyword is the variable name, value is the value)-
    # this dictionary can be empty.  the options part is
    # not implemented yet, but might be later on if
    # more flexibility is needed.  It should always be
    # optional though.

    if (type(return_val) == type(())) and (len(return_val) > 1):

        for ckey in return_val[1].keys():
            if ckey is not 'None':
                gview.app.shell.interp.locals[ckey]=return_val[1][ckey]

class ScrollableTextView(gtk.ScrolledWindow):
     """
     A scrollable text view.
     """
     def __init__(self, text_buff):
         gtk.ScrolledWindow.__init__(self)
         self._textview = gtk.TextView(text_buff)
         self._textview.show()
         self.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
         self.add(self._textview)   

if __name__ == '__main__':
    app = Shell()
    app.show_all()

    # Removed - GTK2 Port - Need alternative? gtk.Extra.debug_main_quit()
    app.connect('destroy', gtk.main_quit)
    gtk.main()


