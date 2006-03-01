
SUBDIRS = scripts src

PROPERDIRS = scripts src 

H_FILES = daqconfig.h

include config.make

SUB_DIRS = $(SUBDIRS)

PROPER_DIRS = $(PROPERDIRS)

all: dirs include lib prog 

dirs::
	$(DIRINST) ./lib/
	$(DIRINST) ./include/
	$(DIRINST) ./man/man1/
	$(DIRINST) ./bin/

include::
	$(DIRINST) $(DAQINCDIR)/dshapi
	$(INSTALL) $(H_FILES) $(DAQINCDIR)/dshapi
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

lib::
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
	make -C src toys

install: include lib prog
	$(DIRINST) $(INST_DAQINCDIR)/dshapi
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)/dshapi
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

docinstall::
	make -C doc install

depend::
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

clean::
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
	rm -f *.o

realclean: clean
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
	rm -f config.cache config.make config.log config.h config.status

proper: clean
	for i in $(PROPER_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make realclean) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
	for i in $(PROPER_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make depend) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
