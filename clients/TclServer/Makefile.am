bin_PROGRAMS = tclserver
tclserver_SOURCES = TclAuthorizer.cpp serverinstance.cpp tclAppInit.cpp \
	tkAppInit.cpp Server.cpp

tclserver_LDADD = -L@top_srcdir@/TCL -lTclPlus -L@top_srcdir@/Exception \
	-L@top_srcdir@/CopyrightTools -lLicense -lException $(TCL_LDFLAGS) \
	$(X11LIBS) 

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception \
	-I@top_srcdir@/TCL -I@top_srcdir@/CopyrightTools $(TCL_FLAGS)

noinst_HEADERS = TclAuthorizer.h \
	server.h \
	serverinstance.h
