/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


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

static const  char* Copyright =
"(c) Copyright NSCL All rights reserved 1999 ReadoutMain.cpp\n";

//
// Include files:
//
#include <config.h>

using namespace std;


#include <daqdatatypes.h>
#include <ReadoutStateMachine.h>
#include <daqinterface.h>
#include <errno.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <spectrodaq.h>
#include <NSCLException.h>
#include <string>
#include <DAQROCNode.h>
//
//   Global Variables:
//


string SetupFile;

extern int UsingVME;
void* pCamac;

class DAQBuff : public DAQROCNode {
protected:
  int operator()(int argc, char** argv);
public:
  bool SetProcessTitle(const char* pTitle) {
    DAQROCNode::SetProcessTitle( pTitle);
  }
};

ReadoutStateMachine* gpStateMachine;
int 
DAQBuff::operator()(int argc, char**argv)
{
    
  // 4. Instantiate a ReadoutStateMachine:
  //
  ReadoutStateMachine Run;
  gpStateMachine = &Run;

  if(argc != 2) {
    cerr << "Usage:\n";
    cerr << "   Readout config-file\n";
    cerr << "       config-file is a tcl configuration file that\n";
    cerr << "       describes what is read out and how the trigger works\n";
    return -1;
  }
  SetupFile = argv[1];

  try {
#ifdef VME
    UsingVME=1;
#else
    UsingVME=0;
#endif
    
    //
    ios::sync_with_stdio();

    // Branchinit will access the camac.
    


    
    //  The line below can be modified to set the buffersize.
    // note that the buffersize is set in words not bytes
    //
    daq_SetBufferSize(4096);
    
    //
    // 5. Run the state machine:
    //
    try {
      Run.Run(Run.NameToState("INACTIVE"));
    }
    catch(char* pErrorMessage) {
      cerr << "Exception caught at main level: " << pErrorMessage << endl;
    }
    catch(string e) {
      cerr << "Exception caught at main level: " << e << endl;
    }
    catch(NSCLException& except) {
      cerr << "NSCLException caught at main level " <<
	except.GetErrorString() << " " <<
	except.GetContextString() << endl;
    }
    catch(...) {
      cerr << "Some other exception occured and was caught at main level" 
	   << endl;
    }
  }
  catch (string error) {
    cout << "A VME Mapping exception occured: "
	 << error << endl;
    exit(-1);
  }
  //
  // 6. Exit when the state machine does:
  //
  cerr << "Run state machine has exited, now so will we:"
       << endl;
  cerr.flush();
  

  exit(0);

}
DAQBuff mydaq;
  
void* gpTCLApplication(0);

#ifdef HAVE_SPECTRODAQ_MAIN
int
main(int argc, char** argv, char** envp) 
{
  return spectrodaq_main(argc, argv, envp);
}


#endif

