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
# @file   CUnknownTypeHandler.h
# @brief  Abstract base class for unknown data type handlers.
# @author <fox@nscl.msu.edu>
*/


#ifndef __CUNKNOWNTYPEHANDLER_H
#define __CUNKNOWNTYPEHANDLER_H

// Forward class definitions.

class CSqlite;
class CTypeFactory;
class CDataType;

/**
 *  @class CUnknownTypeHandler
 *
 *    This abstract base class defines the interface for the handlers
 *    that are called when the type factory fails to find a creator.
 *    The idea is that non primitive types may have data base data that defines
 *    them (consider e.g. an enum).  These types are not registered when a program
 *    starts, but only when needed.  You can think of these non-primitive types as
 *    consisting of type families.  For example, there is an enum type family
 *    which is defined by a type in the variable_types table that has associated
 *    records in the enumerated_values table.
 *      If the type factory base class fails to find
 *    a creator that matches the type, a list of handlers is called passed not
 *    only the type name but also a database reference and the factory itself.
 *    Each handler is expected to understand the data required to define a type from
 *    one type factory.  If the type name looks like it is a type from that
 *    handler's family what it will normally do is:
 *    *   Create a type creator for that instance of the family.
 *    *   Register that type creator with the factory for future lookups.
 *    *   Invoke that type creator to create an instance of the requested type.
 *    *   Return that type creator to the factory.
 *
 *    If the handler does not find a match, it must return Null and the factory
 *    will continue trying handlers until it gets a non null return (sucdess)
 *    or the list of handlers is exhausted (no such type).
 *
 *    @note The order in which handlers are registered with the factory may be
 *          be important as you might have cases where one type family is
 *          derived from another (e.g. a state machine is an enum with
 *          additional constraints on which values can be assigned given the
 *          current value of the enum). In such cases as with C++ try/catch
 *          blocks the type families with the deepest deriviation must be registered
 *          first so that the correct type is created (in the state machine
 *          example above, registering enum then statemachine results in
 *          state machine types being treated as enums...the other way around
 *          allows state machines to be properly instantiated).
 *
 * All of this is a lot of commentary for a very simple class:
 */
class CUnknownTypeHandler
{
public:
    virtual CDataType*
        create(const char* typeName, CSqlite& db, CTypeFactory& factory) = 0;
};

#endif