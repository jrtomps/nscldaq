TCLPACKAGES=bufdumpDialogs.tcl bufdumpWidgets.tcl bufferAssembly.tcl \
	dataSources.tcl eventData.tcl pkgIndex.tcl

HELPFILES=filter.gif open.gif search.gif seqmatch.gif textmatch.gif   \
	datasource.html file.html fileexit.html fileopen.html         \
	filepackets.html fileplugin.html filter.html filters.html     \
	filtersearch.html filtersearchnext.html filterfilter.html     \
	gui.html help.html intro.html overview.html packets.html      \
	plugins.html search.html

WINRUNTIME=./kit-win32-MTall.exe
LINUXRUNTIME=./kit-linux-x86-MTall.bin

docs: manpage manual htmldocs

manpage:
	$(MANDOCBOOK) man manpage.xml

manual:
	$(HCDOCBOOK) manual.xml
	$(DVIPDF)    manual.dvi
htmldocs:
	$(HTMLDOCBOOK) -o htmldocs manual.xml
	cp *.gif htmldocs

clean:
	rm -f  bufdump.1 
	rm -f  manual.dvi manual.pdf
	rm -rf htmldocs
	rm -f *.gz *.kit

starpacks: sdx-toolchain
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
	tar czf - bwidgets iwidgets tcllib | (cd bufdump.vfs/lib; tar xzf -)
	./tclkit sdx.kit wrap bufdump.kit -runtime $(WINRUNTIME) 
	cp bufdump.kit bufdump.exe
	./tclkit sdx.kit wrap bufdump.kit -runtime $(LINUXRUNTIME) 
	cp bufdump.kit bufdump




sdx-toolchain: sdx.kit

sdx.kit:
	$(WGET) http://www.equi4.com/pub/sk/sdx.kit

