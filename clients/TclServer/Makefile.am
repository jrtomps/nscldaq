bin_PROGRAMS = tclserver
tclserver_SOURCES = TclAuthorizer.cpp serverinstance.cpp tclAppInit.cpp \
	tkAppInit.cpp Server.cpp

tclserver_LDADD = -L@top_srcdir@/TCL -lTclPlus -L@top_srcdir@/Exception \
	-L@top_srcdir@/PortManager \
	-L@top_srcdir@/CopyrightTools -lPortManager -lLicense -lException $(TCL_LDFLAGS) \
	$(X11LIBS)  
tclserver_LDFLAGS=-Wl,"-rpath=$(libdir)"

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/PortManager \
	-I@top_srcdir@/TCL -I@top_srcdir@/CopyrightTools $(TCL_FLAGS)

noinst_HEADERS = TclAuthorizer.h \
	server.h \
	serverinstance.h

man_MANS = tclserver.1

EXTRA_DIST = tclserver.1
