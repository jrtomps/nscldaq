/*!
   This file defines PCI/VME interface functions that are
   specific to the WIENER adaptor. 
*/
#ifndef __WIENERAPI_H
#define __WIENERAPI_H
#include <CVMEInterface.h>
class WienerVMEInterface {
public:
   static void ResetVme(void* pHandle);      // Reset the bus.

   // Word sized transfer operations
   static int  ReadWords(void* pHandle,
                         unsigned long nBase,
                         void* pBuffer,
                         unsigned long nWords);
   static int WriteWords(void* pHandle,
                         unsigned long nBase,
                         void* pBuffer,
                         unsigned long nWords);

   // Long sized transfer operations:
                        
   static int ReadLongs(void* pHandle,
                        unsigned long nBase,
                        void* pBuffer,
                        unsigned long nLongs);
   static int WriteLongs(void* pHandle,
                        unsigned long nBase,
                        void* pBuffer,
                        unsigned long nLongs);
                        
   // Byte sized transfers are actually CVMEInterface::Read/Write.
   
   static int ReadBytes(void* pHandle,
                        unsigned long nBase,
                        void* pBuffer,
                        unsigned long nBytes) {
      return CVMEInterface::Read(pHandle, nBase, pBuffer, nBytes);
   }
   static int WriteBytes(void* pHandle,
                        unsigned long nBase,
                        void* pBuffer,
                        unsigned long nBytes) {
      return CVMEInterface::Write(pHandle, nBase, pBuffer, nBytes);
   }
};

#endif
