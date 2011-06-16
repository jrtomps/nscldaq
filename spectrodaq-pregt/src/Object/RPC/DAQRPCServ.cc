/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright = "(C) Copyright Michigan State University 2002, All rights reserved";

using namespace std;

/*=========================================================================*/
// DAQRPCServ.cc 
//
// Author:
//		Eric Kasten
//		NSCL
//		Michigan State University
//		East Lansing, MI 48824-1321
//		mailto:kasten@nscl.msu.edu
//

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#ifndef MAINEXTERNS_H
#include <mainexterns.h>
#endif

#ifndef DAQRPCSERV_H
#include <DAQRPCServ.h>
#endif

#ifndef DAQMAIN_H
#include <DAQMain.h>
#endif

#ifndef DAQSTATUS_H
#include <DAQStatus.h>
#endif

#ifndef DAQRPCSERVTHREAD_H
#include <DAQRPCServThread.h>
#endif

#ifndef DAQTHREADHASHTABLE_H                                                    
#include <DAQThreadHashTable.h>                                                 
#endif

#ifndef DAQDISPATCHER_H
#include <DAQDispatcher.h>
#endif

extern int daq_debug_level;


#define  DAQRPCSERV_POOL_TIMEOUT 1000

/*===================================================================*/
// DAQRPCServ::DAQRPCServ       
//                                   
// Constructor.  Queue will be dynamically resized.                     
//                                 
DAQRPCServ::DAQRPCServ(bool aPersist) 
{
  SetType(DAQTYPEID(DAQRPCServ)); 
  hi_hand = -1;
  persist = aPersist;
  state = RPCSERV_HALTED;
}

/*===================================================================*/
// DAQRPCServ::DAQRPCServ       
//                                   
// Constructor with queue size.
//                                 
DAQRPCServ::DAQRPCServ(int aSize,bool aPersist) 
{
  SetType(DAQTYPEID(DAQRPCServ)); 
  hi_hand = -1;
  persist = aPersist;
  state = RPCSERV_HALTED;
  rqueue.Resize(aSize);
}

/*===================================================================*/
// DAQRPCServ::~DAQRPCServ       
//                                   
// Destructor
//                                 
DAQRPCServ::~DAQRPCServ()
{
  int i;

  Halt();

  for (i = 0; i < hi_hand; i++) {
    if (handtab[i] != NULL) {
       --(handtab[i]->refcnt);
       if (handtab[i]->refcnt() <= 0) delete handtab[i];
       handtab[i] = NULL;
    }
  }

  hi_hand = -1;
  state = RPCSERV_HALTED;
}

/*===================================================================*/
// DAQRPCServ::Listen
//                                   
// Begin networking
//                                 
int DAQRPCServ::Listen(DAQURL& url)
{
  DAQCommInfo info;

  try {
    dcom.Open(url,info);
    state = RPCSERV_LISTENING;
  } catch (NSCLException& e) {
    state = RPCSERV_HALTED;
    THROW(e);
  }

  return(0);
}

/*===================================================================*/
// DAQRPCServ::Halt
//                                   
// Cease networking
//                                 
int DAQRPCServ::Halt()
{
  dcom.Close();
  state = RPCSERV_HALTED;
  return(0);
}

/*===================================================================*/
// DAQRPCServ::IsPersistent
//                                   
// Return true if server is persistent.
//                                 
bool DAQRPCServ::IsPersistent()
{
  return(persist);
}

/*===================================================================*/
// DAQRPCServ::Register
//                                   
// Register an RPC handler.  Notice that this takes a pointer as an
// argument.
//                                 
int DAQRPCServ::Register(DAQRPCHandler *hand)
{
  int i;
  short h;

  h = hand->GetHandle();
 
  if (h > hi_hand) {          // Init the new members of the array
    handtab.Resize(h+1);     
    if (hi_hand < 0) hi_hand = 0;
    for (i = hi_hand+1; i <= h; i++) handtab[i] = NULL;
    hi_hand = h;
  }

  if (handtab[h] != NULL) {  // Remove old handler
    --(handtab[h]->refcnt);
    if (handtab[h]->refcnt() <= 0) delete handtab[h];
    handtab[h] = NULL;
  }

  ++(hand->refcnt); 
  handtab[h] = hand;
  if (h > hi_hand) hi_hand = h;

  return(0);
}

