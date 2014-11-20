

package provide FrameSwitcher 1.0

package require snit
package require Tk

snit::widget FrameSwitcher {

  variable m_frameList
  variable m_current

  option -width  -default ""
  option -height -default ""

  constructor {args} {
    $self configurelist $args

    set m_frameList [dict create]
    set m_current ""

    $self buildGui
  }

  method buildGui {} {
    if {$options(-height) ne ""} {
      grid rowconfigure    $win 0 -weight 1 -minsize $options(-height)
    } else {
      grid rowconfigure    $win 0 -weight 1
    }
    if {$options(-width) ne ""} {
      grid columnconfigure $win 0 -weight 1 -minsize $options(-width) 
    } else {
      grid columnconfigure $win 0 -weight 1
    }
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
      set slaves [grid slaves $win]
      if {[llength $slaves]>0} {
        grid remove $slaves
      }

      set key [lindex $keys $index]
      set widget [dict get $m_frameList $key widget]

      grid $widget -in $win -sticky nsew

      grid rowconfigure    $win 0 -weight 1
      grid columnconfigure $win 0 -weight 1

      set script [dict get $m_frameList $key script]
      if {$script ne ""} {
        uplevel #0 eval [list $script]
      }
    }
  }
}

