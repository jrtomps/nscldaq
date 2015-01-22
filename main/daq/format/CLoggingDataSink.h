
#ifndef CLOGGINGDATASINK_H
#define CLOGGINGDATASINK_H

#include <vector>
#include <CRingItem.h>
#include <CDataSink.h>

class CLoggingDataSink : public CDataSink
{
  private:
    std::vector<CRingItem> m_log;

  public:
    virtual void putItem(const CRingItem& item);
    virtual void put(const void* pData, size_t nBytes);

    std::vector<CRingItem> getLog() { return m_log; }
};

#endif
