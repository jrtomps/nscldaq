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
# @file   CVariableDb.h
# @brief  Header for CVariableDb - class that creates/open variable databases.
# @author <fox@nscl.msu.edu>
*/
#ifndef __CVARIABLEDB_H
#define __CVARIABLEDB_H

#include <CSqlite.h>                // We are derived from that.
#include <exception>
#include <string>

/**
 * @class CVariableDb
 *    This class represents a connection to a variable database file.
 *    -   The create static method can be used to create a new file.
 *    -   Construction represents connection to an existing file but
 *        will fail if the minimal set of tables is not present.
 */
class CVariableDb : public CSqlite
{
public:
    class CException : public std::exception
    {
        private:
           std::string m_message;
        public:
            CException(std::string message) noexcept;
            CException(const CException& rhs) noexcept;
        public:
            CException& operator=(const CException& rhs) noexcept;
            virtual const char* what() const noexcept;
    };

public:
    static void create(const char* pFilePath);
    
    CVariableDb(const char* pfilePath);
    virtual ~CVariableDb();

private:
    void checkSchema();
    
 
    static void createSchema(CSqlite& db);
    
};



#endif