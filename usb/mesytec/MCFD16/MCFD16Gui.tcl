

package provide mcfd16gui 1.0

package require mcfd16usb
package require snit
package require Tk
package require FrameSwitcher
package require scriptheadergenerator
package require BlockCompleter


####### "Base" type for MCFD16View ################

snit::type MCFD16View {

  option -presenter -default {} ;# logical controller of device

  variable _mcfd ;# basic array of values

  ## Constructor
  constructor {args} {
    $self configurelist $args

    $self InitArray 
  }


  ## Retrieve the fully qualified name of the array
  #
  method mcfd {} {
    return [$self info vars _mcfd]
  }

  ## Check whether a channel name contains non-whitespace characters
  # 
  # This is called when a channel entry loses focus
  #
  # @param name   candidate string 
  #
  # @return boolean
  # @retval 0 - string was empty or all whitespace
  # @retval 1 - otherwise
  method ValidateName {name} {
    set name [string trim $name]
    return [expr [string length $name]!=0]
  }

  ## Reset channel to a simple string
  #
  # Typically called with ValidateName returns false
  #
  # @returns ""
  method ResetChannelName {widget} {
    set str [$widget cget -textvariable]
    regexp {^.*(\d+)$} $widget match ch
    set $str "Ch$ch"
  }

  ## Initialize all elements of the array for each setting
  #
  method InitArray {} { 

    # set the channel valus
    for {set i 0} {$i < 16} {incr i} {

      set _mcfd(na$i) "Ch$i" 
      set _mcfd(th$i) 127
      set _mcfd(mo$i) 4 

      if {($i%2)==0} {
        set pair [expr $i/2]
        set _mcfd(po$pair) neg
        set _mcfd(ga$pair) 1
        set _mcfd(wi$pair) 100 
        set _mcfd(dt$pair) 100 
        set _mcfd(dl$pair) 1
        set _mcfd(fr$pair) 20
      }
    }

    # set the common channels
    set _mcfd(po8) neg
    set _mcfd(ga8) 1
    set _mcfd(wi8) 50
    set _mcfd(dt8) 50
    set _mcfd(dl8) 1
    set _mcfd(fr8) 20
    set _mcfd(th16) 127
  }

  # Getters and Setters
  method GetName {ch} { return $_mcfd(na$ch)  }
  method SetName {ch str} { set _mcfd(na$ch) $val }

  method GetThreshold {ch} {  return $_mcfd(th$ch) }
  method SetThreshold {ch val} { set _mcfd(th$ch) $val }

  method GetMonitor {ch} {  return $_mcfd(mo$ch) }
  method SetMonitor {ch onoff} {  set _mcfd(mo$ch) $onoff }

  method GetPolarity {ch} {  return $_mcfd(po$ch) }
  method SetPolarity {ch pol} {  set _mcfd(po$ch) $pol }

  method GetGain {ch} {  return $_mcfd(ga$ch) }
  method SetGain {ch gain} {  set _mcfd(ga$ch) $gain }

  method GetWidth {ch} {  return $_mcfd(wi$ch) }
  method SetWidth {ch width} {  set _mcfd(wi$ch) $width }

  method GetDeadtime {ch} {  return $_mcfd(dt$ch) }
  method SetDeadtime {ch time} {  set _mcfd(dt$ch) $time }

  method GetDelay {ch} {  return $_mcfd(dl$ch) }
  method SetDelay {ch time} {  set _mcfd(dl$ch) $time }

  method GetFraction {ch} {  return $_mcfd(fr$ch) }
  method SetFraction {ch frac} {  set _mcfd(fr$ch) $frac }

  method GetMode {} {
    set mode ""
    if {$_mcfd(mode) eq "on"} {
      set mode "common"
    } else {
      set mode "individual"
    }
    return $mode
  }

  method SetMode {mode} {
    if {$mode eq "common"} {
      set _mcfd(mode) on
    } elseif {$mode eq "individual"} {
      set _mcfd(mode) on
    } else {
      set msg {Invalid mode. Must be either common or individual.}
      return -code error MCFD16View::SetMode $msg
    }
  }

  ##  All MCFD16View-like types simple pass events to their presenter
  #
  method Commit {} {
    $options(-presenter) Commit
  }

}; # end of the View


