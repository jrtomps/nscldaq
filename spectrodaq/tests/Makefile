
O_FILES = 

include ../config.make

prog_files = test001_prog test002_prog test003_prog test004_prog \
test005r_prog test005s_prog test006r_prog test006s_prog \
test007r_prog test007s_prog test008r_prog test008s_prog test009r_prog \
test009s_prog test010_prog test011r_prog test011s_prog test012mr_prog \
test012ms_prog

script_files = test001 test002 test003 test004 test005 test006 test007 \
test008 test009 test010 test011

all: prog

prog: $(prog_files) perms

testsp01r: testsp01r.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ testsp01r.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

testsp01s: testsp01s.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ testsp01s.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test001_prog: test001_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test001_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test002_prog: test002_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test002_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test003_prog: test003_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test003_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test004_prog: test004_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test004_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test005r_prog: test005r_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test005r_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test005s_prog: test005s_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test005s_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test006r_prog: test006r_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test006r_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test006s_prog: test006s_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test006s_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test007r_prog: test007r_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test007r_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test007s_prog: test007s_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test007s_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test008r_prog: test008r_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test008r_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test008s_prog: test008s_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test008s_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test009r_prog: test009r_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test009r_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test009s_prog: test009s_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test009s_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test010_prog: test010_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test010_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test011r_prog: test011r_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test011r_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test011s_prog: test011s_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test011s_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test012mr_prog: test012mr_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test012mr_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

test012ms_prog: test012ms_prog.cc $(DAQLIB)
	$(CXX) $(CXXFLAGS) -o $@ test012ms_prog.cc $(NETIOLIB) $(CLIENTLIB) $(DAQLIB) $(DAQTHREADSLIB) $(XMLLIBS) $(THREADLIBS) $(LDFLAGS)

perms: $(script_files)
	chmod u+x,g+x $(script_files)

include::

lib::

install::

clean::
	rm -f *.o

realclean: clean
	rm -f $(script_files)
	rm -f $(prog_files)
