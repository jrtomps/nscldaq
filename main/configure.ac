#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)

AC_INIT(nscldaq,11.2-000, [scientificsoftware@nscl.msu.edu],[], [http://www.sourceforge.net/projects/nscldaq])


AC_CONFIG_SRCDIR([/base/dataflow/CRingBuffer.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AX_CXX_COMPILE_STDCXX_11([noext])

AC_ARG_ENABLE([docs],
              AS_HELP_STRING([--enable-docs],[Build the documentation]),
              [ENABLE_DOCS=yes],[ENABLE_DOCS=no])
if test "x$ENABLE_DOCS" == "xyes"
then

  AC_CHECK_PROG(HAVE_DOCBOOK2PDF, [docbook2pdf], [true], [false])
  if test "$HAVE_DOCBOOK2PDF" == "false"
  then
    AC_MSG_FAILURE([docbook2pdf is required to build documentation but was not found])
  else
    AC_SUBST(DOCBOOK2PDF,[docbook2pdf])
  fi

  AC_CHECK_PROG(HAVE_DOCBOOK2HTML, [docbook2html], [true], [false])
  if test "$HAVE_DOCBOOK2HTML" == "false"
  then
    AC_MSG_FAILURE([docbook2html is required to build documentation but was not found])
  else
    AC_SUBST(DOCBOOK2HTML,[docbook2html])
  fi

  AC_CHECK_PROG(HAVE_XMLTO, [xmlto], [true], [false])
  if test "$HAVE_XMLTO" == "false"
  then
    AC_MSG_FAILURE([xmlto is required to build documentation but was not found])
  else
    AC_SUBST(DOCBOOK2MAN,["xmlto man"])
  fi

  AC_CHECK_PROG(HAVE_MANDB, mandb, [true], [false])
  if test "$HAVE_MANDB" == "false"
  then
    AC_MSG_FAILURE([mandb is required to build documentation but was not found])
  else
    AC_SUBST(MANDB,[mandb])
  fi

  AM_CONDITIONAL([BUILD_DOCS],[true])
else
  AM_CONDITIONAL([BUILD_DOCS],[false])
fi

AC_CHECK_LIB([X11], [XSetWindowBackground])
AC_CHECK_LIB([Xt], [XtManage])

AM_PATH_CPPUNIT

# Checksums in eventlog require openssl for the digest libs:

AX_CHECK_OPENSSL([AC_MSG_RESULT([found openssl])],[AC_MSG_ERROR([OpenSSL is required but cannot be found])])


#
#   For compatibility with existing AC_Substs:

CPPUNIT_LDFLAGS="${CPPUNIT_LIBS}"

AC_SUBST([AM_CXXFLAGS], [-fno-strict-aliasing])

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


AX_CHECK_COMPILE_FLAG([-Wno-error=date-time], [NODATEWARN="-Wno-error=date-time"])
AC_SUBST(NODATEWARN)


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

AM_INIT_AUTOMAKE([foreign subdir-objects])

# Makefile exports:

SOVERSION="11:0:0"





#---------------------------------------------------------------------
#
#  Local tests:
#

# Ensure Python is 2.7 or greater, and the packages/dependencies we need

AM_PATH_PYTHON([2.7], [], [AC_MSG_ERROR([Python 2.7 or higher is needed for NSCLDAQ])])
AX_PYQT

# Ensure swig is installed: -- this is used to generate wrappers for the
#  VMUSB and CCUSB libraries for use within Tcl drivers.
#
#  Earlier swig versions than 1.3 may well work, however this is the
#  version I used when first successfully building the wrappers
# (ubuntu lucid lynx).
#
AX_PKG_SWIG([1.3], [], [ AC_MSG_ERROR([Swig is required to build nscldaq])])


# We use gengetopt in a few places.  Ideally it's been found by now in the
# path if not we'll need to do the withval thing.. or prompt the user to
# give us a --with-gengetopt-path
#
AX_GENGETOPT

# Tcl/Tk is pervasive in NCLDAQ so we need to have it too:

AX_TCL

#
#  The substs below are for compatibility sake 
#  eventually we should migrate Makefile.am defs to use
#  those in m4/tcl.m4
#
TCL_FLAGS="${TCL_CPPFLAGS} ${TK_CPPFLAGS}"

# Check for redundancy of libieee in the LD_LIBS
TCL_LDFLAGS=${TK_LIBS}
for tclname in ${TCL_LIBS}
do
    if test $tclname != "-lieee" 
    then 
        TCL_LDFLAGS="$TCL_LDFLAGS $tclname"
    fi
done
#TCL_LDFLAGS="${TK_LIBS} ${TCL_LIBS}"
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
	LIBEXCEPTION_LDFLAGS="-lException"
	LIBTCLPLUS_LDFLAGS="-ltclPlus ${LIBEXCEPTION_LDFLAGS}"
	AC_MSG_RESULT([Found in  system libraries])
else
  AC_CHECK_FILE([${prefix}/lib/libException.so],[libtclinstalled="yes"],[libtclinstalled="no"])
  AC_CHECK_FILE([${prefix}/lib/libtclPlus.so],[libexceptioninstalled="yes"],[libexceptioninstalled="no"])
  AC_CHECK_FILE([${prefix}/include/TCLInterpreterObject.h],[tclinterpobjinstalled="yes"],[tclinterpobjinstalled="no"])
  if [ [ $libtclinstalled = "no" ] || [ $libexceptioninstalled = "no" ] || [ $tclinterpobjinstalled = "no" ] ]
  then
    AC_CHECK_FILE([${srcdir}/libtcl], [havelibtcldir="yes"], [havelibtcldir="no"])
	  if test $havelibtcldir = "yes"
  	then
      AC_MSG_RESULT([Need to build local tagged copy])
      AC_CHECK_FILE([${srcdir}/libtcl/configure], [havelibtcldir="yes"], [havelibtcldir="no"])
		  (cd ${srcdir}/libtcl; ./configure --prefix=${prefix}; make clean install)
	  else
		  AC_MSG_ERROR([I seem to be missing libtcl, please use tcl++incorp.sh to fill it with a tagged release of libtclplus])
	  fi
  else
    AC_MSG_RESULT([Previously built version found in ${prefix}])
  fi
  LIBTCLPLUS_CFLAGS="-I${prefix}/include"
  LIBEXCEPTION_LDFLAGS="-L${prefix}/lib -lException -Wl,\"-rpath=${prefix}/lib\""
	LIBTCLPLUS_LDFLAGS="-L${prefix}/lib -ltclPlus ${LIBEXCEPTION_LDFLAGS}"

fi
AC_SUBST(LIBTCLPLUS_CFLAGS)
AC_SUBST(LIBTCLPLUS_LDFLAGS)
AC_SUBST(LIBEXCEPTION_LDFLAGS)


# Epics
#
#  This is only probed for and build if --enable-epics-tools=yes is provided
#  at configuration time.  Oncethat's done, AX_EPICS provideas all the defs
#  and the EPICS/BUILD_EPICS_TOOLS enables those in the Makefiles.
#


BUILD_EPICS_TOOLS=no
AC_ARG_ENABLE([epics-tools],
	AS_HELP_STRING([--enable-epics-tools=yes/no], 
	[Build software that requires EPICS (default no)]),
	[BUILD_EPICS_TOOLS="yes"])

if test "$BUILD_EPICS_TOOLS" == "yes"
then

	AX_EPICS
	AM_CONDITIONAL([EPICS], [true])
	AM_CONDITIONAL(BUILD_EPICS_TOOLS, [true])

else
	AM_CONDITIONAL([EPICS], [false])
	AM_CONDITIONAL(BUILD_EPICS_TOOLS, [false])

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
	AX_LIBUSB
	AM_CONDITIONAL([BUILD_USB], [true])
else
	AM_CONDITIONAL([BUILD_USB], [false])
fi

##
#  Check for zmq presence:
#

AC_SEARCH_LIBS([zmq_init], [zmq], [result=$ac_cv_search_zmq_init], [AC_MSG_ERROR([The zeromq library was not found and is now required for nscldaq])])
if test $result = "none required"
then
   ZMQ_LDFLAGS=""
else
   ZMQ_LDFLAGS=$result
fi

AC_SUBST(ZMQ_LDFLAGS)

##
# need the zmq c++ bindings
#  note in some c++11's we need to include <algorithm> to include zmq.hpp:
AC_LANG_PUSH(C++)
AC_CHECK_HEADERS([algorithm zmq.hpp],[], [AC_MSG_ERROR([The zeromq C++ header (zmq.hpp) is missing and is required for nscldaq])],
			    [#include <algorithm>])
AC_LANG_POP


##
# check for presence of Python zmq package

AC_PYTHON_MODULE([zmq])

#----------------------------------------------------------------------------

# Exports to the makefiles:

AC_DEFINE(__STDC_LIMIT_MACROS, [], [Needed to define UINT64_C])
AC_DEFINE(__STDC_CONSTANT_MACROS, [], [Needed to define UINT64_C])

AC_SUBST(SOVERSION)

AC_SUBST(TCL_FLAGS)
AC_SUBST(TCL_LDFLAGS)
AC_SUBST(TCLSH_CMD)






AC_SUBST(CPPUNIT_LDFLAGS)

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


#
# Ensure that resources for testing are accessible
# for out of tree builds
#
AC_CONFIG_LINKS([utilities/filter/run-0000-00.evt:utilities/filter/run-0000-00.evt])


#---------------------------------------------------------------------------
# Generate the following from their .in's (note that Automake takes
# *.am -> *.in.
#

# Note removed utilities/scalerdisplay in favor of utilities/newscaler
# Sept 25, 2014

AC_CONFIG_FILES([Makefile
		nscldaq
		daqsetup.bash
    base/Makefile
    base/os/Makefile
    base/CopyrightTools/Makefile
    base/bltsubst/Makefile
    base/cvt/Makefile
    base/thread/Makefile
    base/headers/Makefile
    base/security/Makefile
    base/tclwidgets/Makefile
    base/uri/Makefile
    base/dataflow/Makefile
    base/plotchart/Makefile
    base/plotcontainer/Makefile
    base/tcpip/Makefile
    base/tclutils/Makefile
    base/testutils/Makefile
    caenfw-common/Makefile
    servers/Makefile
    servers/tclserver/Makefile
    servers/portmanager/Makefile
    utilities/Makefile
    utilities/common/Makefile
    utilities/Buffer/Makefile
    utilities/daqstart/Makefile
    utilities/dvdburn/Makefile
    utilities/scalerdisplay/ScalerDisplay
    utilities/sequencer/Makefile
    utilities/ringselector/Makefile
    utilities/bufdump/Makefile
    utilities/eventlog/Makefile
    utilities/sclclient/Makefile
    utilities/tkbufdump/Makefile
    utilities/filter/Makefile
    utilities/filter/Makefile-template
    utilities/filter/FilterIncludes
    utilities/compatibility/Makefile
    utilities/compatibility/eventlog-compat
    utilities/compatibility/spectcldaq
    utilities/compatibility/s800toring
    utilities/compatibility/bufdump
    utilities/conversion/Makefile
    utilities/fileanalyzer/Makefile
    utilities/format/Makefile
    utilities/format/V8/Makefile
    utilities/format/V10/Makefile
    utilities/format/V11/Makefile
    utilities/IO/Makefile
    utilities/FormattedIO/Makefile
    utilities/newscaler/Makefile
    utilities/newscaler/ScalerDisplay.tcl
    utilities/scalersum/Makefile
    epics/chanlog/Makefile
    epics/controlpush/Makefile
    epics/epicsdisplay/Makefile
    epics/epicsdisplay/epicsdisplay
    epics/Makefile
    daq/Makefile
    daq/format/Makefile
    daq/readoutgui/Makefile
    daq/readoutgui/InstallRoot.tcl
    daq/readoutgui/ReadoutShell.tcl
    daq/s800callouts/Makefile
    daq/eventbuilder/Makefile
    daq/eventbuilder/Makefile_Skeleton
    daq/eventbuilder/startOrderer
    daq/evbtools/Makefile
    daq/evbtools/ringsource/Makefile
    daq/evbtools/teering/Makefile
    daq/evbtools/glom/Makefile
    daq/evbtools/unglom/Makefile
    daq/evbtools/offlineorderer/Makefile
    daq/evbtools/offlineorderer/figures/Makefile
    daq/actions/Makefile
    simplesetups/Makefile
    simplesetups/v775/Makefile
    simplesetups/v785/Makefile
    simplesetups/vmusb/Makefile
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
    sbs/caenfwloader/Makefile
    usb/Makefile
    usb/loaders/Makefile
    usb/vmusb/Makefile
    usb/vmusb/vmusb/Makefile 
    usb/vmusb/core/Makefile 
    usb/vmusb/ctlconfig/Makefile 
    usb/vmusb/daqconfig/Makefile  
    usb/vmusb/driverkit/Makefile 
    usb/vmusb/driverkit/Makefile-template
    usb/vmusb/tcldrivers/Makefile
    usb/vmusb/driverkit/VMUSBDriverIncludes
    usb/vmusb/controlpanels/Makefile 
    usb/vmusb/controlpanels/cv6533/Makefile
    usb/vmusb/controlpanels/figures/Makefile
    usb/vmusb/devtests/Makefile
    usb/vmusb/vmusbPackage/Makefile
    usb/vmusb/tclservertests/Makefile
    usb/ccusb/Makefile
    usb/ccusb/ccusb/Makefile 
    usb/ccusb/core/Makefile
    usb/ccusb/daqconfig/Makefile 
    usb/ccusb/ctlconfig/Makefile 
    usb/ccusb/ctlconfig/driver_Makefile
    usb/ccusb/tclpackage/Makefile
    usb/ccusb/tcldrivers/Makefile
    usb/ccusb/driverkit/Makefile
		 usb/ccusb/driverkit/Makefile-template
		 usb/ccusb/driverkit/CCUSBDriverIncludes
		 usb/ccusb/controlpanels/Makefile
		 usb/ccusb/devtests/Makefile
		 usb/ccusb/tclservertests/Makefile
		 usb/caenfwloader/Makefile
		 usb/common/Makefile
		 usb/common/tcldrivers/Makefile
		 usb/common/devices/Makefile
		 usb/common/configurableobject/Makefile
		 usb/common/slowcontrols/Makefile
		 usb/mesytec/Makefile
		 usb/mesytec/MCFD16/Makefile
		 usb/mesytec/MCFD16/figures/Makefile
		 usb/mesytec/MSCF16/Makefile
		 usb/mesytec/MSCF16/figures/Makefile
		 docbuild/Makefile
		 docbuild/images/Makefile
		 docbuild/images/callouts/Makefile
		 docconfig/Makefile])

AC_OUTPUT
