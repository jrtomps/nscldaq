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

/**
 * @file CPreBeginCommand.cpp
 * @brief prebegin command implementation.
 */

#include "CPreBeginCommand.h"
#include <TCLObjectProcessor.h>
#include <TCLObject.h>
#include <Exception.h>

#include "CConfiguration.h"
#include "Globals.h"
#include "CVMUSBHighLevelController.h"


#include <stdexcept>

/**
 * constructor
 *   @param interp - interpreter the command will be registered under.
 */
CPreBeginCommand::CPreBeginCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "prebegin", true) {}
    
/**
 * destructor
 */
CPreBeginCommand::~CPreBeginCommand() {}

/**
 *  operator() - executes the actual command.
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - command words.
 *  @return int   - TCL_OK - command succeeded.  TCL_ERROR - command failed
 */
int
CPreBeginCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
}