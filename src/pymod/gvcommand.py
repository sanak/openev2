###############################################################################
# $Id$
#
# Project:  OpenEV
# Purpose:  Base classes for Command Parsing.
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

import string

###############################################################################
class CommandBase:
    """Base class for command objects.

    Commands should have the following data members:

    Name -- command name... the first keyword on any line that is this command.
    Usage -- optional short usage message (eg. 'DEFINE {column_name} [FLOAT/STRING/INTEGER]')
    HelpURL -- web url to help for this command.  Suitable for use with gvhtml.
    Args -- List of CommandArgDef objets defining the arguments to this command.
    Group -- Textual token indicating the command group this command belongs
    to.

    A command must also implement the execute() method.  The execute() method
    can depend on the arg_tokens argument being a list of argument values,
    with one entry for each entry in self.Args.  Switch arguments will have
    a logical TRUE/FALSE (nonzero/zero) value.  Other arguments will be a
    string (the token value) or None if the argument did not occur.  Execute()
    isn't called unless all automated validation passed.

    """
    def __init__(self):
        pass

    def execute( arg_tokens, line, interpreter ):
        pass


###############################################################################
class ArgDef:

    """
Argument Attributes
-------------------

type:
 - The type of the argument. Legal values are "string_word", "string_token",
 "switch", "numeric" and "string_chunk".  Details are later in docs.

name:
 - The name of the argument.  Used as the key for argument values in the
   dictionary of arguments passed into the Evaluate method. 

valid_list:
 - A list of valid values for this argument.  The list will be treated as
   literal text to be case insensitively compared against the value when
   checking if the argument is present/valid.

required:
 - 1 or 0 depending on whether the argument is required or not.  Arguments
   that are not required, and are not present will be passed with a None
   value into the Execute method of the command.

prompt:
 - Text to be shown to the user when they are prompted for the value of
   this argument.  If not supplied the name of the argument will be shown
   to the user as the prompt. 

prompt_func:
 - A function to be called when prompt text is needed.  The returned string
   will be the prompt.  Useful when the prompt will vary depending on the
   current environment. 

parse_func:
 - A function that should be called to parse text output of the input command
   that is intended to be part of the argument.  Normally None indicating
   that default parsing logic should be used. 

validate_method: 
 - A function that should be called on the argument value to determine if it
   is valid or not.  The function should return None if valid, or a string
   indicating the problem if it is not.  It also receives the current
   interpreter as a context, but shouldn't use it for input/output.

read_token_method:
 - A function that should be called on the input text to get the next token.
   If not provided a built-in will be used based on the 'type'.  The read
   token method will receive as input an input text string, and a      
   CommandContext object.  It should return a (token,remaining_text) tuple.

process_token_method:
- A function that should be called on the token to convert it to the
  appropriate argument type (eg. evaluating a variable, converting
  a string to a number).  This gets called in parsing.

default:
 - The value that should be assigned to this argument if it isn't found on
   the commandline.  If not provided, None is assumed.  Not applicable to
   switch arguments.


Argument Types
--------------

string_word: 
  - Normally a white space terminated chunk of text. 
  - Any mix of non-white space characters. 
  - If quoted it may contain white space, and the quotes will be dropped
    from the processed value.

string_token:
  - a well formed token.  Alpha-numeric, starting with a letter.  May
    include underscores but no other special characters.  Used for variable
    names, and so forth. 

switch:
  - May appear at any point in the command (perhaps limit to all switches 
    preceeding non-switch arguments, but allow reordering of switches?)
  - Must start with a '-' or '/'.
  - Must have a valid_list with the set of possible values. eg ['-n','/n'].
  - Currently can't take any arguments. 

multi_switch:
  - Like switch, except that instead of returning 1 or zero, the argument
    returned is the name used for the switch or None (useful for switches
    which can take on one of a few values, but different behaviours are
    needed for the different options- eg. /off or /on)

list_type:
  - Argument is one of a list of possible string values
  - Must supply a list of valid options (valid_opts)

numeric:
  - value is numeric.
  - min/max range values can be tested in validate. 

string_chunk:
  - Similar to string_word, but an arbitrary terminator string can be
    supplied.  The terminator string may be None indicating all the remainder
    of the command should be consumed.

variable:
  - A user variable, such as a numpy array.
  - internally, both the variable's value and its name will be passed to
    the command (as a list) so that in place modifications can be made
    if need be.

    """


    def __init__(self, name = None, type = 'string_word', prompt = None,
                 prompt_func = None, required = 1, valid_list = None,
                 validate_method = None, default = None,
                 read_token_method = None, process_token_method = None, valid_opts=None ):
        self.name = name
        self.type = type
        self.prompt = prompt
        self.required = required
        self.valid_list = valid_list
        self.valid_opts = valid_opts
        self.validate_method = validate_method
        self.default = default

        # Generate valid sequence for switches if not explicitly provided.
        if self.type == 'switch' and valid_list is None:
            self.valid_list = [ '-' + name, '/' + name ]

        if self.type == 'list_type' and valid_opts is None:
            raise AttributeError,'list type requires a list of valid string arguments (valid_opts)'

        # Ensure "valid_list" entries are all forced into lower case for
        # comparison purposes.

        self.valid_list_lower = None
        if self.valid_list is not None:
            self.valid_list_lower = []
            for item in self.valid_list:
                self.valid_list_lower.append( string.lower(item) )

        # Validate some requirements
        if self.type not in [ 'string_word', 'string_token', 'switch','list_type',
                              'multi_switch','numeric', 'string_chunk', 'variable']:
            raise ValueError, 'Unsupported argument type: ' + self.type

        # Provide token reader if not specified in args.
        if read_token_method is not None:
            self.read_token_method = read_token_method

        elif type == 'string_word':
            self.read_token_method = self.read_quotable_token

        elif type == 'variable':
            self.read_token_method = self.read_quotable_token

        elif type == 'string_chunk':
            self.read_token_method = self.read_remainder_token

        else:
            self.read_token_method = self.read_simple_token

        # Processing method: in variable case, this evaluates the
        # variable's value in the pyshell.  In the simple case
        # it does nothing (just returns the input)
        if process_token_method is not None:
            self.process_token_method = process_token_method

        elif type == 'variable':
            self.process_token_method = self.process_variable_token

        elif type == 'numeric':
            self.process_token_method = self.process_numeric_token

        else:
            self.process_token_method = self.process_simple_token


    ###########################################################################
    def validate(self, arg_text, command_context):
        if self.validate_method is not None:
            return self.validate_method( arg_text, command_context )

        if self.valid_list_lower is not None:
            lower_arg_text = string.lower(arg_text)
            if lower_arg_text not in self.valid_list_lower:
                return '%s: %s is not in the set of valid options.' % \
                       ( self.name, arg_text )
            else:
                return None

        if self.type == 'numeric':        
            try:
                string.atof( arg_text )
            except ValueError:
                return '%s: %s is not numeric.' % \
                       ( self.name, arg_text )

        elif self.type == 'string_token':
            # Must start with a letter
            first_char='abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ'
            later_char = first_char + '0123456789'
            if len(arg_text) > 0:
                if arg_text[0] not in first_char:
                    return '%s: %s is not a valid variable name.' % \
                           (self.name, arg_text)
                for cchar in arg_text[1:]:
                    if cchar not in later_char:
                        return '%s: %s is not a valid variable name.' % \
                               (self.name, arg_text)


        return None

    ###########################################################################
    def read_simple_token(self, line, cc ):
        tokens = string.split(line,None,1)
        if len(tokens) == 1:
            return (tokens[0], '')
        elif len(tokens) == 2:
            return (tokens[0], tokens[1])
        else:
            return (None,'')

    ###########################################################################
    def read_quotable_token(self, line, cc ):
        # Check for non-keyword quoted string
        if len(line) < 1:
            return (None,'')

        if (line[0] == '"'):
            tokens1 = string.split(line,'"',1)
            tokens2 = string.split(tokens1[1],'"',1)
            if len(tokens2) < 2:
                interpreter.showText( 'Unmatched "  in argument line ','error' )
                return (None,'')
            return (tokens2[0],tokens2[1])
        elif (line[0] == "'"):
            tokens1 = string.split(line,"'",1)
            tokens2 = string.split(tokens1[1],"'",1)
            if len(tokens2) < 2:
                interpreter.showText( "Unmatched '  in argument line ",'error' )
                return (None,'')
            return (tokens2[0],tokens2[1])

        # Check for keyword
        tokens1=string.split(line,None,1)

        if string.find(tokens1[0],'=') == -1:
            # No keyword found- simple token
            return self.read_simple_token( line, cc )
        else:
            # keyword found- check for quoted token
            tokens2=string.split(tokens1[0],'=')
            if tokens2[1][0] == '"':
                # Keyword found.  return quoted token with keyword
                # to identify the argument in later parsing, but separated
                # from remainder of line
                tokens1 = string.split(line,'"',1)
                tokens2 = string.split(tokens1[1],'"',1)
                if len(tokens2) < 2:
                    interpreter.showText( 'Unmatched "  in argument line ','error' )
                    return (None,'')
                token = tokens1[0]+tokens2[0]
                return (token,tokens2[1])
            elif tokens2[1][0] == "'":
                # Keyword found.  return quoted token with keyword
                # to identify the argument in later parsing, but separated
                # from remainder of line
                tokens1 = string.split(line,"'",1)
                tokens2 = string.split(tokens1[1],"'",1)            
                if len(tokens2) < 2:
                    interpreter.showText( "Unmatched '  in argument line ",'error' )
                    return (None,'')
                token = tokens1[0]+tokens2[0]
                return (token,tokens2[1])
            else:
                return self.read_simple_token( line, cc )    