/*===================================================================*/
// DAQRPCServ::UnRegister
//                                   
// Unregister an RPC handler 
//                                 
int DAQRPCServ::UnRegister(const short h)
{
  if (h < 0) return(-1);        // Bogus call
  if (h > hi_hand) return(0);   // Not registered

  if (handtab[h] != NULL) {
    --(handtab[h]->refcnt);
    if (handtab[h]->refcnt() <= 0) delete handtab[h];
    handtab[h] = NULL;
  }

  return(0);
}

/*===================================================================*/
// DAQRPCServ::operator()
//                                   
// Process and RPC call from the queue.  Return the return code
// from the handler call.
//                                 
int DAQRPCServ::operator()(short aHand,DAQObject *pArgs)
{
  int rc(-1);			// Default value.
  short hand;
  DAQRPCQueueItem *itm = NULL;

  itm = rqueue.GetHead();

  if (itm != NULL) {
    hand = itm->GetHandle();

    if (hand != aHand) {  // Miss call
      LOG_AND_THROW(daq_exception_factory.CreateException(DAQCSTR("DAQRPCServ::operator() Call specifies incorrect handle"),DAQEXCPID(DAQInval)));
      return(-1);
    }

    // Call the handler
    if (daq_debug_level > 8) {
      daq_logger.print("DAQRPCServ::operator():  Calling handle=");
      daq_logger.print(aHand);
      daq_logger.print(" with MsgLen=");
      daq_logger.print(itm->GetMsg().GetDataLength());
      daq_logger.println(DAQCSTR(""));
    }

    try {
      if (hand == PersistentCall_h) {
        Schedule(itm->GetCommunicator(),pArgs);
        rc = RPCSERV_REQ_TERM;
      } else {
        rc = (*handtab[hand])(itm->GetCommunicator(),itm->GetMsg(),pArgs);
      }
    } catch(...) {
      daq_logger.printf("DAQRPCServ::operator() Caught exception while calling handle=%d %s\n",hand,DAQCSTR(""));
      itm->GetCommunicator().Close();
      rqueue.Delete(itm); 
      delete itm;
      return(-1);
    }

    if (rc != RPCSERV_REQ_DELAY) {
      try {
        itm->GetCommunicator().Close();
        rqueue.Delete(itm); 
        delete itm;
      } catch(...) {
        daq_logger.printf("DAQRPCServ::operator() Caught exception while deleting item with handle=%d %s\n",hand,DAQCSTR(""));
      }
    }
  }

  return(rc);
}

/*===================================================================*/
// DAQRPCServ::operator()
//                                   
// Process an RPC call from the queue.  Return the return code
// from the handler call.
//                                 
int DAQRPCServ::operator()(short aHand,int aArgc,char **pArgs)
{
  int rc = -1;
  short hand;
  DAQRPCQueueItem *itm = NULL;

  itm = rqueue.GetTail();

  if (itm != NULL) {
    hand = itm->GetHandle();

    if (hand != aHand) {  // Miss call
      LOG_AND_THROW(daq_exception_factory.CreateException(DAQCSTR("DAQRPCServ::operator() Call specifies incorrect handle"),DAQEXCPID(DAQInval)));
      return(-1);
    }

    // Call the handler
    if (daq_debug_level > 8) {
      daq_logger.printf("DAQRPCServ::operator():  Calling handle=%d with MsgLen=%d %s\n",aHand,itm->GetMsg().GetDataLength(),DAQCSTR(""));
    }

    if (hand == PersistentCall_h) {
        Schedule(itm->GetCommunicator(),aArgc,pArgs);
        rc = RPCSERV_REQ_TERM;
    } else if (handtab[hand] != NULL) {
        rc = (*handtab[hand])(itm->GetCommunicator(),itm->GetMsg(),aArgc,pArgs);
    } else {
      daq_logger.printf("DAQRPCServ::operator():  Calling handle=%d with NULL handler %s\n",aHand,DAQCSTR(""));
      rc = RPCSERV_REQ_TERM;
    }

    if (rc != RPCSERV_REQ_DELAY) {
      itm->GetCommunicator().Close();
      rqueue.Delete(itm); 
      delete itm;
    }
  }

  return(rc);
}

