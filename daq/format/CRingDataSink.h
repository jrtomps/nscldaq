

#ifndef CRINGDATASINK_H
#define CRINGDATASINK_H

#include <CDataSink.h>

#include <string>

class CRingItem;
class CRingBuffer;

class CRingDataSink : public CDataSink
{
  private:
    CRingBuffer*  m_pRing;
    std::string   m_url; 
 
  public:
    CRingDataSink(std::string url);
    virtual ~CRingDataSink();

  private:
    CRingDataSink(const CRingDataSink& rhs);
    CRingDataSink& operator=(const CRingDataSink& rhs);
    int operator==(const CRingDataSink& rhs) const;
    int operator!=(const CRingDataSink& rhs) const;

  public:
    void putItem(const CRingItem& item);

  private:
    void openRing();

};
#endif