###############################################################################
    def read_remainder_token(self, line, cc ):
        return (line, '')

###############################################################################
    def process_variable_token(self, token, cc ):
        # Convention for variables: argument consists of a
        # list: (argument value, argument name).  argument value
        # is None if the token is not already present in the shell
        # environment
        # CURRENTLY ALL VARIABLE ARGUMENTS VALUES ARE PASSED AS STRINGS
        # IF THEY ARE NOT SHELL VARIABLES (ie. '0.5' will be passed
        # as '0.5', not atof('0.5')).  
        if cc.interpreter.locals.has_key(token):
            return (cc.interpreter.locals[token],token)
        else:
            return(None,token)

############################################################################### 
    def process_numeric_token(self, token, cc ):
        return string.atof( token )

###############################################################################    
    def process_simple_token(self, token, cc):
        return token

###############################################################################
# The following is intended to document the methods a command interpreter
# must implement.  It isn't critical that custom command interpreters
# derive from this class as long as they provide reasonable behaviour.
#
class CommandInterpreter:

    def isInteractive():
        """Is command interactive?

        Returns non-zero if the command should be treated as interactive,
        meaning that arguments can be prompted for, and usage help can be
        provided.
        """
        pass

    def showText( text, text_class ):
        """
        Show text to the user.

        The text may be one or more lines.  The class argument indicates
        the purpose of the text, and may be used to differentiate fonts,
        or even to direct output in different ways.  The available values
        for class are "result", "error", or "usage".

        text -- the single, or multi line text to display.

        text_class - the indicator of the purpose of the text.

        No value is returned.
        """
        pass

    def showPrompt( *args ):
        """To be determined."""
        pass

