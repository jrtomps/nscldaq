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


//////////////////////////CNSCLPhysicsBuffer.h file//////////////////////////////////

#ifndef __CNSCLPHYSICSBUFFER_H  
#define __CNSCLPHYSICSBUFFER_H
                               
#ifndef __CNSCLOUTPUTBUFFER_H
#include "CNSCLOutputBuffer.h"
#endif

#ifdef __HAVE_STD_NAMESPACE
using namespace std;
#endif

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#ifndef __SPECTRODAQ_H
#define __SPECTRODAQ_H
#endif
#endif
                               
/*!
   Encapsulates the formatting of a physics
   buffer.  Physics buffers contain
   - The standard buffer header.
   - A series of entities called events.  Each
     event is a series of words which is lead by
     a self inclusive word count.  
   
   While the NSCL daq system makes no requirements
   on the contents of an event, typically an event consists
   of a series of self described packets each of which contains:
   - Packet size
   - Packet type code
   - Packet contents.
   
 */		
class CNSCLPhysicsBuffer  : public CNSCLOutputBuffer        
{ 
private:
#ifndef HIGH_PERFORMANCE
      DAQWordBufferPtr m_EventStartPtr; //!< Points to start of event being built.
 
#else /* HIGH_PERFORMANCE */
  unsigned short*   m_pBuffer;
  unsigned short*   m_pBufferCursor;
  unsigned short    m_nEntityCount;
  unsigned int      m_nBufferSize;

      
#endif /* HIGH_PERFORMANCE */
public:
	// Constructors, destructors and other cannonical operations: 

    CNSCLPhysicsBuffer (unsigned nWords = 4096); //!< Default constructor.
#ifndef HIGH_PERFORMANCE
    virtual ~ CNSCLPhysicsBuffer ( ) { } //!< Destructor.
#else /* HIGH_PERFORMANCE */
    virtual ~ CNSCLPhysicsBuffer ( );            //!< Destructor.
#endif /* HIGH_PERFORMANCE */
private:
    CNSCLPhysicsBuffer& operator= (const CNSCLPhysicsBuffer& rhs); //!< Assignment
    int         operator==(const CNSCLPhysicsBuffer& rhs) const; //!< Comparison for equality.
    int         operator!=(const CNSCLPhysicsBuffer& rhs) const;
    CNSCLPhysicsBuffer(const CNSCLPhysicsBuffer& rhs); //!< Copy constructor.
public:

	// Selectors for class attributes:
public:

#ifndef HIGH_PERFORMANCE
    DAQWordBufferPtr getEventStartPtr() const {
       return m_EventStartPtr;
#else /* HIGH_PERFORMANCE */
    unsigned short* getEventStartPtr() const {
      return m_pBufferCursor;
#endif /* HIGH_PERFORMANCE */
    }

	// Mutators:
protected:  

	// Class operations:

public:
     int  WordsInBody() const;

#ifndef HIGH_PERFORMANCE
     DAQWordBufferPtr StartEvent ()  ;
     void EndEvent (DAQWordBufferPtr& rPtr)  ;
     void RetractEvent (DAQWordBufferPtr& p)  ;
 
#else /* HIGH_PERFORMANCE */
     unsigned short* StartEvent ()  ;
     void EndEvent (unsigned short* rPtr)  ;
     void RetractEvent (unsigned short* p)  ;
     virtual void Route();
     unsigned short getEntityCount() const {
       return m_nEntityCount;
     }
#endif /* HIGH_PERFORMANCE */
};

#endif
