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

#ifndef __CCES8210CREATOR_H
#define __CCES8210CREATOR_H

#ifndef __CCAMACINTERFACECREATOR_H
#include <CCAMACInterfaceCreator.h>
#endif


/*!
   Create a CES8210 interface stocked with crates.
   The configuration information for this interface is keyword value pairs.
\verbatim
   KEYWORD           value
   -vme              VME crate in which the interface is installed (defaults 0)
   -branch           Branch number set in the front panel. (defaults 0)
   -crate            Number of a camac crate installed in the branch.  This
                     keyword/value pair can be used more than once.

\endverbatim
*/

class CCES8210Creator : public CCAMACInterfaceCreator
{
public:
  CCES8210Creator();
  virtual ~CCES8210Creator();
  virtual CCAMACInterface* operator()(STD(string) configuration);
};


#endif