###############################################################################
#
# MCFD16IndividualView 
#
#  A widget for controlling the MCFD16 channels individually. This creates a 
#  16-row table with a header. There is a row for each of the channels in the
#  device and also they are grouped into pairs to make the configuration simpler
#  to understand.
#
snit::widget MCFD16IndividualView {

  component m_base ;# the MCFD16View instance

  delegate method * to m_base
  delegate option * to m_base

  ## Constructs the gui and parses options
  #
  # @param args   option-value pairs (see MCFD16View for supported options)
  constructor {args} {
    install m_base using MCFD16View %AUTO%

    $self configurelist $args 

    $self BuildGUI

  }

  ## Put the widgets together
  #
  #
  method BuildGUI {} {

    $self BuildHeader $win.header
    $self BuildTable $win.table

    grid $win.header -sticky nsew -padx 2 -pady 2
    grid $win.table -sticky nsew -padx 2 -pady 2

    # allow the columns to resize
    grid columnconfigure $win {0} -weight 1
    grid rowconfigure $win {0} -weight 1
  }


  ## Construct the header row
  #
  # @param name   the name of the frame that will hold the row
  method BuildHeader {name} {

    set w $name
    ttk::frame $w -style "Header.TFrame"

    ttk::label $w.na -text Name -style "Header.TLabel" -padding "3 3 3 0" \
                                -width 8     
    ttk::label $w.ch -text Channel  -style "Header.TLabel" -padding "3 3 3 0" 


    ttk::label $w.po -text Polarity  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.ga -text Gain  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.th -text Threshold  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.wi -text Width -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dt -text Deadtime  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dl -text Delayline -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.fr -text Fraction -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.mo -text Monitor -style "Header.TLabel" -padding "3 3 3 0"

    grid $w.na $w.th $w.po $w.ga $w.wi $w.dt $w.dl $w.fr \
      -sticky sew -padx 4 -pady 4
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0} -weight 1
  }

  ## Construct the 16 rows of the table
  #
  # @param name   the name of the frame that will hold the row
  #
  method BuildTable {name } {
    set w $name
    ttk::frame $w 

    set rows [list]
    for {set ch 0} {$ch < 16} {incr ch 2} { 
      if {(($ch/2)%2)==0} {
        set style Even
      } else {
        set style Odd
      }

      set name "$w.rows[expr $ch/2]"
      $self BuildGroupedRows $name $ch $style
      lappend rows $name
    }

    foreach row $rows {
      grid $row -sticky nsew -padx 2 -pady 2
    }
    grid columnconfigure $w {0} -weight 1
    grid rowconfigure $w {0 1 2 3 } -weight 1
  }

  ## Constructs a pair of rows in the table
  #
  # @param name   name of the frame that will be filled 
  #
  method BuildGroupedRows {name ch style} {
    set w $name
    ttk::frame $w -style $style.TFrame

    # construct first row
    ttk::entry $w.na$ch -width 8 -textvariable "[$self mcfd](na$ch)" \
                        -style "$style.TEntry" \
                                -validate focus -validatecommand [mymethod ValidateName %P] \
                                -invalidcommand [mymethod ResetChannelName %W]

    ttk::spinbox $w.th$ch -textvariable "[$self mcfd](th$ch)" -width 4 \
                        -style "$style.TSpinbox" -from 0 -to 255 \
                        -state readonly
    ttk::radiobutton $w.mo$ch -variable "[$self mcfd](monitor)" -value $ch \
                        -command Monitor -style "$style.TRadiobutton"

    set pair [expr $ch/2]
    ttk::radiobutton $w.poneg$pair -variable "[$self mcfd](po$pair)" \
                                   -value neg -text neg \
                                   -style "$style.TRadiobutton"

    ttk::spinbox $w.ga$pair -textvariable "[$self mcfd](ga$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -values {0 1 2} -state readonly

    ttk::spinbox $w.wi$pair -textvariable "[$self mcfd](wi$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -from 16 -to 222 -state readonly

    ttk::spinbox $w.dt$pair -textvariable "[$self mcfd](dt$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -from 27 -to 222 -state readonly

    ttk::spinbox $w.dl$pair -textvariable "[$self mcfd](dl$pair)" \
      -style "$style.TSpinbox" -width 4 \
      -from 0 -to 4 -state readonly

    ttk::radiobutton $w.fr${pair}20 -text "20%" -variable "[$self mcfd](fr$pair)" \
                                    -value 20  -style "$style.TRadiobutton"

    grid $w.na$ch $w.th$ch $w.poneg$pair $w.ga$pair $w.wi$pair \
      $w.dt$pair $w.dl$pair $w.fr${pair}20 \
      -sticky news -padx 4 -pady 4
    grid configure $w.ga$pair $w.wi$pair $w.dt$pair \
                    $w.dl$pair -rowspan 2

    # construct second row 
    incr ch
    ttk::entry $w.na$ch -width 8 -textvariable "[$self mcfd](na$ch)" \
      -style "$style.TEntry" \
      -validate focus -validatecommand [mymethod ValidateName %P] \
                      -invalidcommand [mymethod ResetChannelName %W]
    ttk::spinbox $w.th$ch -textvariable "[$self mcfd](th$ch)" -width 4 \
      -style "$style.TSpinbox" -from 0 -to 255 \
      -state readonly

    ttk::radiobutton $w.popos$pair -variable "[$self mcfd](po$pair)" \
                                   -value pos -text pos \
                                   -style "$style.TRadiobutton"
    ttk::radiobutton $w.fr${pair}40 -text "40%" -variable "[$self mcfd](fr$pair)" \
                                    -value 40  -style "$style.TRadiobutton"
    ttk::radiobutton $w.mo$ch -variable "[$self mcfd](mo$pair)" \
      -value $ch \
      -command Monitor -style "$style.TRadiobutton"
    grid $w.na$ch $w.th$ch $w.popos$pair x x x x $w.fr${pair}40 -sticky news -padx 4 -pady 4

    # allow the columns to resize
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0 1} -weight 1
  }
}

