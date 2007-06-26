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

   
//////////////////////////CNSCLOutputBuffer.h file//////////////////////////////////

#ifndef __CNSCLOUTPUTBUFFER_H  
#define __CNSCLOUTPUTBUFFER_H

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#ifndef __SPECTRODAQ_H                               
#include <spectrodaq.h>
#ifndef __SPECTRODAQ_H
#define __SPECTRODAQ_H
#endif
#endif


                               
/*!
   CNSCLOutputBuffer is the base class of a class
   hierarchy whidh supports creating NSCL formatted
   buffers that can be emitted into the Data Acquisition 
   system. 
   
   NSCL Data Acquisition system buffers have a fixed
   header and variable body.  The "shape" of the body
   depends  on the 'type' word of the buffer header.
   This set of classes simplifies the creation of the
   body of a buffer as well as synchonizing the body
   contents with the appropriate fields of the header.
   
 */		
class CNSCLOutputBuffer      
{ 
protected:
  static int m_ControlTag;
  static int m_EventTag;
protected:
  DAQWordBuffer m_BufferBase; //!< Spectrodaq Buffer to hold the data being emitted.
  DAQWordBufferPtr m_Buffer;
  mutable DAQWordBufferPtr m_BufferPtr; //!< 'pointer' to the current slot of the buffer.
private:

  int m_nWords; //!< Number of words the buffer can hold.
  static  unsigned long m_nSequence; //!< Sequence number for the buffer.
 
public:
	// Constructors, destructors and other cannonical operations: 

    CNSCLOutputBuffer (unsigned nWords=4096); //!< Default constructor.
#ifndef HIGH_PERFORMANCE
     ~ CNSCLOutputBuffer ( ) { } //!< Destructor.
#else /* HIGH_PERFORMANCE */
    virtual  ~ CNSCLOutputBuffer ( ) { } //!< Destructor.
#endif /* HIGH_PERFORMANCE */

  // Copying DAQ buffers is not legal so the various copy stuff is
  // illegal too.
private:
    CNSCLOutputBuffer(const CNSCLOutputBuffer& rhs); //!< Copy constructor.
    CNSCLOutputBuffer& operator= (const CNSCLOutputBuffer& rhs); //!< Assignment
    int         operator==(const CNSCLOutputBuffer& rhs) const; //!< Comparison for equality.
    int         operator!=(const CNSCLOutputBuffer& rhs) const;
public:

	// Selectors for class attributes:
public:

    DAQWordBuffer& getBuffer()  {
       return m_BufferBase;
    }

    DAQWordBufferPtr  getBufferPtr() const  {
       return m_BufferPtr;
    }

    int getWords() const {
       return m_nWords;
    }

    unsigned long getSequence() const {
       return m_nSequence;
    }

    unsigned short getEntityCount();

    unsigned short getBufferType();

	// Mutators:
protected:  

	// Class operations:
public:
     void ComputeSize ()  ;
     void SetType (int nType)  ;
     void ComputeChecksum ()  ;
     DAQWordBufferPtr StartEntity ()  ;
     void EndEntity (const DAQWordBufferPtr& ePtr)  ;
     void SetCpuNum (unsigned short nValue=0)  ;
     void SetNbitRegisters (unsigned  nBitReg=1)  ;
     void SetLamRegisters (unsigned short nLams=0)  ;
     void PutEntity (void* pEntity, unsigned int nWords)  ;
     void PutLong (const unsigned long& rLong)  ;
     void PutWord (unsigned short nData)  ;
     void PutWords (const unsigned short* pWords, unsigned int nWords)  ;
     void PutString (const char*  pData, int nMaxSize=-1)  ;
     void SetRun (unsigned short nRun)  ;
     bool EntityFits (unsigned short  nWords)  ;
#ifndef HIGH_PERFORMANCE
     void Route ()  ;
#else /* HIGH_PERFORMANCE */
     virtual void Route ()  ;
#endif /* HIGH_PERFORMANCE */
     void Seek (int nOffset, int  whence=SEEK_SET)  ;
     void Seek(DAQWordBufferPtr& rPtr) {
       m_BufferPtr = rPtr;
     }
     void Resize(int newsize);
#ifdef HIGH_PERFORMANCE
     void setEntityCount(unsigned short entities);

#endif /* HIGH_PERFORMANCE */
     static void IncrementSequence ()  ;
     static void ClearSequence ()  ;
protected:
     void InitializeHeader();
};

#endif



