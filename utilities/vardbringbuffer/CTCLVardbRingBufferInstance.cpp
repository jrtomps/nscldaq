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
# @file   Implements api instances objects that are tcl bindings to CVardbRingBuffer object.
# @brief  <brief description>
# @author <fox@nscl.msu.edu>
*/


#include "CTCLVardbRingBufferInstance.h"
#include "CVardbRingBuffer.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"

/**
 * constructor
 *    @param interp  - interpreter on whic the command is being registered.
 *    @param command - name of the command.
 *    @param uri     - URI specifying connection to the database.
 *    
 */
CTCLVardbRingBufferInstance::CTCLVardbRingBufferInstance(
    CTCLInterpreter& interp, const char* command, const char* uri
) :
    CTCLObjectProcessor(interp, command, true),
    m_pApi(0)
{
        m_pApi = new CVardbRingBuffer(uri);
}

/**
 * destructor
 */
CTCLVardbRingBufferInstance::~CTCLVardbRingBufferInstance()
{
    delete m_pApi;
}

/**
 * operator()
 *    Gains control when the command is executing.
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - command line words.
 *  @return int   -  TCL_OK on success, TCL_ERROR on error.
 */
int
CTCLVardbRingBufferInstance::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    return TCL_OK;                     // stub.
}