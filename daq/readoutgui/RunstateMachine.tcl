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
# @file RunstateMachine.tcl
# @brief Run State Machine Package
# @author Ron Fox <fox@nscl.msu.edu>

package provide RunstateMachine 1.0
package require snit

##
# @class RunstateMachine
#
#    This snit object provides a run state machine for the readoutGUI
#    The statemachine relies on action callouts that are called when
#    entering an leaving states.  Callouts are managed as 'bundles'  which are
#    basically namespaces with well defined commands exported.
#    These commands are:
#      *  enter  - Called when entering a state, passed the old state, newstate
#      *  leave  - Called when leaving a state, passed the old state, new state.
#      *  attach - Called when added to the callouts list.  Passed the
#                  current state.
#
#    It is up gto the client to ensure the namespaces registered exist.
#    Note that a side effect of registration is to create a namespace
#    ensemble out of the namespace.
#
# States are:
#    *  NotReady - No data sources are active.
#    *  Starting - The data sources are now being started.
#    *  Halted   - Data sources are active, but the run is not.
#    *  Active   - Data taking is in progress.
#    *  Paused   - Data taking is paused.
#
#  All state transitions are triggered by the transition method.
#  Legality of transitions are checked against the state machine definition
#  and errors thrown if an illegal transition is attempted.
#
# METHODS
#   listCalloutBundles
#   addCalloutBundle
#   removeCalloutBundle
#   transition
#   listStates
#   listTransitions
#   getState
#
#  Any methods that start with _ are considered private.  Note that
#
#   _setState  will be used by the tests to force the state machine to a
#              specific state prior to a test.  This method does not invoke
#              any bundle methods, nor does it check the legality of the state
#              transition being forced.
#
snit::type RunstateMachine {
    #--------------------------------------------------------------------------
    #  Class variables:
    
    typevariable validTransitions -array {
        NotReady {Starting}
        Starting {NotReady Halted}
        Halted   {NotReady Active}
        Active   {NotReady Halted Paused}
        Paused   {NotReady Halted Active}
    }
    
    # Arra indexed by callback name contents are the number of parameters the
    # callback must accept.
    
    typevariable requiredExports -array [list \
        enter 2 leave 2 attach 1]
    
    #---------------------------------------------------------------------------
    #  Instance variables
    
    variable state NotReady
    
    #
    #   validTransitions defines the legal edges of the state machine.
    #   this is an array indexed by state name with values a list of valid transitions.
    #
    
    
    variable callouts [list]
    
    ##
    # constructor
    #   Does nothing at this time.
    #
    constructor args {
        
    }
    #---------------------------------------------------------------------------
    # Public methods:
    #
    
    ##
    # listSates
    #
    #  Provides a list of the states the machine knows about. Note that
    #  the states are alphabetized so that the order is predictable.
    #
    # @return list  alphabetized list of allowed states.
    method listStates {} {
        lsort -ascii -increasing [array names validTransitions]
    }
    ##
    # listTransitions
    #
    #   Given a state lists the valid target states for the transition method
    #
    # @param stateName - The state being queried.
    # @return list     - list of valid next states.
    #
    method listTransitions stateName {
        return $validTransitions($stateName)
    }
    ##
    # getState
    #  Returns the current state name
    #
    # @return string - name of current state
    #
    method getState {} {
        return $state
    }
    ##
    # listCalloutBundles
    #
    #   Lists the set of namespaces that are established as callout bundles.
    #   This list defines what happens on state transitions...specifically,
    #   on leaving an old state and on entering a new state.
    #
    method listCalloutBundles {} {
        return $callouts
    }
    
    ##
    # addCalloutBundle
    #
    #   Adds a new callout bundle to the state machine.  To be added (preconditions);
    #   *   The stated namespace must exist.
    #   *   The stated namespace must not already be registered.
    #   *   The stated namespace must export the attach, enter,leave procs.
    #   *   The stated namespace's attach, enter, leave procs must have the
    #       correct number of parameters.
    #
    # @param name - Name of the bundle to add.  This is assumed to be relative to
    #               the global namespace e.g. junk -> ::junk
    #
    # @note When successful (postconditions):
    #   *  The bundle name is added to the callouts variable.
    #   *  A namespace ensemble is generated for that bundle.
    #
    method addCalloutBundle name {
        if {![namespace exists ::$name]} {
            error "No such bundle $name"
        }
        if {[lsearch -exact $callouts $name] != -1} {
            error "$name is already registered as a callback bundle"
        }
        $self _checkRequiredExports $name
        $self _checkRequiredParams $name
        

        #  Now we can make a namespace ensemble from the bundle and
        #  add it to the callout list.
        
        namespace eval ::${name} {
            namespace ensemble create
        }
        
        lappend callouts $name
        
        # Finally invoke the attach method:
        
        ::$name attach $state
    }
    
    ##
    # removeCalloutBundle
    #
    #   Remove a callout bundle:
    #   * Ensure the bundle is registered.
    #   * Remove it from the callouts list.
    #   * Undefine its namespace ensemble.
    #
    # @param name - Name of namespace that holds the bundle methods.
    #
    method removeCalloutBundle name {
        set index [lsearch -exact $callouts $name]
        if {$index == -1} {
            error "$name has not been registered"
        }
        set callouts [lreplace $callouts $index $index]
        rename ::${name} {}
    }
    ##
    # transition
    #
    #    Request a state transition.
    #
    # @param to   - Target state.
    #
    # @error the transition is to a prohibited state.
    #
    method transition to {
        if {$to in $validTransitions($state)} {
            
            $self _callback leave $state $to
            set from $state
            set state $to
            $self _callback enter $from $to
        
        } else {
            error "The transtion from $state to $to is not allowed"
        }
    }
    
    #--------------------------------------------------------------------------
    #
    # Private methods
    #
    ##
    # _setState
    #   Set the state (testing method).
    #
    # @param newState - Requested new state
    #
    # @note no callbacks are performed and no legality checking is done.
    #
    method _setState newState {
        set state $newState
    }
    ##
    #  _callback
    #    Perform a callback in the list of callbacks.
    #
    # @param callback - Name of the namespace callback method (e.g. enter)
    # @param args     - Arguments to hand to the callback method
    #
    method _callback {method args} {
        foreach cb $callouts {
            $cb $method {*}$args
        }
    }
    
    ##
    # _checkRequiredExports
    #
    #   Checks that a namespace has the required exports for a callback bundle.
    #   Throws an error if not.
    #
    # @param name - Name of the namespace to check
    # @throw If one or more exports is missing.
    #
    method _checkRequiredExports name {
        
        # Make a list of missing exported procs.  The only way to figure out which
        # procs are exported from a namespace is to do a namespace export in the
        # context of the namespace itself:
        
        set exports [namespace eval ::${name} { namespace export}]
        set missingProcs [list]
        
        #
        #  The sort below is done to make the output repeatable/predictble
        #  and hence testable.
        #
        foreach proc [lsort -ascii -increasing [array names requiredExports]] {
            if {[lsearch -exact $exports $proc] == -1} {
                lappend missingProcs $proc
            }
        }
        if {[llength $missingProcs] > 0} {
            set missingList [join $missingProcs ", "]
            error "$name is missing the following exported procs: $missingList"
        }
    }
    ##
    # _checkRequiredParams
    #
    #  Checks that a namespace that is being proposed as a callback bundle
    #  has the right number of parameters for each of the required exported procs.
    #
    # @param name - Path to the namespace relative to ::
    #
    # @throw If one or more required exports has the wrong argument signature.
    #
    method _checkRequiredParams name {
        
        set badProcs [list]
        set actualArgs [list]
        set requiredArgs [list]
        foreach proc [lsort -ascii -increasing [array names requiredExports]] {
            set params [llength [info args ::${name}::${proc}]]
            if {$params != $requiredExports($proc)} {
                lappend badProcs     $proc
                lappend actualArgs   $params
                lappend requiredArgs $requiredExports($proc)
            }
        }
        if {[llength $badProcs] > 0} {
            foreach proc $badProcs required $requiredArgs actual $actualArgs {
                lappend badList       ${proc}(${actual})
                lappend requiredList  ${proc}(${required})
            }
            set badList [join $badList ", "]
            set requiredList [join $requiredList ", "]
            error "$name has interface procs with the wrong number of params: $badList should be: $requiredList"
        }
    }
}

##
# @class RunstateMachineSingleton
#
#   This class should be created rather than a RunstateMachine.  It is basically
#   a facade that enforces the singleton pattern on the RunstateMachine type.
#
#
snit::type RunstateMachineSingleton {
    component StateMachine
    delegate method * to StateMachine
    delegate option * to StateMachine
    
    typevariable actualObject ""
    
    ##
    #  The constructor creates actuaObject if necessary
    #  once done with that, installs it as the StateMachine
    #
    constructor args {
        if {$actualObject eq ""} {
            set actualObject [RunstateMachine %AUTO%]
        
        }
        # The delegates and this magic take care of the rest.
        
        install StateMachine using set actualObject
        
    }
}