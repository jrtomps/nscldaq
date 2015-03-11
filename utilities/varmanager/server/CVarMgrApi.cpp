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
# @file   CVarMgrApi.cpp
# @brief  Implementation of common utilities for variable manager apis.
# @author <fox@nscl.msu.edu>
*/
#include "CVarMgrApi.h"
#include <set>

/**
 * addTransition
 *   Add a transition to  state map.
 *   -   If the state is not yet defined, it is created.
 *   -   If the toState is a duplicate a duplicate transition is registered
 *       (which is not a fatal thing).
 *  @param[in,out] map    state transition map that will be modified.
 *  @param fromState[in]  State from which this transition originates.
 *  @param toState[in]    State to which this transition takes the state machine.
 */
void
CVarMgrApi::addTransition(
    StateMap& map, std::string fromState, std::string  toState
)
{
    map[fromState].insert(toState);
    
}

/**
 * validTransitinoMap
 *   Determines if a transition map has errors. The following errors are
 *   detected:
 *   - Transitions to nonexistent states.
 *   - Unreachable states
 *   
 *   
 * @param map - The map being checked.
 * @return bool - true if there are neither transitions to non-existent states
 *                nor unreachable states.
 * @note Initial states may be perfectly ok to be unreachable.
 */
bool
CVarMgrApi::validTransitionMap(StateMap map)
{
    std::set<std::string> reachableStates;
    std::set<std::string> states;
    
    /* Iterate over the state transitions looking for to states that are not
     * valid and building up the reachableStates and states sets.
     */
     
    for(StateMap::iterator pState = map.begin(); pState != map.end(); pState++) {
        states.insert(pState->first);
        std::set<std::string>::iterator i = pState->second.begin();
        for (; i != pState->second.end(); i++) {
            std::string to = *i;
            reachableStates.insert(to);
            if (map.find(to) == map.end()) {
                return false;                 // Transition to no such state.
            }
        }
    }
    /* By now all to states are valid.  If there are states that
     * are not in the reachable states set, however we can have an
     * unreachable initial state:
     */
    std::set<std::string>::iterator p = states.begin();
    for(
        p++;                          // Initial states can be unreachable
        p != states.end();
        p++
    ) {
        if (reachableStates.count(*p) == 0) {
            return false;         // Unreachable state.
        }
    }
        // All states are reachable so:
        
    return true;
}
/**
 * Utilities for base class members.
 */

/**
 * findInvalidTransition
 *    Return a pair containing the from/to states of an invalid
 *    transition.
 *
 *   @param map  - The map for which to find an invalid transition.
 *   @return pair<string, string> Consisting of the from to states of a
 *                 transition to an undefined state.
 *   @throw CVarMgrApi::CException - if there is no invalid transition.
 *   
 */
std::pair<std::string, std::string>
CVarMgrApi::findInvalidTransition(CVarMgrApi::StateMap map)
{
    for(StateMap::iterator pState = map.begin(); pState != map.end(); pState++) {
        std::set<std::string>::iterator i = pState->second.begin();
        while(i != pState->second.end()) {
           std::string to = *i;
           if (map.find(to) == map.end()) {
               return std::pair<std::string, std::string>(pState->first, to);
           }
           i++;
       }
    }
    // No invalid states:
    
    throw CException("There are no transitions to undefined states");
    
}
/**
 * findUnreachableState
 *    Return a string that is the name of a state that has no
 *    transitions to it.  Such a state is unreachable.  This is
 *    ok if the state is the initial state but not for any other state.
 *
 *  @param map map to check
 *  @return std::string - name of the first unreachable non-initial state.
 *  @throw CVarMgrApi::CException - if all states are reachable.
 */
std::string
CVarMgrApi::findUnreachableState(CVarMgrApi::StateMap map)
{
    std::set<std::string> reachableStates;
    std::set<std::string> states;
    
    /* Iterate over the state transitions fillig in the state and
     * reachable state set.
     */
     
    for(StateMap::iterator pState = map.begin(); pState != map.end(); pState++) {
        states.insert(pState->first);
        std::set<std::string>::iterator i = pState->second.begin();
        for (; i != pState->second.end(); i++) {
            std::string to = *i;
            reachableStates.insert(to);
        }
    }
    /*
     * Find the first non-reachable non initial state
     */
    std::set<std::string>::iterator p = states.begin();
    for(
        p++;                          // Initial states can be unreachable
        p != states.end();
        p++
    ) {
        if (reachableStates.count(*p) == 0) {
            return *p;
        }
    }
    throw CException("All non initial states are reachable");
}