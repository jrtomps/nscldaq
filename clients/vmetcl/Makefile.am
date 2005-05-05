SUBDIRS = camactcl vhq202m


lib_LTLIBRARIES = libVme.la
libVme_la_SOURCES = VMECommand.cpp VMEMapCommand.cpp VMEPackage.cpp

libVme_la_LDFLAGS =   -L@top_srcdir@/VMEApi \
	 -L@top_srcdir@/TCL -L@top_srcdir@/Exception  \
	-L@top_srcdir@/CopyrightTools\
	-L@top_srcdir@/DeviceSupport -lDeviceSupport \
	-version-info $(SOVERSION):0 -Wl,"-rpath=$(libdir)"

libVme_la_LIBADD =  -lTclPlus -lException  \
	$(TCL_LDFLAGS)   $(X11LIBS) -lVmeAPI \
	$(VMEBUILDLIBS) -lLicense -lpthread \
	-lm -lstdc++ -lgcc -lc 


INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -I@top_srcdir@/TCL \
	-I@top_srcdir@/CopyrightTools \
	-I@top_srcdir@/DeviceSupport \
	$(TCL_FLAGS) -DHOME=\"$(prefix)\"

install-exec-local:
	$(mkinstalldirs) $(prefix)/Scripts
	$(mkinstalldirs) $(prefix)/TclLibs
	$(INSTALL_DATA) caennet.tcl $(prefix)/Scripts
	export LD_LIBRARY_PATH=$(prefix)/lib
	export TCLLIBPATH="$(prefix)/lib $(prefix)/Scripts"
	rm -f $(prefix)/lib/{pkgIndex.tcl,tclIndex}
	$(LN_S) $(prefix)/TclLibs/pkgIndex.tcl $(prefix)/lib/pkgIndex.tcl
	$(LN_S) $(prefix)/TclLibs/tclIndex     $(prefix)/lib/tclIndex



EXTRA_DIST = mkpackage.tcl caennet.tcl Readme.txt \
		vme.3tcl caennet.3tcl  

noinst_HEADERS = TestHarness.h \
	VMECommand.h \
	VMEMapCommand.h \
	vmcaenet.h

install-data-local:
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/Docs/vmetcl
	$(INSTALL_DATA) Readme.txt $(prefix)/Docs/vmetcl/VmeTcl.txt

man_MANS =  vme.3tcl caennet.3tcl 