SPECTRO_FLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --cflags)
SPECTRO_LDFLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --libs)

CXXFLAGS=$(SPECTRO_FLAGS) -I/opt/daq/Include -I/usr/X11/include
LDFLAGS = $(SPECTRO_LDFLAGS) -L /opt/daq/Lib -L/usr/X11/lib \
	 -lEventFramework -ltk -ltcl -lm -lXm -lXt

TclExample: TclExample.o
	g++ -o TclExample $< $(LDFLAGS)

TclExample.o: TclExample.cpp
	g++ -c $< $(CXXFLAGS) 
