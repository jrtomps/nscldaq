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
#include "CVC32Creator.h"
#include <parseUtilities.h>
#include "CWienerVC32.h"
#include "CVC32CC32.h"
#include <CBadValue.h>

#include <utility>		// Why this isn't <pair> beats me.

#include <stdlib.h>
#include <errno.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
using namespace descriptionFile;


/*!
   Construct a VC32 creator:
*/
CVC32Creator::CVC32Creator()
{}

/*!
   Destroy a VC32 creator...
*/
CVC32Creator::~CVC32Creator()
{}

/*!
    Create an interface.  The configuration line consists of
    keyword value pairs.  See the header comments for information about
    those pairs.
*/
CCAMACInterface*
CVC32Creator::operator()(string configuration)
{
  unsigned int vmeCrate(0);
  unsigned int base;
  bool         baseSeen(false);

  // Parse the configuration file for keyword value pairs:

  configuration = stripLeadingBlanks(configuration);
  while (configuration.size() != 0) {
    pair <string,string> kvPair = getKeywordValue(configuration);
    string keyword = kvPair.first;
    long value                  = strtol(kvPair.second.c_str(), NULL, 0);
    if ((value == 0) && (errno = EINVAL)) {
      throw CBadValue("Required an integer", kvPair.second.c_str(),
		      "Converting value from keyword value pair");
    }
    if (keyword == string("-vme")) {
      vmeCrate = value;
    } 
    else if (keyword == string("-base")) {
      base     = value;
      baseSeen = true;
    }
    else {
      throw CBadValue ("Required -vme or -base", keyword.c_str(),
		       "Processing CVC32 keywords");
    }
  }  
  if (!baseSeen) {
    throw CBadValue("Neede a base", "--",
		    "Creating a CVC32");
  }
  // First create the interface:

  CWienerVC32* pInterface = new CWienerVC32(vmeCrate, base);

  // Then add the CAMAC crate to it:

  CVC32CC32*   pCrate    = new CVC32CC32(*pInterface);
  pInterface->addCrate(*pCrate, 0);

  return static_cast<CCAMACInterface*>(pInterface);

}
