SPECTRO_FLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --cflags)
SPECTRO_LDFLAGS=$(shell /opt/spectrodaq/bin/spectrodaq-conf --libs)

CXXFLAGS=$(SPECTRO_FLAGS) -I/opt/daq/Include -I/usr/X11/include
LDFLAGS = $(SPECTRO_LDFLAGS) -L /opt/daq/Lib -L/usr/X11/lib \
	 -lEventFramework -ltk -ltcl -lm -lXm -lXt

locationex: locationex.o
	g++ -o locationex $< $(LDFLAGS)

locationex.o: locationex.cpp
	g++ -c $< $(CXXFLAGS)