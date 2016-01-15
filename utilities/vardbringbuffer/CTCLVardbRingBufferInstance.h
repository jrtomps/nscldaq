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
#include "CVardbRingBuffer.h"

class CTCLInterpreter;
class CTCLObject;

struct Tcl_Obj;


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
    
    // subcommand handlers:
    
private:
    void haveSchema(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void createSchema(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setMaxData(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void setMaxConsumers(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void ringInfo(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Utilities:
private:  
    void getDictUnsigned(
        CTCLInterpreter& interp, Tcl_Obj* dict, const char* key,
        unsigned* pValue
    );
    unsigned sizeValue(CTCLObject& obj);
    Tcl_Obj* ringDict(CTCLInterpreter& interp, CVardbRingBuffer::pRingInfo pInfo);
    
    void putDict(
        CTCLInterpreter& interp, Tcl_Obj* pDict, const char* pKey,
        std::string value
    );
    void putDict(
        CTCLInterpreter& interp, Tcl_Obj* pDict, const char* pKey, unsigned value
    );
 };

#endif
