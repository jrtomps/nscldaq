lib_LTLIBRARIES = libSecurity.la
libSecurity_la_SOURCES = AccessListCheck.cpp  \
	FdInteractor.cpp HostListCheck.cpp \
	IOInteractor.cpp Interactor.cpp PasswordCheck.cpp  \
	StringInteractor.cpp TclAccessListCheck.cpp UnixUserCheck.cpp
include_HEADERS = AccessListCheck.h  Authenticator.h \
	FdInteractor.h HostListCheck.h \
	IOInteractor.h Interactor.h PasswordCheck.h StringInteractor.h \
	TclAccessListCheck.h UnixUserCheck.h
libSecurity_la_LDFLAGS = -version-info $(SOVERSION):0 -Wl,"-rpath=$(libdir)"

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/TCL $(TCL_FLAGS) $(SPECTRODAQ_FLAGS)

EXTRA_DIS= security.xml



noinst_PROGRAMS = unittests

unittests_SOURCES = test.cpp 

unittests_DEPENDENCIES = AccessListCheck.o  \
	FdInteractor.o  HostListCheck.o \
	IOInteractor.o Interactor.o PasswordCheck.o  \
	StringInteractor.o TclAccessListCheck.o UnixUserCheck.o

unittests_LDADD = $(unittests_DEPENDENCIES) -L@top_srcdir@/TCL -ltclPlus -L@top_srcdir@/Exception -lException \
	$(SPECTRODAQ_LDFLAGS) $(TCL_LDFLAGS)

TESTS = ./unittests

