ACLOCAL_AMFLAGS=-I m4
AUTOMAKE_OPTIONS=subdir-objects
if BUILD_SBS_DRIVER
  SBSDIRS=sbs
endif

if BUILD_EPICS_TOOLS
if EPICS
     EPICSDIRS=epics
endif
endif

if BUILD_USB
   USBDIR=usb
endif

if BUILD_DOCS
   DOCDIR=docbuild docconfig simplesetups
endif

SUBDIRS =	 base caenfw-common servers base/dataflow  \
		daq utilities $(DOCDIR) \
		$(SBSDIRS) $(EPICSDIRS) $(USBDIR)

DIST_SUBDIRS = base caenfw-common servers base/dataflow daq utilities \
							 docbuild docconfig simplesetups sbs epics usb

INTROFIGURES = @srcdir@/figures/intro-data-flow-diagram.png \
								@srcdir@/figures/intro-data-flow-diagram-diagnostics.png 
install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/include
	$(mkinstalldirs) @prefix@/pythonLibs @prefix@/pythonLibs/nscldaq
	$(mkinstalldirs) @prefix@/share/html/figures
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl
	$(INSTALL_SCRIPT) config.h @prefix@/include
	touch @prefix@/pythonLibs/nscldaq/__init__.py
	echo "Version: "NSCLDAQ-$(VERSION) built on `hostname` `date` > @prefix@/VERSION
	$(INSTALL_SCRIPT) daqsetup.bash @prefix@
	$(INSTALL_DATA) @srcdir@/figures/*.png @prefix@/share/html/figures


EXTRA_DIST=dotests introduction.xml libtcl $(INTROFIGURES) 

check-TESTS:
	@top_srcdir@/dotests $(SUBDIRS)
