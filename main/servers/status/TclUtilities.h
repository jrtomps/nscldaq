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
# @file   TclUtilities.h
# @brief  Class with utility methods used by the package:
# @author <fox@nscl.msu.edu>
*/

#ifndef TCLUTILITIES_H
#define TCLUTILITIES_H

#include <stdint.h>
#include <vector>
#include <string>
#include <zmq.hpp>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class TclMessageUtilities
 *      Provides common utility services for the statusMessage package:
 *
 */
class TclMessageUtilities
{
public:
    static zmq::context_t& m_zmqContext;
    static std::vector<std::string> stringVectorFromList(CTCLObject& obj);
    static uint64_t uint64FromObject(
        CTCLInterpreter& interp, CTCLObject& obj,
        const char* doing = "Getting a uint64_t from a command argument"
    );
};

#endif
