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
# @file   CVarMgrTransactionCommand.h
# @brief  Header for varmgr::transaction command.
# @author <fox@nscl.msu.edu>
*/

#ifndef CVARMGRTRANSACTIONCOMMAND_H
#define CVARMGRTRANSACTIONCOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;


/**
 * @class CVarMgrTransactionCommand
 *    Header for the varmgr::transaction operation.  This command has the form:
 *
 * \verbatim
 *    ::varmgr::transaction $h script
 *
 *  \endverbatim
 *
 *  If the handle does not support transactions, this command produces an error
 *  with the text "Underlying database transport does not support transactions"
 *
 *  If the handle supports transactions, a transaction is started and the script
 *  is run.
 *
 *  -  If the script status  TCL_OK, TCL_CONTINUE or TCL_RETURN is successful,
 *     the transaction is comitted.
 *  -  If the script status is an error, the transaction is rolled back and
 *     the command produces an error.
 *  -  If the script status is TCL_BREAK the transaction is rolled back but
 *     no error is produced. this allows applications to rollback transactions
 *     without having to perform error handling.
 *  
 */
class CVarMgrTransactionCommand : public CTCLObjectProcessor
{
public:
    CVarMgrTransactionCommand(CTCLInterpreter& interp, const char* pCommand);
    virtual ~CVarMgrTransactionCommand();
    
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif