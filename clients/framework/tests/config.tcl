set Integer 5
set Float   6.0
set Bool    true
set Char {This is a test string}

for {set i 0} { $i < 99 } {incr i} {
  set IArray($i) $i
  set FArray($i) $i
  set BArray($i) true
  set SArray($i) "Test string with $i in it"
}

foreach i {first second third last onemore} {
  set IAssoc($i) 1
  set FAssoc($i) 3.14159
  set BAssoc($i) true
  set CAssoc($i) $i
} 