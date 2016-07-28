TCLPACKAGES=properties.tcl propertyEditor.tcl daqobject.tcl ringBuffer.tcl \
	ringBufferObject.tcl StateProgramData.tcl StateProgram.tcl	\
	ServiceData.tcl Service.tcl connector.tcl tool.tcl ObjectInstaller.tcl \
	toolbar.tcl ConnectorObject.tcl ConnectorInstaller.tcl eventBuilder.tcl \
	eventBuilderObject.tcl dataSource.tcl dataSourceObject.tcl \
	svcSerializer.tcl ringSerializer.tcl stateProgramSerializer.tcl \
	evbSerializer.tcl dsourceSerializer.tcl \
	svcValidator.tcl ringValidator.tcl stateProgramValidator.tcl \
	eventBuilderValidator.tcl dataSourceValidator.tcl



IMAGES=program.png ringbuffer.png sysprogram.png arrow.png eventBuilder.png dsource.png 
DOC_IMAGES=propertyeditor.png expEditorinitial.jpg


install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/expeditor
	for f in $(TCLPACKAGES) ; do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/expeditor; done
	for f in $(IMAGES) ; do $(INSTALL_DATA) @srcdir@/$$f @prefix@/TclLibs/expeditor; done
	echo pkg_mkIndex -verbose @prefix@/TclLibs/expeditor "*.tcl" | @TCLSH_CMD@
	$(INSTALL_PROGRAM) @srcdir@/expEditor.tcl @bindir@/expEditor
	$(mkinstalldirs) @prefix@/share/html
	$(INSTALL_DATA) $(IMAGES) $(DOC_IMAGES) @prefix@/share/html



TCLTESTS=properties.test proplist.test ringobject.test SPData.test svcdata.test \
	eventbuilder.test datasource.test

TEST_EXTENSIONS=.test .py
TEST_LOG_COMPILER=@TCLSH@
PY_LOG_COMPILER=@PYTHON@
TESTS_ENVIRONMENT=TCLLIBPATH=@prefix@/TclLibs; export TCLLIBPATH; \
	PYTHONPATH=@prefix@/pythonLibs; export PYTHONPATH;

TESTS = $(TCLTESTS)



EXTRA_DIST = $(TCLPACKAGES) $(TCLTESTS) $(IMAGES) $(DOC_IMAGES) expEditor.tcl
