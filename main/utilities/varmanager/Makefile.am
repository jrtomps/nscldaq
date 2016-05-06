#  Programs: initticl is a tclsh that 
#  - requires a script ?args?
#  - Does not exit at EOF on the script.
#



bin_PROGRAMS=inittcl

inittcl_SOURCES=inittcl.c
inittcl_CPPFLAGS=@TCL_CPPFLAGS@
inittcl_LDFLAGS=@TCL_LDFLAGS@

# Shared libraries:

lib_LTLIBRARIES = libvardb.la \
	libvardbtcl.la \
	libvardbpy.la libvardirtreepy.la libvariablepy.la libenumpy.la \
	libstatemachinepy.la

#  Base C++ implementation

libvardb_la_SOURCES=CVariableDb.cpp CVarDirTree.cpp CTypeFactory.cpp \
	CVariable.cpp CIntegerType.cpp CRealType.cpp CStringType.cpp \
	CDataTypeCreatorBase.cpp CIntegerTypeCreator.cpp CRealTypeCreator.cpp \
	CStringTypeCreator.cpp CEnumType.cpp CEnumeration.cpp CEnumTypeCreator.cpp \
	CEnumTypeFamilyHandler.cpp CStateMachineType.cpp CStateMachineTypeCreator.cpp \
	CStateMachineTypeFamilyHandler.cpp CStateMachine.cpp

libvardb_la_CPPFLAGS = @SQLITE3_CFLAGS@ -I@top_srcdir@/base/sqlite++ -I@top_srcdir@/base/factories
libvardb_la_LDFLAGS  = 	@top_builddir@/base/sqlite++/libsqlite3pp.la  @SQLITE3_LDFLAGS@ 

include_HEADERS = CVariableDb.h CVarDirTree.h CVariable.h CTypeFactory.h \
	CDataType.h CIntegerType.h CRealType.h CStringType.h \
	CDataTypeCreatorBase.h CIntegerTypeCreator.h CRealTypeCreator.h \
	CStringTypeCreator.h CEnumType.h CEnumeration.h CUnknownTypeHandler.h \
	CEnumTypeCreator.h CEnumTypeFamilyHandler.h CStateMachineType.h \
	CStateMachineTypeCreator.h CStateMachineTypeFamilyHandler.h	\
	CStateMachine.h


#  Tcl bindings

libvardbtcl_la_SOURCES=VarDbTclPackage.cpp \
	CVarDbCreateCommand.cpp CVarDbCreateCommand.h \
	CVarDbOpenCommand.cpp   CVarDbOpenCommand.h	\
	CVarDbCloseCommand.cpp  CVarDbCloseCommand.h	\
	CVarDbMkdirCommand.cpp  CVarDbMkdirCommand.h	\
	CVarDbCdCommand.cpp     CVarDbCdCommand.h	\
	CVarDbGetwdCommand.cpp  CVarDbGetwdCommand.h	\
	CVarDbLsCommand.cpp     CVarDbLsCommand.h	\
	CVarDbRmdirCommand.cpp  CVarDbRmdirCommand.h	\
	CVarDbVariableCommand.cpp CVarDbVariableCommand.h \
	CVarDBEnumCommand.cpp   CVarDBEnumCommand.h	\
	CStateMachineCommand.cpp CStateMachineCommand.h

libvardbtcl_la_CPPFLAGS=@TCL_FLAGS@ @LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/sqlite++
libvardbtcl_la_LDFLAGS= @builddir@/libvardb.la @top_builddir@/base/sqlite++/libsqlite3pp.la  \
			@SQLITE3_LDFLAGS@ @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@\
			-Wl,"-rpath=@libdir@"


# Python bindings

PYBINDINGS_CPPFLAGS=@PYTHON_CPPFLAGS@ $(libvardb_la_CPPFLAGS)
PYBINDINGS_LDFLAGS=@PYTHON_LDFLAGS@  @builddir@/libvardb.la \
			@top_builddir@/base/sqlite++/libsqlite3pp.la  \
			@SQLITE3_LDFLAGS@ \
			-Wl,"-rpath=@libdir@"

