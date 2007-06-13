lib_LTLIBRARIES = libHPOutputBuffers.la


libHPOutputBuffers_la_LIBADD = @top_srcdir@/Exception/libException.la \
				$(SPECTRODAQ_LDFLAGS)

libHPOutputBuffers_la_LDFLAGS = -version-info $(SOVERSION):0 \
				-g -Wl,"-rpath-link=$(libdir)"

INCLUDES  = -DHIGH_PERFORMANCE		\
	 $(SPECTRODAQ_FLAGS)		\
	-I@top_srcdir@/SpectrodaqLite	\
	-I@top_srcdir@/Exception	\
	-I@top_srcdir@/Headers

libHPOutputBuffers_la_SOURCES = ../CNSCLBeginBuffer.cpp	\
				../CNSCLControlBuffer.cpp	\
				../CNSCLDocumentationBuffer.cpp	\
				../CNSCLEndBuffer.cpp	\
				../CNSCLOutputBuffer.cpp	\
				../CNSCLPauseBuffer.cpp	\
				../CNSCLPhysicsBuffer.cpp	\
				../CNSCLResumeBuffer.cpp	\
				../CNSCLScalerBuffer.cpp	\
				../CNSCLStringListBuffer.cpp


include_HEADERS =		../CNSCLBeginBuffer.h	\
				../CNSCLControlBuffer.h	\
				../CNSCLDocumentationBuffer.h	\
				../CNSCLEndBuffer.h	\
				../CNSCLOutputBuffer.h	\
				../CNSCLPauseBuffer.h	\
				../CNSCLPhysicsBuffer.h	\
				../CNSCLResumeBuffer.h	\
				../CNSCLScalerBuffer.h	\
				../CNSCLStringListBuffer.h
