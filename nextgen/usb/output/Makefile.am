lib_LTLIBRARIES			= libOutputStage.la

libOutputStage_la_SOURCES	= 	DataBuffer.cpp	\
					COutputThread.cpp

noinst_HEADERS			=	DataBuffer.h	\
					COutputThread.h


INCLUDES			=	-I@top_srcdir@/daq/format	\
					-I@top_srcdir@/base/dataflow	\
					-I@top_srcdir@/base/exception	\
					-I@top_srcdir@/base/headers	\
					-I@top_srcdir@/base/thread	\
					-I@top_srcdir@/usb/threadcom

libOutputStage_la_LIBADD	=	@top_srcdir@/daq/format/libdataformat.la	\
					@top_srcdir@/base/dataflow/libDataFlow.la	\
					@top_srcdir@/base/exception/libException.la	\
					@top_srcdir@/base/thread/libdaqthreads.la	\
					@top_srcdir@/usb/threadcom/libThreadComm.la