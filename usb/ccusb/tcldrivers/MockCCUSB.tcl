

package provide MockCCUSB 1.0

package require snit

namespace eval MockCCUSB {

  snit::type Controller {

    variable m_history

    constructor {args} {
      set m_history [list]
    }

    method simpleWrite24 {n a f data} {
      lappend m_history "simpleWrite24 $n $a $f $data"
    }

    method simpleRead24 {n a f} {
      lappend m_history "simpleRead24 $n $a $f"
    }

    method simpleControl {n a f} {
      lappend m_history "simpleControl $n $a $f"
    }

    method getHistory {} {
      return $m_history
    }
  }



  snit::type ReadoutList {
    variable m_history

    constructor {args} {
      set m_history [list]
    }


    method addControl {n a f} {
      lappend m_history "control $n $a $f"
    }

    method addRead16 {n a f} {
      lappend m_history "read16 $n $a $f"
    }

    method getHistory {} {
      return $m_history
    }

  }


}

