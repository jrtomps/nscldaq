lib_LTLIBRARIES = libsqlite3pp.la

include_HEADERS=CSqlite.h CSqliteException.h CSqliteStatement.h\
	 CSqliteTransaction.h

libsqlite3pp_la_SOURCES=CSqlite.cpp CSqliteException.cpp CSqliteStatement.cpp \
	CSqliteTransaction.cpp


libsqlite3pp_la_CPPFLAGS = @SQLITE3_CFLAGS@
libsqlite3pp_la_LDFLAGS  = @SQLITE3_LDFLAGS@

noinst_PROGRAMS=unittests
noinst_HEADERS=Asserts.h

unittests_SOURCES = TestRunner.cpp exceptionTest.cpp connectionTest.cpp \
			statementTest.cpp transactionTest.cpp \
			$(libsqlite3pp_la_SOURCES)

unittests_LDFLAGS = @SQLITE3_LDFLAGS@ @CPPUNIT_LDFLAGS@
unittests_CPPFLAGS= @SQLITE3_CFLAGS@ @CPPUNIT_CFLAGS@


TESTS=@builddir@/unittests


EXTRA_DIST = sqlite++.xml
