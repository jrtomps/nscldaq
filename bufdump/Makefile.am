#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



TCLPACKAGES=bufdumpDialogs.tcl bufdumpWidgets.tcl bufferAssembly.tcl \
	dataSources.tcl eventData.tcl pkgIndex.tcl

HELPFILES=filter.gif open.gif search.gif seqmatch.gif textmatch.gif   \
	datasource.html file.html fileexit.html fileopen.html         \
	filepackets.html fileplugin.html filter.html filters.html     \
	filtersearch.html filtersearchnext.html filterfilter.html     \
	gui.html help.html intro.html overview.html packets.html      \
	plugins.html search.html

HTMLGRAPHICS=filemenu.gif opencallout.gif filtermenu.gif filtercallouts.gif \
	searchcallouts.gif seqmatch.gif helpmenu.gif bufdump.gif open.gif

EPSGRAPHICS=bufdump.eps filemenu.eps filtercallouts.eps helpmenu.eps filtermenu.eps \
	open.eps opencallout.eps searchcallouts.eps seqmatch.eps

DOCUMENTATION=concepts.xml extending.xml filemenu.xml filtermenu.xml helpmenu.xml \
	manpage.xml manual.xml quickstart.xml refman.xml


WINRUNTIME=./kit-win32-MTall.exe
LINUXRUNTIME=./kit-linux-x86-MTall.bin

all: docs  starpacks

# 
#   Heres's where we do the installation.
#   $(prefix) is our installation top level dir.
#   Underneath we build the following tree:
#       lib     - Where the packages we depend on are
#                 installed.
#       help    - Where the online help files are installed.
#       doc     - Where the printed manual is installed.
#       man/man1- Where the manpage is installed.
#       htmldocs- Where the web documentation is installed.
#       images  - Where the htmldocs expect the callout images installed.
#       etc     - Where the packet definition file is.
#       bin     - where the script is installed.  
#
#  Note that if docbook is not installed in all it's
#  glory we will not be able to generate the documentation
#  and that's just life.
#
install-exec-am: docs
	$(mkinstalldirs) $(prefix)/lib
	$(mkinstalldirs) $(prefix)/help
	$(mkinstalldirs) $(prefix)/doc
	$(mkinstalldirs) $(prefix)/man $(prefix)/man/man1
	$(mkinstalldirs) $(prefix)/htmldocs
	$(mkinstalldirs) $(prefix)/images $(prefix)/images/callouts
	$(mkinstalldirs) $(prefix)/etc
	$(mkinstalldirs) $(prefix)/bin
	$(INSTALL_PROGRAM) bufdump.tcl  $(prefix)/bin/bufdump
	$(INSTALL_DATA)  $(TCLPACKAGES) $(prefix)/lib
	$(INSTALL_DATA)  $(HELPFILES)   $(prefix)/help
	$(INSTALL_DATA)  packets.def    $(prefix)/etc
	if [ -e bufdump.1 ]; then \
		$(INSTALL_DATA) bufdump.1 $(prefix)/man/man1; \
	fi
	if [ -e manual.pdf ]; then \
		$(INSTALL_DATA) manual.pdf $(prefix)/doc;     \
	fi
	if [ -d htmldocs ]; then \
		$(INSTALL_DATA) htmldocs/* $(prefix)/htmldocs; \
		$(INSTALL_DATA) $(HTMLGRAPHICS) $(prefix)/htmldocs; \
		$(INSTALL_DATA) calloutgraphics/*.gif      $(prefix)/images/callouts;   \
	fi


#  Install the documentation alone in some web-place.
#
#
install-webdocs: docs
	$(mkinstalldirs) $(prefix)
	$(INSTALL_DATA)   manual.pdf $(prefix)
	if [ -d htmldocs ]; then \
		$(INSTALL_DATA) htmldocs/* $(prefix); \
		$(INSTALL_DATA) $(HTMLGRAPHICS) $(prefix); \
		$(mkinstalldirs) $(prefix)/../images $(prefix)/../images/callouts;     \
		$(INSTALL_DATA) calloutgraphics/*.gif      $(prefix)/../images/callouts;   \
	fi


docs: manpage manual htmldocs

manpage:
	$(MANDOCBOOK) man manpage.xml

manual:
	$(HCDOCBOOK) manual.xml
	$(DVIPDF)    manual.dvi
htmldocs:
	$(HTMLDOCBOOK) -o htmldocs manual.xml
	if [ -d htmldocs ]; then   \
	    cp *.gif htmldocs;     \
	fi

clean:
	rm -f  bufdump.1 
	rm -f  manual.dvi manual.pdf
	rm -rf htmldocs
	rm -f *.gz *.kit

starpacks: sdx-toolchain htmldocs
	rm -rf tclkit
	cp $(SDXRUNTIME) tclkit
	rm -rf bufdump.kit bufdump.vfs
	./tclkit sdx.kit qwrap bufdump.tcl
	./tclkit sdx.kit unwrap bufdump.kit
	cp $(TCLPACKAGES) bufdump.vfs/lib
	mkdir bufdump.vfs/etc
	cp packets.def bufdump.vfs/etc
	mkdir bufdump.vfs/help
	cp $(HELPFILES) bufdump.vfs/help
	if [ -d htmldocs ]; then  \
		mkdir -p bufdump.vfs/htmldocs;           \
		mkdir -p bufdump.vfs/images/callouts;    \
		cp htmldocs/*      bufdump.vfs/htmldocs; \
		cp $(HTMLGRAPHICS) bufdump.vfs/htmldocs; \
		cp calloutgraphics/*.gif  bufdump.vfs/images/callouts; \
	fi
	tar czf - bwidgets iwidgets tcllib | (cd bufdump.vfs/lib; tar xzf -)
	./tclkit sdx.kit wrap bufdump.kit -runtime $(WINRUNTIME) 
	cp bufdump.kit bufdump.exe
	./tclkit sdx.kit wrap bufdump.kit -runtime $(LINUXRUNTIME) 
	cp bufdump.kit bufdump




sdx-toolchain: sdx.kit

sdx.kit:
	$(WGET) http://www.equi4.com/pub/sk/sdx.kit


EXTRA_DIST=$(TCLPACKAGES) $(HELPFILES) $(HTMLGRAPHICS) \
	$(WINRUNTIME) $(LINUXRUNTIME) calloutgraphics  \
	packets.def bwidgets iwidgets tcllib bufdump.tcl \
	$(DOCUMENTATION) $(EPSGRAPHICS)

