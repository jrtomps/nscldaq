SUBDIRS=cv6533 figures

PACKAGES=client.tcl configfile.tcl gdgcontrol.tcl gdgwidget.tcl sclclient.tcl xlm72scalerpanel.tcl \
				 MDGG16Gui.tcl MDGG16Proxy.tcl MDGG16GuiApp.tcl AGD16XLM72Panel.tcl runStateObserver.tcl

APPS=gdgpanel.tcl v812usbcontrol.tcl MDGG16Control.tcl

TCL_TESTS = tclTests.tcl mdgg16gui.test mdgg16guiapp.test

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/vmusbcontrol
	for f in $(PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/vmusbcontrol; done
	echo pkg_mkIndex -verbose @prefix@/TclLibs/vmusbcontrol "*.tcl" | tclsh
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@; done
	$(INSTALL_SCRIPT) @srcdir@/v812usbcontrol.tcl @bindir@/v895usbcontrol.tcl	
	$(INSTALL_SCRIPT) @srcdir@/XLM72ScalerGUI.tcl @bindir@/XLM72ScalerGUI	
	$(INSTALL_SCRIPT) @srcdir@/MDGG16Control.tcl @bindir@/MDGG16Control	
	$(INSTALL_SCRIPT) @srcdir@/XLM72GateDelayControl.tcl @bindir@/XLM72GateDelayControl
	$(mkinstalldirs) @prefix@/share/html/figures
	$(INSTALL_DATA) @srcdir@/gd16_startup.png @prefix@/share/html/figures/.


check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST= $(APPS) $(PACKAGES) XLM72ScalerGUI.tcl $(TCL_TESTS) mdgg16control.xml \
						XLM72GateDelayControl.tcl gd16_startup.png xlm72gatedelaycontrol.xml
