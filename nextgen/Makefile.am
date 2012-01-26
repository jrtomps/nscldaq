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


SUBDIRS =	base/CopyrightTools base/exception base/tclplus servers base  \
		daq utilities docbuild docconfig $(SBSDIRS) $(EPICSDIRS) $(USBDIR)


install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/include
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl
	$(INSTALL_SCRIPT) config.h @prefix@/include
	$(MANDB) $(prefix)/share/man
	echo "Version: " $(VERSION) built on `hostname` `date` > $(prefix)/VERSION


EXTRA_DIST=dotests

TESTS=./dotests
