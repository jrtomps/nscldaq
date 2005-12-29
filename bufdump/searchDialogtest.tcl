source bufdumpDialogs.tcl

searchDialog .t -command {puts ok} -cancelcommand {puts cancel}
wm withdraw .t
.t modal

puts done