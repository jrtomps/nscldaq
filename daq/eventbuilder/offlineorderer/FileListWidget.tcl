#!/usr/bin/env tclsh

package provide FileListWidget 1.0

package require snit
package require Tk


snit::widget FileList {

  option -filelist  -default ""
  option -sort      -default 1
  

  variable m_nJobs 0
  variable m_nAlreadyAdded 0

  ##
  #
  constructor {args} {
    $self configurelist $args
    $self buildGUI 

  }  

  ##
  #
  method buildGUI {} {
      ttk::treeview $win.list -show tree 
      $win.list column #0 -stretch on -minwidth 10
      ttk::scrollbar $win.xscroll -orient horizontal -command "$win.list xview"
      $win.list configure -xscrollcommand "$win.xscroll set"

      ttk::button   $win.addrun      -text "Add Files"    -command [mymethod onAddFiles]
      ttk::button   $win.rem      -text "Remove Selected" -command [mymethod onRemove]

      grid $win.list              -  -sticky nsew
      grid $win.addrun $win.rem    -sticky ew
      grid rowconfigure $win    {0 1} -weight 1
      grid columnconfigure $win {0 1} -weight 1 -minsize 100
  }


  ##
  #
  method onAddFiles {} {
    set types {
                {{Evt Files} {.evt}}
                {{All Files} *     }
              } 

    # Request the names of files
    set selection [tk_getOpenFile -multiple true -filetypes $types ]

    if {$options(-sort)} {
      # Make sure that these are in increasing order
      set selection [lsort -increasing $selection]
    }

    $self appendFiles $selection

    $self sortList
  }

  ##
  #
  method appendFiles {selection} {
    # insert the new files from the selection
    foreach f $selection {
      $win.list insert {} end -id file$m_nAlreadyAdded \
                              -text [file tail $f] -value $f
      incr m_nAlreadyAdded 
    }
  }

  ##
  #
  method sortList {} {
    # order them
    # get the current list of children
    set children [$win.list children {}]

    # remove them from view but don't delete them
    $win.list detach $children

    # create a dict of (file name:id) pairs
    set map [dict create]
    foreach child $children {
      dict set map [$win.list item $child -text] $child
    }

    # sort the keys in ascending order
    set fnames [lsort -increasing [dict keys $map]]

    # put the sorted values back into the list in the 
    # new ordering
    foreach fname $fnames {
      $win.list move [dict get $map $fname] {} end
    }
  }

  ##
  #
  method onRemove {} {
    set entries [$win.list selection]
    if {[llength $entries] == 0 } {
      tk_messageBox -icon error -message "User must select files to remove from list"
    }
    $win.list delete $entries
  }

  # Ensure that this can work
  method clearTree {} {
    set nodes [$win.list children {}]
    $win.list delete $nodes
  }

  ##
  # Populate the tree data using the a dict of job lists
  method populateTree {filelist} {
    foreach file $fileList {
      $self insertFile $file 
    }
  }


  ## Package the job list into a dict
  #
  method getFiles {} {

    set files [list]

    set children [$win.list children {}]
    foreach child $children {

      set fname [$win.list item $child -value]
      lappend files $fname
    }

    return $files
  }


}
