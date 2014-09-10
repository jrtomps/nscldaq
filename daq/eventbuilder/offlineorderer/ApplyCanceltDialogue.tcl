

package provide ApplyCancelWidget 11.0

package require snit
package require Tk


snit::widget ApplyCancelWidgetView {

  option -confignames -default [list] 

  component m_presenter
  component m_combo
  component m_apply
  component m_cancel
  component m_configFrame

  variable m_current

  constructor {presenter args} {
    set m_presenter $presenter
    $self configurelist $args
  
    $self buildGUI
  }

  destructor {
  }

  method buildGUI {} {
    variable m_current

#    set top $win.buttons
#    ttk::frame $top
#    set m_apply $top.apply
#    set m_cancel $top.cancel
#    ttk::button $m_apply -text Apply -command [mymethod onApply] 
#    ttk::button $m_cancel -text Cancel -command [mymethod onCancel] 
#    grid $m_cancel -padx 9 -pady 9 -sticky ew
#    grid $m_cancel $m_apply -padx 9 -pady 9 -sticky ew
#    grid columnconfigure $top {0 1} -weight 1

    set m_configFrame $win.m_configFrame
    ttk::frame $m_configFrame

    set selFrame $win.select
    ttk::frame $selFrame
    set m_combo $selFrame.combo
    ttk::treeview $m_combo -show tree
    bind $m_combo <<TreeviewSelect>> [mymethod onSelectionChange]

    grid $m_combo     -sticky ns
    grid rowconfigure $selFrame 0 -weight 1
    


    grid $selFrame $m_configFrame -padx 9 -pady 9 -sticky nsew
#    grid     x       $win.buttons    -padx 9 -pady 9


    grid rowconfigure $win 0 -weight 1
    grid columnconfigure $win {1} -weight 1

  }


  method onApply {} {
    $m_presenter apply
  }

  method onCancel {} {
    $m_presenter cancel
  }

  method onSelectionChange {} {
    set current [$m_combo selection]
    $m_presenter changeSelection $current
  }

  method updateVisibleWidget {widget} {
    set slaves [grid slaves $m_configFrame]
    
    foreach slave $slaves {
      grid forget $slave
    }

    grid configure $widget -in $m_configFrame
    grid rowconfigure $m_configFrame 0 -weight 1
    grid columnconfigure $m_configFrame 0 -weight 1
  }

  method setNameList {newList} {
#    $self configure -confignames $newList
#    $m_combo configure -values $options(-confignames) 
    $m_combo delete [$m_combo children {}]
    foreach name $newList {
      $m_combo insert {} end -id "$name" -text $name 
    }

    puts [lindex $newList 0]
    $m_combo selection set [list [lindex $newList 0]]
  }

  method getWindowName {} {
    return $win
  }
}


snit::type ApplyCancelWidgetPresenter {

  option -widgetname -default "" 

  component m_view
  component m_presenterMap

  constructor {args} {

    set m_presenterMap [dict create]

    $self configurelist $args

    if {$options(-widgetname) eq ""} {
      set msg "ApplyCancelWidgetPresenter::constructor requires -widgetname "
      append msg "option but none provided."
      return -code error $msg
    }

    set m_view [ApplyCancelWidgetView $options(-widgetname) $self]
  }

  destructor {
    catch {destroy $m_view}
  }

  method apply {} {
    dict for {key value} $m_presenterMap {
      $value apply
    }
  }

  method cancel {} {
  }


  ## @brief Pass in the new presenter map
  #
  # @param  a dict of form: {widg0Name widg0Prsntr} {widg1Name widg1Prsntr} ...
  method setPresenterMap {presenterMap} {
    set m_presenterMap $presenterMap
    set nameList [dict keys $m_presenterMap]

    $m_view setNameList $nameList 

    $self changeSelection [lindex $nameList 0]
  }

  method getPresenterMap {} {
    return $m_presenterMap
  }

  method getViewObj {} {
    return  $m_view
  }
  
  method getViewWidget {} {
    return  [$m_view getWidgetName]
  }

  method changeSelection {name} {
    set name [string trim $name "{}"]
    set prsntr [dict get $m_presenterMap $name]
    set widget [$prsntr getViewWidget]
    $m_view updateVisibleWidget $widget
  }
}
