lib_LTLIBRARIES	=	libSBSDeviceSupport.la

libSBSDeviceSupport_la_SOURCES =	CADC2530.cpp		\
					CAENcard_767.cpp	\
					CAENcard.cpp		\
					CaenIO.cpp		\
					camacmap.cpp		\
					CBD8210.cpp		\
					CCAENChain.cpp		\
					CCAENV1x90.cpp		\
					CCAENV560.cpp		\
					CCAENV830.cpp		\
					CCAENV977.cpp		\
					CCamac.cpp		\
					CCamacModule.cpp	\
					CCamacNimout.cpp	\
					CCAMACScalerLRS2551.cpp	\
					CCAMACScalerLRS4434.cpp	\
					CCrateController.cpp	\
					CModule32.cpp	\
					CScaler.cpp	\
					CSIS3600.cpp	\
					CSIS3820.cpp	\
					CVMEScalerLRS1151.cpp	\
					Nimout.cpp	\
					sis3300.cpp	\
					v890.cpp


include_HEADERS =	CADC2530.h	\
			CAENcard_767.h	\
			CAENcard.h	\
			CaenIO.h	\
			camac.h		\
			camacmap.h	\
			CBD8210.h	\
			CCAENChain.h	\
			CCAENV1x90Data.h	\
			CCAENV1x90.h	\
			CCAENV1x90Opcodes.h	\
			CCAENV1x90Registers.h	\
			CCAENV560.h	\
			CCAENV560Registers.h	\
		 	CCAENV820Registers.h	\
			CCAENV830.h	\
			CCAENV977.h	\
			CCamac.h	\
			CCamacModule.h	\
			CCamacNimout.h	\
			CCAMACScalerLRS2551.h	\
			CCAMACScalerLRS4434.h	\
			CCrateController.h	\
			cescamac.h		\
			cesmacros.h	\
			CModule32.h	\
			CScaler.h	\
			CSIS3600.h	\
			CSIS3820.h	\
			CVMEScalerLRS1151.h	\
			macros.h	\
			Nimout.h	\
			sis3300.h	\
			v890.h		\
			wienercamac.h	\
			wienermacros.h

INCLUDES	=	-I@top_srcdir@/base/headers 	\
			-I@top_srcdir@/base/exception	\
			-I@top_srcdir@/sbs/nsclapi	\
			-I@top_srcdir@/sbs/driver/include


libSBSDeviceSupport_la_LIBADD =	@top_srcdir@/base/exception/libException.la	\
				@top_srcdir@/sbs/nsclapi/libSBSVmeAPI.la	\
				@top_srcdir@/sbs/driver/src/libbtp.la

libSBSDeviceSupport_la_LDFLAGS=	-version-info	1:0:0

