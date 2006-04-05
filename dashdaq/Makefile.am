SUBDIRS = sbsbit3 exceptions


install-data-local:
	$(INSTALL_DATA) config.h  @prefix@/include
#
#   The following target is for installation verification.
#   It runs all post installation tests on all subdirs that have 
#   them.
#
ivp:
	for f in $(SUBDIRS); do (cd $$f; make ivp);  done