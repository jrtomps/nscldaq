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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";// Class: TclServerConnection

//
// Author:
//    Ron Fox
//    NSCL
//    Michigan State University
//    East Lansing, MI 48824-1321
//
//
//////////////////////////.cpp file/////////////////////////////////////////////////////
#include <config.h>
#include "TclServerConnection.h"    				

static const char* pCopyrightNotice = 
"(C) Copyright 1999 NSCL, All rights reserved .cpp \n";

using namespace std;

// Functions for class TclServerConnection

//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     SendCommand(string& rData)
//  Operation Type: 
//     
int TclServerConnection::SendCommand(const string& rData)  
{
  // Sends a command string terminated by a newline "\n".
  // The response is collected into m_response
  //
  // Returns:
  //    Number of characters sent.
  //    0     - Disconnected
  //   -1     - Some errno like error.
  //    n     - Number of bytes actually sent.
  //
  Flush();
  int nSent;
  string MyCopy = rData + '\n';	// Add in the newline.
  m_Response = string("");	// Empty response string first.
  nSent = Send((void*)MyCopy.c_str(), MyCopy.size());
  if(nSent >= 0) GetLine(m_Response);
  return nSent;
}
//////////////////////////////////////////////////////////////////////////////
//
//  Function:       
//     GetLastResponse()
//  Operation Type: 
//     
string TclServerConnection::GetLastResponse()  
{
  // Retrieves  a copy of the  Tcl response to the last command.
    
  return m_Response;

  
}







