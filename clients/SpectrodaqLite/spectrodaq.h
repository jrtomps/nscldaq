#ifndef SPECTRODAQ_H
#define SPECTRODAQ_H

/*
  This file is a miscellany of stuff that attempts to 
  centralize the changes needed to migrate spectrodaq client/data source
  software to spectrodaq-lite.  Note that a full 100% compatible facade
  may not be possible, but we can at least get close.
*/

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif


// All this stuff is dependent on the daq highway and 
// spdaqlite stuff...which we assume is sufficiently protected
// by our spectrodaq_h guard.

#include <dshapi/daqhwyapi.h>
#include <dshnet/daqhwynet.h>


using namespace daqhwyapi;
using namespace daqhwynet;

#include <spdaq/spdaqlite.h>
using namespace spdaq;


typedef daqhwyapi::String DAQString;



/*!
  DAQWordBufferPtr - minimal wrapping of an int16_t* so that it's
                     compatible with spectrodaq as much as possible.

*/
class DAQWordBufferPtr {
private:
  uint16_t*   m_pInitial;
  uint16_t*   m_pData;
public:
  DAQWordBufferPtr(DAQWordBuffer* pBuffer) :
    m_pInitial(pBuffer->GetPtr()),
    m_pData(m_pInitial)
    {}
  DAQWordBufferPtr(uint16_t* p) :
    m_pInitial(p),
    m_pData(p) {}
  DAQWordBufferPtr() :
    m_pInitial(0),
    m_pData(0) {}		/* need to assign to this e.g. */
  DAQWordBufferPtr(const DAQWordBufferPtr* rhs) :
    m_pInitial(rhs->m_pInitial),
    m_pData(rhs->m_pData)
    {}

  uint16_t& operator*();
  uint16_t& operator[](int index);
  DAQWordBufferPtr& operator++();    // ++object
  DAQWordBufferPtr operator++(int); // object++;
  DAQWordBufferPtr& operator+=(int offset);

  int operator==(const DAQWordBufferPtr& rhs) {
    return m_pData == rhs.m_pData;
  }
  int operator!=(const DAQWordBufferPtr& rhs) {
    return m_pData != rhs.m_pData;
  }

  size_t GetIndex();
  void CopyOut(void* pDest, off_t offset, size_t nwds);
  void CopyIn(void*  pSrc,  off_t offset, size_t nwds);
};

DAQWordBufferPtr operator&(DAQWordBuffer& buffer);


/*!
   DAQROCNode - Readout node minimal wrapping of a Main
   class so that DAQRocNode's will work just fine.
*/
class DAQROCNode : public Main {
private:
  virtual int operator()(int argc, char**argv) = 0;  
public:
  virtual void main(int argc, char* argv[]);
  void SetProcessTitle(const char* pTitle);
  void SetProcessTitle(String title);
  void SetProcessTitle(std::string title) {
    SetProcessTitle(title.c_str());
  }
};

/*!
   DAQURL  - just derived from URL so that we can get a 
             string based constructor:
*/
class DAQURL : public URL 
{
 public:
  DAQURL(std::string url);
  DAQURL(const char* url);
  DAQURL();

  DAQURL& operator=(const char* rhs);

  String GetHostName();
  String GetPath();

 private:
  static int                 port(const char* url);
  static daqhwyapi::String&   host(const char* url);

};

// Wrapper to allow the daq_link_mgr members to work.

static const int  ALLBITS_MASK=0xffffffff;
static const bool COS_RELIABLE=false;
static const bool COS_UNRELIABLE=true;

class DAQLinkMgr {
public:
  int AddSink(DAQURL& url,int mask1,  int mask2, const bool deliveryType);
  void DeleteSink(int sinkid);


private:
  PacketRange maskToRanges(int mask, int careBits);
};

extern DAQLinkMgr daq_link_mgr;

extern void SetProcessTitle(const char* title);
extern void SetProcessTitle(DAQString& title);

// Synchronization:

class DAQThreadMutex {
  Synchronizable m_synchObject;
  SyncGuard*      m_pGuard;
  
public:
  DAQThreadMutex();
  ~DAQThreadMutex();

  void Lock();
  void UnLock();
};

// Threads:

typedef unsigned long daqthread_t;     /* Thread id. */
typedef unsigned long DAQThreadId;     /* Same in other contexts */
#define daqthread_self() dshwrappthread_self()


class DAQThread : public Thread {
private:
  int    m_args;
  char **m_pArgs;
public:
  DAQThread() :
    m_args(0), m_pArgs(0)
  {
  }
  void Detach()  { detach(); }
  void Startup(int args, char** argv);
  void Join() { join(); }
  void Join(int id) {Join(); }
  DAQThreadId GetId() {return getId();}
  void Exit(int status);
  virtual void run();
  virtual int operator()(int argc, char** argv) = 0;

};

class DAQScheduler {
 public:
  void Dispatch(DAQThread& pThread, int argc = 0, char** argv = 0);
};

extern DAQScheduler daq_dispatcher;


#endif
