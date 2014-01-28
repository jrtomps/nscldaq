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
