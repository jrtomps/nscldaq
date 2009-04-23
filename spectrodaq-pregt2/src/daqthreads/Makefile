
SUB_DIRS = pthreads

FILES = 

H_FILES = daqthreads.h

include ../../config.make

all: include lib

include::
	$(DIRINST) $(DAQTHREADSINCDIR)
	$(INSTALL) $(H_FILES) $(DAQTHREADSINCDIR)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

lib: $(O_FILES)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

install: $(O_FILES)
	$(DIRINST) $(INST_DAQTHREADSINCDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQTHREADSINCDIR)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

prog::

clean::
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

include .depend
