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
# @file   TclUtilties.cpp
# @brief  Implements common utitilties for the package.
# @author <fox@nscl.msu.edu>
*/
#include "TclUtilities.h"
#include <tcl.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLException.h>


zmq::context_t& TclMessageUtilities::m_zmqContext(*(new zmq::context_t(1)));
/**
 * stringVectorFromList
 *    Turn a CTCLObject that contains a list to an std::vector<std::string.
 *    The object is assumed bound to an interpreter.
 *  @param obj - The object being analyzed.
 *  @return std::vector<std::string>
 */
std::vector<std::string>
TclMessageUtilities::stringVectorFromList(CTCLObject& obj)
{
    std::vector<std::string> result;
        for (int i = 0; i < obj.llength(); i++) {
        result.push_back(std::string(obj.lindex(i)));
    }
    return result;
}

/*
 * uint64FromObject
 *    Fetches a uint64_t from a CTCLObject.
 *
 *  @param interp - interpreter to use to parse the object.
 *  @param obj    - Object we're getting the value from.
 *  @param pDoing - String documenting what's being done.  This is part of the
 *                  error exception if the parse fails.
 *  @return uint64_t
 *  @throw  CTCLException if the parse fails.
 */
uint64_t
TclMessageUtilities::uint64FromObject(
    CTCLInterpreter& interp, CTCLObject& obj, const char* pDoing
)
{
    static_assert(
        sizeof(long) >= sizeof(uint64_t),
        "Long is not wide enough for a uint64_t"
    );   // Ensure we're not chopping.
    
    uint64_t result;
    Tcl_Obj* tclObj = obj.getObject();
    int status = Tcl_GetLongFromObj(interp.getInterpreter(), tclObj, reinterpret_cast<long*>(&result));
    if (status != TCL_OK) {
        throw CTCLException(
            interp, 0, "Failed to get number of operations from command line"
        );       
    }
    return result;
}