###############################################################################
#
# MCFD16CommonView
#
#  A widget for controlling common channel values in MCFD16. This is just one 
#  row of controls that will set the parameters for common mode.
#
snit::widget MCFD16CommonView {

  component m_base ;# the MCFD16View

  delegate method * to m_base
  delegate option * to m_base

  ## Construct the megawidget and process options
  #
  # @param args   option-value pairs (see MCFD16View for valid options)
  constructor {args} {
    install m_base using MCFD16View %AUTO%

    $self configurelist $args 

    $self BuildGUI
  }

  ## Construct the header and the row of controls
  method BuildGUI {} {

    $self BuildHeader $win.header
    $self BuildCommonControls $win.common


    grid $win.header -sticky nsew -padx 2 -pady 2
    grid $win.common -sticky nsew -padx 2 -pady 2

    # allow the columns to resize
    grid columnconfigure $win {0} -weight 1
    grid rowconfigure $win {0} -weight 1
  }

  ## Build the header
  # 
  # @param name    the name of container frame
  #
  method BuildHeader {name} {

    set w $name
    ttk::frame $w -style "Header.TFrame"

    ttk::label $w.na -text Name -style "Header.TLabel" -padding "3 3 3 0" \
                                -width 8     
    ttk::label $w.ch -text Channel  -style "Header.TLabel" -padding "3 3 3 0" 


    ttk::label $w.po -text Polarity  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.ga -text Gain  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.th -text Threshold  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.wi -text Width -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dt -text Deadtime  -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.dl -text Delayline -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.fr -text Fraction -style "Header.TLabel" -padding "3 3 3 0"
    ttk::label $w.mo -text Monitor -style "Header.TLabel" -padding "3 3 3 0"

    grid $w.na $w.th $w.po $w.ga $w.wi $w.dt $w.dl $w.fr \
      -sticky sew -padx 4 -pady 4
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
    grid rowconfigure $w {0} -weight 1
  }

  ## Build the controls
  # 
  # @param name    the name of container frame
  #
  method BuildCommonControls {name} {
    set w $name

    ttk::frame $w -style "Even.TFrame"

    ttk::label $w.name -text Common  -style "Even.TLabel"

    ttk::radiobutton $w.poneg8 -text "neg" -variable [$self mcfd](po8) \
                               -value neg -style "Even.TRadiobutton"
    ttk::radiobutton $w.popos8 -text "pos" -variable [$self mcfd](po8) \
                               -value pos -style "Even.TRadiobutton"
    ttk::spinbox $w.ga8 -textvariable [$self mcfd](ga8) -width 4 \
                               -values {0 1 2} -style "Even.TSpinbox" \
                               -state readonly
    ttk::spinbox $w.wi8 -textvariable [$self mcfd](wi8) -width 4 \
                               -from 16 -to 222 -style "Even.TSpinbox" \
                               -state readonly
    ttk::spinbox $w.dt8 -textvariable [$self mcfd](dt8) -width 4 -from 27 \
                               -to 222 -style "Even.TSpinbox" -state readonly
    ttk::spinbox $w.dl8 -textvariable [$self mcfd](dl8) -width 4 -from 0 \
                               -to 4 -style "Even.TSpinbox" -state readonly
    ttk::radiobutton $w.fr820 -text "20%" -variable [$self mcfd](fr8) \
                              -value 20 -style "Even.TRadiobutton"
    ttk::radiobutton $w.fr840 -text "40%" -variable [$self mcfd](fr8) -value 40 -style "Even.TRadiobutton"
    ttk::spinbox $w.th16 -textvariable [$self mcfd](th16) -width 4 -from 0 -to 255 -style "Even.TSpinbox" -state readonly

    grid $w.name $w.th16 $w.poneg8 $w.ga8 $w.wi8 $w.dt8 $w.dl8 $w.fr820 -sticky news -padx 4 -pady 4
    grid ^       ^       $w.popos8 ^      ^      ^      ^      $w.fr840 -sticky news -padx 4 -pady 4
    grid columnconfigure $w {1 2 3 4 5 6 7} -weight 1 -uniform a
    grid columnconfigure $w {0} -weight 2 -uniform a
  }
}


###############################################################################
###############################################################################
###############################################################################

# PRESENTERS For the views


