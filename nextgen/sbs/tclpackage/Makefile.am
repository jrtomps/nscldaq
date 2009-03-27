PKGDIR = @prefix@/TclLibs/SBSVme

lib_LTLIBRARIES      = libSBSVme.la
libSBSVme_la_SOURCES =  VMECommand.cpp VMEMapCommand.cpp VMEPackage.cpp

libSBSVme_la_LDFLAGS =  	-version-info 1:0:0 -Wl,"-rpath=$(libdir)"

libSBSVme_la_LIBADD = @top_srcdir@/base/tclplus/libtclPlus.la    \
		@top_srcdir@/base/exception/libException.la \
		@top_srcdir@/sbs/nsclapi//libSBSVmeAPI.la       \
		@top_srcdir@/base/CopyrightTools/libLicense.la     \
		$(TCL_LDFLAGS) 					\
		@top_srcdir@/sbs/driver/src/libbtp.la  -lpthread \
		-lm -lstdc++ -lgcc -lc


INCLUDES = -I@top_srcdir@/base/headers -I@top_srcdir@/base/exception \
	-I@top_srcdir@/sbs/nsclapi -I@top_srcdir@/base/tclplus \
	-I@top_srcdir@/base/CopyrightTools \
	$(TCL_FLAGS) -DHOME=\"$(prefix)\"


noinst_HEADERS = VMECommand.h VMEMapCommand.h


install-exec-local:
	$(mkinstalldirs) $(PKGDIR)
	$(INSTALL_PROGRAM) .libs/libSBSVme.a $(PKGDIR)
	$(INSTALL_PROGRAM) .libs/libSBSVme.so.* $(PKGDIR)
	echo "package ifneeded Vme 2.0 [list load [file join \$$dir libSBSVme.so]]" >> $(PKGDIR)/pkgIndex.tcl