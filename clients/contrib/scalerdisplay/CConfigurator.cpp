
#include <TCLInterpreter.h>
#include "CDigitizerDictionary.h"
#include "CModuleCommand.h"
#include "CCAENV775Creator.h"
#include "CCAENV785Creator.h"
#include "CCAENV792Creator.h"
#include "CCAENV830Creator.h"
#include "CPacketCreator.h"

/*!
   Setup the structure needed to parse the
   hardware configuration commands.
*/

void SetupConfigurator(Tcl_Interp* interp)
{
  CTCLInterpreter*   pInterp = new CTCLInterpreter(interp);

  pInterp->Eval("set Scaler true"); // Set scaler conditional.

  // The readout modules and dictionary:

  CDigitizerDictionary* pRdoDict = new CDigitizerDictionary;
  CModuleCommand*       pModule  = new CModuleCommand(pInterp,
						      pRdoDict);
  // The readout module creators:

  pModule->AddCreator(new CCAENV775Creator);
  pModule->AddCreator(new CCAENV785Creator);
  pModule->AddCreator(new CCAENV792Creator);
  pModule->AddCreator(new CPacketCreator);

  // Create a parallel infrastructure for the scaler module
  // configuration (that's what we're interested in.

  CDigitizerDictionary* pSclDict  = new CDigitizerDictionary;
  CModuleCommand*       pScalers  = new CModuleCommand(pInterp,
						       pSclDict,
						       string("scaler"));

  pScalers->AddCreator(new CCAENV830Creator);

}