## Common functionality for MCFD16CommonPresenter and MCFD16IndividualPresenter
#
#
snit::type MCFD16Presenter {

  option -widgetname -default "" ;# name of the view
  option -handle -default ""     ;# device handle (a low-level driver)
  
  option -view -default ""       ;# view instance (owned by this)

  ## Parse options
  #
  constructor {args} {
    $self configurelist $args
  }

  ## Destroy the view
  destructor {
    catch {destroy $options(-view)}
  }


  ## Syncronization utility method for writing view parameter to device
  #
  method CommitViewToParam {param index} {
    [$self cget -handle] Set$param $index [[$self cget -view] Get$param $index]
  }

  ## Syncronization utility for looping CommitViewToParam
  #
  method LoopCommitView {param begin end} {
    for {set ch $begin} {$ch<$end} {incr ch} {
      $self CommitViewToParam $param $ch
    }
  }

  ## Syncronization utility for setting view state based on device state
  #
  method UpdateParamFromView {param index} {
    [$self cget -view] Set$param $index [[$self cget -handle] Get$param $index]
#    puts "$param $index [[$self cget -handle] Get$param $index]"
  }

  ## Sychronization utility for loop UpdateParamFromView
  #
  method LoopUpdateView {param begin end} {
    for {set ch $begin} {$ch<$end} {incr ch} {
      $self UpdateParamFromView $param $ch
    }
  }

}

#------------------------------------------------------------------------------
#
# Preseneter for the MCFD16IndividualView
#
#  Servers as the synchronization logic of the MCFD16IndividualView and the
#  device. It considers the device (aka. -handle) the model that it intends to
#  manage. 
#
snit::type MCFD16IndividualPresenter {

  component m_base ;#  MCFD16Presenter instance

  delegate method * to m_base
  delegate option * to m_base

  ## Build the view and synchronize with module
  constructor {args} {
    install m_base using MCFD16Presenter %AUTO% 

    $self configurelist $args

    $self configure -view [MCFD16IndividualView [$self cget -widgetname] \
                                                  -presenter $self]

    $self UpdateViewFromModel
  }

  destructor {
    $m_base destroy
  }

  if {0} {
  method UpdateViewNames {} {
    # need to implement
  }
  }

  ## Method for committing state of the view to the device
  #
  # It write the state of the module and then reads back to make sure
  # that the view is properly representing the state of the device.
  #
  method Commit {} {
    $self UpdateModelFromView ;# write state
    $self UpdateViewFromModel ;# read back state
  }

  ## Set the state of the view given the model
  #
  method UpdateViewFromModel {} {

    # not sure if the names is something we need to record
    #    $self updateViewNames
    $self UpdateViewThresholds 
    update
    #    $self UpdateViewMonitor
    $self UpdateViewPolarities
    update
    $self UpdateViewGains
    update
    $self UpdateViewWidths
    update
    $self UpdateViewDeadtimes
    update
    $self UpdateViewDelays
    update
    $self UpdateViewFractions
    update
  }


  # Various helper methods
  method UpdateViewThresholds {} { $self LoopUpdateView Threshold 0 16 }
  method UpdateViewPolarities {} { $self LoopUpdateView Polarity 0 8 }
  method UpdateViewGains {} { $self LoopUpdateView Gain 0 8 }
  method UpdateViewWidths {} { $self LoopUpdateView Width 0 8 }
  method UpdateViewDeadtimes {} { $self LoopUpdateView Deadtime 0 8 }
  method UpdateViewDelays {} { $self LoopUpdateView Delay 0 8 }
  method UpdateViewFractions {} { $self LoopUpdateView Fraction 0 8 }

  if {0} {
  method UpdateViewMonitor {} {
    for {set ch 0} {$ch<16} {incr ch} {
      set mon [$options(-handle) MonitorEnabled $ch]
      $m_view SetMonitor $ch $thresh
    }
  }
  }

  ## Write the state of the view to the device
  #
  # This performs the synchronization for each parameter type at a time.
  # It does so by calling various helper methods. The helper methods only
  # iterate over non-common settings so that for threshold it writes values for
  # channels [0,15] and for all others it write values for channels [0,7].
  #
  method UpdateModelFromView {} {

    $self CommitViewThresholds 
    update
    $self CommitViewPolarities
    update
    $self CommitViewGains
    update
    $self CommitViewWidths
    update
    $self CommitViewDeadtimes
    update
    $self CommitViewDelays
    update
    $self CommitViewFractions
  }

  ## Commit data to module
  method CommitViewThresholds {} { $self LoopCommitView Threshold 0 16 }
  method CommitViewPolarities {} { $self LoopCommitView Polarity 0 8 }
  method CommitViewGains {} { $self LoopCommitView Gain 0 8 }
  method CommitViewWidths {} { $self LoopCommitView Width 0 8 }
  method CommitViewDeadtimes {} { $self LoopCommitView Deadtime 0 8 }
  method CommitViewDelays {} { $self LoopCommitView Delay 0 8 }
  method CommitViewFractions {} { $self LoopCommitView Fraction 0 8 }
}


