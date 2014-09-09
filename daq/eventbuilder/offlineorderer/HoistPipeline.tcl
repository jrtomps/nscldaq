


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
}
