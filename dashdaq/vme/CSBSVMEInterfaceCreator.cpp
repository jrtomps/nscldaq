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
#include "CSBSVMEInterfaceCreator.h"
#include "CSBSVMEInterface.h"
#include "parseUtilities.h"

#include <stdio.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace descriptionFile;

/*!
   Create a vme interface connected to an SBS Bit3 PCI/VME bus bridge.
   
   \param type : string
     The interface type (ignored).
   \param configuration : string
     The configuration string.  In this case there should be a single
     integer that represents the sbs interface number.
*/
CVMEInterface*
CSBSVMEInterfaceCreator::operator()(string type, string configuration)
{
  // Get the first word, and ensure there are no more.

  configuration = stripLeadingBlanks(configuration);
  string crate  = firstWord(configuration);
  configuration = configuration.substr(crate.size());
  configuration = stripLeadingBlanks(configuration);

  // The crate must be nonempty, and configuration must be empty

  if (crate == string("")) {
    return static_cast<CSBSVMEInterface*>(NULL);
  }
  if (configuration != string("")) {
    return static_cast<CSBSVMEInterface*>(NULL);
  }

  // The crate string must be an integer.

  int ncrate;
  if (sscanf(crate.c_str(), "%d", &ncrate) != 1) {
    return NULL;
  }
  return new CSBSVMEInterface(ncrate);

}
