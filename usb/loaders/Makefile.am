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

# NOTE: These .bit/.bin files don't purport to be the latest they are
#       only here to support unit tests for skipHeader.
#

EXTRA_DIST=vmeusb_0A00_031713.bit vmeusb_0A00_31913.bin ccusb_0600_072012.bit \
    usbloaders.xml


TESTS=./unittests
