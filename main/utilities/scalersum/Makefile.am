bin_PROGRAMS = sumscaler

sumscaler_SOURCES=   main.cpp App.cpp App.h options.c CRun.h CRun.cpp

sumscaler_CPPFLAGS=-I@top_srcdir@/utilities/IO @LIBTCLPLUS_CFLAGS@
sumscaler_LDFLAGS = @top_builddir@/utilities/IO/libdaqio.la \
    @LIBEXCEPTION_LDFLAGS@

BUILT_SOURCES	= options.c

options.c: options.h

options.h: options.ggo
	$(GENGETOPT) < @srcdir@/options.ggo --file=options \
	    --unamed-opts

clean-local:
	-rm -rf options.h options.h
	
