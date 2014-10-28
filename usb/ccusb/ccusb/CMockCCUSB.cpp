
#include <CMockCCUSB.h>

#include <iomanip>

using namespace std;

void CMockCCUSB::reconnect() {
  m_record.push_back("reconnect");
}


void CMockCCUSB::writeActionRegister(uint16_t val)
{
  m_formatter.str(""); m_formatter.clear();
  m_formatter << hex << setfill('0');
  m_formatter << "writeActionRegister(0x" << setw(8) << val << ")";

  m_record.push_back(m_formatter.str());
}

int CMockCCUSB::executeList(CCCUSBReadoutList& list,
    void* pReadBuffer,
    size_t readBufferSize,
    size_t* bytesRead)
{
  m_formatter << hex << setfill('0');

  std::vector<uint16_t> ops = list.get();
  unsigned int size = ops.size();

  // 
  m_record.reserve(m_record.size() + size + 2);
  m_record.push_back("executelist-begin");

  // add all of the operations
  for (unsigned int index=0; index < size; ++index) {

    m_formatter.str(""); m_formatter.clear();

    m_formatter << setw(8) << ops.at(index);
    m_record.push_back(m_formatter.str());
  }

  m_record.push_back("executelist-end");
}

int CMockCCUSB::loadList(uint8_t listNumber, CCCUSBReadoutList& list) 
{

  m_formatter << hex << setfill('0');

  std::vector<uint16_t> ops = list.get();
  unsigned int size = ops.size();

  //  make sure we have enough space in one potential reallocation
  m_record.reserve(m_record.size() + size + 2);

  m_formatter.str(""); m_formatter.clear();
  m_formatter << "loadlist-begin_" << listNumber;
  m_record.push_back(m_formatter.str());

  // add all of the operations
  for (unsigned int index=0; index < size; ++index) {

    m_formatter.str(""); m_formatter.clear();

    m_formatter << setw(8) << ops.at(index);
    m_record.push_back(m_formatter.str());
  }

  m_record.push_back("loadlist-end");
}

int CMockCCUSB::usbRead(void* data, size_t bufferSize, size_t* transferCount, 
    int timeout) 
{
  m_formatter.str(""); m_formatter.clear();
  m_formatter << hex << setfill('0');

  m_formatter << "usbRead( 0x" << setw(8) << data << ", " << dec << bufferSize;
  m_formatter << ", 0x" << hex << setw(8) << transferCount << ", ";
  m_formatter << dec << timeout << ")";

  m_record.push_back(m_formatter.str());
}

