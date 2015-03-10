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
# @file   CVarMgrApi.h
# @brief  Variable database manager client API
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARMGRAPI_H
#define CVARMGRAPI_H

#include <stdexcept>
#include <string>
#include <vector>
#include <map>


/**
 * @class CVarMgrAPI
 *
 *   Abstract base class that defines a common interface that can be used
 *   for either locally connected variable databases or as server clients.
 *   This API only addresses the functionality in the server REQ/REP
 *   socket pair.
 */
class CVarMgrApi
{
public:
    typedef std::vector<std::string> EnumValues;
    typedef std::map<std::string, std::vector<std::string> > StateMap;
public:
    virtual void cd(const char* path = "/") = 0;
    virtual void mkdir(const char* path)    = 0;
    virtual void rmdir(const char* path)    = 0;
    virtual void declare(const char* path, const char* type, const char* initial=0) = 0;
    virtual void set(const char* path, const char* value) = 0;
    virtual std::string get(const char* path) = 0;
    virtual void defineEnum(const char* typeName, EnumValues values) = 0;
    virtual void defineStateMachine(const char* typeName, StateMap transitions) = 0;
    void addTransition(StateMap& map, std::string fromState, std::string  toState);
    bool validTransitionMap(StateMap map);
protected:
    
    std::pair<std::string, std::string> findInvalidTransition(StateMap map);
    std::string                         findUnreachableState(StateMap map);
    
    // nested classes:
public:
    class CException : public std::runtime_error {
    public:
        CException(std::string what) noexcept :
            runtime_error(what) {}
        CException(const char* what) noexcept :
            runtime_error(what) {}    
    };
};

#endif