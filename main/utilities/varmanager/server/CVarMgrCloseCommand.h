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
# @file   CVarMgrCloseCommand.h
# @brief  close api command for Tcl bindings.
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARMGRCLOSECOMMAND_H
#define CVARMGRCLOSECOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLInterpreterObject;

/**
 * @class CVarMgrCloseCommand
 *    Provides the close command for the database api Tcl bindings.
 *    close destroys the underlying API Object, and invalidates the
 *    handle.
 *
 * \verbatim
 *     varmgr::close handle-value
 * \endverbatim
 */
class CVarMgrCloseCommand : public CTCLObjectProcessor {
public:
    CVarMgrCloseCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrCloseCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif
