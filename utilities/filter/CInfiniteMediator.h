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



#ifndef CINFINITEMEDIATOR_H
#define CINFINITEMEDIATOR_H

#include <CMediator.h>

class CDataSource;
class CFilter;
class CDataSink;
class CRingItem;
class CRingStateChangeItem;


class CInfiniteMediator : public CMediator
{
  public:
    // The constructor
    CInfiniteMediator(CDataSource* source, CFilter* filter, CDataSink* sink);

    virtual ~CInfiniteMediator();

  private:
    // Copy and assignment do not make sense because ownership
    // is not transferrable of the CDataSource and CDataSink.
    CInfiniteMediator(const CInfiniteMediator&);
    CInfiniteMediator& operator=(const CInfiniteMediator&);

  public:
    /**! The main loop
    */
    virtual void mainLoop();

};

#endif
