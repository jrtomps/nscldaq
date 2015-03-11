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
# @file   CVarMgrFileApi.cpp
# @brief  Implement API for file database.
# @author <fox@nscl.msu.edu>
*/

#include "CVarMgrFileApi.h"
#include <CVariableDb.h>
#include <CVarDirTree.h>
#include <CVariable.h>
#include <CEnumeration.h>
#include <CStateMachine.h>

/**
 * constructor
 *    Constructs an instance of the API connected to a specific database file.
 *    -   A data base object is created.
 *    -   A dirtree object is also created in order to maintain a cwd.
 * @param[in] pFilePath - Path to the file being attached.
 * @throw std::runtime_error derived exception, if the constructors for either
 *        object fail.
 */

CVarMgrFileApi::CVarMgrFileApi(const char* pFilePath) :
    m_pDb(0), m_pWd(0)
{
    try {
        m_pDb = new CVariableDb(pFilePath);
        m_pWd = new CVarDirTree(*m_pDb);
    } catch(...) {
        // clean up any created objects.
        
        delete m_pWd;
        delete m_pDb;
        
        
        throw;            // make the caller handle this.
    }
}
/**
 * destructor
 *    Destroy the dynamically allocated objects.
 */
CVarMgrFileApi::~CVarMgrFileApi()
{
    delete m_pWd;
    delete m_pDb;
    
}
/**
 * mkdir
 *   Create a new directory.
 *   @param[in] path - The path to the new directory.
 *   @throw std::runtime_error derived exceptino if the
 *          underlying m_pDb->mkdir call fails.
 *   @note -any missing intermediate diretories will be created as well.
 */
void
CVarMgrFileApi::mkdir(const char* path)
{
    m_pWd->mkdir(path);
}

/**
 * cd
 *    Change the API's default working directory to the path.
 * @param[in] path - Path for the new wd.
 * @throw std::runtime_error derived exception if e.g. the path does not exist.
 */
void
CVarMgrFileApi::cd(const char* path )
{
    m_pWd->cd(path);
}
/**
 * getwd
 *    Return the working directory path.
 *
 *   @return std::string - cwd for m_pWd
 */
std::string CVarMgrFileApi::getwd()
{
    return m_pWd->wdPath();
}
/**
 * rmdir
 *    Remove a directory:
 *  @param[in] rmdir - the directory to remove.
 *  @throw std::runtime_error derived exception if e.g. the path does not exist.
 */
void CVarMgrFileApi::rmdir(const char* path)
{
    m_pWd->rmdir(path);
}
/**
 * declare
 *    Create a new variable.
 *
 *  @param[in] path - Path to the variable... this may be relative to the cd.
 *  @param[in] type - Data type the variable holds (must be defined).
 *  @param[in] initial - Initial value string...if null (default) the variable's
 *                 default value will be used.
 *  @throw  std::runtime_error derived exception on error
 */
void
CVarMgrFileApi::declare(const char* path, const char* type, const char* initial)
{
    CVariable::create(*m_pDb, *m_pWd, path, type, initial);
}
/**
 * set
 *   Provide a new value for a varaible.
 *
 *  @param[in] path - Path to the variable (can be wd relative).
 *  @param[in] value - Proposed new value.
 *  @throw std::runtime_error derived exception on error.
 */
void
CVarMgrFileApi::set(const char* path, const char* value)
{
    CVariable v(*m_pDb, *m_pWd, path);
    v.set(value);
}
/**
 * get
 *    Return the value of a variable.
 *
 *  @param[in] path  - Path of the variable (possibly wd relative).
 *  @return std::string - Value of the variable.
 *  @throw std::runtime_error derived exception on errors.
 */
std::string
CVarMgrFileApi::get(const char* path)
{
    CVariable v(*m_pDb, *m_pWd, path);
    return v.get();
}
/**
 * defineEnum
 *    Create an enumeration data type.
 *
 * @param[in] typeName  - Name of the new type (must be unique).
 * @param[in] values    - values the type can take.
 * @throw std::runtime_error derived exception in the event of an error.
 */
void
CVarMgrFileApi::defineEnum(const char* typeName, CVarMgrApi::EnumValues values)
{
    CEnumeration::create(*m_pDb, typeName, values);
}
/**
 * defineStateMachine
 *    Define a state machine data type
 *
 *  @param[in] typeName - Name of the new data type (must be unique).
 *  @param[in] transitions - State transition map.
 *
 *  @throw std::runtime_error derived excpeption on errors.
 */
void CVarMgrFileApi::defineStateMachine(const char* typeName, CVarMgrApi::StateMap transitions)
{
    CStateMachine::create(*m_pDb, typeName, transitions);
}