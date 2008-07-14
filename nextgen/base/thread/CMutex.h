#ifndef __CMUTEX_H
#define __CMUTEX_H
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CRT_PTHREAD_H
#include <pthread.h>
#ifndef __CRT_PTHREAD_H
#define __CRT_PTHREAD_H
#endif
#endif

/*!
  Direct encapsulation of a pthread mutex attribute block.  We only support the
attributes in the base pthreads specification (not the realtime nor the
advanced realtime specs), as you should be able to count on those always being
available

*/
struct CMutexAttr
{
  pthread_mutexattr_t  m_attributes;
  CMutexAttr();
  ~CMutexAttr();

  void setShared();
  void setUnshared();
  bool isShareable();
  
  void setType(int type);
  int  getType();

  static void  throwifbad (int status, const char* message);
};


/*!
  Direct encapsulation of a pthread  mutex.

*/
class CMutex
{
private:
  pthread_mutex_t      m_mutex;            //  The underlying mutex.
  
public:
  CMutex();
  CMutex(pthread_mutexattr_t& attributes);
  CMutex(CMutexAttr&          attributes);
  virtual ~CMutex();

private:
  CMutex(const CMutex&);
  CMutex& operator=(const CMutex&);
  int operator==(const CMutex&) const;
  int operator!=(const CMutex&) const;



  // Synchronization operations.
public:

  void lock();
  bool trylock();
  void unlock();

private:
  void create(pthread_mutexattr_t* pAttributes);
};

#endif
