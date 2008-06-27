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
#include "StringsToIntegers.h"
#include <stdio.h>
#include <stdlib.h>

using namespace std;
////////////////////////////////////////////////////////////////////////////////
/*!
 *  This header rovides an unbound function that takes a comma separated
 *  list of integer (in string form) and converts them into a 
 *  vector of ints.
 *  This is most useful in decodin things like:
 *  
 * \verbatim
 *   ... --exclude=1,2,3 ...
 * \endverbatim
 *
 * \param items  - Stringified comma separated list of integers.
 * \return std::vector<int>
 * \retval Ordered vector of the integers decoded from the string.
 * \throw CInvalidArgumentException
 */
vector<int>
stringListToIntegers(string items) throw(CInvalidArgumentException)
{
    size_t      start = 0;
    vector<int> result;
    
    while(size_t comma = items.find(string(","), start) != string::npos) {
        string aNumber;
	aNumber.assign(items, start, comma - start);
        char *end;
        int  value = strtol(aNumber.c_str(), &end, 0);
        if (*end != '\0') {
            string whyBad  = " must be an integer but was ";
            whyBad        += aNumber;
            throw CInvalidArgumentException(aNumber, whyBad,
					    string("Converting a list to integers"));
        }
	result.push_back(value);
    }
    return result;
}
