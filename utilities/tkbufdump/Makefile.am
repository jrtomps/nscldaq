install-exec-local:
	$(mkinstalldirs) @prefix@/tkbufdump
	$(INSTALL_SCRIPT) *.tcl @prefix@/tkbufdump
	$(INSTALL_DATA)   *.html @prefix@/tkbufdump
	$(INSTALL_DATA)   packets.def @prefix@/tkbufdump
	$(mkinstalldirs) @prefix@/bin
	echo "#!/bin/bash"  > @prefix@/bin/tkdumper
	echo "@prefix@/tkbufdump/tkbufdump.tcl" >> @prefix@/bin/tkdumper
	chmod a+x @prefix@/bin/tkdumper

EXTRA_DIST = acceptall.tcl	\
controlformatter.tcl		\
datasource.tcl			\
extend.html			\
filter.html			\
intro.html			\
itemdefs.tcl			\
itemtypefilter.tcl		\
menus.html			\
next.html			\
open.html			\
packet.html			\
physicscountformatter.tcl	\
physicsformatter.tcl		\
plugin.html			\
sampleformatter.tcl		\
scalerformatter.tcl		\
stringlistformatter.tcl		\
tkbufdump.tcl			\
typefilterdialog.tcl		\
unknownformatter.tcl		\
packets.def

