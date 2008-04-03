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


//////////////////////////CReadoutMain.h file//////////////////////////////////

#ifndef __CREADOUTMAIN_H  
#define __CREADOUTMAIN_H
                               
#ifndef __CRUNSTATE_H
#include "CRunState.h"
#endif

#ifndef __CEXPERIMENT_H
#include "CExperiment.h"
#endif

#ifndef __CDOCUMENTEDPACKETMANAGER_H
#include "CDocumentedPacketManager.h"
#endif

#ifndef __CTIMER_H
#include <CTimer.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif
                               
class CInterpreterShell;
class CInterpreterStartup;                                
class CInterpreterCore;

/*!
   CReadoutMain is a singleton class which represents 
   the application object.  It creates subcomponent's and
   their related threads, starts them and waits for the interpreter
   thread to exit.  When that happens the application can be shutdown.
   The interpreter thread shuts down in response to the replaced exit
   command.  (Exit here is not the normal Tcl exit command).
   The entry point is able to parse the following switches:
   
   -w  - Requests a windowed interface.
   -pnum - Requests that the server run a TclServer on port num.
   -c - Requests that trigger management be via the NSCL
          standard CAMAC setup.  Default is to use the NSCL standard
          VME trigger setup.
   
 */		
class CReadoutMain : public DAQROCNode     
{ 
private:
  bool m_fWindowed;		//!< True if Tk interpreter is started.
  bool m_fServer;		//!< True if system runs a Tcl Server
  bool m_fExit;                 //!< True if should exit.
  unsigned short m_nPort;	//!< Number of Tcl server port if enabled.
  bool m_fVmeTrigger;	//!< True if trigger is VME based.
 
  CInterpreterShell*       m_pInterpreter; //!< Command interpreter wrapper
  CTimer&                   m_TimerQueue;    //!< Managed queue of timed events. 
  CRunState                m_RunState;     //!< Run State Transition manager.. 
  CExperiment&              m_Experiment;   //!< Experiment specific code container.

  
public:
	// Constructors, destructors and other cannonical operations: 

  CReadoutMain ();                      //!< Default constructor.
  ~ CReadoutMain ( );		//!< Destructor.

private:  
  CReadoutMain(const CReadoutMain& rhs); //!< Copy constructor.
  CReadoutMain& operator= (const CReadoutMain& rhs); //!< Assignment
  int         operator==(const CReadoutMain& rhs) const; //!< Comparison for equality.
  int         operator!=(const CReadoutMain& rhs) const;
public:
  
  // Selectors for class attributes:
public:
  
  bool getWindowed() const {
    return m_fWindowed;
  }
  
  bool getServer() const {
    return m_fServer;
  }
  
  unsigned short getPort() const {
    return m_nPort;
  }
  
  bool getVmeTrigger() const {
    return m_fVmeTrigger;
  }

  string getTitle() const;
  unsigned int getScalerPeriod() const;

  // The selectors below allow modification of their contents and are
  // intended for use by the members themselves callinb back at us.
public:
  static CReadoutMain* getInstance();
  CInterpreterShell*   getInterpreter() {
    return m_pInterpreter;
  }
  CRunState* getRunState() {
    return &m_RunState;
  }
  CExperiment* getExperiment() {
    return &m_Experiment;
  }
  CTimer& getClock() {
    return m_TimerQueue;
  }

  // Mutators:
protected:  
  
  // Class operations:
protected:
  virtual   int operator() (int argc , char** argv)  ;
  void ParseSwitches (int argc, char** argv)  ;
  void CreateInterpreter ()  ;
  void CreateExperiment ()  ;

  virtual void SetupReadout(CExperiment& rExperiment);
  virtual void SetupScalers(CExperiment& rExperiment);
public:
  virtual void SetupRunVariables(CExperiment& rExperiment,
				 CInterpreterStartup& rStartup,
				 CInterpreterCore&    rCore);
  virtual void SetupStateVariables(CExperiment& rExperiment,
				   CInterpreterStartup& rStartup,
				   CInterpreterCore&    rCore);

  virtual void AddUserCommands(CExperiment& rExperiment, 
			       CInterpreterStartup&  rStartup,
			       CInterpreterCore&     rCore);
  void Exit();
};


extern  bool daq_isJumboBuffer();

#endif


