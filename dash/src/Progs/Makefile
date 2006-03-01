
SUB_DIRS = 

H_FILES = 

FILES = dirlist packetparam

include ../../config.make

all: dirlist packetparam

include::

lib::

dirlist: dirlist.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ dirlist.cc $(APILIB) $(LDFLAGS) 

charconvert: charconvert.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ charconvert.cc $(APILIB) $(LDFLAGS) 

packetparam: packetparam.cc $(APILIB) 
	$(CXX) $(CXXFLAGS) -o $@ packetparam.cc $(APILIB) $(LDFLAGS) 

filtersetvark: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset256: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=256 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=256 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=256 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset512: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=512 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=512 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=512 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset768: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=768 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=768 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=768 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset1k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=1024 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=1024 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=1024 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset2k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=2048 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=2048 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=2048 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset3k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=3072 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=3072 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=3072 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset4k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=4096 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=4096 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=4096 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset5k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=5120 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=5120 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=5120 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset6k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=6144 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=6144 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=6144 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset7k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=7168 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=7168 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=7168 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset8k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=8192 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=8192 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=8192 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset16k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=16384 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=16384 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=16384 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset32k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=32768 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=32768 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=32768 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset48k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=49152 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=49152 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=49152 -o filter filter.cc $(APILIB) $(LDFLAGS)

filterset64k: $(H_FILES) $(C_FILES) $(APILIB) 
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=65536 -o source source.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=65536 -o sink sink.cc $(APILIB) $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -D MYBUFSIZ=65536 -o filter filter.cc $(APILIB) $(LDFLAGS)

busy::
	$(CC) $(CFLAGS) -o $@ busy.c 

install::

clean::
	rm -f *.o

realclean: clean
	rm -f $(FILES)
	rm -f .depend
	:> .depend

include .depend
