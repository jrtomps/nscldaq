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
#include "CFragmentHandlerCommand.h"
#include "fragment.h"
#include "CFragmentHandler.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <tcl.h>
#include <stdint.h>
#include <Exception.h>
#include <stdexcept>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>

static const int TCL_CHUNKSIZE(128*1024); // Tcl_Read fails badly on multiMbyte reads!!

/**
 * Construct the object:
 * @param interp - reference to an encpasulated interpreter.
 * @param name   - Name to give to the command.
 * @param registerMe - Optional parameter which if true (default) autoregisters the command.
 * 
 */
CFragmentHandlerCommand::CFragmentHandlerCommand(CTCLInterpreter& interp,
						std::string name,
						bool registerMe) :
  CTCLObjectProcessor(interp, name, registerMe)
{
  
}

/**
 * Destructor
 */
CFragmentHandlerCommand::~CFragmentHandlerCommand() {}

/**
 * Command processor
 * - Ensure a channel name is present.
 * - Drain the message body from the channel
 *
 * @param interp - reference to the encapsulated interpreter.
 * @param objv   - reference to a vetor of encpasulated Tcl_Obj*'s.
 *
 * @return int
 * @retval TCL_OK - success.
 * @retval TCL_ERROR -Failure.
 *
 * @note: TODO:  Deal with running on a big endian system:
 */
int
CFragmentHandlerCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // objv must have the command name and a socket name:
  
  int status = TCL_OK;

  try {
    
    if (objv.size() != 2) {
      interp.setResult(std::string("Incorrect number of parameters"));
      return TCL_ERROR;
    }
    // Translate the channel name to a Tcl_Channel: 
    
    objv[1].Bind(interp);
    std::string channelName = objv[1];
    
    Tcl_Channel pChannel = Tcl_GetChannel(interp.getInterpreter(), channelName.c_str(), NULL);
    if (pChannel == NULL) {
    interp.setResult(std::string("Tcl does not know about this channel name"));
    return TCL_ERROR;
    }


    // Read the message length:

    uint32_t msgLength;
    int n = Tcl_Read(pChannel, reinterpret_cast<char*>(&msgLength), sizeof(msgLength));
    if (n != sizeof(msgLength))  {
      interp.setResult("Messge Lnegth read failed");
      return TCL_ERROR;
    }

    // ..and the body itself.


    if (msgLength > 0) {
      uint8_t* msgBody = new uint8_t[msgLength];
      //n    = Tcl_Read(pChannel, reinterpret_cast<char*>(msgBody), msgLength);
      n = tclRead(pChannel, reinterpret_cast<char*>(msgBody), msgLength);
      if(n != msgLength) {
	interp.setResult("Message Body could not be completely read");
	delete []msgBody;
	return TCL_ERROR;
      }
      
      // Dispatch the body as the flattened fragments they are:
      
      CFragmentHandler* pHandler = CFragmentHandler::getInstance();
      pHandler->addFragments(msgLength, reinterpret_cast<EVB::pFlatFragment>(msgBody));
      delete []msgBody;
    }
    


    
    
    
  }
  catch (const char* m) {
    interp.setResult(m);
    status = TCL_ERROR;
  }
  catch (std::string msg) {
    interp.setResult(msg);
    status = TCL_ERROR;
  }
  catch (CException& e) {
    std::string msg = e.ReasonText();
    msg += ": ";
    msg += e.WasDoing();
    interp.setResult(msg);
    status = TCL_ERROR;
  }
  catch (std::exception& e) {
    interp.setResult(e.what());
    status = TCL_ERROR;
  }
  catch (int i) {
    char msg[1000];
    sprintf(msg, "Integer exception: %d if errno: %s\n", i, strerror(i));
    interp.setResult(msg);
    status = TCL_ERROR;
  }
  catch (...) {
    interp.setResult("Unanticipated exception in fragment handler");
    status = TCL_ERROR;
  }

  return status;
  
}

/**
 * tclRead
 *
 *  Do a large read from a Tcl channel by breaking it up into reads no bigger
 *  than TCL_CHUNKSIZE since multimbyte reads seem to make Tcl_Read segfault!
 *
 * @param pChannel - really a Tcl_Channel the channel to read from.
 * @param pBuffer   - Pointer to the buffer into which to read data.
 * @param bytes    - Number of bytes to read, pBuffer must be at least this big.
 *
 * @return size_t number of bytes read. On error could be less than the requested.
 */
size_t
CFragmentHandlerCommand::tclRead(void* pChannel, char* pBuffer, size_t bytes)
{
  Tcl_Channel c = reinterpret_cast<Tcl_Channel>(pChannel);
  size_t totalReadSize = 0;
  while (bytes) {
    int thisReadSize = bytes;
    if (thisReadSize > TCL_CHUNKSIZE) thisReadSize = TCL_CHUNKSIZE;
    int bytesRead = Tcl_Read(c, pBuffer, thisReadSize);
    if (bytesRead != thisReadSize) {
      // error of some sort:

      if (bytesRead < 0) {
	return totalReadSize;
      } else {
	totalReadSize += bytesRead;
	return totalReadSize;
      }
    }
    // Do the partial read book keeping, thisReadSize more read,
    //  this ReadSize fewer to read, and next read is thisReadSize further into
    // the bufer:

    totalReadSize += thisReadSize;
    pBuffer       += thisReadSize;
    bytes         -= thisReadSize;
  }
  // Success
  return totalReadSize;
}
