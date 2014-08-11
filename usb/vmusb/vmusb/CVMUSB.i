%module CVMUSB
%include "stdint.i"
%{
  #include <CVMUSB.h> 
  #include <CVMUSBusb.h> 

  class CTCLApplication;
  CTCLApplication *gpTCLApplication = 0;
%}

%include "CVMUSB.h"
%include "CVMUSBusb.h"
