install-data-local:
	$(HYPERLATEX) Readout.tex
	$(LATEX)      Readout.tex
	$(LATEX)      Readout.tex
	$(DVIPS)  -o      Readout.ps Readout.dvi
	if [ -d html ] ; then  \
	   $(mkinstalldirs) $(prefix)/Docs/ProductionReadout/html; \
	   chmod a+rx $(prefix)/Docs $(prefix)/Docs/ProductionReadout \
	              $(prefix)/Docs/ProductionReadout/html; \
	   (cd html; \
            for f in $$(ls); \
            do \
              $(INSTALL_DATA)  $$f $(prefix)/Docs/ProductionReadout/html; \
	    done); \
        fi
	if [ -f Readout.ps ] ; \
        then \
	   $(INSTALL_DATA)  Readout.ps $(prefix)/Docs/ProductionReadout; \
        fi

EXTRA_DIST = CAMACTriggerelec.eps \
	CAMACTriggerelec.gif \
	EventPacket.eps \
	EventPacket.gif \
	InitializationFlow.eps \
	InitializationFlow.gif \
	MajorClasses.eps \
	MajorClasses.gif \
	Readout.tex \
	ReadoutBody.tex \
	RunStateDiagram.eps \
	RunStateDiagram.gif \
	Theadingdiagram.eps \
	Threadingdiagram.eps \
	Threadingdiagram.gif \
	VMETriggerelec.eps \
	VMETriggerelec.gif \
	definitions.tex \
	eventformat.eps \
	eventformat.gif