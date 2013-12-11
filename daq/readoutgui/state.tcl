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
# @file state.tcl
# @brief State manager for save/restore arbitrary states.
# @author Ron Fox <fox@nscl.msu.edu>

package provide StateManager 1.0
package require snit

##
# @class StateManager
#
#  Provides a mechanism to allow packages in a program to save and restore
#  internal state.  This is done by associating state variables with
#  getters and setters and providing a changed method to indicate
#  when to write a new copy of the saved file.
#
#  Save files are simply Tcl scripts that contain a bunch of set commands.
#  Those commands will be executed in a safe interpreter.
#
# OPTIONS
#   -file  - Path to the file that will hold the save script.
#
# METHOD
#   addStateVariable   - Adds a new state variable to state manager
#   listStateVariables - lists the state variables in the state manager.
#   removeStateVariable - Removes a state variable from the registered list.
#   save               - Save the state (-file must be defined by now).
#   restore            - restore the state (-file must be defined by now).
#
snit::type StateManager {
    option -file
    
    
    # State variables are an ordered list so that
    # it is possible to define the order in which data are written to file
    # and restored from the file to internal state.
    #
    #  Each element of the list is a three element list containing:
    #  *  The variable name.
    #  *  A command that will provide the correct value for that variable.  This
    #     is parameterized by the variable name when called.
    #  * A command that will react to a restored value for this variable.
    #    This command is parameterized by the variable name and new value.
    #
    variable registeredVariables [list]
    
    ##
    # constructor
    #   Contstruct the object.  Simply process the configuration options.
    # @param args  option/value pairs.
    #
    constructor args {
        $self configurelist $args
    }
    ##
    # addStateVariable
    #   adds a stateVariable to the list
    #
    # @param name   - Name of the state variable.
    # @param getter - Command to get the variable value.
    # @param setter - Command to set the variable value.
    #
    method addStateVariable {name getter value} {
        if {[lsearch -exact -index 0 $registeredVariables $name] != -1} {
            error "There is already a registration for the variable '$name'"
        }
        lappend registeredVariables [list $name $getter $value]
    }
    ##
    # listStateVariables
    #
    # @return the current list of state variables.  This is a list whose
    #         elements are triples of name, getter, setter.
    #
    method listStateVariables {} {
        return $registeredVariables
    }
}