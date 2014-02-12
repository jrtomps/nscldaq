/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";

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
  // try to open the ring as a producer...
  // check if the ring exists... if it does just
  // try to attach to it as a producer. If a producer
  // exists, this will throw a CErrnoException of errno=EACCES
  if (CRingBuffer::isRing(m_url)) {
    m_pRing = new CRingBuffer(m_url,CRingBuffer::producer);
  } else {
    // If we are here, no ring exists already with the desired
    // name so we can create and produce
    m_pRing = CRingBuffer::createAndProduce(m_url);
  }
}
