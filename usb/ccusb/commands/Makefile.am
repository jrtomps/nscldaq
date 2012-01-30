lib_LTLIBRARIES		= libCommands.la
libCommands_la_SOURCES	=	CBeginRun.cpp	\
				CEndRun.cpp	\
				CPauseRun.cpp	\
				CResumeRun.cpp	\
				tclUtil.cpp	

include_HEADERS		= 	CBeginRun.h	\
				CEndRun.h	\
				CPauseRun.h	\
				CResumeRun.h	\
				tclUtil.h

INCLUDES		= 	-I@top_srcdir@/threadcomm	\
			  	-I@top_srcdir@/rdothread	\
				-I@top_srcdir@/router		\
				-I@top_srcdir@/daqconfig	\
				$(TCL_FLAGS)

libCommands_la_LIBADD	= 	@top_srcdir@/rdothread/libRdoThread.la	\
				@top_srcdir@/threadcomm/libThreadComm.la	\
				@top_srcdir@/router/libRouter.la	\
				@top_srcdir@/daqconfig/libConfig.la	\
				-lTclPlus -lException	\
				$(TCL_LDFLAGS)