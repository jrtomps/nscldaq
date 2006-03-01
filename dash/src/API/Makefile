
SUB_DIRS = 

FILES = Exceptions String StringTokenizer globals Process \
Main daqhwyapi_main main System Buffer ByteBuffer \
FdInputStream FdOutputStream FileInputStream FileOutputStream \
LineRecordReader PipeConnector Record FdSelector ByteArray \
FdRecordReader PipeInjector BufferedRecordReader \
BufferedReader BufferedWriter FdRecordWriter BufferedRecordWriter \
FSUtils StringUtils StringArray DSHUtils IntArray WordArray \
DWordArray BinaryRecordReader

H_FILES = daqhwyapi.h cstr.h maindefs.h Exceptions.h Object.h String.h \
StringTokenizer.h Runnable.h Process.h Main.h System.h \
InputStream.h OutputStream.h Buffer.h ByteBuffer.h RecordReader.h \
FdInputStream.h FdOutputStream.h FileInputStream.h FileOutputStream.h \
RecordWriter.h LineRecordReader.h PipeConnector.h Record.h \
Selector.h FdSelector.h ByteArray.h FdRecordReader.h PipeInjector.h \
BufferedRecordReader.h Reader.h Writer.h DSHArray.h \
BufferedReader.h BufferedWriter.h FdRecordWriter.h BufferedRecordWriter.h \
FSUtils.h StringUtils.h StringArray.h DSHUtils.h IntArray.h WordArray.h \
DWordArray.h BinaryRecordReader.h

include ../../config.make

all: include lib prog

include::
	$(DIRINST) $(APIINCDIR)
	$(INSTALL) $(H_FILES) $(APIINCDIR)
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
	$(AR) $(APILIB) $(O_FILES)

prog::

install: $(O_FILES)
	$(DIRINST) $(INST_APIINCDIR)
	$(DIRINST) $(INST_LIBDIR)
	$(INSTALL) $(H_FILES) $(INST_APIINCDIR)
	for i in $(SUB_DIRS); do \
          echo "Making $@ in $$i..."; \
          (cd $$i && make $@) || \
          case "$$MAKEFLAGS" in *k*) FAIL="yes";; *) exit 1;; esac; \
        done; test -z "$$FAIL"
	$(AR) $(INST_APILIB) $(O_FILES)

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
	rm -f .depend
	:> .depend

include .depend
