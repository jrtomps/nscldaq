#ifndef DAQ_V12_CDATAFORMATITEM_H
#define DAQ_V12_CDATAFORMATITEM_H
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt
*/

/**
 * @file CDataFormatItem.h
 * @brief Define the CDataFormatItem a class for producing data format ring items.
 */

#include "V12/CRingItem.h"            // Base class

namespace DAQ {
  namespace V12 {

/**
 * @class CDataFormatItem
 *
 * This class encapsulates ring items of the type DataFormat (RING_FORMAT).
 * It is used to provide an indicator to the user of the format of data that is
 * being inserted into a ring.  This can be used by data decoders to determine
 * how to handle the specific data types.  If a data source never provides a
 * RING_FORMAT item, you can assume the format to be for NSCLDAQ-10.* as this
 * ring type was added in NSCLDAQ 11.0.
 * The ring format will contain the major and minor versions of the
 * lowest NSCLDAQ version that can process this data format.
 *
 * The data format item is always a leaf item.
 *
 */
class CDataFormatItem : public CRingItem
{
private:
    uint64_t m_evtTimestamp;
    uint32_t m_sourceId;
    uint16_t m_major;
    uint16_t m_minor;

    // Canonical methods:
public:
    CDataFormatItem();
    CDataFormatItem(uint64_t tstamp, uint32_t sourceId, uint16_t major, uint16_t minor);
    explicit CDataFormatItem(const CRawRingItem& rawItem);
    virtual ~CDataFormatItem();
    
    CDataFormatItem(const CDataFormatItem& rhs) = default;

    CDataFormatItem& operator=(const CDataFormatItem& rhs) = default;

    virtual bool operator==(const CRingItem& rhs) const;
    virtual bool operator!=(const CRingItem& rhs) const;
    
public:

    uint32_t type() const;
    void setType(uint32_t type);

    uint32_t size() const;

    uint64_t getEventTimestamp() const;
    void setEventTimestamp(uint64_t tstamp);

    uint32_t getSourceId() const;
    void setSourceId(uint32_t id);


    bool isComposite() const;
    bool mustSwap() const;

    void toRawRingItem(CRawRingItem& item) const;
    
    uint16_t getMajor() const;
    void setMajor(uint16_t major);

    uint16_t getMinor() const;
    void setMinor(uint16_t minor);

    // object methods:

    virtual std::string typeName() const;
    virtual std::string toString() const;
    
    // Private utilities
private:
    void init();
    
};

  } // end of V12 namespace
} // end DAq

#endif