###############################################################################
# CommandContext
#
# Objects of this class capture the state of a command as it is parsed.  It
# allows customized parsing actions to be done taking into account the
# arguments that come before, and it is responsible for command parsing.

class CommandContext:

    ###########################################################################
    def __init__(self, line, command_dict = None, interpreter = None ):
        self.interpreter = interpreter
        self.command_dict = command_dict
        self.line = line

        self.text_remaining = line

        # The following are set after a successful parse.
        self.args = None
        self.cmd_obj = None
        self.parsed = 0
        self.error = 0
        self.args_parsed = 0


    ###########################################################################
    def parse_command_text(self):
        tokens = string.split( self.line, None, 1 )

        # Null input line is considered valid.
        if len(tokens) == 0:
            return ('', '')

        if len(tokens) == 1:
            return (string.lower(tokens[0]), '')
        else:
            return (string.lower(tokens[0]), tokens[1])

    ###########################################################################
    def parse_args(self):

        self.parsed = 0
        self.args_parsed = 0
        self.args = []

        # Next argument to try to parse (added to allow
        # out-of-order keywords)
        self.current_arg = 0
        self.arg_is_parsed = []

        for i in range(len(self.cmd_obj.Args)):
            self.args.append( None )
            self.arg_is_parsed.append(0)

        while self.args_parsed < len(self.cmd_obj.Args):
            if not self.parse_one_arg():
                return 0                

        self.parsed = 1
        return 1

    ###########################################################################
    def parse_one_arg(self):

        #######################################################################
        # Argument parsing logic:
        # Maintain an argument "current_arg" to indicate which argument is
        # currently being searched for.  "current_arg" starts at 0 and goes
        # up to len(self.cmd_obj.Args)-1 (number of args-1).  Each time
        # parse_one_arg is called, check if current_arg has been parsed yet.
        # If it has, increment current_arg and continue.  If not, read the
        # next token as a quotable token and determine if the next argument
        # is a switch or multi_switch (which may occur anywhere in the line)
        # or an argument specified by keyword (which may also appear out-of-
        # order).  If the argument is found to be a switch/multi_switch or
        # a later argument specified by keyword, reset arg to reflect the
        # appropriate command argument.  Set next_arg_index to current_arg,
        # since current_arg still hasn't been found and we are parsing a
        # later argument.  Next, reread the token using the proper argument
        # method (discard the quotable token results) and finish parsing the
        # argument.

        if self.arg_is_parsed[self.current_arg] == 1:
            # argument has already been found and parsed by keyword
            self.current_arg = self.current_arg + 1
            return 1

        # Index of next argument to search for
        next_arg_index = self.current_arg + 1

        arg = self.cmd_obj.Args[self.current_arg]
        interpreter = self.interpreter

        #######################################################################
        # Before parsing using the current argument's method, check that
        # the argument isn't a switch/multi-switch (which can occur anywhere)
        # or designated by a keyword.
        # 
        test_token, test_remainder = ArgDef.read_quotable_token(arg,self.text_remaining,self)

        #######################################################################
        # Has the token been assigned a keyword?

        arg_keyword = None
        if test_token is not None:
            checknamed = string.split(test_token,'=',1)
            if len(checknamed) == 2:
                if arg.name != string.strip(checknamed[0]):
                    checknamed[0] = string.strip(checknamed[0])
                    # keyword doesn't match current argument name:
                    arg = None

                    # Next argument to search for should be the
                    # original again, but fill in the one that
                    # does match so its token isn't lost
                    next_arg_index = self.current_arg 
                    for idx in range(len(self.cmd_obj.Args)):
                        new_arg=self.cmd_obj.Args[idx]
                        if new_arg.name == checknamed[0]:
                            arg = new_arg
                            self.current_arg = idx

                if arg is None:
                    # If keyword not found, = may be part of current
                    # argument's value.  Reset to original argument
                    # and try to parse
                    arg = self.cmd_obj.Args[self.current_arg]
                    next_arg_index = self.current_arg + 1
                else:
                    arg_keyword=string.strip(checknamed[0])



            elif (len(checknamed) == 1) and (len(test_token) > 1) and (test_token[0] in ['/','-']):
                #######################################################################
                # Check to make sure this isn't a switch or multi_switch argument
                # rather than the token for the current argument.  Check the command
                # line argument list in order, stopping at the first switch or
                # multi_switch variable that is compatible with the switch value
                # and has not been parsed yet.

                for idx in range(len(self.cmd_obj.Args)):
                    new_arg=self.cmd_obj.Args[idx]
                    if ((new_arg.type in ['switch','multi_switch']) and
                        (self.arg_is_parsed[idx] == 0)):
                        if test_token in new_arg.valid_list:
                            next_arg_index = self.current_arg
                            arg = new_arg
                            self.current_arg = idx


        #######################################################################
        # At this point, the correct argument should have been selected.                
        # Parse out a candidate token using the appropriate method for this 
        # argument.

        if arg_keyword is None:
            token, next_remainder = arg.read_token_method( self.text_remaining,
                                                       self )
        else:
            # Read token, leaving off keyword.
            token, next_remainder = arg.read_token_method( self.text_remaining[len(arg_keyword)+1:],
                                                           self )

        #######################################################################
        # If this is not really a switch, reset the token
        if ((arg.type == 'switch') and (token is not None) and
            (token[0] not in ['/','-'])):
            token = None

        #######################################################################
        # If we are out of tokens, ensure that the arg is optional.
        if token is None:
            if arg.required:
                interpreter.showText( 'missing argument ' + arg.name,
                                      'error' )
                return 0
            elif arg.type == 'switch':
                self.args[self.current_arg] = 0
                self.arg_is_parsed[self.current_arg] = 1
                self.current_arg = next_arg_index
                self.args_parsed = self.args_parsed + 1
                return 1
            elif  arg.type == 'multi_switch':
                self.args[self.current_arg] = arg.default
                self.arg_is_parsed[self.current_arg] = 1
                self.current_arg = next_arg_index
                self.args_parsed = self.args_parsed + 1
                return 1
            else:
                self.args[self.current_arg] = arg.default
                self.arg_is_parsed[self.current_arg]=1
                self.current_arg = next_arg_index               
                self.args_parsed = self.args_parsed + 1
                return 1


        #######################################################################
        # Is this a legitimate token for this argment or was it skipped.


        val_msg = arg.validate( token, self )

        # Did we actually consume the token?
        if val_msg is None:
            self.text_remaining = next_remainder

        else:
            interpreter.showText( val_msg, 'error' )
            return 0

        # If argument type was variable, do required processing
        token = arg.process_token_method(token, self)

        # Report validation errors for required arguments.
        if arg.type == 'switch':
            self.args[self.current_arg] = val_msg is None
            self.arg_is_parsed[self.current_arg] = 1
            self.current_arg = next_arg_index
            self.args_parsed = self.args_parsed + 1
        elif arg.type == 'multi_switch':
            self.args[self.current_arg] = token
            self.arg_is_parsed[self.current_arg] = 1
            self.current_arg = next_arg_index
            self.args_parsed = self.args_parsed + 1

        elif arg.type == 'list_type':
            if token not in self.valid_opts:
                txt=token+' is not a valid option. Valid options are:\n'
                for copt in self.valid_opts:
                    txt=txt + str(copt) + ' '
                interpreter.showText(txt,'error')
                return 0
            else:
                self.args[self.current_arg] = token
                self.arg_is_parsed[self.current_arg] = 1            
                self.current_arg = next_arg_index
                self.args_parsed = self.args_parsed + 1

        else:
            self.args[self.current_arg] = token
            self.arg_is_parsed[self.current_arg] = 1            
            self.current_arg = next_arg_index
            self.args_parsed = self.args_parsed + 1

        return 1

    ###########################################################################
    # Parse complete command.  Returns 0 on failure, or non-zero on success.
    def parse(self):
        if self.interpreter is None:
            raise ValueError, 'No interpreter set in CommandContext.parse()'

        if self.command_dict is None:
            raise ValueError, 'No command dictionary set in CommandContext.parse()'

        #######################################################################
        # Parse out the command and check for it in the command dictionary.

        cmd, self.text_remaining = self.parse_command_text()

        if not self.command_dict.has_key(cmd):
            self.interpreter.showText( "Command '%s' not recognised." % cmd,
                                       'error' )
            return 0

        self.cmd_obj = self.command_dict[cmd]

        #######################################################################
        # Parse the arguments out.

        self.error = 0
        result = self.parse_args()

        if self.error or result == 0:
            return 0

        return 1

    ###########################################################################
    def execute(self):
        if not self.parsed and not self.parse():
            return 0

        return self.cmd_obj.execute( self.args, self, self.interpreter )
