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
# @file   CTCLStateChangeMessage.cpp
# @brief  Implement Tcl bindings to State change messages.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLStateChangeMessage.h"
#include "CTCLRingStatistics.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>

unsigned CTCLStateChangeMessage::m_instanceNumber(0);
