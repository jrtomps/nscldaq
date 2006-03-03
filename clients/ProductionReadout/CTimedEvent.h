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


//////////////////////////CTimedEvent.h file//////////////////////////////////
#ifndef __CTIMEDEVENT_H  
#define __CTIMEDEVENT_H                               
/*!
   Abstract base class representing timed events.  Example of a timed
   event in the Readout context is the trigger for scaler readouts.

   Timed events are to be inserted in a CTimer which will periodically
   call the Tick member function so that the event can schedule itself
   at the resolution of the timer.

 */		
class CTimedEvent      
{ 
private:
      unsigned int m_nMsInterval; //!< Number of milliseconds between events.
      int          m_nCountDown; //!< Number of milliseconds until next event fire.
 
public:
	// Constructors, destructors and other cannonical operations: 

    CTimedEvent (unsigned int nms = 1000); //!< Default constructor.
    CTimedEvent(const CTimedEvent& rhs); //!< Copy constructor.
    virtual  ~ CTimedEvent ( ) { } //!< Destructor.

    CTimedEvent& operator= (const CTimedEvent& rhs); //!< Assignment
    int         operator==(const CTimedEvent& rhs) const; //!< Comparison for equality.
    int         operator!=(const CTimedEvent& rhs) const {
       return !(operator==(rhs));
    }

	// Selectors for class attributes:
public:

    unsigned int getMsInterval() const {
       return m_nMsInterval;
    }

    unsigned int getCountDown() const {
       return m_nCountDown;
    }

	// Mutators:
protected:  
  void setCountdown(unsigned int nmsleft) {
    m_nCountDown = nmsleft;
  }

	// Class operations:

public:
  void Tick (unsigned int nms)  ;
  void SetInterval (unsigned int nms)  ;
  void Reset ()  ;

  // pure virtual members:

  /*!
      Invoked when the timer expires.  The 
      user of this class must subclass and override this to supply
      application specific functionality.

      */

  virtual   void operator() ()   = 0;

};

#endif
