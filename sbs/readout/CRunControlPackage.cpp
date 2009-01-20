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

#include <config.h>
#include "CRunControlPackage.h"
#include "RunState.h"
#include "CExperiment.h"
#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>

using namespace std;

// Static member data:

CRunControlPackage*  CRunControlPackage::m_pInstance(0);



/*!
  Constructs the singleton.  This is private so that nobody
  but getInstance() can invoke it.
  \param CTCLInterpreter& interp Interpreter to which the commands will be bound:

*/

CRunControlPackage::CRunControlPackage(CTCLInterpreter* pInterp) :
  m_pTheState(RunState::getInstance()),
  m_pTheExperiment(0)		// Need to figure this one out.
{
}
/*!
   I think I May need a destructor to avoid 
