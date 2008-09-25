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

#ifndef __TCLSERVER_H
#define __TCLSERVER_H
using namespace std;


#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __THREAD_H
#include <Thread.h>
#endif


class CControlModule;
class CTCLInterpreter;
class CConditionVariable;
class CMutex;
/*!
  The TclServer class implements a little Tcl server for our readout software.
  The server is intended to allow external software to perform control operations
  to the VMUSB.  It does so by:
  - Reading a configuration file that is used to create the control objects
    that are available to the remote system (Subclasses of CControlHardware).
  - Adding the commands SET and GET to the interpreter, that manipulate
    or query the hardware (synchronizing with the readout thread if needed
    for access to the VMUSB.
  - runing the Tcp/IP event loop so that Tcp/IP connections and file events can be honored.

   The Tcp/IP server section is completely stolen from the TclServer that comes with the
   nscldaq software.

   Note that this TclServer is a thread in spectrodaq and therefore this class must be
   instantiated and scheduled.  Scheduling is done by invoking the Start method
   which is a simple way to get the thread its parameters.
   Note that the connection authorization has been dumbed down to only accept
   connections from localhost.

   Module types are supported via a the prototype pattern from Design patterns.
   An STL map keyed by module type contains pointers to CControlModule's  these
   are cloned by the module command as needed to create module instances.

*/
class TclServer : public Thread
{
private:
  typedef std::map<std::string, CControlModule*> Prototypes;
  typedef Prototypes::iterator                   PrototypeIterator;
  // Member data:
private:
  int                          m_port;		// Port we are listening to for connections.
  std::string                  m_configFilename;
  std::vector<CControlModule*> m_Modules;       // Hardware we can access.
  Prototypes                   m_modulePrototypes;
  CTCLInterpreter*             m_pInterpreter;
  unsigned long                m_tid;
  CMutex*                      m_pMutex;
  CConditionVariable*          m_pInitialized;

public:
  TclServer();
  ~TclServer();			// This is a final class.
private:
  TclServer(const TclServer& rhs);
  TclServer& operator=(const TclServer& rhs);
  int operator==(const TclServer& rhs) const;
  int operator!=(const TclServer& rhs) const;


public:
  unsigned long   start(int port, const char* configFile);
  CControlModule* findModule(std::string name);
  void            addModule(CControlModule* pNewModule);
  void            addPrototype(std::string typeName, CControlModule* prototype);
  CControlModule* createModule(std::string typeName);

  void            setResult(std::string resultText);
protected:
  virtual void run();

private:
  void initInterpreter();
  void readConfigFile();
  void initModules();
  void startTcpServer();
  void EventLoop();

};
 



#endif
