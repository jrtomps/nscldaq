SHELL = /bin/sh -e

srcdir = .


prefix = /usr/opt/spectrodaq
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
cryptlibs = -lcrypt
gtk_libs = 
gtk_cflags = 

TOPDIR=/usr/src/nscl/spectrodaq-pregt
THREADLIBS=-lpthread
CRYPTLIBS=-lcrypt
NSCL_CODE=/usr/opt/spectrodaq
NSCL_CODE_INC=/usr/opt/spectrodaq/include/nscl
NSCL_CODE_LIB=/usr/opt/spectrodaq/lib/libNSCLException.a

DAQLIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libspectrodaq.a
MPILIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libnsclmpi.a
NETIOLIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libnetio.a
DAQTHREADSLIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libdaqthreads.a
CLIENTLIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libdaqclient.a
SERVLIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libdaqserv.a
NSCLLIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libNSCLException.a
XMLOOPLIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libxmloop.a
XMLPARSELIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libxmlparse.a
XMLTOKLIB=/usr/src/nscl/spectrodaq-pregt/src/lib/libxmltok.a
DAQINCDIR=/usr/src/nscl/spectrodaq-pregt/src/include/
NSCLINCDIR=/usr/src/nscl/spectrodaq-pregt/src/include/nscl
MPIINCDIR=/usr/src/nscl/spectrodaq-pregt/src/include/mpi
NETIOINCDIR=/usr/src/nscl/spectrodaq-pregt/src/include/netio
DAQTHREADSINCDIR=/usr/src/nscl/spectrodaq-pregt/src/include/threads
XMLINCDIR=/usr/src/nscl/spectrodaq-pregt/src/include/xmloop

XMLLIBS=$(XMLOOPLIB) $(XMLPARSELIB) $(XMLTOKLIB)

INST_LIBDIR=/usr/opt/spectrodaq/lib/
INST_BINDIR=/usr/opt/spectrodaq/bin/
INST_EXMPDIR=/usr/opt/spectrodaq/examples/
INST_CONFDIR=/usr/opt/spectrodaq/etc/
INST_DOCDIR=/usr/opt/spectrodaq/doc/
INST_DAQLIB=/usr/opt/spectrodaq/lib/libspectrodaq.a
INST_MPILIB=/usr/opt/spectrodaq/lib/libnsclmpi.a
INST_NETIOLIB=/usr/opt/spectrodaq/lib/libnetio.a
INST_NSCLLIB=/usr/opt/spectrodaq/lib/libNSCLException.a
INST_DAQTHREADSLIB=/usr/opt/spectrodaq/lib/libdaqthreads.a
INST_CLIENTLIB=/usr/opt/spectrodaq/lib/libdaqclient.a
INST_SERVLIB=/usr/opt/spectrodaq/lib/libdaqserv.a
INST_XMLOOPLIB=/usr/opt/spectrodaq/lib/libxmloop.a
INST_XMLPARSELIB=/usr/opt/spectrodaq/lib/libxmlparse.a
INST_XMLTOKLIB=/usr/opt/spectrodaq/lib/libxmltok.a
INST_DAQINCDIR=/usr/opt/spectrodaq/include/
INST_MPIINCDIR=/usr/opt/spectrodaq/include/mpi
INST_NSCLINCDIR=/usr/opt/spectrodaq/include/nscl
INST_NETIOINCDIR=/usr/opt/spectrodaq/include/netio
INST_DAQTHREADSINCDIR=/usr/opt/spectrodaq/include/threads
INST_DAQDOCDIR=/usr/opt/spectrodaq/doc
INST_XMLINCDIR=/usr/opt/spectrodaq/include/xmloop

INST_XMLLIBS=$(INST_XMLOOPLIB) $(INST_XMLPARSELIB) $(INST_XMLTOKLIB)

FLAGS= -g -O2 -D_REENTRANT=1 -I$(DAQINCDIR) -I$(NSCLINCDIR) -I$(XMLINCDIR)
CFLAGS= -g -O2 -D_REENTRANT=1 -I$(DAQINCDIR) -I$(NSCLINCDIR) -I$(XMLINCDIR)
CXXFLAGS= -g -O2 -D_REENTRANT=1 -I$(DAQINCDIR) -I$(NSCLINCDIR) -I$(XMLINCDIR)
COMPILER_VERSION=
CC=gcc
CXX=g++
RANLIB=ranlib
AR=ar rs
CP=cp -f
MAKEDEP=makedepend -f .depend 
TOUCH=touch
BININST=install -m 0755
DIRINST=install -d -m 0755
INSTALL=install -m 0644
MKDIR=mkdir -p
DOXYGEN=doxygen

LIBS=$(NSCLLIB)
LDFLAGS=$(NSCLLIB)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

O_FILES=$(addsuffix .o,$(FILES))
C_FILES=$(addsuffix .cc,$(FILES))

build: all

depend::
	@echo "Creating Dependencies in: $(shell pwd)"
	@:> .depend
	$(MAKEDEP) -- $(CFLAGS) -- $(C_FILES) 1>/dev/null 2>&1
ifneq (,$(strip $(SUB_DIRS)))
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make -s $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
endif

