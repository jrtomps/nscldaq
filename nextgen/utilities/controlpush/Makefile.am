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
			CSocket.cpp CStrings.cpp CTCPBadSocketState.cpp \
			CTCPConnectionFailed.cpp CTCPConnectionLost.cpp \
			CTCPNoSuchHost.cpp CTCPNoSuchService.cpp \
			CLookupVisitor.cpp  CUnitChannel.cpp \
			CApplicationSerializer.cpp CBuildChannelData.cpp \
			CChannel.cpp
controlpush_SOURCES = Main.cpp $(APPITEMS)


noinst_HEADERS= cmdline.h CApplication.h CChannelList.h \
			CChannelVisitor.h CSocket.h CStrings.h \
			CTCPBadSocketState.h CTCPConnectionFailed.h \
			CTCPConnectionLost.h CTCPNoSuchHost.h \
			CTCPNoSuchService.h \
			CLookupVisitor.h CChannel.h CUnitChannel.h \
			CApplicationSerializer.h CBuildChannelData.h

controlpush_LDADD   =  -L@top_srcdir@/base/exception -lException \
			-L@top_srcdir@/base/CopyrightTools -lLicense \
			$(EPICS_LDFLAGS)  $(THREADLD_FLAGS)
controlpush_LDFLAGS= -Wl,"-rpath=$(libdir)"

INCLUDES            =	-I@top_srcdir@/base/exception		\
			-I@top_srcdir@/base/CopyrightTools	\
			-I@top_srcdir@/base/headers		\
			$(EPICS_BIN) $(EPICS_INCLUDES) 

controlpush_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)


cmdline.c: options.ggo
	$(GENGETOPT) --unamed-opts <options.ggo

EXTRA_DIST=options.ggo  Channels.dat  controlpush.xml


noinst_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp ChannelListTests.cpp  \
		LookupVisitorTest.cpp \
		TestChannel.cpp TestUnitChannel.cpp \
		$(APPITEMS)

unittests_LDADD = $(CPPUNIT_LDFLAGS) $(controlpush_LDADD)

TESTS=./unittests

endif