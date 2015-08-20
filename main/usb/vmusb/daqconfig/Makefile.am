lib_LTLIBRARIES		= libVMUSBConfig.la
libVMUSBConfig_la_SOURCES	= 	CADCCommand.cpp		\
				CConfiguration.cpp	\
				CScalerCommand.cpp	\
				CCAENChainCommand.cpp	\
				CStackCommand.cpp	\
				CCAENV830Command.cpp	\
				CMarkerCommand.cpp	\
				CMADCScalerCommand.cpp	\
				CMADCChainCommand.cpp   \
				CHINPCommand.cpp \
				CPSDCommand.cpp \
				CV1729Command.cpp	\
				CV1495scCommand.cpp	\
				CUserCommand.cpp	\
				CAddTclDriver.cpp	\
				CHiRACommand.cpp	\
				CVMUSBCommand.cpp	\
				CDeviceCommand.cpp	

libVMUSBConfig_la_CXXFLAGS=@THREADCXX_FLAGS@


noinst_HEADERS		=	CADCCommand.h  		\
				CConfiguration.h 	\
				CScalerCommand.h	\
				CCAENChainCommand.h	\
				CStackCommand.h		\
				CCAENV830Command.h	\
				CMarkerCommand.h	\
				CMADCScalerCommand.h	\
				CMADCChainCommand.h	\
				CHINPCommand.h \
				CPSDCommand.h		\
				CV1729Command.h		\
				CV1495scCommand.h	\
				CUserCommand.h		\
				CAddTclDriver.h		\
				CHiRACommand.h		\
				CVMUSBCommand.h		\
				CDeviceCommand.h



libVMUSBConfig_la_CPPFLAGS		= 	-I@top_srcdir@/usb/vmusb/devices	\
		    -I@top_srcdir@/usb/common/configurableobject \
				-I@top_srcdir@/usb/vmusb		\
				-I@top_srcdir@/usb/vmusb/vmusb		\
				-I@top_srcdir@/usb/common/devices		\
				@LIBTCLPLUS_CFLAGS@		\
				-I@top_srcdir@/base/headers		\
				$(TCL_FLAGS) 


libVMUSBConfig_la_LIBADD	= @top_builddir@/usb/vmusb/devices/libVMUSBDevices.la \
				@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
				@LIBTCLPLUS_LDFLAGS@	\
				$(TCL_LDFLAGS) @THREADLD_FLAGS@

install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/vmusbdriver/includes


noinst_PROGRAMS=cmdtests

cmdtests_SOURCES=TestRunner.cpp Asserts.h ad811test.cpp \
	v830tests.cpp hinptest.cpp hiratests.cpp madcscaler.cpp \
	markertests.cpp sis3820test.cpp v1495tests.cpp v1729tests.cpp \
	sis3804tests.cpp hytectests.cpp madctests.cpp tdc1x90tests.cpp \
	v977tests.cpp masetests.cpp mtdctests.cpp delaytests.cpp \
	xlmferatests.cpp cbdtests.cpp xlmtimestamptests.cpp mqdctests.cpp \
	cbdcratetests.cpp ulmtriggertests.cpp lrs4300tests.cpp lrs4434tests.cpp \
	lrs2551tests.cpp


cmdtests_CXXFLAGS=@CPPUNIT_CFLAGS@ $(libVMUSBConfig_la_CXXFLAGS) $(libVMUSBConfig_la_CPPFLAGS)
cmdtests_LDFLAGS=@CPPUNIT_LDFLAGS@ libVMUSBConfig.la $(libVMUSBConfig_la_LIBADD)

TESTS=cmdtests
