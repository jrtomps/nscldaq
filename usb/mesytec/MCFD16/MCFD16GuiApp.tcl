package provide mcfd16guiapp 1.0

package require Tk
package require snit
package require mcfd16factory
package require mcfd16gui
package require FrameSequencer


snit::type MCFD16AppOptions {
  option -protocol
  option -serialfile
  option -module
  option -host
  option -port 
  option -devno

  constructor {args} {
    $self configurelist $args 
  }
}


snit::type MCFD16GuiApp {
  option -optionarray -default ::params
  option -widgetname -default .app

  component _handle
  component _options 
  
  delegate option * to _options

  variable _controlPrsntr 
  variable _configFr
  variable _saveFr
  variable _loadFr
  variable _sequencer

  constructor {args} {
    install _options using MCFD16AppOptions %AUTO% 

    $self configurelist $args

    $self processCmdlineOptions

    $self setUpMenu
    $self BuildGUI

  }

  destructor {
    catch {$_handle destroy}
  }

  method processCmdlineOptions {} {

    # Create the driver that serves as the backend of the gui
    MCFD16Factory factory $_options

    set res [catch {
      set paramDict [array get [$self cget -optionarray]]
      set protocol [dict get $paramDict -protocol]
      if {$protocol eq "usb"} {
        set serialFile [dict get $paramDict -serialfile]
        if {![file exists $serialFile]} {
          puts "Serial file \"$serialFile\" provided but does not exist."
          exit
        }

        install _handle using factory create usb
        flush stdout
      } else {
      # at this point the only other option is mxdcrcbus because 
      # assertProtocolDependencies would have exited otherwise.
        install _handle using factory create mxdcrcbus
      }
    } msg]

    if {$res == 1} {
      puts "Failed to create an MCFD16 device driver with error : $msg"
      exit
    }
  }

  method setUpMenu {} {
    option add *tearOff 0

    # get the menu for the toplevel
    set menu [[winfo toplevel [$self cget -widgetname]] cget -menu]
    if {$menu eq ""} {
      set m .menu
      menu $m
      menu $m.file 
      $m.file add command -command [mymethod ToSaveAs] -label "Save as..."
      $m.file add command -command [mymethod ToLoad] -label "Load settings..."
      $m add cascade -menu $m.file -label "File"
      . configure -menu $m
    }
  }

  method BuildGUI {} {
    set win [$self cget -widgetname]

    ttk::label $win.title -text "MCFD-16 Controls" -style "Title.TLabel"

    set _sequencer $win.frames
    FrameSequencer $_sequencer

    
    set _configFr [$self BuildControlFrame $_sequencer]
    set _saveFr [$self BuildSaveAsFrame $_sequencer]
    set _loadFr [$self BuildLoadFrame $_sequencer]

    $_sequencer staticAdd config $_configFr {}
    $_sequencer staticAdd save $_saveFr config
    $_sequencer staticAdd load $_loadFr config
    $_sequencer select config

    grid $_sequencer -sticky nsew -padx 8 -pady 8

  }

  method BuildControlFrame {top} {
    set configFr $top.config
    ttk::frame $configFr
    set title [ttk::label $configFr.title -text "MCFD-16 Controls" \
                                          -style "Title.TLabel"]
    set infoFrame [$self constructInfoFrame $configFr]
    set _controlPrsntr $configFr.ctl
    set control [MCFD16ControlPanel $_controlPrsntr -handle $_handle]

    grid $title -sticky nsew -padx 8 -pady 8
    grid $infoFrame -sticky nsew -padx 8 -pady 8
    grid $control -sticky nsew -padx 8 -pady 8
    grid rowconfigure $configFr {2} -weight 1
    grid columnconfigure $configFr 0 -weight 1

    return $configFr
  }

  method BuildSaveAsFrame {top} {
    set saveFr $top.save
    SaveToFileForm $top.save $self

    return $saveFr
  }

  method BuildLoadFrame {top} {
    set loadFr $top.load
    LoadFromFileForm $top.load
    set load [LoadFromFilePresenter %AUTO% $_controlPrsntr -view $top.load]

    return $loadFr
  }

  method constructInfoFrame {top} {
    set paramDict [array get [$self cget -optionarray]]
    set protoLbl ""
    ttk::frame $top.info -style "Info.TFrame"
    if {[dict get $paramDict -protocol] eq "usb"} {
      set protoLbl "Protocol : USB"
      set serialFile "Serial file : [dict get $paramDict -serialfile]"

      ttk::label $top.info.protoLbl -text $protoLbl
      ttk::label $top.info.serialFile -text $serialFile

      grid $top.info.protoLbl $top.info.serialFile -sticky nsew
      grid columnconfigure $top.info {0 1} -weight 1
    } else {
      set protoLbl "Protocol : MxDC-RCbus"
      set module "Module name : [dict get $paramDict -module]"
      set host "Server host : [dict get $paramDict -host]"
      set port "Server port : [dict get $paramDict -port]"
      set devno "Device address : [dict get $paramDict -devno]"

      ttk::label $top.info.protoLbl -text $protoLbl
      ttk::label $top.info.module -text $module
      ttk::label $top.info.host -text $host
      ttk::label $top.info.port -text $port
      ttk::label $top.info.devno -text $devno

      grid $top.info.protoLbl $top.info.host -sticky nsew
      grid $top.info.module $top.info.port -sticky nsew
      grid $top.info.devno x -sticky nsew
      grid columnconfigure $top.info {0 1} -weight 1
    }


    return $top.info
  }

  method ToSaveAs {} {
    $_sequencer select save
  }

  method ToLoad {} {
    $_sequencer select load
  }

  method GetHandle {} {
    return $_handle
  }

  method GetControlPresenter {} {
    return $_controlPrsntr
  }

  method GetOptions {} {
    return $_options
  }
}

