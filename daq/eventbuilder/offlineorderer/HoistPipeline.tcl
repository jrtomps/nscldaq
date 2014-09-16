


package provide OfflineEVBHoistPipeline 11.0
package require snit


snit::type OfflineEVBHoistPipeParams {

  option -sourcering  -default "OfflineEVBIn"
  option -tstamplib   -default ""
  option -id          -default 0
  option -info        -default "Data from OfflineEVBIn"
  option -expectbheaders -default 1

  constructor {args} {
    $self configurelist $args
  }

  method validate {} {
     set errors [list]

     $self validateSourceRing errors
     $self validateTstampLib errors
     $self validateId errors
     $self validateInfo errors

     return $errors

  }

  method validateSourceRing {errors_} {
    upvar $errors_ errors
    if {! [file exists [file join /dev shm $options(-sourcering)]]} {
       lappend errors "Ring $options(-sourcering) does not exist on localhost."
    }
  }

  method validateTstampLib {errors_} {
    upvar $errors_ errors
    # check for missing tstamplib
    if {$options(-tstamplib) eq "" } {
      # this is only bad if the -exectbheaders option is specified
      if { ! $options(-expectbheaders) } {
        lappend errors "Timestamp extractor library has not been specified and is mandatory."
      }
    } elseif {! [file exists $options(-tstamplib)] } {
       lappend errors "Timestamp extractor library \"$options(-tstamplib)\" does not exist."
    }
  }


  method validateId {errors_} {
    upvar $errors_ errors
    if {$options(-id) < 0} {
      lappend errors "Source id for ringFragmentSource must be greater than or equal to 0. User specified $options(-id)."
    }
  }

  method validateInfo {errors_} {
    upvar $errors_ errors
    if {$options(-info) eq ""} {
      lappend errors "Source info for ringFragmentSource has not been defined."
    }
  }

  method clone {} {
    
    # get all of the options and their values and make a dict of them
    set state [dict create]
    foreach opt [$self info options] {
      set value [$self cget $opt]
      dict set state $opt $value 
    }

    # return a new snit object with the same params
    return [[$self info type] %AUTO% {*}$state]
     
  }


  
}
