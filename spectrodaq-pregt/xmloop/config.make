SHELL = /bin/sh

srcdir = .


prefix = /usr/opt/spectrodaq-pregt
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/share/info
mandir = ${prefix}/share/man
includedir = ${prefix}/include
threadlibs = -lpthread

TOPDIR=/scratch/fox/daq/nscldaq/trunk/spectrodaq-pregt
THREADLIBS=-lpthread

XMLTOKLIB=/scratch/fox/daq/nscldaq/trunk/spectrodaq-pregt/src/lib/libxmltok.a
XMLPARSELIB=/scratch/fox/daq/nscldaq/trunk/spectrodaq-pregt/src/lib/libxmlparse.a
XMLPLIB=/scratch/fox/daq/nscldaq/trunk/spectrodaq-pregt/src/lib/libxmloop.a
XMLINCDIR=/scratch/fox/daq/nscldaq/trunk/spectrodaq-pregt/src/include/xmloop
XMLBASEINCDIR=/scratch/fox/daq/nscldaq/trunk/spectrodaq-pregt/src/include

INST_LIBDIR=/usr/opt/spectrodaq-pregt/lib/
INST_BINDIR=/usr/opt/spectrodaq-pregt/bin/
INST_XMLTOKLIB=/usr/opt/spectrodaq-pregt/lib/libxmltok.a
INST_XMLPARSELIB=/usr/opt/spectrodaq-pregt/lib/libxmlparse.a
INST_XMLPLIB=/usr/opt/spectrodaq-pregt/lib/libxmloop.a
INST_XMLINCDIR=/usr/opt/spectrodaq-pregt/include/xmloop

FLAGS=-g -O2 -D_REENTRANT=1 -DXML_NS=1 -DXML_DTD=1 -I$(XMLINCDIR) -I$(XMLBASEINCDIR)
CFLAGS=-g -O2 -D_REENTRANT=1 -DXML_NS=1 -DXML_DTD=1 -I$(XMLINCDIR) -I$(XMLBASEINCDIR)
CXXFLAGS=-g -O2 -D_REENTRANT=1 -DXML_NS=1 -DXML_DTD=1 -I$(XMLINCDIR) -I$(XMLBASEINCDIR)
CC=g++
CXX=g++
RANLIB=ranlib
AR=ar rs
CP=cp -f
MAKEDEP=: 
TOUCH=touch
BININST=install -m 0755
DIRINST=install -d -m 0755
INSTALL=install -m 0644
MKDIR=mkdir -p

%.o: %.cpp
	@echo "Compiling(xmloop): $<"
	@$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.cc
	@echo "Compiling(xmloop): $<"
	@$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.c
	@echo "Compiling(xmloop): $<"
	@$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

O_FILES=$(addsuffix .o,$(FILES))
C_FILES=$(addsuffix .cc,$(FILES))

build: all

depend::
	@echo "Creating Dependencies in: $(shell pwd)"
	@:> .depend
	@$(MAKEDEP) -- $(CFLAGS) -- $(C_FILES) 1>/dev/null 2>&1
ifneq (,$(strip $(SUB_DIRS)))
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make -s $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
endif

