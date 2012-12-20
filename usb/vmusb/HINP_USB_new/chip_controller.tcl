#!/usr/local/bin/wish

wm title . "Silicon Chip Controller"

#Create frames



# Creating the main frames
#pack .hira1.geometry .hira1.electronics -side left -fill y
#pack .hira1.geometry.title .hira1.electronics.title

# buttons

source notebook.tcl
source tabnbook.tcl
source mclistbox.tcl
source main_menu.tcl
source silstrip.tcl

tabnotebook_create .main
pack .main -expand 1 -fill both

tabnotebook_page .main Strip_cont

tabnotebook_display .main Strip_cont
