%module CVMUSB
%include <stdint.i>

%ignore CVMUSB::executeList(CVMUSBReadoutList& list, void* pReadBuffer, size_t readBufferSize, size_t* bytesRead);
%ignore CVMUSBusb::executeList(CVMUSBReadoutList& list, void* pReadBuffer, size_t readBufferSize, size_t* bytesRead);

%{
  #include <CVMUSB.h> 
  #include <CVMUSBusb.h> 

  class CTCLApplication;
  CTCLApplication *gpTCLApplication = 0;
%}

%include "CVMUSB.h"
%include "CVMUSBusb.h"

