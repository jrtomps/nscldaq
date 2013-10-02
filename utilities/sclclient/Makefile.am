bin_PROGRAMS		=	sclclient

noinst_PROGRAMS         =       testData

sclclient_SOURCES	=	TclServerConnection.cpp		\
				TcpClient.cpp			\
				sclclientMain.cpp		\
				sclclient.cpp

noinst_HEADERS		=	TclServerConnection.h		\
				TcpClient.h			\
				sclclientMain.h			\
				sclclientargs.h

INCLUDES		=	-I@top_srcdir@/base/headers	\
				-I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/daq/format	\
				@LIBTCLPLUS_CFLAGS@		\
				-I@top_srcdir@/servers/portmanager \
				-I@top_srcdir@/base/os

sclclient_DEPENDENCIES	=	sclclientargs.o

BUILT_SOURCES		= 	sclclientargs.o

sclclient_LDADD		=	sclclientargs.o			\
				@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/base/uri/liburl.la			\
				@top_builddir@/base/os/libdaqshm.la    \
				@LIBEXCEPTION_LDFLAGS@	\
				$(THREADLD_FLAGS)

sclclient_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

testData_SOURCES        =       testData.cpp
testData_CXXFLAGS       =       -I@top_srcdir@/daq/format
testData_LDADD          =       @top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/servers/portmanager/libPortManager.la \
				@top_builddir@/base/uri/liburl.la			\
				@top_builddir@/base/os/libdaqshm.la    \
				@LIBEXCEPTION_LDFLAGS@ 	\
				$(THREADLD_FLAGS)



sclclientargs.o: sclclientargs.ggo
	$(GENGETOPT) <sclclientargs.ggo --file sclclientargs
	$(CC) -c -I. sclclientargs.c

sclclientargs.h: sclclientargs.ggo
	$(GENGETOPT) <sclclientargs.ggo --file sclclientargs


EXTRA_DIST		= 	sclclientargs.ggo	\
				sclclient.xml
