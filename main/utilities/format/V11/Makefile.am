lib_LTLIBRARIES = libdataformatv11.la

libdataformatv11_la_SOURCES = CRingItem.cpp \
                           CAbnormalEndItem.cpp \
                           CRingScalerItem.cpp \
                           CRingStateChangeItem.cpp \
                           CPhysicsEventItem.cpp \
                           CRingPhysicsEventCountItem.cpp \
                           CRingTextItem.cpp \
                           CRingFragmentItem.cpp \
                           CGlomParameters.cpp \
                           CUnknownFragment.cpp \
                           CDataFormatItem.cpp \
                           CRingItemFactory.cpp \
                                                                                                        RingItemComparisons.cpp \
													 ringitem.c 
nscldaq11dir = @includedir@/V11

nscldaq11_HEADERS = CRingItem.h \
                  CAbnormalEndItem.h \
                  CRingScalerItem.h \
                  CRingStateChangeItem.h \
                  CPhysicsEventItem.h \
                  CRingPhysicsEventCountItem.h \
                  CRingTextItem.h \
                  CRingFragmentItem.h \
                  CGlomParameters.h \
                  CUnknownFragment.h \
                  CDataFormatItem.h \
                  CRingItemFactory.h \
                  RingItemComparisons.h \
                  DataFormatV11.h 

libdataformatv11_la_CFLAGS = -I@srcdir@/..

libdataformatv11_la_CXXFLAGS = -I@srcdir@/.. \
                               @LIBTCLPLUS_CFLAGS@

libdataformatv11_la_LDFLAGS = @LIBTCLPLUS_LDFLAGS@

noinst_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp \
										ringitemtests.cpp teststate.cpp	\
										scalertests.cpp texttest.cpp physcounttests.cpp		\
                                                                                physeventtests.cpp \
										formatoutputtests.cpp dataformattests.cpp       \
										fragmenttest.cpp glomparamtests.cpp factorytests.cpp 

unittests_LDADD	= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
			@builddir@/libdataformatv11.la	\
			@LIBEXCEPTION_LDFLAGS@

unittests_CFLAGS= -I@srcdir@/..
unittests_CPPFLAGS= -I@srcdir@/.. \
		    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
                    -I@top_srcdir@/base/testutils

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                            @TCL_LDFLAGS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
