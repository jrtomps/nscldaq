#!/usr/bin/env tclsh

package provide FileListWidget 1.0

package require snit
package require Tk


snit::widget FileList {

  option -filelist  -default ""
  

  variable m_nJobs 0
  variable m_nAlreadyAdded 0

  constructor {args} {
    $self configurelist $args
    $self buildGUI 

  }  


  method buildGUI {} {
      ttk::treeview $win.list -show tree 
      $win.list column #0 -stretch on -minwidth 10
      ttk::scrollbar $win.xscroll -orient horizontal -command "$win.list xview"
      $win.list configure -xscrollcommand "$win.xscroll set"

      ttk::button   $win.addrun      -text "Add Files"    -command [mymethod onAddFiles]
      ttk::button   $win.rem      -text "Remove" -command [mymethod onRemove]

      grid $win.list              -  -sticky nsew
      grid $win.xscroll           -  -sticky ew
      grid $win.addrun $win.rem    -sticky ew
      grid rowconfigure $win    {0 2} -weight 1
      grid columnconfigure $win {0 1} -weight 1 -minsize 100
  }

  method onAddFiles {} {
    variable m_nAlreadyAdded
    set types {
                {{Evt Files} {.evt}}
                {{All Files} *     }
              } 

    # Request the names of files
    set selection [tk_getOpenFile -multiple true -filetypes $types ]
    # Add the selections
    if {$selection ne ""} {
      $win.list insert {} end -id job$m_nJobs -text "Job $m_nJobs"
    }

    foreach f $selection {
      $win.list insert job$m_nJobs end -id file$m_nAlreadyAdded \
                              -text $f
      incr m_nAlreadyAdded 
    }

    incr m_nJobs
  }


  method onRemove {} {
    set entries [$win.list selection]
    if {[llength $entries] == 0 } {
      tk_messageBox -icon error -message "User must select files to remove from list"
    }
    $win.list delete $entries
  }


  method getJobs {} {

    set jobs [dict create]

    set children [$win.list children {}]
    foreach child $children {
      set files [list]

      set jobchildren [$win.list children $child]
      foreach jobchild $jobchildren {
        set fname [$win.list item $jobchild -text]
        lappend files $fname
      }

      dict append jobs $child $files
    }

    return $jobs
  }
}
