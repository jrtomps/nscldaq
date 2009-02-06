#ifndef __CREADOUTMAIN_H
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


#ifndef __TCLAPPLICATION_H
#include <TCLApplication.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef OPTIONS_H
#include "options.h"
#endif

// Forward class definitions.

class CTCLServer;
class CTCLInterpreter;
class CExperiment;

/*!

  This class is the entry point class for the
  production readout software.   It is  subclassed
  (see Skeleton.cpp) by the user who must provide the
  SetupReadout virtual member.

*/

class CReadoutMain : public CTCLApplication
{
  CTCLServer*   m_pTclServer;
  CExperiment*  m_pExperiment;
public:
  CReadoutMain();
  virtual ~CReadoutMain();


  // Selectors:

  CTCLServer*  getTclServer();
  CExperiment* getExperiment();

  // Entry point

  virtual int operator()();
protected:
  virtual CExperiment*  CreateExperiment(void* parsed);
  virtual void          SetupRunVariables(CTCLInterpreter* pInterp);
  virtual void          SetupStateVariables(CTCLInterpreter* pInerp);
  virtual void          SetupReadout(CExperiment* pExperiment);
  virtual void          SetupScalers(CExperiment* pExperiment);
  virtual void          addCommands();

protected:
  void startTclServer(std::string port);
  std::string getRingName(struct gengetopt_args_info& arguments);
};

#endif
