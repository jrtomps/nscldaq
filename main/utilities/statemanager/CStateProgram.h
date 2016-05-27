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
# @file   CStateProgram.h
# @brief  Provide API for manipulating state programs.
# @author <fox@nscl.msu.edu>
*/

#ifndef CSTATEPROGRAM_H
#define CSTATEPROGRAM_H
#include <vector>
#include <string>

class CVarMgrApi;

/**
 * @class CStateProgram
 *    Factors all the code to actually define manipulate and list state
 *    programs from the CStateManager class to a separate class.  This was done
 *    because
 *
 *    #   CStateManager is a bit of a kitchen sink.
 *    #   We want to allow applications to manipulate the state program database
 *        without a server and the CStateManager class requires a sub URI as well
 *        as a pub URI eliminating that possibility.
 */
class CStateProgram {
    // Public data types:
    
public:
    typedef struct _ProgramDefinition {
        bool        s_enabled;
        bool        s_standalone;
        std::string s_path;
        std::string s_host;
        std::string s_outRing;
        std::string s_inRing;
    } ProgramDefinition, *pProgramDefinition;
    
    // Object data:
    
private:
    CVarMgrApi*   m_pApi;
    bool          m_ownedApi;
    bool          m_canTransact;

    // canonicals
public:
    CStateProgram(const char* uri);
    CStateProgram(CVarMgrApi* pApi);
    virtual ~CStateProgram();
  
    // Managing programs:
public:
    std::string       getProgramParentDir();
    void              setProgramParentDir(const char* path);
    void              addProgram(const char* name, const pProgramDefinition def);
    ProgramDefinition getProgramDefinition(const char* name);
    void              modifyProgram(const char* name, const pProgramDefinition def);
    void              enableProgram(const char* name);
    void              disableProgram(const char* name);
    bool              isProgramEnabled(const char* name);
    void              setProgramStandalone(const char* name);
    void              setProgramNoStandalone(const char* name);
    bool              isProgramStandalone(const char* name);
    std::vector<std::string> listPrograms();
    std::vector<std::string> listEnabledPrograms();
    std::vector<std::string> listStandalonePrograms();
    std::vector<std::string> listInactivePrograms();
    std::vector<std::string> listActivePrograms();
    void               deleteProgram(const char* name);
    void               setEditorPosition(const char* name, int x, int y);
    int                getEditorXPosition(const char* name);
    int                getEditorYPosition(const char* name);
private:
    std::string getProgramDirectoryPath(const char* name);
    std::string  getVarpath(const char* program, const char* name);
    void         setProgramVar(
        const char* program, const char* var, const char* value
    );
    std::string getProgramVar(const char* program, const char* var);
    bool        getProgramBool(const char* program, const char* var);
    std::string intToString(int v);
    

};

#endif