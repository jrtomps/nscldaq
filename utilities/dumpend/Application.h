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
# @file    Application.h
# @brief  The application class.
# @author <fox@nscl.msu.edu>
*/
#ifndef APPLICATION_H
#define APPLICATION_H
#include "options.h"

class CEndRunInfo;

/**
 * @class Application
 *     This class contains the code that drives the application.  Embedding it
 *     in a class allows it to be driven by some other mechanism than
 *     command line arguments.
 */

class Application {
private:
    struct gengetopt_args_info& m_Args;
    
public:
    Application(gengetopt_args_info& args);
    ~Application();                               // Final.
    
public:
    void operator()();
    
    // Utilities:
    
private:
    void processFile(const char* name);
    void dumpEndRunInfo(const char* name, CEndRunInfo& erInfo);
    void dumpBodyHeader(int i, CEndRunInfo& erInfo);
    void dumpBody(int i, CEndRunInfo& erInfo);
};

#endif