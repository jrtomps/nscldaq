FILES =

H_FILES = spectrodaq.h

SUB_DIRS = Support Netio Daq_thread Main Object Main Client Server 

include ../config.make

all: include lib prog

lib::
	for i in $(SUB_DIRS); do (cd $$i; make lib); done;

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)
	for i in $(SUB_DIRS); do (cd $$i; make include); done;

prog::
	for i in $(SUB_DIRS); do (cd $$i; make prog); done;

toys::
	(cd Progs; make toys)

install::
	$(DIRINST) $(INST_DAQINCDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	for i in $(SUB_DIRS); do (cd $$i; make install); done;

clean:
	rm -f *.o 
	for i in $(SUB_DIRS); do (cd $$i; make clean); done;

realclean: clean
	rm -f .depend
	:> .depend
	for i in $(SUB_DIRS); do (cd $$i; make realclean); done;
	rm -f $(DAQLIB)
	rm -f $(NETIOLIB)
	rm -f $(CLIENTLIB)
	rm -f $(SERVLIB)
	rm -f $(DAQTHREADSLIB)
	rm -f $(NSCLLIB)
	rm -f $(XMLOOPLIB)
	rm -f $(XMLPARSELIB)
	rm -f $(XMLTOKLIB)
	rm -f $(DAQINCDIR)/*.h
	rm -f $(NETIOINCDIR)/*.h
	rm -f $(MPIINCDIR)/*.h
	rm -f $(DAQTHREADSINCDIR)/*.h
	rm -f $(NSCLINCDIR)/*.h
	rm -f $(XMLINCDIR)/*.h
	rmdir $(NETIOINCDIR)
	rmdir $(DAQTHREADSINCDIR)
	rmdir $(NSCLINCDIR)
	rmdir $(XMLINCDIR)
#	rmdir $(MPIINCDIR)

include .depend
