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
# @file   CTCLEvbInstance.h
# @brief  Instance command for event builder manipulation on a specific
#         db.
# @author <fox@nscl.msu.edu>
*/
#ifndef CTCLEVBINSTANCE_H
#define CTCLEVBINSTANCE_H

#include <TCLObjectProcessor.h>
#include "CVardbEventBuilder.h"

class CTCLInterpreter;
class CTCLObject;


/**
 * @class CTCLEvbInstance
 *    Command for instances of connections that manipulate the event builder
 *    schema of the variable database.
 *    This is a command ensemble whose subcommands closely match the method
 *    names for the CVardbEventBuilder class.
 */
class CTCLEvbInstance : public CTCLObjectProcessor
{
private:
    CVardbEventBuilder* m_pApi;

public:
    CTCLEvbInstance(
        CTCLInterpreter& interp, const char* cmd, CVardbEventBuilder* pApi
    );
    virtual ~CTCLEvbInstance();
    
public:
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Command methods:
private:
    void haveSchema(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void createSchema(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void createEventBuilder(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void evbSetHost(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void evbSetCoincidenceInterval(
       CTCLInterpreter& interp, std::vector<CTCLObject>& objv     
    );
    void evbSetSourceId(CTCLInterpreter& interp, std::vector<CTCLObject>& objv );
    void evbSetServicePrefix(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
    void evbDisableBuild(CTCLInterpreter& interp, std::vector<CTCLObject>& objv );
    void evbEnableBuild(CTCLInterpreter& interp, std::vector<CTCLObject>& objv );
    void evbSetTimestampPolicy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void evbSetServiceSuffix(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void rmevb(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void evbInfo(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void evbList(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Utilities
private:
    
    // Dict utilities:
    
    bool getDictValue(
        std::string& value, CTCLInterpreter& interp, Tcl_Obj* dict,
        const char* key
    );
    bool getDictValue(
        unsigned& value, CTCLInterpreter& interp, Tcl_Obj* dict,
        const char* key
    );
    bool getDictValue(
        bool& value, CTCLInterpreter& interp, Tcl_Obj* dict,
        const char* key
    );
    void setDictValue(
        CTCLInterpreter& interp, Tcl_Obj* pDict, const char* key,
        const char* value
    );
    void setDictValue(
        CTCLInterpreter& interp, Tcl_Obj* pDict, const char* key,
        unsigned value
    );
    void setDictValue(
        CTCLInterpreter& interp, Tcl_Obj* pDict, const char* key, bool value
    );
    
    // Event builder utilities.
    
    std::string tsPolicyToText(CVardbEventBuilder::TimestampPolicy policy);
    CVardbEventBuilder::TimestampPolicy textToTsPolicy(std::string strPolicy);
    
    void evbInfoToDict(
        CTCLObject& infoDict, CTCLInterpreter& interp,
        CVardbEventBuilder::pEvbDescription pInfo
    );
    
};


#endif