# -----------------------------------------------------------------------------
#
# Presenter for the MCFD16CommonView
#
# This is essentially the same thing as the MCFD16IndividualPresenter except
# that it only sets the common parameters. It is expected that the device is
# already in a common mode for writing. If it isn't, it is not gauranteed that
# the write operations will succeed.
snit::type MCFD16CommonPresenter {

  component m_base ;# MCFD16Presenter instance

  delegate option * to m_base
  delegate method * to m_base

  ## Set up the view and synchronize view to the device
  #
  constructor {args} {
    install m_base using MCFD16Presenter %AUTO% 

    $self configurelist $args

    $self configure -view [MCFD16CommonView [$self cget -widgetname] -presenter $self]

    $self UpdateViewFromModel
  }

  destructor {
    $m_base destroy
  }

  method Commit {} {
    $self UpdateModelFromView
    update
    $self UpdateViewFromModel
  }

  ## Write the state of the parameters in the view to the device
  #
  method UpdateViewFromModel {} {

    # not sure if the names is something we need to record
    #    $self updateViewNames
    $self UpdateViewThresholds 
    update
    #    $self UpdateViewMonitor
    $self UpdateViewPolarities
    update
    $self UpdateViewGains
    update
    $self UpdateViewWidths
    update
    $self UpdateViewDeadtimes
    update
    $self UpdateViewDelays
    update
    $self UpdateViewFractions
    update
  }

  ## Write the state of the device to the view
  #
  # Uses a whole bunch of helper methods that perform the 
  # synchronization for only a single parameter at a time.
  #
  method UpdateModelFromView {} {
    # make sure the mode is set first
    # becuase subsequent writes may depend on it

    $self CommitViewThresholds 
    update
    $self CommitViewPolarities
    update
    $self CommitViewGains
    update
    $self CommitViewWidths
    update
    $self CommitViewDeadtimes
    update
    $self CommitViewDelays
    update
    $self CommitViewFractions
  }

  if {0} {
  method UpdateViewNames {} {
    # need to implement
  }
  }

  method UpdateViewThresholds {} { $self UpdateParamFromView Threshold 16 }

  if {0} {
  method UpdateViewMonitor {} {
    for {set ch 0} {$ch<16} {incr ch} {
      set mon [$options(-handle) MonitorEnabled $ch]
      $m_view SetMonitor $ch $thresh
    }
  }
  }

  # ---- Utility methods 

  # for synchronizing the view state to the device state
  method UpdateViewPolarities {} { $self UpdateParamFromView Polarity 8 }
  method UpdateViewGains {} { $self UpdateParamFromView Gain 8 }
  method UpdateViewWidths {} { $self UpdateParamFromView Width 8 }
  method UpdateViewDeadtimes {} { $self UpdateParamFromView Deadtime 8 }
  method UpdateViewDelays {} { $self UpdateParamFromView Delay 8 }
  method UpdateViewFractions {} { $self UpdateParamFromView Fraction 8 }


  ## Commit data to module
  method CommitViewThresholds {} { $self CommitViewToParam Threshold 16 }
  method CommitViewPolarities {} { $self CommitViewToParam Polarity 8 }
  method CommitViewGains {} { $self CommitViewToParam Gain 8 }
  method CommitViewWidths {} { $self CommitViewToParam Width 8 }
  method CommitViewDeadtimes {} { $self CommitViewToParam Deadtime 8 }
  method CommitViewDelays {} { $self CommitViewToParam Delay 8 }
  method CommitViewFractions {} { $self CommitViewToParam Fraction 8 }
}


