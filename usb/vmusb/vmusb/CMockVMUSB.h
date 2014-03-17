
#ifndef CMOCKVMUSB_H
#define CMOCKVMUSB_H


#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"

class CMockVMUSB : public CVMUSB
{
    private:
    std::vector<std::string> m_opRecord;
    std::map<uint32_t, uint32_t> m_registers;
    std::map<uint32_t, uint32_t> m_addressData;

    public:
    CMockVMUSB();

    int vmeWrite16(uint32_t address, uint8_t addrMod, uint16_t data);
    int vmeWrite32(uint32_t address, uint8_t addrMod, uint32_t data);
    int vmeRead16(uint32_t address, uint8_t addrMod, uint16_t& data);
    int vmeRead32(uint32_t address, uint8_t addrMod, uint32_t& data);

//    int executeList(CVMUSBReadoutList& list, void* buffer, size_t bufsize, size_t* nbytes);
     void     writeActionRegister(uint16_t value);

     int readFirmwareID();

     void     writeGlobalMode(uint16_t value);
     int  readGlobalMode();

     void     writeDAQSettings(uint32_t value);
     uint32_t readDAQSettings();

     void    writeLEDSource(uint32_t value);
     int     readLEDSource();

     void     writeDeviceSource(uint32_t value);
     int      readDeviceSource();

     void     writeDGG_A(uint32_t value);
     uint32_t readDGG_A();

     void     writeDGG_B(uint32_t value);
     uint32_t readDGG_B();

     void     writeDGG_Extended(uint32_t value);
     uint32_t readDGG_Extended();

     uint32_t readScalerA();
     uint32_t readScalerB();

     void     writeBulkXferSetup(uint32_t value);
     int      readBulkXferSetup();

     void writeEventsPerBuffer(uint32_t value);
     uint32_t readEventsPerBuffer(void);

    const std::vector<std::string>& getOperationRecord() const { return m_opRecord;}

    private:
     void setUpRegisterMap(); 

    template<class T>
     void recordVMEOperation(std::string opname, uint32_t address, uint8_t addrMod, T data);
    template<class T>
     void recordOperation(std::string opname, T data);
};

#endif
