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
using namespace std;
#include <spectrodaq.h>


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

   Due to the way Spectrodaq clients work, we are not able to make a pure
   singleton object with private constructors, because we will need to
   make a static instantiation of the object.  However that static instance
   provides us with the handles we need to get the tcl interpreter started and
   extended.   There will be more comments about this in startTcl and
   AppInit().
   
   Since the lifetime of this application is the lifetime of the program,
   storage management will be a bit sloppy.

*/
class CTheApplication : DAQROCNode // Mandatory for spectrodaq init etc.
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
protected:
  virtual int operator()(int argc, char** argv);

  // Segments of operation.

private:
  void startOutputThread();
  void startTclServer();
  void startInterpreter();
  void createUsbController();
  void setConfigFiles();
  void initializeBufferPool();

  // static functions:

  static int AppInit(Tcl_Interp* interp);
  std::string makeConfigFile(std::string baseName);

};
#endif
