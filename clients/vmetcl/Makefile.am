SUBDIRS = camactcl vhq202m
PKGDIR  = $(prefix)/TclLibs/caennet
LIBDIR  = $(prefix)/TclLibs

noinst_PROGRAMS=TestHarness
TestHarness_SOURCES= TestHarness.cpp
TestHarness_DEPENDENCIES=VMECommand.o VMEMapCommand.o

TestHarness_LDFLAGS = VMECommand.o VMEMapCommand.o \
	 @top_srcdir@/VMEApi/libVmeAPI.la $(VMEBUILDLIBS) \
	 -L@top_srcdir@/TCL -L@top_srcdir@/Exception  \
	-L@top_srcdir@/CopyrightTools\
	-L@top_srcdir@/DeviceSupport -lDeviceSupport -ltclPlus -lException \
	$(TCL_LDFLAGS)

lib_LTLIBRARIES = libVme.la
libVme_la_SOURCES = VMECommand.cpp VMEMapCommand.cpp VMEPackage.cpp

libVme_la_LDFLAGS =  	-version-info $(SOVERSION):0 -Wl,"-rpath=$(libdir)"

libVme_la_LIBADD = -L@top_srcdir@/TCL @top_srcdir@/TCL/libtclPlus.la    \
		@top_srcdir@/Exception/libException.la \
		@top_srcdir@/VMEApi/libVmeAPI.la       \
		@top_srcdir@/DeviceSupport/libDeviceSupport.la \
		@top_srcdir@/CopyrightTools/libLicense.la     \
		$(TCL_LDFLAGS) $(X11LIBX) $(VMEBUILDLIBS) -lpthread \
		-lm -lstdc++ -lgcc -lc


INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -I@top_srcdir@/TCL \
	-I@top_srcdir@/CopyrightTools \
	-I@top_srcdir@/DeviceSupport \
	$(TCL_FLAGS) -DHOME=\"$(prefix)\"

install-exec-local:
	echo exec-local -----------------------
	$(mkinstalldirs) $(PKGDIR)
	$(mkinstalldirs) $(LIBIDR)
	$(INSTALL_SCRIPT) caennet.tcl $(PKGDIR)
	echo "pkg_mkIndex -verbose  $(PKGDIR) *.tcl" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl
	echo ----------------------------------

EXTRA_DIST = mkpackage.tcl caennet.tcl Readme.txt \
		vme.3tcl caennet.3tcl  

noinst_HEADERS = TestHarness.h \
	VMECommand.h \
	VMEMapCommand.h \
	vmcaenet.h

install-data-local:
	echo data-local ------------------------------
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/Docs/vmetcl
	$(mkinstalldirs) $(prefix)/etc
	$(INSTALL_DATA) Readme.txt $(prefix)/Docs/vmetcl/VmeTcl.txt
	cp $(prefix)/lib/libVme.* $(LIBDIR)
	echo -----------------------

man_MANS =  vme.3tcl caennet.3tcl 