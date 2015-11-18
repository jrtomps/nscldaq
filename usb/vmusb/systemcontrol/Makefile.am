lib_LTLIBRARIES		= libVMUSBSystemControl.la
libVMUSBSystemControl_la_SOURCES	= CSystemControl.cpp	
noinst_HEADERS		=  CSystemControl.h

libVMUSBSystemControl_la_CXXFLAGS= \
				@THREADCXX_FLAGS@ \
				-I@top_srcdir@/usb/vmusb/commands	\
				-I@top_srcdir@/usb/vmusb	\
				@LIBTCLPLUS_CFLAGS@	\
				@TCL_CPPFLAGS@
				@TCL_FLAGS@

libVMUSBSystemControl_la_LIBADD	= \
				@top_builddir@/usb/vmusb/commands/libVMUSBCommands.la \
				@LIBTCLPLUS_LDFLAGS@		\
				@TCL_LDFLAGS@ \
				@THREADLD_FLAGS@
