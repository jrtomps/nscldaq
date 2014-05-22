ACLOCAL_AMFLAGS=-I m4

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


SUBDIRS =	 base caenfw-common servers base/dataflow  \
		daq utilities docbuild docconfig $(SBSDIRS) $(EPICSDIRS) $(USBDIR)


install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/include
	$(mkinstalldirs) @prefix@/pythonLibs @prefix@/pythonLibs/nscldaq
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl
	$(INSTALL_SCRIPT) config.h @prefix@/include
	$(MANDB) @prefix@/share/man
	touch @prefix@/pythonLibs/nscldaq/__init__.py
	echo "Version: "NSCLDAQ-$(VERSION) built on `hostname` `date` > @prefix@/VERSION
	$(INSTALL_SCRIPT) daqsetup.bash @prefix@


EXTRA_DIST=dotests introduction.xml libtcl

TESTS=./dotests
