lib_LTLIBRARIES = libSecurity.la
libSecurity_la_SOURCES = AccessListCheck.cpp  \
	FdInteractor.cpp HostListCheck.cpp \
	IOInteractor.cpp Interactor.cpp PasswordCheck.cpp  \
	StringInteractor.cpp TclAccessListCheck.cpp UnixUserCheck.cpp
include_HEADERS = AccessListCheck.h  Authenticator.h \
	FdInteractor.h HostListCheck.h \
	IOInteractor.h Interactor.h PasswordCheck.h StringInteractor.h \
	TclAccessListCheck.h UnixUserCheck.h
libSecurity_la_LDFLAGS = -version-info $(SOVERSION) -Wl,"-rpath=$(libdir)" \
		$(THREADLD_FLAGS)

libSecurity_la_CXXFLAGS	= $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

INCLUDES = -I@top_srcdir@/base/headers -I@top_srcdir@/base/tclplus $(TCL_FLAGS) $(SPECTRODAQ_FLAGS)

EXTRA_DIST= security.xml



