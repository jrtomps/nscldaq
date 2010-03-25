package provide EventLoop 1.0

set command ""

proc prompt {} {
    if {$::command eq ""} {
	eval $::tcl_prompt1
    } else {
	eval $::tcl_prompt2
    }
}
proc processInput c {
    if {![eof $c]} {
	set line [gets $c]
	set continuation 0
	if {[regexp {\\$} $line]} {
	    set line [string range $line 0 end-1]
	    set continuation 1
	}
	append ::command $line
	if {!$continuation && [info complete $::command]} {
	    if {[string trimright $::command] ne ""} {
		set result [catch {uplevel #0 $::command} msg]
		if {$result} {
		    puts "ERROR - $msg"
		} else {
		    puts $msg
		}
	    }
	    set ::command ""
	}
	prompt
    } else {
	exit 0
    }
}



set tcl_prompt1 {
    puts -nonewline "% "
    flush stdout
}
set tcl_prompt2 {
    puts -nonewline "_ "
    flush stdout
}


proc startEventLoop {} {
    fileevent stdin readable [list processInput stdin]
    prompt
    vwait ::forever
}

proc stopEventLoop {} {
    set ::forever 1
    fileevent stdin readable [list]
    unset ::forever
}
