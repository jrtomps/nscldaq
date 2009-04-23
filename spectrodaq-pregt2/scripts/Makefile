
SCRIPTS = spectrodaq-conf

include ../config.make

all: prog

include::

lib::

prog::
	chmod a+x,g-w,o-w,a+r $(SCRIPTS)

install: prog
	$(DIRINST) $(INST_BINDIR)
	$(BININST) $(SCRIPTS) $(INST_BINDIR)

clean::

realclean: clean
	rm -f spectrodaq-conf
