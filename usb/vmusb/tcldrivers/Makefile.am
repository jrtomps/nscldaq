DRIVERDest=@prefix@/TclLibs/vmusbdrivers

TCLDRIVERS=sis330xdriver.tcl VMUSBDriverSupport.tcl vmusb.tcl \
           AXLM72.tcl APpacXLM72.tcl ACrdcXLM72.tcl AXLM72Scaler.tcl \
            CBD8210DriverSupport.tcl

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) $(DRIVERDest)
	$(INSTALL_SCRIPT) @srcdir@/*driver.tcl $(DRIVERDest)
	$(INSTALL_SCRIPT) @srcdir@/VMUSBDriverSupport.tcl $(DRIVERDest)
	$(INSTALL_SCRIPT) @srcdir@/vmusb.tcl $(DRIVERDest)
	for f in $(TCLDRIVERS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(DRIVERDest) ; done
	echo "pkg_mkIndex -verbose $(DRIVERDest) *.tcl" | $(TCLSH_CMD)

#
#  All driver and docs files need to be added here:
#
TCLTESTS = xlm72scaler.test vmusbdriversupport.test


check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(TCLDRIVERS) $(TCLTESTS)
