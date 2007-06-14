bin_PROGRAMS = assembler

assembler_SOURCES =	AssemblerApplication.cpp	\
			AssemblerCommand.cpp		\
			AssemblerErrors.cpp		\
			AssembledEvent.cpp		\
			AssembledPhysicsEvent.cpp	\
			AssembledScalerEvent.cpp	\
			AssembledStateTransitionEvent.cpp

noinst_HEADERS = 	AssemblerApplication.h		\
			AssemblerCommand.h		\
			AssemblerErrors.h		\
			AssembledEvent.h		\
			AssembledPhysicsEvent.h		\
			AssembledScalerEvent.h		\
			AssembledStateTransitionEvent.h	


assembler_LDADD =	@top_srcdir@/TCL/libtclPlus.la	\
			@top_srcdir@/Exception/libException.la

assembler_LDFLAGS = 	$(TCL_LDFLAGS)			\
			-Wl,"-rpath=$(libdir)"

INCLUDES	= 	-I@top_srcdir@/TCL		\
			-I@top_srcdir@/Exception	\
			-I@top_srcdir@/Headers		\
			$(TCL_FLAGS)

EXTRA_DIST=configtests.tcl

installcheck-local:
	$(bindir)/assembler configtests.tcl
