bin_PROGRAMS		=	sclclient

noinst_PROGRAMS         =       testData

sclclient_SOURCES	=	TclServerConnection.cpp		\
				TcpClient.cpp			\
				sclclientMain.cpp		\
				sclclient.cpp @srcdir@/sclclientargs.c

noinst_HEADERS		=	TclServerConnection.h		\
				TcpClient.h			\
				sclclientMain.h	



COMPILATION_FLAGS		=	-I@top_srcdir@/base/headers	\
				-I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/daq/format	\
                                -I@top_srcdir@/utilities/IO	\
				@LIBTCLPLUS_CFLAGS@		\
				-I@top_srcdir@/servers/portmanager \
				-I@top_srcdir@/base/os


sclclient_CPPFLAGS=$(COMPILATION_FLAGS)

BUILT_SOURCES		= 	@srcdir@/sclclientargs.c

sclclient_LDADD		=	@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/daq/format/libdataformat.la	\
                                @top_builddir@/utilities/IO/libdaqio.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/base/uri/liburl.la			\
				@top_builddir@/base/os/libdaqshm.la    \
				@LIBEXCEPTION_LDFLAGS@	\
				$(THREADLD_FLAGS)

sclclient_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

testData_SOURCES        =       testData.cpp
testData_CXXFLAGS       =       -I@top_srcdir@/daq/format $(COMPILATION_FLAGS)
testData_LDADD          =       @top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/base/uri/liburl.la			\
				@top_builddir@/base/os/libdaqshm.la    \
				@LIBEXCEPTION_LDFLAGS@ 	\
				$(THREADLD_FLAGS)

CLEANFILES=@srcdir@/sclclientargs.c @srcdir@/sclclientargs.h


@srcdir@/sclclientargs.c: @srcdir@/sclclientargs.h

@srcdir@/sclclientargs.h: @srcdir@/sclclientargs.ggo
	$(GENGETOPT) < @srcdir@/sclclientargs.ggo --file sclclientargs \
		--output-dir=@srcdir@


EXTRA_DIST		= 	sclclientargs.ggo	\
				sclclient.xml
