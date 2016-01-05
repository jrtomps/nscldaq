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
# @file   App.h
# @brief  Application driver code.
# @author <fox@nscl.msu.edu>
*/


#ifndef APP_H
#define APP_H
#include <iostream>
#include <map>
#include <vector>
#include <string>


#include "options.h"
/**
 * @class App
 *    The application class for the scaler sum program.
 */
class App {
public:
    typedef struct _Channel {
        unsigned s_dataSource;
        unsigned s_channel;
        int operator>(const struct _Channel& rhs) const;
        int operator<(const struct _Channel& rhs) const;
        int operator==(const struct _Channel& rhs) const;
        int operator!=(const struct _Channel& rhs) const;
        int operator<=(const struct _Channel& rhs) const;
        int operator>=(const struct _Channel& rhs) const;
    } Channel, *pChannel;
private:
    bool m_omitLabels;
    bool m_flip;
    
    std::vector<std::string>         m_files;
    std::map<Channel, std::string>   m_channelNames;
    
public:
    App(struct gengetopt_args_info& args);
    virtual ~App() {}
   
    void operator()();
    void outputResults(std::ostream& out);
    
    // for testing/debugging.
    
    void dumpScalerNames(std::ostream& out);

private:
    void processNameFile(const char* name);
    std::string getScalerName(unsigned chNum);

};

#endif