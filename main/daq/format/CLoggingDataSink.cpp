
#include <stdint.h>
#include <CLoggingDataSink.h>
#include <CRingItemFactory.h>
#include <DataFormat.h>

void CLoggingDataSink::putItem(const CRingItem& item)
{
  m_log.push_back(item); 
}

void CLoggingDataSink::put(const void* pData, size_t nBytes)
{
  const uint8_t* iter = reinterpret_cast<const uint8_t*>(pData);
  const uint8_t* end   = iter + nBytes;

  while (iter != end) {
    const RingItemHeader* header = reinterpret_cast<const RingItemHeader*>(iter);
    
    CRingItem* item = CRingItemFactory::createRingItem(iter);
    m_log.push_back(*item);
    delete item;

    iter += header->s_size;
  }
}
