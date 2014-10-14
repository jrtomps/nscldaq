DRIVERDest=@prefix@/TclLibs/ccusbdrivers

TCLDRIVERS=ccusb.tcl CCUSBDriverSupport.tcl \
					 ACAENC671.tcl ACAENC808.tcl \
					 ALeCroy4448.tcl ATrigger2367.tcl \
					 APhillips71xx.tcl ALeCroy4300B.tcl
					 
           

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

#
#  All driver and docs files need to be added here:
#
#CCUSBTCLTESTS = tclCCUSB.test ccusbTests.tcl


check-TESTS:
	TCLLIBPATH="@prefix@/TclLibs @libdir@ @srcdir@" HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS)  tclTests.tcl
#$(CCUSBTCLTESTS) 

