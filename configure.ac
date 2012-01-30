#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(nscldaq, 10.1, fox@nscl.msu.edu)
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

AC_CHECK_LIB([X11], [XSetWindowBackground])
AC_CHECK_LIB([Xt], [XtManage])
AC_CHECK_LIB([cppunit], [_ZN7CppUnit12XmlOutputterD1Ev], [CPPUNIT_LDFLAGS=-lcppunit])
AC_CHECK_LIB([pub], [xrealloc], [])





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

# Checks for library functions.

AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 gethostbyaddr gethostbyname gettimeofday inet_ntoa memmove memset regcomp socket strchr strcspn strdup strerror strspn strtol])

AC_CHECK_FUNCS([ftruncate munmap])
AC_FUNC_MMAP
AC_FUNC_STAT

# Automake:

AM_INIT_AUTOMAKE([])

# Makefile exports:

SOVERSION="10:0:0"





#---------------------------------------------------------------------
#
#  Local tests:
#

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
	AC_MSG_ERROR([Can't find gengetop in path, install it or help me out with --with-gengetopt-path])
  fi
fi



# Locate Tcl/Tk and setup compile/link switches for them too:
# The list below is the set of versions we will look for.  At this
# time, 8.4 is pretty standard and 8.5 is impending which means that
# soon some redhat systems will have it I suppose.
# This list must be updated as time goes by.
#
tcl_versions="8.5 8.4 8.3 8.2 8.1 8.0"     # First match;  so order new -> old.

#  The TCL Shell - used to run tests... must be in path.
#  First check for a version qualified command:
#

AC_MSG_CHECKING([for tcl shell command...])
for version in $tcl_versions
do
	name=tclsh${version}
	AC_CHECK_PROG(TCLSH_CMD, $name, $name)
done
#
#   Could be unqualified by version:
#
AC_CHECK_PROG(TCLSH_CMD, tclsh, tclsh)

if test "$TCLSH_CMD" == ""
then
	AC_MSG_ERROR([Can't find a tcl shell])
else
	AC_MSG_RESULT([found: $TCLSH_CMD])
fi

#  TCL headers:
#    We know of a few places the headers can live:
#       /usr/include              redhat linux.
#       /usr/include/tcl          debian dude that made  link to default tcl.
#       /usr/include/tcl<version> debian linux
#       /usr/local/include        default for source based install.
#       Whatever the user specifies. 
#        For now, the versions we look for are determined by the
#        variable tcl_versions  that variable is a list of version 
#        number specifiers.
#


tcl_header_dir=""
AC_MSG_CHECKING([for tcl headers])

AC_ARG_WITH(tcl-header-dir,
	[  --with-tcl-header-dir=path Path to tcl headers],
	[tcl_header_dir="$withval"
	tcl_headers_found="yes"],[tcl_headers_found="no"])


	# /usr/include

if test $tcl_headers_found == "no"; then
   AC_CHECK_FILE([/usr/include/tcl.h],
	[tcl_header_dir="/usr/include"
	tcl_headers_found="yes"])
fi

	# /usr/include/tcl

if test $tcl_headers_found == "no"; then
   AC_CHECK_FILE([/usr/include/tcl/tcl.h],
	[tcl_header_dir="/usr/include/tcl"
	tcl_headers_found="yes"])
fi

	# /usr/include/tcl<version>

if test $tcl_headers_found == "no" ; then
 for version in $tcl_versions;  do
   if test $tcl_headers_found == "no" ; then   # stop at first match
      testdir=/usr/include/tcl$version
      AC_CHECK_FILE([${testdir}/tcl.h],
	[tcl_header_dir=${testdir}
	 tcl_headers_found="yes"])
   fi
 done
fi

      # /usr/local/include
if test $tcl_headers_found == "no"; then
   AC_CHECK_FILE([/usr/local/include/tcl.h],
	[tcl_header_dir="/usr/local/include/tcl"
	tcl_headers_found="yes"])
fi


if test $tcl_headers_found == "yes"; then
   TCL_FLAGS="-I${tcl_header_dir}"
   AC_MSG_RESULT([found in $tcl_header_dir])
else
   AC_MSG_ERROR([can't find tcl.h try using --with-tcl-header-dir to help me])
fi

#  Tcl libraries:
#     We look for the following:
#      /usr/lib/libtcl.a           - Usually a link to a default lib.
#      /usr/lib/libtcl<version>.a  - version of tcl from tcl_versions
#      /usr/local/lib/libtcl.a     - As above but installed from source.
#      /usr/local/lib<version>.a   - As Above but installed from source.
#      If --with-tcl-libdir=dir      is supplied, that directory is 
#                                    searched first.
# ASSUMPTIONS:  libtcl and libtk will have the same naming conventions.
#

tcl_libsearchpath=""
tcl_libfound="no"
AC_MSG_CHECKING([Tcl/Tk library to use])

	# Does the user want us to look somewhere in particular:

AC_ARG_WITH(tcl-libdir,
	[  --with-tcl-libdir=dir Specify where the tcl libraries live.],
	[tcl_libsearchpath=$withval])

tcl_libsearchpath="$tcl_libsearchpath /usr/lib /usr/local/lib"

tcl_libfiles=libtcl.a

for v in ${tcl_versions}
do
   tcl_libfiles="$tcl_libfiles libtcl${v}.a"
   tcl_libfiles="$tcl_libfiles libtcl${v}.so"
done


for dir in ${tcl_libsearchpath} 
do
  for file in ${tcl_libfiles}
  do
    fname=$dir/$file
    if test $tcl_libfound == "no" 
    then
      AC_CHECK_FILE($fname,
	[tcl_libdir=$dir
         tcl_libname=$file
	 tcl_libfound="yes"
	])
    fi
  done
done

if test $tcl_libfound == "yes"
then
  # Find the lib basename:

  tcl_libname=$(echo $tcl_libname|cut -c 4- )  # trim off lib 
  #
  #  Use basename to trim of the .a or .so:
  #

  tcl_libname=$(basename $(basename $tcl_libname .a) .so)
  echo tcl libname $tcl_libname



  tk_libname=tk$(echo $tcl_libname|cut -c 4-)   # trim off tc...and use tk

  TCL_LDFLAGS="-L${tcl_libdir} -l${tk_libname}  -l${tcl_libname}"

else
  AC_MSG_ERROR([Unable to find the tcl library, help me out with --with-tcl-libdir])
fi


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

epicstestdirs="/usr/local/epics /opt/epics /soft/intel/epics"
haveepics="no"


AC_MSG_CHECKING([ for epics includes, libs and binaries])

#  Check for --with-epics-rootdir....

AC_ARG_WITH(epics-rootdir,
	[  --with-epics-rootdir=top level directory of EPICS install (requires --enable-epics-tools)],
	[haveepics="yes"
	 epicsroot="$withval"],
	[haveepics="no"])



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
   EPICS_LDFLAGS="-L$epicsroot/lib/linux-x86 -lca -Wl,\"-rpath=$epicsroot/lib/linux-x86\""
   EPICS_BIN="-DEPICS_BIN=\"\\\"$epicsroot/bin/linux-x86\\\"\""
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
	libdirs="/lib /usr/lib /usr/local/lib"

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
                 base/CopyrightTools/Makefile
                 base/cvt/Makefile
                 base/exception/Makefile
		 base/thread/Makefile
                 base/headers/Makefile
                 base/security/Makefile
                 base/tclplus/Makefile
                 base/tclwidgets/Makefile
                 base/uri/Makefile
		 base/dataflow/Makefile
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
                 epics/chanlog/Makefile
                 epics/controlpush/Makefile
                 epics/epicsdisplay/Makefile
		 epics/epicsdisplay/epicsdisplay
		 epics/Makefile
		 daq/Makefile
		 daq/format/Makefile
		 daq/readoutgui/Makefile
		 daq/readoutgui/InstallRoot.tcl
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
		 usb/vmusb/Makefile
		 usb/vmusb/vmusb/Makefile usb/vmusb/devices/Makefile 
		 usb/vmusb/threadcomm/Makefile usb/vmusb/router/Makefile
		 usb/vmusb/rdothread/Makefile usb/vmusb/tclserver/Makefile 
		 usb/vmusb/daqconfig/Makefile  usb/vmusb/commands/Makefile
		 usb/ccusb/Makefile
		 usb/ccusb/ccusb/Makefile usb/ccusb/devices/Makefile 
		 usb/ccusb/threadcomm/Makefile usb/ccusb/router/Makefile
		 usb/ccusb/rdothread/Makefile usb/ccusb/tclserver/Makefile 
		 usb/ccusb/daqconfig/Makefile  usb/ccusb/commands/Makefile
		 docbuild/Makefile
		 docconfig/Makefile])

AC_OUTPUT

