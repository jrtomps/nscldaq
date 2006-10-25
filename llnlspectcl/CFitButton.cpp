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
#include "CFitButton.h"
#include "ButtonEvent.h"
#include <Xamine.h>
#include <CGaussianFit.h>
#include <CSpectrumFit.h>
#include <CFitDictionary.h>
#include <Histogrammer.h>
#include <SpecTcl.h>
#include <Spectrum.h>
#include <CFitCommand.h>

#include <clientops.h>
#include <string>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <iostream>		// For debugging.

/*!
  The constructor must create the button as directed int he Xamine
   button box.  We assume that someone else has already created the button
   box with the appropriate dimensions to handle our button location.
   \param id  : int
      Id associated with the button.  This must be unique amongst all of the
      buttons in the button box.  It is what allows us to recognize our button
      event amongst any other button events that have occured.
   \param row, column : int
       The coordinates of the button in the button box.
   \param pEventHandler : CXamineEventHandler*
      Pointer to the xamine event handler (we register ourselvse on this).

*/
CFitButton::CFitButton(int id, int row, int column,
		       CXamineEventHandler* pEventHandler) :
  m_buttonId(id)
{
  // Fill in the button description to match what we want:

  ButtonDescription myButton;
  myButton.button_code   = id;
  strcpy(myButton.label, "Gauss Fit");
  myButton.type          = Push;
  myButton.sensitive     = T;
  myButton.prompter      = Points;
  myButton.whenavailable = In1dSpectrum;
  strcpy(myButton.promptstr, "Fit limits"); // Ingored?
  myButton.spectype      = Oned;            // I think this is ignored for this prompter
  myButton.minpts        = 2;
  myButton.maxpts        = 2;               // need exactly 2 pts.

  Xamine_DefineButton(row, column, &myButton);

  pEventHandler->addButtonHandler(*this);
  
  
  
}
/*!
   The destructor does nothing for now, however note that a call to the
   destructor is really really bad since at present, there's no way to
   unregister us as a button event handler!!
*/
CFitButton::~CFitButton() {}
/*
   The button handler is called when \em any button in the button box
   sends its message back to SpecTcl.  We need to determine if we can
   successfully process the message.  If so return true if not,
   false so that other handlers can try.

   \param event : CButtonEvent&
      Reference to an object that describes the button event.

   \return Bool_t
   \retval  kfTRUE - we processed the event, no need for other handlers to fire.
   \retval  kfFALSE - we did not process the event, continue checking handlers.
*/

Bool_t 
CFitButton::operator()(CButtonEvent& event)
{
  // If this is not our button, return false right away:

  if (m_buttonId != event.getId()) {
    return kfFALSE;
  }
  // Before we can fit, we need to get:
  // - The name of the fit.
  // - The name of the spectrum.
  // - The fit points.

  string         fitName       = event.getm_sPromptedString();
  PointArray     pts           = event.getPoints();
  int            bindId        = event.getPromptedSpectrum();
  SpecTcl*       pApi          = SpecTcl::getInstance();
  CHistogrammer* pHistogrammer = pApi->GetHistogrammer();
  CSpectrum*     pSpectrum     = pHistogrammer->DisplayBinding(bindId-1);
  string         spectrumName  = pSpectrum->getName();

  // Now we can create the fit:

  CGaussianFit*  pFit  = new CGaussianFit(fitName, CFitCommand::id());
  int            low   = pts[0].X();
  int            high  = pts[1].X();
  if (low > high) {
    int temp = low;
    low      = high;
    high     = temp;
  }
  // This is in a try/catch block in case the user did the truly 
  // pathalogical thing of deleting the spectrum just as they
  // accepted the fit ... kids these days.
  //
  CSpectrumFit*   pSpectrumFit;
  try {
      pSpectrumFit = new CSpectrumFit(spectrumName,
				      pFit->getNumber(),
				      low, high, *pFit);
  }
  catch (...) {
    delete pFit;		// Just abort the operation silently.
    return kfTRUE;
  }

  // we use the addOrReplace function of the fit dictionary:

  CFitDictionary& dict(CFitDictionary::getInstance());
  dict.addOrReplace(*pSpectrumFit);




  return kfTRUE;
}
