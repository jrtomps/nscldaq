FILES =

H_FILES = spectrodaq.h

SUB_DIRS = Support Netio Daq_thread Main Object Client Server 

include ../config.make

all: include lib prog

lib::
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

prog::
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

toys::
	make -C Progs toys

install::
	$(DIRINST) $(INST_DAQINCDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

clean:
	rm -f *.o 
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

realclean: clean
	rm -f .depend
	:> .depend
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
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