/*===================================================================*/
// DAQRPCServ::GetOutStanding
//                                   
// Return the number of messages left in the queue. 
//                                 
int DAQRPCServ::GetOutStanding()
{
  return(rqueue.Count());
}

/*===================================================================*/
// DAQRPCServ::GetNextHandle
//                                   
// Return the next handle that'll be processed by operator()
//                                 
short DAQRPCServ::GetNextHandle()
{
  short hand;
  DAQRPCQueueItem *itm = NULL;

  hand = -1;
  itm = rqueue.GetHead();
  if (itm != NULL) {
    hand = itm->GetHandle();
    rqueue.Delete(itm);   // Round robin the queue, so we don't keep
    rqueue.Append(itm);   //   trying to process the same thing.
  }

  return(hand);
}

/*===================================================================*/
// DAQRPCServ::Accept()
//                                   
// Wait for an RPC call and place in queue.  Return the handle of
// the next request to be processed, or -1 on error.
//                                 
short DAQRPCServ::Accept(int timeout)
{
  int rc = 0;
  short hand;
  short pollevts,rpollevts;
  DAQCommunicator ncom;
  DAQCommInfo info;
  DAQCommMsg  msg;
  DAQRPCQueueItem *itm;

  if (timeout == -1) {
    timeout = DAQRPCSERV_POOL_TIMEOUT;
  }

  if ((state != RPCSERV_LISTENING)&&(state != RPCSERV_PROCESSING)) {
    THROW(daq_exception_factory.CreateException(DAQCSTR("DAQRPCServ::Accept() Can't process without Listen()ing"),DAQEXCPID(DAQInval)));
    return(-1);
  }

  hand = -1;
  if (IsPersistent()) Cleaner();

  // Accept a new call.
  try {
    state = RPCSERV_ACCEPTING;
    if (IsPersistent()) {
      do {
        pollevts = POLLIN;
        rpollevts = dcom.Poll(pollevts,timeout);
        if (rpollevts > 0) {
          ncom = dcom.Accept(info);
          break;
        } else {
          Cleaner();
        }
      } while(rpollevts >= 0);
    } else {
      ncom = dcom.Accept(info);
    }

    state = RPCSERV_PROCESSING;

    if (persist) {
      hand = PersistentCall_h;
      msg = "";
    } else { 
      try {
        rc = ncom.Recv(msg);
      } catch(...) {
        state = RPCSERV_LISTENING;
        ncom.Close();
//        THROW(daq_exception_factory.CreateException(DAQCSTR("DAQRPCServ::Accept() Failed to receive message"),DAQEXCPID(DAQUnknown)));
        return(-1);
      } 

      if (rc < 0) {
        state = RPCSERV_LISTENING;
        ncom.Close();
//        THROW(daq_exception_factory.CreateException(DAQCSTR("DAQRPCServ::Accept() Failed to receive message"),DAQEXCPID(DAQUnknown)));
        return(-1);
      }

      if ((hand = GetHandle(msg)) < 0) {
        state = RPCSERV_LISTENING;
        ncom.Close();
//        THROW(daq_exception_factory.CreateException(DAQCSTR("DAQRPCServ::Accept() Couldn't retrieve handle from message"),DAQEXCPID(DAQInval)));
        return(-1);
      } 

      if ((hand > hi_hand)||(hand < 0)) {
        state = RPCSERV_LISTENING;
        ncom.Close();
//        THROW(daq_exception_factory.CreateException(DAQCSTR("DAQRPCServ::Accept() Handle doesn't exist"),DAQEXCPID(DAQInval)));
        return(-1);
      }    

      if (handtab[hand] == NULL) {
        state = RPCSERV_LISTENING;
        ncom.Close();
//        THROW(daq_exception_factory.CreateException(DAQCSTR("DAQRPCServ::Accept() Handler is NULL"),DAQEXCPID(DAQInval)));
        return(-1);
      }
    }

    itm = new DAQRPCQueueItem(hand,ncom,msg);

    if (!rqueue.Prepend(itm)) {  // Put it at the beginning
      delete itm;  // If we can't add it, then
                   // get rid of it.
    }

    ncom.Close();                         // Will really close if
                                          // this is the last reference
                                          // (if we deleted above)

    if (persist) {
      hand = PersistentCall_h;
    } else {
      hand = -1;
      itm = rqueue.GetHead();
      if (itm != NULL) hand = itm->GetHandle();
    }

    return(hand);
  } catch (NSCLException& e) {  
    state = RPCSERV_LISTENING;
    THROW(e);
    return(-1);
  }

  state = RPCSERV_LISTENING;

  if (persist) {
    hand = PersistentCall_h;
  } else {
    hand = -1;
    itm = rqueue.GetHead();
    if (itm != NULL) hand = itm->GetHandle();
  }

  return(hand);
}

