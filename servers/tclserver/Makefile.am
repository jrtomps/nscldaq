bin_PROGRAMS = tclserver
tclserver_SOURCES = TclAuthorizer.cpp serverinstance.cpp tclAppInit.cpp \
	tkAppInit.cpp Server.cpp

tclserver_LDADD = -L@top_srcdir@/base/tclplus -ltclPlus -L@top_srcdir@/base/exception \
	-L@top_srcdir@/servers/portmanager \
	-L@top_srcdir@/base/CopyrightTools -lPortManager -lLicense -lException $(TCL_LDFLAGS) \
	$(X11LIBS)  
tclserver_LDFLAGS=-Wl,"-rpath=$(libdir)"

INCLUDES = -I@top_srcdir@/base/headers -I@top_srcdir@/base/exception \
	-I@top_srcdir@/servers/portmanager \
	-I@top_srcdir@/base/tclplus -I@top_srcdir@/base/CopyrightTools $(TCL_FLAGS)

noinst_HEADERS = TclAuthorizer.h \
	server.h \
	serverinstance.h


EXTRA_DIST = tclserver.xml
