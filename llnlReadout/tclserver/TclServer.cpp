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
#include "CMonCommand.h"
#include <DataBuffer.h>

#include <tcl.h>
#include <TCLInterpreter.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include "CControlModule.h"
#include <Exception.h>
#include <string>
#include <iostream>
#include <CRunState.h>

static const int MonitorInterval(1); // Number of seconds between monitor interval.

TclServer::TclServer* m_pInstance(0); // static->object context. ptr.
/**
 ** This strruct is used to pass data between the readout thread and us:
 */
struct TclServerEvent {
  struct Tcl_Event event;
  void*            pData;
};

/*!  Constructor is not very interesting 'cause all the action is in 
    start and operator()
*/
TclServer::TclServer() :
  m_port(-1),
  m_configFilename(string("")),
  m_pVme(0),
  m_pInterpreter(0),
  m_pMonitorList(0)
{
  m_pInstance = this;		// static->object context.
}
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

DAQThreadId
TclServer::start(int port, const char* configFile, CVMUSB& vme)
{
  // Set up the member data needed to run the thread...

  m_port           = port;
  m_configFilename = configFile;
  m_pVme           = &vme;

  // Schedule the thread for execution:

  m_tid = daq_dispatcher.Dispatch(*this);

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
   Entry point for the thread.  This will be called when the thread is first
   scheduled after start was called.  We just need to call our
   private functions in order :-).
   Parameters are ignored (start stocked the member data with everything
   we need) and we never return.
