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

/*!
    Manages a readout buffer.

    Readout buffers are managed for optimal packing:  Buffers are 
    twice as large as the system buffer size.  Events are acquired using
    ::readevt() into the buffer until they spill over past the system buffer
    size.   At that time, that event is copied into a new buffer and the old
    one is routed to the system.  This class also contains experiment
    management functions which formerly were part of the Active state class.
    These are provided as virtual functions to allow them to be overridden
    for experiment specific applications.
 
    */
// Author:
//        Ron Fox
//        NSCL
//        Michigan State University
//        East Lansing, MI 48824-1321
//        fox@nscl.msu.edu
//
// Version information:
//    $Header$
//
/* Change log:
      $Log$
      Revision 8.3  2007/05/17 21:26:15  ron-fox
      Work on porting to spectrodq-lite.

      Revision 8.2  2005/06/24 11:32:03  ron-fox
      Bring the entire world onto the 8.2 line

      Revision 4.2  2004/11/16 18:51:37  ron-fox
      Port to gcc/g++ 3.x

      Revision 4.1  2004/11/08 17:37:40  ron-fox
      bring to mainline

      Revision 3.1  2003/03/22 04:03:29  ron-fox
      Added SBS/Bit3 device driver.

      Revision 2.1  2003/02/11 16:44:32  ron-fox
      Retag to version 2.1 to remove the weird branch I accidently made.

      Revision 1.1.1.1  2003/02/05 14:04:32  ron-fox
      Initial import of the NSCL Daq clients version 7.0-000 to sourceforge.


      Revision 2.3  2002/10/09 11:27:40  fox
      Add copyright/license stamp.

 * Revision 2.2  2002/07/02  15:12:16  fox
 * Go to 2.xx based releases (recover from client crash too).
 *
 * Revision 2.1  2002/07/02  15:05:18  fox
 * Transition to 2.1 releases
 *
 * Revision 1.2  2002/06/27  15:55:07  fox
 * - Debug tight packed buffer Readout (note still problems with Spectrodaq)
 * - Support SBS/Bit3 device driver in vmetcl et seq.
 *
 * Revision 1.1  2002/06/19  17:20:52  fox
 * Initial drafts.
 *
*/
#ifndef __READER_H
#define __READER_H

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#define __SPECTRODAQ_H
#endif

// Forward classes.

class ReadoutStateMachine;
class CTrigger;
class CBusy;


class CReader
{
  // private data:
private:
  mutable 
    ReadoutStateMachine&   m_rManager; //!< State machine (some services)
  mutable 
    DAQWordBuffer*         m_pBuffer;  //!< Current buffer.
  mutable 
    DAQWordBufferPtr       m_BufferPtr; //!< Cursor into current buffer.
  unsigned int             m_nEvents;  //!< Current Event count.
  unsigned int             m_nWords;   //!< Current word count.
  unsigned int             m_nBufferSize; //!< System buffersize (High w.mrk).

  mutable CTrigger*        m_pTrigger; //!< Trigger manager.
  mutable CBusy*	   m_pBusy;    //!< Dead-time module.
  // Constructors and other canonical functions.

public:
  CReader(ReadoutStateMachine& rManager);
  virtual ~CReader() {}
private:
  CReader(const CReader& rRhs);	//!< Copy constructor illegal.
  CReader& operator=(const CReader& rRhs); //!< Assignment illegal.
  int     operator==(const CReader& rRhs) const; //!<< Comparison senseless.
  int     operator!=(const CReader& rRhs) const; //!<< Comparison senseless.
public:

  //  Selectors:

public:
  ReadoutStateMachine& getManager() const {
    return m_rManager;
  }
  DAQWordBuffer* getBuffer() const {
    return m_pBuffer;
  }
  DAQWordBufferPtr getBufferPointer() const {
    return m_BufferPtr;
  }
  unsigned int getEventCount() const {
    return m_nEvents;
  }
  unsigned int getWordCount() const {
    return m_nWords;
  }
  unsigned int getBufferSize() const {
    return m_nBufferSize;
  }
  CTrigger* getTrigger() const {
    return m_pTrigger;
  }
  CBusy* getBusy() const {
    return m_pBusy;
  }

  // Simple mutators:

protected:
  void setBuffer(DAQWordBuffer* pBuffer) {
    m_pBuffer    = pBuffer;
  }
  void setBufferPointer(DAQWordBufferPtr& rBufferPtr) {
    m_BufferPtr = rBufferPtr;
  }
  void setEventCount(unsigned int nEvents) {
    m_nEvents = nEvents;
  }
  void setWordCount(unsigned int nWords) {
    m_nWords = nWords;
  }
  void setBufferSize(unsigned int nBufferSize) {
    m_nBufferSize;
  }
public:
  void setTrigger(CTrigger* pTrigger) {
    m_pTrigger = pTrigger;
  }
  void setBusy(CBusy* pBusy) {
    m_pBusy = pBusy;
  }

  // Invariant functions.

public:
  void Enable();
  void Disable();
  void ReadSomeEvents(unsigned int nPasses);
  void FlushBuffer();

  // Utility functions.

private:
  void OverFlow(DAQWordBufferPtr& rLastEventPtr);

};

#endif

