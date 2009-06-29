
H_FILES = ConnectionFactoryException.h ErrnoException.h NSCLException.h \
NTException.h NTExceptionFactory.h OSException.h OSExceptionFactory.h \
ResolvException.h SCC1600Exception.h UNIXExceptionFactory.h WinsockException.h

FILES = ConnectionFactoryException ErrnoException NSCLException \
NTException NTExceptionFactory OSExceptionFactory ResolvException \
SCC1600Exception UNIXExceptionFactory WinsockException

include ../../../config.make

all: include lib prog

include::
	$(DIRINST) $(NSCLINCDIR)
	$(INSTALL) $(H_FILES) $(NSCLINCDIR)

lib: $(O_FILES)
	$(AR) $(NSCLLIB) $(O_FILES)

prog::

install: $(O_FILES)
	$(DIRINST) $(INST_NSCLINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_NSCLINCDIR)
	$(AR) $(INST_NSCLLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend
