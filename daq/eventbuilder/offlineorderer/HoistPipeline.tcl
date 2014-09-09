


package provide OfflineEVBHoistPipeline 11.0
package require snit


snit::type OfflineEVBHoistPipeParams {

  option -sourcering  -default "OfflineEVBIn"
  option -tstamplib   -default ""
  option -id          -default 0
  option -info        -default "Data from OfflineEVBIn"

  constructor {args} {
    $self configurelist $args
  }

  method validate {} {
     set errors [list]

     set ringErr      [$self validateSourceRing $errors]
     set tstampErr    [$self validateTstampLib $errors]
     set idErr        [$self validateId $errors]
     set infoErr      [$self validateInfo $errors]

     return $errors

  }

  method validateSourceRing {errors} {
    if {! [file exists [file join /dev shm $options(-sourcering)]]} {
       lappend errors "Ring $options(-sourcering) does not exist on localhost."
    }
  }

  method validateTstampLib {errors} {
    if {$options(-tstamplib) eq "" } {
       lappend errors "Timestamp extractor library has not been specified and is mandatory."
    } elseif {! [file exists $options(-tstamplib)] } {
       lappend errors "Timestamp extractor library \"$options(-tstamplib)\" does not exist."
    }
  }


  method validateId {errors} {
    if {$options(-id) < 0} {
      lappend errors "Source id for ringFragmentSource must be greater than or equal to 0. User specified $options(-id)."
    }
  }

  method validateInfo {errors} {
    if {$options(-info) eq ""} {
      lappend errors "Source info for ringFragmentSource has not been defined."
    }
  }
}