*/
int
TclServer::operator()(int argc, char** argv)
{
  m_threadId = Tcl_GetCurrentThread(); // Save for later use.
  try {
    initInterpreter();		// Create interp and add commands.
    readConfigFile();		// Initialize the modules.
    initModules();              // Initialize the fully configured modules.
    createMonitorList();	// Figure out the set of modules that need monitoring.
    startTcpServer();		// Set up the Tcp/Ip listener event.
    EventLoop();		// Run the Tcl event loop forever.
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
  Tcl_Init(pInterp);

  m_pInterpreter       = new CTCLInterpreter(pInterp);

 
  
  // Add the commands... these don't get saved.. as they will live forever

  new CModuleCommand(*m_pInterpreter,
		     *this);
  new CSetCommand(*m_pInterpreter,
		  *this,
		  *m_pVme);
  new CGetCommand(*m_pInterpreter,
		  *this,
		  *m_pVme);
  new CUpdateCommand(*m_pInterpreter,
		    *this,
		    *m_pVme);
  new CMonCommand(*m_pInterpreter, *this);
  
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
  /** Now build the monitor list from the modules that have been configured */

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
  // If there's a nonempty monitor list we need to start its periodic execution

  if (m_pMonitorList && m_pMonitorList->size()) {
    MonitorDevices(this);	// Allow it to locate us.
  }
  // Start the event loop:

  while(1) {
    Tcl_DoOneEvent(TCL_ALL_EVENTS);
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
    m_Modules[i]->Initialize(*m_pVme);
  }
}

/**
 ** Create the monitor list and populate it with contributions from the 
 ** modules that have been created.  
 ** @note Site effect: m_pMonitorList is constructed and, potentially
 **       filled in.
 */
void
TclServer::createMonitorList()
{
  m_pMonitorList = new CVMUSBReadoutList;
  for (int i =0; i < m_Modules.size(); i++) {
    m_Modules[i]->addMonitorList(*m_pMonitorList);
  }
}
/**
 ** Provide a copy of the monitor list to a client.
 ** if m_pModules is null an empty list will be returned.
 ** @return CVMUSBReadoutList
 ** @retval Copy of the local monitor list.
 **
 */
CVMUSBReadoutList
TclServer::getMonitorList()
{
  CVMUSBReadoutList result;
  if (m_pMonitorList) {
    result = *m_pMonitorList;
  }
  return result;
}
/*
** This function queues an event to the tclserver thread.  It is intended to be
** called by other threads.  The event indicates the arrival of a monitor
** buffer when the run is active.  The event handling function
** will have to funge stuff up to allow processMonitorData to be
** executed on the received data.
** @param pBuffer - Actually a DataBufer* which contains the data gotten from
**                  the vmusb.
*/
void
TclServer::QueueBuffer(void* pBuffer)
{
  TclServerEvent* pEvent = reinterpret_cast<TclServerEvent*>(Tcl_Alloc(sizeof(TclServerEvent)));
  pEvent->event.proc = receiveMonitorData;
  pEvent->pData      = pBuffer;

  Tcl_ThreadQueueEvent(m_threadId, 
		       reinterpret_cast<Tcl_Event*>(pEvent), 
		       TCL_QUEUE_HEAD); // out of band processing.
}

/**
 ** Monitor devices:
 ** If the run is inactive, the list is done in immediate mode, and the data are dispatched
 ** Directly to the device handlers.
 ** If the run is active, the list is popped off and the readout thread will do that dispatch for us.
 ** Regardless, we reschedule ourself via Tcl_CreateTimerHandler.
 ** @param pData - Void pointer that is really a TclServer pointer that allows us to gain object
 **                context.
 */
void
TclServer::MonitorDevices(void* pData)
{
  TclServer* pObject = reinterpret_cast<TclServer*>(pData);

  // If the run is active  we just trigger list 7.
  // otherwise we execute the list immediate and ship the data around
  // to the various devices.

  CVMUSB* pController = pObject->m_pVme;
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    pController->writeActionRegister( CVMUSB::ActionRegister::triggerL7 | 
				      CVMUSB::ActionRegister::startDAQ); // StartDAQ keeps acquisition alive.
  }
  else {
    uint16_t readData[13*1024];	// Big data pot...ought to be big enough...one event buffer worth?
    size_t   dataRead(0);
    CVMUSBReadoutList* pList  = pObject->m_pMonitorList;
    if (pList->size() > 0) {
      int                status = pController->executeList(*pList, readData, sizeof(readData), &dataRead);
      if (status != 0) {
	cerr << "Warning: Monitor list read failed\n";
	
      }
      else {
	pObject->processMonitorList(readData, dataRead);
      }
	
    }						 
  }

  Tcl_Interp* pInterp = pObject->m_pInterpreter->getInterpreter();
  Tcl_CreateTimerHandler(MonitorInterval*1000, TclServer::MonitorDevices, pData);
}
/*
** Process control module data:
**
** @param pData   - Pointer to the data read from the moduels.
** @param nBytes  - Number of bytes of data to process.
*/
void
TclServer::processMonitorList(void* pData, size_t nBytes)
{
  // It will be easier to track the progress through the list
  // by treating the data as uint8_t*

  uint8_t* p = reinterpret_cast<uint8_t*>(pData);
  for (int i =0; i < m_Modules.size(); i++) {
    uint8_t* pNewPosition;
    pNewPosition = reinterpret_cast<uint8_t*>(m_Modules[i]->processMonitorList(p, nBytes));
    nBytes      -= (pNewPosition - p);
    p            = pNewPosition;
    if (nBytes ==0) break;	// Short circuit once we're out of bytes.
					      
  }
}
/**
 ** When a buffer of monitor data arrives an event is queued to this thread.
 ** the event invokes this function.
 */
int
TclServer::receiveMonitorData(Tcl_Event* pEvent, int flags)
{
  // Get the data buffer out of the Tcl_Event payload

  TclServerEvent* pMyEvent = reinterpret_cast<TclServerEvent*>(pEvent);
  DataBuffer*     pBuffer = reinterpret_cast<DataBuffer*>(pMyEvent->pData);

  // figure out how much data we have and pass it to
  // process monitor list so that the monitored data
  // will get updated:
  void* pData;
  size_t dataBytes;
  pData = &(pBuffer->s_rawData[2]); // first event word.
  dataBytes = pBuffer->s_rawData[1] & 0xfff;
  dataBytes = dataBytes * sizeof(uint16_t);

  m_pInstance->processMonitorList(pData, dataBytes);

  // Return the buffer to the free pool:

  gFreeBuffers.queue(pBuffer);
  return 1;			// Done with the event.
}