libvardbpy_la_SOURCES=PyVarDb.cpp PyVarDb.h
libvardbpy_la_CPPFLAGS=$(PYBINDINGS_CPPFLAGS)
libvardbpy_la_LDFLAGS= $(PYBINDINGS_LDFLAGS)

libvardirtreepy_la_SOURCES=PyVarDirTree.cpp
libvardirtreepy_la_CPPFLAGS= $(PYBINDINGS_CPPFLAGS)
libvardirtreepy_la_LDFLAGS=  $(PYBINDINGS_LDFLAGS)

libvariablepy_la_SOURCES = PyVariable.cpp
libvariablepy_la_CPPFLAGS= $(PYBINDINGS_CPPFLAGS)
libvariablepy_la_LDFLAGS = $(PYBINDINGS_LDFLAGS)

libenumpy_la_SOURCES = PyEnum.cpp
libenumpy_la_CPPFLAGS= $(PYBINDINGS_CPPFLAGS)
libenumpy_la_LDFLAGS=  $(PYBINDINGS_LDFLAGS)


libstatemachinepy_la_SOURCES = PyStateMachine.cpp
libstatemachinepy_la_CPPFLAGS= $(PYBINDINGS_CPPFLAGS)
libstatemachinepy_la_LDFLAGS = $(PYBINDINGS_LDFLAGS)

## Tests for compiled base


noinst_PROGRAMS=unittests
unittests_SOURCES = TestRunner.cpp  dirTests.cpp varTests.cpp typeFactoryTests.cpp \
	enumtests.cpp	statemachinetests.cpp								\
	$(libvardb_la_SOURCES)

unittests_CPPFLAGS = $(libvardb_la_CPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS =  $(libvardb_la_LDFLAGS) @CPPUNIT_LDFLAGS@


# Tests for Tcl bindings.


tclTests=TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/vardb.test


#  Get the tests run.

PYTEST = PYTHONPATH=@prefix@/pythonLibs @PYTHON@

vardbPyTest=$(PYTEST)    @srcdir@/vardbtest.py
dirtreePyTest=$(PYTEST)  @srcdir@/dirtreetest.py
variablePyTest=$(PYTEST) @srcdir@/variabletest.py
enumPyTest = $(PYTEST)   @srcdir@/enumtest.py
statemachinePyTest = $(PYTEST) @srcdir@/statemachinetests.py

check-TESTS:
	@builddir@/unittests 
	$(tclTests)
	$(vardbPyTest)
	$(dirtreePyTest)
	$(variablePyTest)
	$(enumPyTest)
	$(statemachinePyTest)




noinst_HEADERS=Asserts.h

DOCFILES=libref.xml progref.xml tutorial.xml

EXTRA_DIST= vardb.test \
	vardbtest.py dirtreetest.py variabletest.py \
	enumtest.py statemachinetests.py   \
	vardbsh.tcl.in $(DOCFILES)

# Get compiled bindings installed where the languages can find them relies on the libs being
# already built/installed in libdir -- hence install-exec-hook is used.

install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/vardb
	$(INSTALL_PROGRAM) @libdir@/libvardbtcl.* @prefix@/TclLibs/vardb
	echo  pkg_mkIndex -verbose  @prefix@/TclLibs/vardb "*.so"| @TCLSH_CMD@
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/vardb
	$(INSTALL_PROGRAM) @libdir@/libvardbpy.so       @prefix@/pythonLibs/nscldaq/vardb/vardbmodule.so
	$(INSTALL_PROGRAM) @libdir@/libvardirtreepy.so  @prefix@/pythonLibs/nscldaq/vardb/dirtree.so
	$(INSTALL_PROGRAM) @libdir@/libvariablepy.so    @prefix@/pythonLibs/nscldaq/vardb/variable.so
	$(INSTALL_PROGRAM) @libdir@/libenumpy.so        @prefix@/pythonLibs/nscldaq/vardb/enum.so
	$(INSTALL_PROGRAM) @libdir@/libstatemachinepy.so @prefix@/pythonLibs/nscldaq/vardb/statemachine.so
	touch @prefix@/pythonLibs/nscldaq/vardb/__init__.py
	$(INSTALL_PROGRAM) @builddir@/vardbsh.tcl @bindir@/vardbsh
