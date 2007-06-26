bin_PROGRAMS = assembler

assembler_SOURCES =	AssemblerApplication.cpp	\
			AssemblerCommand.cpp		\
			AssemblerErrors.cpp		\
			AssembledEvent.cpp		\
			AssembledPhysicsEvent.cpp	\
			AssembledScalerEvent.cpp	\
			AssembledStateTransitionEvent.cpp \
			AssembledStringArrayEvent.cpp	\
			BadEventTypeException.cpp	\
			AssemblerOutputStage.cpp

noinst_HEADERS = 	AssemblerApplication.h		\
			AssemblerCommand.h		\
			AssemblerErrors.h		\
			AssembledEvent.h		\
			AssembledPhysicsEvent.h		\
			AssembledScalerEvent.h		\
			AssembledStateTransitionEvent.h	\
			AssembledStringArrayEvent.h	\
			BadEventTypeException.h		\
			AssemblerOutputStage.h


assembler_LDADD =	@top_srcdir@/TCL/libtclPlus.la	\
			@top_srcdir@/Exception/libException.la \
			@top_srcdir@/OutputBuffers/libOutputBuffers.la \
			@top_srcdir@/SpectrodaqLite/libSpectrodaqCompatibility.la \
			$(SPECTRODAQ_LDFLAGS)

assembler_LDFLAGS = 	$(TCL_LDFLAGS)			\
			-Wl,"-rpath=$(libdir)"

INCLUDES	= 	-I@top_srcdir@/TCL		\
			-I@top_srcdir@/Exception	\
			-I@top_srcdir@/Headers		\
			-I@top_srcdir@/OutputBuffers	\
			-I@top_srcdir@/SpectrodaqLite   \
			$(TCL_FLAGS)			\
			$(SPECTRODAQ_FLAGS)

EXTRA_DIST=configtests.tcl

installcheck-local:
	$(bindir)/assembler configtests.tcl
	$(bindir)/assembler outstagetests.tcl
