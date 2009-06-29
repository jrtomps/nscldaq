
SUBDIRS = nscl xmloop src scripts server tools tests

PROPERDIRS = nscl xmloop src server tools

H_FILES = daqconfig.h

include config.make

SUB_DIRS = $(SUBDIRS)

PROPER_DIRS = $(PROPERDIRS)

all: include lib prog examples

include::
	$(INSTALL) $(H_FILES) $(DAQINCDIR)
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
	$(DIRINST) $(INST_DAQINCDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
	make -C examples -f Makefile.install install

docinstall::
	make -C doc install

examples::
	make -C examples -f Makefile.install

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
	make -C examples -f Makefile.install clean
	rm -f *.o

realclean: clean
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
	make -C examples -f Makefile.install realclean
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
