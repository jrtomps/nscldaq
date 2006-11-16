package require ReadoutGUIPanel

proc epicsSet {name value} {
    puts "Setting $name -> $value"
}
proc epicsAccess name {
    puts "Setting up access to $name"
}
proc setTitle {name title} {
    ReadougGUIPanel::setTitle $title
}

puts "Actions sourced in"