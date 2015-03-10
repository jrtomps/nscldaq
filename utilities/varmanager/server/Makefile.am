lib_LTLIBRARIES=libvarmgr.la
libvarmgr_la_SOURCES = CVarMgrApi.cpp
include_HEADERS=CVarMgrApi.h

install-exec-local:
	echo "#!@PYTHON@" >  @bindir@/vardbServer
	cat @srcdir@/vardbServer.py >>@bindir@/vardbServer
	chmod a+x @bindir@/vardbServer


SERVER_TESTS=startTests.py serviceTests.py mkdirTests.py  \
	testGet.py

noinst_PROGRAMS = testApi

testApi_SOURCES=TestRunner.cpp TestApiBase.cpp $(libvarmgr_la_SOURCES)
testApi_CXXFLAGS=@CPPUNIT_CFLAGS@
testApi_LDFLAGS = @CPPUNIT_LDFLAGS@

check-TESTS:
	for f in $(SERVER_TESTS); do echo $$f : ; PYTHONPATH=@prefix@/pythonLibs  BINDIR=@bindir@  @PYTHON@ @srcdir@/$$f; done
	testApi


EXTRA_DIST=$(SERVER_TESTS) testBase.py vardbServer.py varserver.xml

