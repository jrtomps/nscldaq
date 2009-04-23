
SUB_DIRS = 

FILES = XMLHandler XMLParser XMLOut XMLPipe XMLNamespace XMLINSStack \
XMLLinkedList XMLStateStack XMLStack

H_FILES = XMLLinkedList.h XMLHandler.h XMLParser.h xmlpmacros.h xmlpcstr.h \
XMLOut.h XMLPipe.h XMLStateStack.h XMLINSStack.h XMLNamespace.h \
XMLObject.h XMLStack.h

include ../config.make

all: pre include lib 

pre::

include: $(H_FILES)
	$(DIRINST) $(XMLINCDIR)
	$(INSTALL) $(H_FILES) $(XMLINCDIR)

lib: $(O_FILES)
	$(AR) $(XMLPLIB) $(O_FILES)

install: $(O_FILES)
	$(DIRINST) $(INST_XMLINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_XMLINCDIR)
	$(AR) $(INST_XMLPLIB) $(O_FILES)

prog: xmltest

xmltest: xmltest.cc ../lib/libxmloop.a
	$(CXX) $(CXXFLAGS) -I../include -o xmltest xmltest.cc -L../lib -lxmloop -lxmlparse -lxmltok

clean::
	rm -f *.o

realclean: clean
	rm -f .depend
	:> .depend

include .depend
# DO NOT DELETE