########## Unified Controls ####################
#
# This is the core gui component of the application. It provides a
# megawidget that holds a MCFD16Common* and MCFD16Individual* set of controls.
# The views for these controls are in a FrameSwitcher widget that allows for
# simple switching back and forth between the visible frame. The main
# responsibility of this widget is to manage what is visible and also to
# coordinate events between the various controls it manages. There is also a
# PulserPresenter that forms a single row of controls at the bottom of the
# widget.
#
# The read may notice that the separation of view and presenter is not present
# for this widget. It could be made to follow the paradigm but I will leave that
# as a task for the future.
snit::widget MCFD16ControlPanel {

  component m_frames        ;# the frame switcher
  component m_comPrsntr     ;# instance of MCFD16CommonPresenter
  component m_indPrsntr     ;# instance of MCFD16IndividualPresenter
  component m_pulserPrsntr  ;# instance of PulserPresenter

  # 
  option -handle -default {} -configuremethod {SetHandle}
  variable m_mode  ;# current config mode of the device (selects visible frame)
  variable m_current

  ## Create all presenters and build the gui
  #
  # This reads from the device the configuration mode and then displays the
  # appropriate controls for the response it receives. 
  #
  # @throws error if no handle is provided.
  constructor {args} {
    $self configurelist $args

    if {[$self cget -handle] eq ""} {
      return -code error "MCFD16ControlPanel must be provided -handle option."
    }

    install m_frames using FrameSwitcher $win.frames
    install m_comPrsntr using \
      MCFD16CommonPresenter %AUTO% -widgetname $win.com \
                                   -handle [$self cget -handle]

    install m_indPrsntr using \
      MCFD16IndividualPresenter %AUTO% -widgetname $win.ind \
                                   -handle [$self cget -handle]

    install m_pulserPrsntr using \
      PulserPresenter %AUTO% [PulserView $win.plsr] \
                                   [$self cget -handle]

    set m_mode [[$self cget -handle] GetMode]
    trace add variable [myvar m_mode] write [mymethod OnModeChange]

    $self BuildGUI

    # sets the m_current and updates the frame switcher
    $self OnModeChange {} {} {}

  }

  ## Create the widget and then install them into the megawidget.
  #
  method BuildGUI {} {
    # add the frames
    $m_frames add common $win.com
    $m_frames add individual $win.ind

    ## construct the frame containing mode elements
    set mode $win.mode
    ttk::frame $mode -style Mode.TFrame
    ttk::label $mode.modeLbl -text "Configuration Mode" -style Mode.TLabel
    ttk::radiobutton $mode.modeCom -text "common" -variable [myvar m_mode] \
                                  -value common -style Mode.TRadiobutton
    ttk::radiobutton $mode.modeInd -text "individual" -variable [myvar m_mode] \
                                  -value individual -style Mode.TRadiobutton
    grid $mode.modeLbl $mode.modeCom $mode.modeInd -sticky new
    grid columnconfigure $mode {0 1 2} -weight 1

    ttk::button $win.commit -text "Commit to Device" -command [mymethod Commit] \
                                  -style "Commit.TButton"
    ttk::button $win.update -text "Update from Device" -command [mymethod Update] \
                                  -style "Commit.TButton"

    grid $win.mode - -sticky new
    grid $m_frames - -sticky nsew

    grid $win.commit  $win.update  -sticky ew -pady 4
    grid $win.plsr - -sticky sew -pady 4

    grid rowconfigure $win 1 -weight 1
    grid columnconfigure $win {0 1 2} -weight 1
  }


  ## Display the appropriate controls for the current mode
  #
  # This gets called every time the m_mode variable is written to
  # and invokes select in the FrameSwitcher to keep the displayed state correct.
  # 
  # @param name0  name of variable
  # @param name1  subname of variable
  # @param op     operation
  #
  method OnModeChange {name0 name1 op} {
    if {$m_mode eq "common"} {
      $m_frames select common
      set m_current $m_comPrsntr
    } else {
      $m_frames select individual 
      set m_current $m_indPrsntr
    } 
  }


  ## Command triggered when "commit to device" button is pressed
  #
  # Calls commit for the current presenter object. This is a more efficient
  # way of operating than writing every parameter to the device every time.
  # Rather, this will only set the common values if that is the desired state.
  #
  method Commit {} {
    [$self cget -handle] SetMode $m_mode ;# set mode first to make sure that
                                         ;# subsequent writes succeed.
    $m_current Commit
  }

  ## Call each of the presenters' UpdateViewFromModel methods
  #
  # Basically, this resynchronizes the state of the view to module.
  # It is called by the "Update from Device"
  method Update {} {
    # first read from the device the mode 
    set m_mode [[$self cget -handle] GetMode]

    # for each of the modes, read in the modules values
    $m_comPrsntr UpdateViewFromModel 
    $m_indPrsntr UpdateViewFromModel 
    $m_pulserPrsntr UpdateViewFromModel
  }

  ## Retrieve the current controls presenter
  #
  method GetCurrent {} { return $m_current }

  method SetHandle {opt val} {
    # pass the handle around to all of the sub-presenters
    if {$m_comPrsntr ne ""} {
      $m_comPrsntr configure -handle $val
    }

    if {$m_indPrsntr ne ""} {
       $m_indPrsntr configure -handle $val
    }
    if {$m_pulserPrsntr ne ""} {
      $m_pulserPrsntr SetHandle $val
    }

    # set the option
    set options($opt) $val
  }
}

#######
#

snit::widget SaveToFileForm {

  variable _theApp
  variable _path
  variable _displayPath

  ##
  #
  constructor {app args} {
    set _theApp $app
    # this sets up the path and the display path
    $self setPath ""

    $self BuildGUI
  }

  ##
  #
  method BuildGUI {} {
    ttk::label  $win.pathLbl -text "Output file name"
    ttk::entry  $win.pathEntry -textvariable [myvar _path] -width 24 
    ttk::button $win.browse -text "Browse"  -command [mymethod Browse] -width 8
    ttk::button $win.save -text "Save"  -command [mymethod Save] -width 8

    grid  $win.pathLbl $win.pathEntry $win.browse -sticky ew -padx 4 -pady 4
    grid  $win.save - - -sticky ew -padx 4 -pady 4
    grid columnconfigure $win 1 -weight 1
  }

  ##
  #
  method Browse {} {
    set path [tk_getSaveFile -confirmoverwrite 1 -defaultextension ".tcl" \
                    -title {Save as} ] 
    if {$path ne ""} {
      $self setPath $path  
    }
  }


  ##
  #
  method Save {} {
    if {$_path eq ""} {
      tk_messageBox -icon error -message "User must specify file name."
      return
    }

    if {[catch {open $_path w} logFile]} {
      tk_messageBox -icon error -message "Failed to open $_path for writing."
      return
    }

    set options [$_theApp GetOptions]
    
    # write the header portion that instantiates a device driver
    ScriptHeaderGenerator gen $options
    set lines [gen generateHeader]
    foreach line $lines {
      chan puts $logFile $line
    }

    # generate the lines in the file make device driver calls
    MCFD16Factory factory $options
    set logger [factory create cmdlogger $logFile]
    set realHandle [$_theApp GetHandle]
    set control [$_theApp GetControlPresenter]

    # replace real driver with logging driver
    $control configure -handle $logger

    # commit (this causes all of the gui state to be written to file)
    $control Commit
    $logger Flush ;# flush buffers to make sure it all gets into the file

    # replace the logging driver with the real driver
    $control configure -handle $realHandle
    $control Update

    # cleanup
    $logger destroy
    factory destroy
  }

  method setPath path {
    set _path $path
    set _displayPath [file tail $path]
  }
}


