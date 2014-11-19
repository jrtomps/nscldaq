
#ifndef CVMEREADOUTLIST_H
#define CVMEREADOUTLIST_H

#include <stdint.h>

class CVMEReadoutList {

  public:
    virtual void clear()=0;
    virtual size_t size() const=0;

    virtual void append(const CVMEReadoutList& list)=0;

    virtual  void addRegisterRead(unsigned int address)=0;
    virtual  void addWrite32(uint32_t address, uint8_t amod, uint32_t datum)=0;
    virtual  void addWrite16(uint32_t address, uint8_t amod, uint16_t datum)=0;
    virtual  void addWrite8(uint32_t address, uint8_t amod, uint8_t datum)=0;

    virtual  void addRead32(uint32_t address, uint8_t amod)=0;
    virtual  void addRead16(uint32_t address, uint8_t amod)=0;
    virtual  void addRead8(uint32_t address, uint8_t amod)=0;

    virtual void addBlockRead32(uint32_t baseAddress, uint8_t amod, size_t transfers)=0;
    virtual void addFifoRead32(uint32_t baseAddress, uint8_t amod, size_t transfers)=0;
    virtual void addFifoRead16(uint32_t baseAddress, uint8_t amod, size_t transfers)=0;

    virtual void addBlockWrite32(uint32_t baseAddress, uint8_t amod, void* data, 
                                 size_t transfers)=0;

    virtual void addBlockCountRead8(uint32_t address, uint32_t mask, uint8_t amod)=0;
    virtual void addBlockCountRead16(uint32_t address, uint32_t mask, uint8_t amod)=0;
    virtual void addBlockCountRead32(uint32_t address, uint32_t mask, uint8_t amod)=0;
    virtual void addMaskedCountBlockRead32(uint32_t address, uint8_t amod)=0;
    virtual void addMaskedCountFifoRead32(uint32_t address, uint8_t amod)=0;

    void addDelay(uint8_t clocks)=0;
    void addMarker(uint16_t value)=0;
    
};

#endif
