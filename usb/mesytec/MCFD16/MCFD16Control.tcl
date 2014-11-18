
set here [file dirname [file normalize [info script]]]
lappend auto_path $here

package require mcfd16usb
package require mcfd16gui

################## GLOBAL STUFF #############################################
#

proc ConfigureStyle {} {
  ttk::style configure "Title.TLabel" -foreground "firebrick" \
                                      -font "helvetica 28 bold"

  ttk::style configure Header.TLabel -background {orange red} 
  ttk::style configure Header.TFrame -background {orange red}

  ttk::style configure Even.TEntry -background snow3
  ttk::style configure Even.TRadiobutton -background snow3
  ttk::style configure Even.TSpinbox -background snow3
  ttk::style configure Even.TFrame -background snow3
  ttk::style configure Even.TLabel -background snow3

  ttk::style configure Odd.TEntry -background snow3
  ttk::style configure Odd.TRadiobutton -background snow3
  ttk::style configure Odd.TSpinbox -background snow3
  ttk::style configure Odd.TFrame -background snow3
  ttk::style configure Odd.TLabel -background snow3

  ttk::style configure Commit.TButton -background orange
}

ConfigureStyle

ttk::label .title -text "MCFD-16 Controls" -style "Title.TLabel"
ttk::frame .info -style "Info.TFrame"
ttk::label .info.fwVsnLbl -text "Firmware version:"
ttk::label .info.swVsnLbl -text "Software version:"
ttk::label .info.protoLbl -text "Protocol:"

grid .info.fwVsnLbl .info.protoLbl -sticky nsew
grid .info.swVsnLbl x             -sticky nsew
grid columnconfigure .info {0 1} -weight 1
#grid rowconfigure .info {2} -weight 1


MCFD16USB dev2 /dev/ttyUSB0

set control [MCFD16ControlPanel .ctl -handle ::dev2]
PulserPresenter pulserCtlr [PulserView .pulser] dev2

grid .title -sticky nsew -padx 8 -pady 8
grid .info -sticky nsew -padx 8 -pady 8
grid .ctl -sticky nsew -padx 8 -pady 8
grid .pulser -sticky nsew -padx 8 -pady 8

grid rowconfigure . {2} -weight 1
grid columnconfigure . 0 -weight 1

