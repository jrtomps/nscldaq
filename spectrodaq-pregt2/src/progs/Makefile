
O_FILES = 

BUILD = $(DAQLIB) $(CLIENTLIB) $(NETIOLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)


include ../../config.make

prog_files = bufrw alarm trap daqxml pagehead bufcyc bufrecv bufrecv2 bufsend2 bufsend daqthreads daqqueue daqstream daqbuf daqrpc daqtest daqcom daqhash daqstrtest urbufsend2 urbufrecv2 mixbufrecv2 mixbufsend2 buffree bufrecv2ur cpysend cpyrecv rpcsrvr rpcclnt comsrvr comclnt daqlog daqstringtest daqthreads2

all: toys

toys: $(prog_files)

pagehead: pagehead.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ pagehead.cc $(BUILD) 

daqxml: daqxml.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqxml.cc $(BUILD)

daqthreads: daqthreads.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqthreads.cc $(BUILD)

daqthreads2: daqthreads2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqthreads2.cc $(BUILD)

daqstringtest: daqstringtest.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqstringtest.cc $(BUILD) 

daqtest: daqtest.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqtest.cc $(BUILD) 

daqstrtest: daqstrtest.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqstrtest.cc $(BUILD) 

daqrpc: daqrpc.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqrpc.cc $(BUILD) 

rpcsrvr: rpcsrvr.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ rpcsrvr.cc $(BUILD) 

rpcclnt: rpcclnt.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ rpcclnt.cc $(BUILD) 

daqcom: daqcom.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqcom.cc $(BUILD) 

comsrvr: comsrvr.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ comsrvr.cc $(BUILD) 

comclnt: comclnt.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ comclnt.cc $(BUILD) 

daqlog: daqlog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqlog.cc $(BUILD) 

alarm: alarm.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ alarm.cc $(BUILD) 

trap: trap.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ trap.cc $(BUILD) 

daqbuf: daqbuf.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqbuf.cc $(BUILD) 

bufrw: bufrw.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufrw.cc $(BUILD) 

daqbuf2: daqbuf2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqbuf2.cc $(BUILD) 

bufrecv: bufrecv.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufrecv.cc $(BUILD) 

bufrecv2: bufrecv2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufrecv2.cc $(BUILD) 

bufrecv2ur: bufrecv2ur.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufrecv2ur.cc $(BUILD)

bufsend2: bufsend2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufsend2.cc $(BUILD) 

bufsend3: bufsend3.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufsend3.cc $(BUILD) 

bufsend4: bufsend4.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufsend4.cc $(BUILD) 

cpyrecv: cpyrecv.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ cpyrecv.cc $(BUILD) 

cpysend: cpysend.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ cpysend.cc $(BUILD) 

urbufrecv2: urbufrecv2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ urbufrecv2.cc $(BUILD) 

urbufsend2: urbufsend2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ urbufsend2.cc $(BUILD) 

buffree: buffree.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ buffree.cc $(BUILD) 

mixbufrecv2: mixbufrecv2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ mixbufrecv2.cc $(BUILD)

mixbufsend2: mixbufsend2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ mixbufsend2.cc $(BUILD) 

bufsend: bufsend.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufsend.cc $(BUILD)

bufcyc: bufcyc.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufcyc.cc $(BUILD) 

daqhash: daqhash.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqhash.cc $(BUILD)

daqstream: daqstream.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqstream.cc $(BUILD) 

daqqueue: daqqueue.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqqueue.cc $(BUILD)

cpyouttest: cpyouttest.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ cpyouttest.cc $(BUILD) 

include::

lib::

prog::

install::

clean::
	rm -f *.o

realclean: clean
	rm -f $(prog_files)
