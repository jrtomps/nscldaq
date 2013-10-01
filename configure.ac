#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(nscldaq, 11.0, fox@nscl.msu.edu)

AC_CONFIG_SRCDIR([/utilities/StringsToIntegers.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL


AC_CHECK_PROG(HAVE_GENGETOPT, [gengetopt], true, false)
AC_CHECK_PROG(HAVE_DOCBOOK2PDF, [docbook2pdf], true, false)
AC_CHECK_PROG(HAVE_XMLTO, [xmlto], true, false)
AC_CHECK_PROG(MANDB, mandb, mandb, echo)
AC_CHECK_PROG(SWIG, [swig], [swig], [])

AC_CHECK_LIB([X11], [XSetWindowBackground])
AC_CHECK_LIB([Xt], [XtManage])
# AC_CHECK_LIB([cppunit], [_ZN7CppUnit12XmlOutputterD1Ev], [CPPUNIT_LDFLAGS=-lcppunit])


AM_PATH_CPPUNIT

#
#   For compatibility with existing AC_Substs:

CPPUNIT_LDFLAGS="${CPPUNIT_LIBS}"


AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# check to find the size of address
AC_CHECK_SIZEOF([void*])
AS_IF([test "$ac_cv_sizeof_voidp" -eq 8],
    AC_DEFINE([ADDR64],1,[Specifies that memory addresses are 64-bits ]),
     [])
   

# Checks for library functions.

AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 gethostbyaddr gethostbyname gettimeofday inet_ntoa memmove memset regcomp socket strchr strcspn strdup strerror strspn strtol])

AC_CHECK_FUNCS([ftruncate munmap])
AC_FUNC_MMAP
AC_FUNC_STAT


# Automake:

AM_INIT_AUTOMAKE([foreign])

# Makefile exports:

SOVERSION="10:0:0"





#---------------------------------------------------------------------
#
#  Local tests:
#
# Ensure swig is installed:

