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

#include <config.h>
using namespace std;

#include "TclServer.h"
#include "server.h"
#include "serverinstance.h"
#include "CModuleCommand.h"
#include "CSetCommand.h"
#include "CGetCommand.h"
#include "CUpdateCommand.h"

#include <CMutex.h>
#include <CCondition.h>

#include <tcl.h>
#include <TCLInterpreter.h>
#include "CControlModule.h"
#include <Exception.h>
#include <string>
#include <iostream>

/*!  Constructor is not very interesting 'cause all the action is in 
    start and operator()
*/
TclServer::TclServer() :
  m_port(-1),
  m_configFilename(string("")),
  m_pInterpreter(0)
{}
/*!
  These threads are built to live 'forever' so the destructor is also 
uninteresting.
*/
TclServer::~TclServer()
{}

/*!
  Start sets up the variables the entry point needs to initialize
  and schedules the thread.   The thread id is returned to the caller.
  Note that while the entry point includes parameters, it's just so much
  easier to stuff them into the member data rather than trying to force fit
  them to argc,argv formalism in the caller and receiver.
  \param port : int
     Number of the Tcp/IP port on which we will listen for connections.
  \param configFile : const char* 
     Name of the configuration file that is used to instantiate our controllers.
     This is intended to be a Tcl script and is therefore processed by the
     interpreter after it has been initialized with all the added commands,
     but prior to entering the event loop.
   \param vme : CVMUSB& vme
     Vme controller that is used to interact with the hardware.

*/

unsigned long
TclServer::start(int port, const char* configFile)
{
  m_pMutex       = new CMutex;
  m_pInitialized = new CConditionVariable;
  m_pMutex->lock();

  // Set up the member data needed to run the thread...

  m_port           = port;
  m_configFilename = configFile;

  // Schedule the thread for execution:

  Thread::start();
  m_pInitialized->wait(*m_pMutex);
  m_pMutex->unlock();
  m_tid = getId();
  return m_tid;
}

/*!
  Locate a module by name.  
  \param name : std::string
     Name of the module to find.
  \return CControlModule*
  \retval NULL - not found
  \retval Other - Pointer to the found module.
*/
CControlModule*
TclServer::findModule(string name)
{
  for (int i=0; i < m_Modules.size(); i++) {
    CControlModule* pModule = m_Modules[i];
    if (pModule->getName() == name) {
      return pModule;
    }
  }
  return static_cast<CControlModule*>(NULL);
}
/*!
   Add a new module to the list of modules
   \param pNewModule : CControLModule*
      Pointer to the new module to add.
*/
void
TclServer::addModule(CControlModule* pNewModule)
{
  m_Modules.push_back(pNewModule);
}

/*!
   Set the interpreter result to a string value.
*/
void
TclServer::setResult(string msg)
{
  Tcl_Obj* result = Tcl_NewStringObj(msg.c_str(), -1);
  Tcl_SetObjResult(m_pInterpreter->getInterpreter(), result);
  
  
}
/*!
   Add a prototype from which a module can be created.

   This is used by the module command to create the actual modules.

   \param typeName - Type of module.
   \param prototype- Pointer to a prototype module, which must have
                     a lifespan that is the length of the program.

\note prototypes should not have a configuration attached to them.
      The act of attaching the configuration provides the module with a name.

*/
void
TclServer::addPrototype(string typeName, CControlModule* prototype)
{
  m_modulePrototypes[typeName] = prototype;
}
/*!
   Use the prototype data base to return a dynamically created module
   \param typeName - type of the module to create.
   \return CControlModule*
   \retval Pointer to a new (supposedly) unconfigured module.
   \retval NULL - no prototype for typename exists.
*/
CControlModule*
TclServer::createModule(string typeName)
{
  PrototypeIterator p = m_modulePrototypes.find(typeName);
  if (p != m_modulePrototypes.end()) {
    return p->second->clone();
  }
  else {
    return reinterpret_cast<CControlModule*>(NULL);
  }
}

