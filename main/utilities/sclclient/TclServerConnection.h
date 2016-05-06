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

// Class: TclServerConnection                     //ANSI C++
//
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved .h
//

#ifndef __TCLSERVERCONNECTION_H  //Required for current class
#define __TCLSERVERCONNECTION_H

                               //Required for base classes
#ifndef __TCPCLIENT_H
#include "TcpClient.h"
#endif
                               
#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


class TclServerConnection  : public TcpClientConnection        
{                       
			
   std::string m_Response; //Response from last tcl command        

protected:

public:

   // Constructors and other cannonical operations:

  TclServerConnection (const std::string& RemoteHost=std::string("localhost"), 
		       int nPort=2700)    : 
    TcpClientConnection(RemoteHost, nPort),
    m_Response(std::string("")) 
  {}
  virtual ~TclServerConnection ( )  // Destructor 
  { }  
private:
  TclServerConnection (const TclServerConnection& aTclServerConnection );
  TclServerConnection& 
  operator= (const TclServerConnection& aTclServerConnection);
  int operator== (const TclServerConnection& aTclServerConnection) const;
public:	

   int SendCommand (const std::string& rData)    ;
   std::string GetLastResponse ()    ;
 
protected:

private:

};

#endif
