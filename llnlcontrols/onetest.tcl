source client.tcl
source gdgwidget.tcl
source gdgcontrol.tcl

set con [controlClient %AUTO%]
set w   [gdgwidget .g -title {GDG 1 device}]

set full [gdgcontrol %AUTO% -widget $w -connection $con -name gdg1]
$full UpdateValues

pack $w
