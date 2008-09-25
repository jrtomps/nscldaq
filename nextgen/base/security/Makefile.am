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

EXTRA_DIS= security.xml



noinst_PROGRAMS = unittests

unittests_SOURCES = test.cpp 

unittests_DEPENDENCIES = AccessListCheck.o  \
	FdInteractor.o  HostListCheck.o \
	IOInteractor.o Interactor.o PasswordCheck.o  \
	StringInteractor.o TclAccessListCheck.o UnixUserCheck.o

unittests_LDADD = $(unittests_DEPENDENCIES) -L@top_srcdir@/base/tclplus \
	-ltclPlus -L@top_srcdir@/base/exception -lException \
	$(SPECTRODAQ_LDFLAGS) $(TCL_LDFLAGS) -lcrypt

TESTS = ./unittests

