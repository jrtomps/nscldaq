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

// Author:
//   Jason Venema
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto: venemaja@msu.edu
//
// Copyright
//   NSCL All rights reserved.
//
// CAlarmServer encapsulates a database server for alarms.  Alarms can be 
// created by DAQ (or any other) software entities, which notify alarm
// displayers, which log the alarm with this server.  The server is implemented
// using the gdbm UNIX database to store alarm content.  There are two database
// files:
//    (1) .alarmdb contains all of the alarm information fields, such as 
//        the logging faclity, the alarm status, the alarm message and date.
//    (2) .alarmcount contains experiment ID and alarm count information to be
//        used in generating count numbers for alarm displayers.
//

#ifndef __CALARMSERVER_H
#define __CALARMSERVER_H

//
// Include files
//

#ifndef __CSOCKET_H
#include <CSocket.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#ifndef __STL_ALGORITHM
#include <algorithm>
#ifndef __STL_ALGORITHM
#define __STL_ALGORITHM
#endif
#endif

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#endif

#ifndef __SPECTROFRAMEWORK_H
#include <SpectroFramework.h>
#endif

#ifndef __GDBM_H
#include <gdbm.h>
#endif

#ifndef __GDBMEXCEPTION_H
#include <CGDBMException.h>
#endif

#ifndef __CALARMSERVEREXCEPTION_H
#include <CAlarmServerException.h>
#endif

using namespace std;

typedef STD(map)<STD(string), STD(string)>::iterator MapIterator;

class CAlarmServer
{
  STD(string)m_sPort;               //! The port this server listens on
  STD(string) m_sExpId;              //! The experiment requesting the service
  STD(string) m_sAlarmId;            //! The alarm id of this alarm
  STD(map)<STD(string), STD(string)> m_CurrAlarm; //! Maps the exp. id to the current alarm id

 public:
  enum ClientReason {
    LOG,
    ACKNOWLEDGE,
    DISMISS,
    REMOVE,
    UPDATE,
    INIT,
    CREATE,
    HISTORY,
  };

 public:
  // Default constructor
  CAlarmServer();

  // Copy constructor
  CAlarmServer(const CAlarmServer& aCAlarmServer);

  // Destructor
  ~CAlarmServer() { }

  // The following functions are disabled
 private:
  // Operator= assignment operator
  CAlarmServer operator=(const CAlarmServer& aCAlarmServer);
  // Operator== equality operator
  int operator== (const CAlarmServer& aCAlarmServer);

  // Public selectors
 public:
  STD(string) getPort() { return m_sPort; }
  STD(string) getExpId() { return m_sExpId; }
  STD(string) getAlarmId() { return m_sAlarmId; }

  // Protected mutators
 protected:
  void setPort(const STD(string)& aPort) { m_sPort = aPort; }
  void setExpId(const STD(string)& aExpId) { m_sExpId = aExpId; }
  void setAlarmId(const STD(string)& aAlarmId) { m_sAlarmId = aAlarmId; }

  // Public member functions
 public:
  bool operator()();

  // Protected member fuctions
 protected:
  void Log(STD(string)& sFacility, STD(string)& sMessage, STD(string)& sDate, STD(string)& srFrom);
  void EditAlarm(Int_t nReason);
  STD(string) Init();
  void CreateExperiment();
  STD(string) GetExperimentHistory();
};

#endif
