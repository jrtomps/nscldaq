SPECTRO_FLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --cflags)
SPECTRO_LDFLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --libs)

CXXFLAGS=-ggdb $(SPECTRO_FLAGS) -I/opt/daq/Include -I/usr/X11/include
LDFLAGS = -ggdb $(SPECTRO_LDFLAGS) -L /opt/daq/Lib -L/usr/X11/lib \
	 -lEventFramework -ltk -ltcl -lm -lXm -lXt

buffers: buffers.o
	g++ -o buffers $< $(LDFLAGS)

buffers.o: buffers.cpp
	g++ -c $< $(CXXFLAGS)