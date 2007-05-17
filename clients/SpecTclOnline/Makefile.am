bin_PROGRAMS = spectcldaq

spectcldaq_SOURCES = spectcldaq.cpp

INCLUDES=-I@top_srcdir@/Headers \
	-I@top_srcdir@/SpectrodaqLite \
	-I@top_srcdir@/CopyrightTools \
	$(SPECTRODAQ_FLAGS)

spectcldaq_LDADD=@top_srcdir@/CopyrightTools/libLicense.la			\
		@top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la	\
		$(SPECTRODAQ_LDFLAGS) 

spectcldaq_LDFLAGS= -Wl,"-rpath=$(libdir)"

man_MANS = spectcldaq.1

EXTRA_DIST = spectcldaq.1


if CPPUNIT
noinst_PROGRAMS  = testdriver injector

injector_SOURCES   = injector.cpp
injector_LDADD     = $(SPECTRODAQ_LDFLAGS)

testdriver_SOURCES = TestRunner.cpp testSpectclDaq.cpp 
testdriver_LDADD   = $(CPPUNIT_LDFLAGS)
noinst_HEADERS     = Asserts.h


TESTS  = ./testdriver
endif