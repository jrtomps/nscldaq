//  Simple program that will check/clear the errors on the bit 3 interface.
//  output to stdout will be:
//    0 - no errors.
//    1 - Power outage on VME
//    2 - Status errors.
//    3 - Power just turned on on VME
//    4 - I/O driver error requesting status.
//    5 - Some status we don't know about.
#include <config.h>
#include <CVMEInterface.h>
#include <SBSBit3API.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <btngpci.h>
#include <Exception.h>
using namespace std;

//  Takes one parameter: the vme crate number.

int
main(int argc, char**argv)
{

  // Check the usage:

  if(argc != 2) {
    cerr << "Usage:\n";
    cerr << "   " << argv[0];
    cerr << "  vmecratenumber\n";
    return -1;
  }

  // Attempt to open the crate:

  void* pVmeHandle;
  try {
    int nCrate       = atoi (argv[1]);
    pVmeHandle       = CVMEInterface::Open(CVMEInterface::ShortIO, 
					   nCrate);
  }
  catch (CException& except) {
    cerr << "VME openfailed: " << except.ReasonText() <<endl;
    return -1;
  }
  catch (string msg) {
    cerr << "VME open failed: " << msg << endl;
    return -1;
  }

  // Check errors on the crate, reset and close

  bt_error_t status = CSBSBit3VmeInterface::CheckErrors(pVmeHandle);
  CSBSBit3VmeInterface::ClearErrors(pVmeHandle);
  CVMEInterface::Close(pVmeHandle);

  // analyze the error status:

  switch (status) {
  case BT_SUCCESS:
    cerr << 0 << endl;
    break;
  case BT_ESTATUS:
    cerr << 2 << endl;
    break;
  case BT_ENOPWR:
    cerr << 1 << endl;
    break;
  case BT_EPWRCYC:
    cerr << 3 << endl;
    break;
  case BT_EIO:
    cerr << 4 << endl;
    break;
  default:
    cerr << 5 << endl;
  }
  exit(0);
}
