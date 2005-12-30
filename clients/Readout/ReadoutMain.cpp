/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

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

static char* Copyright =
"(c) Copyright NSCL All rights reserved 1999 ReadoutMain.cpp\n";

//
// Include files:
//
#include <config.h>
#include <daqdatatypes.h>
#include <ReadoutStateMachine.h>
#include <daqinterface.h>
#include <errno.h>
#include <Iostream.h>
#include <stdlib.h>
#include <spectrodaq.h>
#include <NSCLException.h>
#include <string>
#include <DesignByContract.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace DesignByContract;
//
//   Global Variables:
//


extern int UsingVME;
void* pCamac;

class DAQBuff : public DAQROCNode {
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
  try {
#ifdef VME
    UsingVME=1;
#else
    UsingVME=0;
#endif
    
    //
    ios::sync_with_stdio();

    // Branchinit will access the camac.
    

    
    // 4. Instantiate a ReadoutStateMachine:
    //
    ReadoutStateMachine Run;
    gpStateMachine = &Run;
    
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
    catch (DesignByContractException& rDesignViolation) {
      string message = string(rDesignViolation);
      cerr << "Interface contract violation: " << rDesignViolation << endl;
    }
    catch(char* pErrorMessage) {
      cerr << "Exception caught at main level: " << pErrorMessage << endl;
    }
    catch(NSCLException& except) {
      cerr << "NSCLException caught at main level " <<
	except.GetErrorString() << " " <<
	except.GetContextString() << endl;
    }
    catch (string& error) {
      cerr << "A string exception was caught: " << error << endl;
    }
    catch(...) {
      cerr << "Some other exception occured and was caught at main level" 
	   << endl;
    }
  }
  catch (string& error) {
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
  
