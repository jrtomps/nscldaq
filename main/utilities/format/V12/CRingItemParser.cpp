#include <V12/CRingItemParser.h>


namespace DAQ {
namespace V12 {
namespace Parser {

bool isCompound(uint32_t type) {
    return ((0x8000 & type) == 0x8000);
}

}
}
}

