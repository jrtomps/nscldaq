lib_LTLIBRARIES = libdataformatv12.la

libdataformatv12_la_SOURCES = CRawRingItem.cpp \
                              CPhysicsEventItem.cpp \
                              CRingScalerItem.cpp 

#                           CAbnormalEndItem.cpp \
#                           CRingStateChangeItem.cpp \
#                           CPhysicsEventItem.cpp \
#                           CRingPhysicsEventCountItem.cpp \
#                           CRingTextItem.cpp \
#                           CRingFragmentItem.cpp \
#                           CGlomParameters.cpp \
#                           CUnknownFragment.cpp \
#                           CDataFormatItem.cpp \
#                           CRingItemFactory.cpp \
#                                                                                                        RingItemComparisons.cpp \
#													 ringitem.c
nscldaq12dir = @includedir@/V12

nscldaq12_HEADERS = CRingItem.h \
                    CRawRingItem.h \
                    CPhysicsEventItem.h \
                    CRingScalerItem.h 

#                  CAbnormalEndItem.h \
#                  CRingStateChangeItem.h \
#                  CPhysicsEventItem.h \
#                  CRingPhysicsEventCountItem.h \
#                  CRingTextItem.h \
#                  CRingFragmentItem.h \
#                  CGlomParameters.h \
#                  CUnknownFragment.h \
#                  CDataFormatItem.h \
#                  CRingItemFactory.h \
#                  RingItemComparisons.h \
#                  DataFormatV11.h

libdataformatv12_la_CFLAGS = -I@srcdir@/..

libdataformatv12_la_CPPFLAGS = -I@srcdir@/.. \
                               -I@top_srcdir@/utilities/Buffer \
                               @LIBTCLPLUS_CFLAGS@

libdataformatv12_la_LDFLAGS = @top_builddir@/utilities/Buffer/libbuffer.la \
                                @LIBTCLPLUS_LDFLAGS@

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp \
                          rawringitemtests.cpp \
                          physeventtests.cpp \
													scalertests.cpp
#										ringitemtests.cpp teststate.cpp	\
#										 texttest.cpp physcounttests.cpp		\
#										formatoutputtests.cpp dataformattests.cpp       \
#										fragmenttest.cpp glomparamtests.cpp factorytests.cpp

unittests_LDADD	= $(CPPUNIT_LDFLAGS) 		\
                        @builddir@/libdataformatv12.la	\
                        @top_builddir@/utilities/Buffer/libbuffer.la \
                        @LIBEXCEPTION_LDFLAGS@

unittests_CFLAGS= -I@srcdir@/..
unittests_CPPFLAGS= -I@srcdir@/.. \
                    -I@top_srcdir@/utilities/Buffer \
		    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
                    -I@top_srcdir@/base/testutils

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                            @TCL_LDFLAGS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
