#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


#  This application requires epics:

if EPICS
LIBDIR=$(prefix)/TclLibs

bin_PROGRAMS        = controlpush


APPITEMS= cmdline.c CApplication.cpp CChannelList.cpp \
			CStrings.cpp \
			CLookupVisitor.cpp  CUnitChannel.cpp \
			CBuildChannelData.cpp \
			CChannel.cpp
controlpush_SOURCES = Main.cpp $(APPITEMS)


noinst_HEADERS= cmdline.h CApplication.h CChannelList.h \
			CChannelVisitor.h CStrings.h \
			CLookupVisitor.h CChannel.h CUnitChannel.h \
			CBuildChannelData.h

controlpush_LDADD   =   @top_builddir@/base/tcpip/libTcp.la	\
			@top_builddir@/base/exception/libException.la \
			@top_builddir@/base/CopyrightTools/libLicense.la \
			$(EPICS_LDFLAGS)  $(THREADLD_FLAGS)
controlpush_LDFLAGS= -Wl,"-rpath=$(libdir)"

INCLUDES            =	-I@top_srcdir@/base/exception		\
			-I@top_srcdir@/base/CopyrightTools	\
			-I@top_srcdir@/base/headers		\
			-I@top_srcdir@/base/tcpip	\
			$(EPICS_BIN) $(EPICS_INCLUDES) 

controlpush_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)


cmdline.c: options.ggo
	$(GENGETOPT) --unamed-opts <options.ggo

EXTRA_DIST = options.ggo  Channels.dat  controlpush.xml


noinst_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp ChannelListTests.cpp  \
		LookupVisitorTest.cpp \
		TestChannel.cpp TestUnitChannel.cpp \
		$(APPITEMS)

unittests_LDADD = $(CPPUNIT_LDFLAGS) $(controlpush_LDADD)

# TESTS=./unittests          # These tests are NSCL specific!!!

endif