snit::widget LoadFromFileForm {

  option -presenter -default {}

  variable _path

  constructor {args} {
    $self configurelist $args

    $self BuildGUI
  }

  method BuildGUI {} {
    ttk::label  $win.pathLbl -text "Input file name"
    ttk::entry  $win.pathEntry -textvariable [myvar _path] -width 24 
    ttk::button $win.browse -text "Browse"  -command [mymethod Browse] -width 8
    ttk::button $win.load -text "Load"  -command [mymethod Load] -width 8

    grid $win.pathLbl $win.pathEntry $win.browse -sticky ew -padx 4 -pady 4
    grid $win.load - - -sticky ew -padx 4 -pady 4
    grid columnconfigure $win 1 -weight 1
  }

  method Browse {} {
    if {[$self cget -presenter] eq {} } {
      return
    } else {
      [$self cget -presenter] Browse 
    }
  }

  method Load {} {
    if {[$self cget -presenter] eq {} } {
      return
    } else {
      [$self cget -presenter] Load $_path
    }
  }

  method SetPath {path} {
    set _path $path
  }
}

snit::type LoadFromFilePresenter {

  option -view -default {} -configuremethod SetView
  variable _contentFr

  constructor {contentFr args} {
    set _contentFr $contentFr 

    $self configurelist $args
  }
  
  method SetView {opt val} {
    $val configure -presenter $self
    set options($opt) $val
  }

  method Browse {} {
    set path [tk_getOpenFile -defaultextension ".tcl" \
                    -title {Choose file to load} ] 
                  
    if {($path ne "") && ([$self cget -view] ne {})} {
      [$self cget -view] SetPath $path  
    }
    
  }

  method Load {path} {
    if {![file exists $path]} {
      set msg "Cannot load from $path, because file does not exist."
      tk_messageBox -icon error -message $msg
    } elseif {! [file readable $path]} { 
      set msg "Cannot load from $path, because file is not readable."
      tk_messageBox -icon error -message $msg
    }

    set rawLines [$self TokenizeFile $path]

    # find the lines we can safely execute
    set executableLines [$self FilterOutNonAPICalls $rawLines]
    puts "Executable Lines : "
    puts "------------------"
    foreach line $executableLines {
      puts $line
    }
    puts "DoNE"

    set devName [$self ExtractDeviceName [lindex $executableLines 0]]
    set fakeHandle [MCFD16Memorizer $devName]

    # load state into device
    $self EvaluateAPILines $executableLines

    # update the actual content
    set realHandle [$self SwapInHandle $fakeHandle]
    $_contentFr Update
    set fakeHandle [$self SwapInHandle $realHandle]

    $fakeHandle destroy
    catch {close $loadFile}
  }

  method TokenizeFile {path} {
    # if here, the file exists and can be updated
    set loadFile [open $path r]

    set blocks [list]
    BlockCompleter bc -left "{" -right "}"
 
    while {![chan eof $loadFile]} {
      bc appendLine [chan gets $loadFile]
      while {![bc isComplete] && ![chan eof $loadFile]} {
        bc appendLine "\n[chan gets $loadFile]"
      }
      lappend blocks [bc getText]
      bc Reset
    }

    bc destroy
    return $blocks
  }

  # swap in handle
  method SwapInHandle {newHandle} {
    set oldHandle [$_contentFr cget -handle]
    $_contentFr configure -handle $newHandle

    return $oldHandle
  }

  # check to see if second element of
  method FilterOutNonAPICalls lines {
    set validLines [list]
    foreach line $lines {
      if {[$self IsValidAPICall $line]} {
        lappend validLines $line
      }
    }
    return $validLines
  }

  method ExtractDeviceName {line} {
    set tokens [split $line " "]
    set name [lindex $tokens 0]
    if {[string first "::" $name] != 0} {
      set name "::$name"
    }
    return $name
  }

  method IsValidAPICall {line} {
    # we are only treating simple lines where the second element is the verb
    # this is valid for all 
    set verb [lindex $line 1]
    return [expr {$verb in $_validAPICalls}]
  }

  method EvaluateAPILines {lines} {
    foreach line $lines {
      puts $line
      uplevel #0 eval $line
    }
  }

  typevariable _validAPICalls
  typeconstructor {
    set _validAPICalls [list]
    lappend _validAPICalls "SetThreshold"
    lappend _validAPICalls "GetThreshold"
    lappend _validAPICalls "SetGain"
    lappend _validAPICalls "GetGain"
    lappend _validAPICalls "SetWidth"
    lappend _validAPICalls "GetWidth"
    lappend _validAPICalls "SetDeadtime"
    lappend _validAPICalls "GetDeadtime"
    lappend _validAPICalls "SetDelay"
    lappend _validAPICalls "GetDelay"
    lappend _validAPICalls "SetFraction"
    lappend _validAPICalls "GetFraction"
    lappend _validAPICalls "SetPolarity"
    lappend _validAPICalls "GetPolarity"
    lappend _validAPICalls "SetMode"
    lappend _validAPICalls "GetMode"
    lappend _validAPICalls "EnableRC"
    lappend _validAPICalls "RCEnabled"
    lappend _validAPICalls "SetChannelMask"
    lappend _validAPICalls "GetChannelMask"
    lappend _validAPICalls "EnablePulser"
    lappend _validAPICalls "DisablePulser"
    lappend _validAPICalls "PulserEnabled"
  }
}


