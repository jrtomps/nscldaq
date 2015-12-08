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
# @file   CVardbEventBuilder.h
# @brief  Header for C++ api to maniuplate event builder defs in variable database.
# @author <fox@nscl.msu.edu>
*/
#ifndef CVARDBEVENTBUILDER_H
#define CVARDBEVENTBUILDER_H

class CVarMgrApi;

/**
 * @class CVardbEventBuilder
 *    Provides a C++ API to the event builder part of the variable database.
 *    Using this class you can:
 *    -   Check for/create the directory structure used to maintain event builder
 *        definitions.
 *    -   Create new event builder definitions.
 *    -   Create new data sources for existing event builder definitions.
 *    -   Obtain information about the existing event builder definitions.
 *    -   Obtain informationa bout the data sources that have been defined for
 *        existing event builders.
 *    -   Modify the definitions of event builders.
 *    -   Modify the definitions of event builder data sources.
 *    -   Delete event builder data sources
 *    -   Delete event builders (and by implication their data sources).
 */
class CVardbEventBuilder
{
    // Data types:
public:
    typedef enum _TimestampPolicy {
        earliest, latest, average
    } TimestampPolicy;
    
    // Internal attributes
private:
   CVarMgrApi*   m_pApi;                  // underlying database api.

   // canonicals:

public:
    CVardbEventBuilder(const char* uri);
    ~CVardbEventBuilder();
    
    // schema functions:
    
    bool schemaExists();
    void createSchema();
    
    // Creating event builders:
    
    void createEventBuilder(
        const char* name, const char* host, unsigned coincidenceInterval,
        unsigned outputSourceId = 0, const char* servicePrefix = 0,
        bool build = true, TimestampPolicy = earliest,
        const char* serviceSuffix = 0
    );
    
};

#endif 