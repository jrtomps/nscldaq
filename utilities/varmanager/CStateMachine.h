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
# @file   CStateMachine.h
# @brief  External API for state machine data types.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CSTATEMACHINE_H
#define _CSTATEMACHINE_H

#include <vector>
#include <map>
#include <set>
#include <string>
#include <stdexcept>

class CVariableDb;

/**
 * @class CStateMachine
 *   This class provides a set of static methods that allow you to
 *   create and manipulate state machines data types.
 *
 *   A key data type is CStateMachine::TransitionMap.  This is a map of sets of
 *   strings.  The map is indexed by a from state.  Each element of the set is
 *   the name of a state that can be entered from the index state.
 *
 *   Transition maps can be a pain to build up so a helper is provided.
 *   
 */
class CStateMachine {
public:
    typedef std::map<std::string, std::set<std::string> > TransitionMap;
public:
    static void addTransition(
        TransitionMap& map, std::string from, std::string to
    );
    static int create(
        CVariableDb& db, std::string typeName,
        TransitionMap transitions
    );
    static bool isStateMachine(CVariableDb& db, int typeId);
    
    static std::vector<std::string> validNextStates(
        CVariableDb& db, int typeId, std::string from
    );
    static std::vector<std::string> validNextStates(
        CVariableDb& db, int varId
    );
    static TransitionMap getTransitionMap(
        CVariableDb& db, int typeId
    );
    // Utilities:
private:
    static int addState(
        CVariableDb& db, int typeId, std::string type
    );
    static void addTransition(CVariableDb& db,  int from, int to);

public:    
    class CException : public std::runtime_error {
    public:
        explicit CException(const std::string& msg);
    };


    
};


#endif