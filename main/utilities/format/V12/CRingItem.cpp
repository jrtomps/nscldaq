
#include <V12/CRingItem.h>
#include <V12/DataFormat.h>
#include <iostream>
#include <sstream>


namespace DAQ {
namespace V12 {

std::string headerToString(const DAQ::V12::CRingItem &item) {
    std::ostringstream result;
    result << "Size (bytes) : " << item.size() << std::endl;
    result << "Type         : " << item.typeName() << std::endl;
    uint64_t tstamp = item.getEventTimestamp();
    if (tstamp != NULL_TIMESTAMP) {
        result << "Timestamp    : " << tstamp << std::endl;
    } else {
        result << "Timestamp    : NULL_TIMESTAMP" << std::endl;
    }
    result << "Source Id    : " << item.getSourceId()  << std::endl;

    return result.str();
    }




} // end V12 namespace
} // end DAQ namespace
