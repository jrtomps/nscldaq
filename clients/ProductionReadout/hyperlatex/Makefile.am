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
              $(INSTALL_DATA) $$f $(prefix)/Docs/ProductionReadout/html; \
	    done); \
        fi
	if [ -f Readout.ps ] ; \
        then \
	   $(INSTALL_DATA) Readout.ps $(prefix)/Docs/ProductionReadout; \
        fi
