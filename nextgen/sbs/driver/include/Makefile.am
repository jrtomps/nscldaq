install-data-local:
	$(mkinstalldirs) $(prefix)/include
	for f in *.h; do \
	  $(INSTALL_DATA) $$f $(prefix)/include; \
        done

EXTRA_DIST = btapi.h \
btdef.h \
btio.h \
btngpci.h \
btpapi.h \
btpdef.h \
btpio.h
