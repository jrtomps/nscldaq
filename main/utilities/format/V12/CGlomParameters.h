#ifndef DAQ_V12_CGLOMPARAMETERS_H
#define DAQ_V12_CGLOMPARAMETERS_H
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt
*/

/**
 * @file CGlomParameters.h
 * @brief  Encapsulates a ring item that contains glom parametrs.
 * @author  Jeromy Tompkins
 */

#include "V12/CRingItem.h"

#include <typeinfo>

namespace DAQ {
  namespace V12 {

  class CRawRingItem;

/**
 * @class CGlomParameters
 *
 * Encapsulates a ring item of type EVB_GLOM_INFO.  The structure of this ring
 * item is given by the GlomParameters struct.  No body header is required for
 * this type..though the mbz field is present in case later we decide that
 * was a mistake (e.g. cascaded event building we may have gloms with different
 * parameters at different levels and knowing that by assigning each glom an
 * event source id may be needed).
 */
class CGlomParameters : public CRingItem
{
public:
    // Note the enum values below _must_ match those in DataFormat.h:

    enum TimestampPolicy {
        first = 0, last = 1, average = 2
    };

private:
    uint32_t m_sourceId;
    uint64_t m_evtTimestamp;
    uint64_t m_coincTicks;
    bool     m_isBuilding;
    TimestampPolicy m_policy;


    // Canonicals
public:
    CGlomParameters(uint64_t interval, bool isBuilding, TimestampPolicy policy);
    CGlomParameters(uint64_t tstamp, uint32_t sourceId,
                    uint64_t interval, bool isBuilding, TimestampPolicy policy);
    virtual ~CGlomParameters();
    CGlomParameters(const CGlomParameters& rhs) = default;
    CGlomParameters(const CRawRingItem& rhs);
    
    CGlomParameters& operator=(const CGlomParameters& rhs) = default;
    virtual bool operator==(const CRingItem& rhs) const;
    virtual bool operator!=(const CRingItem& rhs) const;
    
    // Selectors:
public:

    uint32_t type() const;
    void setType(uint32_t type);

    uint32_t size() const;

    uint32_t getSourceId() const;
    void setSourceId(uint32_t id);

    uint64_t getEventTimestamp() const;
    void setEventTimestamp(uint64_t tstamp);

    bool mustSwap() const;
    bool isComposite() const;

    void toRawRingItem(CRawRingItem& raw) const;


   uint64_t coincidenceTicks() const;
   void setCoincidenceTicks(uint64_t ticks);

   bool     isBuilding() const;
   void setBuildingState(bool enable);

   TimestampPolicy timestampPolicy() const;
   void setTimestampPolicy(TimestampPolicy policy);
   
   // Object methods:
public:
    virtual std::string typeName() const;
    virtual std::string toString() const;

};

  } // end of V12 namespace
} // end DAQ

#endif
