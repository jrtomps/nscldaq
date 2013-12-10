#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file sshPrompt.tcl
# @brief Prompter for parameters for the SSHPipe provder.
# @author Ron Fox <fox@nscl.msu.edu>

package provide SSHPipe_Prompter 1.0

##
#  The prompter is going to get defaults from a set of configuration parameters
#  which have some defaults as well as environment variables as follows:
#
#   | variable          | Env variable   |  Default  |  Meaning               |
#   ------------------- | -------------- | --------- | ---------------------- |
#   | SSHPipeHost       | DAQHOST        | localhost | Host readout runs on   |
#   | SSHProgram        | RDOFILE        | ""        | Readout executable     |
#
#
#  These definitions are consistent with the 10.x definitions.
#
package require SSHPipe_Provider
package require Configuration
package require Tk
package require DataSourceUI
package require snit

# Make sure the provider namespace has been established:

namespace eval ::SSHPipe {}

##
# ::SSHPipe::setParamDefaults
#
#   Sets the default parameter value.  See the table above.
#
proc ::SSHPipe::setParamDefaults {} {
    Configuration::Set SSHPipeHost "localhost"
    Configuration::Set SSHPipeProgram  ""
}
##
#  ::SSHPipe::paramEnvironmentOverrides
#
#  Apply any environment variables that override the defaults.
#
proc ::SSHPipe::paramEnvironmentOverrides {} {
    Configuration::readEnvironment SSHPipeHost DAQHOST
    Configuration::readEnvironment SSHPipeProgram  RDOFILE
}

#
#  Initialize the configuration parameters when the package is loaded:

::SSHPipe::setParamDefaults
::SSHPipe::paramEnvironmentOverrides

#----------------------------------------------------------------------------
#
#  Graphical prompting


##
# @class ::SSHPipe::ParameterPromptDialog
#
# Dialog that prompts for the parameters needed for an SSHPipe data source.
# The parameters we provide are the host, the program path and optional
# parameters passed to the program.
#
# LAYOUT:
#    +-----------------------------------------+
#    | Host:   [ entry widget ]                |
#    | Readout:[ entry widget ]  <Browse...>   |
#    | Parameters: [ entry widget]             |
#    +-----------------------------------------+
#    | <Ok>   <Cancel>                         |
#    +-----------------------------------------+
#
# OPTIONS:
#   -host    - Name of the host the program runs on.
#   -program - Path to the readout program that is run on the ssh pipe.
#   -parameters - Optional parameters
#
# METHODS
#   modal    - Block until either OK or Cancel has been clicked (or the dialog
#              destroyed).  [delegated to a DialogWrapper object].
#
snit::widgetadaptor ::SSHPipe::ParameterPromptDialog {
    component wrapper
    
    option -host       
    option -program    
    option -parameters 
    
    delegate method modal to wrapper
    
    ##
    # constructor
    #   *  Install a top level as the hull.
    #   *  set default values for -host, -program  -parameters.
    #   *  Install a DialogWrapper as wrapper
    #   *  Ask the wrapper for the frame in which we build the prompt elements
    #   *  Build and layout the prompt areas
    #   *  Process configuration options.
    #
    # @param args - Configuration options.
    #
    constructor args {
        installhull using toplevel
        set options(-host)    [Configuration::get SSHPipeHost]
        set options(-program) [Configuration::get SSHPipeProgram]
        set options(-parameters) ""
        
        install wrapper using DialogWrapper $win.wrapper
        set f [$wrapper controlarea]
        
        ttk::label $f.hostlabel  -text {Host name:}
        ttk::entry $f.host       -textvariable [myvar options(-host)]
        
        ttk::label  $f.programlabel -text {Readout program:}
        ttk::entry  $f.program      -textvariable [myvar options(-program)]
        ttk::button $f.findprogram  -text Browse... -command [mymethod _browseProgram]
        
        ttk::label  $f.paramlabel   -text {Command line options}
        ttk::entry  $f.params       -textvariable [myvar options(-parameters)]
        
        grid $f.hostlabel $f.host
        grid $f.programlabel $f.program $f.findprogram
        grid $f.paramlabel $f.params
        
        pack $wrapper
        
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    # Private methods.
    #
    
    
    ##
    # _browseProgram
    #    Use tk_getSaveFile to select a readout program.
    #
    method _browseProgram {} {
        set filename [tk_getOpenFile -title {Choose Readout} -parent $win \
            -defaultextension "" -filetypes [list \
                {{Exectuable files} *}            \
                {{Shell scripts}   .sh}           \
                {{Bash scripts}    .bash}         \
                {{Tcl scripts}     .tcl}          \
            ]]
        if {$filename ne ""} {
            set f [$wrapper controlarea]
            $f.program delete 0 end
            $f.program insert end $filename
        }
    }
}
##
# ::SSHPipe::promptParameters
#
#   Prompt for the parameters and turn them into the dict that
#   DataSourceUI::getParameters normally returns.
#
# @return dict Keys are parameter names, values are three element lists of
#              long prompt, dummy widget name, parameter value.
#
proc ::SSHPipe::promptParameters {} {
    ::SSHPipe::ParameterPromptDialog .sshpipeprompt
    
    set action [.sshpipeprompt modal]
    if {$action eq "Ok"} {
        set result [::SSHPipe::parameters]
        array set optionlookup [list host -host path -program parameters -parameters]
        dict for {key value} $result {
            dict lappend result $key [list] [.sshpipeprompt cget $optionlookup($key)]
        }
        
    } else {
        set result ""
        
    }
    destroy .sshpipeprompt
    return $result
}