if test x$SWIG == x
then
	AC_MSG_ERROR([Can't find SWIG in your program search path.])
fi

# We use gengetopt in a few places.  Ideally it's been found by now in the
# path if not we'll need to do the withval thing.. or prompt the user to
# give us a --with-gengetopt-path
#

if test x$HAVE_GENGETOPT == xtrue
then
	GENGETOPT=gengetopt
else
	AC_ARG_WITH(gengetopt-path,
			[ --with-gengetopt-path=directory that has gengetopt],
			[GENGETOPT=$withval],
			GENGETOPT="")
  if test x$GENGETOPT == x
  then
	AC_MSG_ERROR([Can't find gengetopt in path, install it or help me out with --with-gengetopt-path])
  fi
fi


AX_TCL

#
#  The substs below are for compatibility sake 
#  eventually we should migrate Makefile.am defs to use
#  those in m4/tcl.m4
#
TCL_FLAGS="${TCL_CPPFLAGS} ${TK_CPPFLAGS}"
TCL_LDFLAGS="${TK_LIBS} ${TCL_LIBS}"
TCLSH_CMD=${TCLSH} 



#
#  IF systemtcl is yes we're pretty much done.. otherwise
#  we need to configure/build/install libtcl into prefix and then set
#  the flags:

AC_MSG_CHECKING([libtcl++ installed as package (e.g. .deb)])

AC_CHECK_FILE([/usr/include/TCLInterpreterObject.h], [systemtcl="yes"], [systemtcl="no"])

if test $systemtcl = "yes"
then
	LIBTCLPLUS_CFLAGS=""
	LIBTCLPLUS_LDFLAGS="-ltclPlus -lException"
	AC_MSG_RESULT([Found in  system libraries])
else
	AC_MSG_RESULT([Need to build local tagged copy])
	AC_CHECK_FILE([./libtcl/configure], [havelibtcldir="yes"], [havelibtcldir="no"])
	if test $havelibtcldir = "yes"
	then
		(cd libtcl; ./configure --prefix=${prefix}; make clean install)
		LIBTCLPLUS_CFLAGS="-I${prefix}/include"
		LIBTCLPLUS_LDFLAGS=-"L${prefix}/lib -ltclPlus -lException -Wl,\"-rpath=${prefix}/lib\""
	else
		AC_MSG_ERROR([I seem to be missing libtcl, please use tcl++incorp.sh to fill it with a tagged release of libtclplus])
	fi

fi
AC_SUBST(LIBTCLPLUS_CFLAGS)
AC_SUBST(LIBTCLPLUS_LDFLAGS)



# Epics:

#  Requires --enable-epics-tools=yes
#   Look for epics in:
#    /usr/local/epics
#    /opt/epics
#    /soft/intel/epics   [nscl].
#    --with-epics-rootdir=
# Underneath that we have (we require intel linux remember):
#    include                  - the include directory
#    include/os/linux-x86     - os specific includes.
#    lib/linux-x86            - libs
#    bin/linux-x86/caRepeater - The caRepeater program.
#

BUILD_EPICS_TOOLS=no
AC_ARG_ENABLE([epics-tools],
	AS_HELP_STRING([--enable-epics-tools=yes/no], 
	[Build software that requires EPICS (default no)]),
	[BUILD_EPICS_TOOLS="yes"])

if test "$BUILD_EPICS_TOOLS" == "yes"
then

epicstestdirs="/usr/local/epics /opt/epics /usr/lib/epics"
haveepics="no"


AC_MSG_CHECKING([ for epics includes, libs and binaries])

#  Check for --with-epics-rootdir....

AC_ARG_WITH(epics-rootdir,
	[  --with-epics-rootdir=top level directory of EPICS install (requires --enable-epics-tools)],
	[haveepics="yes"
	 epicsroot="$withval"],
	[haveepics="no"])

#
#  Figure out which architecture specific directory to use.
#
#   For now we're just going to support various types of linux:
#   linux-x86_64 (64 bit linux) and linxu-x86 (32 bit linux).
#   Later this can be expanded to other os's and archs.
#
bits=`arch`
kernel=`uname |tr [A-Z] [a-z]`

case $bits in
   i686 | i386)
   	bits=x86
   ;;
esac

epicsarch=$kernel-$bits


if test $haveepics == "no"
then
   for dir in $epicstestdirs
   do
      if test $haveepics == "no"
      then
        AC_CHECK_FILE([${dir}/include/cadef.h],
	              [haveepics="yes"
                       epicsroot=$dir])
      fi
   done
fi

#  If we found epics we can build the software that needs it.
#  otherwise it's optional.

if test $haveepics == "yes"
then
	# Have epics:

   epics=true
   EPICS_INCLUDES="-I$epicsroot/include -I$epicsroot/include/os/Linux"
   EPICS_LDFLAGS="-L$epicsroot/lib/$epicsarch -lca -Wl,\"-rpath=$epicsroot/lib/$epicsarch\""
   EPICS_BIN="-DEPICS_BIN=\"\\\"$epicsroot/bin/$epicsarch\\\"\""
else
   epics=false
fi

fi

#
#  The SBS software will only be built if
#  --enable-sbs=kernel-source-dir was supplied.
#  In that case, the configured kernel source is assumed to be in 
#  kernel-source-dir and that will be used to compile the device driver
#  as well.  
#  The default is to _not_ build the sbs stuff.
#

BUILD_SBS="no"

AC_ARG_ENABLE([sbs], 
	      AS_HELP_STRING([--enable-sbs=kernel-source-dir], [Build SBS software using kernel-source-dir as directory with configured kernel source]),
	      [BUILD_SBS="yes"
	       KERNEL_SOURCE_DIR="$enableval"])




if test "$BUILD_SBS" == "yes"
then
  echo Building SBS software with kernel source at: $KERNEL_SOURCE_DIR
else
  echo SBS software support not selected!
fi


#
#  If --enable-usb is turned on, we build the VM/CC-usb support directories
#
enableusb="no"
AC_ARG_ENABLE([usb],
	AS_HELP_STRING([--enable-usb], [Build support for CC/VM usb data taking]),
	[enableusb="yes"])

#  If building is turned on we need to locate the headers and the includes.


if test "$enableusb" == "yes"
then
	headerdirs="/usr/include /usr/local/include"
	libdirs="/lib /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu"

	usbheaderdir=""
	usblibdir=""

	AC_MSG_CHECKING([for libusb headers])
	AC_ARG_WITH(usb-headerdir,
		[   --with-usb-headerdir=Path to usb.h],
		[usbheaderdir=$withval],[])

	if test "$usbheaderdir" == ""
	then
	   	for dir in $headerdirs
   		do
			if test "$usbheaderdir" == ""
      			then
				AC_CHECK_FILE([$dir/usb.h],
					[usbheaderdir=$dir])
			fi
   		done
	fi

	if test "$usbheaderdir" != ""
	then
		USBSWITCHES="-I${usbheaderdir}"
		AC_MSG_RESULT([found in $usbheaderdir])
	else
		AC_MSG_ERROR([Can't find usb.h install libusb development or try using --with-usb-header-dir to help me find it])
	fi

	AC_MSG_CHECKING([for libusb library]) 
	AC_ARG_WITH(usb-libdir,
		[   --with-usb-libdir=Path to libusb.a],
		[usblibdir=$withval],[])

	if test "$usblibdir" == ""
	then	
   		for dir in $libdirs
   		do	
      			if test "$usblibdir" == ""
      			then
				AC_CHECK_FILE([$dir/libusb.a],
					[usblibdir=$dir])
      			fi
   		done
	fi

	if test "$usblibdir" != "" 
	then
		USB_LIBS="-L${usblibdir} -lusb"
  		 AC_MSG_RESULT([found in $usblibdir])
	else
		AC_MSG_ERROR([Can't find libusb.a install libusb or try using --with-usb-libdir to tell me where to find it])
	fi 
	#  If we got this far the symbos can be exported to the makefiles:

	AC_SUBST(USBSWITCHES)
	AC_SUBST(USB_LIBS)
fi


#----------------------------------------------------------------------------

# Exports to the makefiles:

AC_DEFINE(__STDC_LIMIT_MACROS, [], [Needed to define UINT64_C])
AC_DEFINE(__STDC_CONSTANT_MACROS, [], [Needed to define UINT64_C])

AC_SUBST(SOVERSION)
AC_SUBST(MANDB)

AC_SUBST(TCL_FLAGS)
AC_SUBST(TCL_LDFLAGS)
AC_SUBST(TCLSH_CMD)

echo "Epics: " x$haveepics

AM_CONDITIONAL([EPICS], [test x$haveepics = xyes])
AM_CONDITIONAL(BUILD_EPICS_TOOLS, [test x$BUILD_EPICS_TOOLS = xyes])

AC_SUBST(EPICS_INCLUDES)
AC_SUBST(EPICS_LDFLAGS)
AC_SUBST(EPICS_BIN)


AC_SUBST(GENGETOPT)

AC_SUBST(CPPUNIT_LDFLAGS)

AM_CONDITIONAL([PDFDOCS], [test x$HAVE_DOCBOOK2PDF = xtrue])
AM_CONDITIONAL([XMLMANS], [test x$HAVE_XMLTO = xtrue])
AM_CONDITIONAL([BUILD_USB], [test x$enableusb = xyes])

#
#  Threading is hard coded to -pthread now.
#
THREADCXX_FLAGS="-pthread"
THREADC_FLAGS="-pthread"
THREADLD_FLAGS="-lpthread -lrt"

AC_SUBST(THREADCXX_FLAGS)
AC_SUBST(THREADC_FLAGS)
AC_SUBST(THREADLD_FLAGS)

# SBS stuff:

AC_SUBST(KERNEL_SOURCE_DIR)
AM_CONDITIONAL([BUILD_SBS_DRIVER], [test "$BUILD_SBS" == "yes"])




#---------------------------------------------------------------------------
# Generate the following from their .in's (note that Automake takes
# *.am -> *.in.
#

AC_CONFIG_FILES([Makefile
		base/Makefile
		base/os/Makefile
                 base/CopyrightTools/Makefile
                 base/bltsubst/Makefile
                 base/cvt/Makefile
                 base/exception/Makefile
		 base/thread/Makefile
                 base/headers/Makefile
                 base/security/Makefile
                 base/tclplus/Makefile
                 base/tclwidgets/Makefile
                 base/uri/Makefile
		 base/dataflow/Makefile
                 base/plotchart/Makefile
		 base/tcpip/Makefile
		 servers/Makefile
		 servers/tclserver/Makefile
                 servers/portmanager/Makefile
		 utilities/Makefile
                 utilities/daqstart/Makefile
                 utilities/dvdburn/Makefile
                 utilities/scalerdisplay/Makefile 
		 utilities/scalerdisplay/ScalerDisplay
                 utilities/sequencer/Makefile
		 utilities/ringselector/Makefile
		 utilities/bufdump/Makefile
		 utilities/eventlog/Makefile
		 utilities/sclclient/Makefile
		 utilities/tkbufdump/Makefile
		 utilities/compatibility/Makefile
		 utilities/compatibility/eventlog-compat
		 utilities/compatibility/spectcldaq
		 utilities/compatibility/s800toring
                 epics/chanlog/Makefile
                 epics/controlpush/Makefile
                 epics/epicsdisplay/Makefile
		 epics/epicsdisplay/epicsdisplay
		 epics/Makefile
		 daq/Makefile
		 daq/format/Makefile
		 daq/readoutgui/Makefile
		 daq/readoutgui/InstallRoot.tcl
		 daq/s800callouts/Makefile
		 daq/eventbuilder/Makefile daq/eventbuilder/Makefile_Skeleton
		 daq/eventbuilder/ringsource/Makefile
		 daq/eventbuilder/teering/Makefile
		 daq/eventbuilder/glom/Makefile
		 daq/eventbuilder/unglom/Makefile
		 daq/eventbuilder/startOrderer
		 sbs/Makefile
		 sbs/driver/Makefile
		 sbs/driver/src/Makefile
		 sbs/driver/include/Makefile
		 sbs/driver/dd/GNUmakefile
		 sbs/nsclapi/Makefile
		 sbs/tclpackage/Makefile
		 sbs/puretcl/Makefile
		 sbs/vmemodules/Makefile
		 sbs/readout/Makefile
		 sbs/readout/UserMakefile
		 sbs/readout/SBSRdoMakeIncludes
		 sbs/slowcontrols/Makefile
		 sbs/slowcontrols/vhq/Makefile
		 sbs/slowcontrols/caenv812/Makefile
		 sbs/slowcontrols/n568b/Makefile
		 sbs/slowcontrols/bcnaf/Makefile
		 sbs/slowcontrols/vhs/Makefile
		 sbs/scripted/Makefile
		 sbs/VmeDevice/Makefile
		 usb/Makefile
                 usb/loaders/Makefile
		 usb/vmusb/Makefile
		 usb/vmusb/vmusb/Makefile usb/vmusb/devices/Makefile 
		 usb/vmusb/threadcomm/Makefile usb/vmusb/router/Makefile
		 usb/vmusb/rdothread/Makefile usb/vmusb/tclserver/Makefile 
		 usb/vmusb/daqconfig/Makefile  usb/vmusb/commands/Makefile
		 usb/vmusb/driverkit/Makefile usb/vmusb/driverkit/Makefile-template
		 usb/vmusb/tcldrivers/Makefile
		 usb/vmusb/driverkit/VMUSBDriverIncludes
                 usb/vmusb/controlpanels/Makefile usb/vmusb/controlpanels/cv6533/Makefile
		 usb/ccusb/Makefile
		 usb/ccusb/ccusb/Makefile usb/ccusb/devices/Makefile 
		 usb/ccusb/threadcomm/Makefile usb/ccusb/router/Makefile
		 usb/ccusb/rdothread/Makefile usb/ccusb/tclserver/Makefile 
		 usb/ccusb/daqconfig/Makefile  usb/ccusb/commands/Makefile
		 usb/ccusb/driverkit/Makefile
		 usb/ccusb/driverkit/Makefile-template
		 usb/ccusb/driverkit/CCUSBDriverIncludes
                 usb/ccusb/controlpanels/Makefile
		 docbuild/Makefile
		 docconfig/Makefile])

AC_OUTPUT

