bin_PROGRAMS		=	sclclient

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
				-I@top_srcdir@/base/exception	\
				-I@top_srcdir@/servers/portmanager

sclclient_DEPENDENCIES	=	sclclientargs.o


sclclient_LDADD		=	sclclientargs.o			\
				@top_srcdir@/daq/format/libdataformat.la	\
				@top_srcdir@/base/dataflow/libDataFlow.la	\
				@top_srcdir@/base/exception/libException.la


sclclientargs.o: sclclientargs.ggo
	$(GENGETOPT) <sclclientargs.ggo --file sclclientargs
	$(CC) -c -I. sclclientargs.c

sclclientargs.h: sclclientargs.ggo
	$(GENGETOPT) <sclclientargs.ggo --file sclclientargs


EXTRA_DIST		= 	sclclientargs.ggo	\
				sclclient.xml