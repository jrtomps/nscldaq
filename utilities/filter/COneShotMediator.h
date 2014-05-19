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



#ifndef CONESHOTMEDIATOR_H
#define CONESHOTMEDIATOR_H

#include <COneShotHandler.h>
#include <CMediator.h>

class CDataSource;
class CFilter;
class CDataSink;
class CRingItem;
class CRingStateChangeItem;


class COneShotMediator : public CMediator
{
  private:
    COneShotHandler m_oneShot; //!< logic handler for oneshot operations

  public:
    // The constructor
    COneShotMediator(CDataSource* source, CFilter* filter, CDataSink* sink);
    // The constructor
    COneShotMediator(CDataSource* source, CFilter* filter, CDataSink* sink, 
                      int nsources);

    virtual ~COneShotMediator();

  private:
    // Copy and assignment do not make sense because ownership
    // is not transferrable of the CDataSource and CDataSink.
    COneShotMediator(const COneShotMediator&);
    COneShotMediator& operator=(const COneShotMediator&);

  public:

    /**! The main loop
    */
    void mainLoop();

    /**! Set one shot handler 
    */
    

};

#endif
