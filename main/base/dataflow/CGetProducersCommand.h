/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file CGetProducersCommand.h
 * @brief TCL Command to get the hosts that are pumping data into proxy rings.
 *
 */
#ifndef CGETPRODUCERSCOMMAND_H
#define CGETPRODUCERSCOMMAND_H

#include <TCLObjectProcessor.h>


class CTCLInterpreter;
class CTCLObject;

/**
 * @class CGetProducersCommand
 *      TCL Command processor that returns the host names of the producers
 *      that are pushing data into the specified host's proxy ringbuffers.
 *      The form of this command is:
 *
 *  \verbatim
 *      producers  ?hostname?
 *  \endverbatim
 *
 *      If omitted, hostname defaults to localhost, getting the producer hosts
 *      for the system the script is running in.
 */
class CGetProducersCommand : public CTCLObjectProcessor
{
public:
    CGetProducersCommand(CTCLInterpreter& interp, const char* command = "producers");
    virtual ~CGetProducersCommand();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
};

#endif