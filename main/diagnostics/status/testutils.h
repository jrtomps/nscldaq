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
# @file   testutils.h
# @brief  define useful functions for testing.
# @author <fox@nscl.msu.edu>
*/
#ifndef TESTUTILS_H
#define TESTUTILS_H
#include <string>
#include <vector>
#include <iostream>

std::vector<std::string> marshallVector(const char* s);
std::ostream& operator<<(std::ostream& s, const std::vector<std::string>& v);

#endif