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
#ifndef RINGSELECTORMAIN_H
#define RINGSELECTORMAIN_H

#include <string>

#include "RingBufferQueue.h"
#include <unistd.h>
#include <memory>

class CRingSelectionPredicate;
struct gengetopt_args_info;

namespace DAQ {
class CDataSource;
}

/*!
   This class is the ring selector application. Written as a separate class, we
   can think of ways to get some automated tests written that don't require running the program.
*/
class RingSelectorMain {
  // Object storage:
private:
  DAQ::CDataSource*              m_pRing;          // Data source.
  std::shared_ptr<CRingSelectionPredicate>  m_pPredicate;     // Predicate used to select data.
  bool                      m_formatted;      // Format output.
  bool                      m_exitOnEnd;      // If true exit when end run seen.
  bool                      m_nonBlocking;    // IF true use non-blocking mode.
  Queues                    m_queues;         // inter-thread communication.
  // Constructors..
public:
  RingSelectorMain();
  ~RingSelectorMain();

  // uniimplemented canonicals/constructors:

private:
  RingSelectorMain(const RingSelectorMain&);
  RingSelectorMain& operator=(const RingSelectorMain&);
  int operator==(const RingSelectorMain&) const;
  int operator!=(const RingSelectorMain&) const;

  // Entry point:

public:
  int operator()(int argc, char** argv);

  // Sub-chunks of the program:

  std::shared_ptr<CRingSelectionPredicate>  createPredicate(struct gengetopt_args_info* parse);
  DAQ::CDataSource*                selectRing(struct gengetopt_args_info* parse);
  void                      processData();


  // Utilities:

  void        writeBlock(int fd, void* pData, size_t size);
};
#endif
