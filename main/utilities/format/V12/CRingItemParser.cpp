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
        // only compare the lower 15 bits for consistency
        if ( (item.type() & 0x7fff) != (type & 0x7fff) ) {
            return false;
        }
    }
    return true;
}


} // end Parser namespace
} // end V12 namespace
} // end DAQ namesapce

