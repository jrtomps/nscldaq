bin_PROGRAMS = TcpHoist
TcpHoist_SOURCES = TcpHoist.cpp TcpClient.cpp  
INCLUDES=-I@top_srcdir@/Headers \
	$(SPECTRODAQ_FLAGS) -I@top_srcdir@/CopyrightTools
TcpHoist_LDADD=$(SPECTRODAQ_LDFLAGS) -L@top_srcdir@/CopyrightTools -lLicense 
TcpHoist_LDFLAGS=-Wl,"-rpath=$(libdir)"

noinst_HEADERS = TcpClient.h

EXTRA_DIST = 0README.txt systest.sh tcphoist.1

install-data-local:
	$(mkinstalldirs) $(prefix)/Docs
	$(mkinstalldirs) $(prefix)/Docs/TcpHoister
	$(INSTALL_DATA) 0README.txt $(prefix)/Docs/TcpHoister/TcpHoister.txt

man_MANS = tcphoist.1



#  The tests:


noinst_PROGRAMS = systest systest_server

systest_SOURCES = systest.cpp
systest_server_SOURCES  = systest_server.cpp

systest_LDFLAGS=$(SPECTRODAQ_LDFLAGS)



TESTS = ./systest.sh

