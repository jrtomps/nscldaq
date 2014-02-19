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


class CTCLInterpreter;
struct Tcl_Interp;


/*!
   This class is  the thread that is the main application startup thread.
   We have to do a bunch of initialization:
   - Set up the initial run state.
   - Set up the Tcl Interpreter and its commands for the main
     program.
   - Start the output thread
   - Start the Tcl Server thread.
   - Pass control to the Tcl event loop.

   
   Since the lifetime of this application is the lifetime of the program,
   storage management will be a bit sloppy.

*/
class CTheApplication
{
private:
  static bool          m_Exists; //!< Enforce singletons via exceptions.
  int                  m_Argc;
  char**               m_Argv;
  CTCLInterpreter*     m_pInterpreter;
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

  // Segments of operation.

private:
  void startOutputThread(std::string pRing);
  void startTclServer(int port);
  void startInterpreter();
  void createUsbController(const char* pSerialNo = NULL);
  void setConfigFiles(const char* pDaqConfig, const char* pCtlConfig);
  void initializeBufferPool();
  void enumerate();
  std::string destinationRing(const char* pRingName);

  // static functions:

  static int AppInit(Tcl_Interp* interp);
  static std::string makeConfigFile(std::string baseName);

  static void ExitHandler(void* pData);

};
#endif
