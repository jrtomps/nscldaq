
#include "CMockVMUSB.h"
#include <sstream>
#include <iomanip>

CMockVMUSB::CMockVMUSB()
  : CVMUSB(), 
    m_opRecord(), 
    m_registers()
{
  setUpRegisterMap();
}


int CMockVMUSB::vmeWrite16(uint32_t address, uint8_t addrMod, uint16_t data)
{
    // add entry to log that we did this operation
    recordVMEOperation("vmeWrite16", address, addrMod, data);

    m_addressData[address] = data;

    return 1;
}

int CMockVMUSB::vmeWrite32(uint32_t address, uint8_t addrMod, uint32_t data)
{
    // add entry to log that we did this operation
    recordVMEOperation("vmeWrite32", address, addrMod, data);

    m_addressData[address] = data;

    return 1;
}

int CMockVMUSB::vmeRead16(uint32_t address, uint8_t addrMod, uint16_t& data)
{
   
  data = 0;
  std::map<uint32_t, uint32_t>::iterator it = m_addressData.find(address);
  if (it != m_addressData.end()) {
      data = it->second; 
  }

  // add entry to log that we did this operation
  recordVMEOperation("vmeRead16", address, addrMod, data);

  return 1;
}

int CMockVMUSB::vmeRead32(uint32_t address, uint8_t addrMod, uint32_t& data)
{

  data = 0;
  std::map<uint32_t, uint32_t>::iterator it = m_addressData.find(address);
  if (it != m_addressData.end()) {
      data = it->second; 
  }

  // add entry to log that we did this operation
  recordVMEOperation("vmeRead32", address, addrMod, data);

  return 1;
}

int CMockVMUSB::readFirmwareID()
{
  uint32_t fakeID = m_registers[0x0]; 
  // add entry to log that we did this operation
  recordOperation("readFirmwareID", fakeID);

  return fakeID; 
}

void CMockVMUSB::writeActionRegister(uint16_t value)
{
  // add entry to log that we did this operation
  recordOperation("writeActionRegister", value);
  m_registers[0x1] = value;
}

void CMockVMUSB::writeGlobalMode(uint16_t value)
{
  // add entry to log that we did this operation
  recordOperation("writeGlobalMode", value);
  m_registers[0x4] = value;
}

int CMockVMUSB::readGlobalMode()
{
  int value = m_registers[0x4];
  // add entry to log that we did this operation
  recordOperation("readGlobalMode", value);
  return value;
}

void CMockVMUSB::writeDAQSettings(uint32_t value)
{
  m_registers[0x8] = value;
  // add entry to log that we did this operation
  recordOperation("readDAQSettings", value);
}

uint32_t CMockVMUSB::readDAQSettings()
{
  uint32_t value = m_registers[0x8];
  // add entry to log that we did this operation
  recordOperation("readDAQSettings", value);

  return value;
}

void CMockVMUSB::writeLEDSource(uint32_t value)
{
  m_registers[0xc] = value;
  // add entry to log that we did this operation
  recordOperation("writeLEDSource", value);
}

int CMockVMUSB::readLEDSource()
{
  uint32_t value = m_registers[0xC];
  // add entry to log that we did this operation
  recordOperation("readLEDSource", value);

  return value;
}

void CMockVMUSB::writeDeviceSource(uint32_t value)
{
  m_registers[0x10] = value;
  // add entry to log that we did this operation
  recordOperation("writeDeviceSource", value);
}

int CMockVMUSB::readDeviceSource()
{
  uint32_t value = m_registers[0x10];
  // add entry to log that we did this operation
  recordOperation("readDeviceSource", value);

  return value;
}

void CMockVMUSB::writeDGG_A(uint32_t value)
{
  m_registers[0x14] = value;
  // add entry to log that we did this operation
  recordOperation("writeDGG_A", value);
}

uint32_t CMockVMUSB::readDGG_A()
{
  uint32_t value = m_registers[0x14];
  // add entry to log that we did this operation
  recordOperation("readDGG_A", value);

  return value;
}

