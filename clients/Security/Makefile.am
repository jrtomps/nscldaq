lib_LTLIBRARIES = libSecurity.la
libSecurity_la_SOURCES = AccessListCheck.cpp AuthTokenCheck.cpp \
	DAQAuthTokenInteractor.cpp FdInteractor.cpp HostListCheck.cpp \
	IOInteractor.cpp Interactor.cpp PasswordCheck.cpp  \
	StringInteractor.cpp TclAccessListCheck.cpp UnixUserCheck.cpp
include_HEADERS = AccessListCheck.h AuthTokenCheck.h Authenticator.h \
	DAQAuthTokenInteractor.h DaqSecurity.h FdInteractor.h HostListCheck.h \
	IOInteractor.h Interactor.h PasswordCheck.h StringInteractor.h \
	TclAccessListCheck.h UnixUserCheck.h
libSecurity_la_LDFLAGS = -version-info $(SOVERSION):0 -Wl,"-rpath=$(libdir)"

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/TCL $(TCL_FLAGS) $(SPECTRODAQ_FLAGS)


if CPPUNIT

noinst_PROGRAMS = unittests

unittests_SOURCES = test.cpp AccessListCheck.cpp AuthTokenCheck.cpp \
	DAQAuthTokenInteractor.cpp FdInteractor.cpp HostListCheck.cpp \
	IOInteractor.cpp Interactor.cpp PasswordCheck.cpp  \
	StringInteractor.cpp TclAccessListCheck.cpp UnixUserCheck.cpp

unittests_LDADD = -L@top_srcdir@/TCL -lTclPlus -L@top_srcdir@/Exception -lException \
	$(SPECTRODAQ_LDFLAGS) $(TCL_LDFLAGS)

TESTS = ./unittests

endif