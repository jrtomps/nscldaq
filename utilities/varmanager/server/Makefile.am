lib_LTLIBRARIES=libvarmgr.la

install-exec-local:
	echo "#!@PYTHON@" >  @bindir@/vardbServer
	cat @srcdir@/vardbServer.py >>@bindir@/vardbServer
	chmod a+x @bindir@/vardbServer


SERVER_TESTS=startTests.py serviceTests.py mkdirTests.py  \
	testGet.py

check-TESTS:
	for f in $(SERVER_TESTS); do echo $$f : ; PYTHONPATH=@prefix@/pythonLibs  BINDIR=@bindir@  @PYTHON@ @srcdir@/$$f; done


EXTRA_DIST=$(SERVER_TESTS) testBase.py vardbServer.py varserver.xml

libvarmgr_la_SOURCES = 
libvarmgr_la_HEADERS=CVarMgrApi.h