/*!
  This file describes interfaces into the CAMAC system.  
  At present the following CAMAC interfaces are supported
  and this file simply includes the one selected at compile time by the
  value of the CAMACINTERFACE macro:

  CESCAMAC  - CES CBD8210 VME Camac branch highway interface.
  VC32CAMAC - WIENER VC32/CC32 VME - single crate controller.

  Acknowledgements:
   - Thanks to Dave Caussyn of FSU for the intial VC32CAMAC support this
     version's software is based on.

*/
#ifndef __CAMAC_H
#define __CAMAC_H


#ifdef CESCAMAC
#include <cescamac.h>
#else
#ifdef  VC32CAMAC
#include <wienercamac.h>
#endif
#endif

#ifndef CBDPTR                      /* Macro defined by *camac.h */

#error "You must select a supported value for the CAMAC INTERFACE macro"

#endif

#endif
