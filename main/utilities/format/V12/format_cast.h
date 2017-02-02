#ifndef DAQ_V12_FORMAT_CAST_H
#define DAQ_V12_FORMAT_CAST_H

#include <V12/CRawRingItem.h>
#include <V12/CRingItem.h>
#include <typeinfo>
#include <iostream>

namespace DAQ {
namespace V12 {


template<class T>
T format_cast(const CRingItem& item) {
    auto& rawItem = dynamic_cast<const CRawRingItem&>(item);
    return T(rawItem);
}

template<> CRawRingItem format_cast(const CRingItem& item) {
    CRawRingItem rawItem;
    item.toRawRingItem(rawItem);
    return rawItem;
}

} // end V12
} // end DAQ

#endif // DAQ_V12_FORMAT_CAST_H