/*===================================================================*/
// DAQRPCServ::GetSelectable
//                                   
// Retrieve selectable
//                                 
DAQSelectObj& DAQRPCServ::GetSelectable()
{
  return(dcom.GetSelectable()); 
}

/*===================================================================*/
// DAQRPCServ::GetURL
//                                   
// Retrieve URL
//                                 
DAQURL DAQRPCServ::GetURL()
{
  return(dcom.GetURL()); 
}

/*===================================================================*/
// DAQRPCServ::Schedule()
//                                                                  
// Schedule a persistent thread.
//
bool DAQRPCServ::Schedule(DAQCommunicator& aCom,int aArgc,char **pArgs)
{
  DAQRPCServThread *thrd = NULL;

  thrd = new DAQRPCServThread(hi_hand,handtab,aCom,aArgc,pArgs); 
  sync_begin(sthrdsync);
    sthrdqueue.Append(thrd); 
  sync_end;

  daq_dispatcher.Dispatch(*thrd); 

  return(true);
}

/*===================================================================*/
// DAQRPCServ::Schedule()
//                                                                  
// Schedule a persistent thread.
//
bool DAQRPCServ::Schedule(DAQCommunicator& aCom,DAQObject *pArgs)
{
  DAQRPCServThread *thrd = NULL;

  thrd = new DAQRPCServThread(hi_hand,handtab,aCom,pArgs); 
  sync_begin(sthrdsync);
    sthrdqueue.Append(thrd); 
  sync_end;

  daq_dispatcher.Dispatch(*thrd); 

  return(true);
}

/*===================================================================*/
// DAQRPCServ::Cleaner()
//                                                                  
// Clean up after threads.
//
bool DAQRPCServ::Cleaner()
{
  DAQRPCServThread *thrd = NULL;
  DAQThreadHashData *tdat = NULL;

  // Now delete all the dead threads
  sync_begin(sthrdsync);
    thrd = sthrdqueue.GetHead();
    do {
      if (thrd == NULL) break; 
  
      if (thrd->killme) {
          if ((tdat = daq_threadtable(thrd->GetId().Get().val.tid)) != NULL) {
            if (tdat->GetRunStat() == THREAD_RUNSTAT_ZOMBIE) {
              sthrdqueue.Delete(thrd);
              thrd->Purge();
              delete (DAQRPCServThread *)thrd;
            }
          }
      }
      thrd = sthrdqueue.GetNext();
    } while (!sthrdqueue.AtHead());
  sync_end; // sthrdsync
  return(true);
}
