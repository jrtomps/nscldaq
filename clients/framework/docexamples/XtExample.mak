SPECTRO_FLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --cflags)
SPECTRO_LDFLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --libs)

CXXFLAGS=$(SPECTRO_FLAGS) -I/opt/daq/Include -I/usr/X11/include
LDFLAGS = $(SPECTRO_LDFLAGS) -L /opt/daq/Lib -L/usr/X11/lib \
	 -lEventFramework -ltk -ltcl -lm -lXm -lXt

XtExample: XtExample.o
	g++ -o XtExample $< $(LDFLAGS)

XtExample.o: XtExample.cpp
	g++ -c $< $(CXXFLAGS) -ltcl 