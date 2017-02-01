#ifndef NSCLDAQV12_CPHYSICSEVENTITEM_H
#define NSCLDAQV12_CPHYSICSEVENTITEM_H
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

#include <V12/CRawRingItem.h>		/* Base class */


namespace DAQ {
  namespace V12 {

/**
 *  This class is a wrapper for physics events.
 *  It's mainly provided so that textual dumps
 *  can be performed as typeName and toString
 *  are the only substantive methods...everything
 *  else just delegates to the base class.
 */

class CPhysicsEventItem : public CRawRingItem
{
public:
  CPhysicsEventItem();
  CPhysicsEventItem(uint64_t timestamp, uint32_t source, const Buffer::ByteBuffer& body = Buffer::ByteBuffer() );

  CPhysicsEventItem(const CRawRingItem& rhs);
  CPhysicsEventItem(const CPhysicsEventItem& rhs);
  virtual ~CPhysicsEventItem();

  CPhysicsEventItem& operator=(const CPhysicsEventItem& rhs);
  virtual bool operator==(const CRingItem& rhs) const;
  virtual bool operator!=(const CRingItem& rhs) const;

  // Virtual methods that all ring items must provide:

  virtual std::string typeName() const;	// Textual type of item.
  
};


} // end of V12 namespace
} // end DAQ

#endif
