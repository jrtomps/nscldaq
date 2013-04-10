bin_PROGRAMS=ccusbloader vmusbloader

noinst_HEADERS=libxxusb.h skipHeader.h Asserts.h

ccusbloader_SOURCES=ccusb_flash.cpp skipHeader.cpp libxxusb.cpp

vmusbloader_SOURCES=vmusb_flash.cpp skipHeader.cpp libxxusb.cpp

ccusbloader_LDFLAGS=@USB_LIBS@

vmusbloader_LDFLAGS=@USB_LIBS@


#-----------------------------------------------------------------------------
# Unit tests:
#

noinst_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp skiptest.cpp skipHeader.cpp
unittests_LDADD= $(CPPUNIT_LDFLAGS)

# Test bit file.

EXTRA_DIST=vmeusb_0A00_031713.bit


TESTS=./unittests
