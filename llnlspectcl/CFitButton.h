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

#ifndef __CFITBUTTON_H
#define __CFITBUTTON_H

#ifndef __XAMINEEVENTHANDLER_H
#include <XamineEventHandler.h>	// For base class.
#endif

class CButtonEvent;

/*!
   This class defines a subclass of CXamineEventHandler::CButtonHandler.
   - The constructor of this class creates a point prompt button that is
     only enabled on 1-d spectra, and requires exactly 2 points (left/right limits).
   - We register ourselves as a button hander with XamineEventHandler as well.
   - When events come in, we use the information in the button event (if it's
     for us) to create a Gaussian fit on that spectrum, and add the fit to the
     Fit dictionary.. the observers on the dictionary, in turn will ensure that
     the fit is added to the display as well... furthermore, since the fit
     command operates off the spectrum dictionary, fit list etc. will show
     the fit.  We'll execute the appropriate fit list command as well
     to ensure that the data about the fit get output to the tkcon window.

*/
class CFitButton : public CXamineEventHandler::CButtonHandler
{
  // Member data.
private:
  int   m_buttonId;		// Id assigned to button.

  // constructors and canonicals.. note that copy-like ops are forbidden.
public:
  CFitButton(int id, int row, int column, CXamineEventHandler* pHandler);
  virtual ~CFitButton();
private:
  CFitButton(const CFitButton& rhs);
  CFitButton& operator=(const CFitButton& rhs);
  int operator==(const CFitButton& rhs) const;
  int operator!=(const CFitButton& rhs) const;

  // The following is called when the button is successfully used:

  virtual Bool_t operator()(CButtonEvent& event);
 
};



#endif
