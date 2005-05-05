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

// Class: CVmeCommand                     //ANSI C++

#ifndef __VMECOMMAND_H  //Required for current class
#define __VMECOMMAND_H

#ifndef __IOSTREAM_H            //Required for cin and cout
#include <Iostream.h>                 
#endif

                               //Required for base classes
#ifndef __TCLPROCESSOR_H
#include <TCLProcessor.h>
#endif

#ifndef _VMEMAPCOMMAND_H
#include <VMEMapCommand.h>
#endif

#ifndef __STL_MAP                               
#include <map>        //Required for include files
#define __STL_MAP
#endif

#ifndef __STL_STRING
#include <string>        //Required for include files
#define __STL_STRING
#endif

#ifndef __VMEMODULE_H
#include <VmeModule.h>
#endif

class CVmeCommand  : public CTCLProcessor        
{                       
			
  STD(map)<STD(string),CVMEMapCommand*>        m_Spaces;    //Attribute data member        

  enum Switches {		// switch values
    device,			// -device
    crate,			// -crate
    unknown };			// Anything else.
protected:

    int AddMap (CTCLInterpreter& rInterp, 
		CTCLResult& rResult, int nArgs, char** pArgs)   ;

    int ListMaps (CTCLInterpreter& rInterp, 
		  CTCLResult& rResult, int nArgs, char** pArgs)   ;

    int DeleteMap (CTCLInterpreter& rInterp, 
		   CTCLResult& rResult, int nArgs, char** pArgs)   ;

    int Usage(CTCLResult& rResult);

public:

  CVmeCommand (CTCLInterpreter* pInterp)    :
    CTCLProcessor("vme", pInterp)
  { 
  } 
  virtual ~ CVmeCommand ( );
				
  //Get accessor function for non-static attribute data member

  STD(map)<STD(string),CVMEMapCommand*>& getm_Spaces()
  { 
    return m_Spaces;
  }
protected:
                       
  void setm_Spaces (const STD(map)<STD(string),CVMEMapCommand*>& am_Spaces)
  { 
    m_Spaces = am_Spaces;
  }

public:
  virtual   int operator() (CTCLInterpreter& rInterpreter, 
			    CTCLResult& rResult, int nArgs, char** pArgs)    ;


  // Utility functions:
public:

  static unsigned long     TextToULong(const char* text);

private:
  // The functions below throw strings if they fail.
  //
  static  CVmeModule::Space  UserToLogical(const STD(string)& rsUser);
  static Switches MatchSwitch(const char* pSwitch);
};


#endif
