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

#ifndef __CTHEAPPLICATION_H
#define __CTHEAPPLICATION_H
#include <config.h>


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __TCLOBJECT_H
#include <TCLObject.h>
#endif

#include <memory>

class CTCLInterpreter;
struct Tcl_Interp;
struct Tcl_Event;
class CMutex;
class CConditionVariable;

/*!
   This class is  the thread that is the main application startup thread.
   We have to do a bunch of initialization:
   - Set up the initial run state.
   - Set up the Tcl Interpreter and its commands for the main
     program.
   - Start the output thread
   - Start the Tcl Server thread.
   - Pass control to the Tcl event loop.

   Due to the way Spectrodaq clients work, we are not able to make a pure
   singleton object with private constructors, because we will need to
   make a static instantiation of the object.  However that static instance
   provides us with the handles we need to get the tcl interpreter started and
   extended.   There will be more comments about this in startTcl and
   AppInit().
   
   Since the lifetime of this application is the lifetime of the program,
   storage management will be a bit sloppy.

*/
class CTheApplication        
{
private:
  static bool          m_Exists; //!< Enforce singletons via exceptions.
  static std::string   m_initScript;
  int                  m_Argc;
  char**               m_Argv;
  CTCLInterpreter*     m_pInterpreter;
  std::shared_ptr<CMutex> m_pMutex;
  std::shared_ptr<CConditionVariable> m_pCondition;

public:
  // Canonicals

  CTheApplication();
  ~CTheApplication();
private:
  CTheApplication(const CTheApplication& rhs);
  CTheApplication& operator=(const CTheApplication& rhs);
  int operator==(const CTheApplication& rhs) const;
  int operator!=(const CTheApplication& rhs) const;
public:

  // entry point:

  virtual int operator()(int argc, char** argv);
  static int  AcquisitionErrorHandler(Tcl_Event* pEvent, int flags);

  // Segments of operation.

private:
  void startOutputThread(std::string ring);
  void startTclServer();
  void startInterpreter();

  void setConfigFiles();
  void initializeBufferPool();
  void enumerateVMUSB();

  // static functions:

  static int AppInit(Tcl_Interp* interp);
  static std::string makeConfigFile(std::string baseName);
  static std::string destinationRing(const char* pRingName);

  static void ExitHandler(void* pData);
  static CTCLObject makeCommand(
    CTCLInterpreter* pInterp, const char* verb, std::string argument
  );
  

};
#endif
