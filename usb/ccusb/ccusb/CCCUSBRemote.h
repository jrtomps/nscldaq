/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       NSCL DAQ Development Group 
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/


#ifndef __CCCUSBREMOTE_H
#define __CCCUSBREMOTE_H

#include <CCCUSB.h> 
#include <vector>
#include <stdint.h>
#include <sys/types.h>
#include <string>
#include <cstdlib>

// Forward Class definitions:

class CCCUSBReadoutList;
class CSocket;
class CTCLInterpreter;

/**
 *  This class provides remote access to the VM-USB through the
 *  VMUSBModule support in the tcl server.
 *  An external program can link to libVMUSBRemote.so, instantiate
 *  this class and pretty much have its way with the VM-USB
 *  as if it owned it.  
 *  Restrictions:
 *   - lists cannot be downloaded (only immediate lists can be run).
 *   - the action register is not accessible.
 *   - The IRQ Mask cannot be written as that abomination requires 
 *     action register operations
 *   - while internal registers can be manipulated be aware that
 *     # This may cuase an active readout to have undefined results.
 *     # Registers you write may be overwritten by Readout's startup 
 *     # code.
 */
class CCCUSBRemote : public CCCUSB
{

    // Class member data.
private:
  std::string      m_deviceName;
  std::string      m_host;    /* host name */
  std::string      m_portStr; /* port number as a string */
  CSocket*         m_pSocket; /* Connection with remote system. */
  CTCLInterpreter* m_pInterp; /* Having this makes TclList processing easier. */
  std::string      m_lastError;
public:

    // Constructors and other canonical functions.
    // Note that since destruction closes the handle and there's no
    // good way to share usb objects, copy construction and
    // assignment are forbidden.
    // Furthermore, since constructing implies a usb_claim_interface()
    // and destruction implies a usb_release_interface(),
    // equality comparison has no useful meaning either:

  CCCUSBRemote(std::string deviceName = "ccusb", 
               std::string host = "localhost",
               unsigned int port = 27000);
    virtual ~CCCUSBRemote();    // Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CCCUSBRemote(const CCCUSBRemote& rhs);
    CCCUSBRemote& operator=(const CCCUSBRemote& rhs);
    int operator==(const CCCUSBRemote& rhs) const;
    int operator!=(const CCCUSBRemote& rhs) const;
public:
    std::string getLastError() {
      return m_lastError;
    }
    // Register I/O operations.
public:
    virtual void reconnect();
    
    virtual void writeActionRegister(uint16_t action);

    virtual int executeList( CCCUSBReadoutList& list,
                             void*              pReadBuffer,
                             size_t             readBufferSize,
                             size_t*            bytesRead);
    
    virtual int loadList( uint8_t listNumber, CCCUSBReadoutList& list);
      

    // Once the interface is in DAQ auntonomous mode, the application
    // should call the following function to read acquired data.

    virtual int usbRead( void* data, size_t bufferSize, size_t* transferCount,
                         int timeout = 2000);
    
    
    std::string  getHost() { return m_host; }
    unsigned int getPort() { return atoi(m_portStr.c_str()); }
    // Local functions:
private:
    std::string marshallList(CCCUSBReadoutList& list);
    size_t      marshallOutputData(void* pOutputBuffer, const char* reply, size_t maxOutputSize);

    void connect();
    void disconnect();

};



#endif
