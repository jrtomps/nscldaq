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
			InvalidNodeException.cpp	\
			AssemblerOutputStage.cpp	\
			AssemblerWrongStateException.cpp \
			EventTooSmallException.cpp	\
			EventFragment.cpp		\
			PhysicsFragment.cpp		\
			ScalerFragment.cpp		\
			StringListFragment.cpp		\
			InputBuffer.cpp			\
			ScalerInputBuffer.cpp		\
			PhysicsInputBuffer.cpp		\
			InputBufferFactory.cpp		\
			StringListInputBuffer.cpp	\
			StateTransitionInputBuffer.cpp	\
			StateTransitionFragment.cpp	\
			InputStage.cpp 			\
			InputStageCommand.cpp 		\
			FragmentQueue.cpp

noinst_HEADERS = 	AssemblerApplication.h		\
			AssemblerCommand.h		\
			AssemblerErrors.h		\
			AssembledEvent.h		\
			AssembledPhysicsEvent.h		\
			AssembledScalerEvent.h		\
			AssembledStateTransitionEvent.h	\
			AssembledStringArrayEvent.h	\
			BadEventTypeException.h		\
			InvalidNodeException.h		\
			AssemblerOutputStage.h		\
			AssemblerWrongStateException.h	\
			EventTooSmallException.h	\
			EventFragment.h			\
			PhysicsFragment.h		\
			ScalerFragment.h		\
			StringListFragment.h		\
			StateTransitionFragment.h	\
			InputBuffer.h			\
			ScalerInputBuffer.h		\
			StateTransitionInputBuffer.h	\
			StringListInputBuffer.h		\
			PhysicsInputBuffer.h		\
			InputBufferFactory.h		\
			InputStage.h			\
			InputStageCommand.h		\
			FragmentQueue.h


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
			-DPREFIX="\"@prefix@\""		\
			$(TCL_FLAGS)			\
			$(SPECTRODAQ_FLAGS)

EXTRA_DIST=configtests.tcl outstagetests.tcl inputstagetests.tcl

installcheck-local:
	$(bindir)/assembler configtests.tcl
	$(bindir)/assembler outstagetests.tcl
	$(bindir)/assembler inputstagetests.tcl
