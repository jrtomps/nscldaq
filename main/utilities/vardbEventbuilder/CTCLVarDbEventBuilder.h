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
# @file   CTCLVarDbEventBuilder.h
# @brief  Event builder data API Tcl wrapper.
# @author <fox@nscl.msu.edu>
*/

#ifndef CTCLVARDBEVENTBUILDER_H
#define CTCLVARDBEVENTBUILDER_H
#include <map>
#include <string>

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CTCLVarDbEventBuilder
 *
 *    Class that provides an ensemble (::nscldaq::evb) that wraps the
 *    CVardbEventBuilder API.
 *     We support two subcommands:
 *
 *     * create - create an instance of the interface.
 *     * destroy - destroy an instance of the interface.
 *
 *    Instances of the interface are, themselves, command ensembles that operate
 *    over a specific database connection.
 */
class CTCLVarDbEventBuilder : public CTCLObjectProcessor
{
private:
    std::map<std::string, CTCLObjectProcessor*> m_Connections;
public:
   CTCLVarDbEventBuilder(CTCLInterpreter* pInterp, const char* command);
   virtual ~CTCLVarDbEventBuilder();
   
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};


#endif