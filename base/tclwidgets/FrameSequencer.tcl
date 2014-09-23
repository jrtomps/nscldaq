
package provide FrameSequencer 1.0

package require snit
package require Tk

snit::widget FrameSequencer {

  variable m_frameList
  variable m_current

  constructor {args} {
    set m_frameList [dict create]
    set m_current ""

    $self buildGui
  }

  method buildGui {} {

    ttk::frame $win.buttons
    ttk::button $win.buttons.back -text "Back" -command [mymethod onBack] -style "Flat.TButton"
    grid $win.buttons.back -sticky nw
    grid rowconfigure    $win.buttons 0 -weight 1
    grid columnconfigure $win.buttons 0 -weight 1

    ttk::frame $win.content
   
    grid $win.content -sticky nsew 
    grid $win.buttons -sticky new

    grid rowconfigure    $win {0 1} -weight 1
    grid rowconfigure    $win 0 -minsize 300
    grid columnconfigure $win 0 -weight 1 -minsize 300
  }

  method add {name fr {onviewscript ""}} {
    if {[dict size $m_frameList]==0} {
      set m_current $name
    }
    dict set m_frameList $name [dict create widget $fr script $onviewscript]
  }

  method select {name} {
    set keys [dict keys $m_frameList]
    set m_current $name

    set index [lsearch $keys $name]
    if {$index >= 0} {
      set slaves [grid slaves $win.content]
      if {[llength $slaves]>0} {
        grid remove $slaves
      }

      set key [lindex $keys $index]
      set widget [dict get $m_frameList $key widget]

      grid $widget -in $win.content -sticky nsew

      grid rowconfigure    $win.content 0 -weight 1
      grid columnconfigure $win.content 0 -weight 1

      set script [dict get $m_frameList $key script]
      if {$script ne ""} {
        uplevel #0 eval [list $script]
      }
    }

    if {$index > 0} {
      grid $win.buttons.back -sticky nw
    } else {
      grid remove $win.buttons.back
    }

  }


  method onBack {} {
    set keys [dict keys $m_frameList]
    set index [lsearch $keys $m_current]
    if {$index > 0} {
      set newname [lindex $keys [incr index -1]]
      $self select $newname
    }
  }
  
}

