lib_LTLIBRARIES = libTclPlus.la
libTclPlus_la_SOURCES = TCLApplication.cpp TCLCommandPackage.cpp \
	TCLException.cpp TCLFileHandler.cpp TCLIdleProcess.cpp \
	TCLInterpreter.cpp TCLInterpreterObject.cpp TCLList.cpp \
	TCLObject.cpp TCLProcessor.cpp TCLResult.cpp TCLString.cpp \
	TCLTimer.cpp TCLVariable.cpp

libTclPlus_la_LDFLAGS = -version-info 1:0:0

include_HEADERS = TCLApplication.h TCLCommandPackage.h TCLException.h \
	TCLFileHandler.h TCLHashTable.h TCLHashTableItem.h \
	TCLHashTableIterator.h TCLIdleProcess.h TCLInterpreter.h \
	TCLInterpreterObject.h TCLList.h TCLObject.h TCLPackagedCommand.h \
	TCLProcessor.h TCLResult.h TCLString.h TCLTimer.h TCLVariable.h

INCLUDES = -I@top_srcdir@/Headers -I@top_srcdir@/Exception $(TCL_FLAGS)
