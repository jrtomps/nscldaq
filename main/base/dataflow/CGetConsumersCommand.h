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
 * @file CGetConsumersCommand.h
 * @brief Tcl command to determine which hosts consume our ring buffer data.
 */


#ifndef CGETCONSUMERSCOMMAND_H
#define CGETCONSUMERSCOMMAND_H

#include <TCLObjectProcessor.h>

class CTCLInterpreter;
class CTCLObject;


/**
 * @class CGetConsumersCommand
 *
 *    Provides the 'consumers' command  This returns a list of the
 *    hosts that consume data from our ring buffers (via proxy ring pipelines).
 *    Command usage is:
 *
 *  \verbatim
 *     consumers ?host?
 *  \endverbatim
 *
 *    If the optional 'host' parameter is not supplied, it defaults to 'localhost'.
 */
class CGetConsumersCommand : public CTCLObjectProcessor
{
public:
    CGetConsumersCommand (CTCLInterpreter& interp, const char* command="consumers");
    virtual ~CGetConsumersCommand ();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif