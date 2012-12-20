//  ReadoutMain.cpp:
//     This file is the version 0.1 readout control main program.
//     we must:
//       1. Open the VME device driver in A24/D16 mode.
//       2. Ensure that the VME crate is online.
//       3. Map to the CAMAC crate and save the base address as a global
//          variable named CAMBAS.
//       4. Instantiate a ReadoutStateMachine object.
//       5. Run it.
//       6. Exit when the state machine exits.
//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//    mailto:fox@nscl.msu.edu
//
//////////////////////////////////////////////////////////////////////////


static const char* Copyright =
"(c) Copyright NSCL All rights reserved 1999 ReadoutMain.cpp\n";

//
// Include files:
//
#include <errno.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include "TCLThread.h"
#include <TCLApplication.h>
#include <TCLInterpreter.h>
#include <Exception.h>
#include <tcl.h>

#include <string>
//
//   Global Variables:
//

extern int UsingVME;
void* pCamac;

//UINT32  CAMBAS = 0;			// Base address of camac space.
//int     nCamacFd;

// External Prototypes
extern void InitChip(int MBNO);




int
main(int argc, char**argv)
{

  TCLTK  interp;
  try {

  // Figure out where the script is and
  // start the interp so that it will source it in.
  // INSTDIR is a -D for to level directory tree.
  //

  std::string script(INSTDIR);
  script += "/TclLibs/CHIP/silstrip.tcl";

  const char* TclArgs[]={"CHIP", script.c_str()};
  interp.Start(2,const_cast<char**>(TclArgs));

  while(1)
    sleep(100);
  }
  catch (std::string message) {
    std::cerr << "Caught string exception: " << message << std::endl;
  }
  catch (const char* message) {
    std::cerr << "Caught char* exception: " << message << std::endl;
  }
  catch (CException& e) {
    std::cerr << "Caught CException: " << e.ReasonText() << std::endl;
    std::cerr << e.WasDoing() << std::endl;
  }
  catch (...) {
    std::cerr << "last chance handler - unanticipated exception type\n";
    throw;
  }
}

