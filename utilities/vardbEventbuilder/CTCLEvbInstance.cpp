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
# @file   CTCLEvbInstance.cpp
# @brief  Implementation of evb connection instance command.
# @author <fox@nscl.msu.edu>
*/

#include "CTCLEvbInstance.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CVardbEventBuilder.h"


/**
 * constructor
 *    Construct a new instance command.
 *
 *  @param interp - the interpreter the command is registered on.
 *  @param cmd    - Command name string.
 *  @param pApi   - API object used to perform the operations.
 *                  note that this object is assumed dynamically created
 *                  and we own it.
 */
CTCLEvbInstance::CTCLEvbInstance(
    CTCLInterpreter& interp, const char* cmd, CVardbEventBuilder* pApi
) : CTCLObjectProcessor(interp, cmd, true),
    m_pApi(pApi)
{
}

/**
 * destructor
 */
CTCLEvbInstance::~CTCLEvbInstance()
{
    delete m_pApi;
}

int
CTCLEvbInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    return TCL_OK;
}