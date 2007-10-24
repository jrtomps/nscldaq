#ifndef __ASSEMBLERUTILITIES_H
#define __ASSEMBLERUTILITIES_H
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
#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_ECTOR
#endif
#endif

static const size_t BUFFERSIZE(16*1024);        // Size of a data buffer.

class CTCLObject;
class CTCLInterpreter;

/*!
 * This class has a few re-usable utility functions and definitions
 */

class AssemblerUtilities
{
public:
    typedef std::pair<uint16_t, uint32_t> typeCountPair;

public:
	static
	std::vector<typeCountPair> makeTypeCountVector(const uint32_t* statistics, 
			                       				   size_t          size) const;
	static
	CTCLObject* typeValuePairToList(CTCLInterpreter& interp,
					std::vector<typeCountPair>& stats);
};

#endif /*ASSEMBLERUTILITIES_H_*/
