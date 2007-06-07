/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/
#include <config.h>

#include "AssemblerErrors.h"

#include <TCLInterpreter.h>

using namespace std;

// The following is a table of error messages strings:

static const char * const pErrorMessages[] =
{
    "Invalid sub command",
    "Node with this name already exists",
    "Node with this id has already been defined",
    "This node cannot be found in DNS",
    "No Such host",
    "Invalid node id, node ids are positive integers less than 65536",
    "There is no node with this node id",
    "No trigger node has been defined",
    "No window width has been defined",
    "The window width must be a positive integer",
    "The offset must be a valid integer",
    "Too many parameters on the command line",
    "Too few parameters on the command line",
    "INVALID ERROR CODE!"
};
static const unsigned int MESSAGECOUNT(sizeof(pErrorMessages)/sizeof(char*));

/*!
   Turn an error code into a text message.
   \param code  - the error code to translate.
    
    If the code is not valid, the INVALIDERRORCODE message is returned.
 */
string
AssemblerErrors::errorText(AssemblerErrors::ErrorCode code)
{
    unsigned intCode = static_cast<unsigned int>(code);
    if (intCode >= MESSAGECOUNT) {
        return string(pErrorMessages[INVALIDERRORCODE]);
    }
    return string(pErrorMessages[intCode]);
}
/*!
   Set an interpreter result with an error code string
   and a trailer message.   returns TCL_ERROR.
   \param interp   - The interpreter whose result is set.
   \param code     - the code for the error message.
   \param messageTail - Stuff that will follow the message string.
*/
int
AssemblerErrors::setErrorMsg(CTCLInterpreter& interp, 
			     AssemblerErrors::ErrorCode code,
                             string messageTail)
{
    string result = errorText(code);
    result       += '\n';
    result       += messageTail;

    interp.setResult(result);

    return TCL_ERROR;
}