void CMockVMUSB::writeDGG_B(uint32_t value)
{
  m_registers[0x18] = value;
  // add entry to log that we did this operation
  recordOperation("writeDGG_B", value);
}

uint32_t CMockVMUSB::readDGG_B()
{
  uint32_t value = m_registers[0x18];
  // add entry to log that we did this operation
  recordOperation("readDGG_B", value);

  return value;
}

void CMockVMUSB::writeDGG_Extended(uint32_t value)
{
  m_registers[0x38] = value;
  // add entry to log that we did this operation
  recordOperation("writeDGG_Extended", value);
}

uint32_t CMockVMUSB::readDGG_Extended()
{
  uint32_t value = m_registers[0x48];
  // add entry to log that we did this operation
  recordOperation("readDGG_Extended", value);

  return value;
}

uint32_t CMockVMUSB::readScalerA()
{
  uint32_t value = m_registers[0x1c];
  // add entry to log that we did this operation
  recordOperation("readScalerA", value);

  return value;
}

uint32_t CMockVMUSB::readScalerB()
{
  uint32_t value = m_registers[0x20];
  // add entry to log that we did this operation
  recordOperation("readScalerB", value);

  return value;
}

void CMockVMUSB::writeBulkXferSetup(uint32_t value)
{
  m_registers[0x3c] = value;
  // add entry to log that we did this operation
  recordOperation("writeBulkXferSetup", value);
}

int CMockVMUSB::readBulkXferSetup()
{
  uint32_t value = m_registers[0x3c];
  // add entry to log that we did this operation
  recordOperation("readBulkXferSetup", value);

  return value;
}

void CMockVMUSB::writeEventsPerBuffer(uint32_t value)
{
  m_registers[0x24] = value;
  // add entry to log that we did this operation
  recordOperation("writeEventsPerBuffer", value);
}

uint32_t CMockVMUSB::readEventsPerBuffer()
{
  uint32_t value = m_registers[0x24];
  // add entry to log that we did this operation
  recordOperation("readEventsPerBuffer", value);

  return value;
}

void CMockVMUSB::setUpRegisterMap()
{
  m_registers[0x0] = 0xffffffff; // firmwareID
  m_registers[0x1] = 0; // action register 
  m_registers[0x4] = 0; // global mode
  m_registers[0x8] = 0; // daq settings
  m_registers[0xC] = 0; // user led
  m_registers[0x10] = 0; // user devices source selecor
  m_registers[0x14] = 0; // dgg_A settings
  m_registers[0x18] = 0; // dgg_B settings
  m_registers[0x1C] = 0; // scaler_A data
  m_registers[0x20] = 0; // scaler_B data
  m_registers[0x24] = 0; // events per buffer
  m_registers[0x28] = 0; // IRQ vectors 1&2
  m_registers[0x2C] = 0; // IRQ vectors 3&4
  m_registers[0x30] = 0; // IRQ vectors 5&6
  m_registers[0x34] = 0; // IRQ vectors 7&8
  m_registers[0x38] = 0; // extended dgg_A/B settings
  m_registers[0x3C] = 0; // USB bulk transfer

}


template<class T>
void CMockVMUSB::recordVMEOperation(std::string opname, uint32_t address, uint8_t addrMod, T data)
{

    std::stringstream logentry;
    logentry << opname << "(0x" 
             << std::hex << std::setfill('0') << std::setw(8) 
             << address;
    logentry << "," << std::setw(2) << uint16_t(addrMod);
    logentry << "," << std::setw(8) << data;
    logentry << ")";

    m_opRecord.push_back(logentry.str());
}

template<class T>
void CMockVMUSB::recordOperation(std::string opname, T data)
{

    std::stringstream logentry;
    logentry << opname << "(0x" 
             << std::hex << std::setfill('0') << std::setw(8) 
             << data << ")";

    m_opRecord.push_back(logentry.str());
}
