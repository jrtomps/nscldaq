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

// Implementation of the C785 class VM-USB support for the CAEN V785.


#include <config.h>
#include <CHiRA.h>
#include "CReadoutModule.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <Globals.h>
#include <CConfiguration.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

#include <iostream>
using namespace std;

// Limit the -id to [0-0xffff].




/*-------------------------------------------------------------------------*
 * Canonicals                                                              *
 *------------------------------------------------------------------------*/


/**
 * CHiRA():
 *    Null out the m_pConfiguration pointer which gets filled in by
 *    onAttach.
 */
CHiRA::CHiRA() :
  m_pConfiguration(0)
{}
/**
 * ~CHiRA():
 *    No-op and yes this does mean that memory can leak if the caller does not
 *    delete the configuration if it was dynamic.
 */

CHiRA::~CHiRA() {}

/*----------------------------------------------------------------------*
 * Object methods:                                                      *
 *---------------------------------------------------------------------*/

/**
 * onAttach(configuration)
 *
 *   Called to associated a configuration object with the configuration of
 *   this module.  The configuration parameters we speak are defined
 *   and a pointer to the configuration retained so that 
 *   we can query it after it has been filled in.
 *
 * param rConfig       - Reference to the configuration that is associated
 *                       with *this.
 */
void
CHiRA::onAttach(CReadoutModule& rConfig)
{
  m_pConfiguration = &rConfig;

  // We want a integer parameter for the id

  m_pConfiguration->addIntegerParameter("-id", 0, 0xffff, 0x618a);

  // Next we need both an -xlm and -fadc module which require the parameter be
  // a defined module our static isModule checks that.
  // we're not going to do type checking here.. so this is really a 'module-pair' device.
  //

  m_pConfiguration->addParameter("-xlm", &CHiRA::isModule, NULL);
  m_pConfiguration->addParameter("-fadc", &CHiRA::isModule, NULL);

  

  
}
/**
 * CHiRA::Initialize
 * Peform pre-start run initializations.  This just invokes the sam e methods in the
 * xlm and fadc modules 
 *
 * @param controller - reference to the VM-USB controller object.
 *
 */
void
CHiRA::Initialize(CVMUSB& controller)
{
  // Fetch the configuration:

  std::string xlm  = m_pConfiguration->cget("-xlm");
  std::string fadc = m_pConfiguration->cget("-fadc");

  // If we can't get a module that's an error:

  CConfiguration* pConfig = Globals::pConfig;
  CReadoutModule* pXLM = pConfig->findAdc(xlm);
  if (!pXLM) {
    throw std::string("-xlm must define a module and does not");
  }
  CReadoutModule* pFADC = pConfig->findAdc(fadc);
  if(!pFADC) {
    throw std::string("-fadc must define a module and does not");
  }

  // Init the xlm and the fadc:

  pXLM->Initialize(controller);
  pFADC->Initialize(controller);
  
}
/**
 * CHiRA::addReadoutList:
 *    Adds entries to the readout list to
 *    - Insert an id value.
 *    - Read the XLM.
 *    - Read the FADC.
 *
 * @param list - Reference to the readout list that is bgeing built up for this experiment.  
 */
void
CHiRA::addReadoutList(CVMUSBReadoutList& list)
{
  // By now we know tht the xlm and fadc switches are good since Initialize checked
  // them:


  // Fetch the configuration:

  std::string xlm  = m_pConfiguration->cget("-xlm");
  std::string fadc = m_pConfiguration->cget("-fadc");
  int         id   = m_pConfiguration->getIntegerParameter("-id");

  // Get the modules:

  CConfiguration* pConfig = Globals::pConfig;
  CReadoutModule* pXLM = pConfig->findAdc(xlm);
  CReadoutModule* pFADC = pConfig->findAdc(fadc);
  
  // The list we generate is our id value,
  // the XLM list and then the FADC list:

  list.addMarker(id);
  pXLM->addReadoutList(list);
  pFADC->addReadoutList(list);
 
}
/*------------------------------------------------------------------------*
 * Utility methods (private)                                              *
 *-----------------------------------------------------------------------*/

/**
 * Check that a parameter value is a defined module.
 *
 * @param name   - Option name (e.g. -fadc)
 * @param value  - Value of the option.
 * @param pArg   - Application specific parameter (not used).
 *
 * @return bool
 * @retval true if the parameter is a module.
 */
bool
CHiRA::isModule(std::string name, std::string value, void* pArg)
{
  CConfiguration* pConfiguration = Globals::pConfig;

  return (pConfiguration->findAdc(name) != NULL);

}
