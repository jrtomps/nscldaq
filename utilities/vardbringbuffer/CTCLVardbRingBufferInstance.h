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
# @file   CTCLVardbRingBufferInstance.h
# @brief  Instance of a bound vardb ring buffer api.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLVARDBRINGBUFFERINSTANCE_H
#define CTCLVARDBRINGBUFFERINSTANCE_H
#include <vector>

#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;
class CVardbRingBuffer;



/**
 *  @class CTCLVardbRingBufferInstance
 *     Encapsulates an instance of a Tclbinding to a CVardbRingBuffer api object.
 */
 class CTCLVardbRingBufferInstance : public CTCLObjectProcessor
 {
 private:
    CVardbRingBuffer* m_pApi;
public:
    CTCLVardbRingBufferInstance(
        CTCLInterpreter& interp, const char* command, const char* uri
    );
    virtual ~CTCLVardbRingBufferInstance();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
 };

#endif
