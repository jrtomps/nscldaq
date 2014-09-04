#!/usr/bin/env tclsh

package provide FileListWidget 1.0

package require snit
package require Tk


snit::widget FileList {

  option -filelist  -default ""
  

  variable m_nAlreadyAdded 0

  constructor {args} {
    $self configurelist $args
    $self buildGUI 

  }  


  method buildGUI {} {
      ttk::treeview $win.list -show tree 
#      tk::text $win.list -wrap none
      $win.list column #0 -stretch on -minwidth 10
      ttk::scrollbar $win.xscroll -orient horizontal -command "$win.list xview"
      $win.list configure -xscrollcommand "$win.xscroll set"

      ttk::button   $win.add      -text "Add"    -command [mymethod onAdd]
      ttk::button   $win.rem      -text "Remove" -command [mymethod onRemove]

      grid $win.list        -     -sticky nsew
      grid $win.xscroll     -     -sticky ew
      grid $win.add   $win.rem    -sticky ew
      grid rowconfigure $win    {0 2} -weight 1
      grid columnconfigure $win {0 1} -weight 1 -minsize 300
  }

  method onAdd {} {
    variable m_nAlreadyAdded
    set types {
                {{Evt Files} {.evt}}
                {{All Files} *     }
              } 

    # Request the names of files
    set selection [tk_getOpenFile -multiple true -filetypes $types ]
    # Add the selections
    foreach f $selection {
      $win.list insert {} end -id item$m_nAlreadyAdded \
                              -tag tag$m_nAlreadyAdded \
                              -text $f
#      $win.list insert end "$f\n"
      incr m_nAlreadyAdded 
    }
  }

  method onRemove {} {
    set entries [$win.list selection]
    if {[llength $entries] == 0 } {
      tk_messageBox -icon error -message "User must select files to remove from list"
    }
    $win.list delete $entries
  }
}
