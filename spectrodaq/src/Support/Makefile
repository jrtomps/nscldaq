
H_FILES = Bag.h PointerBag.h Array.h Pointer.h BufferAllocator.h \
ArrayAllocator.h LinkedList.h Buffer.h daqcstr.h HashTable.h \
OStream.h Queue.h Printable.h MutexIface.h NullMutex.h DebugMutex.h \
genmacros.h Stack.h PtrStack.h

FILES = 

include ../../config.make

all: include lib prog

lib: $(O_FILES)
#	$(AR) $(DAQLIB) $(O_FILES)

include::
	$(CP) $(H_FILES) $(DAQINCDIR)

prog::

install: $(O_FILES)
	$(DIRINST) $(INST_DAQINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_DAQINCDIR)
	$(AR) $(INST_DAQLIB) $(O_FILES)

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend
