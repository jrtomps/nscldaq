/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
/**
 * @file CRingSource.h
 * @brief defines ring specific event builder data source class.
 */
#ifndef __CRINGSOURCE_H
#define __CRINGSOURCE_H

#ifndef __CONFIG_H
#include <config.h>
#ifndef __CONFIG_H
#define __CONFIG_H
#endif
#endif



#ifndef __CEVBCLIENTAPP_H
#include <CEVBClientApp.h>
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __EVBFRAMEWORK_H
#include <EVBFramework.h>
#ifndef __EVBFRAMEWORK_H
#define __EVBFRAMEWORK_H
#endif
#endif

#ifndef __DATAFORMAT_H
#include <DataFormat.h>
#ifndef __DATAFORMAT_H
#define __DATAFORMAT_H
#endif
#endif

// Forward definitions:

class CRingBuffer;
class CRingItem;
struct _PhysicsEventItem;
typedef _PhysicsEventItem *pPhysicsEventItem;

struct gengetopt_args_info;
struct timespec;

/**
 * Provides experiment specific code for the Ring Buffer experiment specific
 * data source.   This takes data from the ring named --ring (TCP/IP if needed)
 * and invokes a user written timestamp extractor in the library defined by
 * --timestampextractor.
 *
 *  The timestamp extractor is event format specific and must be supplied by 
 *  the user.  It must have "C" linkage and have a single of the signature:
 * \verbatim
 *    uint64_t timestamp(pPhysicsEventItem item);
 * \endverbatim
 * 
 * The assumption is that only responses to physics triggers actually have timestamps.
 * all other ring item types either have no timestamp (scaler items e.g.) or are barrier
 * fragments (e.g. BEGIN_RUN.
 *
 */
class CRingSource : public CEVBClientApp 
{
  // Prototype for the timestamp getter:

  typedef uint64_t (*tsExtractor)(pPhysicsEventItem item);

  // attributes:

private:
  struct gengetopt_args_info* m_pArgs;
  CRingBuffer*     m_pBuffer;
  uint32_t         m_sourceId;
  tsExtractor      m_timestamp;
  bool             m_stall;
  uint32_t         m_stallCount;
  bool             m_fOneshot;
  unsigned         m_nEndRuns;
  unsigned         m_nEndsSeen;
  unsigned         m_nTimeout;
  unsigned         m_nTimeWaited;

 
  
  // Canonicals:

public:
  CRingSource(int argc, char** argv);
  virtual ~CRingSource();

private:
  CRingSource(const CRingSource&);
  CRingSource& operator=(const CRingSource&);
  int operator==(const CRingSource&) const;
  int operator!=(const CRingSource&) const;

public:
  virtual void initialize();
  virtual bool dataReady(int ms);
  virtual void getEvents();
  virtual void shutdown();
private:
  uint64_t timedifMs(struct timespec& tlater, struct timespec& tearlier); 
  std::string copyLib(std::string original);

  bool formatPhysicsEvent(pRingItem item, CRingItem* p, ClientEventFragment& frag);
};

#endif
