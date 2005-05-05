#ifndef __CCHANEL_H
#define __CHANNEL_H
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

#ifndef __EPICS_CADEF
#include <cadef.h>
#ifndef __EPICS_CADEF
#define __EPICS_CADEF
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_TIME
#include <time.h>
#ifndef __CRT_TIME
#define __CRT_TIME
#endif
#endif

/**
 *   The CChannel class implements transparently updated EPICS channels for C++
 * programs.  Objects require a two-phase construction, as we won't be sure of the
 * timing of static construction vs. the EPICS startup.
 *  - The first phase intializes the class member data.
 *  - The second phase connects the channel to the EPICS updating mechanism.
 */

class CChannel 
{
private:
  STD(string)   m_sName;
  bool          m_fConnected;
  chid          m_nChannel;
  bool          m_fUpdateHandlerEstablished;
  bool          m_fConnectionHandlerEstablished;
  STD(string)   m_sValue;
  time_t        m_LastUpdateTime;
public:
  CChannel(STD(string) name);
  virtual ~CChannel();

  // Forbidden operations.

private:
  CChannel(const CChannel&);
  CChannel& operator=(const CChannel&);
  int operator==(const CChannel&) const;
  int operator!=(const CChannel&) const;
public:

  // Operations on the object:

  STD(string) getName()     const;
  bool        isConnected() const;

  virtual void Connect();
  time_t       getLastUpdate() const;
  STD(string)  getValue()      const;


  // Class level operations.

  static void doEvents(float seconds);

protected:
  static void StateHandler(connection_handler_args args);
  static void UpdateHandler(event_handler_args     args);
};



#endif
