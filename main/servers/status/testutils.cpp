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