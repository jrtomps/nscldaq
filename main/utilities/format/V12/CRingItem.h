#ifndef NSCLDAQV12_CRINGITEM_H
#define NSCLDAQV12_CRINGITEM_H

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

#include <cstdint>
#include <string>

namespace DAQ {
    namespace V12 {

    class CRawRingItem;

    class CRingItem {

      public:
        virtual uint32_t type() const = 0;
        virtual void setType(uint32_t type) = 0;

        virtual uint32_t size() const = 0;

        virtual uint64_t getEventTimestamp() const = 0;
        virtual void setEventTimestamp(uint64_t tstamp) = 0;

        virtual uint32_t getSourceId() const = 0;
        virtual void setSourceId(uint32_t id) = 0;

        virtual bool     isComposite() const = 0;

        virtual bool     mustSwap() const = 0;

        virtual std::string typeName() const = 0;
        virtual std::string toString() const = 0;

        virtual void toRawRingItem(CRawRingItem& item) const = 0;
    };

    std::string headerToString(const CRingItem& item);

} // end of V12 namespace
} // end DAQ

#endif
