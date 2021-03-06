
package provide mscf16fileloader 1.0

package require mscf16gui
package require mscf16guiapp
package require mscf16memorizer
package require BlockCompleter
package require TclSourceFilter

## @brief Logic for the LoadFromFileView
#
# The logic for this operation is more complicated than in most parts of this
# gui in order to ensure that loading from a file is safe. The main
# functionality of this is to respond to the events generated by the view,
# react, and the update the view. The complexity lies in the fact that it parses
# the file to load for only the allowed API calls. In this way, the method
# prevents the execution of lines like: exec rm *. The logic does not go so far
# as to totally prevent such a thing from being executed but does reduce the
# risk. It also writes the names to the file. One could argue that some of that
# logic should live in a separate snit::type, but for the moment it is the
# responsibility of this.
#
# It is possible to construct this snit::type without an associated view to
# control. This makes it easier to test instances.
#
snit::type MSCF16FileLoader {

  variable _opts
  variable _presenter

  component _filter
  
  ## @brief Construct and set up the state
  #
  # @param presenter    an MCFD16ControlPanel instance
  # @param args         option-value pairs
  #
  constructor {opts presenter} {
    set _opts $opts
    set _presenter $presenter 
    
    install _filter using TclSourceFilter %AUTO%
    $_filter SetValidPatterns $_validAPICalls
  }
  
  destructor {
    $_filter destroy
  }

  ## @brief Load a file into memory and use it to set the state of the Gui
  #
  # Comments are provided inline to facilitate the understanding of the logic.
  # FOr the most part, this reads evaluable chunks of a file into a list,
  # removes all chunks that are not relevant, recognized MCFD16 driver API calls
  # or lines that set the name from the list, creates an MCFD16Memorizer that
  # will be operated on by the list elements, swaps in an MCFD16Memorizer to the
  # MCFD16ControlPanel, updates the MCFD16ControlPanel using the memorizer, and
  # then replaces the memorizer with the real driver.
  #
  # @param  path    the path to a file containing state 
  method Load {path} {
    if {![file exists $path]} {
      set msg "Cannot load from $path, because file does not exist."
      tk_messageBox -icon error -message $msg
    } elseif {! [file readable $path]} { 
      set msg "Cannot load from $path, because file is not readable."
      tk_messageBox -icon error -message $msg
    }

    set scriptFile [open $path r]
    set content [chan read $scriptFile]
    set executableLines [$_filter Filter $content]


    # determine the first
    set devName [$self ExtractDeviceName [lindex $executableLines 0]]
    if {[llength [info commands $devName]]>0} {
      if {[$_presenter cget -handle] ne $devName} {
        rename $devName {}
      } else {
        set msg "Device driver instance in load file shares same name "
        append msg "as current driver ($devName). User must use a different "
        append msg "instance name in the load file."
        tk_messageBox -icon error -message $msg
        return
      }
    }

    set fakeHandle [MSCF16Memorizer $devName]

    # load state into device
    $self EvaluateAPILines $executableLines

    # update the actual content, swapping in the handle triggers the 
    # view to be updated...
    $_presenter configure -autoupdate 1
    set realHandle [$self SwapInHandle $fakeHandle]

    # pass the real handle back in, but make sure that that does not 
    # trigger an UpdateViewFromModel. So... we turn autoupdate off, then
    # the real handle can be put back. 
    $_presenter configure -autoupdate 0
    set fakeHandle [$self SwapInHandle $realHandle]

    # now make sure that the state of the view makes it to the device.
    $_presenter Commit

    $fakeHandle destroy
    catch {close $loadFile}
  }

  ## @brief Replace current handle with a new handle
  #
  # @param  newHandle name of the instance to pass into the MCFD16ControlPanel
  #
  # @returns the name of the previous instance managed by the MCFD16ControlPanel
  method SwapInHandle {newHandle} {
    set oldHandle [$_presenter cget -handle]
    $_presenter configure -handle $newHandle

    return $oldHandle
  }

  ## @brief Retrieve name of the device driver from a line
  #
  # Given a line that is a driver call, the first element is the driver name.
  # This first element is extracted. It is up to the caller to ensure that the
  # line is actually an API call.
  #
  # @param line   the line of code to look in.
  #
  # @returns a device name specified as existing at the global scope
  method ExtractDeviceName {line} {
    set tokens [split $line " "]
    set name [lindex $tokens 0]
    if {[string first "::" $name] != 0} {
      set name "::$name"
    }
    return $name
  }

  method EvaluateAPILines {lines} {
    foreach line $lines {
      uplevel #0 eval $line
    }
  }
  # Type data .... 
  typevariable _validAPICalls ;# list of calls consider valid API calls

  ## @brief Populate the list of valid api calls
  #
  typeconstructor {
    set _validAPICalls [list]
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+SetThreshold\s+\d+\s+\d+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+GetThreshold\s+\d+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+SetGain\s+\d+\s+\d+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+GetGain\s+\d+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+SetPoleZero\s+\d+\s+\d+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+GetPoleZero\s+\d+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+SetShapingTime\s+\d+\s+\d+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+GetShapingTime\s+\d+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+SetMode\s+(common|individual)$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+GetMode$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+SetMonitor\s+\d+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+GetMonitor$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+EnableRC\s+\w+$}
    lappend _validAPICalls {^\s*[[:alnum:]:]+\s+RCEnabled$}
  }
}

snit::type MSCF16NameLoader {

  component _filter
  
  ## @brief Construct and set up the state
  #
  # @param presenter    an MCFD16ControlPanel instance
  # @param args         option-value pairs
  #
  constructor {} {
    install _filter using TclSourceFilter %AUTO%
    $_filter SetValidPatterns $_validAPICalls
  }
  
  destructor {
    $_filter destroy
  }

  method Load {path} {
    if {![file exists $path]} {
      set msg "Cannot load from $path, because file does not exist."
      tk_messageBox -icon error -message $msg
      return
    } elseif {! [file readable $path]} { 
      set msg "Cannot load from $path, because file is not readable."
      tk_messageBox -icon error -message $msg
      return
    }

    set scriptFile [open $path r]
    set content [chan read $scriptFile]
    close $scriptFile

    set executableLines [$_filter Filter $content]

    $self EvaluateAPILines $executableLines
  }

  method EvaluateAPILines {lines} {
    foreach line $lines {
      uplevel #0 eval $line
    }
  }
  # Type data .... 
  typevariable _validAPICalls ;# list of calls consider valid API calls

  ## @brief Populate the list of valid api calls
  #
  typeconstructor {
    set _validAPICalls [list]
    lappend _validAPICalls {^\s*set\s+(::)*MSCF16ChannelNames::chan\d+.+$}
    lappend _validAPICalls {^\s*namespace\s+::MSCF16ChannelNames\s+eval\s+{\s*}\s*$}
  }
}
