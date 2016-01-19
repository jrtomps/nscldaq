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
# @file   CStateProgram.cpp
# @brief  Class that manipulates the state program part of the vardb.
# @author <fox@nscl.msu.edu>
*/

#include "CStateProgram.h"
#include <CVarMgrApi.h>
#include <CVarMgrApiFactory.h>


/**
 * constructor
 *   @param uri -  Uri used to create an API.
 *
 *   Since the factory is used to create the API we will own it and need to
 *   destroy it destruction time.
 */
CStateProgram::CStateProgram(const char* uri) :
    m_pApi(0), m_ownedApi(true)
{
    m_pApi =  CVarMgrApiFactory::create(std::string(uri));
}

/**
 * constructor
 *    @param pApi - An existing api object pointer.
 *
 *   Since we don't know how the api came into being storage management is the
 *   responsibility of the instantiator.  This is provided because we don't
 *   need/want proliferation of API objects, so CStateManager can just hand us
 *   the API its CStateTransitionMonitor held.
 */
CStateProgram::CStateProgram(CVarMgrApi* pApi) :
    m_pApi(pApi), m_ownedApi(false)
{}

/**
 * destructor
 *    If we own the api, delete it:
 */
CStateProgram::~CStateProgram()
{
    if (m_ownedApi) {
        delete m_pApi;
    }
    
}

#ifdef NOTDEFINED
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
#endif