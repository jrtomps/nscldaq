install-data-local:
	$(mkinstalldirs) @prefix@/include
	for f in $(EXTRA_DIST); do \
	  $(INSTALL_DATA) @srcdir@/$$f @prefix@/include; \
        done

EXTRA_DIST =btapi.h \
btdbpci.h \
btdef.h   \
bt_error.h \
btio.h \
btpapi.h \
btpdef.h \
btpio.h \
bt_time.h \
btuisr.h 

