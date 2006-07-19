SUBDIRS = sbsbit3 exceptions vme camac devices


install-data-local:
	$(mkinstalldirs)  @prefix@/include
	$(INSTALL_DATA) config.h  @prefix@/include

#
#   The following target is for installation verification.
#   It runs all post installation tests on all subdirs that have 
#   them.
#
ivp:
	for f in $(SUBDIRS); do (cd $$f; make ivp);  done