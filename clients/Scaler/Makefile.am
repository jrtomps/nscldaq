bin_PROGRAMS = sclclient
sclclient_SOURCES = ScalerClient.cpp \
	Consumer.cpp DataSource.cpp EventIterator.cpp \
	NSCLBufferCreator.cpp NSCLBufferFactory.cpp NSCLDaqBuffer.cpp \
	NSCLEventBuffer.cpp NSCLPhysicsCreator.cpp NSCLScalerBuffer.cpp \
	NSCLScalerCreator.cpp NSCLStateChangeBuffer.cpp \
	NSCLStateChangeCreator.cpp TclServerConnection.cpp \
	TcpClient.cpp
INCLUDES=-I@top_srcdir@/Headers \
	-I@top_srcdir@/SpectrodaqLite \
	-I@top_srcdir@/CopyrightTools  \
	-I@top_srcdir@/cvt \
	$(SPECTRODAQ_FLAGS)

sclclient_LDADD = @top_srcdir@/CopyrightTools/libLicense.la \
		@top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la \
		@top_srcdir@/cvt/libcvt.la \
		 $(SPECTRODAQ_LDFLAGS) 

sclclient_LDFLAGS=-Wl,"-rpath=$(libdir)"

AM_CXXFLAGS= -DINSTDIR=\"$(prefix)\"

noinst_HEADERS = Consumer.h \
	DataSource.h \
	EventIterator.h \
	NSCLBufferCreator.h \
	NSCLBufferFactory.h \
	NSCLDaqBuffer.h \
	NSCLEventBuffer.h \
	NSCLPhysicsCreator.h \
	NSCLScalerBuffer.h \
	NSCLScalerCreator.h \
	NSCLStateChangeBuffer.h \
	NSCLStateChangeCreator.h \
	ScalerClient.h \
	TclServerConnection.h \
	TcpClient.h \
	buftypes.h \
	nsclbuffer.h	

install-exec-local:
	$(mkinstalldirs) $(bindir)
	$(INSTALL_SCRIPT) scalerlocal.tk $(bindir)


EXTRA_DIST = scalerlocal.tk sclclient.1

man_MANS = sclclient.1