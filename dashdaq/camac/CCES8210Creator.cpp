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
#include <CCES8210Creator.h>
#include <parseUtilities.h>
#include <CCESCBD8210.h>
#include <CBiRA1302CES8210.h>
#include <RangeError.h>
#include <CBadValue.h>

#include <stdlib.h>
#include <errno.h>

#include <map>


#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace descriptionFile;

// Construction is a no-op for now.

CCES8210Creator::CCES8210Creator()
{}

// As is destruction.

CCES8210Creator::~CCES8210Creator()
{}

/*! 
   Creating an interface requires parsing its configuration line.
   See the comments in the class header for a descrition of the valid
   keyword /value pairs.
*/
CCAMACInterface* 
CCES8210Creator::operator()(string configuration)
{
  // Default the VME crate and branch, and provide a crate map.
  // we use a map to allow duplicate crate definitions.

  unsigned int vmeCrate(0);
  unsigned int branchNumber(0);
  map<unsigned int, unsigned int> cratesRequested;

  // The configuration file is now parsed for keyword/value pairs.
  // The local function nextPair modifies the line to strip the pair from it.

  configuration = stripLeadingBlanks(configuration);
  while(configuration.size() != 0) {
    pair<string,string> kvPair = getKeywordValue(configuration);

    // In all cases the value must be an integer:
    string keyword = kvPair.first;
    long value     = strtol(kvPair.second.c_str(), NULL, 0);
    if ((value == 0) && (errno == EINVAL)) {
      throw CBadValue("Required an integer", kvPair.second.c_str(),
		      "Converting value from keyword value pair");
    }
    if (keyword == string("-vme")) {
      vmeCrate = value;
    }
    else if (keyword == string("-branch")) {
      if ((value < 0) || (value > 7)) {
	throw CRangeError(0, 7, value, "Processing -branch in CCES8210Creator");
      } 
      else {
	branchNumber = value;
      }
    }
    else if (keyword == string("-crate")) {
      if ((value < 1) || (value > 7)) {
	throw CRangeError(1,7, value, "Processing -crate in CCES8210Creator");
      }
      else {
	cratesRequested[value] = value;
      }
    }
    else {
      throw CBadValue("Required -vme, -branch, or -crate", keyword.c_str(),
		      "Processing CES8210 keywords");
    }

  }

  // We made it this far.  We can create the branch highway driver
  // object and add crates to it:

  CCESCBD8210* pBranch = new CCESCBD8210(vmeCrate, branchNumber);
  map<unsigned int, unsigned int>::iterator i = cratesRequested.begin();
  while (i != cratesRequested.end()) {
    CBiRA1302CES8210* pCrate = new CBiRA1302CES8210(*pBranch,
						    vmeCrate,
						    i->first);
    pBranch->addCrate(*pCrate, i->first);
    i++;			// Next crate...
  }
  return pBranch;  
  
}
