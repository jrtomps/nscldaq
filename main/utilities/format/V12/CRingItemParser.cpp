/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
        Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/


#include <V12/CRingItemParser.h>


namespace DAQ {
namespace V12 {
namespace Parser {

bool isComposite(uint32_t type) {
    return ((0x8000 & type) == 0x8000);
}


bool isTypeConsistent(CRingItem& item, uint32_t type)
{
    if (item.isComposite()) {
        auto& compositeItem = dynamic_cast<CCompositeRingItem&>(item);
        for (auto pChild : compositeItem.getChildren()) {
            if (! isTypeConsistent(*pChild, type) ) {
                return false;
            }
        }
    } else {
        if ( ! isTypeConsistent(item.type(), type) ) {
            return false;
        }
    }
    return true;
}

// only compare the lower 15 bits for consistency
bool isTypeConsistent(uint32_t type1, uint32_t type2) {
    return ( (type1 & 0x7fff) == (type2 & 0x7fff) );
}

} // end Parser namespace
} // end V12 namespace
} // end DAQ namesapce

