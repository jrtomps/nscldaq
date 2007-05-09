/*
    This software is Copyright by Ron Fox
    Copyright (c) 2005, all rights reserved.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     1918 Pinecrest Dr.
	     East Lansing, MI 48823
	     fox@kendo.msu.edu
*/

// This is the code for a usb permissions daemon.
// The program runs periodically looking for usb devices by a particular
// manufacturer (defaults to Wiener Plein & Baus), 
// and sets the device special files for that device to world
// mode 0666, the purpose of this is to allow non-root users access to the
// device.

// One wrinkle.  libusb will give us a directory and dev special filename
// relative to some base which for e.g. Debian is /proc/bus/usb
// For other distros of linux this may be something else.
// In addition, just for the heck of it, we may want to allow
// the software to work for other vendor's devices...since the world is full
// of usb these days... so the usage is:
//
//   wienerusbd [--vendor=id] [--root=directoryroot] [--poll=seconds] \
//              [--debug]
//   Where:
//     -vendor=id specifies the USB Vendor id to look for (defaults to
//                0x16dc which is Wiener P&B.
//     -root=directoryroot specifies the path to the directory tree that
//                defines where the usb devices live. defaults to
//                /proc/bus/usb
//     --poll=seconds Defines how often this daemon runs. defaults to 5 seconds.
//     --debug   If present causes copious debugging output to be emitted to stdout.
//               note that if --debug is not present, the program
//               will fork off as a daemon.
//

// Headers

#include <usb.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "cmdline.h"

using namespace std;

// Constants

const int    defaultId(0x16dc);	           // Default vendor Id to look for.
const string defaultRoot("/proc/bus/usb"); // Default root of usb device tree.
const unsigned int defaultPolltime(5);     // Default polling interval.
const bool   defaultDebug(false);           // Default state of debugging. 

/*!
   setMode - set the mode on a device

   \param root
     root of usb dev special tree.
   \param bus
     Bus the device lives in.
   \param device
     Pointer to the usb_device structure for the device.
*/
void setMode(string root, struct usb_bus* bus, struct usb_device* device,
	     bool debug)
{
  string special = root;
  special += "/";
  special += bus->dirname;
  special += "/";
  special += device->filename;
  
  if (debug) {
    cerr << "Setting mode of " << special << " to 0666" << endl;
  }

  int status = chmod(special.c_str(), 0666);	// Probably should use symbolic perms...
  if (status < 0) {
    cerr << "Failed to chmod " << special << " : " << strerror(errno) << endl;
  }
}


/*!
  Main loop
  \param id
     USB id we're looking for
  \param root
     Root of the USB device special file.
  \param polltime
     Number of seconds betwee polls.
  \param debug
     Enable debug.
*/   
void MainLoop(int id, string root, unsigned int polltime,
	      bool debug) 
{
  while (1) {
    if (debug) {cerr << "Starting a pass" << endl;}
    if (debug) {cerr << "Locating busses" << endl;}
    if (usb_find_busses() < 0) {
      cerr << "usb_find_busses failed " << strerror(errno) << endl;
    }
    if(usb_find_devices()) {
      cerr << "usb_find_devices failed: " << strerror(errno) << endl;
    }
    struct usb_bus* busses = usb_get_busses();
    while (busses) {
      if (debug) {cerr << "Enumerating devices on a bus" << endl;}
      struct usb_device* device = busses->devices;
      while(device) {
	int devId = device->descriptor.idVendor;
	if (debug) {
	  cerr << hex << "Device id = " << devId << " matching for " << id 
	       << dec << endl;
	}
	if(devId == id) {
	  if (debug) {cerr << "Matched!" << endl;}
	  setMode(root, busses, device, debug);
	}
	device = device->next;
      }
      busses = busses->next;
    }
    cerr.flush();
    sleep(polltime);
  }
}

// Entry point (process commands).

int 
main(int argc, char** argv)
{
  usb_init();
  int          id(defaultId);
  string       root(defaultRoot);
  unsigned int pollTime(defaultPolltime);
  bool         debug(defaultDebug);

  struct gengetopt_args_info info;
  if (cmdline_parser(argc, argv, &info) < 0) {
    exit( -1);
  }

  if(info.vendor_given) {
    id = info.vendor_arg;
  }
  if(info.root_given) {
    root = info.root_given;
  }
  if(info.poll_given) {
    pollTime = info.poll_arg;
  }
  if(info.debug_given) {
    debug = info.debug_flag;
  }
  if (debug) {
    cerr << "Running as follows: " << endl;
    cerr << "    vendor id: " << hex << id << dec << endl;
    cerr << "    Root dir:  " << root << endl;
    cerr << "    Poll time: " << pollTime << endl;
    cerr << "    debug:     " << (debug ? "on" : "off") << endl;
  }

  if(!debug) {
    daemon(0, 0);
  }
  MainLoop(id, root, pollTime, debug);
}
