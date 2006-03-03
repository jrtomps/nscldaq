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


//////////////////////////CDocumentedPacket.h file//////////////////////////////////

#ifndef __CDOCUMENTEDPACKET_H  
#define __CDOCUMENTEDPACKET_H
                               
#ifndef __STL_STRING
#include <string>   
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#include <spectrodaq.h>
                               
/*!
   Encapsulates a documented packet for two purposes:
   - Generation of packet headers for the readout.
   - Generation of documentation of packets used by the system.
   Member data comes in two (overalpping) chunks therefore:
   - m_nTag, m_sName, m_sDescription m_sVersion m_sInstantiationDate
     are used to document the tag  in documentation buffers.
   - m_nTag, m_pHeaderPtr, m_fPacketInProgress are used during
      packet formatting as an event is being taken.
   
 */

class CDocumentedPacket      
{ 
private:
  unsigned short m_nTag;	//!< Tag identifying event in the buffer.
  STD(string) m_sName;		//!< String name of tag.
  STD(string) m_sDescription;	//!< Long description of tag.
  STD(string) m_sVersion;		//!< Version of the packet.
  STD(string) m_sInstantiationDate;	//!< Date/time object instantiated.
#ifndef HIGH_PERFORMANCE
  DAQWordBufferPtr m_pHeaderPtr; //!< 'pointer' to header of current packet.
#else /* HIGH_PERFORMANCE */
  unsigned short* m_pHeaderPtr; //!< 'pointer' to header of current packet.
#endif /* HIGH_PERFORMANCE */
  bool m_fPacketInProgress;	//!< true if packet being built now.
 
public:
	// Constructors, destructors and other cannonical operations: 

  CDocumentedPacket (unsigned short nTag,
		     const STD(string)&  rName,
		     const STD(string)&  rDescription,
		     const STD(string)&  rVersion); //!<  Constructor
   virtual ~CDocumentedPacket();
      
private:  
  CDocumentedPacket(const CDocumentedPacket& rhs); //!< Copy constructor.
  CDocumentedPacket& operator= (const CDocumentedPacket& rhs); //!< Assignment
public:
   int         operator==(const CDocumentedPacket& rhs) const //!< Comparison for equality.
   {
      return (m_sName == rhs.m_sName);
   }
   int         operator==(const STD(string)& rhs) const 
   {
      return m_sName == rhs;
   }
   int         operator!=(const CDocumentedPacket& rhs) const
   {
      return !(operator==(rhs));
   }

   int         operator!=(const STD(string)& rhs) const
   {
      return !(operator==(rhs));
   }
   // Selectors for class attributes:
public:
  
  unsigned short getTag() const {
    return m_nTag;
  }
  
  STD(string) getName() const {
    return m_sName;
  }
  
  STD(string) getDescription() const {
    return m_sDescription;
  }
  
  STD(string) getVersion() const {
    return m_sVersion;
  }
  
  STD(string) getInstantiationDate() const {
    return m_sInstantiationDate;
  }
  
#ifndef HIGH_PERFORMANCE
  DAQWordBufferPtr getHeaderPtr() const; // Throws if pkt not open.
#else /* HIGH_PERFORMANCE */
  unsigned short* getHeaderPtr() const; // Throws if pkt not open.
#endif /* HIGH_PERFORMANCE */
 
  bool getPacketInProgress() const {
    return m_fPacketInProgress;
  }
  
  // Class operations:
  
  STD(string) Format ()  ;
#ifndef HIGH_PERFORMANCE
  DAQWordBufferPtr Begin (DAQWordBufferPtr& rPointer)  ;
  DAQWordBufferPtr End (DAQWordBufferPtr& rBuffer)  ;
#else /* HIGH_PERFORMANCE */
  unsigned short* Begin (unsigned short* rPointer)  ;
  unsigned short* End (unsigned short* rBuffer)  ;
#endif /* HIGH_PERFORMANCE */
  
};

#endif
