DRIVERDest=@prefix@/TclLibs/ccusbdrivers

TCLDRIVERS=ccusb.tcl CCUSBDriverSupport.tcl
           

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

#
#  All driver and docs files need to be added here:
#
#CCUSBTCLTESTS = tclCCUSB.test ccusbTests.tcl


#ccusbtest:
#	TCLLIBPATH="@prefix@/TclLibs @libdir@" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/ccusbTests.tcl

EXTRA_DIST=$(TCLDRIVERS) 
#$(CCUSBTCLTESTS)