/*!
   Entry point for the thread.  This will be called when the thread is first
   scheduled after start was called.  We just need to call our
   private functions in order :-).
   Parameters are ignored (start stocked the member data with everything
   we need) and we never return.
*/
void
TclServer::run()
{
  m_tid = getId();		// In case starter could not get it.
  try {
    initInterpreter();		// Create interp and add commands.
    readConfigFile();		// Initialize the modules.
    initModules();              // Initialize the fully configured modules.
    startTcpServer();		// Set up the Tcp/Ip listener event.
    m_pInitialized->signal();
    EventLoop();			// Run the Tcl event loop forever.
  }
  catch (string msg) {
    cerr << "TclServer thread caught a string exception: " << msg << endl;
    throw;
  }
  catch (char* msg) {
    cerr << "TclServer thread caught a char* exception: " << msg << endl;
    throw;
  }
  catch (CException& err) {
    cerr << "CAcquisitino thread caught a daq exception: "
	 << err.ReasonText() << " while " << err.WasDoing() << endl;
    throw;
  }
  catch (...) {
    cerr << "TclServer thread caught some other exception type.\n";
    throw;
  }
}

/*
   - Create and initialize an interpreter
   - Objectify it and store it in m_pInterpreter
   - Add the following three commands:
   - module   - Create/configure a module
   - Set      - Set an item in a module to a value.
   - Get      - Get an item's value from a module.
   - Update   - Update all module's state for get


   On exit we should be able to read a configuration file.


*/
void
TclServer::initInterpreter()
{

  // Create the interpreter:


  Tcl_Interp*  pInterp = Tcl_CreateInterp();
  m_pInterpreter       = new CTCLInterpreter(pInterp);

  
  // Add the commands... these don't get saved.. as they will live forever

  new CModuleCommand(*m_pInterpreter,
		     *this);
  new CSetCommand(*m_pInterpreter,
		  *this);

  new CGetCommand(*m_pInterpreter,
		  *this);

  new CUpdateCommand(*m_pInterpreter,
		     *this);

  
}
/*
   Read the configuration file.  This is just sourcing the the file
   into our brand new interpreter.  This should cause the 
   modules vector to get stocked with the appropriate set of preconfigured
   modules.  It will be up to external control programs to 
   set values for the parameters of these modules.
*/
void
TclServer::readConfigFile()
{
  try {
    m_pInterpreter->EvalFile(m_configFilename);
  }
  catch (string msg) {
    cerr << "TclServer::readConfigFile - string exception: " << msg << endl;
    throw;
  }
  catch (char* msg) {
    cerr << "TclServer::readConfigFile - char* exception: " << msg << endl;
    throw;
  }
  catch (CException& error) {
    cerr << "TclServer::readConfigFile - CException: " 
	 << error.ReasonText() << " while " << error.WasDoing() << endl;
    throw;
  }
  catch (...) {
    cerr << "TclServer::readConfigFile - unanticipated exception type\n";
    throw;
  }
}
/*
   Start the Tcl server.
   This just means calling ::Server_Init giving it the interpreter and
   the port
*/
void
TclServer::startTcpServer()
{
  ::Server_Init(m_pInterpreter->getInterpreter(),
		m_port);
}
/*
  run the event loop.  This should never exit (although in theory the
  user could poke an exit into the interpreter which would finish us off
  right quick).
  Running the server as an event loop is the only way to ensure that Tcp/IP
  events get served (honoring connections and accepting commands).
  We are actually a pretty wierd interpreter. We don't have stdin...
  just 0 or more Tcp/Ip sockets on which commands can be accepted.

*/
void
TclServer::EventLoop()
{
  while(1) {
    Tcl_WaitForEvent(NULL);
    Tcl_SetServiceMode(TCL_SERVICE_ALL);
    Tcl_ServiceAll();
  }
 std::cerr << "The Tcl Server event loop has exited. No Tcp ops can be done\n"; 
}


/*   
   Initialize the modules by calling all their Update functions:
*/
void
TclServer::initModules()
{
  for (int i =0; i < m_Modules.size(); i++) {
    m_Modules[i]->Initialize();
  }
}