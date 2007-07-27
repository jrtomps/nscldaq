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
#ifndef __ASSEMBLERERRORS_H
#define __ASSEMBLERERRORS_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class CTCLInterpreter;

/*!
   Class with only static members for error
   reporting in the assembler.
*/
class AssemblerErrors {
public:
    typedef enum _ErrorCode {
        InvalidSubcommand,
        DuplicateNode,
        DuplicateId,
        NoDnsName,
        NoSuchHost,
        BadId,
        NoSuchId,
        NoTriggerNode,
        NoWindowWidth,
        BadWindowWidth,
        BadOffset,
        TooManyParameters,
        TooFewParameters,
	BadEventSize,
	InvalidType,
	InvalidEventBody,
	ExceptionEvent,
	AlreadyExists,
	Running,
	Stopped,
	DoesNotExist,
	Empty,
     INVALIDERRORCODE
    } ErrorCode;
        
    static std::string errorText(ErrorCode code);
    static int setErrorMsg(CTCLInterpreter& interp,
                           ErrorCode code,
                           std::string messageTail);
};


#endif
