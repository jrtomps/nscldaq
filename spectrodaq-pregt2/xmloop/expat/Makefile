
SUB_DIRS = gennmtab xmltok xmlparse

FILES = 

H_FILES = 

include ../config.make

all: pre include lib 

pre::
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"

include::
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

install::
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
# DO NOT DELETE
