TCLPACKAGES=properties.tcl propertyEditor.tcl daqobject.tcl ringBuffer.tcl \
	ringBufferObject.tcl StateProgramData.tcl StateProgram.tcl	\
	ServiceData.tcl Service.tcl

IMAGES=program.png ringbuffer.png sysprogram.png

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/expeditor
	for f in $(TCLPACKAGES) ; do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/expeditor; done
	for f in $(IMAGES) ; do $(INSTALL_DATA) @srcdir@/$$f @prefix@/TclLibs/expeditor; done
	echo pkg_mkIndex -verbose @prefix@/TclLibs/expeditor "*.tcl" | @TCLSH_CMD@


TCLTESTS=properties.test proplist.test ringobject.test SPData.test svcdata.test

check-TESTS:
	for f in $(TCLTESTS); do TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ $$f; done


EXTRA_DIST = $(TCLPACKAGES) $(TCLTESTS) $(IMAGES)