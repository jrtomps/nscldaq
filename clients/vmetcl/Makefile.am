SUBDIRS = camactcl vhq202m
PKGDIR  = $(prefix)/TclLibs/caennet
LIBDIR  = $(prefix)/TclLibs

noinst_PROGRAMS=TestHarness
TestHarness_SOURCES= TestHarness.cpp
TestHarness_DEPENDENCIES=VMECommand.o VMEMapCommand.o

TestHarness_LDFLAGS = VMECommand.o VMEMapCommand.o \
	 -L@top_srcdir@/VMEApi -lVmeAPI $(VMEBUILDLIBS) \
	 -L@top_srcdir@/TCL -L@top_srcdir@/Exception  \
	-L@top_srcdir@/CopyrightTools\
	-L@top_srcdir@/DeviceSupport -lDeviceSupport -ltclPlus -lException \
	$(TCL_LDFLAGS)

lib_LTLIBRARIES = libVme.la
libVme_la_SOURCES = VMECommand.cpp VMEMapCommand.cpp VMEPackage.cpp

libVme_la_LDFLAGS =    \
	 -L@top_srcdir@/TCL -L@top_srcdir@/Exception  \
	-L@top_srcdir@/CopyrightTools\
	-L@top_srcdir@/DeviceSupport -lDeviceSupport \
	-version-info $(SOVERSION):0 -Wl,"-rpath=$(libdir)"

libVme_la_LIBADD =  -ltclPlus -lException  \
	$(TCL_LDFLAGS)   $(X11LIBS)  \
	-lVmeAPI $(VMEBUILDLIBS)  -lLicense -lpthread \
	-lm -lstdc++ -lgcc -lc 


INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/VMEApi -I@top_srcdir@/TCL \
	-I@top_srcdir@/CopyrightTools \
	-I@top_srcdir@/DeviceSupport \
	$(TCL_FLAGS) -DHOME=\"$(prefix)\"

install-exec-local:
	$(mkinstalldirs) $(PKGDIR)
	$(mkinstalldirs) $(LIBIDR)
	$(INSTALL_SCRIPT) caennet.tcl $(PKGDIR)
	echo "pkg_mkIndex -verbose  $(PKGDIR) *.tcl" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl

EXTRA_DIST = mkpackage.tcl caennet.tcl Readme.txt \
		vme.3tcl caennet.3tcl  

noinst_HEADERS = TestHarness.h \
	VMECommand.h \
	VMEMapCommand.h \
	vmcaenet.h

install-data-local:
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/Docs/vmetcl
	$(mkinstalldirs) $(prefix)/etc
	$(INSTALL_DATA) Readme.txt $(prefix)/Docs/vmetcl/VmeTcl.txt
	mv $(prefix)/lib/libVme* $(LIBDIR)

man_MANS =  vme.3tcl caennet.3tcl 