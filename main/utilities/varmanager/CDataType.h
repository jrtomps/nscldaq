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
# @file   CDataType.h
# @brief  ABC for all data type classes.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CDATATYPE_H
#define _CDATATYPE_H
#include <string>

/**
 * @class CDataType
 *     Abstract base class for all data types.
 *
 */
class CDataType
{
private:
    int         m_id;
    std::string m_typeName;
    
    // Canonicals:
public:
    CDataType(int id, std::string type) :
        m_id(id), m_typeName(type) {}
    virtual ~CDataType() {}
    
    // getters:
    
    int         id()   const {return m_id;}
    std::string type() const {return m_typeName;}
    
    // Interface implemented by subclasses:
    /*  -  legal -true if attempt is a valid string representation of an instance
     *            of the data type
     *  - defaultValue - default value for variables of this type.
     */
    
    virtual bool        legal(const char* attempt, int varId = -1) const = 0;
    virtual std::string defaultValue()             const = 0;
};

#endif


