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


#ifndef CVMUSBREMOTE_H
#define CVMUSBREMOTE_H

#include <CVMUSB.h> 

#include <vector>
#include <stdint.h>
#include <sys/types.h>
#include <string>

// Forward Class definitions:

class CVMUSBReadoutList;
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
class CVMUSBRemote : public CVMUSB
{

    // Class member data.
private:
  std::string      m_deviceName;
  CSocket*         m_pSocket;	/* Connection with remote system. */
  CTCLInterpreter* m_pInterp;	/* Having this makes TclList processing easier. */
  std::string      m_lastError;
public:

    // Constructors and other canonical functions.
    // Note that since destruction closes the handle and there's no
    // good way to share usb objects, copy construction and
    // assignment are forbidden.
    // Furthermore, since constructing implies a usb_claim_interface()
    // and destruction implies a usb_release_interface(),
    // equality comparison has no useful meaning either:

  CVMUSBRemote(std::string deviceName = "vmusb", 
	       std::string host = "localhost",
	       unsigned int port = 27000);
    virtual ~CVMUSBRemote();		// Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CVMUSBRemote(const CVMUSBRemote& rhs);
    CVMUSBRemote& operator=(const CVMUSBRemote& rhs);
    int operator==(const CVMUSBRemote& rhs) const;
    int operator!=(const CVMUSBRemote& rhs) const;
public:
    std::string getLastError() {
      return m_lastError;
    }
    // Register I/O operations.
public:

    void writeActionRegister(uint16_t action);
    void  writeRegister(unsigned int address, uint32_t data);
    uint32_t readRegister(unsigned int address);

    int executeList(CVMUSBReadoutList& list,
		    void*               pReadBuffer,
		    size_t              readBufferSize,
		    size_t*             bytesRead);
    
    int loadList(uint8_t listNumber, CVMUSBReadoutList& list,
                 off_t listOffset = 0);
      

    // Once the interface is in DAQ auntonomous mode, the application
    // should call the following function to read acquired data.

    int usbRead(void* data, size_t bufferSize, size_t* transferCount,
		            int timeout = 2000);
    

    // Local functions:
private:


    std::string marshallList(CVMUSBReadoutList& list);
    size_t      marshallOutputData(void* pOutputBuffer, const char* reply, size_t maxOutputSize);


};



#endif
