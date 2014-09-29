

package provide MockCCUSB 1.0

package require snit


snit::type MockCCUSB {

  variable m_history

  constructor {args} {

  }

  method simpleWrite24 {n a f data} {
    lappend m_history "simpleWrite24 $n $a $f $data"
  }


  method getHistory {} {
    return $m_history
  }
}

