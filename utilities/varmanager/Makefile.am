#  Programs: initticl is a tclsh that 
#  - requires a script ?args?
#  - Does not exit at EOF on the script.
#

bin_PROGRAMS=inittcl

inittcl_SOURCES=inittcl.c
inittcl_CPPFLAGS=@TCL_CPPFLAGS@
inittcl_LDFLAGS=@TCL_LDFLAGS@

# Shared libraries:

lib_LTLIBRARIES = libvardb.la libvardbtcl.la libvardbpy.la libvardirtreepy.la

#  Base C++ implementation

libvardb_la_SOURCES=CVariableDb.cpp CVarDirTree.cpp CTypeFactory.cpp \
	CVariable.cpp CIntegerType.cpp CRealType.cpp CStringType.cpp \
	CDataTypeCreatorBase.cpp CIntegerTypeCreator.cpp CRealTypeCreator.cpp \
	CStringTypeCreator.cpp

libvardb_la_CPPFLAGS = @SQLITE3_CFLAGS@ -I@top_srcdir@/base/sqlite++ -I@top_srcdir@/base/factories
libvardb_la_LDFLAGS  = 	@top_srcdir@/base/sqlite++/libsqlite3pp.la  @SQLITE3_LDFLAGS@ 

include_HEADERS = CVariableDb.h CVarDirTree.h CVariable.h CTypeFactory.h \
	CDataType.h CIntegerType.h CRealType.h CStringType.h \
	CDataTypeCreatorBase.h CIntegerTypeCreator.h CRealTypeCreator.h
	CStringTypeCreator.h


#  Tcl bindings

libvardbtcl_la_SOURCES=VarDbTclPackage.cpp \
	CVarDbCreateCommand.cpp CVarDbCreateCommand.h \
	CVarDbOpenCommand.cpp   CVarDbOpenCommand.h	\
	CVarDbCloseCommand.cpp  CVarDbCloseCommand.h	\
	CVarDbMkdirCommand.cpp  CVarDbMkdirCommand.h	\
	CVarDbCdCommand.cpp     CVarDbCdCommand.h	\
	CVarDbGetwdCommand.cpp  CVarDbGetwdCommand.h	\
	CVarDbLsCommand.cpp     CVarDbLsCommand.h	\
	CVarDbRmdirCommand.cpp  CVarDbRmdirCommand.h

libvardbtcl_la_CPPFLAGS=@TCL_FLAGS@ @LIBTCLPLUS_CFLAGS@ 
libvardbtcl_la_LDFLAGS= @builddir@/libvardb.la @top_srcdir@/base/sqlite++/libsqlite3pp.la  \
			@SQLITE3_LDFLAGS@ @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@\
			-Wl,"-rpath=@libdir@"


# Python bindings

libvardbpy_la_SOURCES=PyVarDb.cpp PyVarDb.h
libvardbpy_la_CPPFLAGS=@PYTHON_CPPFLAGS@ $(libvardb_la_CPPFLAGS)
libvardbpy_la_LDFLAGS=@PYTHON_LDFLAGS@  @builddir@/libvardb.la @top_srcdir@/base/sqlite++/libsqlite3pp.la  \
			@SQLITE3_LDFLAGS@ \
			-Wl,"-rpath=@libdir@"

libvardirtreepy_la_SOURCES=PyVarDirTree.cpp
libvardirtreepy_la_CPPFLAGS=@PYTHON_CPPFLAGS@ $(libvardb_la_CPPFLAGS)
libvardirtreepy_la_LDFLAGS=@PYTHON_LDFLAGS@  @builddir@/libvardb.la @top_srcdir@/base/sqlite++/libsqlite3pp.la  \
			@SQLITE3_LDFLAGS@ \
			-Wl,"-rpath=@libdir@"



## Tests for compiled base


noinst_PROGRAMS=unittests
unittests_SOURCES = TestRunner.cpp  dirTests.cpp varTests.cpp typeFactoryTests.cpp $(libvardb_la_SOURCES)
unittests_CPPFLAGS = $(libvardb_la_CPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS =  $(libvardb_la_LDFLAGS) @CPPUNIT_LDFLAGS@


# Tests for Tcl bindings.


tclTests=TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ vardb.test


#  Get the tests run.

vardbPyTest=PYTHONPATH=@prefix@/pythonLibs @PYTHON@ vardbtest.py
dirtreePyTest=PYTHONPATH=@prefix@/pythonLibs @PYTHON@ dirtreetest.py

check-TESTS:
	@builddir@/unittests 
	$(tclTests)
	$(vardbPyTest)
	$(dirtreePyTest)




noinst_HEADERS=Asserts.h

DOCFILES=libref.xml progref.xml tutorial.xml

EXTRA_DIST=$(tclTests) vardbtest.py dirtreetest.py vardbsh.tcl.in $(DOCFILES)

# Get compiled bindings installed where the languages can find them relies on the libs being
# already built/installed in libdir -- hence install-exec-hook is used.

install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/vardb
	$(INSTALL_PROGRAM) @libdir@/libvardbtcl.* @prefix@/TclLibs/vardb
	echo  pkg_mkIndex -verbose  @prefix@/TclLibs/vardb "*.so"| @TCLSH_CMD@
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/vardb
	rm @prefix@/pythonLibs/nscldaq/vardb/*
	$(INSTALL_PROGRAM) @libdir@/libvardbpy.so @prefix@/pythonLibs/nscldaq/vardb/vardbmodule.so
	$(INSTALL_PROGRAM) @libdir@/libvardirtreepy.so @prefix@/pythonLibs/nscldaq/vardb/dirtree.so
	touch @prefix@/pythonLibs/nscldaq/vardb/__init__.py
	$(INSTALL_PROGRAM) @builddir@/vardbsh.tcl @bindir@/vardbsh
