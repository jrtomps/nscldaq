
O_FILES = 


include ../../config.make

prog_files = bufrw alarm trap daqxml pagehead bufcyc bufrecv bufrecv2 bufsend2 bufsend daqencode daqthreads daqqueue daqstream daqbuf daqrpc daqtest daqcom daqhash daqstrtest urbufsend2 urbufrecv2 mixbufrecv2 mixbufsend2 buffree daqfence bufrecv2ur cpysend cpyrecv

all: toys

toys: $(prog_files)

daqfence: daqfence.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqfence.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

pagehead: pagehead.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ pagehead.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqencode: daqencode.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqencode.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqxml: daqxml.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqxml.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqthreads: daqthreads.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqthreads.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqtest: daqtest.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqtest.cc $(NETIOLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqstrtest: daqstrtest.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqstrtest.cc $(NETIOLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqrpc: daqrpc.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqrpc.cc $(NETIOLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqcom: daqcom.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqcom.cc $(NETIOLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

alarm: alarm.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ alarm.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

trap: trap.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ trap.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqbuf: daqbuf.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqbuf.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

bufrw: bufrw.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufrw.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqbuf2: daqbuf2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqbuf2.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

bufrecv: bufrecv.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufrecv.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

bufrecv2: bufrecv2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufrecv2.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

bufrecv2ur: bufrecv2ur.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufrecv2ur.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

bufsend2: bufsend2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufsend2.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

cpyrecv: cpyrecv.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ cpyrecv.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

cpysend: cpysend.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ cpysend.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

urbufrecv2: urbufrecv2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ urbufrecv2.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

urbufsend2: urbufsend2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ urbufsend2.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

buffree: buffree.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ buffree.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

mixbufrecv2: mixbufrecv2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ mixbufrecv2.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

mixbufsend2: mixbufsend2.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ mixbufsend2.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

bufsend: bufsend.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufsend.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

bufcyc: bufcyc.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ bufcyc.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqhash: daqhash.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqhash.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqstream: daqstream.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqstream.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

daqqueue: daqqueue.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ daqqueue.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

include::

lib::

prog::

install::

clean::
	rm -f *.o

realclean: clean
	rm -f $(prog_files)
