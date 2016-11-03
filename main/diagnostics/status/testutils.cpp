/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   testutils.cpp
# @brief  implementations of useful functions fo testing:
# @author <fox@nscl.msu.edu>
*/
#include "testutils.h"
#include <cstring>
#include <CRingMaster.h>
#include <CRingBuffer.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>

std::vector<std::string>
marshallVector(const char* s)
{
  std::vector<std::string> result;
  while(*s) {
    result.push_back(std::string(s));
    s += std::strlen(s) + 1;
  }
  return result;
}
// So we can EQ on vectors e.g.
std::ostream& operator<<(std::ostream& s, const std::vector<std::string>& v)
{
  s <<  "[";
  for (int i = 0; i < v.size(); i++) {
    s << v[i];
    if (i < (v.size() -1)) s << ", ";
  }
  s << "]";
  
  return s;
}

void killRings()
{
  CTCLInterpreter interp;
  CRingMaster master;
  std::string usage = master.requestUsage();
  CTCLObject  oUsage;
  oUsage.Bind(interp);
  oUsage = usage;
  
  // Usage is a list of two element sublists.  The first element of each
  // list is a ring name:
  
  for(int i = 0; i < oUsage.llength(); i++) {
    CTCLObject ringInfo;
    ringInfo.Bind(interp);
    ringInfo = oUsage.lindex(i);
    
    CTCLObject ringName;
    ringName.Bind(interp);
    ringName = ringInfo.lindex(0);
    
    std::string name = std::string(ringName);
    
    CRingBuffer::remove(name);
    
  }
}

std::vector<zmq::message_t*>
receiveMessage(zmq::socket_t* socket)
{
  std::vector<zmq::message_t*> result;
  std::uint64_t more(0);
  size_t   smore(sizeof(more));
  
  do {
    zmq::message_t* pMessage = new zmq::message_t;
    socket->recv(pMessage);
    result.push_back(pMessage);
    
    socket->getsockopt(ZMQ_RCVMORE, &more, &smore);
    
  } while(more);
  
  return result;
}

//  Delete message parts in a multipart message vector:

void
freeMessage(std::vector<zmq::message_t*>& message)
{
  for_each(message.begin(), message.end(), [](zmq::message_t* msg) {
    delete msg;
  });
}

