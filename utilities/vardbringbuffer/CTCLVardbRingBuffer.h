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

##exit

# @file   CTCLVardbRingBuffer.h
# @brief  nscldaq::vardbringbuffer command.  Create/delete api instances.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLVARDBRINGBUFFER_H
#define CTCLVARDBRINGBUFFER_H

#include <TCLObjectProcessor.h>                // Base class.
#include <map>
#include <string>
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CTCLVardbRingBuffer
 *
 *     The Tcl bindings for the ring buffer consist of an API instance
 *     manager and instances.   The instance manager (factory) allows you
 *     to create and delete instances connected to specific databases.
 *     The instances allow you to perform operations on a database that
 *     are relevant to a specific database/connection.
 */
class CTCLVardbRingBuffer : public CTCLObjectProcessor
{
private:
    std::map<std::string, CTCLObjectProcessor*> m_Instances;
public:
    CTCLVardbRingBuffer(CTCLInterpreter& interp, const char* command);
    virtual ~CTCLVardbRingBuffer();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};

#endif
