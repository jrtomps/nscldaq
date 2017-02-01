
#include <V12/CRingItem.h>
#include <V12/DataFormat.h>
#include <iostream>
#include <sstream>


namespace DAQ {
namespace V12 {

std::string headerToString(const DAQ::V12::CRingItem &item) {
    std::ostringstream result;
    result << "Size (bytes): " << item.size() << std::endl;
    result << "Type:         " << item.typeName() << std::endl;
    result << "Timestamp:    " << item.getEventTimestamp() << std::endl;
    result << "Source Id:    " << item.getSourceId()  << std::endl;

    return result.str();
    }




} // end V12 namespace
} // end DAQ namespace
