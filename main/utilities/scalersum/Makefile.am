bin_PROGRAMS = sumscaler

sumscaler_SOURCES=   main.cpp App.cpp App.h options.c CRun.h CRun.cpp \
	CChannel.h CIncrementalChannel.j CIncrementalChannel.cpp \
	CCumulativeChannel.h CCumulativeChannel.cpp

sumscaler_CPPFLAGS=-I@top_srcdir@/utilities/IO 			\
		    -I@top_srcdir@/daq/format			\
		    @LIBTCLPLUS_CFLAGS@
sumscaler_LDFLAGS = @top_builddir@/utilities/IO/libdaqio.la 	\
    @top_builddir@/daq/format/libdataformat.la			\
    @LIBEXCEPTION_LDFLAGS@

BUILT_SOURCES	= options.c

options.c: options.h

options.h: options.ggo
	$(GENGETOPT) < @srcdir@/options.ggo --file=options \
	    --unamed-opts

clean-local:
	-rm -rf options.h options.h