#########
#
snit::widget PulserView {
  hulltype ttk::frame
  option -pulserid   -default 1
  option -enabled    -default 0
  option -buttontext -default {Enable} 
  option -radiobuttonstate -default 0 -configuremethod RadiobuttonStateChange

  option -presenter -default {}

  delegate option * to hull

  constructor {args} {
    $self configurelist $args

    $self BuildGUI
  }

  method BuildGUI {} {
    $self configure -style Pulser.TFrame

    ttk::label $win.lbl -text "Test Pulser" -style "Pulser.TLabel"

    ttk::radiobutton $win.mHzPulser -text "2.5 MHz" \
                                    -variable [myvar options(-pulserid)] \
                                    -value 1 -style "Pulser.TRadiobutton"
    ttk::radiobutton $win.kHzPulser -text "1.22 kHz" \
                                    -variable [myvar options(-pulserid)] \
                                    -value 2 -style "Pulser.TRadiobutton"

    ttk::button $win.onoff -textvariable [myvar options(-buttontext)]\
                           -command [mymethod OnPress] \
                                          -style "Pulser.TButton"

    grid $win.lbl $win.kHzPulser $win.mHzPulser $win.onoff -sticky sew \
                                                -padx 4 -pady 4
    grid columnconfigure $win {0 1 2 3} -weight 1
  }

  method OnPress {} {
    set presenter [$self cget -presenter] 
    if {$presenter ne {}} {
      $presenter OnPress
    }
    # it is not an error if there is no presenter... it is instead a noop
  }

  method RadiobuttonStateChange {option val} {
    if {$val == 0} {
      $win.mHzPulser state disabled
      $win.kHzPulser state disabled
    } else {
      $win.mHzPulser state !disabled
      $win.kHzPulser state !disabled
    }
    set options($option) $val
  }
}


snit::type PulserPresenter {

  variable _handle
  variable _view
  
  constructor {view handle} {
    set _view $view
    set _handle $handle

    # tell the view that this is its presenter
    $_view configure -presenter $self

    # we don't know which pulser state exists, so we will disable by default
    $_handle DisablePulser 

    $self UpdateViewFromModel

  }

  method OnPress {} {
    $self CommitViewToModel
  }

  method UpdateViewFromModel {} {
    # get state of the device
    set state [$_handle PulserEnabled]
    if {$state == 0} {
      set state inactive
    } else {
      set state active
    }

    # update the view given the state of device
    $self UpdateButtonText $state
    $self UpdateRadiobuttonState $state
  }

  method CommitViewToModel {} {
    # get state of the view (determined by button text)
    set trans [$self GetTransitionType]

    # commit the state to the device
    $self TransitionPulser $trans

    # update the view state to reflect the next state
    $self UpdateViewFromModel
  }

  method GetViewEnabled {} {
    set text [$_view cget -buttontext]
    if {$text eq "Enable"} {
      return 0 ; # the pulser is disabled
    } else {
      return 1 ; # pulser is enabled
    }
  }

  method TransitionPulser {transType} {
    if {$transType == "enable"} {
      # user wants to enable...
      $_handle EnablePulser [$_view cget -pulserid]
    } else {
      $_handle DisablePulser
    }
    
  }

  method GetTransitionType {} {
    set buttontext [$_view cget -buttontext]
    if {$buttontext eq "Enable"} {
      return enable
    } else {
      return disable
    }
  }
  method UpdateButtonText {state} {
    if {$state eq "active"} {
      # user has set it into enabled
      $_view configure -buttontext "Disable"
    } else {
      $_view configure -buttontext "Enable"
    }
  }

  method UpdateRadiobuttonState {state} {
    if {$state eq "active"} {
      $_view configure -radiobuttonstate 0
    } else {
      $_view configure -radiobuttonstate 1
    }
  }

  method SetHandle {handle} {
    set _handle $handle
  }

  method GetHandle {} {
    return $_handle
  }
}


