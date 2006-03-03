SUBDIRS = sbsbit3 exceptions


#
#   The following target is for installation verification.
#   It runs all post installation tests on all subdirs that have 
#   them.
#
ivp:
	for f in $(SUBDIRS); do (cd $$f; make ivp);  done