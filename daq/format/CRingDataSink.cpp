
#include <CRingDataSink.h>
#include <CRingBuffer.h>
#include <CRingItem.h>
#include <URL.h>

CRingDataSink::CRingDataSink(std::string url)
  : m_pRing(0),
    m_url(url)
{
  openRing();
}

CRingDataSink::~CRingDataSink()
{
  delete m_pRing;
  m_pRing;
}

void CRingDataSink::putItem(const CRingItem& item)
{
  CRingItem& the_item = const_cast<CRingItem&>(item); 

  the_item.commitToRing(*m_pRing);
}


void CRingDataSink::openRing()
{
  m_pRing = CRingBuffer::createAndProduce(m_url);
}
