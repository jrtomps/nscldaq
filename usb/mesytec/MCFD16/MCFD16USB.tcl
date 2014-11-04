



package provide mcfd16 1.0

package require snit

snit::type MCFD16USB {

  variable m_serialFile

  constructor {serialFile args} {
    set m_serialFile $serialFile

    $self configurelist $args
  }

  




}
