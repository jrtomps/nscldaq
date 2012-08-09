#
bin_PROGRAMS = CHIP

TCLDEST = @prefix@/TclLibs/CHIP

DATADEST= @sysconfdir@/CHIP


AM_CXXFLAGS=-DCESCAMAC -DVME -DINSTDIR=\"@prefix@\"

CHIP_SOURCES = ReadoutMain.cpp TCLThread.cpp XLM.cpp AVmeModule.cpp chip_cont.cpp chip_cont_wrap.cxx 


CHIP_LDADD = @top_builddir@/usb/vmusb/vmusb/libVMUSBRemote.la \
	@top_builddir@/base/tclplus/libtclPlus.la \
	@top_builddir@/base/exception/libException.la \
	@TCL_LDFLAGS@  @THREADLD_FLAGS@

noinst_HEADERS = chip_cont.h constants.h HiRA_readout.h TCLThread.h XLM.h \
	AVmeModule.h

INCLUDES = -I@top_srcdir@/usb/vmusb/devices \
	-I@top_srcdir@/usb/vmusb/vmusb      \
	-I@top_srcdir@/base/headers	\
	-I@top_srcdir@/sbs/vmemodules   \
	-I@top_srcdir@/sbs/nsclapi	\
	-I@top_srcdir@/sbs/driver/include \
	-I@top_srcdir@/base/tclplus  \
	-I@top_srcdir@/base/exception \
	@TCL_FLAGS@ @THREADCXX_FLAGS@



BUILT_SOURCES=chip_cont_wrap.cxx

TCL_SCRIPTFILES=chip_controller.tcl \
loader_831.tcl \
loader.tcl \
loadlv1.tcl \
mbcont.tcl \
mclistbox.tcl \
motherboard1.tcl \
newloader.tcl \
notebook.tcl \
silstrip.tcl \
tabnbook.tcl \
xlmint.tcl

FIRMWARE=xlmxxv_2mb_rev502.bit xlmxxv_rev502.bit

DATAFILES=config2.dat config.dat configXXV1.dat configXXV2.dat configXXV.dat

EXTRA_DIST=$(TCL_SCRIPTFILES) $(FIRMWARE) $(DATAFILES)

install-exec-local:
	$(mkinstalldirs) $(TCLDEST)
	$(mkinstalldirs) $(DATADEST)
	$(INSTALL_SCRIPT) $(TCL_SCRIPTFILES) $(TCLDEST)
	$(INSTALL_DATA) $(FIRMWARE) $(DATAFILES)  $(DATADEST)


chip_cont_wrap.cxx: chip_cont.swi
	swig -tcl -c++ chip_cont.swi


